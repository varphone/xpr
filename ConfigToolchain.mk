# Fix some toolchain crashed on Ubuntu 18+
export LC_ALL=C

ifeq ($(TARGET_BOARD),a5s)
ARCH_CFLAGS += \
	-march=armv6k \
	-mtune=arm1136j-s \
	-msoft-float \
	-mlittle-endian
endif

ifneq ($(TOOLSET_NAME),)
EXTRA_CFLAGS    += -I$(TOP_DIR)/contrib/$(TOOLSET_NAME)/include
EXTRA_LDFLAGS   += -L$(TOP_DIR)/contrib/$(TOOLSET_NAME)/lib
endif

TARGET_CFLAGS   += $(BUILD_OPT_CFLAGS) $(ARCH_CFLAGS) $(EXTRA_CFLAGS)
TARGET_CXXFLAGS += $(BUILD_OPT_CFLAGS) $(ARCH_CFLAGS) $(EXTRA_CFLAGS)
TARGET_LDFLAGS  += $(BUILD_OPT_LDFLAGS) $(EXTRA_LDFLAGS)

CFLAGS   += \
	$(TARGET_CFLAGS) \
	$(PROJECT_CFLAGS) \
	-I${TOP_DIR}/include

CXXFlAGS += \
	$(TARGET_CXXFLAGS) \
	$(PROJECT_CXXFLAGS) \
	-I${TOP_DIR}/include

LDFLAGS  += \
	$(TARGET_LDFLAGS) \
	$(PROJECT_LDFLAGS) \
	-L${TOP_DIR}/lib
