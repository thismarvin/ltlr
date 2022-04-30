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
		| str replace 'content/aseprite/((?:\w+/)*)(?:(\w+)\.aseprite)' $"($env.OUT_DIR)/$1$2.png"
		| wrap 'output'
	)
	let level-output = (
		$level-sources.input
		| str replace 'content/tiled/((?:\w+/)*)(?:(\w+)\.tmx)' $"($env.OUT_DIR)/$1$2.json"
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
		| each { |it|
			^$env.ASEPRITE -b $it.input --save-as $it.output
			| complete
			| if $in.exit_code != 0 { error make { msg: 'An error occurred during compilation!' }}
		}
	)
	let _ = (
		$levels
		| each { |it|
			^$env.TILED --embed-tilesets --export-map json $it.input $it.output
			| complete
			| if $in.exit_code != 0 { error make { msg: 'An error occurred during compilation!' }}
		}
	)

	if $release {
		let _ = (
			$levels
			| each { |it|
				^$env.PRETTIER -w --use-tabs $it.output
				| complete
				| if $in.exit_code != 0 { error make { msg: 'An error occurred during compilation!' }}
			}
		)
	}
}

export def "builder desktop" [] {
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

	# Make sure the directories required by output exist.
	let _ = (
		mkdir $env.OUT_DIR
	)

	^$env.CC $cflags -o $out $input $ldlibs
	| complete
	| if $in.exit_code != 0 { error make { msg: 'An error occurred during compilation!' }}
}

export def "builder web" [] {
	let cflags = (
		$env.CFLAGS
		| split row ' '
	)
	let out = (
		[ $env.OUT_DIR 'index.html' ]
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
	let total-memory = $"TOTAL_MEMORY=($env.TOTAL_MEMORY)"

	# Make sure the directories required by output exist.
	let _ = (
		mkdir $env.OUT_DIR
	)

	^$env.EMCC $cflags -o $out $input $ldlibs -s USE_GLFW=3 -s $total-memory --memory-init-file 0 --shell-file $env.SHELL_FILE --preload-file $env.CONTENT_DIR
	| complete
	| if $in.exit_code != 0 { error make { msg: 'An error occurred during compilation!' }}
}

# Compile ltlr
export def build [
	--bin: string # Change the name of the output binary
	--release (-r) # Compile with optimizations enabled
	--out-dir: string # Change the output directory
	--content-out-dir: string # Change the name of the content's output directory
	--target: string # Change the target platform the project will be built for
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

	let target = (
		if ($target | empty?) {
			'linux'
		} else {
			$target
		}
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
			'-O2'
		}
	)

	let ldlibs = (
		if $target == 'linux' {
			'-lm -lpthread -ldl -Llib/desktop -lraylib -lcJSON'
		} else if $target == 'windows' {
			'-Llib/desktop -static -lraylib -lcJSON -lopengl32 -lgdi32 -lwinmm'
		} else {
			error make {
				msg: $"'($target)' is not a supported target."
			}
		}
	)

	# Build Content.
	with-env [
		OUT_DIR $content-output-directory
		ASEPRITE 'aseprite'
		TILED 'tiled'
		PRETTIER 'prettier'
	] {
		if not $release {
			builder content
		} else {
			builder content --release
		}
	}

	# Build Binary.
	with-env [
		OUT_DIR $output-directory
		BIN $output
		CC 'gcc'
		CFLAGS $"-std=c17 -Wall -Wextra -Wpedantic ($optimization-flags) -Ivendor/raylib/src -Ivendor/cJSON -DPLATFORM_DESKTOP"
		LDLIBS $ldlibs
	] {
		builder desktop
	}
}

export def run [
	--bin: string # Change the name of the output binary
	--release (-r) # Compile with optimizations enabled
	--out-dir: string # Change the output directory
	--content-out-dir: string # Change the name of the content's output directory
	--target: string # Change the target platform the project will be built for
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
		build --bin $output --out-dir $output-directory --content-out-dir $content-out-dir --target $target
		cd ([ $output-directory 'debug' ] | path join)
		^([ './' $output ] | path join)
	} else {
		build --bin $output --release --out-dir $output-directory --content-out-dir $content-out-dir --target $target
		cd ([ $output-directory 'release' ] | path join)
		^([ './' $output ] | path join)
	}
}

export def 'build web' [
	--out-dir: string # Change the output directory
] {
	let output-directory-basename = (
		if ($out-dir | empty?) {
			'target'
		} else {
			$out-dir
		}
	)
	let output-directory = (
		[$output-directory-basename 'web'] | path join
	)

	# Build Content.
	with-env [
		OUT_DIR 'build/content'
		ASEPRITE 'aseprite'
		TILED 'tiled'
		PRETTIER 'prettier'
	] {
		builder content --release
	}

	with-env [
		OUT_DIR $output-directory
		EMCC 'emcc'
		CFLAGS '-std=c17 -Wall -Wextra -Wpedantic -O2 -Ivendor/raylib/src -Ivendor/cJSON -DPLATFORM_WEB'
		LDLIBS '-Llib/web -lraylib -lcJSON'
		SHELL_FILE 'src/minshell.html'
		TOTAL_MEMORY '33554432'
		CONTENT_DIR 'build/content'
	] {
		builder web
	}
}
