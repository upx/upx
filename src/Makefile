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
CXX = g++
endif
ifeq ($(firstword $(CXX)),g++)
USE_GNUC ?= 1
endif
ifeq ($(USE_GNUC),1)
CXXFLAGS += -MMD
ifeq ($(DEBUG),1)
CXXFLAGS += -O0 -g
else
CXXFLAGS += -O2
endif
CXXFLAGS += -Wall -W -Wcast-align -Wcast-qual -Wpointer-arith -Wwrite-strings -Werror
##CXXFLAGS += -Wshadow
endif
CPPFLAGS += $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES)
CXXLD ?= $(CXX)

exeext ?= .out
libext ?= .a
objext ?= .o

upx_SOURCES := $(wildcard $(srcdir)/*.cpp)
upx_OBJECTS := $(notdir $(upx_SOURCES:.cpp=$(objext)))

# we need UCL and zlib - you can set envvar UPX_UCLDIR
ifneq ($(wildcard $(UPX_UCLDIR)/include/ucl/ucl.h),)
INCLUDES += -I$(UPX_UCLDIR)/include
LIBS += $(addprefix -L,$(dir $(wildcard $(UPX_UCLDIR)/libucl$(libext) $(UPX_UCLDIR)/src/.libs/libucl$(libext))))
endif
LIBS += -lucl -lz
# you should set envvar UPX_LZMADIR to point to your unpacked lzma443.tar.bz2
ifneq ($(wildcard $(UPX_LZMADIR)/C/7zip/.),)
DEFS += -DWITH_LZMA
INCLUDES += -I$(UPX_LZMADIR)
endif
# you should set envvar UPX_LUADIR to point to your unpacked lua-5.1.1.tar.gz
# [disabled for now]
ifneq ($(wildcard $(UPX_LUADIR)/src/lua.h),)
##DEFS += -DWITH_LUA
##upx_SOURCES_LUA := $(wildcard $(UPX_LUADIR)/src/*.c)
##upx_SOURCES_LUA := $(filter-out %lua.c, $(upx_SOURCES_LUA))
##upx_SOURCES_LUA := $(filter-out %luac.c, $(upx_SOURCES_LUA))
##upx_OBJECTS_LUA := $(notdir $(upx_SOURCES_LUA:.c=$(objext)))
##upx_OBJECTS_LUA := $(addprefix lua_,$(upx_OBJECTS_LUA))
##upx_OBJECTS     += $(upx_OBJECTS_LUA)
else
##$(error please set UPX_LUADIR)
endif


all: upx$(exeext)

upx$(exeext): $(upx_OBJECTS) $(upx_DEPENDENCIES)
	$($(notdir $@).PRE_LINK_STEP)
	$(strip $(CXXLD) $(call e,CPPFLAGS) $(call e,CXXFLAGS) $(call e,LDFLAGS) -o $@ $(upx_OBJECTS) $(call e,LDADD) $(call e,LIBS))
	$($(notdir $@).POST_LINK_STEP)

%.o : %.cpp
	$(strip $(CXX) $(call e,CPPFLAGS) $(call e,CXXFLAGS) -o $@ -c $<)


ifeq ($(USE_GNUC),1)
##compress_lzma$(objext) : CXXFLAGS += -O3 -fomit-frame-pointer
compress_lzma$(objext) : CXXFLAGS += -Wno-cast-qual
compress_lzma$(objext) : CXXFLAGS += -Wno-non-virtual-dtor
compress_lzma$(objext) : CXXFLAGS += -Wno-shadow
compress_lzma$(objext) : CXXFLAGS += -Wno-unused
##compress_lzma$(objext) : CXXFLAGS += -Wno-error
endif

ifneq ($(strip $(upx_OBJECTS_LUA)),)
$(upx_OBJECTS_LUA) : lua_%.o : $(UPX_LUADIR)/src/%.c
	$(strip $(CXX) $(call e,CPPFLAGS) $(call e,CXXFLAGS) -o $@ -c $<)
ifeq ($(USE_GNUC),1)
$(upx_OBJECTS_LUA)     : CXXFLAGS += -Wno-cast-align
$(upx_OBJECTS_LUA)     : CXXFLAGS += -Wno-cast-qual
$(upx_OBJECTS_LUA)     : CXXFLAGS += -Wno-error
endif
endif


mostlyclean clean distclean maintainer-clean:
	rm -f *.d *.map *.o *.obj *.res upx.exe upx.out upx.ttp upx$(exeext)

-include *.d
.PHONY: all mostlyclean clean distclean maintainer-clean
