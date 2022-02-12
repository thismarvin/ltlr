$(VERBOSE).SILENT:

.PHONY: all
all: clean desktop web

.PHONY: desktop
desktop:
	make -f Makefile.Desktop release

.PHONY: web
web:
	make -f Makefile.Web release

.PHONY: format
format:
	astyle -n --recursive "src/*.c,*.h"
	prettier --write --use-tabs src/minshell.html

.PHONY: clean
clean:
	if [ -d "deps" ]; then rm -rf deps; fi
	if [ -d "build" ]; then rm -rf build; fi
	if [ -d "bin" ]; then rm -rf bin; fi
	@echo "Done"
