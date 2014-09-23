include ../Common.mk

CC      = $(CROSS_COMPILER_PREFIX)gcc
CFLAGS  = -O2 -g -Wall -I../include
LDFLAGS = -L../lib

subdirs = plugins
targets = plugin_framework

all: $(targets) subs

plugin_framework: plugin_framework.c
	@$(CC) $(CFLAGS) -fPIC -rdynamic $^ $(LDFLAGS) -lxpr -o $@ -Wl,-rpath,\$$ORIGIN -Wl,-rpath,\$$ORIGIN/lib -Wl,-rpath,\$$ORIGIN/plugins

subs: $(subdirs)
	$(call make_subdir, $^)

clean:
	$(call make_subdir, $(subdirs), clean)
	-rm $(targets)

