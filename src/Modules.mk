# Builtin module(s) configs
################################################################################
# Syntax: XPR_{MODULE}=[0/1], 0 = enabled, 1 = disabled
################################################################################
XPR_ALL=1
XPR_ADC=$(XPR_ALL)
XPR_AVFRAME=$(XPR_ALL)
XPR_BASE64=$(XPR_ALL)
XPR_BITVECTOR=$(XPR_ALL)
XPR_DEVCAPS=$(XPR_ALL)
XPR_DPU=$(XPR_ALL)
XPR_DRM=$(XPR_ALL)
XPR_FIFO=$(XPR_ALL)
XPR_FILE=$(XPR_ALL)
XPR_H264=$(XPR_ALL)
XPR_JSON=$(XPR_ALL)
XPR_MD5=$(XPR_ALL)
XPR_MEM=$(XPR_ALL)
XPR_PES=$(XPR_ALL)
XPR_PLUGIN=$(XPR_ALL)
XPR_STREAMBLOCK=$(XPR_ALL)
XPR_SYNC=$(XPR_ALL)
XPR_SYS=$(XPR_ALL)
XPR_THREAD=$(XPR_ALL)
XPR_UPS=$(XPR_ALL)
XPR_URL=$(XPR_ALL)
XPR_UTILS=$(XPR_ALL)
XPR_XML=$(XPR_ALL)

# Analog Data Channel
################################################################################
XPR_ADC_DRIVER_ALL=1
XPR_ADC_DRIVER_A5S=0

ifeq ($(XPR_ADC),1)
ifeq ($(XPR_ADC_DRIVER_A5S),1)
#libxpr_CFLAGS += -I${INSTALL_PREFIX}/usr/include/ambarella/a5s
#libxpr_LDFLAGS += -I${INSTALL_PREFIX}/usr/lib/ambarella/a5s
libxpr_DEFS += -DXPR_ADC_DRIVER_A5S=1
libxpr_SRCS += drivers/adc/a5s.c
endif
libxpr_SRCS += xpr_adc.c
endif

# Audio and Video frame
################################################################################
ifeq ($(XPR_AVFRAME),1)
libxpr_SRCS += xpr_avframe.c
endif

# Base64
################################################################################
ifeq ($(XPR_BASE64),1)
libxpr_SRCS += xpr_base64.c
endif

# BitVector
################################################################################
ifeq ($(XPR_BITVECTOR),1)
libxpr_SRCS += xpr_bitvector.c
endif

# Device Capability
################################################################################
ifeq ($(XPR_DEVCAPS),1)
libxpr_SRCS += xpr_devcaps.c
endif

# Data Process Unit
################################################################################
dpu-defs-  :=
dpu-objs-  :=
dpu-defs-0 :=
dpu-srcs-0 :=
dpu-defs-1 :=
dpu-srcs-1 :=
dpu-cflags :=
dpu-ldfalgs:=
dpu-libs   :=

# add_dpu(Id,Switch?[0|1],Objects,CFLAGS,LDFLAGS,LIBS)
define add_dpu
$(eval XPR_DPU_DRIVER_$1 := $2)
$(eval dpu-defs-$(XPR_DPU_DRIVER_$1) += -DXPR_DPU_DRIVER_$1=$(XPR_DPU_DRIVER_$1))
$(eval dpu-srcs-$(XPR_DPU_DRIVER_$1) += $(patsubst %,drivers/dpu/%,$3))
$(eval dpu-cflags += $4)
$(eval dpu-ldflags += $5)
$(eval dpu-libs += $6)
endef

ifeq ($(XPR_DPU),1)
$(eval $(call add_dpu,A2SVIDEO,0,a2svideo.c))
$(eval $(call add_dpu,A5SVIDEO,0,a5svideo.c))
$(eval $(call add_dpu,A5SYUV,0,a5syuv.c))
$(eval $(call add_dpu,ALSAPCM,1,alsapcm.c,,,-lasound))
$(eval $(call add_dpu,G711TEST,1,g711test.c))
$(eval $(call add_dpu,MDSD,1,mdsd.c))
$(eval $(call add_dpu,PCMTEST,1,pcmtest.c))
libxpr_CFLAGS += $(dpu-cflags)
libxpr_LDFLAGS += $(dpu-ldflags)
libxpr_LIBS += $(dpu-libs)
libxpr_DEFS += $(dpu-defs-0) $(dpu-defs-1)
libxpr_SRCS += $(dpu-srcs-1) \
xpr_dpu.c \
xpr_dpu_options.c
endif

# Digital Rights Management
################################################################################
XPR_DRM_DRIVER_ALL=0
XPR_DRM_DRIVER_ALPUC_016=$(XPR_DRM_DRIVER_ALL)
XPR_DRM_DRIVER_DS18B20=$(XPR_DRM_DRIVER_ALL)
XPR_DRM_DRIVER_RECONBALL=$(XPR_DRM_DRIVER_ALL)

ifeq ($(XPR_DRM),1)
ifeq ($(XPR_DRM_DRIVER_ALPUC_016),1)
#libxpr_CFLAGS += 
#libxpr_LDFLAGS += 
libxpr_DEFS += -DXPR_DRM_DRIVER_ALPUC_016=1
libxpr_SRCS += drivers/drm/alpuc-016.c
endif
ifeq ($(XPR_DRM_DRIVER_DS18B20),1)
#libxpr_CFLAGS += 
#libxpr_LDFLAGS += 
libxpr_DEFS += -DXPR_DRM_DRIVER_DS18B20=1
libxpr_SRCS += drivers/drm/ds18b20.c
endif
ifeq ($(XPR_DRM_DRIVER_RECONBALL),1)
#libxpr_CFLAGS += 
#libxpr_LDFLAGS += 
libxpr_DEFS += -DXPR_DRM_DRIVER_RECONBALL=1
libxpr_SRCS += drivers/drm/reconball.c
endif
libxpr_SRCS += xpr_drm.c
endif

# Fifo
################################################################################
ifeq ($(XPR_FIFO),1)
libxpr_SRCS += xpr_fifo.c
endif

# File
################################################################################
ifeq ($(XPR_FILE),1)
libxpr_SRCS += \
xpr_file_unix.c \
xpr_file.c
endif

# H264
################################################################################
ifeq ($(XPR_H264),1)
libxpr_SRCS += xpr_h264.c
endif

# JSON
################################################################################
ifeq ($(XPR_JSON),1)
libxpr_SRCS += \
deps/jansson/dump.c \
deps/jansson/error.c \
deps/jansson/hashtable.c \
deps/jansson/hashtable_seed.c \
deps/jansson/load.c \
deps/jansson/memory.c \
deps/jansson/pack_unpack.c \
deps/jansson/strbuffer.c \
deps/jansson/strconv.c \
deps/jansson/utf.c \
deps/jansson/value.c \
xpr_json.c
endif

# MD5
################################################################################
ifeq ($(XPR_MD5),1)
libxpr_SRCS += xpr_md5.c
endif

# Memory management
################################################################################
ifeq ($(XPR_MEM),1)
libxpr_SRCS += xpr_mem.c
endif

# Packetized Elementary Stream
################################################################################
ifeq ($(XPR_PES),1)
libxpr_SRCS += xpr_pes.c
endif

# Plugin framework
################################################################################
ifeq ($(XPR_PLUGIN),1)
libxpr_SRCS += xpr_plugin.c
endif

# Stream Block
################################################################################
ifeq ($(XPR_STREAMBLOCK),1)
libxpr_SRCS += xpr_streamblock.c
endif

# Sync
################################################################################
ifeq ($(XPR_SYNC),1)
libxpr_OBJS += xpr_sync.c xpr_sync_unix.c
endif

# System relates
################################################################################
ifeq ($(XPR_SYS),1)
libxpr_SRCS += xpr_sys.c
endif

# Thread
################################################################################
ifeq ($(XPR_THREAD),1)
libxpr_SRCS += xpr_thread.c xpr_thread_unix.c
endif

# Universal Preference Settings framework
################################################################################
ifeq ($(XPR_UPS),1)
libxpr_SRCS += \
drivers/ups/root.c \
drivers/ups/cam_img.c \
drivers/ups/sys_info.c \
drivers/ups/sys_net.c \
xpr_ups.c
endif

# URL
################################################################################
ifeq ($(XPR_URL),1)
libxpr_SRCS += xpr_url.c
endif

# Utils
################################################################################
ifeq ($(XPR_UTILS),1)
libxpr_SRCS += xpr_utils.c
endif

# XML
################################################################################
ifeq ($(XPR_XML),1)
libxpr_SRCS += \
deps/roxml/roxml.c \
deps/roxml/roxml-internal.c \
deps/roxml/roxml-parse-engine.c \
xpr_xml.c
endif

# Objects from sources
################################################################################
libxpr_OBJS := $(patsubst %.c,%.o,$(libxpr_SRCS))
libxpr_OBJS := $(patsubst %.cpp,%.o,$(libxpr_OBJS))
libxpr_OBJS := $(patsubst %.cxx,%.o,$(libxpr_OBJS))

# add_cxxflags() Examples
# $(eval $(call add_cflags,xpr_test.c,-DHELLO_ADD_CFLAGS=1))
# $(eval $(call add_cxxflags,xpr_test.cpp,-DHELLO_ADD_CXXFLAGS=1))

