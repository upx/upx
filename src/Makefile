#
# UPX Makefile - needs GNU make 3.80 or better
#

.SUFFIXES:
MAKEFLAGS += -rR
override m = $($1) $(EXTRA_$1) $(UPX_EXTRA_$1)

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
LIBS += -L$(UCLDIR) -L$(UCLDIR)/src/.libs
endif
LIBS += -lucl -lz

upx_SOURCES := $(wildcard $(srcdir)/*.cpp)
upx_OBJECTS := $(notdir $(upx_SOURCES:.cpp=.o))

all: upx.out

upx.out: $(upx_OBJECTS) $(upx_DEPENDENCIES)
	$(strip $(CXXLD) $(call m,CPPFLAGS) $(call m,CXXFLAGS) $(call m,LDFLAGS) -o $@ $(upx_OBJECTS) $(upx_OJBECTS) $(call m,LIBS))

%.o : %.cpp
	$(strip $(CXX) $(call m,CPPFLAGS) $(call m,CXXFLAGS) -o $@ -c $<)

clean:
	rm -f *.d *.map *.o *.res upx.out

-include *.d
.PHONY: all clean
