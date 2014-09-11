include ../Common.mk

CC      = $(CROSS_COMPILER_PREFIX)gcc
CFLAGS  = -O2 -g -Wall -I../include
LDFLAGS = -L../lib -lxpr -Wl,-rpath,\$$ORIGIN -Wl,-rpath,\$$ORIGIN/plugins

targets = qsort realpath struct ups json
objects = $(targets:%=%.o)

all: $(targets)

$(targets): $(objects)
	@echo "[ Building] $@"
	@$(CC) $@.o -o $@ $(LDFLAGS)

%.o: %.c
	@echo "[Compiling] $^"
	@$(CC) $(CFLAGS) -c $^

clean:
	-rm $(targets) $(objects)

test: $(targets)
	$(call run_test, $^)

