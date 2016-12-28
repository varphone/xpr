include ../Common.mk
-include ../Config.mk
-include ../src/Modules.mk
-include ../src/Modules.local

CFLAGS  = $(BUILD_OPT_FLAGS) -I../include
LDFLAGS = -L../lib
ifneq ($(TOOLSET_NAME),)
CFLAGS  += -I../contrib/$(TOOLSET_NAME)/include
LDFLAGS += -L../contrib/$(TOOLSET_NAME)/lib
endif

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

.PONY: clean distclean install subs
