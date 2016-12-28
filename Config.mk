BUILD_ALL=y
BUILD_SRC=$(BUILD_ALL)
BUILD_EXAMPLES=$(BUILD_ALL)
BUILD_TESTS=$(BUILD_ALL)
BUILD_DOCS=n
BUILD_STATIC=n
BUILD_SHARED=y
BUILD_DEBUG=n
BUILD_SAMLL=n
BUILD_STRICT=n

ifeq ($(BUILD_DEBUG),y)
BUILD_OPT_FLAGS=-O0 -g
else
ifeq ($(BUILD_SMALL),y)
BUILD_OPT_FLAGS=-Os
else
BUILD_OPT_FLAGS=-O2 -g
endif
endif

ifeq ($(CROSS_COMPILER_NAME),arm-none-linux-gnueabi)
BUILD_OPT_FLAGS += -Wall -Werror -march=armv6k -mtune=arm1136j-s -msoft-float -mlittle-endian
endif

ifneq ($(BUILD_STRICT),y)
BUILD_OPT_FLAGS += -Wall -Werror -Wno-unused-function -Wno-unused-variable -Wno-unused-result -Wno-unused-but-set-variable
endif
