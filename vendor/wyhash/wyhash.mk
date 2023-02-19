GIT ?= git

TAG := wyhash_final4

.PHONY: @all
@all: wyhash.h

wyhash.h:
	$(GIT) clone --depth 1 --branch $(TAG) https://github.com/wangyi-fudan/wyhash
	mv wyhash/LICENSE .
	mv wyhash/wyhash.h .
	$(RM) -r wyhash

.PHONY: @clean
@clean:
	if [ -f "LICENSE" ]; then $(RM) LICENSE; fi
	if [ -f "wyhash.h" ]; then $(RM) wyhash.h; fi
