include Common.mk

BUILD_ALL=1
BUILD_SRC=1
BUILD_EXAMPLES=0
BUILD_TESTS=0
BUILD_DOCS=0

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

install:
	mkdir -p $(DESTDIR)$(PREFIX)/include/xpr
	install -m 644 include/xpr/*.* $(DESTDIR)$(PREFIX)/include/xpr/
	install -m 755 lib/libxpr.so $(DESTDIR)$(PREFIX)/lib/libxpr.so

install-bin:
	install -m 755 lib/libxpr.so $(DESTDIR)$(PREFIX)/lib/libxpr.so
