def 'builder archive' [record] {
	do {
		if (not ($record.outdir | path exists)) {
			mkdir $record.outdir
		}
	}

	do {
		$record.strategy
		| each {
			|it|
				do -s { ^$record.cc $record.cflags -o $it.output -c $it.input }
				| complete
				| if $in.exit_code != 0 { error make { msg: 'An error occurred during compilation!' }}
		}
	}

	do -s { ^$record.ar rcs $record.library $record.strategy.output }
	| complete
	| if $in.exit_code != 0 { error make { msg: 'An error occurred while archiving!' }}
}

def 'builder application' [record] {
	do {
		if (not ($record.outdir | path exists)) {
			mkdir $record.outdir
		}
	}

	do {
		let args = do {
			let out = ([$record.outdir $record.bin] | path join)
			let input = (ls src/**/*.c).name

			[]
			| append $record.cflags
			| append '-o'
			| append $out
			| append $input
			| append $record.ldlibs
		}

		do -s { ^$record.cc $args }
		| complete
		| if $in.exit_code != 0 { error make { msg: 'An error occurred during compilation!' }}
	}

	do {
		let content_directory = ([$record.outdir 'content'] | path join)
		if (not ($content_directory | path exists)) {
			cp -r content $content_directory
		}
	}
}

# Compile ltlr for Windows
export def build [] {
	let cc = do {
		if ('CC' in (env).name) {
			$env.CC
		} else {
			'gcc'
		}
	}
	let ar = do {
		if ('AR' in (env).name) {
			$env.AR
		} else {
			'ar'
		}
	}

	# Build raylib.
	do {
		let outdir = 'target/lib/raylib'

		let strategy = do {
			let input = do {
				(ls 'vendor/raylib/src/*.c').name
				| str replace -a '\\' '/'
				| wrap 'input'
			}
			let output = do {
				$input.input
				| str replace 'vendor/raylib/src/(.+)\.c' $'($outdir)/$1.o'
				| wrap 'output'
			}

			($input | merge $output)
		}
		let cflags = do {
			[]
			| append '-D_DEFAULT_SOURCE'
			| append '-DPLATFORM_DESKTOP'
			| append '-DGRAPHICS_API_OPENGL_33'
			| append '-O1'
			| append '-Ivendor/raylib/src'
			| append '-Ivendor/raylib/src/external/glfw/include'
			| append '-Ivendor/raylib/external/glfw/deps/mingw'
		}

		let record = {
			cc: $cc,
			ar: $ar,
			outdir: $outdir,
			library: 'target/lib/libraylib.a'
			strategy: $strategy
			cflags: $cflags
		}

		builder archive $record
	}

	# Build application.
	do {
		let cflags = do {
			[]
			| append '-std=c17'
			| append '-Wall'
			| append '-Wextra'
			| append '-Wpedantic'
			| append '-g'
			| append '-O2'
			| append '-Ivendor/raylib/src'
			| append '-DPLATFORM_DESKTOP'
		}
		let ldlibs = do {
			# https://github.com/raysan5/raylib/issues/1153#issuecomment-604628379
			[
				'-Ltarget/lib'
				'-static'
				'-lraylib'
				'-lopengl32'
				'-lgdi32'
				'-lwinmm'
			]
		}

		let record = {
			cc: $cc,
			outdir: 'target/bin',
			bin: 'ltlr',
			cflags: $cflags,
			ldlibs: $ldlibs,
		}

		builder application $record
	}
}
