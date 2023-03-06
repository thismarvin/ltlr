GIT ?= git

TAG := 4.2.0

.PHONY: @all
@all: src/raylib.h

src/raylib.h:
	$(GIT) clone --depth 1 --branch $(TAG) https://github.com/raysan5/raylib
	mv raylib/.gitignore .
	mv raylib/LICENSE .
	mv raylib/src .
	patch -d src < address-glfw-compile-error.patch
	patch -d src < apply-custom-configuration.patch
	$(RM) -r raylib

.PHONY: @clean
@clean:
	if [ -f ".gitignore" ]; then $(RM) .gitignore; fi
	if [ -f "LICENSE" ]; then $(RM) LICENSE; fi
	if [ -d "src" ]; then $(RM) -r src; fi
