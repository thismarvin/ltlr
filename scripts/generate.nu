def get-sources [output_directory: string] {
	let input = do {
		(ls src/**/*.c).name
		| wrap 'input'
	}
	let output = do {
		$input.input
		| str replace 'src/(.+/)*(.+)\.c' $'($output_directory)/$1$2.o'
		| wrap 'output'
	}

	($input | merge $output)
}

def get-content [output_directory: string] {
	let input = do {
		ls content/**/*
		| where $it.type == file
		| get name
		| wrap 'input'
	}
	let output = do {
		$input.input
		| each { |it| $'($output_directory)/($it)' }
		| wrap 'output'
	}

	($input | merge $output)
}

def stagger-path [ path: string ] {
	let segments = ($path | path split)
	let length = ($segments | length)
	
	(1..$length)
	| each { |it| ($segments | take $it) }
	| each { path join }
}

export def "compilation database" [
	--out-dir: string
] {
	let output_directory = if ($out_dir | is-empty) { 'build/desktop' } else { $out_dir }
	let record = get-sources $output_directory

	let common_arguments = do {
		[]
		| append 'gcc'
		| append '-std=c17'
		| append '-Wall'
		| append '-Wextra'
		| append '-Wpedantic'
		| append '-Ivendor/raylib/src'
		| append '-DPLATFORM_DESKTOP'
	}

	let commands = do {
		def generate [input: string, output: string] {
			let directory = ($env.PWD)
			let arguments = do {
				$common_arguments
				| append '-o'
				| append $output
				| append '-c'
				| append $input
			}
			let arguments = do {
				$arguments
				| each { |it| $""($it)"" }
			}
			let arguments = ($arguments | str collect ', ')
			let file = ($input | path expand)

			$"\t{\n\t\t"directory": "($directory)",\n\t\t"file": "($file)",\n\t\t"arguments": [($arguments)],\n\t}"
		}

		$record
		| each { |it| generate $it.input $it.output }
		| str collect ",\n"
	}

	($"[\n($commands)\n]" | from json | to json)
}

export def "makefile desktop" [
	--out-dir: string # Change the output directory
] {
	let output_directory = if ($out_dir | is-empty) { 'build/desktop' } else { $out_dir }
	let sources = get-sources $output_directory
	let content = get-content $output_directory

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
		let dependency_rules = (
			"-include $\(DEPENDENCIES)\n"
		)
		let directory_rules = do {
			let directories = do {
				[]
				| append $sources.output
				| append $content.output
				| each { path dirname }
				| uniq
				| each { |it| stagger-path $it }
				| flatten
				| uniq
			}

			def generate-rule [ directory: string ] {
				let target = $directory
				let prerequisites = do {
					let dirname = ($directory | path dirname)
					if (not ($dirname | is-empty)) and ($dirname != $target) {
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
		let object_rules = do {
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
		let content_rules = do {
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
		let output_rules = do {
			let target = $"($output_directory)/$\(BIN)"
			let prerequisites = $" $\(OBJECTS) | ($target | path dirname)"
			let recipe = $"$\(CC) $\(CFLAGS) -o $@ $^ $\(LDLIBS)"

			$"($target):($prerequisites)\n\t($recipe)"
		}

		[]
		| append $dependency_rules
		| append $directory_rules
		| append $object_rules
		| append $content_rules
		| append $output_rules
		| str collect "\n"
	}

	[]
	| append $"# This file is auto-generated; any changes you make may be overwritten."
	| append $""
	| append $"CC ?= gcc"
	| append $""
	| append $"DESTDIR ?= ."
	| append $"BIN ?= ltlr"
	| append $"BUILD ?= debug"
	| append $"cflags.warnings := -Wall -Wextra -Wpedantic"
	| append $"cflags.debug := -g -pg -Og"
	| append $"cflags.release := -g -O2"
	| append $"CFLAGS := -std=c17 $\(cflags.warnings) $\(cflags.$\(BUILD)) -Ivendor/raylib/src -DPLATFORM_DESKTOP"
	| append $"ldlibs.vendor := $\(shell pkg-config --libs x11 xrandr xinerama xcursor xi xext xfixes xrender xcb xau xdmcp glfw3)"
	| append $"LDLIBS := -L$\(DESTDIR)/lib/desktop -lraylib $\(ldlibs.vendor) -lrt -ldl -lm"
	| append $""
	| append $kickstarter
	| append $""
	| append $".PHONY: @all"
	| append $"@all: @desktop"
	| append $""
	| append $"$\(VERBOSE).SILENT:"
	| append $""
	| append $rules
	| append $""
	| append $".PHONY: @desktop"
	| append $"@desktop: ($output_directory)/$\(BIN) $\(CONTENT)"
	| append $""
	| append $".PHONY: @clean"
	| append $"@clean:"
	| append $"\tif [ -d \"($output_directory)\" ]; then rm -r ($output_directory); fi"
	| str collect "\n"
}

export def "makefile web" [
	--out-dir: string # Change the output directory
] {
	# TODO(thismarvin): A lot of this is copied from "makefile desktop"...

	let output_directory = if ($out_dir | is-empty) { 'build/web' } else { $out_dir }
	let sources = get-sources $output_directory

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

		[]
		| append $objects
		| append $dependencies
		| str collect "\n\n"
	}

	let rules = do {
		let dependency_rules = (
			"-include $\(DEPENDENCIES)\n"
		)
		let directory_rules = do {
			let directories = do {
				[]
				| append $sources.output
				| each { path dirname }
				| uniq
				| each { |it| stagger-path $it }
				| flatten
				| uniq
			}

			def generate-rule [ directory: string ] {
				let target = $directory
				let prerequisites = do {
					let dirname = ($directory | path dirname)
					if (not ($dirname | is-empty)) and ($dirname != $target) {
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
		let object_rules = do {
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
		let output_rules = do {
			let target = $"($output_directory)/$\(BIN)"
			let prerequisites = $" $\(OBJECTS) | ($target | path dirname)"
			let recipe = $"$\(CC) $\(CFLAGS) -o $@ $^ $\(LDLIBS)"

			$"($target):($prerequisites)\n\t($recipe)"
		}

		[]
		| append $dependency_rules
		| append $directory_rules
		| append $object_rules
		| append $output_rules
		| str collect "\n"
	}

	[]
	| append $"# This file is auto-generated; any changes you make may be overwritten."
	| append $""
	| append $"EMCC ?= emcc"
	| append $""
	| append $"CFLAGS := -std=c17 -Wall -Wextra -Wpedantic -g -O2 -Ivendor/raylib/src -DPLATFORM_WEB"
	| append $"LDLIBS := -Llib/web -lraylib"
	| append $""
	| append $"EM_CACHE ?= .emscripten-cache"
	| append $""
	| append $"TOTAL_MEMORY ?= 33554432"
	| append $"SHELL_FILE := src/minshell.html"
	| append $""
	| append $kickstarter
	| append $""
	| append $".PHONY: @all"
	| append $"@all: @web"
	| append $""
	| append $"$\(VERBOSE).SILENT:"
	| append $""
	| append $rules
	| append $""
	| append $"$\(EM_CACHE):"
	| append $"\tmkdir $@"
	| append $""
	| append $"($output_directory)/index.html: $\(OBJECTS) | $\(EM_CACHE) ($output_directory)"
	| append $"\t$\(EMCC) $\(CFLAGS) -o $@ $^ $\(LDLIBS) -s USE_GLFW=3 -s TOTAL_MEMORY=$\(TOTAL_MEMORY) --memory-init-file 0 --shell-file $\(SHELL_FILE) --preload-file content --cache $\(EM_CACHE)"
	| append $""
	| append $".PHONY: @web"
	| append $"@web: ($output_directory)/index.html"
	| append $""
	| append $".PHONY: @clean"
	| append $"@clean:"
	| append $"\tif [ -d \"($output_directory)\" ]; then rm -r ($output_directory); fi"
	| str collect "\n"
}
