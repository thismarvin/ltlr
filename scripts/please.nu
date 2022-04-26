export def builder [] {
	let cflags = (
		$env.CFLAGS
		| split row ' '
	)
	let out = (
		[ $env.OUT_DIR $env.BIN ]
		| path join
	)
	let input = (
		(ls src/**/*.c).name
		| str replace '\\' '/'
	)
	let ldlibs = (
		$env.LDLIBS
		| split row ' '
	)

	^$env.CC $cflags -o $out $input $ldlibs
}

export def "builder content" [
	--release (-r) # Format output for release
] {
	let image-sources = (
		(ls content/aseprite/**/*.aseprite).name
		| str replace '\\' '/'
		| wrap 'input'
	)
	let level-sources = (
		(ls content/tiled/**/*.tmx).name
		| str replace '\\' '/'
		| wrap 'input'
	)

	let image-output = (
		$image-sources.input
		| str replace 'content/aseprite/(\w+/)*((\w+)\.aseprite)' $"($env.OUT_DIR)/$1$3.png"
		| wrap 'output'
	)
	let level-output = (
		$level-sources.input
		| str replace 'content/tiled/(\w+/)*((\w+)\.tmx)' $"($env.OUT_DIR)/$1$3.json"
		| wrap 'output'
	)

	let images = (
		$image-sources
		| merge { $image-output }
	)
	let levels = (
		$level-sources
		| merge { $level-output }
	)

	let _ = (
		$images
		| each { |it| ^$env.ASEPRITE -b $it.input --save-as $it.output }
	)
	let _ = (
		$levels
		| each { |it| ^$env.TILED --embed-tilesets --export-map json $it.input $it.output }
	)

	if $release {
		let _ = (
			$levels
			| each { |it| ^$env.PRETTIER -w --use-tabs $it.output }
		)
	}
}

# Compile ltlr
export def build [
	--bin: string # Change the name of the output binary
	--release (-r) # Compile with optimizations enabled
	--out-dir: string # Change the output directory
	--content-out-dir: string # Change the name of the content's output directory
] {
	let output-directory-basename = (
		if ($out-dir | empty?) {
			'target'
		} else {
			$out-dir
		}
	)
	let output-directory = (
		if not $release {
			[$output-directory-basename 'debug'] | path join
		} else {
			[$output-directory-basename 'release'] | path join
		}
	)

	let content-output-directory-basename = (
		if ($content-out-dir | empty?) {
			'content'
		} else {
			$content-out-dir
		}
	)
	let content-output-directory = (
		[$output-directory $content-output-directory-basename ] | path join
	)

	let output = (
		if ($bin | empty?) {
			'ltlr'
		} else {
			$bin
		}
	)
	let optimization-flags = (
		if not $release {
			'-g -pg -O0'
		} else {
			'-O3'
		}
	)

	# Build Content.
	with-env [
		OUT_DIR $content-output-directory
		ASEPRITE 'aseprite'
		TILED 'tiled'
		PRETTIER 'prettier'
	] {
		builder content
	}

	# Build Binary.
	with-env [
		OUT_DIR $output-directory
		BIN $output
		CC 'gcc'
		CFLAGS $"-std=c17 -Wall -Wextra -Wpedantic ($optimization-flags) -Ivendor/raylib/src -Ivendor/cJSON -DPLATFORM_DESKTOP"
		LDLIBS '-lm -lpthread -ldl -Llib/desktop -lraylib -lcJSON'
	] {
		builder
	}
}

export def run [
	--bin: string # Change the name of the output binary
	--release (-r) # Compile with optimizations enabled
	--out-dir: string # Change the output directory
	--content-out-dir: string # Change the name of the content's output directory
] {
	let output-directory = (
		if ($out-dir | empty?) {
			'target'
		} else {
			$out-dir
		}
	)
	let output = (
		if ($bin | empty?) {
			'ltlr'
		} else {
			$bin
		}
	)

	if not $release {
		build --bin $output --out-dir $output-directory --content-out-dir $content-out-dir
		cd ([ $output-directory 'debug' ] | path join)
		^([ './' $output ] | path join)
	} else {
		build --bin $output --release --out-dir $output-directory --content-out-dir $content-out-dir
		cd ([ $output-directory 'release' ] | path join)
		^([ './' $output ] | path join)
	}
}
