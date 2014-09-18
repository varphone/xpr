CC          = $(CROSS_COMPILER_PREFIX)gcc
AR			= $(CROSS_COMPILER_PREFIX)ar
AS			= $(CROSS_COMPILER_PREFIX)as

CFLAGS      = -O2 -g -Wall -Werror -D_BSD_SOURCE -D_GNU_SOURCE -DPIC -fPIC -I../include
libxpr_DEFS     =
libxpr_OBJS     =
libxpr_LDFLAGS  =

include Modules.mk

CFLAGS      += -DHAVE_STDINT_H=1 $(libxpr_DEFS)
CFLAGS      += -Wno-unused-function -Wno-unused-variable -Wno-unused-result -Wno-unused-but-set-variable

all: libxpr.a libxpr.so local_install

libxpr.a: $(libxpr_OBJS)
	@echo "[  Archive] $@"
	@$(AR) crv $@ $^

libxpr.so: $(libxpr_OBJS)
	@echo "[     Link] $@"
	@$(CC) -shared -rdynamic $^ -o $@ $(libxpr_LDFLAGS) -lpthread -ldl -Wl,--version-script=libxpr.sym

%.o: %.c
	@echo "[Compiling] $^"
	@$(CC) $(CFLAGS) -c $^ -o $@

local_install: libxpr.a libxpr.so
	@echo "[  Install] Copy $^ to ../lib/"
	@cp $^ ../lib/

clean:
	-rm $(libxpr_OBJS) libxpr.a libxpr.so
