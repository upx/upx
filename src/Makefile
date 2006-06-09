#
# UPX Makefile - needs GNU make 3.80 or better
#

MAKEFLAGS += -rR
.SUFFIXES:
export SHELL = /bin/sh
override e = $($1) $(EXTRA_$1) $(upx_$1) $($(basename $(notdir $@)).$1)

srcdir ?= $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))
ifneq ($(srcdir),./)
srcdir := $(shell echo '$(srcdir)' | sed 's,/*$$,,')
##$(info Info: using VPATH . $(srcdir))
VPATH := . $(srcdir)
endif

ifeq ($(CXX),)
USE_GNUC = 1
CXX = g++
endif
ifneq ($(USE_GNUC),)
CXXFLAGS += -O2 -MMD
CXXFLAGS += -Wall -W -Wcast-align -Wcast-qual -Wpointer-arith -Wwrite-strings -Werror
endif
CPPFLAGS += $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES)
CXXLD ?= $(CXX)

exeext ?= .out
libext ?= .a
objext ?= .o

# we need UCL and zlib - you can set envvar UCLDIR
ifneq ($(wildcard $(UCLDIR)/include/ucl/ucl.h),)
INCLUDES += -I$(UCLDIR)/include
LIBS += $(addprefix -L,$(dir $(wildcard $(UCLDIR)/libucl$(libext) $(UCLDIR)/src/.libs/libucl$(libext))))
endif
ifneq ($(wildcard $(LZMADIR)/include/ucl/ucl.h),)
INCLUDES += -I$(LZMADIR)
endif
LIBS += -lucl -lz

upx_SOURCES := $(wildcard $(srcdir)/*.cpp)
upx_OBJECTS := $(notdir $(upx_SOURCES:.cpp=$(objext)))

all: upx$(exeext)

upx$(exeext): $(upx_OBJECTS) $(upx_DEPENDENCIES)
	$($(notdir $@).PRE_LINK_STEP)
	$(strip $(CXXLD) $(call e,CPPFLAGS) $(call e,CXXFLAGS) $(call e,LDFLAGS) -o $@ $(upx_OBJECTS) $(call e,LDADD) $(call e,LIBS))
	$($(notdir $@).POST_LINK_STEP)

%.o : %.cpp
	$(strip $(CXX) $(call e,CPPFLAGS) $(call e,CXXFLAGS) -o $@ -c $<)

ifneq ($(USE_GNUC),)
compress.o : CXXFLAGS += -Wno-non-virtual-dtor
endif

mostlyclean clean distclean maintainer-clean:
	rm -f *.d *.map *.o *.obj *.res upx.exe upx.out upx.ttp upx$(exeext)

-include *.d
.PHONY: all mostlyclean clean distclean maintainer-clean
