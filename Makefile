$(VERBOSE).SILENT:

VENDOR := vendor/raylib/src/raylib.h vendor/cJSON/cJSON.h
EXTERNAL := src/vendor/cJSON.h src/vendor/cJSON.c

LEVEL_SOURCES := $(shell find content/tiled -type f -name "*.tmx")
LEVEL_JSON := $(patsubst content/tiled/%.tmx,src/resources/build/%.json, $(LEVEL_SOURCES))

CONTENT := $(LEVEL_JSON)

DEPS := $(VENDOR) $(EXTERNAL) $(CONTENT)

src/vendor:
	mkdir $@

src/resources/build:
	mkdir $@

.PHONY: all
all:
	$(MAKE) clean
	$(MAKE) desktop
	$(MAKE) web

src/vendor/cJSON.h: vendor/cJSON/cJSON.h | src/vendor
	cp $< src/vendor

src/vendor/cJSON.c: vendor/cJSON/cJSON.c | src/vendor
	cp $< src/vendor

$(LEVEL_JSON): src/resources/build/%.json: content/tiled/%.tmx | src/resources/build
	tiled --embed-tilesets --export-map json $< $@
	prettier -w --use-tabs $@

$(VENDOR):
	git submodule update --init --recursive

.PHONY: desktop
desktop: $(DEPS)
	$(MAKE) -f Makefile.Desktop release

.PHONY: web
web: $(DEPS)
	$(MAKE) -f Makefile.Web release

.PHONY: dev
dev: $(DEPS)
	$(MAKE) -f Makefile.Desktop dev

.PHONY: format
format:
	astyle -n --project=.astylerc --recursive "src/*.c,*.h"
	prettier --write --use-tabs src/minshell.html

.PHONY: clean
clean:
	if [ -d "src/resources/build" ]; then rm -rf src/resources/build; fi
	if [ -d "build" ]; then rm -rf build; fi
	if [ -d "bin" ]; then rm -rf bin; fi
	@echo "Done"
