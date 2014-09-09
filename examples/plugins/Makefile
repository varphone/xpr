CC      = $(CROSS_COMPILER_PREFIX)gcc
CFLAGS  = -O2 -g -Wall -I../../include
LDFLAGS = -L../../lib

targets = plugin_a.p

all: $(targets)

%.p: %.c
	@echo "[ Building] $@"
	@$(CC) $(CFLAGS) -shared -rdynamic $^ -o $@

clean:
	-rm $(targets)

