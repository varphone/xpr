CC          = $(CROSS_COMPILER_PREFIX)gcc
AR			= $(CROSS_COMPILER_PREFIX)ar
AS			= $(CROSS_COMPILER_PREFIX)as

CFLAGS      = -O2 -g -Wall -D_BSD_SOURCE -D_GNU_SOURCE -DPIC -fPIC -I../include
libxpr_OBJ  =

include Modules.mk

CFLAGS      += -DHAVE_STDINT_H=1

all: libxpr.a libxpr.so local_install

libxpr.a: $(libxpr_OBJ)
	@echo "[  Archive] $@"
	@$(AR) crv $@ $^

libxpr.so: $(libxpr_OBJ)
	@echo "[     Link] $@"
	@$(CC) -shared -rdynamic $^ -o $@ -lpthread -ldl -Wl,--version-script=libxpr.sym

%.o: %.c
	@echo "[Compiling] $^"
	@$(CC) $(CFLAGS) -c $^ -o $@

local_install: libxpr.a libxpr.so
	@echo "[  Install] Copy $^ to ../lib/"
	@cp $^ ../lib/

clean:
	-rm $(libxpr_OBJ) libxpr.a libxpr.so
