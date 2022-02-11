EMCC := /usr/lib/emscripten/emcc
EMAR := /usr/lib/emscripten/emar

RAYLIB_FILES := rcore.c rshapes.c rtextures.c rtext.c rmodels.c utils.c raudio.c
RAYLIB_SOURCES := $(patsubst %, raylib/src/%, $(RAYLIB_FILES))
RAYLIB_OBJECTS := $(patsubst raylib/src/%.c, raylib/src/%.o, $(RAYLIB_SOURCES))

GAME_HEADERS := $(shell find src -name "*.h")
GAME_SOURCES := $(shell find src -name "*.c")
CONTENT_SOURCES := $(shell find src/resources)

$(VERBOSE).SILENT:

.PHONY: all
all: clean release
	@echo "Done"

build:
	mkdir $@

raylib:
	git clone https://github.com/raysan5/raylib.git
	@echo ""

$(RAYLIB_SOURCES): raylib

$(RAYLIB_OBJECTS): raylib/src/%.o: raylib/src/%.c
	$(EMCC) -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -o $@ -c $<

raylib/src/libraylib.a: $(RAYLIB_OBJECTS)
	$(EMAR) rcs $@ $^
	@echo ""

src/Makefile: raylib

src/raylib_game.o: src/Makefile $(GAME_HEADERS) $(GAME_SOURCES) $(CONTENT_SOURCES)
	cd src; make clean; make

build/index.html: raylib/src/libraylib.a src/minshell.html $(GAME_HEADERS) $(GAME_SOURCES) $(CONTENT_SOURCES) | build
	$(EMCC) -o build/index.html $(GAME_SOURCES) -Os -Wall raylib/src/libraylib.a -I. -Iraylib/src -L. -Lraylib/src -s USE_GLFW=3 --shell-file src/minshell.html -s TOTAL_MEMORY=33554432 --preload-file src/resources -DPLATFORM_WEB

.PHONY: release
release: build/index.html
	@echo Done

.PHONY: dev
dev: src/raylib_game.o
	cd src; ./raylib_game
	@echo Done

.PHONY: format
format:
	astyle -n -xC100 -S -xb -j --style=allman --recursive "src/*.c,*.h"
	prettier --write --use-tabs src/minshell.html

.PHONY: clean
clean:
	if [ -d "raylib" ]; then rm -rf raylib; fi
	if [ -d "build" ]; then rm -rf build; fi
	@echo "Done"
