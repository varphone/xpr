# Builtin module(s) configs
###############################################################################
# Syntax: XPR_{MODULE}=[y/n], y = enabled, n = disabled
###############################################################################
XPR_ALL=y
XPR_ADC=$(XPR_ALL)
XPR_ADC_DRIVER_ALL=n
XPR_ADC_DRIVER_A5S=$(XPR_ADC_DRIVER_ALL)

XPR_ARP=$(XPR_ALL)
XPR_ARR=$(XPR_ALL)
XPR_AVFRAME=$(XPR_ALL)
XPR_BASE64=$(XPR_ALL)
XPR_BITVECTOR=$(XPR_ALL)
XPR_CRC=$(XPR_ALL)
XPR_DEVCAPS=$(XPR_ALL)

XPR_DPU=$(XPR_ALL)
XPR_DPU_DRIVER_ALL=n
XPR_DPU_DRIVER_A2SVIDEO=$(XPR_DPU_DRIVER_ALL)
XPR_DPU_DRIVER_A5SVIDEO=$(XPR_DPU_DRIVER_ALL)
XPR_DPU_DRIVER_A5SYUV=$(XPR_DPU_DRIVER_ALL)
XPR_DPU_DRIVER_ALSAPCM=$(XPR_DPU_DRIVER_ALL)
XPR_DPU_DRIVER_G711TEST=$(XPR_DPU_DRIVER_ALL)
XPR_DPU_DRIVER_MDSD=$(XPR_DPU_DRIVER_ALL)
XPR_DPU_DRIVER_PCMTEST=$(XPR_DPU_DRIVER_ALL)

XPR_DRM=$(XPR_ALL)
XPR_DRM_DRIVER_ALL=n
XPR_DRM_DRIVER_ALPUC_016=$(XPR_DRM_DRIVER_ALL)
XPR_DRM_DRIVER_DS18B20=$(XPR_DRM_DRIVER_ALL)
XPR_DRM_DRIVER_RECONBALL=$(XPR_DRM_DRIVER_ALL)

XPR_FIFO=$(XPR_ALL)
XPR_FILE=$(XPR_ALL)
XPR_FQ=$(XPR_ALL)
XPR_GPIO=$(XPR_ALL)
XPR_GPIO_DRIVER_ALL=n
XPR_GPIO_DRIVER_A5S=$(XPR_GPIO_DRIVER_ALL)

XPR_H264=$(XPR_ALL)
XPR_ICMP=$(XPR_ALL)
XPR_JSON=$(XPR_ALL)
XPR_LIST=$(XPR_ALL)
XPR_MCDEC=$(XPR_ALL)
XPR_MD5=$(XPR_ALL)
XPR_MEM=$(XPR_ALL)
XPR_META=$(XPR_ALL)
XPR_PES=$(XPR_ALL)
XPR_PLUGIN=$(XPR_ALL)
XPR_RTSP=$(XPR_ALL)
XPR_RTSP_DRIVER_ALL=$(XPR_RTSP)
XPR_RTSP_DRIVER_LIVE=$(XPR_RTSP_DRIVER_ALL)
XPR_RTSP_CLIENT=$(XPR_RTSP)
XPR_RTSP_SERVER=$(XPR_RTSP)
XPR_SERIAL=$(XPR_ALL)
XPR_SHA1=$(XPR_ALL)
XPR_STREAMBLOCK=$(XPR_ALL)
XPR_SYNC=$(XPR_ALL)
XPR_SYS=$(XPR_ALL)
XPR_THREAD=$(XPR_ALL)
XPR_UPS=$(XPR_ALL)
XPR_URL=$(XPR_ALL)
XPR_UTILS=$(XPR_ALL)
XPR_XML=$(XPR_ALL)

# Load local modules configs
###############################################################################
-include Modules.local

# Analog Data Channel
###############################################################################
ifeq ($(XPR_ADC),y)
$(eval $(call add_driver,ADC,A5S,$(XPR_ADC_DRIVER_A5S),a5s.c))
libxpr_SRCS += xpr_adc.c
endif

# ARP
###############################################################################
ifeq ($(XPR_ARP),y)
libxpr_SRCS += xpr_arp_unix.c
endif

# Audio Resampler
###############################################################################
ifeq ($(XPR_ARR),y)
libxpr_SRCS += xpr_arr.c
endif

# Audio and Video frame
###############################################################################
ifeq ($(XPR_AVFRAME),y)
libxpr_SRCS += xpr_avframe.c
endif

# Base64
###############################################################################
ifeq ($(XPR_BASE64),y)
libxpr_SRCS += xpr_base64.c
endif

# BitVector
###############################################################################
ifeq ($(XPR_BITVECTOR),y)
libxpr_SRCS += xpr_bitvector.c
endif

# CRC
###############################################################################
ifeq ($(XPR_CRC),y)
libxpr_SRCS += xpr_crc.c
endif

# Device Capability
###############################################################################
ifeq ($(XPR_DEVCAPS),y)
libxpr_SRCS += xpr_devcaps.c
endif

# Data Process Unit
###############################################################################
ifeq ($(XPR_DPU),y)
$(eval $(call add_driver,DPU,A2SVIDEO,$(XPR_DPU_DRIVER_A2SVIDEO),a2svideo.c,,,))
$(eval $(call add_driver,DPU,A5SVIDEO,$(XPR_DPU_DRIVER_A5SVIDEO),a5svideo.c,,,))
$(eval $(call add_driver,DPU,A5SYUV,$(XPR_DPU_DRIVER_A5SYUV),a5syuv.c,,,))
$(eval $(call add_driver,DPU,ALSAPCM,$(XPR_DPU_DRIVER_ALSAPCM),alsapcm.c,,,-lasound))
$(eval $(call add_driver,DPU,G711TEST,$(XPR_DPU_DRIVER_G711TEST),g711test.c,,,))
$(eval $(call add_driver,DPU,MDSD,$(XPR_DPU_DRIVER_MDSD),mdsd.c,,,))
$(eval $(call add_driver,DPU,PCMTEST,$(XPR_DPU_DRIVER_PCMTEST),pcmtest.c,,,))
libxpr_SRCS += xpr_dpu.c xpr_dpu_options.c
endif

# Digital Rights Management
###############################################################################
ifeq ($(XPR_DRM),y)
$(eval $(call add_driver,DRM,ALPUC_016,$(XPR_DRM_DRIVER_ALPUC_016),alpuc-016.c,,,-lalpuc-016))
$(eval $(call add_driver,DRM,DS18B20,$(XPR_DRM_DRIVER_DS18B20),ds18b20.c,,,))
$(eval $(call add_driver,DRM,RECONBALL,$(XPR_DRM_DRIVER_RECONBALL),reconball.c,,,))
libxpr_SRCS += xpr_drm.c
endif

# Fifo
###############################################################################
ifeq ($(XPR_FIFO),y)
libxpr_SRCS += xpr_fifo.c
endif

# File
###############################################################################
ifeq ($(XPR_FILE),y)
libxpr_SRCS += \
xpr_file_unix.c \
xpr_file.c
endif

# Flex Queue
###############################################################################
ifeq ($(XPR_FQ),y)
libxpr_SRCS += xpr_fq.c
endif

# GPIO
###############################################################################
ifeq ($(XPR_GPIO),y)
$(eval $(call add_driver,GPIO,A5S,$(XPR_GPIO_DRIVER_A5S),a5s.c,,,))
libxpr_SRCS += xpr_gpio.c
endif

# H264
###############################################################################
ifeq ($(XPR_H264),y)
libxpr_SRCS += xpr_h264.c
endif

# ICMP
###############################################################################
ifeq ($(XPR_ICMP),y)
libxpr_SRCS += xpr_icmp_unix.c
endif

# JSON
###############################################################################
ifeq ($(XPR_JSON),y)
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

# List
###############################################################################
ifeq ($(XPR_LIST),y)
libxpr_SRCS += xpr_list.c
endif

# MCDEC
###############################################################################
ifeq ($(XPR_MCDEC),y)
libxpr_SRCS += xpr_mcdec.c
libxpr_LIBS += -Wl,-Bstatic -lavcodec -lavresample -lavutil -lswscale -Wl,-Bdynamic
endif

# MD5
###############################################################################
ifeq ($(XPR_MD5),y)
libxpr_SRCS += xpr_md5.c
endif

# Memory management
###############################################################################
ifeq ($(XPR_MEM),y)
libxpr_SRCS += xpr_mem.c
endif

# Packetized Elementary Stream
###############################################################################
ifeq ($(XPR_PES),y)
libxpr_SRCS += xpr_pes.c
endif

# Plugin framework
###############################################################################
ifeq ($(XPR_PLUGIN),y)
libxpr_SRCS += xpr_plugin.c
endif

# RTSP framework
###############################################################################
ifeq ($(XPR_RTSP),y)
$(eval $(call add_driver,RTSP,LIVE,$(XPR_RTSP_DRIVER_LIVE),live/rtsp.cpp live/rtsp_connection.cpp live/rtsp_connectionmanager.cpp live/rtsp_framemerger.cpp live/rtsp_server.cpp live/rtsp_worker.cpp,,,-lliveMedia -lBasicUsageEnvironment -lgroupsock -lUsageEnvironment))
libxpr_SRCS += xpr_rtsp.c
endif


# SHA1
###############################################################################
ifeq ($(XPR_SHA1),y)
libxpr_SRCS += xpr_sha1.c
endif

# Serial
###############################################################################
ifeq ($(XPR_SERIAL),y)
libxpr_SRCS += xpr_serial_unix.c
endif

# Stream Block
###############################################################################
ifeq ($(XPR_STREAMBLOCK),y)
libxpr_SRCS += xpr_streamblock.c
endif

# Sync
###############################################################################
ifeq ($(XPR_SYNC),y)
libxpr_SRCS += xpr_sync.c xpr_sync_unix.c
endif

# System relates
###############################################################################
ifeq ($(XPR_SYS),y)
libxpr_SRCS += xpr_sys.c xpr_sys_unix.c
endif

# Thread
###############################################################################
ifeq ($(XPR_THREAD),y)
libxpr_SRCS += xpr_thread.c xpr_thread_unix.c
endif

# Universal Preference Settings framework
###############################################################################
ifeq ($(XPR_UPS),y)
libxpr_SRCS += \
drivers/ups/root.c \
drivers/ups/cam_img.c \
drivers/ups/sys_info.c \
drivers/ups/sys_net.c \
xpr_ups.c
endif

# URL
###############################################################################
ifeq ($(XPR_URL),y)
libxpr_SRCS += xpr_url.c
endif

# Utils
###############################################################################
ifeq ($(XPR_UTILS),y)
libxpr_SRCS += xpr_utils.c
endif

# XML
###############################################################################
ifeq ($(XPR_XML),y)
libxpr_SRCS += \
deps/roxml/roxml.c \
deps/roxml/roxml-internal.c \
deps/roxml/roxml-parse-engine.c \
xpr_xml.c
endif

# Concat drivers flags
###############################################################################
libxpr_CFLAGS += $(libxpr_CFLAGS-y)
libxpr_LDFLAGS += $(libxpr_LDFLAGS-y)
libxpr_LIBS += $(libxpr_LIBS-y)
#libxpr_DEFS += $(libxpr_DEFS-n) $(libxpr_DEFS-n)
libxpr_SRCS += $(libxpr_SRCS-y)

# Objects from sources
###############################################################################
libxpr_OBJS := $(patsubst %.c,%.o,$(libxpr_SRCS))
libxpr_OBJS := $(patsubst %.cpp,%.o,$(libxpr_OBJS))
libxpr_OBJS := $(patsubst %.cxx,%.o,$(libxpr_OBJS))

# add_cxxflags() Examples
# $(eval $(call add_cflags,xpr_test.c,-DHELLO_ADD_CFLAGS=1))
# $(eval $(call add_cxxflags,xpr_test.cpp,-DHELLO_ADD_CXXFLAGS=1))

