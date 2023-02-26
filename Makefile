SHELL = /bin/sh

override self := $(firstword $(MAKEFILE_LIST))

.DEFAULT_GOAL := all
.EXTRA_PREREQS := $(MAKEFILE_LIST)

nix.run.nixgl := nix run --override-input nixpkgs flake:nixpkgs github:guibou/nixGL

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

.PHONY: @dev/nixgl/intel
@dev/nixgl/intel:
	$(nix.run.nixgl)\#nixGLIntel -- $(MAKE) -f Desktop.mk @dev

.PHONY: @dev/nixgl/nvidia
@dev/nixgl/nvidia:
	$(nix.run.nixgl)\#nixGLNvidia -- $(MAKE) -f Desktop.mk @dev

.PHONY: @dev/nixgl/nvidia/bumblebee
@dev/nixgl/nvidia/bumblebee:
	$(nix.run.nixgl)\#nixGLNvidiaBumblebee -- $(MAKE) -f Desktop.mk @dev

.PHONY: @test
@test:
	$(MAKE) -f Test.mk @test

.PHONY: @format
@format:
	nu -c "use scripts/ci.nu; ci format"

.PHONY: @lint
@lint:
	-nu -c "use scripts/ci.nu; ci lint"
