SHELL = /bin/sh

override self := $(firstword $(MAKEFILE_LIST))

.DEFAULT_GOAL := @all
.EXTRA_PREREQS := $(MAKEFILE_LIST)

CC ?= gcc
ZIG ?= zig

OUTDIR ?= build/desktop
BIN ?= ltlr

INSTALL ?= install

INSTALL_PROGRAM := $(INSTALL)
INSTALL_DATA := $(INSTALL) -m 644

prefix ?= /usr/local

exec_prefix := $(prefix)
bindir := $(exec_prefix)/bin
datarootdir := $(prefix)/share
datadir := $(datarootdir)

include Common.mk

sources.content != find content -type f
sources.directories := $(sort $(dir $(sources.src) $(sources.vendor.raylib)))

output := $(OUTDIR)/ltlr

cflags.debug = -ggdb -pg -Og
cflags.release = -O2 -DNDEBUG -DDATADIR=\"$(DESTDIR)$(datadir)/$(BIN)/\"

CFLAGS ?= $(cflags.$(build))

private cflags.src.warnings := -Wall -Wextra -Wpedantic
private cflags.src.defines := -DPLATFORM_DESKTOP

cflags.src := -std=gnu17 $(cflags.src.warnings) $(cflags.src.defines) $(cflags.src.vendor) -MMD -g $(CFLAGS)

private cflags.vendor.raylib.defines := -D_GNU_SOURCE -DPLATFORM_DESKTOP -DGRAPHICS_API_OPENGL_33

cflags.vendor.raylib := -std=gnu99 $(cflags.vendor.raylib.defines) -MMD -g $(CFLAGS)

pkgs := glfw3

LDFLAGS ?= $(shell pkg-config --libs-only-L $(pkgs))
LDLIBS ?= -lm -ldl -lrt $(shell pkg-config --libs-only-l $(pkgs))

include Build.mk

.PHONY: @all
@all: @build/release

.PHONY: @install
@install: @all
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) $(output) $(DESTDIR)$(bindir)
	$(INSTALL) -d $(DESTDIR)$(datadir)/$(BIN)/content
	for entry in $(sources.content); do $(INSTALL_DATA) $$entry $(DESTDIR)$(datadir)/$(BIN)/content; done

.PHONY: @uninstall
@uninstall:
	$(RM) $(DESTDIR)$(bindir)/$(BIN)
	$(RM) -r $(DESTDIR)$(datadir)/$(BIN)

$(objects.directories):
	mkdir -p $@

$(output): $(objects)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

.PHONY: @build/debug
@build/debug: $(objects.directories)
	@$(MAKE) -f $(self) $(output) build=debug

.PHONY: @build/release
@build/release: $(objects.directories)
	@$(MAKE) -f $(self) $(output) build=release

.PHONY: @zig/build
@zig/build:
	$(ZIG) build

.PHONY: @zig/run
@zig/run:
	$(ZIG) build run

zig-out/bin/$(BIN).exe:
	$(ZIG) build -Dtarget=x86_64-windows-gnu --release=fast

.PHONY: @zig/build/windows
@zig/build/windows: zig-out/bin/$(BIN).exe

compile_commands.json: scripts/generate.nu $(sources.directories)
	nu -c "use $<; generate compilation database --out-dir $(OUTDIR) | save -f $@"

.PHONY: @dev
@dev: compile_commands.json @build/debug
	./$(OUTDIR)/$(BIN)
	gprof $(output) gmon.out > profile

.PHONY: @clean/prerequisites
@clean/prerequisites:
	-@$(RM) $(objects.prerequisites)

.PHONY: @clean
@clean:
	-@$(RM) $(objects)
	-@$(RM) $(objects.prerequisites)
	-@$(RM) $(output)
