TOP_DIR      ?= $(shell realpath ../)
DESTDIR      ?= $(TOP_DIR)/out
TARGET_BOARD ?= unknown

include ../Common.mk
-include ../Config.mk
-include ../ConfigToolchain.mk

ifeq ($(BUILD_STATIC),y)
LDFLAGS += -Wl,-Bstatic -lxpr -Wl,-Bdynamic -lpthread -ldl 
endif

ifeq ($(BUILD_SHARED),y)
LDFLAGS += -Wl,-Bdynamic -lxpr -Wl,-Bdynamic
endif

subdirs = plugins
targets = plugin_framework

all: $(targets) subs

plugin_framework: plugin_framework.c
	$(CC) $(CFLAGS) -rdynamic $^ $(LDFLAGS) -o $@ -Wl,-rpath,\$$ORIGIN -Wl,-rpath,\$$ORIGIN/lib -Wl,-rpath,\$$ORIGIN/plugins

subs: $(subdirs)
	$(call make_subdir, $^)

clean:
	$(call make_subdir, $(subdirs), clean)
	$(RM) $(targets)

distclean: clean

install:
	@echo "Nothing to do!"

.PONY: clean distclean install subs
