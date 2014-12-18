include Common.mk
-include Config.mk

subdirs =

ifeq ($(BUILD_SRC),1)
subdirs += src
endif

ifeq ($(BUILD_EXAMPLES),1)
subdirs += examples
endif

ifeq ($(BUILD_TESTS),1)
subdirs += tests
endif

ifeq ($(BUILD_DOCS),1)
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
