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

.PHONY: @vendor/desktop
@vendor/desktop: Makefile.Vendor
	$(MAKE) -f $< @vendor/desktop

.PHONY: @vendor/web
@vendor/web: Makefile.Vendor
	$(MAKE) -f $< @vendor/web

.PHONY: @vendor
@vendor: @vendor/desktop @vendor/web

.PHONY: @content
@content: Makefile.Content
	$(MAKE) -f $< @content

.PHONY: @desktop
@desktop: @vendor/desktop @content Makefile.Desktop
	$(MAKE) -f Makefile.Desktop @desktop

	if [ -d "build/desktop/content" ]; then rm -r build/desktop/content; fi
	mkdir build/desktop/content
	cp -R build/content/. build/desktop/content

.PHONY: @web
@web: @vendor/web @content Makefile.Web
	$(MAKE) -f Makefile.Web @web

.PHONY: @dev
@dev: @desktop
	cd build/desktop; ./ltlr
	$(GPROF) build/desktop/ltlr build/desktop/gmon.out > build/desktop/profile

.PHONY: @test
@test: @vendor/desktop Makefile.Test
	$(MAKE) -f Makefile.Test @test

.PHONY: @format
@format:
	$(ASTYLE) -n --project=.astylerc --recursive "src/*.c,*.h"
	$(ASTYLE) -n --project=.astylerc --recursive "tests/*.c,*.h"
	$(PRETTIER) --write --use-tabs src/minshell.html

.PHONY: @clean
@clean:
	if [ -f "Makefile.Content" ]; then rm Makefile.Content; fi
	if [ -f "Makefile.Desktop" ]; then rm Makefile.Desktop; fi
	if [ -f "Makefile.Web" ]; then rm Makefile.Web; fi
	if [ -d "build" ]; then rm -r build; fi
