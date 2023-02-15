SHELL = /bin/sh

override self := $(firstword $(MAKEFILE_LIST))

.DEFAULT_GOAL := @all
.EXTRA_PREREQS := $(MAKEFILE_LIST)

CC := emcc

OUTDIR ?= build/web

include Common.mk

output := \
	$(OUTDIR)/index.html \
	$(OUTDIR)/index.js \
	$(OUTDIR)/index.wasm \
	$(OUTDIR)/index.data

cflags.debug := -g -O0
cflags.release = -Os -DNDEBUG

CFLAGS ?= $(cflags.$(build))

private cflags.src.warnings := -Wall -Wextra -Wpedantic
private cflags.src.defines := -DPLATFORM_WEB

cflags.src := -std=c17 $(cflags.src.warnings) $(cflags.src.defines) -Ivendor/raylib/src -MMD $(CFLAGS)

private cflags.vendor.defines := -D_DEFAULT_SOURCE -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2

cflags.vendor := -std=gnu99 $(cflags.vendor.defines) -MMD $(CFLAGS)

LDFLAGS ?= $(CFLAGS)

include Build.mk

.PHONY: @all
@all: @build/release

$(objects.directories):
	mkdir -p $@

$(output) &: $(objects)
	$(CC) $(LDFLAGS) -o $(OUTDIR)/index.html $^ -sUSE_GLFW=3 --shell-file src/minshell.html --preload-file content

.PHONY: @build/debug
@build/debug: $(objects.directories)
	$(MAKE) -f $(self) $(output) build=debug

.PHONY: @build/release
@build/release: $(objects.directories)
	$(MAKE) -f $(self) $(output) build=release

.PHONY: @clean/prerequisites
@clean/prerequisites:
	-@$(RM) $(objects.prerequisites)

.PHONY: @clean
@clean:
	-@$(RM) $(objects)
	-@$(RM) $(objects.prerequisites)
	-@$(RM) $(output)
