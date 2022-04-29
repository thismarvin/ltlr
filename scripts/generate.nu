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

export def "makefile content" [
	--out-dir: string # Change the output directory
] {
	let output-directory = (
		if ($out-dir | empty?) {
			'build/content'
		} else {
			$out-dir
		}
	)

	let images-input = (
		(ls content/aseprite/**/*.aseprite).name
		| wrap 'input'
	)
	let levels-input = (
		(ls content/tiled/**/*.tmx).name
		| wrap 'input'
	)

	let images-output = (
		$images-input.input
		| str replace 'content/aseprite/((?:\w+/)*)(?:(\w+)\.aseprite)' $"($output-directory)/$1$2.png"
		| wrap 'output'
	)
	let levels-output = (
		$levels-input.input
		| str replace 'content/tiled/((?:\w+/)*)(?:(\w+)\.tmx)' $"($output-directory)/$1$2.json"
		| wrap 'output'
	)

	let images = (
		$images-input
		| merge { $images-output }
	)
	let levels = (
		$levels-input
		| merge { $levels-output }
	)

	let images-directories = (
		$images.output
		| each { |it| $it | path dirname }
		| uniq
	)
	let levels-directories = (
		$levels.output
		| each { |it| $it | path dirname }
		| uniq
	)

	let required-directories = (
		[]
		| append $images-directories
		| append $levels-directories
		| uniq
		| each { |it| stagger-path $it }
		| flatten
		| uniq
	)

	let makefile-images-input = (
		$images.input
		| each { |it| $"\t($it) \\" }
		| str collect "\n"
	)
	let makefile-levels-input = (
		$levels.input
		| each { |it| $"\t($it) \\" }
		| str collect "\n"
	)
	let makefile-images-output = (
		$images.output
		| each { |it| $"\t($it) \\" }
		| str collect "\n"
	)
	let makefile-levels-output = (
		$levels.output
		| each { |it| $"\t($it) \\" }
		| str collect "\n"
	)

	let entries = (
		$required-directories
		| each { |it| { dir: $it, parent: ($it | path dirname) }}
	)
	let makefile-directory-rules = (
		$entries
		| each { |it| if not ($it.parent | empty?) { $"($it.dir): | ($it.parent)\n" } else { $"($it.dir):\n" }}
		| each { |it| $"($it)\tmkdir $@\n" }
		| str collect "\n"
	)

	let makefile-image-rules = (
		$images
		| each { |it| $"($it.output): ($it.input) | ($it.output | path dirname)\n" }
		| each { |it| $"($it)\t$\(ASEPRITE) -b $< --save-as $@\n" }
		| str collect "\n"
	)

	let makefile-level-rules = (
		$levels
		| each { |it| $"($it.output): ($it.input) | ($it.output | path dirname)\n" }
		| each { |it| $"($it)\t$\(TILED) --embed-tilesets --export-map json $< $@\n" }
		| each { |it| $"($it)\t$\(PRETTIER) -w --use-tabs $@\n" }
		| str collect "\n"
	)

$"# This file is auto-generated; any changes you make may be overwritten.

ASEPRITE := aseprite
TILED := tiled
PRETTIER := prettier

IMAGES_INPUT := \\
($makefile-images-input)

IMAGES_OUTPUT := \\
($makefile-images-output)

LEVELS_INPUT := \\
($makefile-levels-input)

LEVELS_OUTPUT := \\
($makefile-levels-output)

CONTENT := $\(IMAGES_OUTPUT) $\(LEVELS_OUTPUT)

$\(VERBOSE).SILENT:

.PHONY: all
all: clean content

($makefile-directory-rules)
($makefile-image-rules)
($makefile-level-rules)
.PHONY: content
content: $\(CONTENT)

.PHONY: clean
clean:
	if [ -d \"($output-directory)\" ]; then rm -r ($output-directory); fi
"
}

export def "makefile desktop" [
	--out-dir: string # Change the output directory
] {
	let output-directory = (
		if ($out-dir | empty?) {
			'build'
		} else {
			$out-dir
		}
	)

	let source-headers = (
		(ls src/**/*.h).name
		| wrap 'header'
	)
	let source-input = (
		(ls src/**/*.c).name
		| wrap 'input'
	)

	let source-output = (
		$source-input.input
		| str replace 'src/((?:\w+/)*)(?:(\w+)\.c)' $"($output-directory)/$1$2.o"
		| wrap 'output'
	)

	let source = (
		$source-input
		| merge { $source-output }
	)

	let source-directories = (
		$source.output
		| each { |it| $it | path dirname }
		| uniq
	)

	let required-directories = (
		[]
		| append $source-directories
		| uniq
		| each { |it| stagger-path $it }
		| flatten
		| uniq
	)

	let makefile-source-headers = (
		$source-headers.header
		| each { |it| $"\t($it) \\" }
		| str collect "\n"
	)
	let makefile-source-input = (
		$source.input
		| each { |it| $"\t($it) \\" }
		| str collect "\n"
	)
	let makefile-source-output = (
		$source.output
		| each { |it| $"\t($it) \\" }
		| str collect "\n"
	)

	let entries = (
		$required-directories
		| each { |it| { dir: $it, parent: ($it | path dirname) }}
	)
	let makefile-directory-rules = (
		$entries
		| each { |it| if not ($it.parent | empty?) { $"($it.dir): | ($it.parent)\n" } else { $"($it.dir):\n" }}
		| each { |it| $"($it)\tmkdir $@\n" }
		| str collect "\n"
	)

	let makefile-source-rules = (
		$source
		| each { |it| $"($it.output): ($it.input) $\(SOURCE_HEADERS) | ($it.output | path dirname)\n" }
		| each { |it| $"($it)\t$\(CC) $\(CFLAGS) -o $@ -c $<\n" }
		| str collect "\n"
	)

$"# This file is auto-generated; any changes you make may be overwritten.

CC := gcc

BIN := ltlr
CFLAGS := -std=c17 -Wall -Wextra -Wpedantic -g -pg -O0 -Ivendor/raylib/src -Ivendor/cJSON -DPLATFORM_DESKTOP
LDLIBS := -lm -lpthread -ldl -Llib/desktop -lraylib -lcJSON

SOURCE_HEADERS := \\
($makefile-source-headers)

SOURCE_INPUT := \\
($makefile-source-input)

SOURCE_OUTPUT := \\
($makefile-source-output)

$\(VERBOSE).SILENT:

.PHONY: all
all: clean desktop

($makefile-directory-rules)
($makefile-source-rules)
($output-directory)/$\(BIN): $\(SOURCE_OUTPUT) | ($output-directory)
	$\(CC) $\(CFLAGS) -o $@ $^ $\(LDLIBS)

.PHONY: desktop
desktop: ($output-directory)/$\(BIN)

.PHONY: clean
clean:
	if [ -d \"($output-directory)\" ]; then rm -r ($output-directory); fi
"
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

	let source-headers = (
		(ls src/**/*.h).name
		| wrap 'header'
	)
	let source-input = (
		(ls src/**/*.c).name
		| wrap 'input'
	)

	let source-output = (
		$source-input.input
		| str replace 'src/((?:\w+/)*)(?:(\w+)\.c)' $"($output-directory)/$1$2.o"
		| wrap 'output'
	)

	let source = (
		$source-input
		| merge { $source-output }
	)

	let source-directories = (
		$source.output
		| each { |it| $it | path dirname }
		| uniq
	)

	let required-directories = (
		[]
		| append $source-directories
		| uniq
		| each { |it| stagger-path $it }
		| flatten
		| uniq
	)

	let makefile-source-headers = (
		$source-headers.header
		| each { |it| $"\t($it) \\" }
		| str collect "\n"
	)
	let makefile-source-input = (
		$source.input
		| each { |it| $"\t($it) \\" }
		| str collect "\n"
	)
	let makefile-source-output = (
		$source.output
		| each { |it| $"\t($it) \\" }
		| str collect "\n"
	)

	let entries = (
		$required-directories
		| each { |it| { dir: $it, parent: ($it | path dirname) }}
	)
	let makefile-directory-rules = (
		$entries
		| each { |it| if not ($it.parent | empty?) { $"($it.dir): | ($it.parent)\n" } else { $"($it.dir):\n" }}
		| each { |it| $"($it)\tmkdir $@\n" }
		| str collect "\n"
	)

	let makefile-source-rules = (
		$source
		| each { |it| $"($it.output): ($it.input) $\(SOURCE_HEADERS) | ($it.output | path dirname)\n" }
		| each { |it| $"($it)\t$\(EMCC) $\(CFLAGS) -o $@ -c $<\n" }
		| str collect "\n"
	)

$"# This file is auto-generated; any changes you make may be overwritten.

EMCC := emcc

CFLAGS := -std=c17 -Wall -Wextra -Wpedantic -O3 -Ivendor/raylib/src -Ivendor/cJSON -DPLATFORM_WEB
LDLIBS := -Llib/web -lraylib -lcJSON

TOTAL_MEMORY := 33554432
SHELL_FILE := src/minshell.html

SOURCE_HEADERS := \\
($makefile-source-headers)

SOURCE_INPUT := \\
($makefile-source-input)

SOURCE_OUTPUT := \\
($makefile-source-output)

$\(VERBOSE).SILENT:

.PHONY: all
all: clean web

($makefile-directory-rules)
($makefile-source-rules)
($output-directory)/index.html: $\(SOURCE_OUTPUT) | ($output-directory)
	$\(EMCC) $\(CFLAGS) -o $@ $^ $\(LDLIBS) -s USE_GLFW=3 -s TOTAL_MEMORY=$\(TOTAL_MEMORY) --memory-init-file 0 --shell-file $\(SHELL_FILE) --preload-file content/build

.PHONY: web
web: ($output-directory)/index.html

.PHONY: clean
clean:
	if [ -d \"($output-directory)\" ]; then rm -r ($output-directory); fi
"
}
