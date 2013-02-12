#
# UPX Makefile - needs GNU make 3.81 or better
#
# Copyright (C) 1996-2013 Markus Franz Xaver Johannes Oberhumer
#

MAKEFLAGS += -rR
.SUFFIXES:
export SHELL = /bin/sh
override e = $($1) $(EXTRA_$1) $(upx_$1) $($(basename $(notdir $@)).$1)

ifneq ($(findstring $(firstword $(MAKE_VERSION)),3.77 3.78 3.78.1 3.79 3.79.1 3.80),)
$(error GNU make 3.81 or better is required)
endif

ifndef srcdir
srcdir := $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))
srcdir := $(shell echo '$(srcdir)' | sed 's,/*$$,,')
endif
ifndef top_srcdir
top_srcdir := $(srcdir)/..
endif
include $(wildcard $(top_srcdir)/Makevars.global ./Makevars.local)
ifneq ($(srcdir),.)
##$(info Info: using VPATH . $(srcdir))
VPATH := . $(srcdir)
endif

ifeq ($(CXX),)
CXX = g++
endif
ifneq ($(findstring $(firstword $(CXX)),g++),)
USE_GNUC ?= 1
endif
ifeq ($(USE_GNUC),1)
ifeq ($(DEBUG),1)
CXXFLAGS += -O0 -g
else
CXXFLAGS += -O2
endif
CXXFLAGS_WERROR = -Werror
CXXFLAGS += -Wall -W -Wcast-align -Wcast-qual -Wpointer-arith -Wshadow -Wwrite-strings
CXXFLAGS += $(CXXFLAGS_WERROR)
endif
CPPFLAGS += $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES)
CXXLD ?= $(CXX)

exeext ?= .out
libext ?= .a
objext ?= .o

upx_SOURCES := $(sort $(wildcard $(srcdir)/*.cpp))
upx_OBJECTS := $(notdir $(upx_SOURCES:.cpp=$(objext)))

# we need UCL and zlib - you can set envvar UPX_UCLDIR
ifneq ($(wildcard $(UPX_UCLDIR)/include/ucl/ucl.h),)
INCLUDES += -I$(UPX_UCLDIR)/include
LIBS += $(addprefix -L,$(dir $(wildcard $(UPX_UCLDIR)/libucl$(libext) $(UPX_UCLDIR)/src/.libs/libucl$(libext))))
endif
LIBS += -lucl -lz
# you should set envvar UPX_LZMADIR to point to your unpacked LZMA SDK
include $(top_srcdir)/src/stub/src/c/Makevars.lzma
ifneq ($(UPX_LZMA_VERSION),)
DEFS += -DWITH_LZMA=$(UPX_LZMA_VERSION)
INCLUDES += -I$(UPX_LZMADIR)
endif


all: upx$(exeext) | .depend
.DELETE_ON_ERROR: upx$(exeext) $(upx_OBJECTS) .depend

upx$(exeext): $(upx_OBJECTS) $(upx_DEPENDENCIES)
	$($(notdir $@).PRE_LINK_STEP)
	$(strip $(CXXLD) $(call e,CPPFLAGS) $(call e,CXXFLAGS) $(call e,LDFLAGS) -o $@ $(upx_OBJECTS) $(call e,LDADD) $(call e,LIBS))
	$($(notdir $@).POST_LINK_STEP)

%.o : %.cpp | .depend
	$(strip $(CXX) $(call e,CPPFLAGS) $(call e,CXXFLAGS) -o $@ -c $<)

.depend: $(sort $(wildcard $(srcdir)/*.cpp $(srcdir)/*.h)) $(MAKEFILE_LIST)
	@rm -f $@
ifeq ($(USE_GNUC),1)
	@echo "Updating $@"
	@$(strip $(CXX) $(call e,CPPFLAGS) -MM) $(filter %.cpp,$^) > $@
else
	touch $@
endif


ifeq ($(USE_GNUC),1)
##compress_lzma$(objext) : CXXFLAGS += -O3 -fomit-frame-pointer
compress_lzma$(objext) : CXXFLAGS += -Wno-cast-qual
compress_lzma$(objext) : CXXFLAGS += -Wno-shadow
ifeq ($(UPX_LZMA_VERSION),0x443)
compress_lzma$(objext) : CXXFLAGS += -Wno-non-virtual-dtor
compress_lzma$(objext) : CXXFLAGS += -Wno-unused
endif
# needed for gcc-4.3:
compress_lzma$(objext) : CXXFLAGS += -Wno-error
endif


mostlyclean clean distclean maintainer-clean:
	rm -f *.d *.map *.o *.obj *.res .depend upx.exe upx.out upx.ttp upx$(exeext)

.PHONY: all mostlyclean clean distclean maintainer-clean

ifeq ($(MAKECMDGOALS),mostlyclean)
else ifeq ($(MAKECMDGOALS),clean)
else ifeq ($(MAKECMDGOALS),distclean)
else ifeq ($(MAKECMDGOALS),maintainer-clean)
else
-include .depend
endif
