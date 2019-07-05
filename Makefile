# Export the top working directory
export TOP_DIR ?= $(shell pwd)

# Export install settings
export DESTDIR ?= $(TOP_DIR)/out
export INSTALL_PREFIX ?=

# Export target board specs
export TARGET_BOARD ?= unknown

include Common.mk
-include Config.mk

subdirs =

ifeq ($(BUILD_SRC),y)
subdirs += src
endif

ifeq ($(BUILD_BENCHMARKS),y)
subdirs += benchmarks
endif

ifeq ($(BUILD_EXAMPLES),y)
subdirs += examples
endif

ifeq ($(BUILD_TESTS),y)
subdirs += tests
endif

ifeq ($(BUILD_DOCS),y)
subdirs += docs
endif

all: $(subdirs)
	$(call make_subdir, $^)

clean: $(subdirs)
	$(call make_subdir, $^, clean)

distclean: $(subdirs)
	$(call make_subdir, $^, clean)
	-rm lib/*

install: $(subdirs)
	$(call make_subdir, $^, install)
