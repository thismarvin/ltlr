SHELL = /bin/sh

override self := $(firstword $(MAKEFILE_LIST))

.DEFAULT_GOAL := $(self)
.EXTRA_PREREQS := $(MAKEFILE_LIST)

$(self):

-include $(objects.prerequisites)

$(objects.src): $(OUTDIR)/%.o: %.c
	$(CC) $(cflags.src) -o $@ -c $<

$(objects.vendor): $(OUTDIR)/%.o: %.c
	$(CC) $(cflags.vendor) -o $@ -c $<
