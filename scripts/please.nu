export def builder [] {
	let cflags = (
		$env.CFLAGS
		| split row ' '
	)
	let ldlibs = (
		$env.LDLIBS
		| split row ' '
	)

	let input = (
		(ls src/**/*.c).name
		| wrap 'input'
	)

	let output = (
		$input.input
		| str replace 'src/(\w+/)*((\w+)\.c)' $"($env.OUT_DIR)/$1$3.o"
		| wrap 'output'
	)

	let pair = (
		$input
		| merge { $output }
	)

	let _ = (
		$output.output
		| each { |it| $it | path dirname }
		| uniq
		| each { |it| mkdir $it }
	)

	let _ = (
		$pair
		| each { |pair| ^$env.CC $cflags -o $pair.output -c $pair.input }
	)

	^$env.CC $cflags -o ([ $env.OUT_DIR $env.BIN] | path join) $output.output $ldlibs
}

export def build [ 
	--bin: string # Change the name of the output binary
	--release (-r) # Compile with optimizations enabled
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
		if not $release {
			[$output-directory-basename 'debug'] | path join
		} else {
			[$output-directory-basename 'release'] | path join
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
			'-O3'
		}
	)

	with-env [
		OUT_DIR $output-directory
		BIN $output
		CC 'gcc'
		CFLAGS $"-std=c17 -Wall -Wextra -Wpedantic ($optimization-flags) -Ivendor/raylib/src -DPLATFORM_DESKTOP"
		LDLIBS '-lm -lpthread -ldl -Llib/desktop -lraylib'
	] {
		builder
	}
}
