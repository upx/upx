#
# UPX Makefile - needs GNU make 3.80 or better
#

.SUFFIXES:
MAKEFLAGS += -rR
override e = $($1) $(EXTRA_$1) $(upx_$1) $($(basename $(notdir $@)).$1)

srcdir ?= $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))
ifneq ($(srcdir),./)
srcdir := $(shell echo '$(srcdir)' | sed 's,/*$$,,')
##$(warning Info: using VPATH . $(srcdir))
VPATH = . $(srcdir)
endif

CXX = g++
CPPFLAGS += $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES)
CXXFLAGS += -O2 -MMD
CXXFLAGS += -Wall -W -Wcast-align -Wcast-qual -Wpointer-arith -Wwrite-strings -Werror
CXXLD = $(CXX)

# we need UCL and zlib - you can set envvar UCLDIR
ifneq ($(wildcard $(UCLDIR)/include/ucl/ucl.h),)
INCLUDES += -I$(UCLDIR)/include
LIBS += $(addprefix -L,$(dir $(wildcard $(UCLDIR)/libucl.a $(UCLDIR)/src/.libs/libucl.a)))
endif
LIBS += -lucl -lz

upx_SOURCES := $(wildcard $(srcdir)/*.cpp)
upx_OBJECTS := $(notdir $(upx_SOURCES:.cpp=.o))

all: upx.out

upx.out: $(upx_OBJECTS) $(upx_DEPENDENCIES)
	$(strip $(CXXLD) $(call e,CPPFLAGS) $(call e,CXXFLAGS) $(call e,LDFLAGS) -o $@ $(upx_OBJECTS) $(call e,LDADD) $(call e,LIBS))

%.o : %.cpp
	$(strip $(CXX) $(call e,CPPFLAGS) $(call e,CXXFLAGS) -o $@ -c $<)

mostlyclean clean distclean maintainer-clean:
	rm -f *.d *.map *.o *.obj *.res upx.exe upx.out

-include *.d
.PHONY: all mostlyclean clean distclean maintainer-clean
