$(VERBOSE).SILENT:

.PHONY: all
all: clean desktop web

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
	if [ -d "build" ]; then rm -rf build; fi
	if [ -d "bin" ]; then rm -rf bin; fi
	@echo "Done"
