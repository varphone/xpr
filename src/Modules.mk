# Builtin module(s)
XPR_AVFRAME=1
XPR_BASE64=1
XPR_DEVCAPS=1
XPR_DPU=1
XPR_FIFO=1
XPR_JSON=1
XPR_MD5=1
XPR_MEM=1
XPR_PLUGIN=1
XPR_STREAMBLOCK=1
XPR_SYNC=1
XPR_SYS=1
XPR_THREAD=1
XPR_UPS=1
XPR_URL=1
XPR_UTILS=1

# Audio and Video frame
################################################################################
ifeq ($(XPR_AVFRAME),1)
libxpr_OBJS += xpr_avframe.o
endif

# Base64
################################################################################
ifeq ($(XPR_BASE64),1)
libxpr_OBJS += xpr_base64.o
endif

# Device Capability
################################################################################
ifeq ($(XPR_DEVCAPS),1)
libxpr_OBJS += xpr_devcaps.o
endif

# Data Process Unit
################################################################################
dpu-defs-  :=
dpu-objs-  :=
dpu-defs-0 :=
dpu-objs-0 :=
dpu-defs-1 :=
dpu-objs-1 :=
dpu-cflags :=
dpu-ldfalgs:=

# add_dpu(Id,Switch?[0|1],Objects,CFLAGS,LDFLAGS)
define add_dpu
$(eval XPR_DPU_DRIVER_$1 := $2)
$(eval dpu-defs-$(XPR_DPU_DRIVER_$1) += -DXPR_DPU_DRIVER_$1=$(XPR_DPU_DRIVER_$1))
$(eval dpu-objs-$(XPR_DPU_DRIVER_$1) += $(patsubst %,drivers/dpu/%,$3))
$(eval dpu-cflags += $4)
$(eval dpu-ldflags += $5)
endef

ifeq ($(XPR_DPU),1)
$(eval $(call add_dpu,A2SVIDEO,0,a2svideo.o))
$(eval $(call add_dpu,A5SVIDEO,0,a5svideo.o))
$(eval $(call add_dpu,A5SYUV,0,a5syuv.o))
$(eval $(call add_dpu,ALSAPCM,1,alsapcm.o,,-lasound))
$(eval $(call add_dpu,G711TEST,1,g711test.o))
$(eval $(call add_dpu,MDSD,1,mdsd.o))
$(eval $(call add_dpu,PCMTEST,1,pcmtest.o))
libxpr_CFLAGS += $(dpu-cflags)
libxpr_LDFLAGS += $(dpu-ldflags)
libxpr_DEFS += $(dpu-defs-0) $(dpu-defs-1)
libxpr_OBJS += $(dpu-objs-1) \
xpr_dpu.o \
xpr_dpu_options.o
endif

# Fifo
################################################################################
ifeq ($(XPR_FIFO),1)
libxpr_OBJS += xpr_fifo.o
endif

# JSON
################################################################################
ifeq ($(XPR_JSON),1)
libxpr_OBJS += \
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

# MD5
################################################################################
ifeq ($(XPR_MD5),1)
libxpr_OBJS += xpr_md5.o
endif

# Memory management
################################################################################
ifeq ($(XPR_MEM),1)
libxpr_OBJS += xpr_mem.o
endif

# Plugin framework
################################################################################
ifeq ($(XPR_PLUGIN),1)
libxpr_OBJS += xpr_plugin.o
endif

# Stream Block
################################################################################
ifeq ($(XPR_STREAMBLOCK),1)
libxpr_OBJS += xpr_streamblock.o
endif

# Sync
################################################################################
ifeq ($(XPR_SYNC),1)
libxpr_OBJS += xpr_sync.o xpr_sync_unix.o
endif

# System relates
################################################################################
ifeq ($(XPR_SYS),1)
libxpr_OBJS += xpr_sys.o
endif

# Thread
################################################################################
ifeq ($(XPR_THREAD),1)
libxpr_OBJS += xpr_thread.o xpr_thread_unix.o
endif

# Universal Preference Settings framework
################################################################################
ifeq ($(XPR_UPS),1)
libxpr_OBJS += \
drivers/ups/root.o \
drivers/ups/sys_info.o \
drivers/ups/sys_net.o \
xpr_ups.o
endif

# URL
################################################################################
ifeq ($(XPR_URL),1)
libxpr_OBJS += xpr_url.o
endif

# Utils
################################################################################
ifeq ($(XPR_UTILS),1)
libxpr_OBJS += xpr_utils.o
endif

