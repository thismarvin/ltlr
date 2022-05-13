NU := nu
ASTYLE := astyle
PRETTIER := prettier
GPROF := gprof

$(VERBOSE).SILENT:

.PHONY: @all
@all: @clean @desktop @web

Makefile.Content: content
	$(NU) -c "use scripts/generate.nu; generate makefile content | save Makefile.Content"

Makefile.Desktop: src
	$(NU) -c "use scripts/generate.nu; generate makefile desktop | save Makefile.Desktop"

Makefile.Web: src
	$(NU) -c "use scripts/generate.nu; generate makefile web | save Makefile.Web"

.PHONY: @vendor
@vendor: Makefile.Vendor
	$(MAKE) -f Makefile.Vendor @vendor

.PHONY: @content
@content: Makefile.Content
	$(MAKE) -f Makefile.Content @content

.PHONY: @desktop
@desktop: @vendor @content Makefile.Desktop
	$(MAKE) -f Makefile.Desktop @desktop

	if [ -d "build/desktop/content" ]; then rm -r build/desktop/content; fi
	mkdir build/desktop/content
	cp -R build/content/. build/desktop/content

.PHONY: @web
@web: @vendor @content Makefile.Web
	$(MAKE) -f Makefile.Web @web

.PHONY: @dev
@dev: @desktop
	cd build/desktop; ./ltlr
	$(GPROF) build/desktop/ltlr build/desktop/gmon.out > build/desktop/profile

.PHONY: @format
@format:
	$(ASTYLE) -n --project=.astylerc --recursive "src/*.c,*.h"
	$(PRETTIER) --write --use-tabs src/minshell.html

.PHONY: @clean
@clean:
	if [ -f "Makefile.Content" ]; then rm Makefile.Content; fi
	if [ -f "Makefile.Desktop" ]; then rm Makefile.Desktop; fi
	if [ -f "Makefile.Web" ]; then rm Makefile.Web; fi
	if [ -d "build" ]; then rm -r build; fi
