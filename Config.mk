BUILD_ALL=1
BUILD_SRC=$(BUILD_ALL)
BUILD_EXAMPLES=$(BUILD_ALL)
BUILD_TESTS=$(BUILD_ALL)
BUILD_DOCS=0
BUILD_STATIC=0
BUILD_SHARED=1
BUILD_DEBUG=0
BUILD_SAMLL=0
BUILD_STRICT=0

ifeq ($(BUILD_DEBUG),1)
BUILD_OPT_FLAGS=-O0 -g
else
ifeq ($(BUILD_SMALL),1)
BUILD_OPT_FLAGS=-Os
else
BUILD_OPT_FLAGS=-O2 -g
endif
endif

ifeq ($(CROSS_COMPILER_NAME),arm-none-linux-gnueabi)
BUILD_OPT_FLAGS += -Wall -Werror -march=armv6k -mtune=arm1136j-s -msoft-float -mlittle-endian
endif

ifeq ($(BUILD_STRICT),1)
BUILD_OPT_FLAGS += -Wall -Werror -Wno-unused-function -Wno-unused-variable -Wno-unused-result -Wno-unused-but-set-variable
endif
