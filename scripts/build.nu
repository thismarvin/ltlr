let cc = 'gcc'
let make = 'mingw32-make'

let directory-build = 'build\release\desktop'
let directory-bin = 'bin\release\desktop'

let directory-raylib = 'vendor\raylib'

if (ls $directory-raylib | length) == 0 {
	error make { msg: $"(ansi wi)Please(ansi reset) run (ansi cb)git submodule update --init --recursive(ansi reset)!" }
}

let flags = '-std=c17 -O3 -DPLATFORM_DESKTOP'
let flags-include = '-I.\vendor\raylib\src'
let flags-library = '-L.\build\vendor\raylib\desktop -static -lraylib -lopengl32 -lgdi32 -lwinmm'
let exc = 'ltlr'

let files-source = (
	(ls src/**/*.c).name
	| wrap 'input'
)
let files-object = (
	$files-source
	| each { |it| $it.input | str find-replace '(src)(\\\w+)*((\\\w+)(\.c))' '$2$4.o' }
	| each { |it| $"($directory-build)($it)" }
	| wrap 'output'
)

let data = ($files-source | merge { $files-object })

let commands-setup = $"
rmdir /s /q src\\vendor
rmdir /s /q build
rmdir /s /q bin

cd vendor\\raylib\\src
($make) PLATFORM=PLATFORM_DESKTOP CUSTOM_CFLAGS=-DSUPPORT_CUSTOM_FRAME_CONTROL
cd ..
cd ..
cd ..

mkdir src\\vendor
mkdir build
mkdir build\\release
mkdir build\\release\\desktop
mkdir build\\release\\desktop\\vendor
mkdir build\\vendor
mkdir build\\vendor\\raylib
mkdir build\\vendor\\raylib\\desktop
mkdir bin
mkdir bin\\release
mkdir bin\\release\\desktop
mkdir bin\\release\\desktop\\resources

copy vendor\\cJSON\\cJSON.h src\\vendor
copy vendor\\cJSON\\cJSON.c src\\vendor
move vendor\\raylib\\src\\libraylib.a build\\vendor\\raylib\\desktop
move vendor\\raylib\\src\\raudio.o build\\vendor\\raylib\\desktop
move vendor\\raylib\\src\\rcore.o build\\vendor\\raylib\\desktop
move vendor\\raylib\\src\\rglfw.o build\\vendor\\raylib\\desktop
move vendor\\raylib\\src\\rmodels.o build\\vendor\\raylib\\desktop
move vendor\\raylib\\src\\rshapes.o build\\vendor\\raylib\\desktop
move vendor\\raylib\\src\\rtext.o build\\vendor\\raylib\\desktop
move vendor\\raylib\\src\\rtextures.o build\\vendor\\raylib\\desktop
move vendor\\raylib\\src\\utils.o build\\vendor\\raylib\\desktop
"

let commands-build = (
	$data
	| each { |it| $"($cc) ($flags) ($flags-include) -c ($it.input) -o ($it.output)" }
)

let commands-link = (
	($files-object).output | str collect ' '
	| $"($cc) ($flags) -o ($directory-bin)\\($exc) ($in) ($flags-library)"
)

$":: Windows Command Prompt Build Script
($commands-setup)
($commands-build | str collect "\n")

($commands-link)
"
| save build.bat
