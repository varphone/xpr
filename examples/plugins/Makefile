TOP_DIR      ?= $(shell realpath ../../)
DESTDIR      ?= $(TOP_DIR)/out
TARGET_BOARD ?= unknown

include ../../Common.mk
-include ../../Config.mk
-include ../../ConfigToolchain.mk

LDFLAGS  += -shared -rdynamic

targets = \
	plugin_a.p \
	plugin_b.p \
	plugin_c.p

objects = \
	plugin_a.o \
	plugin_b.o \
	plugin_c.o

all: $(targets)

%.o: %.c
	$(ECHO) "[Compiling] $^"
	$(CC) $(CFLAGS) -fPIC -c $^

%.o: %.cpp
	$(ECHO) "[Compiling] $^"
	$(CXX) $(CXXFLAGS) -c $^

%.p: %.o
	$(ECHO) "[ Building] $@"
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	$(RM) $(targets) $(objects)

distclean: clean

.PHONY: clean distclean install
