#
# UPX Makefile - needs GNU make 3.81 or better
#
# Copyright (C) 1996-2016 Markus Franz Xaver Johannes Oberhumer
#

# build configuration options for this Makefile
BUILD_TYPE_DEBUG    ?= 0
BUILD_TYPE_SANITIZE ?= 0
BUILD_USE_DEPEND    ?= 1

MAKEFLAGS += -r
ifneq ($(upx_CXX),)
CXX = $(upx_CXX)
endif
ifneq ($(upx_CXXLD),)
CXXLD = $(upx_CXXLD)
else
CXXLD = $(CXX)
endif

.SUFFIXES:
export SHELL = /bin/sh
override e = $($1) $(EXTRA_$1) $(upx_$1) $(upx_EXTRA_$1) $($(basename $(notdir $@)).$1)

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

exeext ?= .out
libext ?= .a
objext ?= .o

upx_SOURCES := $(sort $(wildcard $(srcdir)/*.cpp))
upx_OBJECTS := $(notdir $(upx_SOURCES:.cpp=$(objext)))

ifneq ($(wildcard $(top_srcdir)/.git/.),)
UPX_VERSION_GITREV := $(strip $(shell cd '$(top_srcdir)' && git rev-parse --short=12 HEAD || echo 'ERROR'))
ifneq ($(UPX_VERSION_GITREV),)
x := $(strip $(shell cd '$(top_srcdir)' && git diff --exit-code HEAD >/dev/null && echo '' || echo '+'))
DEFS += '-DUPX_VERSION_GITREV="$(UPX_VERSION_GITREV)$(x)"'
endif
endif

# we need UCL and zlib - you can set envvar UPX_UCLDIR
ifneq ($(wildcard $(UPX_UCLDIR)/include/ucl/ucl.h),)
INCLUDES += -I$(UPX_UCLDIR)/include
LIBS += $(addprefix -L,$(dir $(wildcard $(UPX_UCLDIR)/libucl$(libext) $(UPX_UCLDIR)/src/.libs/libucl$(libext))))
endif
LIBS += -lucl -lz
# LZMA from https://github.com/upx/upx-lzma-sdk
include $(top_srcdir)/src/stub/src/c/Makevars.lzma
DEFS += -DWITH_LZMA=$(UPX_LZMA_VERSION)

CPPFLAGS += $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES)
ifeq ($(BUILD_TYPE_DEBUG),1)
CXXFLAGS += -O0 -g
else
CXXFLAGS += -O2
endif
# protect against security threats caused by misguided C++ compiler "optimizations"
ifeq ($(findstring clang,$(CXX)),)
CXXFLAGS += -fno-delete-null-pointer-checks
endif
CXXFLAGS += -fno-strict-aliasing -fwrapv
CXXFLAGS += -Wall -W -Wcast-align -Wcast-qual -Wmissing-declarations -Wpointer-arith -Wshadow -Wwrite-strings
CXXFLAGS_WERROR ?= -Werror
CXXFLAGS += $(CXXFLAGS_WERROR)

ifeq ($(BUILD_TYPE_SANITIZE),1)
CXXFLAGS_SANITIZE := -fsanitize=address,undefined -fno-omit-frame-pointer -DACC_CFG_NO_UNALIGNED
CXXFLAGS += $(CXXFLAGS_SANITIZE)
LDFLAGS  += $(CXXFLAGS_SANITIZE)
# these are the only 2 objects that are actually speed-sensitive
compress_lzma$(objext) filteri$(objext) : CXXFLAGS := $(filter-out $(CXXFLAGS_SANITIZE),$(CXXFLAGS))
endif


all: upx$(exeext) | ./.depend
.DELETE_ON_ERROR: upx$(exeext) $(upx_OBJECTS) ./.depend

upx$(exeext): $(upx_OBJECTS) $(upx_DEPENDENCIES)
	$($(notdir $@).PRE_LINK_STEP)
	$(strip $(CXXLD) $(call e,CPPFLAGS) $(call e,CXXFLAGS) $(call e,LDFLAGS) -o $@ $(upx_OBJECTS) $(call e,LDADD) $(call e,LIBS))
	$($(notdir $@).POST_LINK_STEP)

%.o : %.cpp | ./.depend
	$(strip $(CXX) $(call e,CPPFLAGS) $(call e,CXXFLAGS) -o $@ -c $<)

ifeq ($(BUILD_USE_DEPEND),1)
./.depend: $(sort $(wildcard $(srcdir)/*.cpp $(srcdir)/*.h)) $(MAKEFILE_LIST)
	@rm -f $@
	@echo "Updating $@"
	@$(strip $(CXX) $(call e,CPPFLAGS) -MM) $(filter %.cpp,$^) > $@
else
./.depend:
.PHONY: ./.depend
endif


./.depend compress_lzma$(objext) : INCLUDES += -I$(UPX_LZMADIR)

compress_lzma$(objext) : CXXFLAGS += -Wno-shadow
p_mach$(objext)        : CXXFLAGS += -Wno-cast-align


mostlyclean clean distclean maintainer-clean:
	rm -f *.d *.map *.o *.obj *.res ./.depend upx.exe upx.out upx.ttp upx$(exeext)

.PHONY: all mostlyclean clean distclean maintainer-clean

ifeq ($(MAKECMDGOALS),mostlyclean)
else ifeq ($(MAKECMDGOALS),clean)
else ifeq ($(MAKECMDGOALS),distclean)
else ifeq ($(MAKECMDGOALS),maintainer-clean)
else ifeq ($(MAKECMDGOALS),clang-format)
else
ifeq ($(BUILD_USE_DEPEND),1)
-include ./.depend
endif
help$(objext): $(MAKEFILE_LIST)
endif


CLANG_FORMAT_FILES += packhead.cpp
CLANG_FORMAT_FILES += s_djgpp2.cpp s_object.cpp s_vcsa.cpp s_win32.cpp screen.h
CLANG_FORMAT_FILES += snprintf.cpp
CLANG_FORMAT_FILES += stdcxx.cpp stdcxx.h
CLANG_FORMAT_FILES += ui.cpp ui.h util.h
clang-format:
	$(top_srcdir)/src/stub/scripts/upx-clang-format -i $(addprefix $(top_srcdir)/src/,$(CLANG_FORMAT_FILES))
.PHONY: clang-format

# vim:set ts=8 sw=8 noet:
