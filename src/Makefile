#
# UPX Makefile (GNU make) - works with unix/win32/djgpp2
#
# usage:
#   `make target=linux'             # linux
#   `make target=djggp2'            # dos32 - djggp2 2.03
#   `make target=cygwin'            # win32 - cygwin 1.3.x
#   `make target=mingw32'           # win32 - mingw32
#   `make target=no-cygwin'         # win32 - mingw32 as included in cygwin 1.3.x
#   `make target=bc'                # win32 - Borland C++ 5.5.1
#   `make target=dm'                # win32 - Digital Mars C++ 8.33
#   `make target=ic'                # win32 - Intel C++ 7.0
#   `make target=vc'                # win32 - Visual C++ 6
#   `make target=wc'                # win32 - Watcom C++ 11.0c
#   `make target=cross-m68k-linux'  # m68k-linux cross compiler
#   `make target=cross-mingw32'     # i586-mingw32msvc cross compiler
#   `make target=cross-mint'        # m68k-atari-mint cross compiler
#


# configuration section

ifeq ($(strip $(UCLDIR)),)
# change this to reflect where the UCL library is
UCLDIR = $(HOME)/local/src/ucl-1.01
endif

## you can set the default target here
##target =

DEBUG = 0

srcdir = .
top_srcdir = ..


##### You should not have to change anything below this line. #####


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
# // main
# ************************************************************************/

override WITH_ZLIB = 1

include $(srcdir)/Makedefs.upx


# /***********************************************************************
# // dependencies
# ************************************************************************/

-include $(srcdir)/Makedefs.dep

# vi:nowrap
