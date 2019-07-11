TOP_DIR      ?= $(shell realpath ../)
DESTDIR      ?= $(TOP_DIR)/out
TARGET_BOARD ?= unknown

include ../Common.mk
-include ../Config.mk
-include ../ConfigToolchain.mk
-include ../src/Modules.mk
-include ../src/Modules.local

LDFLAGS += -Wl,-rpath,\$$ORIGIN -Wl,-rpath,\$$ORIGIN/lib -Wl,-rpath,\$$ORIGIN/plugins

ifeq ($(BUILD_STATIC),y)
LDFLAGS += -Wl,-Bstatic -lxpr -Wl,-Bdynamic -lpthread -ldl 
endif

ifeq ($(BUILD_SHARED),y)
LDFLAGS += -Wl,-Bdynamic -lxpr -Wl,-Bdynamic
endif

targets   :=
targets-y :=
targets-$(XPR_FIFO) += fifo
targets-$(XPR_FILE) += file
targets-$(XPR_FQ) += fq
targets-$(XPR_JSON) += json
targets-$(XPR_REGEX) += regex
targets-$(XPR_RTSP) += rtsp
targets-$(XPR_THREAD) += thread
targets-$(XPR_UPS) += ups ups-basic
targets-$(XPR_XML) += utils
targets-$(XPR_XML) += xml
targets += $(targets-y)
targets += qsort realpath struct

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

install:
	@echo "Nothing to do!"

test: $(targets)
	$(call run_test, $^)

.PHONY: clean distclean install test
