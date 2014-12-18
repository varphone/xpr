include ../../Common.mk
-include ../../Config.mk

CFLAGS   = $(BUILD_OPT_FLAGS) -I../../include
CXXFLAGS = $(CFLAGS)
LDFLAGS  = -L../../lib -shared -rdynamic

targets = plugin_a.p
objects = plugin_a.o

all: $(targets)

%.o: %.c
	$(ECHO) "[Compiling] $^"
	$(CC) $(CFLAGS) -c $^

%.o: %.cpp
	$(ECHO) "[Compiling] $^"
	$(CXX) $(CXXFLAGS) -c $^

plugin_a.p: plugin_a.o
	$(ECHO) "[ Building] $@"
	$(CC) $(LDFLAGS) $^ -o $@

clean:
	$(RM) $(targets) $(objects)

distclean: clean

.PHONY: clean distclean install
