export def "bootstrapper lib" [] {
	mkdir lib
	mkdir lib/desktop
	mkdir lib/desktop/raylib
	mkdir lib/desktop/cJSON
	mkdir lib/web
	mkdir lib/web/raylib
	mkdir lib/web/raylib/cJSON

	^$env.GIT submodule update --init --recursive

	cd vendor/raylib/src
	^$env.MAKE PLATFORM=PLATFORM_DESKTOP CUSTOM_CFLAGS=-DSUPPORT_CUSTOM_FRAME_CONTROL
	cd ../../../
	mv vendor/raylib/src/raudio.o lib/desktop/raylib
	mv vendor/raylib/src/rcore.o lib/desktop/raylib
	mv vendor/raylib/src/rglfw.o lib/desktop/raylib
	mv vendor/raylib/src/rmodels.o lib/desktop/raylib
	mv vendor/raylib/src/rshapes.o lib/desktop/raylib
	mv vendor/raylib/src/rtext.o lib/desktop/raylib
	mv vendor/raylib/src/rtextures.o lib/desktop/raylib
	mv vendor/raylib/src/utils.o lib/desktop/raylib
	mv vendor/raylib/src/libraylib.a lib/desktop

	^$env.CC -std=c89 -O1 -o lib/desktop/cJSON/cJSON.o -c vendor/cJSON/cJSON.c
	^$env.AR rcs lib/desktop/libcJSON.a lib/desktop/cJSON/cJSON.o
}

export def linux [] {
	with-env [
		GIT 'git'
		MAKE 'make'
		CC 'gcc'
		AR 'ar'
	] {
		bootstrapper lib
	}
}

export def windows [] {
	with-env [
		GIT 'git'
		MAKE 'mingw32-make'
		CC 'gcc'
		AR 'ar'
	] {
		bootstrapper lib
	}
}
