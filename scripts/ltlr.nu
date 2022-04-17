def find-respective-header [ file, files ] {
	$files
	| find ($file | str replace '(.+)\.c' '$1.h')
	| if not ($in | empty?) { first } else { '' }
}

def get-includes [ file ] {
	open $file
	| lines
	| find -r '\s*#include\s+.+'
	| str replace '\s*#include\s+["<](.+)[">]' '$1'
	| path basename
}

def get-local-dependencies [ file, files ] {
	(get-includes $file)
	| each { |it| $files | find $it }
	| flatten
}

export def analyze-src [] {
	let files-c = (ls src/**/*.c).name
	let files-h = (ls src/**/*.h).name

	let sources = (
		$files-c
		| wrap 'source'
	)

	let headers = (
		$sources.source
		| each { |file| find-respective-header $file $files-h }
		| wrap 'header'
	)

	let pairs = (
		$sources
		| merge { $headers }
	)

	let includes-in-source = (
		$pairs.source
		| each { |file| if not ($file | empty?) { get-local-dependencies $file $files-h } else { [] }}
		| wrap 'lhs'
	)

	let includes-in-header = (
		$pairs.header
		| each { |file| if not ($file | empty?) { get-local-dependencies $file $files-h } else { [] }}
		| wrap 'rhs'
	)

	let includes = (
		($includes-in-source | merge { $includes-in-header })
		| each { |pair| $pair.lhs | append $pair.rhs | uniq }
		| each { |it| if not ($it | empty?) { $it | sort } else { $it }}
		| wrap 'includes'
	)

	($sources.source | wrap 'file')
	| merge { $includes }
}

def get-unique-directories [ files ] {
	$files
	| each { |it| $it | path dirname }
	| uniq
	| if not ($in | empty?) { $in | sort } else { [] }
}

export def generate-makefile [] {
	let data = analyze-src
	let unique-directories = get-unique-directories $data.file

	let directory-output = '$(DIR_OUT)'

	let directories = (
		$unique-directories
		| each { |it| $it | str replace 'src' $directory-output }
	)

	let directory-rules = (
		$directories
		| each { |it| $it | path dirname | if not ($in | empty?) { $"($it): | ($in)" } else { $"($it):" }}
		| each { |it| $"($it)\n\t$\(MKDIR) $@\n" }
	)

	let targets = (
		$data.file
		| each { |it| $it | str replace '(src/)((\w+/)*\w+)(\.c)' $"($directory-output)/$2.o" }
		| wrap 'target'
	)

	let prerequisites = (
		$data.includes
		| each { |it| $it | str collect ' ' }
		| wrap 'prerequisites'
	)

	let builder = (
		$targets
		| merge { $data.file | wrap 'input' }
		| merge { $prerequisites }
	)

	let recipes = (
		$builder
		| each { |entry| $"($entry.target): ($entry.input) ($entry.prerequisites) | ($entry.target | path dirname)\n" }
	)

	let command = '$(CC) $(FLAGS_INCLUDE) $(FLAGS) -o $@ -c $<'
	let rules = (
		$recipes
		| each { |it| $"($it)\t($command)\n" }
	)

	let makefile-inputs = (
		$data.file
		| each { |it| $"\t($in) \\" }
		| str collect "\n"
	)

$"# This file is auto-generated; any changes you make may be overwritten.

CC := gcc
MKDIR := mkdir
RM := rm

DIR_OUT := build
OUT := ltlr

FLAGS_INCLUDE := -Ivendor/raylib/src
FLAGS := -std=c17 -Wall -DPLATFORM_DESKTOP
FLAGS_LIBRARY := -lm -lpthread -ldl -Llibraries/desktop -lraylib

INPUT := \\
($makefile-inputs)\n
OUTPUT := $\(patsubst src/%.c,$\(DIR_OUT)/%.o, $\(INPUT))

all: clean output

($directory-rules | str collect "\n")
($rules | str collect "\n")
$\(DIR_OUT)/$\(OUT): $\(OUTPUT)
\t$\(CC) $\(FLAGS) -o $@ $^ $\(FLAGS_LIBRARY)

.PHONY: output
output: $\(DIR_OUT)/$\(OUT)

.PHONY: clean
clean:
\tif [ -d $\(DIR_OUT) ]; then rm -rf $\(DIR_OUT); fi\n"
	| str trim
}
