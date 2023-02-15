CC ?= gcc
GPROF ?= gprof

CFLAGS := -std=c17 -Wall -Wextra -Wpedantic -g -pg -Og -DPLATFORM_DESKTOP
LDLIBS := -lm

DEPS := \
	src/collections/deque.c \
	src/utils/quadtree.c \
	tests/testing.c \

$(VERBOSE).SILENT:

.PHONY: @all
@all: build/tests/unit_tests

build:
	mkdir $@

build/tests: | build
	mkdir $@

build/tests/unit_tests: tests/unit_tests.c $(DEPS) | build/tests
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: @test
@test: build/tests/unit_tests | build/tests
	cd build/tests; ./unit_tests
	$(GPROF) build/tests/unit_tests build/tests/gmon.out > build/tests/profile

.PHONY: @clean
@clean:
	if [ -d "build/tests" ]; then $(RM) -r build/tests; fi
