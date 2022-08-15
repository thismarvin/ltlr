def stagger-path [ path: string ] {
	let segments = ($path | path split)
	let length = ($segments | length)
	
	let staggered = (
		for i in 0..($length - 1) {
			for j in 0..$i {
				$"($segments | get $j)"
			}
		}
	)

	$staggered
	| each { |it| $it | path join }
	| flatten
}

export def "makefile desktop" [
	--out-dir: string # Change the output directory
] {
	let output-directory = (
		if ($out-dir | empty?) {
			'build/desktop'
		} else {
			$out-dir
		}
	)

	let sources = do {
		let input = (ls src/**/*.c).name
		let output = do {
			$input
			| str replace 'src/(.+/)*(.+)\.c' $'($output-directory)/$1$2.o'
		}
		let input = ($input | wrap 'input')
		let output = ($output | wrap 'output')

		($input | merge { $output })
	}
	let content = do {
		let input = do {
			ls content/**/*
			| where $it.type == file
			| get name
		}
		let output = do {
			$input
			| each { |it| $'($output-directory)/($it)' }
		}
		let input = ($input | wrap 'input')
		let output = ($output | wrap 'output')

		($input | merge { $output })
	}

	let kickstarter = do {
		let objects = do {
			$sources.output
			| each { |it| $"\t($it) \\" }
			| ($in | str collect "\n")
			| $"OBJECTS := \\\n($in)"
		}

		let dependencies = do {
			$sources.output
			| str replace '(.+)\.o' '$1.d'
			| each { |it| $"\t($it) \\" }
			| ($in | str collect "\n")
			| $"DEPENDENCIES := \\\n($in)"
		}

		let content = do {
			$content.output
			| each { |it| $"\t($it) \\" }
			| ($in | str collect "\n")
			| $"CONTENT := \\\n($in)"
		}

		[]
		| append $objects
		| append $dependencies
		| append $content
		| str collect "\n\n"
	}

	let rules = do {
		let dependency-rules = (
			"-include $\(DEPENDENCIES)\n"
		)
		let directory-rules = do {
			let directories = do {
				[]
				| append $sources.output
				| append $content.output
				| each { |it| $it | path dirname }
				| uniq
				| each { |it| stagger-path $it }
				| flatten
				| uniq
			}

			def generate-rule [ directory: string ] {
				let target = $directory
				let prerequisites = do {
					let dirname = ($directory | path dirname)
					if (not ($dirname | empty?)) and ($dirname != $target) {
						$' | ($dirname)'
					} else {
						''
					}
				}
				let recipe = 'mkdir $@'

				$"($target):($prerequisites)\n\t($recipe)\n"
			}

			$directories
			| each { |it| generate-rule $it }
			| str collect "\n"
		};
		let object-rules = do {
			def generate-rule [ input: string, output: string ] {
				let target = $output
				let prerequisites = $' ($input) | ($target | path dirname)'
				let recipe = $"$\(CC) $\(CFLAGS) -MMD -o $@ -c $<"

				$"($target):($prerequisites)\n\t($recipe)\n"
			}

			$sources
			| each { |it| generate-rule $it.input $it.output }
			| str collect "\n"
		}
		let content-rules = do {
			def generate-rule [ input: string, output: string ] {
				let target = $output
				let prerequisites = $' ($input) | ($target | path dirname)'
				let recipe = $"cp $< ($target | path dirname)"

				$"($target):($prerequisites)\n\t($recipe)\n"
			}

			$content
			| each { |it| generate-rule $it.input $it.output }
			| str collect "\n"
		}
		let output-rules = do {
			let target = $"($output-directory)/$\(BIN)"
			let prerequisites = $" $\(OBJECTS) | ($target | path dirname)"
			let recipe = $"$\(CC) $\(CFLAGS) -o $@ $^ $\(LDLIBS)"

			$"($target):($prerequisites)\n\t($recipe)"
		}

		[]
		| append $dependency-rules
		| append $directory-rules
		| append $object-rules
		| append $content-rules
		| append $output-rules
		| str collect "\n"
	}

$"# This file is auto-generated; any changes you make may be overwritten.

CC ?= gcc

DESTDIR ?= .
BIN ?= ltlr
BUILD ?= debug
cflags.warnings := -Wall -Wextra -Wpedantic
cflags.debug := -g -pg -Og
cflags.release := -g -O2
CFLAGS := -std=c17 $\(cflags.warnings) $\(cflags.$\(BUILD)) -Ivendor/raylib/src -Ivendor/cJSON/src -DPLATFORM_DESKTOP
ldlibs.vendor = $\(shell pkg-config --libs gl)
LDLIBS := -L$\(DESTDIR)/lib/desktop -lcJSON -lraylib $\(ldlibs.vendor) -lm

$\(VERBOSE).SILENT:

($kickstarter)

.PHONY: @all
@all: @clean @desktop

($rules)

.PHONY: @desktop
@desktop: ($output-directory)/$\(BIN) $\(CONTENT)

.PHONY: @clean
@clean:
	if [ -d \"($output-directory)\" ]; then rm -r ($output-directory); fi
"
	| str trim
}

export def "makefile web" [
	--out-dir: string # Change the output directory
] {
	# TODO(thismarvin): A lot of this is copied from "makefile desktop"...

	let output-directory = (
		if ($out-dir | empty?) {
			'build/web'
		} else {
			$out-dir
		}
	)

	let sources = do {
		let input = (ls src/**/*.c).name
		let output = do {
			$input
			| str replace 'src/(.+/)*(.+)\.c' $'($output-directory)/$1$2.o'
		}
		let input = ($input | wrap 'input')
		let output = ($output | wrap 'output')

		($input | merge { $output })
	}
	let content = do {
		let input = do {
			ls content/**/*
			| where $it.type == file
			| get name
		}
		let output = do {
			$input
			| str replace 'content/(.+/)*(.+)' $'($output-directory)/$1$2'
		}
		let input = ($input | wrap 'input')
		let output = ($output | wrap 'output')

		($input | merge { $output })
	}

	let kickstarter = do {
		let objects = do {
			$sources.output
			| each { |it| $"\t($it) \\" }
			| ($in | str collect "\n")
			| $"OBJECTS := \\\n($in)"
		}

		let dependencies = do {
			$sources.output
			| str replace '(.+)\.o' '$1.d'
			| each { |it| $"\t($it) \\" }
			| ($in | str collect "\n")
			| $"DEPENDENCIES := \\\n($in)"
		}

		let content = do {
			$content.output
			| each { |it| $"\t($it) \\" }
			| ($in | str collect "\n")
			| $"CONTENT := \\\n($in)"
		}

		[]
		| append $objects
		| append $dependencies
		| append $content
		| str collect "\n\n"
	}

	let rules = do {
		let dependency-rules = (
			"-include $\(DEPENDENCIES)\n"
		)
		let directory-rules = do {
			let directories = do {
				[]
				| append $sources.output
				| append $content.output
				| each { |it| $it | path dirname }
				| uniq
				| each { |it| stagger-path $it }
				| flatten
				| uniq
			}

			def generate-rule [ directory: string ] {
				let target = $directory
				let prerequisites = do {
					let dirname = ($directory | path dirname)
					if (not ($dirname | empty?)) and ($dirname != $target) {
						$' | ($dirname)'
					} else {
						''
					}
				}
				let recipe = 'mkdir $@'

				$"($target):($prerequisites)\n\t($recipe)\n"
			}

			$directories
			| each { |it| generate-rule $it }
			| str collect "\n"
		};
		let object-rules = do {
			def generate-rule [ input: string, output: string ] {
				let target = $output
				let prerequisites = $' ($input) | ($target | path dirname)'
				let recipe = $"$\(EMCC) $\(CFLAGS) -MMD -o $@ -c $<"

				$"($target):($prerequisites)\n\t($recipe)\n"
			}

			$sources
			| each { |it| generate-rule $it.input $it.output }
			| str collect "\n"
		}
		let content-rules = do {
			def generate-rule [ input: string, output: string ] {
				let target = $output
				let prerequisites = $' ($input) | ($target | path dirname)'
				let recipe = $"cp $< ($target | path dirname)"

				$"($target):($prerequisites)\n\t($recipe)\n"
			}

			$content
			| each { |it| generate-rule $it.input $it.output }
			| str collect "\n"
		}
		let output-rules = do {
			let target = $"($output-directory)/$\(BIN)"
			let prerequisites = $" $\(OBJECTS) | ($target | path dirname)"
			let recipe = $"$\(CC) $\(CFLAGS) -o $@ $^ $\(LDLIBS)"

			$"($target):($prerequisites)\n\t($recipe)"
		}

		[]
		| append $dependency-rules
		| append $directory-rules
		| append $object-rules
		| append $content-rules
		| append $output-rules
		| str collect "\n"
	}

$"# This file is auto-generated; any changes you make may be overwritten.

EMCC ?= emcc

CFLAGS := -std=c17 -Wall -Wextra -Wpedantic -g -O2 -Ivendor/raylib/src -Ivendor/cJSON/src -DPLATFORM_WEB
LDLIBS := -Llib/web -lraylib -lcJSON
EM_CACHE ?= .emscripten-cache

TOTAL_MEMORY ?= 33554432
SHELL_FILE := src/minshell.html

$\(VERBOSE).SILENT:

($kickstarter)

.PHONY: @all
@all: @clean @web

($rules)
$\(EM_CACHE):
	mkdir $@

($output-directory)/index.html: $\(OBJECTS) | $\(EM_CACHE) ($output-directory)
	$\(EMCC) $\(CFLAGS) -o $@ $^ $\(LDLIBS) -s USE_GLFW=3 -s TOTAL_MEMORY=$\(TOTAL_MEMORY) --memory-init-file 0 --shell-file $\(SHELL_FILE) --preload-file content --cache $\(EM_CACHE)

.PHONY: @web
@web: ($output-directory)/index.html

.PHONY: @clean
@clean:
	if [ -d \"($output-directory)\" ]; then rm -r ($output-directory); fi
"
	| str trim
}
