BUILD_ALL=1
BUILD_SRC=$(BUILD_ALL)
BUILD_EXAMPLES=$(BUILD_ALL)
BUILD_TESTS=$(BUILD_ALL)
BUILD_DOCS=0
BUILD_STATIC=1
BUILD_SHARED=0
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

BUILD_OPT_FLAGS += -Wall -Werror

ifneq ($(BUILD_STRICT),1)
BUILD_OPT_FLAGS += -Wno-unused-function -Wno-unused-variable -Wno-unused-result -Wno-unused-but-set-variable
endif
