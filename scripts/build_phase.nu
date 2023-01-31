let-env VERBOSE = 1
let-env DESTDIR = $env.TMP
let-env BUILD = 'release'

let makefile = $'($env.TMP)/Makefile.Desktop'

cd $env.src

nu -c $'use scripts/generate.nu; generate makefile desktop --out-dir ($env.TMP) | save ($makefile)'

^make -C vendor/raylib -f Makefile.Desktop @lib -j $env.NIX_BUILD_CORES
^make -f $makefile @desktop -j $env.NIX_BUILD_CORES
