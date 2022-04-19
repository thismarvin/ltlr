$(VERBOSE).SILENT:

ASEPRITE := aseprite
TILED := tiled
ASTYLE := astyle
PRETTIER := prettier

VENDOR := vendor/raylib/src/raylib.h vendor/cJSON/cJSON.h
EXTERNAL := src/vendor/cJSON.h src/vendor/cJSON.c

IMAGE_SOURCES := $(shell find content/aseprite -type f -name "*.aseprite")
IMAGE_PNG := $(patsubst content/aseprite/%.aseprite,src/resources/build/%.png, $(IMAGE_SOURCES))

LEVEL_SOURCES := $(shell find content/tiled -type f -name "*.tmx")
LEVEL_JSON := $(patsubst content/tiled/%.tmx,src/resources/build/%.json, $(LEVEL_SOURCES))

CONTENT := $(IMAGE_PNG) $(LEVEL_JSON)

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

$(IMAGE_PNG): src/resources/build/%.png: content/aseprite/%.aseprite | src/resources/build
	$(ASEPRITE) -b $< --save-as $@

$(LEVEL_JSON): src/resources/build/%.json: content/tiled/%.tmx | src/resources/build
	$(TILED) --embed-tilesets --export-map json $< $@
	$(PRETTIER) -w --use-tabs $@

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
	$(ASTYLE) -n --project=.astylerc --recursive "src/*.c,*.h"
	$(PRETTIER) --write --use-tabs src/minshell.html

.PHONY: clean
clean:
	if [ -d "src/resources/build" ]; then rm -rf src/resources/build; fi
	if [ -d "build" ]; then rm -rf build; fi
	if [ -d "bin" ]; then rm -rf bin; fi
	@echo "Done"

# TODO(thismarvin): We need to automate this somehow!
.PHONY: test
test:
	gcc -Ivendor/raylib/src -o ./bin/debug/desktop/test src/unit_tests.c src/testing.c src/deque.c -Lbuild/vendor/raylib/desktop -DDO_TESTING
	./bin/debug/desktop/test
