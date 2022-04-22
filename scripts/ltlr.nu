def find-respective-header [ file, files ] {
	$files
	| find ($file | str replace '(.+)\.c' '$1.h')
	| if not ($in | empty?) { first } else { '' }
}

def get-includes [ file ] {
	open $file
	| lines
	| find -r '#include\s+".+"'
	| str trim
	| str replace '.*#include\s+"(.+)".*' '$1'
	| path basename
}

def get-local-dependencies [ file, files ] {
	(get-includes $file)
	| each { |it| $files | find $it }
	| flatten
}

export def analyze-files [ c-files, h-files ] {
	let sources = (
		$c-files
		| wrap 'source'
	)

	let headers = (
		$sources.source
		| each { |file| find-respective-header $file $h-files }
		| wrap 'header'
	)

	let pairs = (
		$sources
		| merge { $headers }
	)

	let includes-in-source = (
		$pairs.source
		| each { |file| if not ($file | empty?) { get-local-dependencies $file $h-files } else { [] }}
		| wrap 'lhs'
	)

	let includes-in-header = (
		$pairs.header
		| each { |file| if not ($file | empty?) { get-local-dependencies $file $h-files } else { [] }}
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
	let data = analyze-files (ls src/**/*.c).name (ls src/**/*.h).name
	let unique-directories = get-unique-directories $data.file

	let output-directory = '$(OUT_DIR)'

	let directories = (
		$unique-directories
		| each { |it| $it | str replace 'src' $output-directory }
	)

	let directory-rules = (
		$directories
		| each { |it| $it | path dirname | if not ($in | empty?) { $"($it): | ($in)" } else { $"($it):" }}
		| each { |it| $"($it)\n\t$\(MKDIR) $@\n" }
	)

	let targets = (
		$data.file
		| each { |it| $it | str replace '(src/)((\w+/)*\w+)(\.c)' $"($output-directory)/$2.o" }
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

	let command = '$(CC) $(INCLUDE_FLAGS) $(FLAGS) -o $@ -c $<'
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

OUT_DIR := build
OUT := ltlr

FLAGS := -std=c17 -Wall -Wextra -Wpedantic -DPLATFORM_DESKTOP
INCLUDE_FLAGS := -Ivendor/raylib/src
LIBRARY_FLAGS := -lm -lpthread -ldl -Llibraries/desktop -lraylib

INPUT_FILES := \\
($makefile-inputs)\n
OUTPUT_FILES := $\(patsubst src/%.c,$\(OUT_DIR)/%.o, $\(INPUT_FILES))

$\(VERBOSE).SILENT:

.PHONY: all
all: clean output

($directory-rules | str collect "\n")
($rules | str collect "\n")
$\(OUT_DIR)/$\(OUT): $\(OUTPUT_FILES)
\t$\(CC) $\(FLAGS) -o $@ $^ $\(LIBRARY_FLAGS)

.PHONY: output
output: $\(OUT_DIR)/$\(OUT)

.PHONY: clean
clean:
\tif [ -d $\(OUT_DIR) ]; then $\(RM) -r $\(OUT_DIR); fi\n"
	| str trim
}
