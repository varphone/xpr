include Common.mk
-include Config.mk

subdirs =

ifeq ($(BUILD_SRC),1)
subdirs += src
endif

ifeq ($(BUILD_TESTS),1)
subdirs += tests
endif

ifeq ($(BUILD_APPS),1)
subdirs += apps
endif

ifeq ($(BUILD_LIBMAL),1)
subdirs += libmal
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
