$(VERBOSE).SILENT:

LEVEL_SOURCES := $(shell find content/tiled -type f -name "*.tmx")
LEVEL_JSON := $(patsubst content/tiled/%.tmx,src/resources/build/%.json, $(LEVEL_SOURCES))

CONTENT := $(LEVEL_JSON)

src/resources/build:
	mkdir $@

.PHONY: all
all:
	$(MAKE) clean
	$(MAKE) desktop
	$(MAKE) web

$(LEVEL_JSON): src/resources/build/%.json: content/tiled/%.tmx | src/resources/build
	tiled --export-map json $< $@
	prettier -w --use-tabs $@

.PHONY: desktop
desktop:
	$(MAKE) -f Makefile.Desktop release

.PHONY: web
web:
	$(MAKE) -f Makefile.Web release

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
