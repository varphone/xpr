TOP_DIR      ?= $(shell realpath ../../)
DESTDIR      ?= $(TOP_DIR)/out
TARGET_BOARD ?= unknown

include ../../Common.mk
-include ../../Config.mk
-include ../../ConfigToolchain.mk

LDFLAGS  += -shared -rdynamic

targets = \
	libplugin_a.so \
	libplugin_b.so \
	libplugin_c.so \
	libplugin_d.so \
	libplugin_e.so \
	libplugin_f.so \
	libplugin_g.so

objects = \
	plugin_a.o \
	plugin_b.o \
	plugin_c.o \
	plugin_d.o \
	plugin_e.o \
	plugin_f.o \
	plugin_g.o

all: $(targets)

%.o: %.c
	$(ECHO) "[Compiling] $^"
	$(CC) $(CFLAGS) -fPIC -c $^

%.o: %.cpp
	$(ECHO) "[Compiling] $^"
	$(CXX) $(CXXFLAGS) -c $^

lib%.so: %.o
	$(ECHO) "[ Building] $@"
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	$(RM) $(targets) $(objects)

distclean: clean

.PHONY: clean distclean install
