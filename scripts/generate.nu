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
		| str replace 'content/aseprite/(\w+/)*((\w+)\.aseprite)' $"($output-directory)/$1$3.png"
		| wrap 'output'
	)
	let levels-output = (
		$levels-input.input
		| str replace 'content/tiled/(\w+/)*((\w+)\.tmx)' $"($output-directory)/$1$3.json"
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

	let pairs = (
		$required-directories
		| each { |it| [ $it ($it | path dirname) ] }
	)
	let makefile-directory-rules = (
		$pairs
		| each { |pair| if not ($pair.1 | empty?) { $"($pair.0): | ($pair.1)\n" } else { $"($pair.0):\n" } }
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
