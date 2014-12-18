include ../Common.mk
-include ../Config.mk

CFLAGS  = $(BUILD_OPT_FLAGS) -I../include
CXXFLAGS= $(CFLAGS)
LDFLAGS = -L../lib -Wl,-rpath,\$$ORIGIN -Wl,-rpath,\$$ORIGIN/lib -Wl,-rpath,\$$ORIGIN/plugins

ifeq ($(BUILD_STATIC),1)
LDFLAGS += -Wl,-Bstatic -lxpr -Wl,-Bdynamic -lpthread -ldl 
endif

ifeq ($(BUILD_SHARED),1)
LDFLAGS += -Wl,-Bdynamic -lxpr -Wl,-Bdynamic
endif

targets = fifo json qsort realpath struct thread ups xml
objects = $(targets:%=%.o)

all: $(targets)

$(targets): $(objects)
	$(ECHO) "[ Building] $@"
	$(CC) $@.o -o $@ $(LDFLAGS)

%.o: %.c
	$(ECHO) "[Compiling] $^"
	$(CC) $(CFLAGS) -c $^

%.o: %.cpp
	$(ECHO) "[Compiling] $^"
	$(CXX) $(CXXFLAGS) -c $^

clean:
	$(RM) $(targets) $(objects)

distclean: clean

test: $(targets)
	$(call run_test, $^)

.PHONY: clean distclean install test
