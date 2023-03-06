def get-input-output [output_directory: string] {
	let output_directory = ($output_directory | path split)

	let input = do {
		(ls src/**/*.c).name
		| wrap 'input'
	}

	let output = do {
		$input.input
		| each { |it|
				$it
				| path split
				| upsert 0 $output_directory
				| flatten
				| path join
				| path parse
				| upsert extension o
				| path join
		}
		| wrap 'output'
	}

	($input | merge $output)
}

export def "compilation database" [
	--out-dir: string
] {
	let output_directory = do {
		if ($out_dir | is-empty) {
			(["build", "desktop"] | path join)
		} else {
			$out_dir
		}
	}

	let record = get-input-output $output_directory

	let common_arguments = do {
		[]
		| append 'gcc'
		| append '-std=gnu17'
		| append '-Wall'
		| append '-Wextra'
		| append '-Wpedantic'
		| append '-DPLATFORM_DESKTOP'
		| append '-Ivendor/raylib/src'
		| append '-Ivendor/wyhash'
	}

	let commands = do {
		def generate [input: string, output: string] {
			let directory = ($env.PWD)
			let file = ($input | path expand)
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
			let arguments = ($arguments | str join ', ')

			$'{"directory": "($directory)","file": "($file)","arguments": [($arguments)]}'
		}

		$record
		| each { |it| generate $it.input $it.output }
		| str join ",\n"
	}

	($"[\n($commands)\n]" | from json | to json)
}
