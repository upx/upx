#
# UPX Makefile (GNU make) - works with unix/win32/djgpp2
#
# usage:
#   `make target=linux'             # linux
#   `make target=djggp2'            # dos32 - djggp2 2.03
#   `make target=cygwin'            # win32 - cygwin 1.3.x
#   `make target=mingw32'           # win32 - mingw32
#   `make target=no-cygwin'         # win32 - mingw32 as included in cygwin 1.3.x
#   `make target=bcc'               # win32 - Borland C++ 5.5.1
#   `make target=dmc'               # win32 - Digital Mars C++ 8.32
#   `make target=vc6'               # win32 - Visual C++ 6.0
#   `make target=wcc'               # win32 - Watcom C++ 11.0c
#   `make target=cross-m68k-linux'  # m68k-linux cross compiler
#   `make target=cross-mingw32'     # i586-mingw32msvc cross compiler
#   `make target=cross-mint'        # m68k-atari-mint cross compiler
#


# configuration section

ifeq ($(strip $(UCLDIR)),)
# change this to reflect where the UCL library is
UCLDIR = $(HOME)/local/src/ucl-1.01
endif

##target =

DEBUG = 0


# -------------------------------------------------------
# You should not have to change anything below this line.
# -------------------------------------------------------

SHELL = /bin/sh


.SUFFIXES:
.SUFFIXES: .a .c .cpp .exe .lib .o .obj .rc .res .ttp


srcdir = .
top_srcdir = ..

VPATH = $(srcdir)


# auto-detect the target unless given on the commandline
ifeq ($(strip $(target)),)
##target = djgpp2
ifneq ($(strip $(wildcard /usr/include/linux)),)
target = linux
endif
##ifneq ($(strip $(wildcard /platform/sun4?/kernel/unix)),)
##target = sparc
##endif
endif

ifeq ($(strip $(target)),)
include please_specify_a_target
endif


# util var for use in the rules - basename of the current target
override T = $(basename $@)


# /***********************************************************************
# // object files
# ************************************************************************/

# these use exceptions & RTTI
OBJECTS1 = \
	compress$o except$o file$o lefile$o \
	filter$o linker$o mem$o msg$o stdcxx$o work$o ui$o \
	packer$o packerf$o packhead$o packmast$o \
	p_com$o p_djgpp2$o p_elks$o p_exe$o \
	p_lx_elf$o p_lx_exc$o p_lx_sep$o p_lx_sh$o \
	p_ps1$o p_sys$o p_tmt$o p_tos$o \
	p_unix$o p_vmlinz$o p_w16ne$o p_w32pe$o p_wcle$o

# no exceptions or RTTI
OBJECTS2 = \
	filteri$o help$o main$o mygetopt$o snprintf$o util$o \
	c_init$o c_file$o c_none$o c_screen$o \
	s_object$o s_djgpp2$o s_vcsa$o s_win32$o

# pure C sources
OBJECTS3 =

OBJECTS = $(OBJECTS1) $(OBJECTS2) $(OBJECTS3)
LIBS =
RESOURCES =


# /***********************************************************************
# // compiler defaults
# //   CXX_1: use exceptions & RTTI
# //   CXX_2: no exceptions or RTTI
# ************************************************************************/

CC_COMPILE    = $(CC) $(CCARCH) $(DEFS) $(INCLUDES) $(CFLAGS) $(CFLAGS_OUTPUT) -c $<
CXX_COMPILE_1 = $(CXX) $(CXXARCH) $(DEFS) $(INCLUDES) $(CXXFLAGS_1) $(CXXFLAGS_OUTPUT) -c $<
CXX_COMPILE_2 = $(CXX) $(CXXARCH) $(DEFS) $(INCLUDES) $(CXXFLAGS_2) $(CXXFLAGS_OUTPUT) -c $<
CXX_COMPILE   = $(CXX_COMPILE_1)
CXX_LINK_EXE  = $(CXXLD) $(CXXLDARCH) $(LDFLAGS) -o $@ $^ $(LDLIBS)
STUBEDIT_EXE  =
STUBIFY_EXE   =
CHMOD_EXE     =

# set defaults
o = .o
a = .a
e =
DEFS =
INCLUDES =
LIBDIRS =
DOS_LDLIBS =
DOS_LIBDIRS =

# set CC defaults
CC =
CCARCH =
CCFLAGS =
CFLAGS_OUTPUT =

# set CXX defaults
CXX = $(CC)
CXXARCH = $(CCARCH)
CXXFLAGS = $(CFLAGS)
CXXFLAGS_1 = $(CXXFLAGS)
CXXFLAGS_2 = $(CXXFLAGS)
CXXFLAGS_OUTPUT = $(CFLAGS_OUTPUT)

# set CXXLD defaults
CXXLD = $(CXX)
CXXLDARCH = $(CXXARCH)
LDFLAGS =
LDLIBS =


# update defaults
ifeq ($(DEBUG),1)
  DEFS += -DDEBUG
##  DEFS += -DTESTING
endif


# /***********************************************************************
# // compression library
# ************************************************************************/

UCLDIR:=$(strip $(subst \,/,$(UCLDIR)))
NRVDIR:=$(strip $(subst \,/,$(NRVDIR)))
u = ucl
U = UCL
upx_exe_tail =
include $(srcdir)/Makedefs.inc
ifneq ($(strip $(wildcard $(NRVDIR)/include/nrv)),)
  u = nrv
  U = NRV
  upx_exe_tail = _$(u)
  include $(srcdir)/Makedefs.inc
endif
upx_exe = upx$(upx_exe_tail)$(e)

override tmp := -Wl,--rpath,
LDRPATH := $(addprefix $(tmp),$(LIBDIRS))
LIBDIRS += .
LDLIBDIRS := $(addprefix -L,$(LIBDIRS))

##LDFLAGS += $(LDRPATH)
LDFLAGS += $(LDLIBDIRS)


# /***********************************************************************
# // compiler settings
# ************************************************************************/

###
### Linux
###

ifeq ($(target),linux)
include $(srcdir)/Makedefs.gcc
override arch := $(shell uname -m | sed -e 's/^i[3456789]86$$/i386/')
DEFS += '-DUPX_CONFIG_H="config_h/linux.h"'
DEFS += -D_FILE_OFFSET_BITS=64
LDLIBS += -lmcheck
ifeq ($(arch),i386)
  CCARCH += -march=i386 -mcpu=i686
endif
##CFLAGS_WERROR = -Werror

ifeq ($(DEBUG),0)
  ##LDFLAGS += -static
  STUBEDIT_EXE = objcopy -S -R .comment -R .note $@
  ifeq ($(arch),i386)
    STUBIFY_EXE = perl $(srcdir)/stub/scripts/brandelf.pl $@
  endif
endif
CHMOD_EXE = chmod 755 $@

endif	# linux


###
### Linux cross compilers
###

ifeq ($(target),cross-m68k-linux)
include $(srcdir)/Makedefs.gcc
CC = m68k-linux-gcc
CXX = m68k-linux-g++
DEFS += '-DUPX_CONFIG_H="config_h/linux.h"'
##LDLIBS += -lmcheck
endif	# cross-m68k-linux


###
### sparc-sun-solaris2.8
###

ifeq ($(target),sparc)
include $(srcdir)/Makedefs.gcc
DEFS += '-DUPX_CONFIG_H="config_h/sparc_sun_solaris28.h"'
endif	# sparc


###
### Atari cross compiler
###

ifeq ($(target),cross-mint)
include $(srcdir)/Makedefs.gcc
e = .ttp
CC = m68k-atari-mint-gcc
CXX = m68k-atari-mint-g++
CCARCH += -m68000
endif

ifeq ($(target),cross-mint-m68040)
include $(srcdir)/Makedefs.gcc
e = .ttp
CC = m68k-atari-mint-gcc
CXX = m68k-atari-mint-g++
CCARCH += -m68040
endif

ifeq ($(target),cross-mint-m68020-60)
include $(srcdir)/Makedefs.gcc
e = .ttp
CC = m68k-atari-mint-gcc
CXX = m68k-atari-mint-g++
CCARCH += -m68020-60 -m68881
endif


###
### djgpp2
###

ifeq ($(target),djgpp2)
include $(srcdir)/Makedefs.gcc
e = .exe
CCARCH += -march=i386 -mcpu=i686
CFLAGS_WERROR = -Werror
STUBEDIT_EXE = stubedit $@ bufsize=0xfc00
ifneq ($(strip $(wildcard $(DJDIR)/bin/mfxdjstubify.ex[eE])),)
ifneq ($(strip $(wildcard $(DJDIR)/bin/cwsdstub.ex[eE])),)
STUBIFY_EXE = mfxdjstubify -v -s $(DJDIR)/bin/cwsdstub.exe $(upx_exe)
endif
endif
endif	# djgpp2


###
### cygwin / mingw32
###

ifeq ($(target),cygwin)
include $(srcdir)/Makedefs.gcc
e = .exe
CCARCH += -march=i386 -mcpu=i686
endif

ifeq ($(target),mingw32)
include $(srcdir)/Makedefs.gcc
e = .exe
CCARCH += -mno-cygwin -march=i386 -mcpu=i686
endif

# mingw32 as included in cygwin
ifeq ($(target),no-cygwin)
include $(srcdir)/Makedefs.gcc
e = .exe
CCARCH += -mno-cygwin -march=i386 -mcpu=i686
endif

# mingw32 cross compiler
ifeq ($(target),cross-mingw32)
include $(srcdir)/Makedefs.gcc
e = .exe
CC = i586-mingw32msvc-gcc
CXX = i586-mingw32msvc-g++
CCARCH += -march=i386 -mcpu=i686
endif


###
### Borland C++ 5.5.1
###

ifeq ($(target),bcc)
o = .obj
a = .lib
e = .exe
CC = bcc32 -q
CFLAGS = -w -w-aus -g1
CXXFLAGS_2 = $(CXXFLAGS) -x- -xd- -RT-
CFLAGS_OUTPUT = -o$@
LDFLAGS =
LDLIBS = $(DOS_LDLIBS) zlib114.lib

ifneq ($(strip $(DOS_LIBDIRS)),)
LIB := $(DOS_LIBDIRS);$(LIB)
endif
export LIB

ifeq ($(DEBUG),1)
  CFLAGS += -Od -d
else
  CFLAGS += -O2 -d
endif

CXX_LINK_EXE  = $(CXXLD) $(CXXLDARCH) $(LDFLAGS) -e$@ $^ $(LDLIBS)

endif	# bcc


###
### Digital Mars C++ 8.32
###

ifeq ($(target),dmc)
o = .obj
a = .lib
e = .exe
CC = dmc -mn
CFLAGS = -w- -wx
CXXFLAGS = $(CFLAGS) -Aa -Ab
CXXFLAGS_1 = $(CXXFLAGS) -Ae -Ar
CFLAGS_OUTPUT = -o$@
LDFLAGS =
LDLIBS = $(DOS_LDLIBS) zlib114.lib

ifneq ($(strip $(DOS_LIBDIRS)),)
LIB := $(DOS_LIBDIRS);$(LIB)
endif
export LIB

ifeq ($(DEBUG),1)
  CFLAGS += -o- -g -s
else
  CFLAGS += -o
endif

CXX_LINK_EXE  = $(CXXLD) $(CXXLDARCH) $(LDFLAGS) -o$@ $^ $(LDLIBS)

endif	# dmc


###
### Visual C++ 6 (aka Microsoft C/C++ Compiler 12.00)
###

ifeq ($(target),vc6)
o = .obj
a = .lib
e = .exe
CC = cl -nologo
CFLAGS = -W4 -WX
CXXFLAGS_1 = $(CXXFLAGS) -EHac -GR
LDFLAGS =
LINK_LDFLAGS = /link /map:$T.map

ifneq ($(strip $(DOS_LIBDIRS)),)
LIB := $(DOS_LIBDIRS);$(LIB)
endif
export LIB

ifeq (1,2)
  # statically link libc.lib
  CC += -ML
  LDLIBS = $(u)_s.lib zlib114.lib setargv.obj
else
  # link against msvcrt.dll
  CC += -MD
  LDLIBS = $(DOS_LDLIBS) zlib114.lib setargv.obj
endif
ifeq ($(DEBUG),1)
  CFLAGS += -Od -ZI
  LINK_LDFLAGS += /debug
else
  CFLAGS += -O2 -GF
  LINK_LDFLAGS += /release
endif
RESOURCES = upx.res

##LINK_LDFLAGS += /verbose
CXX_LINK_EXE  = $(CXXLD) $(CXXLDARCH) $(LDFLAGS) -Fe$@ $^ $(LDLIBS) $(LINK_LDFLAGS)

endif	# vc6


###
### Watcom C++ 11.0c
###

ifeq ($(target),wcc)
o = .obj
a = .lib
e = .exe
CC = wcl386 -zq -bt=nt -mf -5r
CFLAGS = -zc -w5 -we
CXXFLAGS_1 = $(CXXFLAGS) -xs -xr
CFLAGS_OUTPUT = -fo=$@
LDFLAGS = -k0x100000 -fm
LDLIBS = $(DOS_LDLIBS) zlib114.lib

INCLUDES:=$(strip $(subst /,\\,$(INCLUDES)))
ifeq (1,2)
  WCL386:=$(INCLUDES)
  export WCL386
  INCLUDES:=
endif

ifneq ($(strip $(DOS_LIBDIRS)),)
LIB := $(DOS_LIBDIRS);$(LIB)
endif
export LIB

ifeq ($(DEBUG),1)
  CFLAGS +=
else
  # note: it seems that the optimizer is generating bad code with `-ox'
  CFLAGS += -olr
endif

CXX_LINK_EXE  = $(CXXLD) $(CXXLDARCH) $(LDFLAGS) -fe=$@ $^ $(LDLIBS)

endif	# wcc


# /***********************************************************************
# // malloc debuggers and memory checkers - somewhat obsolete, use valgrind
# ************************************************************************/

ifeq (1,2)
  # compile in extra valgrind support
  DEFS += -DWITH_VALGRIND
endif

ifeq (1,2)
  LDLIBS += -lefence
endif

ifeq (1,2)
  DEFS += -DWITH_DMALLOC
  LDLIBS += -ldmalloc
endif

ifeq (1,2)
  DEFS += -DWITH_GC -DLINUX_THREADS -D_REENTRANT
  LDLIBS += -lgc -lpthread
  # only needed when using -static:
  ##LDFLAGS += -Wl,-defsym,_DYNAMIC=0
endif


# /***********************************************************************
# // finish settings
# ************************************************************************/

# extra flags
DEFS       += $(EXTRA_DEFS)
CFLAGS     += $(EXTRA_CFLAGS)
CFLAGS_W   += $(EXTRA_CFLAGS_W)
CFLAGS_M   += $(EXTRA_CFLAGS_M)
CFLAGS_O   += $(EXTRA_CFLAGS_O)
CXXFLAGS_W += $(EXTRA_CXXFLAGS_W)
CXXFLAGS_M += $(EXTRA_CXXFLAGS_M)
CXXFLAGS_O += $(EXTRA_CXXFLAGS_O)
CXXFLAGS   += $(EXTRA_CXXFLAGS)
CXXFLAGS_1 += $(EXTRA_CXXFLAGS_1)
CXXFLAGS_2 += $(EXTRA_CXXFLAGS_2)
LDFLAGS    += $(EXTRA_LDFLAGS)
LDLIBS     += $(EXTRA_LDLIBS)


# To better deal with asynchronous exceptions we compile all source
# files with exception handling and RTTI enabled - the size overhead
# is negligible.
CXXFLAGS_2 = $(CXXFLAGS_1)


# /***********************************************************************
# // main targets
# ************************************************************************/

all: $(upx_exe)

.PHONY: all mostlyclean clean distclean maintainer-clean untabify tags

$(upx_exe): $(OBJECTS) $(LIBS) $(RESOURCES)
	$(CXX_LINK_EXE)
	$(STUBEDIT_EXE)
	$(STUBIFY_EXE)
	$(CHMOD_EXE)


mostlyclean:
	-rm -f *.d *.err *.i *.log *.map *~ gdb-trans*

clean: mostlyclean
	-rm -f *.a *.lib *.o *.obj *.res *.tds tags TAGS ID
	-rm -f *.idb *.pdb
	-rm -f $(upx_exe) upx upx.exe upx.ttp upx_nrv upx_nrv.exe upx_nrv.ttp upx_ucl upx_ucl.exe upx_ucl.ttp
	-rm -rf .deps

distclean: clean

maintainer-clean: distclean


untabify:
	mfxtu -d4 -t *.h *.cpp *.ch
	mfxtu -d8 -t stub/[ln]*.asm stub/*.ash stub/*.[cs]

tags TAGS:
	ctags *.h *.cpp *.ch

ID:
	mkid *.h *.cpp *.ch


# /***********************************************************************
# // rules
# ************************************************************************/

%$o : %.c
	$(CC_COMPILE)

%$o : %.cpp
	$(CXX_COMPILE_1)

ifneq ($(strip $(OBJECTS2)),)
$(OBJECTS2): %$o : %.cpp
	$(CXX_COMPILE_2)
endif

%.res : %.rc
	rc -l 0x409 -fo$@ $<


# /***********************************************************************
# // dependencies
# ************************************************************************/

-include $(srcdir)/Makedefs.dep

# vi:nowrap
