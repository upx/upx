# UPX Makefile (GNU make) - works with djgpp2/win32/unix
#
# usage:
#   `make target=linux'         # linux
#   `make target=djggp2'        # djggp2
#   `make target=mingw32'       # mingw32
#   `make target=no-cygwin'     # mingw32 as of cygwin b20.1
#   `make target=msc'           # Visual C++ 6.0
#


# configuration section

ifeq ($(strip $(UCLDIR)),)
# change this to reflect where the UCL library is
UCLDIR = $(HOME)/local/src/ucl-0.90
endif

DEBUG = 1


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


# /***********************************************************************
# // object files
# ************************************************************************/

# these use exceptions & RTTI
OBJECTS1 = \
	compress$o except$o file$o lefile$o \
	filter$o mem$o msg$o stdcxx$o work$o ui$o \
	packer$o packhead$o packmast$o \
	p_com$o p_djgpp2$o p_exe$o p_lx_elf$o p_lx_sep$o p_lx_sh$o \
	p_sys$o p_tmt$o p_tos$o \
	p_unix$o p_vmlinux$o p_w32pe$o p_wcle$o

# no exceptions or RTTI
OBJECTS2 = \
	filteri$o help$o main$o mygetopt$o util$o linker$o \
	c_init$o c_file$o c_none$o c_screen$o \
	s_object$o s_djgpp2$o s_vcsa$o s_win32$o

# pure C sources
OBJECTS3 =

OBJECTS = $(OBJECTS1) $(OBJECTS2) $(OBJECTS3)


# /***********************************************************************
# // compiler settings
# ************************************************************************/

# default to a unix-type compiler
CC = gcc
CXX = $(CC)
DEFS =
INCLUDES = -I$(srcdir)
CFLAGS_OUTPUT = -o $@
CXXFLAGS_OUTPUT = $(CFLAGS_OUTPUT)

LINK_EXE = $(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)
STUBEDIT_EXE =

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
CXXFLAGS  = $(CFLAGS) -Wsynth -fconserve-space
CXXFLAGS1 = $(CXXFLAGS)
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
LDFLAGS += -Wl,-Map,$(basename $@).map
LDLIBS = -lz
LIBDIRS =


###
### compression library
###

UCLDIR:=$(strip $(subst \,/,$(UCLDIR)))
NRVDIR:=$(strip $(subst \,/,$(NRVDIR)))
ifeq ($(strip $(wildcard $(NRVDIR)/include/nrv)),)
  u = ucl
  U = UCL
  upx_exe = upx$e
else
  u = nrv
  U = NRV
  upx_exe = upx_$u$e
endif
UDIR := $($(U)DIR)

DEFS += -DWITH_$(U)
ifneq ($(strip $(wildcard $(UDIR)/include)),)
INCLUDES += -I$(UDIR)/include
endif
ifneq ($(strip $(wildcard $(UDIR)/src/.libs)),)
LIBDIRS += $(UDIR)/src/.libs
endif
ifeq ($(DEBUG),1)
ifneq ($(strip $(wildcard $(UDIR)/build/debug/src/.libs)),)
LIBDIRS += $(UDIR)/build/debug/src/.libs
endif
endif
ifneq ($(strip $(wildcard $(UDIR)/build/release/src/.libs)),)
LIBDIRS += $(UDIR)/build/release/src/.libs
endif
ifneq ($(strip $(wildcard $(UDIR)/build/src/.libs)),)
LIBDIRS += $(UDIR)/build/src/.libs
endif
ifneq ($(strip $(wildcard $(UDIR))),)
LIBDIRS += $(UDIR)
endif

tmp := -Wl,--rpath,
LDRPATH := $(addprefix $(tmp),$(LIBDIRS))
LIBDIRS += .
LDLIBDIRS := $(addprefix -L,$(LIBDIRS))

##LDFLAGS += $(LDRPATH)
LDFLAGS += $(LDLIBDIRS)
LDLIBS += -l$(u)


###
### linux
###

ifeq ($(target),linux)
e =
DEFS += '-DUPX_CONFIG_H="config_h/linux.h"'
CFLAGS_M += -mno-schedule-prologue
CFLAGS_M += -march=i386 -mcpu=pentium
CFLAGS_WERROR = -Werror
LDLIBS += -lmcheck

ifeq (1,2) 	# checkergcc
  CC = checkergcc
  CXX = checkerg++
else
ifeq ($(DEBUG),1)
  ##CFLAGS += -O0 -gstabs+3
  CFLAGS += -O0 -gstabs+3
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
endif	# djgpp2


###
### cygwin / mingw32
###

ifeq ($(target),cygwin)
CFLAGS_M += -mno-schedule-prologue
CFLAGS_M += -march=i386 -mcpu=pentium
endif

ifeq ($(target),mingw32)
CFLAGS_M += -mno-schedule-prologue
CFLAGS_M += -march=i386 -mcpu=pentium
endif

# mingw32 as included in cygwin b20.1
ifeq ($(target),no-cygwin)
CC = gcc -mno-cygwin
CFLAGS_M += -mno-schedule-prologue
CFLAGS_M += -march=i386 -mcpu=pentium
endif


###
### Microsoft 32-bit C/C++ Compiler 12.00 (aka Visual C++ 6)
###

ifeq ($(target),msc)
o = .obj
a = .lib
CC = cl -nologo
CFLAGS = -W4
CXXFLAGS1 = $(CFLAGS) -GX -GR
CXXFLAGS2 = $(CFLAGS)
LDFLAGS =
LINK_LDFLAGS = /link /map:$(basename $@).map

ifneq ($(strip $(wildcard $(UDIR))),)
LIB := $(UDIR);$(LIB)
endif
export LIB

ifeq (1,2)
  # statically link libc
  CC += -ML
  LDLIBS = $(u)_s.lib zlib_s.lib setargv.obj
else
  # link against msvcrt.dll
  CC += -MD
  LDLIBS = $(u).lib zlib.lib setargv.obj
endif
ifeq ($(DEBUG),1)
  CFLAGS += -Od -ZI
  LINK_LDFLAGS += /debug
else
  CFLAGS += -O2 -Gs -GF
  LINK_LDFLAGS += /release
endif

##LINK_LDFLAGS += /verbose
LINK_EXE = $(CC) $(LDFLAGS) -Fe$@ $^ $(LDLIBS) $(LINK_LDFLAGS)

endif	# msc


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
LDLIBS = $(u).lib zlib.lib

ifneq ($(strip $(wildcard $(UDIR))),)
LIB := $(UDIR);$(LIB)
endif
export LIB

ifeq ($(DEBUG),1)
  CFLAGS +=
else
  CFLAGS += -O2
endif

LINK_EXE = $(CC) $(LDFLAGS) -e$@ $^ $(LDLIBS)

endif	# bcc


###
### malloc debuggers (Linux only)
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


# /***********************************************************************
# // main targets
# ************************************************************************/

all: $(upx_exe)

.PHONY: all unupx mostlyclean clean distclean maintainer-clean untabify tags

$(upx_exe): $(OBJECTS) $(LIBS)
	$(LINK_EXE)
	$(STUBEDIT_EXE)

unupx:
	$(MAKE) target=msc unupx.dll

unupx.dll: $(OBJECTS) $(LIBS)
	$(LINK_DLL)


mostlyclean:
	-rm -f *.d *.err *.i *.log *.map *~ gdb-trans*

clean: mostlyclean
	-rm -f *.a *.lib *.o *.obj tags TAGS ID
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

# FIXME: use automated dependencies

main$o:     mygetopt.h version.h
filter$o:   filter.h
filteri$o:  filter.h fcto_ml.ch fcto_ml2.ch
help$o:     version.h
msg$o:      ui.h
lefile$o:   lefile.h
linker$o:   linker.h
mygetopt$o: mygetopt.h
packer$o:   packer.h filter.h linker.h ui.h version.h
packhead$o: packer.h
packmast$o: packmast.h packer.h lefile.h \
		p_com.h p_djgpp2.h p_exe.h p_lx_elf.h p_lx_sep.h p_lx_sh.h \
		p_sys.h p_tmt.h p_tos.h \
		p_unix.h p_vxd.h p_w32pe.h p_wcle.h
ui$o:       packer.h ui.h
work$o:     packer.h ui.h packmast.h

p_com$o:    packer.h p_com.h           stub/l_com.h
p_djgpp2$o: packer.h p_djgpp2.h stub/stubify.h \
		stub/l_djgpp2.h
p_exe$o:    packer.h p_exe.h           stub/l_exe.h
p_lx_elf$o: packer.h p_lx_elf.h p_unix.h p_elf.h \
		stub/l_le_n2b.h stub/l_le_n2d.h
p_lx_sep$o: packer.h p_lx_sep.h
p_lx_sh$o:  packer.h p_lx_elf.h p_unix.h p_elf.h \
		stub/l_sh_n2b.h stub/l_sh_n2d.h
p_sys$o:    packer.h p_sys.h p_com.h   stub/l_sys.h
p_tmt$o:    packer.h p_tmt.h \
		stub/l_tmt.h
p_tos$o:    packer.h p_tos.h \
		stub/l_t_n2b.h stub/l_t_n2bs.h \
		stub/l_t_n2d.h stub/l_t_n2ds.h
p_unix$o:   packer.h p_unix.h p_elf.h \
		stub/l_lx_n2b.h stub/l_lx_n2d.h
p_vmlinux$o: packer.h p_unix.h p_elf.h \
		stub/l_lx_n2b.h stub/l_lx_n2d.h
p_w32pe$o:  packer.h p_w32pe.h \
		stub/l_w32pe.h
p_wcle$o:   packer.h p_wcle.h lefile.h \
		stub/l_wcle.h

# vi:nowrap
