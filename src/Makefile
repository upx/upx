# UPX Makefile (GNU make) - works with djgpp2/win32/unix
#
# usage:
#   `make target=linux'         # linux
#   `make target=djggp2'        # djggp2 2.03
#   `make target=cygwin'        # cygwin 1.1.x
#   `make target=mingw32'       # mingw32
#   `make target=no-cygwin'     # mingw32 as included in cygwin 1.1.x
#   `make target=vc6'           # Visual C++ 6.0
#


# configuration section

ifeq ($(strip $(UCLDIR)),)
# change this to reflect where the UCL library is
UCLDIR = $(HOME)/local/src/ucl-0.92
endif

DEBUG = 0


# -------------------------------------------------------
# You should not have to change anything below this line.
# -------------------------------------------------------

SHELL = /bin/sh


.SUFFIXES:
.SUFFIXES: .a .c .cpp .exe .lib .o .obj


srcdir = .
top_srcdir = ..


# auto-detect the target unless given on the commandline
target = djgpp2
ifneq ($(strip $(wildcard /usr/include/linux)),)
target = linux
endif
ifneq ($(strip $(wildcard /platform/sun4?/kernel/unix)),)
target = sparc
endif
ifeq ($(target),msc)
override target = vc6
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
	packer$o packhead$o packmast$o \
	p_com$o p_djgpp2$o p_elks$o p_exe$o \
	p_lx_elf$o p_lx_exc$o p_lx_sep$o p_lx_sh$o \
	p_sys$o p_tmt$o p_tos$o \
	p_unix$o p_vmlinz$o p_w16ne$o p_w32pe$o p_wcle$o

# no exceptions or RTTI
OBJECTS2 = \
	filteri$o help$o main$o mygetopt$o util$o \
	c_init$o c_file$o c_none$o c_screen$o \
	s_object$o s_djgpp2$o s_vcsa$o s_win32$o

# pure C sources
OBJECTS3 =

OBJECTS = $(OBJECTS1) $(OBJECTS2) $(OBJECTS3)


# /***********************************************************************
# // compiler settings
# ************************************************************************/

# default to a gcc unix-type compiler
CC = gcc
CXX = $(CC)
DEFS =
INCLUDES = -I. -I$(srcdir)
CFLAGS_OUTPUT = -o $@
CXXFLAGS_OUTPUT = $(CFLAGS_OUTPUT)

LINK_EXE = $(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)
STUBEDIT_EXE =
STUBIFY_EXE =

o = .o
a = .a
e = .exe


###
### gcc defaults
###

##CFLAGS_O  = -Os
CFLAGS_O  = -O2
##CFLAGS_WERROR = -Werror
CFLAGS_W  = $(CFLAGS_WERROR)
CFLAGS_W += -Wall -W -Wcast-align -Wcast-qual -Wmissing-declarations -Wmissing-prototypes -Wshadow -Wwrite-strings
##CFLAGS_M  = -fno-builtin
## CFLAGS_M += -malign-functions=0 -malign-jumps=0 -malign-loops=0

CFLAGS    = $(CFLAGS_W) $(CFLAGS_O) $(CFLAGS_M)
CFLAGS   += -Wp,-MMD,.deps/$(*F).pp
CXXFLAGS  = $(CFLAGS) -Wsynth -fconserve-space
CXXFLAGS1 = $(CXXFLAGS)
##CXXFLAGS1 += -fasynchronous-exceptions
CXXFLAGS2 = $(CXXFLAGS) -fno-exceptions -fno-rtti

ifeq ($(DEBUG),1)
DEFS += -DDEBUG
## DEFS += -DTESTING
endif

ifeq ($(DEBUG),1)
LDFLAGS = -g
else
LDFLAGS = -s
endif
LDFLAGS += -Wl,-Map,$T.map
LDLIBS =
LDLIBS = -lz
LIBDIRS =
DOS_LIBDIRS =


###
### compression library
###

UCLDIR:=$(strip $(subst \,/,$(UCLDIR)))
NRVDIR:=$(strip $(subst \,/,$(NRVDIR)))
u = ucl
U = UCL
upx_exe = upx$e
include $(srcdir)/Makefile.inc
ifneq ($(strip $(wildcard $(NRVDIR)/include/nrv)),)
  u = nrv
  U = NRV
  upx_exe = upx_$u$e
  include $(srcdir)/Makefile.inc
endif

tmp := -Wl,--rpath,
LDRPATH := $(addprefix $(tmp),$(LIBDIRS))
LIBDIRS += .
LDLIBDIRS := $(addprefix -L,$(LIBDIRS))

##LDFLAGS += $(LDRPATH)
LDFLAGS += $(LDLIBDIRS)


###
### linux/i386
###

ifeq ($(target),linux)
e =
###CC = /usr/local/packages/gcc-cvs/bin/g++
DEFS += '-DUPX_CONFIG_H="config_h/linux.h"'
##CFLAGS_M += -mno-schedule-prologue
CFLAGS_M += -march=i386 -mcpu=pentium
CFLAGS_WERROR = -Werror
LDLIBS += -lmcheck

ifeq (1,2) 	# checkergcc
  CC = checkergcc
  CXX = checkerg++
else
ifeq ($(DEBUG),1)
  ##CFLAGS += -O0 -gstabs+3
  ##CFLAGS += -O0 -gdwarf-2
  CFLAGS += -O0 -g
else
  ##LDFLAGS += -static
  STUBEDIT_EXE = objcopy -S -R .comment -R .note $@ && perl $(srcdir)/stub/scripts/brandelf.pl $@ && chmod 755 $@
endif
endif

endif	# linux


###
### djgpp2
###

ifeq ($(target),djgpp2)
CFLAGS_M += -mno-schedule-prologue
CFLAGS_M += -march=i386 -mcpu=pentium
CFLAGS_WERROR = -Werror
STUBEDIT_EXE = stubedit $@ bufsize=0xfc00
ifneq ($(strip $(wildcard $(DJDIR)/bin/mfxdjstubify.exe)),)
ifneq ($(strip $(wildcard $(DJDIR)/bin/cwsdstub.exe)),)
STUBIFY_EXE = mfxdjstubify -v -s $(DJDIR)/bin/cwsdstub.exe $(upx_exe)
endif
endif
endif	# djgpp2


###
### cygwin / mingw32
###

ifeq ($(target),cygwin)
CFLAGS_M += -mno-schedule-prologue
CFLAGS_M += -march=i386 -mcpu=pentium
CFLAGS_WERROR = -Werror
endif

ifeq ($(target),mingw32)
CFLAGS_M += -mno-schedule-prologue
CFLAGS_M += -march=i386 -mcpu=pentium
endif

# mingw32 as included in cygwin
ifeq ($(target),no-cygwin)
CC = gcc -mno-cygwin
CFLAGS_M += -mno-schedule-prologue
CFLAGS_M += -march=i386 -mcpu=pentium
CFLAGS_WERROR = -Werror
endif


###
### Microsoft 32-bit C/C++ Compiler 12.00 (aka Visual C++ 6)
###

ifeq ($(target),vc6)
o = .obj
a = .lib
CC = cl -nologo
CFLAGS = -W4 -WX
CXXFLAGS1 = $(CFLAGS) -GR -GX -EHa
CXXFLAGS2 = $(CFLAGS)
LDFLAGS =
LINK_LDFLAGS = /link /map:$T.map

ifneq ($(strip $(DOS_LIBDIRS)),)
LIB := $(DOS_LIBDIRS);$(LIB)
endif
export LIB

ifeq (1,2)
  # statically link libc.lib
  CC += -ML
  ##LDLIBS = $(u)_s.lib zlib_s.lib setargv.obj
  LDLIBS = $(u)_s.lib setargv.obj
else
  # link against msvcrt.dll
  CC += -MD
  LDLIBS = $(DOS_LDLIBS) zlib.lib setargv.obj
endif
ifeq ($(DEBUG),1)
  CFLAGS += -Od -ZI
  LINK_LDFLAGS += /debug
else
  CFLAGS += -O2 -GF
  LINK_LDFLAGS += /release
endif

##LINK_LDFLAGS += /verbose
LINK_EXE = $(CC) $(LDFLAGS) -Fe$@ $^ $(LDLIBS) $(LINK_LDFLAGS)

endif	# vc6


###
### sparc
###

ifeq ($(target),sparc)
e =
DEFS += '-DUPX_CONFIG_H="config_h/sparc.h"'
INCLUDES += -I/home/ethmola/local/include

ifeq (1,2) # native compiler
  CFLAGS = -O0 -g
  CXXFLAGS1 =
  CXXFLAGS2 =
  CFLAGS_M =
  DEFS += -DUSE_STDNAMESPACE
else # gcc
  CFLAGS += -O0 -gstabs+
endif

ifeq (1,2) # purify
  DEFS += -D__PURIFY__
  LDFLAGS = -g -L/home/ethmola/local/lib
  LINK_EXE = purify $(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)
else
  LDFLAGS += -g -L/home/ethmola/local/lib
  LINK_EXE = $(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)
endif

endif	# sparc


###
### Borland C++ 5.5 (DOES NOT WORK - INCOMPLETE C++ IMPLEMENTATION)
###

ifeq ($(target),bcc)
o = .obj
a = .lib
CC = bcc32
CFLAGS = -w -w-par
CXXFLAGS1 = $(CFLAGS)
CXXFLAGS2 = $(CFLAGS)
CFLAGS_OUTPUT = -o$@
LDFLAGS =
LDLIBS = $(DOS_LDLIBS) zlib.lib

ifneq ($(strip $(DOS_LIBDIRS)),)
LIB := $(DOS_LIBDIRS);$(LIB)
endif
export LIB

ifeq ($(DEBUG),1)
  CFLAGS += -Od -d
else
  CFLAGS += -O2 -d
endif

LINK_EXE = $(CC) $(LDFLAGS) -e$@ $^ $(LDLIBS)

endif	# bcc


###
### malloc debuggers and memory checkers
###

ifeq (1,2)
  LDLIBS += -lefence
endif

ifeq (1,2)
  CFLAGS += -DWITH_DMALLOC
  LDLIBS += -ldmalloc
endif

ifeq (1,2)
  CFLAGS += -DWITH_GC -DLINUX_THREADS -D_REENTRANT
  LDLIBS += -lgc -lpthread
  # only needed when using -static:
  ##LDFLAGS += -Wl,-defsym,_DYNAMIC=0
endif

ifeq (1,2)
  CFLAGS += -DWITH_MSS
  LDLIBS += -lmss
endif


###
### extra flags
###

CFLAGS    += $(EXTRA_CFLAGS)
CXXFLAGS1 += $(EXTRA_CXXFLAGS1)
CXXFLAGS2 += $(EXTRA_CXXFLAGS2)
LDFLAGS   += $(EXTRA_LDFLAGS)
LDLIBS    += $(EXTRA_LDLIBS)


# /***********************************************************************
# // main targets
# ************************************************************************/

all: $(upx_exe)

.PHONY: all unupx mostlyclean clean distclean maintainer-clean untabify tags

$(upx_exe): $(OBJECTS) $(LIBS)
	$(LINK_EXE)
	$(STUBEDIT_EXE)
	$(STUBIFY_EXE)


unupx:
	$(MAKE) target=vc6 unupx.dll

unupx.dll: $(OBJECTS) $(LIBS)
	$(LINK_DLL)


mostlyclean:
	-rm -f *.d *.err *.i *.log *.map *~ gdb-trans*

clean: mostlyclean
	-rm -f *.a *.lib *.o *.obj tags TAGS ID
	-rm -f *.idb *.pdb
	-rm -rf .deps
	-rm -f upx upx.exe upx_nrv upx_nrv.exe upx_ucl upx_ucl.exe

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

.c$o:
	$(CC) $(DEFS) $(INCLUDES) $(CFLAGS) $(CFLAGS_OUTPUT) -c $<

.cpp$o:
	$(CXX) $(DEFS) $(INCLUDES) $(CXXFLAGS1) $(CXXFLAGS_OUTPUT) -c $<

$(OBJECTS1): %$o : %.cpp
	$(CXX) $(DEFS) $(INCLUDES) $(CXXFLAGS1) $(CXXFLAGS_OUTPUT) -c $<

$(OBJECTS2): %$o : %.cpp
	$(CXX) $(DEFS) $(INCLUDES) $(CXXFLAGS2) $(CXXFLAGS_OUTPUT) -c $<

ifneq ($(strip $(OBJECTS3)),)
$(OBJECTS3): %$o : %.c
	$(CC) $(DEFS) $(INCLUDES) $(CFLAGS) $(CFLAGS_OUTPUT) -c $<
endif


# /***********************************************************************
# // dependencies
# ************************************************************************/

ifeq ($(strip $(wildcard .deps)),)
  DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1 || :)
else
  DEP_FILES := $(wildcard .deps/*.pp)
  ifneq ($(strip $(DEP_FILES)),)
    include $(DEP_FILES)
  endif
endif


# vi:nowrap
