export def format [
	--check
] {
	let inputs = do {
		[]
		| append (ls src/**/*.h).name
		| append (ls src/**/*.c).name
		| append (ls tests/**/*.h).name
		| append (ls tests/**/*.c).name
	}

	if ($check) {
		^clang-format -n -Werror $inputs
	} else {
		^clang-format -i $inputs
	}
}
