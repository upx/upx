#
# UPX Makefile (GNU make) - works with unix/win32/djgpp2
#
# usage:
#   `make target=linux'             # linux
#   `make target=dos32-djggp2'      # dos32 - djggp2
#   `make target=win32-cygwin'      # win32 - cygwin
#   `make target=win32-mingw32'     # win32 - mingw32
#   `make target=win32-rsxnt'       # win32 - rsxnt
#   `make target=win32-bc'          # win32 - Borland C++
#   `make target=win32-dm'          # win32 - Digital Mars C++
#   `make target=win32-ic'          # win32 - Intel C++
#   `make target=win32-vc'          # win32 - Visual C++
#   `make target=win32-wc'          # win32 - Watcom C++
#   `make target=cross-amd64-linux' # x86_64-linux cross compiler
#   `make target=cross-mingw32'     # i586-mingw32msvc cross compiler
#   `make target=cross-mint'        # m68k-atari-mint cross compiler
#   `make target=gcc'               # generic gcc
#
# you can also set the `UPX_MAKE_TARGET' environment variable.
#


# configuration section

ifeq ($(strip $(UCLDIR)),)
# change this to reflect where the UCL library is
UCLDIR = $(HOME)/local/src/ucl-1.02
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

include $(srcdir)/Makefile.bld


# /***********************************************************************
# // dependencies
# ************************************************************************/

include $(srcdir)/Makedefs.dep

# vi:nowrap
