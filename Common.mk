SHELL = /bin/sh

override self := $(firstword $(MAKEFILE_LIST))

.DEFAULT_GOAL := $(self)
.EXTRA_PREREQS := $(MAKEFILE_LIST)

sources.src != find src -name "*.c"
sources.vendor.raylib := \
	vendor/raylib/src/raudio.c \
	vendor/raylib/src/rcore.c \
	vendor/raylib/src/rshapes.c \
	vendor/raylib/src/rtext.c \
	vendor/raylib/src/rtextures.c \
	vendor/raylib/src/utils.c

cflags.src.vendor := -Ivendor/raylib/src -Ivendor/wyhash

objects.src := $(patsubst %.c,$(OUTDIR)/%.o,$(sources.src))
objects.vendor.raylib := $(patsubst %.c,$(OUTDIR)/%.o,$(sources.vendor.raylib))

objects := $(objects.src) $(objects.vendor.raylib)

objects.directories := $(sort $(dir $(objects)))

objects.prerequisites := $(patsubst %.o,%.d,$(objects))

$(self):
