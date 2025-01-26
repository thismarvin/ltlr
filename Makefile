SHELL = /bin/sh

override self := $(firstword $(MAKEFILE_LIST))

.DEFAULT_GOAL := all
.EXTRA_PREREQS := $(MAKEFILE_LIST)

.PHONY: all
all: @os/linux

.PHONY: install
install:
	$(MAKE) -f Desktop.mk @install

.PHONY: uninstall
uninstall:
	$(MAKE) -f Desktop.mk @uninstall

.PHONY: @os/linux
@os/linux:
	$(MAKE) -f Desktop.mk @build/release

.PHONY: @os/emscripten
@os/emscripten:
	$(MAKE) -f Web.mk @build/release

.PHONY: @zig/build
@zig/build:
	$(MAKE) -f Desktop.mk @zig/build

.PHONY: @zig/run
@zig/run:
	$(MAKE) -f Desktop.mk @zig/run

.PHONY: @dev
@dev:
	$(MAKE) -f Desktop.mk @dev

.PHONY: @test
@test:
	$(MAKE) -f Test.mk @test

.PHONY: @format
@format:
	nu -c "use scripts/ci.nu; ci format"

.PHONY: @lint
@lint:
	nu -c "use scripts/ci.nu; ci lint"
