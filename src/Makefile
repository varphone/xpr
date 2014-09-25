include ../Common.mk

CFLAGS          = -O2 -g -Wall -Werror -D_BSD_SOURCE -D_GNU_SOURCE -DPIC -fPIC -I../include
CXXFLAGS        =
libxpr_DEFS     =
libxpr_OBJS     =
libxpr_CFLAGS   =
libxpr_CXXFLAGS =
libxpr_LDFLAGS  = -shared -rdynamic -Wl,--version-script=libxpr.sym
libxpr_LIBS     =

include Modules.mk

CFLAGS      += -DHAVE_STDINT_H=1 $(libxpr_DEFS)
CFLAGS      += -Wno-unused-function -Wno-unused-variable -Wno-unused-result -Wno-unused-but-set-variable
CFLAGS      += $(libxpr_CFLAGS)
CXXFLAGS    += $(CFLAGS) $(libxpr_CXXFLAGS)
libxpr_LIBS += -lstdc++ -lpthread -ldl

all: prepare libxpr.a libxpr.so local_install

prepare:

libxpr.a: $(libxpr_OBJS)
	$(ECHO) "[  Archive] $@"
	$(AR) crv $@ $^

libxpr.so: $(libxpr_OBJS)
	$(ECHO) "[     Link] $@"
	$(LD) $(libxpr_LDFLAGS) $^ -o $@ $(libxpr_LIBS)

%.o: %.cpp
	$(ECHO) "[Compiling] $^"
	$(CXX) $(CXXFLAGS) $($(call filter_spc,$^)_CXXFLAGS) -c $^ -o $@

%.o: %.c
	$(ECHO) "[Compiling] $^"
	$(CC) $(CFLAGS) $($(call filter_spc,$^)_CFLAGS) -c $^ -o $@

local_install: libxpr.a libxpr.so
	$(ECHO) "[  Install] Copy $^ to ../lib/"
	$(CP) $^ ../lib/

clean:
	$(RM) $(libxpr_OBJS) libxpr.a libxpr.so

