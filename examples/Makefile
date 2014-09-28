include ../Common.mk

CFLAGS  = -O2 -g -Wall -I../include
LDFLAGS = -L../lib -lxpr
ifneq ($(TOOLSET_NAME),)
CFLAGS  += -I../contrib/$(TOOLSET_NAME)/include
LDFLAGS += -L../contrib/$(TOOLSET_NAME)/lib
endif

subdirs = plugins
targets = plugin_framework

all: $(targets) subs

plugin_framework: plugin_framework.c
	$(CC) $(CFLAGS) -fPIC -rdynamic $^ $(LDFLAGS) -lxpr -o $@ -Wl,-rpath,\$$ORIGIN -Wl,-rpath,\$$ORIGIN/lib -Wl,-rpath,\$$ORIGIN/plugins

subs: $(subdirs)
	$(call make_subdir, $^)

clean:
	$(call make_subdir, $(subdirs), clean)
	-rm $(targets)

