let build_inputs = (
	$env.buildInputs
	| split row ' '
	| str trim
)

let-env PATH = (
	$build_inputs
	| find -p { |it| $'($it)/bin' | path exists }
	| each { |it| $'($it)/bin' }
	| str collect ':'
)
let-env PKG_CONFIG_PATH = do {
	let lib = (
		$build_inputs
		| find -p { |it| $'($it)/lib/pkgconfig' | path exists }
		| each { |it| $'($it)/lib/pkgconfig' }
		| str collect ':'
	)
	let share = (
		$build_inputs
		| find -p { |it| $'($it)/share/pkgconfig' | path exists }
		| each { |it| $'($it)/share/pkgconfig' }
		| str collect ':'
	)

	[]
	| append $lib
	| append $share
	| str collect ':'
}

let-env VERBOSE = 1
let-env DESTDIR = $env.TMP
let-env BUILD = 'release'
let-env BIN = 'app'

do {
	let makefile = $'($env.TMP)/Makefile.Desktop'
	cd $env.src
	^make -C vendor/raylib -f Makefile.Desktop @lib
	^make -C vendor/cJSON -f Makefile.Desktop @lib
	^nu -c $'use scripts/generate.nu; generate makefile desktop --out-dir ($env.TMP) | save ($makefile)'
	^make -f $makefile @desktop
}

do {
	let runner = $'($env.out)/bin/($env.pname)'
	mkdir $'($env.out)/bin'
	mv $'($env.TMP)/($env.BIN)' $'($env.out)/bin'
	mv $'($env.TMP)/content' $'($env.out)/bin'
	$"#!/bin/sh\n\ncd ($env.out)/bin; ./($env.BIN)" | save $runner
	^chmod +x $runner
}

do {
	cd $'($env.out)/bin'
	^patchelf --shrink-rpath $env.BIN
	^strip $env.BIN
}
