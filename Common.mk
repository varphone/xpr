define make_subdir
 @for d in $1; do \
  (make -C $$d $2) \
 done
endef

define run_test
 @for a in $^; do \
  echo "====== Running test: $$a ======"; \
  ./$$a && echo "====== SUCCESS ======" || echo "====== FAILURE ======"; \
 done
endef

define filter_spc
$(subst .,_,$(subst -,_,$(subst /,_,$1)))
endef

# add_cflags(source file,clfags)
define add_cflags
$(eval $(call filter_spc,$1)_CFLAGS=$2)
endef

# add_cxxflags(source file,cxxflags)
define add_cxxflags
$(eval $(call filter_spc,$1)_CXXFLAGS=$2)
endef

VERBOSE = @

ifeq ($(V),1)
VERBOSE =
endif

CC      = $(VERBOSE)$(CROSS_COMPILER_PREFIX)gcc
CXX     = $(VERBOSE)$(CROSS_COMPILER_PREFIX)g++
AR      = $(VERBOSE)$(CROSS_COMPILER_PREFIX)ar
AS      = $(VERBOSE)$(CROSS_COMPILER_PREFIX)as
LD      = $(VERBOSE)$(CROSS_COMPILER_PREFIX)gcc

CP      = @cp
ECHO    = @echo
RM      = -rm

