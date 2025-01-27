def get-source-files []: nothing -> list<path> {
	[]
	| append (ls src/**/*.h).name
	| append (ls src/**/*.c).name
	| append (ls tests/**/*.h).name
	| append (ls tests/**/*.c).name
}

export def 'main format' [
	--check
] {
	let inputs = get-source-files

	if ($check) {
		^clang-format -n -Werror ...$inputs
	} else {
		^clang-format -i ...$inputs
	}
}

export def 'main lint' [] {
	# clang-tidy only works if a compilation database exists.
	nu scripts/generate.nu compilation database | save -f compile_commands.json

	let inputs = (
		get-source-files
		| where $it !~ 'segments'
	)

	^clang-tidy ...$inputs
}

export def main [] {}
