#
# UPX Makefile - needs GNU make
#
# Copyright (C) 1996-2020 Markus Franz Xaver Johannes Oberhumer
#

MAKEFLAGS += -rR
.SUFFIXES:
export SHELL = /bin/sh

# build configuration options for this Makefile
BUILD_TYPE_DEBUG    ?= 0
BUILD_TYPE_SANITIZE ?= 0
BUILD_USE_DEPEND    ?= 1

ifndef srcdir
  srcdir := $(dir $(lastword $(MAKEFILE_LIST)))
  srcdir := $(shell echo '$(srcdir)' | sed 's,/*$$,,' || echo 'ERROR')
endif
ifndef top_srcdir
  top_srcdir := $(srcdir)/..
endif
include $(wildcard $(top_srcdir)/Makevars.global ./Makevars.local)
ifneq ($(srcdir),.)
  vpath %.cpp .:$(srcdir)
  vpath %.h   .:$(srcdir)
endif

# toolchain
CXX    ?= g++
CXXLD   = $(CXX)
exeext ?= .out
libext ?= .a
objext ?= .o

# flags composition
override all_flags = $(mf.$1) $($1) $(EXTRA_$1) $(upx_$1) $(upx_EXTRA_$1) $($(basename $(notdir $@)).$1)
ALL_CPPFLAGS = $(strip $(call all_flags,CPPFLAGS))
ALL_CXXFLAGS = $(strip $(call all_flags,CXXFLAGS))
ALL_LDFLAGS  = $(strip $(call all_flags,LDFLAGS))
ALL_LDADD    = $(strip $(call all_flags,LDADD))
ALL_LIBS     = $(strip $(call all_flags,LIBS))

upx_SOURCES := $(sort $(wildcard $(srcdir)/*.cpp))
upx_OBJECTS := $(notdir $(upx_SOURCES:.cpp=$(objext)))

ifneq ($(wildcard $(top_srcdir)/.git/.),)
UPX_VERSION_GITREV := $(strip $(shell cd '$(top_srcdir)' && git rev-parse --short=12 HEAD || echo 'ERROR'))
ifneq ($(UPX_VERSION_GITREV),)
  GITREV_PLUS := $(strip $(shell cd '$(top_srcdir)' && git diff --exit-code HEAD >/dev/null && echo '' || echo '+'))
  DEFS += '-DUPX_VERSION_GITREV="$(UPX_VERSION_GITREV)$(GITREV_PLUS)"'
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

# optional flags
ifeq ($(BUILD_TYPE_DEBUG),1)
  CXXFLAGS_OPTIMIZE ?= -O0 -g
else
  CXXFLAGS_OPTIMIZE ?= -O2
endif
ifeq ($(BUILD_TYPE_SANITIZE),1)
  CXXFLAGS_SANITIZE ?= -fsanitize=address,undefined -fno-omit-frame-pointer -DACC_CFG_NO_UNALIGNED
else
  CXXFLAGS_SANITIZE ?= -fstack-protector-all
endif
ifeq ($(findstring clang,$(CXX)),)
  CXXFLAGS_NO_DELETE_NULL_POINTER_CHECKS ?= -fno-delete-null-pointer-checks
endif
CXXFLAGS_WERROR ?= -Werror

# Mandatory Flags - DO NOT CHANGE!
mf.CPPFLAGS += $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES)
mf.CXXFLAGS += $(CXXFLAGS_OPTIMIZE)
# protect against security threats caused by misguided C++ compiler "optimizations"
mf.CXXFLAGS += $(CXXFLAGS_NO_DELETE_NULL_POINTER_CHECKS)
mf.CXXFLAGS += -fno-strict-aliasing -fno-strict-overflow
mf.CXXFLAGS += -funsigned-char
mf.CXXFLAGS += $(CXXFLAGS_SANITIZE)
mf.CXXFLAGS += -Wall -W -Wcast-align -Wcast-qual -Wmissing-declarations -Wpointer-arith -Wshadow -Wvla -Wwrite-strings
mf.CXXFLAGS += $(CXXFLAGS_WERROR)

# rules
all: upx$(exeext) | ./.depend
.DELETE_ON_ERROR: upx$(exeext) $(upx_OBJECTS) ./.depend

upx$(exeext): $(upx_OBJECTS) $(upx_DEPENDENCIES)
	$($(notdir $@).PRE_LINK_STEP)
	$(strip $(CXXLD) $(ALL_CXXFLAGS) $(ALL_LDFLAGS) -o $@ $(upx_OBJECTS) $(ALL_LDADD) $(ALL_LIBS))
	$($(notdir $@).POST_LINK_STEP)
	$(CHECK_WHITESPACE)

%.o : %.cpp | ./.depend
	$(strip $(CXX) $(ALL_CPPFLAGS) $(ALL_CXXFLAGS) -o $@ -c $<)
%.cpp.ii : %.cpp
	$(strip $(CXX) $(ALL_CPPFLAGS) $(ALL_CXXFLAGS) -o $@ -E $<)

# object overrides
compress_lzma$(objext) : INCLUDES += -I$(UPX_LZMADIR)
# these are the only 2 objects that are actually speed-sensitive
compress_lzma$(objext) : override CXXFLAGS_SANITIZE =
filteri$(objext)       : override CXXFLAGS_SANITIZE =
# disable some warnings
compress_lzma$(objext) : upx_CXXFLAGS += -Wno-shadow
p_mach$(objext)        : upx_CXXFLAGS += -Wno-cast-align

ifeq ($(BUILD_USE_DEPEND),1)
./.depend: $(sort $(wildcard $(srcdir)/*.cpp $(srcdir)/*.h)) $(MAKEFILE_LIST)
	@rm -f $@
	@echo "Updating $@"
	@$(strip $(CXX) $(ALL_CPPFLAGS) $(ALL_CXXFLAGS) -MM) $(filter %.cpp,$^) > $@
./.depend : INCLUDES += -I$(UPX_LZMADIR)
else
./.depend:
.PHONY: ./.depend
endif

CHECK_WHITESPACE =
ifeq ($(shell uname),Linux)
CHECK_WHITESPACE = $(top_srcdir)/src/stub/scripts/check_whitespace.sh $(top_srcdir)
ifneq ($(wildcard $(top_srcdir)/.git/.),)
CHECK_WHITESPACE = $(top_srcdir)/src/stub/scripts/check_whitespace_git.sh $(top_srcdir)
endif
check-whitespace : ; $(CHECK_WHITESPACE)
endif
.PHONY: check-whitespace

mostlyclean clean distclean maintainer-clean:
	rm -f *.d *.ii *.map *.o *.obj *.res ./.depend upx.exe upx.out upx.ttp upx$(exeext)

.PHONY: all mostlyclean clean distclean maintainer-clean

ifeq ($(MAKECMDGOALS),mostlyclean)
else ifeq ($(MAKECMDGOALS),clean)
else ifeq ($(MAKECMDGOALS),distclean)
else ifeq ($(MAKECMDGOALS),maintainer-clean)
else ifeq ($(MAKECMDGOALS),clang-format)
else ifeq ($(MAKECMDGOALS),check-whitespace)
else
ifeq ($(BUILD_USE_DEPEND),1)
-include ./.depend
endif
help$(objext): $(MAKEFILE_LIST)
endif

# "make run-testsuite"
# search for the UPX testsuite -- git clone https://github.com/upx/upx-testsuite.git
# you also can override upx_testsuite_SRCDIR
ifndef upx_testsuite_SRCDIR
# search standard locations below $(top_srcdir)
ifneq ($(wildcard $(top_srcdir)/../upx-testsuite.git/files/packed/.),)
upx_testsuite_SRCDIR := $(top_srcdir)/../upx-testsuite.git
else ifneq ($(wildcard $(top_srcdir)/../upx-testsuite/files/packed/.),)
upx_testsuite_SRCDIR := $(top_srcdir)/../upx-testsuite
endif
endif
# run the UPX testsuite
# The expected (old) checksums are in $(top_srcdir)/.github/travis_testsuite_1-expected_sha256sums.sh
# The   actual (new) checksums are in tmp-testsuite/testsuite_1/.sha256sums.recreate
ifneq ($(wildcard $(upx_testsuite_SRCDIR)/files/packed/.),)
ifneq ($(wildcard $(top_srcdir)/.github/travis_testsuite_1.sh),)
run-testsuite: export upx_exe                := ./upx$(exeext)
run-testsuite: export upx_testsuite_SRCDIR   := $(upx_testsuite_SRCDIR)
run-testsuite: export upx_testsuite_BUILDDIR := ./tmp-testsuite
run-testsuite: ./upx$(exeext)
	time -p bash $(top_srcdir)/.github/travis_testsuite_1.sh
.PHONY: run-testsuite
endif
endif

# automatically format some C++ source code files
ifeq ($(shell uname),Linux)
CLANG_FORMAT_FILES += linker.cpp linker.h packhead.cpp packmast.cpp packmast.h
CLANG_FORMAT_FILES += s_djgpp2.cpp s_object.cpp s_vcsa.cpp s_win32.cpp screen.h
CLANG_FORMAT_FILES += snprintf.cpp
CLANG_FORMAT_FILES += stdcxx.cpp stdcxx.h
CLANG_FORMAT_FILES += ui.cpp ui.h util.cpp util.h work.cpp
clang-format:
	$(top_srcdir)/src/stub/scripts/upx-clang-format -i $(addprefix $(top_srcdir)/src/,$(CLANG_FORMAT_FILES))
.PHONY: clang-format
endif

# vim:set ts=8 sw=8 noet:
