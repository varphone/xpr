# make_subdir()
###############################################################################
define make_subdir
 @for d in $1; do \
  ($(MAKE) -C $$d $2) || exit 2; \
 done
endef

# run_test()
###############################################################################
define run_test
 @for a in $^; do \
  echo "====== Running test: $$a ======"; \
  ./$$a && echo "====== SUCCESS ======" || echo "====== FAILURE ======"; \
 done
endef

# filter_spc()
###############################################################################
define filter_spc
$(subst .,_,$(subst -,_,$(subst /,_,$1)))
endef

# add_cflags(source file,clfags)
###############################################################################
define add_cflags
$(eval $(call filter_spc,$1)_CFLAGS=$2)
endef

# add_cxxflags(source file,cxxflags)
###############################################################################
define add_cxxflags
$(eval $(call filter_spc,$1)_CXXFLAGS=$2)
endef

# add_driver(Cat,Id,Switch?[0|1],Objects,CFLAGS,LDFLAGS,LIBS)
###############################################################################
define add_driver
$(eval XPR_$1_DRIVER_$2 := $3)
$(eval libxpr_SRCS-$(XPR_$1_DRIVER_$2) += $(patsubst %,drivers/$(shell echo $1 | tr A-Z a-z)/%,$4))
$(eval libxpr_DEFS-$(XPR_$1_DRIVER_$2) += -DXPR_$1_DRIVER_$2=$(XPR_$1_DRIVER_$2))
$(eval libxpr_CFLAGS-$(XPR_$1_DRIVER_$2) += $5)
$(eval libxpr_LDFLAGS-$(XPR_$1_DRIVER_$2) += $6)
$(eval libxpr_LIBS-$(XPR_$1_DRIVER_$2) += $7)
endef

VERBOSE = @

ifeq ($(V),y)
VERBOSE =
endif

CC      = $(VERBOSE)$(CROSS_COMPILER_PREFIX)gcc
CXX     = $(VERBOSE)$(CROSS_COMPILER_PREFIX)g++
AR      = $(VERBOSE)$(CROSS_COMPILER_PREFIX)ar
AS      = $(VERBOSE)$(CROSS_COMPILER_PREFIX)as
LD      = $(VERBOSE)$(CROSS_COMPILER_PREFIX)gcc
OBJCOPY = $(VERBOSE)$(CROSS_COMPILER_PREFIX)objcopy
STRIP   = $(VERBOSE)$(CROSS_COMPILER_PREFIX)strip

HOST_CC = $(VERBOSE)gcc
HOST_CXX= $(VERBOSE)g++
HOST_AR = $(VERBOSE)ar
HOST_AS = $(VERBOSE)as
HOST_LD = $(VERBOSE)ld
HOST_OBJCOPY = $(VERBOSE)objcopy
HOST_STRIP   = $(VERBOSE)strip

CP      = @cp
ECHO    = @echo
RM      = -rm -f
