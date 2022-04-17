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

export def generate-makefile [] {
	let data = analyze-src

	let targets = (
		$data.file
		| each { |it| $it | str replace '(.+)\.c' '$1.o' }
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
		| each { |entry| $"($entry.target): ($entry.input) ($entry.prerequisites)\n" }
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
RM := rm

EXC := ltlr
FLAGS_INCLUDE := -Ivendor/raylib/src
FLAGS := -std=c17 -Wall -DPLATFORM_DESKTOP
FLAGS_LIBRARY := -lm -lpthread -ldl -Lbuild/vendor/raylib/desktop -lraylib

INPUT := \\
($makefile-inputs)\n
OUTPUT := $\(patsubst src/%.c,src/%.o, $\(INPUT))

($rules | str collect "\n")
src/$\(EXC): $\(OUTPUT)
\t$\(CC) $\(FLAGS) -o $@ $^ $\(FLAGS_LIBRARY)

.PHONY: clean
clean:
\t$\(RM) -f $\(OUTPUT)
\t$\(RM) -f src/$\(EXC)\n"
	| str trim
}
