include ../Common.mk

BUILD_DOCS_ALL=1
BUILD_DOCS_DEVELOP=$(BUILD_DOCS_ALL)
BUILD_DOCS_MAN=$(BUILD_DOCS_ALL)

subdirs = 

ifeq ($(BUILD_DOCS_DEVELOP),1)
subdirs += develop
endif

ifeq ($(BUILD_DOCS_MAN),1)
subdirs += man
endif

all: $(subdirs)
	$(call make_subdir, $^)

clean: $(subdirs)
	$(call make_subdir, $^, clean)

distclean: $(subdirs)
	$(call make_subdir, $^, clean)

