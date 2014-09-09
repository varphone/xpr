# Builtin module(s)
XPR_BASE64=1
XPR_DEVCAPS=1
XPR_DPU=0
XPR_FIFO=1
XPR_JSON=1
XPR_MD5=1
XPR_PLUGIN=1
XPR_SYNC=1
XPR_SYS=1
XPR_THREAD=1
XPR_UPS=1
XPR_URL=1
XPR_UTILS=1

ifeq ($(XPR_BASE64),1)
libxpr_OBJ += xpr_base64.o
endif

ifeq ($(XPR_DEVCAPS),1)
libxpr_OBJ += xpr_devcaps.o
endif

ifeq ($(XPR_DPU),1)
libxpr_OBJ += xpr_dpu.o
endif

ifeq ($(XPR_FIFO),1)
libxpr_OBJ += xpr_fifo.o
endif

ifeq ($(XPR_JSON),1)
libxpr_OBJ += \
deps/jansson/dump.o \
deps/jansson/error.o \
deps/jansson/hashtable.o \
deps/jansson/hashtable_seed.o \
deps/jansson/load.o \
deps/jansson/memory.o \
deps/jansson/pack_unpack.o \
deps/jansson/strbuffer.o \
deps/jansson/strconv.o \
deps/jansson/utf.o \
deps/jansson/value.o \
xpr_json.o
endif

ifeq ($(XPR_MD5),1)
libxpr_OBJ += xpr_md5.o
endif

ifeq ($(XPR_PLUGIN),1)
libxpr_OBJ += xpr_plugin.o
endif

ifeq ($(XPR_SYNC),1)
libxpr_OBJ += xpr_sync.o xpr_sync_unix.o
endif

ifeq ($(XPR_SYS),1)
libxpr_OBJ += xpr_sys.o
endif

ifeq ($(XPR_THREAD),1)
libxpr_OBJ += xpr_thread.o xpr_thread_unix.o
endif

ifeq ($(XPR_UPS),1)
libxpr_OBJ += \
drivers/ups/root.o \
drivers/ups/sys_net.o \
xpr_ups.o
endif

ifeq ($(XPR_URL),1)
libxpr_OBJ += xpr_url.o
endif

ifeq ($(XPR_UTILS),1)
libxpr_OBJ += xpr_utils.o
endif

