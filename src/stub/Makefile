#
# UPX stub Makefile - needs GNU make 3.81 or better
#
# You also will need a number of special build tools like various
# cross-assemblers and cross-compilers - please see README.SRC
# for details.
# And see misc/rebuild-stubs-with-podman how to use Podman/Docker.
#
# Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
#

MAKEFLAGS += -rR
.SUFFIXES:
.SECONDEXPANSION:
.NOTPARALLEL:
export LC_ALL = C
export SHELL = /bin/sh

# internal make variables
comma := ,
empty :=
space := $(empty) $(empty)
tab   := $(empty)	$(empty)

ifneq ($(findstring $(firstword $(MAKE_VERSION)),3.77 3.78 3.78.1 3.79 3.79.1 3.80),)
$(error GNU make 3.81 or better is required)
endif

ifndef srcdir
srcdir := $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))
srcdir := $(shell echo '$(srcdir)' | sed 's,/*$$,,')
endif
ifndef top_srcdir
top_srcdir := $(srcdir)/../..
endif
include $(wildcard $(top_srcdir)/Makevars.global ./Makevars.local)

# update $PATH for our special stub build tools
ifneq ($(wildcard $(HOME)/local/bin/bin-upx/upx-stubtools-check-version),)
export PATH := $(HOME)/local/bin/bin-upx:$(PATH)
else ifneq ($(wildcard $(HOME)/.local/bin/bin-upx/upx-stubtools-check-version),)
export PATH := $(HOME)/.local/bin/bin-upx:$(PATH)
else ifneq ($(wildcard $(HOME)/bin/bin-upx/upx-stubtools-check-version),)
export PATH := $(HOME)/bin/bin-upx:$(PATH)
endif


# /***********************************************************************
# //
# ************************************************************************/

ifndef STUBS
STUBS += amd64-darwin.dylib-entry.h
STUBS += amd64-darwin.macho-entry.h
STUBS += amd64-darwin.macho-fold.h
STUBS += amd64-darwin.macho-upxmain.h
STUBS += amd64-linux.elf-entry.h
STUBS += amd64-linux.elf-so_entry.h
STUBS += amd64-linux.elf-fold.h
STUBS += amd64-linux.elf-so_fold.h
STUBS += amd64-linux.kernel.vmlinux-head.h
STUBS += amd64-linux.kernel.vmlinux.h
STUBS += amd64-linux.shlib-init.h
STUBS += amd64-win64.pep.h
STUBS += arm.v4a-linux.elf-entry.h
STUBS += arm.v4a-linux.elf-fold.h
STUBS += arm.v4a-wince.pe.h
STUBS += arm.v4t-wince.pe.h
STUBS += arm.v5a-darwin.macho-entry.h
STUBS += arm.v5a-darwin.macho-fold.h
STUBS += arm.v5a-linux.elf-entry.h
STUBS += arm.v5a-linux.elf-fold.h
STUBS += arm.v5a-linux.kernel.vmlinux-head.h
STUBS += arm.v5a-linux.kernel.vmlinux.h
STUBS += arm.v5a-linux.kernel.vmlinuz-head.h
STUBS += arm.v5a-linux.kernel.vmlinuz.h
STUBS += arm.v5a-linux.shlib-init.h
STUBS += arm.v5t-linux.shlib-init.h
STUBS += armeb.v4a-linux.elf-entry.h
STUBS += armeb.v4a-linux.elf-fold.h
STUBS += armeb.v5a-linux.kernel.vmlinux-head.h
STUBS += armeb.v5a-linux.kernel.vmlinux.h
STUBS += arm64-linux.elf-entry.h
STUBS += arm64-linux.elf-fold.h
STUBS += arm64-linux.shlib-init.h
STUBS += arm64-darwin.macho-entry.h
STUBS += arm64-darwin.macho-fold.h
STUBS += i086-dos16.com.h
STUBS += i086-dos16.exe.h
STUBS += i086-dos16.sys.h
STUBS += i386-bsd.elf-entry.h
STUBS += i386-bsd.elf-fold.h
STUBS += i386-bsd.elf.execve-entry.h
STUBS += i386-bsd.elf.execve-fold.h
STUBS += i386-darwin.dylib-entry.h
# STUBS += i386-darwin.dylib-fold.h
STUBS += i386-darwin.macho-entry.h
STUBS += i386-darwin.macho-fold.h
STUBS += i386-darwin.macho-upxmain.h
STUBS += i386-dos32.djgpp2-stubify.h
STUBS += i386-dos32.djgpp2.h
STUBS += i386-dos32.tmt.h
STUBS += i386-dos32.watcom.le.h
STUBS += i386-linux.elf-entry.h
STUBS += i386-linux.elf-fold.h
STUBS += i386-linux.elf.execve-entry.h
STUBS += i386-linux.elf.execve-fold.h
STUBS += i386-linux.elf.interp-entry.h
STUBS += i386-linux.elf.interp-fold.h
STUBS += i386-linux.elf.shell-entry.h
STUBS += i386-linux.elf.shell-fold.h
STUBS += i386-linux.kernel.vmlinux-head.h
STUBS += i386-linux.kernel.vmlinux.h
STUBS += i386-linux.kernel.vmlinuz.h
STUBS += i386-linux.shlib-init.h
STUBS += i386-netbsd.elf-entry.h
STUBS += i386-netbsd.elf-fold.h
STUBS += i386-openbsd.elf-fold.h
STUBS += i386-win32.pe.h
STUBS += m68k-atari.tos.h
STUBS += mips.r3000-linux.elf-entry.h
STUBS += mips.r3000-linux.elf-fold.h
STUBS += mips.r3000-linux.shlib-init.h
STUBS += mipsel.r3000-linux.elf-entry.h
STUBS += mipsel.r3000-linux.elf-fold.h
STUBS += mipsel.r3000-linux.shlib-init.h
STUBS += mipsel.r3000-ps1.h
STUBS += powerpc-darwin.dylib-entry.h
STUBS += powerpc-darwin.macho-entry.h
STUBS += powerpc-darwin.macho-fold.h
STUBS += powerpc-darwin.macho-upxmain.h
STUBS += powerpc-linux.elf-entry.h
STUBS += powerpc-linux.elf-fold.h
STUBS += powerpc-linux.kernel.vmlinux-head.h
STUBS += powerpc-linux.kernel.vmlinux.h
STUBS += powerpc64le-linux.elf-entry.h
STUBS += powerpc64le-linux.elf-fold.h
STUBS += powerpc64le-linux.kernel.vmlinux-head.h
STUBS += powerpc64le-linux.kernel.vmlinux.h
STUBS += powerpc64-darwin.dylib-entry.h
STUBS += powerpc64-darwin.macho-entry.h
STUBS += powerpc64-darwin.macho-fold.h
STUBS += powerpc64-linux.elf-entry.h
STUBS += powerpc64-linux.elf-fold.h
#STUBS += powerpc64-linux.kernel.vmlinux-head.h
#STUBS += powerpc64-linux.kernel.vmlinux.h
endif


ifndef default.targets
ifeq ($(wildcard .all-stamp),)
default.targets =
default:
	@echo "UPX info: type 'make all' if you have all the required build tools."
else
default.targets = all
default: $$(default.targets)
endif
endif


all.targets ?= .upx-stubtools-stamp tmp/.tmp-stamp .all-stamp
all: $$(all.targets)
.upx-stubtools-stamp: $(MAKEFILE_LIST)
	upx-stubtools-check-version 20221212
	@echo "timestamp" > $@
.DELETE_ON_ERROR: .upx-stubtools-stamp
%/.tmp-stamp:
	@mkdir -p $(dir $@)
	@echo "timestamp" > $@
.PRECIOUS: %/.tmp-stamp
.all-stamp: $$(STUBS)
	@echo "timestamp" > $@

ifeq ($(wildcard .all-stamp),)
mostlyclean clean: distclean
else
mostlyclean clean: maintainer-clean
endif
distclean:
	rm -f .*-stamp
	rm -rf tmp
maintainer-clean:
	rm -f .upx-stubtools-stamp
	rm -rf tmp
	rm -f $(STUBS)
list-stubs: $(STUBS)
#	@ls -l $(STUBS)
	@wc $(STUBS)

.PHONY: default all mostlyclean clean distclean maintainer-clean list-stubs


# util var for use in the rules - basename of the current target
override T = $(basename $(notdir $@))

# clear some vars, just in case
LABEL_PREFIX =
PP_FLAGS =
tc_bfdarch =
tc_bfdname =
tc_list =
tc_objdump_disasm_options =

# commands
ECHO_e     = /bin/echo -e
ECHO_E     = /bin/echo -E
PERL       = perl
PYTHON     = python2
UNIX2DOS  := $(PERL) -i -pe 's/$$/\r/;'

# trim (strip) trailing whitespace
RTRIM     := sed -e 's/[ $(tab)]*$$//'
# squeeze duplicate blank lines, delete empty first & last line
BLSQUEEZE := $(RTRIM) | cat --squeeze-blank | sed -e '1{/^$$/d}' -e '$${/^$$/d}'
# delete blank lines
BLDEL     := $(RTRIM) | sed -e '/^$$/d'
# delete blank lines at top (beginning) of file
BLDELTOP  := sed -e '/./,$$!d'
# delete blank lines at bottom (end) of file
BLDELBOT  := sed -e ':a' -e '/^\n*$$/{$$d;N;ba' -e '}'


# /***********************************************************************
# // setup toolchain globals
# ************************************************************************/

# enumerate the names of all variables that will get tested (from basename and $(tc_list))
__tc_varlist   = tc.$(basename $(notdir $@)).$1 $(foreach v,$(tc_list),tc.$v.$1)
# return the name of the first variable that is not empty
__tc_varsearch = $(firstword $(foreach v,$1,$(if $($v),$v,)))
# error sentinel for missing commands
__tc_FALSE     = false tc_FALSE: '1:$1 2:$2 3:$3 4:$4 5:$5' '$@' '$<' '$(tc_list)'
# call (expand) the first variable that is not empty
  # [move the next comment line within the "tc" macro for debugging]
  # tc debug '1:$1 2:$2 3:$3 4:$4 5:$5' '$@' '$<' '$(tc_list)'
define tc
  $(call $(call __tc_varsearch,$(call __tc_varlist,$1) __tc_FALSE),$2,$3,$4,$5)
endef

# default tools
tc.default.bin2h      = $(PYTHON) $(top_srcdir)/src/stub/scripts/bin2h.py --ident=auto-stub
##tc.default.bin2h-c    = $(call tc,bin2h) --compress=14,15,0
tc.default.bin2h-c    = $(call tc,bin2h) --compress=0
tc.default.brandelf   = $(PYTHON) $(top_srcdir)/src/stub/scripts/brandelf.py $(if $(tc_bfdname),--bfdname=$(tc_bfdname))
tc.default.gpp_inc    = $(PYTHON) $(top_srcdir)/src/stub/scripts/gpp_inc.py
tc.default.gpp_mkdep  = $(PYTHON) $(top_srcdir)/src/stub/scripts/gpp_inc.py -o /dev/null
tc.default.pp-as      = i386-linux-gcc-3.4.6 -E -nostdinc -x assembler-with-cpp -Wall
tc.default.sstrip     = sstrip-20060518
tc.default.xstrip     = $(PYTHON) $(top_srcdir)/src/stub/scripts/xstrip.py

# default multiarch-binutils
tc.default.m-ar       = multiarch-ar-2.17
tc.default.m-ld       = multiarch-ld-2.17 $(if $(tc_bfdname),-b $(tc_bfdname))
tc.default.m-nm       = multiarch-nm-2.17 $(if $(tc_bfdname),--target=$(tc_bfdname))
tc.default.m-objcopy  = multiarch-objcopy-2.17 $(if $(tc_bfdname),-F $(tc_bfdname))
tc.default.m-objdump  = multiarch-objdump-2.17 $(if $(tc_bfdname),-b $(tc_bfdname)) $(if $(tc_bfdarch),-m $(tc_bfdarch))
tc.default.m-readelf  = multiarch-readelf-2.17

# default binutils
tc.default.ld         = $(call tc,m-ld)
tc.default.nm         = $(call tc,m-nm)
tc.default.objcopy    = $(call tc,m-objcopy)
tc.default.objdump    = $(call tc,m-objdump)
tc.default.readelf    = $(call tc,m-readelf)

# default binutils functions
define tc.default.f-embed_objinfo
	chmod a-x $1
	$(call tc,objcopy) --strip-unneeded --keep-symbol=_start $1
	$(call tc,objcopy) -R .text -R .data -R .bss $1
	$(call tc,objcopy) -R .comment -R .note -R .note.GNU-stack -R .reginfo $1
	$(call tc,objdump) -htr -w $1 | $(BLSQUEEZE) | sed \
	  -e '1s/^.*: *file format/file format/' \
	  -e 's/$(tab)/ /g' \
	  -e 's/ 00*/ 0/g' \
	  -e 's/CONTENTS.*/CONTENTS/' \
	  > $1.dump
	$(call tc,xstrip) --with-dump=$1.dump $1
	cat $1.dump >> $1
endef

define tc.default.f-embed_objinfo_without_xstrip
	chmod a-x $1
	$(call tc,objcopy) -R .text -R .data -R .bss $1
	$(call tc,objcopy) -R .comment -R .note -R .note.GNU-stack -R .reginfo $1
	$(call tc,objcopy) --strip-unneeded --keep-symbol=_start $1
#
# Disassemble for human readability
# objdump
#   -Dr disassemble-all, with interspersed relocations
	$(call tc,objdump) -Dr $(tc_objdump_disasm_options) $1 | $(RTRIM) > $1.disasm
#
# Disassemble for post-processing by buildLinuxLoader()
# objdump
#   -r  reloc
#   -h  section-headers
#   -t  syms
#   -w  (wide) do not truncate symbol names
# sed
#   remove filename from first line
#   change tab to space
#   collapse multiple leading zeroes
#   remove everything after "CONTENTS"
	$(call tc,objdump) -htr -w $1 | $(BLSQUEEZE) | sed \
	  -e '1s/^.*: *file format/file format/' \
	  -e 's/	/ /g' \
	  -e 's/ 00*/ 0/g' \
	  -e 's/CONTENTS.*/CONTENTS/' \
	  > $1.dump
	cat $1.dump >> $1
endef

tc.default.f-objstrip-disasm.bin = @true
tc.default.f-objstrip-disasm.o   = $(call tc,objdump) -dr $(tc_objdump_disasm_options) $1 | $(RTRIM) > $1.disasm
tc.default.f-objstrip-disasm.obj = $(call tc,objdump) -dr $(tc_objdump_disasm_options) $1 | $(RTRIM) > $1.disasm
define tc.default.f-objstrip
	chmod a-x $1
	$(call tc,objcopy) -R .comment -R .note -R .note.GNU-stack -R .reginfo $1
	$(call tc,f-objstrip-disasm$(suffix $1),$1)
endef

# some common arch settings
tc.arch-i086.gcc      = i386-linux-gcc-3.4.6 -m32 -march=i386 -nostdinc -MMD -MT $@
tc.arch-i086.wdis     = $(WATCOM)/binl/wdis
tc.arch-i386.gcc      = i386-linux-gcc-3.4.6 -m32 -march=i386 -nostdinc -MMD -MT $@
tc.arch-i386.djasm    = djasm-20060614


# /***********************************************************************
# // amd64-darwin.dylib
# ************************************************************************/

# info: we use the tc settings from amd64-linux.elf
amd64-darwin.dylib%.h : tc_list = amd64-linux.elf default
amd64-darwin.dylib%.h : tc_bfdname = elf64-x86-64

## All code is in dylib-entry.  There is no dylib-fold, no dylib-main.
amd64-darwin.dylib-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@


# /***********************************************************************
# // amd64-darwin.macho
# ************************************************************************/

# info: we use the tc settings from amd64-linux.elf
amd64-darwin.macho%.h : tc_list = amd64-linux.elf default
amd64-darwin.macho%.h : tc_bfdname = elf64-x86-64

amd64-darwin.macho-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

amd64-darwin.macho-fold.h : tmp/$$T.o tmp/amd64-darwin.macho-main.o
	$(call tc,ld) --no-warn-mismatch --strip-all --oformat binary -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	chmod a-x tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

amd64-darwin.macho-upxmain.h: amd64-darwin.macho-upxmain.exe
	$(call tc,bin2h) $< $@

tmp/amd64-darwin.macho-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)

tmp/amd64-darwin.macho-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c -Os -fPIC $< -o $@
	$(call tc,f-objstrip,$@)


# /***********************************************************************
# // amd64-linux.elf
# ************************************************************************/

amd64-linux.elf%.h : tc_list = amd64-linux.elf default
amd64-linux.elf%.h : tc_bfdname = elf64-x86-64

tc.amd64-linux.elf.gcc  = amd64-linux-gcc-3.4.4 -fPIC -m64 -nostdinc -MMD -MT $@
tc.amd64-linux.elf.gcc += -fno-exceptions -fno-asynchronous-unwind-tables
tc.amd64-linux.elf.gcc += -Wall -W -Wcast-align -Wcast-qual -Wstrict-prototypes -Wwrite-strings -Werror

amd64-linux.elf-entry.h: $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

amd64-linux.elf-so_entry.h: $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

amd64-linux.elf-fold.h : tmp/$$T.o tmp/amd64-linux.elf-main.o $(srcdir)/src/$$T.lds tmp/amd64-expand.o
#	# FIXME: multiarch-ld-2.18 creates a huge file here, so use 2.17
#	####$(call tc,ld) --strip-all -T $(srcdir)/src/$T.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	multiarch-ld-2.17 --strip-all -T $(srcdir)/src/$T.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	$(call tc,f-objstrip,tmp/$T.bin)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

amd64-linux.elf-so_fold.h : tmp/$$T.o tmp/amd64-linux.elf-so_main.o $(srcdir)/src/$$T.lds tmp/amd64-expand.o
#	# FIXME: multiarch-ld-2.18 creates a huge file here, so use 2.17
#	####$(call tc,ld) --strip-all -T $(srcdir)/src/$T.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	multiarch-ld-2.17 -r          -T $(srcdir)/src/$T.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
#	$(call tc,f-objstrip,tmp/$T.bin)
#	$(call tc,sstrip) tmp/$T.bin
#
#	$(call tc,f-embed_objinfo,tmp/$T.bin)
# f-embed_objinfo complains:
#   File "./../../src/stub/scripts/xstrip.py", line 163, in do_file
#       assert e_shstrndx + 3 == e_shnum
# Therefore use a subset of f-embed_objinfo:
	$(call tc,f-embed_objinfo_without_xstrip,tmp/$T.bin)
#
	$(call tc,bin2h) tmp/$T.bin $@

tmp/amd64-expand.o: $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o $@

tmp/amd64-linux.elf-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)

tmp/amd64-linux.elf-so_fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o $@
	$(call tc,objcopy) -R .text $@
	$(call tc,f-objstrip,$@)

tmp/amd64-linux.elf-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c -Os $< -o $@
	$(call tc,f-objstrip,$@)

tmp/amd64-linux.elf-so_main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c -O $< -o $@
	$(call tc,objcopy) --rename-section .text=SO_MAIN -R .comment -R .note -R .note.GNU-stack -R .reginfo $@
	$(call tc,f-objstrip,$@)


# /***********************************************************************
# // amd64-linux.kernel.vmlinux
# // amd64-linux.kernel.vmlinux-head
# ************************************************************************/

# info: we use the tc settings from arch-i386 !!
amd64-linux.kernel.vmlinu%.h : tc_list = arch-i386 default
amd64-linux.kernel.vmlinu%.h : tc_bfdname = elf32-i386

amd64-linux.kernel.vmlinu%.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@

amd64-linux.kernel.vmlinux-head.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.o
	$(call tc,objcopy) --output-target binary --only-section .text tmp/$T.o tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@


# /***********************************************************************
# // amd64-linux.shlib
# ************************************************************************/

amd64-linux.shlib%.h : tc_list = amd64-linux.elf default
amd64-linux.shlib%.h : tc_bfdname = elf64-x86-64

amd64-linux.shlib-init.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@


# /***********************************************************************
# // amd64-win64.pep
# ************************************************************************/

amd64-win64.pep.h : tc_list = amd64-win64.pep default
amd64-win64.pep.h : tc_bfdname = elf64-x86-64
amd64-win64.pep.h : tc_objdump_disasm_options = -M intel-mnemonic

tc.amd64-win64.pep.gcc  = amd64-linux-gcc-4.1.1 -m64 -nostdinc -MMD -MT $@
tc.amd64-win64.pep.gcc += -fno-exceptions -fno-asynchronous-unwind-tables
tc.amd64-win64.pep.gcc += -Wall -W -Wcast-align -Wcast-qual -Wstrict-prototypes -Wwrite-strings -Werror
tc.amd64-win64.pep.objdump = multiarch-objdump-2.23.90

amd64-win64.pep.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@


# /***********************************************************************
# // arm.v5a-darwin.macho (arm.v5a)
# ************************************************************************/

# info: we use the tc settings from arm.v4a-linux.elf, but override v4 with v5
arm.v5a-darwin.macho%.h : tc_list = arm.v4a-linux.elf default
arm.v5a-darwin.macho%.h : tc_bfdname = elf32-littlearm

tc.arm.v5a-darwin.macho-entry.gcc  = arm-linux-gcc-4.1.0 -march=armv5 -nostdinc -MMD -MT $@
tc.arm.v5a-darwin.macho-fold.gcc   = arm-linux-gcc-4.1.0 -march=armv5 -nostdinc -MMD -MT $@

arm.v5a-darwin.macho-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

arm.v5a-darwin.macho-fold.h : tmp/$$T.o tmp/arm.v5a-darwin.macho-main.o
	$(call tc,ld) --no-warn-mismatch --strip-all --oformat binary -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	chmod a-x tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

tmp/arm.v5a-darwin.macho-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)

tmp/arm.v5a-darwin.macho-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c -Os $< -o $@
	$(call tc,f-objstrip,$@)


# /***********************************************************************
# // arm.v4a-linux.elf (arm.v4a)
# ************************************************************************/

arm.v4a-linux.elf%.h : tc_list = arm.v4a-linux.elf default
arm.v4a-linux.elf%.h : tc_bfdname = elf32-littlearm

tc.arm.v4a-linux.elf.gcc  = arm-linux-gcc-4.1.0 -march=armv4 -nostdinc -MMD -MT $@
tc.arm.v4a-linux.elf.gcc += -fno-exceptions -fno-asynchronous-unwind-tables
tc.arm.v4a-linux.elf.gcc += -Wall -W -Wcast-align -Wcast-qual -Wstrict-prototypes -Wwrite-strings -Werror

arm.v4a-linux.elf-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -march=armv4 -c $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

arm.v4a-linux.elf-fold.h : tmp/$$T.o tmp/arm.v4a-linux.elf-main.o $(srcdir)/src/$$T.lds
	$(call tc,ld) --strip-all -T $(srcdir)/src/$T.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	$(call tc,f-objstrip,tmp/$T.bin)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

tmp/arm.v4a-linux.elf-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)

tmp/arm.v4a-linux.elf-main.o : $(srcdir)/src/$$T.c $(srcdir)/src/i386-linux.elf-main.c
	$(call tc,gcc) -c -Os $< -o $@
	$(call tc,f-objstrip,$@)


# /***********************************************************************
# // arm.v5a-linux.kernel.vmlinux      (arm.v5a)
# // arm.v5a-linux.kernel.vmlinux-head (arm.v5a)
# // arm.v5a-linux.kernel.vmlinuz      (arm.v5a)
# // arm.v5a-linux.kernel.vmlinuz-head (arm.v5a)
# ************************************************************************/

arm.v5a-linux.kernel.vmlinu%.h : tc_list = arm.v5a-linux.kernel default
arm.v5a-linux.kernel.vmlinu%.h : tc_bfdname = elf32-littlearm

tc.arm.v5a-linux.kernel.gcc  = arm-linux-gcc-4.1.0 -march=armv5 -nostdinc -MMD -MT $@
tc.arm.v5a-linux.kernel.gcc += -fno-exceptions -fno-asynchronous-unwind-tables
tc.arm.v5a-linux.kernel.gcc += -Wall -W -Wcast-align -Wcast-qual -Wstrict-prototypes -Wwrite-strings -Werror

arm.v5a-linux.kernel.vmlinu%.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@

arm.v5a-linux.kernel.vmlinux-head.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.o
	$(call tc,objcopy) --output-target binary --only-section .text tmp/$T.o tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

arm.v5a-linux.kernel.vmlinuz-head.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.o
	$(call tc,objcopy) --output-target binary --only-section .text tmp/$T.o tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@


# /***********************************************************************
# // arm.v5a-linux.elf (arm.v5a)
# ************************************************************************/

arm.v5a-linux.elf%.h : tc_list = arm.v5a-linux.elf default
arm.v5a-linux.elf%.h : tc_bfdname = elf32-littlearm

tc.arm.v5a-linux.elf.gcc  = arm-linux-gcc-4.1.0 -march=armv5 -nostdinc -MMD -MT $@
tc.arm.v5a-linux.elf.gcc += -fno-exceptions -fno-asynchronous-unwind-tables
tc.arm.v5a-linux.elf.gcc += -Wall -W -Wcast-align -Wcast-qual -Wstrict-prototypes -Wwrite-strings -Werror

arm.v5a-linux.elf-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -march=armv5 -c $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

arm.v5a-linux.elf-fold.h : tmp/$$T.o tmp/armel-linux.elf-main.o $(srcdir)/src/arm.v4a-linux.elf-fold.lds
	$(call tc,ld) --strip-all -T $(srcdir)/src/arm.v4a-linux.elf-fold.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	$(call tc,f-objstrip,tmp/$T.bin)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

tmp/arm.v5a-linux.elf-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)

tmp/armel-linux.elf-main.o : $(srcdir)/src/$$T.c $(srcdir)/src/i386-linux.elf-main.c
	$(call tc,gcc) -c -Os $< -o $@
	$(call tc,f-objstrip,$@)


# /***********************************************************************
# // arm64-linux.elf (arm64)
# ************************************************************************/

arm64-linux.elf%.h : tc_list = arm64-linux.elf default
arm64-linux.elf%.h : tc_bfdname = elf64-littleaarch64

tc.arm64-linux.elf.gcc  = arm64-linux-gcc-4.9.2 -nostdinc -MMD -MT $@
tc.arm64-linux.elf.gcc += -fno-exceptions -fno-asynchronous-unwind-tables
tc.arm64-linux.elf.gcc += -Wall -W -Wcast-align -Wcast-qual -Wstrict-prototypes -Wwrite-strings -Werror

tc.arm64-linux.elf-fold.ld    = arm64-linux-ld-2.25

tc.arm64-linux.elf-entry.objcopy  = arm64-linux-objcopy-2.25 -F elf64-littleaarch64
tc.arm64-linux.elf-fold.objcopy   = arm64-linux-objcopy-2.25 -F elf64-littleaarch64
tc.arm64-linux.elf-main.objcopy   = arm64-linux-objcopy-2.25 -F elf64-littleaarch64

tc.arm64-linux.elf-entry.objdump  = arm64-linux-objdump-2.25
tc.arm64-linux.elf-fold.objdump   = arm64-linux-objdump-2.25
tc.arm64-linux.elf-main.objdump   = arm64-linux-objdump-2.25

arm64-linux.elf-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

arm64-linux.elf-fold.h : tmp/$$T.o tmp/arm64-linux.elf-main.o $(srcdir)/src/arm64-linux.elf-fold.lds
	$(call tc,ld) --strip-all -T $(srcdir)/src/arm64-linux.elf-fold.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	$(call tc,f-objstrip,tmp/$T.bin)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

tmp/arm64-linux.elf-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)

tmp/arm64-linux.elf-main.o : $(srcdir)/src/$$T.c $(srcdir)/src/arm64-linux.elf-main.c
	$(call tc,gcc) -c -Os $< -o $@
	$(call tc,f-objstrip,$@)

# /***********************************************************************
# // arm64-linux.shlib arm64
# ************************************************************************/

arm64-linux.shlib%.h : tc_list = arm64-linux.elf default
arm64-linux.shlib%.h : tc_bfdname = elf64-littleaarch64
tc.arm64-linux.shlib-init.objcopy  = arm64-linux-objcopy-2.25 -F elf64-littleaarch64
tc.arm64-linux.shlib-init.objdump  = arm64-linux-objdump-2.25

arm64-linux.shlib%.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@


# /***********************************************************************
# // arm.v5a-linux.shlib (arm.v5a)
# ************************************************************************/

arm.v5a-linux.shlib%.h : tc_list = arm.v5a-linux.elf default
arm.v5a-linux.shlib%.h : tc_bfdname = elf32-littlearm

arm.v5a-linux.shlib%.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@


# /***********************************************************************
# // arm.v5t-linux.shlib (arm.v5t)
# ************************************************************************/

arm.v5t-linux.shlib%.h : tc_list = arm.v5t-linux.elf default
arm.v5t-linux.shlib%.h : tc_bfdname = elf32-littlearm

tc.arm.v5t-linux.elf.gcc  = $(tc.arm.v5a-linux.elf.gcc) -march=armv5t

arm.v5t-linux.shlib%.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@


# /***********************************************************************
# // armeb.v4a-linux.elf (arm.v4a)
# ************************************************************************/

armeb.v4a-linux.elf%.h : tc_list = armeb.v4a-linux.elf default
armeb.v4a-linux.elf%.h : tc_bfdname = elf32-bigarm

tc.armeb.v4a-linux.elf.gcc = $(tc.arm.v4a-linux.elf.gcc) -mbig-endian

armeb.v4a-linux.elf-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

armeb.v4a-linux.elf-fold.h : tmp/$$T.o tmp/armeb.v4a-linux.elf-main.o $(srcdir)/src/$$T.lds
	$(call tc,ld) --strip-all -T $(srcdir)/src/$T.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	$(call tc,f-objstrip,tmp/$T.bin)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

tmp/armeb.v4a-linux.elf-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)

tmp/armeb.v4a-linux.elf-main.o : $(srcdir)/src/$$T.c $(srcdir)/src/i386-linux.elf-main.c
	$(call tc,gcc) -c -Os $< -o $@
	$(call tc,f-objstrip,$@)


# /***********************************************************************
# // armeb.v5a-linux.kernel.vmlinux      (arm.v5a)
# // armeb.v5a-linux.kernel.vmlinux-head (arm.v5a)
# ************************************************************************/

armeb.v5a-linux.kernel.vmlinu%.h : tc_list = armeb.v5a-linux.kernel default
armeb.v5a-linux.kernel.vmlinu%.h : tc_bfdname = elf32-bigarm

tc.armeb.v5a-linux.kernel.gcc  = $(tc.arm.v4a-linux.elf.gcc) -mbig-endian -march=armv5

armeb.v5a-linux.kernel.vmlinu%.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@

armeb.v5a-linux.kernel.vmlinux-head.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.o
	$(call tc,objcopy) --output-target binary --only-section .text tmp/$T.o tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@


# /***********************************************************************
# // arm.v4a-wince.pe (arm.v4a)
# // arm.v4t-wince.pe (arm.v4t)
# ************************************************************************/

# info: we use the tc settings from arm.v4a-linux.elf
arm.v4a-wince.pe.h : tc_list = arm.v4a-linux.elf default
arm.v4t-wince.pe.h : tc_list = arm.v4a-linux.elf default
arm.v4a-wince.pe.h : tc_bfdname = elf32-littlearm
arm.v4t-wince.pe.h : tc_bfdname = elf32-littlearm
arm.v4t-wince.pe.h : tc_objdump_disasm_options = -M force-thumb

arm.v4a-wince.pe.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -march=armv4 -c $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@

arm.v4t-wince.pe.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -march=armv4t -c $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@


# /***********************************************************************
# // arm64-darwin.macho
# ************************************************************************/

arm64-darwin.macho%.h : tc_list = arm64-darwin.macho default
arm64-darwin.macho%.h : tc_bfdname = elf64-littleaarch64

tc.arm64-darwin.macho.gcc      = arm64-linux-gcc-4.9.2 -nostdinc -MMD -MT $@
tc.arm64-darwin.macho.ld       = arm64-linux-ld-2.25
tc.arm64-darwin.macho.objcopy  = arm64-linux-objcopy-2.25 -F $(tc_bfdname)
tc.arm64-darwin.macho.objdump  = arm64-linux-objdump-2.25 -b $(tc_bfdname)

arm64-darwin.macho-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

arm64-darwin.macho-fold.h : tmp/$$T.o tmp/arm64-darwin.macho-main.o
	$(call tc,ld) --no-warn-mismatch --strip-all -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.tmp
	$(call tc,objcopy) -O binary tmp/$T.tmp tmp/$T.bin
	rm tmp/$T.tmp
	chmod a-x tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

tmp/arm64-darwin.macho-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)

tmp/arm64-darwin.macho-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c -Os $< -o $@
	$(call tc,f-objstrip,$@)

# /***********************************************************************
# // i086-dos16.com
# ************************************************************************/

i086-dos16.com.h : tc_list = arch-i086 default
i086-dos16.com.h : tc_bfdname = elf32-i386
i086-dos16.com.h : tc_bfdarch = i8086

i086-dos16.com.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@


# /***********************************************************************
# // i086-dos16.exe
# ************************************************************************/

i086-dos16.exe.h : tc_list = arch-i086 default
i086-dos16.exe.h : tc_bfdname = elf32-i386
i086-dos16.exe.h : tc_bfdarch = i8086

i086-dos16.exe.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@


# /***********************************************************************
# // i086-dos16.sys
# ************************************************************************/

i086-dos16.sys.h : tc_list = arch-i086 default
i086-dos16.sys.h : tc_bfdname = elf32-i386
i086-dos16.sys.h : tc_bfdarch = i8086

i086-dos16.sys.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@


# /***********************************************************************
# // i386-bsd.elf
# // i386-netbsd.elf
# // i386-openbsd.elf
# ************************************************************************/

# info: we use the tc settings from i386-linux.elf
i386-bsd.elf%.h : tc_list = i386-linux.elf arch-i386 default
i386-bsd.elf%.h : tc_bfdname = elf32-i386

i386-bsd.elf-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

i386-bsd.elf-fold.h : tmp/$$T.o tmp/i386-bsd.elf-main.o tmp/i386-bsd.syscall.o $(srcdir)/src/$$T.lds
	$(call tc,ld) --strip-all -T $(srcdir)/src/$T.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	$(call tc,f-objstrip,tmp/$T.bin)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,brandelf) --elfosabi=freebsd tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

tmp/i386-bsd.elf-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.o
	$(call tc,f-objstrip,$@)

tmp/i386-bsd.syscall.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o $@
	$(call tc,f-objstrip,$@)

tmp/i386-bsd.elf-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)


# info: we use the tc settings from i386-linux.elf
i386-netbsd.elf%.h : tc_list = i386-linux.elf arch-i386 default
i386-netbsd.elf%.h : tc_bfdname = elf32-i386

i386-netbsd.elf-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

# NetBSD uses the plain BSD fold.o and the plain BSD entry.o and main.o
i386-netbsd.elf-fold.h :  tmp/i386-bsd.elf-fold.o tmp/i386-bsd.elf-main.o tmp/i386-bsd.syscall.o $(srcdir)/src/i386-bsd.elf-fold.lds
	$(call tc,ld) --strip-all -T $(srcdir)/src/i386-bsd.elf-fold.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	$(call tc,f-objstrip,tmp/$T.bin)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,brandelf) --elfosabi=netbsd tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@


# info: we use the tc settings from i386-linux.elf
i386-openbsd.elf%.h : tc_list = i386-linux.elf arch-i386 default
i386-openbsd.elf%.h : tc_bfdname = elf32-i386

# Note the re-use of i386-bsd.elf-entry.h as output (no separate i386-openbsd.elf-entry.h).
# Note the re-use of i386-bsd.elf-fold.lds as input (no separate i386-openbsd.elf-fold.lds).
i386-openbsd.elf-fold.h : tmp/$$T.o tmp/i386-openbsd.elf-main.o tmp/i386-bsd.syscall.o $(srcdir)/src/i386-bsd.elf-fold.lds
	$(call tc,ld) --strip-all -T $(srcdir)/src/i386-bsd.elf-fold.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	$(call tc,f-objstrip,tmp/$T.bin)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,brandelf) --elfosabi=openbsd tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

tmp/i386-openbsd.elf-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.o
	$(call tc,f-objstrip,$@)

tmp/i386-openbsd.elf-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)


# /***********************************************************************
# // i386-bsd.elf.execve
# ************************************************************************/

# note: tc_list settings are inherited from i386-bsd.elf because of wildcard matching

i386-bsd.elf.execve-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

# Note the re-use of i386-linux.elf.execve-fold.lds as input (no separate i386-bsd.elf.execve-fold.lds).
i386-bsd.elf.execve-fold.h : tmp/$$T.o tmp/i386-bsd.elf.execve-main.o tmp/i386-bsd.syscall.o tmp/i386-linux.elf.execve-upx_itoa.o $(srcdir)/src/i386-linux.elf.execve-fold.lds
	$(call tc,ld) --strip-all -T $(srcdir)/src/i386-linux.elf.execve-fold.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	$(call tc,f-objstrip,tmp/$T.bin)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,brandelf) --elfosabi=freebsd tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

tmp/i386-bsd.elf.execve-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.o
	$(call tc,f-objstrip,$@)

tmp/i386-bsd.elf.execve-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)

tmp/i386-bsd.elf.execve-upx_itoa.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o $@
	$(call tc,f-objstrip,$@)


# /***********************************************************************
# // i386-darwin.dylib
# ************************************************************************/

# info: we use the tc settings from i386-linux.elf
i386-darwin.dylib%.h : tc_list = i386-linux.elf default
i386-darwin.dylib%.h : tc_bfdname = elf32-i386

## All code is in dylib-entry.  There is no dylib-fold, no dylib-main.
i386-darwin.dylib-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@


# /***********************************************************************
# // i386-darwin.macho
# ************************************************************************/

# info: we use the tc settings from i386-linux.elf
i386-darwin.macho%.h : tc_list = i386-linux.elf default
i386-darwin.macho%.h : tc_bfdname = elf32-i386

i386-darwin.macho-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

i386-darwin.macho-fold.h : tmp/$$T.o tmp/i386-darwin.macho-main.o
	$(call tc,ld) --no-warn-mismatch --strip-all --oformat binary -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	chmod a-x tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

i386-darwin.macho-upxmain.h: i386-darwin.macho-upxmain.exe
	$(call tc,bin2h) $< $@

tmp/i386-darwin.macho-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)

tmp/i386-darwin.macho-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c -Os $< -o $@
	$(call tc,f-objstrip,$@)


# /***********************************************************************
# // i386-dos32.djgpp2
# ************************************************************************/

i386-dos32.djgpp2% : tc_list = arch-i386 default
i386-dos32.djggp2% : tc_bfdname = elf32-i386

i386-dos32.djgpp2.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@

i386-dos32.djgpp2-stubify.h : $(srcdir)/src/$$T.asm
	$(call tc,djasm) --outtype=exe --gmtime=1070220810 --inname=stub.asm --outname=stub.h $< tmp/$T.bin
	$(call tc,objdump) -b binary -m i8086 -D --start-address=0x254 tmp/$T.bin | $(RTRIM) > tmp/$T.bin.disasm
	$(call tc,bin2h) -q tmp/$T.bin $@


# /***********************************************************************
# // i386-dos32.tmt
# ************************************************************************/

i386-dos32.tmt.h : tc_list = arch-i386 default
i386-dos32.tmt.h : tc_bfdname = elf32-i386

i386-dos32.tmt.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@


# /***********************************************************************
# // i386-dos32.watcom.le
# ************************************************************************/

i386-dos32.watcom.le.h : tc_list = arch-i386 default
i386-dos32.watcom.le.h : tc_bfdname = elf32-i386

i386-dos32.watcom.le.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@


# /***********************************************************************
# // i386-linux.elf
# ************************************************************************/

i386-linux.elf%.h : tc_list = i386-linux.elf arch-i386 default
i386-linux.elf%.h : tc_bfdname = elf32-i386

tc.i386-linux.elf.gcc      = i386-linux-gcc-3.4.6 -m32 -march=i386 -nostdinc -MMD -MT $@
tc.i386-linux.elf.gcc     += -fno-exceptions -fno-asynchronous-unwind-tables
tc.i386-linux.elf.gcc     += -Wall -W -Wcast-align -Wcast-qual -Wstrict-prototypes -Wwrite-strings -Werror
tc.i386-linux.elf.gcc     += -mtune=k6
tc.i386-linux.elf.gcc     += -Os -fno-omit-frame-pointer
tc.i386-linux.elf.gcc     += -momit-leaf-frame-pointer
tc.i386-linux.elf.gcc     += -fno-align-functions -fno-align-jumps -fno-align-labels -fno-align-loops
tc.i386-linux.elf.gcc     += -mpreferred-stack-boundary=2
tc.i386-linux.elf.gcc     += -fweb

i386-linux.elf-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

i386-linux.elf-fold.h : tmp/$$T.o tmp/i386-linux.elf-main.o $(srcdir)/src/$$T.lds
	$(call tc,ld) --strip-all -T $(srcdir)/src/$T.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	$(call tc,f-objstrip,tmp/$T.bin)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,brandelf) --elfosabi=linux tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

tmp/i386-linux.elf-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.o
	$(call tc,f-objstrip,$@)

tmp/i386-linux.elf-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)


# /***********************************************************************
# // i386-linux.elf.execve
# ************************************************************************/

# note: tc_list settings are inherited from i386-linux.elf because of wildcard matching

i386-linux.elf.execve-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

i386-linux.elf.execve-fold.h : tmp/$$T.o tmp/i386-linux.elf.execve-main.o tmp/i386-linux.elf.execve-upx_itoa.o $(srcdir)/src/$$T.lds
	$(call tc,ld) --strip-all -T $(srcdir)/src/$T.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	$(call tc,f-objstrip,tmp/$T.bin)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,brandelf) --elfosabi=linux tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

tmp/i386-linux.elf.execve-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.o
	$(call tc,f-objstrip,$@)

tmp/i386-linux.elf.execve-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)

tmp/i386-linux.elf.execve-upx_itoa.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o $@
	$(call tc,f-objstrip,$@)


# /***********************************************************************
# // i386-linux.elf.interp
# ************************************************************************/

# note: tc_list settings are inherited from i386-linux.elf because of wildcard matching

i386-linux.elf.interp-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

i386-linux.elf.interp-fold.h : tmp/$$T.o tmp/i386-linux.elf.interp-main.o $(srcdir)/src/$$T.lds
	$(call tc,ld) --strip-all -T $(srcdir)/src/$T.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	$(call tc,f-objstrip,tmp/$T.bin)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,brandelf) --elfosabi=linux tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

tmp/i386-linux.elf.interp-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.o
	$(call tc,f-objstrip,$@)

tmp/i386-linux.elf.interp-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)


# /***********************************************************************
# // i386-linux.elf.shell
# ************************************************************************/

# note: tc_list settings are inherited from i386-linux.elf because of wildcard matching

i386-linux.elf.shell-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

i386-linux.elf.shell-fold.h : tmp/$$T.o tmp/i386-linux.elf.shell-main.o $(srcdir)/src/$$T.lds
	$(call tc,ld) --strip-all -T $(srcdir)/src/$T.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	$(call tc,f-objstrip,tmp/$T.bin)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,brandelf) --elfosabi=linux tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

tmp/i386-linux.elf.shell-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.o
	$(call tc,f-objstrip,$@)

tmp/i386-linux.elf.shell-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)


# /***********************************************************************
# // i386-linux.kernel.vmlinux
# // i386-linux.kernel.vmlinux-head
# // i386-linux.kernel.vmlinuz
# ************************************************************************/

i386-linux.kernel.vmlinu%.h : tc_list = arch-i386 default
i386-linux.kernel.vmlinu%.h : tc_bfdname = elf32-i386

i386-linux.kernel.vmlinu%.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@

i386-linux.kernel.vmlinux-head.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.o
	$(call tc,objcopy) --output-target binary --only-section .text tmp/$T.o tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@


# /***********************************************************************
# // i386-linux.shlib
# ************************************************************************/

i386-linux.shlib%.h : tc_list = i386-linux.elf arch-i386 default
i386-linux.shlib%.h : tc_bfdname = elf32-i386

i386-linux.shlib-init.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@


# /***********************************************************************
# // i386-win32.pe
# ************************************************************************/

i386-win32.pe.h : tc_list = arch-i386 default
i386-win32.pe.h : tc_bfdname = elf32-i386

i386-win32.pe.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@


# /***********************************************************************
# // m68k-atari.tos
# ************************************************************************/

m68k-atari.tos.h : tc_list = m68k-atari.tos default
m68k-atari.tos.h : tc_bfdname = elf32-m68k
m68k-atari.tos.h : tc_objdump_disasm_options = -m m68k:68040

tc.m68k-atari.tos.gcc  = m68k-linux-gcc-4.1.1 -m68000 -malign-int -nostdinc -MMD -MT $@
tc.m68k-atari.tos.gcc += -fno-exceptions -fno-asynchronous-unwind-tables
tc.m68k-atari.tos.gcc += -Wall -W -Wcast-align -Wcast-qual -Wstrict-prototypes -Wwrite-strings -Werror

m68k-atari.tos.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp -Wa,-m68000,-l,--pcrel,--register-prefix-optional $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@


# /***********************************************************************
# // mips.r3000-linux.elf
# ************************************************************************/

mips.r3000-linux.elf%.h : tc_list = mips.r3000-linux.elf default
mips.r3000-linux.elf%.h : tc_bfdname = elf32-bigmips

tc.mips.r3000-linux.elf.as   = mipsel-elf-as-20060406 -EB -O -mno-pdr
tc.mips.r3000-linux.elf.gcc  = mipsel-linux-gcc-4.1.1 -meb -march=r3000 -mno-abicalls -mabi=eabi -G0 -nostdinc -MMD -MT $@
tc.mips.r3000-linux.elf.gcc += -fno-exceptions -fno-asynchronous-unwind-tables
tc.mips.r3000-linux.elf.gcc += -Wall -W -Wcast-align -Wcast-qual -Wstrict-prototypes -Wwrite-strings -Werror

mips.r3000-linux.elf-entry.h : $(srcdir)/src/$$T.S
ifeq (1,1)
	# info: we really need as-2.17 here
	$(call tc,pp-as) -D_TARGET_LINUX_ -D__MIPSEB__ $< -o - | $(RTRIM) > tmp/$T.i
	$(call tc,as) tmp/$T.i -o tmp/$T.bin
	$(call tc,gpp_mkdep) --mode=c --MMD=$@ --MF=tmp/$T.d $<
else
	# info: as-2.16.1 as used by gcc-4.1.1 does _not_ work
	$(call tc,gcc) -c -D_TARGET_LINUX_ -Wa,-O,-mno-pdr $< -o tmp/$T.bin
endif
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@

mips.r3000-linux.elf-fold.h : tmp/$$T.o tmp/mips.r3000-linux.elf-main.o $(srcdir)/src/$$T.lds
	$(call tc,ld) --strip-all -T $(srcdir)/src/$T.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	$(call tc,f-objstrip,tmp/$T.bin)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

tmp/mips.r3000-linux.elf-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -D_TARGET_LINUX_ $< -o $@
	$(call tc,f-objstrip,$@)

tmp/mips.r3000-linux.elf-main.o : $(srcdir)/src/$$T.c $(srcdir)/src/i386-linux.elf-main.c
	$(call tc,gcc) -S -Os -MF tmp/$T.d $< -o - | $(RTRIM) > tmp/$T.i
	sed -e 's/	j[ 	][ 	]*$$L/	b $$L/' \
	    -e 's/	jal[ 	][ 	]*\([^\$$]\)/	bal \1/' < tmp/$T.i > tmp/$T.s
	$(call tc,gcc) -c -Wa,-O,-mno-pdr tmp/$T.s -o $@
	$(call tc,f-objstrip,$@)


# /***********************************************************************
# // mips.r3000-linux.shlib
# ************************************************************************/

mips.r3000-linux.shlib%.h : tc_list = mips.r3000-linux.elf default
mips.r3000-linux.shlib%.h : tc_bfdname = elf32-bigmips

mips.r3000-linux.shlib%.h : $(srcdir)/src/$$T.S
ifeq (1,1)
	# info: we really need as-2.17 here
	$(call tc,pp-as) -D_TARGET_LINUX_ -D__MIPSEB__ $< -o - | $(RTRIM) > tmp/$T.i
	$(call tc,as) tmp/$T.i -o tmp/$T.bin
	$(call tc,gpp_mkdep) --mode=c --MMD=$@ --MF=tmp/$T.d $<
else
	# info: as-2.16.1 as used by gcc-4.1.1 does _not_ work
	$(call tc,gcc) -c -D_TARGET_LINUX_ -Wa,-O,-mno-pdr $< -o tmp/$T.bin
endif
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@


# /***********************************************************************
# // mipsel.r3000-linux.elf
# ************************************************************************/

mipsel.r3000-linux.elf%.h : tc_list = mipsel.r3000-ps1 default
mipsel.r3000-linux.elf%.h : tc_bfdname = elf32-littlemips

mipsel.r3000-linux.elf-entry.h : $(srcdir)/src/$$T.S
ifeq (1,1)
	# info: we really need as-2.17 here
	$(call tc,pp-as) -D_TARGET_LINUX_ -D__MIPSEL__ $< -o - | $(RTRIM) > tmp/$T.i
	$(call tc,as) tmp/$T.i -o tmp/$T.bin
	$(call tc,gpp_mkdep) --mode=c --MMD=$@ --MF=tmp/$T.d $<
else
	# info: as-2.16.1 as used by gcc-4.1.1 does _not_ work
	$(call tc,gcc) -c -D_TARGET_LINUX_ -Wa,-O,-mno-pdr $< -o tmp/$T.bin
endif
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@

mipsel.r3000-linux.elf-fold.h : tmp/$$T.o tmp/mipsel.r3000-linux.elf-main.o $(srcdir)/src/$$T.lds
	$(call tc,ld) --strip-all -T $(srcdir)/src/$T.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	$(call tc,f-objstrip,tmp/$T.bin)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

tmp/mipsel.r3000-linux.elf-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -D_TARGET_LINUX_ $< -o $@
	$(call tc,f-objstrip,$@)

tmp/mipsel.r3000-linux.elf-main.o : $(srcdir)/src/$$T.c $(srcdir)/src/i386-linux.elf-main.c
	$(call tc,gcc) -S -Os -MF tmp/$T.d $< -o - | $(RTRIM) > tmp/$T.i
	sed -e 's/	j[ 	][ 	]*$$L/	b $$L/' \
	    -e 's/	jal[ 	][ 	]*\([^\$$]\)/	bal \1/' < tmp/$T.i > tmp/$T.s
	$(call tc,gcc) -c -Wa,-O,-mno-pdr tmp/$T.s -o $@
	$(call tc,f-objstrip,$@)


# /***********************************************************************
# // mipsel.r3000-linux.shlib
# ************************************************************************/

mipsel.r3000-linux.shlib%.h : tc_list = mipsel.r3000-ps1 default
mipsel.r3000-linux.shlib%.h : tc_bfdname = elf32-littlemips

mipsel.r3000-linux.shlib%.h : $(srcdir)/src/$$T.S
ifeq (1,1)
	# info: we really need as-2.17 here
	$(call tc,pp-as) -D_TARGET_LINUX_ -D__MIPSEL__ $< -o - | $(RTRIM) > tmp/$T.i
	$(call tc,as) tmp/$T.i -o tmp/$T.bin
	$(call tc,gpp_mkdep) --mode=c --MMD=$@ --MF=tmp/$T.d $<
else
	# info: as-2.16.1 as used by gcc-4.1.1 does _not_ work
	$(call tc,gcc) -c -D_TARGET_LINUX_ -Wa,-O,-mno-pdr $< -o tmp/$T.bin
endif
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@


# /***********************************************************************
# // mipsel.r3000-ps1
# ************************************************************************/

mipsel.r3000-ps1.h : tc_list = mipsel.r3000-ps1 default
mipsel.r3000-ps1.h : tc_bfdname = elf32-littlemips

tc.mipsel.r3000-ps1.as   = mipsel-elf-as-20060406 -O -mno-pdr
##tc.mipsel.r3000-ps1.as   = mipsel-linux-as-2.16.1 -O -mno-pdr
tc.mipsel.r3000-ps1.gcc  = mipsel-linux-gcc-4.1.1 -mel -march=r3000 -mno-abicalls -mabi=eabi -G0 -nostdinc -MMD -MT $@
tc.mipsel.r3000-ps1.gcc += -fno-exceptions -fno-asynchronous-unwind-tables
tc.mipsel.r3000-ps1.gcc += -Wall -W -Wcast-align -Wcast-qual -Wstrict-prototypes -Wwrite-strings -Werror

mipsel.r3000-ps1.h : $(srcdir)/src/$$T.S
ifeq (1,1)
	# info: we really need as-2.17 here
	$(call tc,pp-as) -DPS1=1 -D__MIPSEL__ $< -o - | $(RTRIM) > tmp/$T.i
	$(call tc,as) tmp/$T.i -o tmp/$T.bin
	$(call tc,gpp_mkdep) --mode=c --MMD=$@ --MF=tmp/$T.d $<
else
	# info: as-2.16.1 as used by gcc-4.1.1 does _not_ work
	$(call tc,gcc) -c -DPS1 -Wa,-O,-mno-pdr $< -o tmp/$T.bin
endif
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@


# /***********************************************************************
# // powerpc-darwin.macho
# ************************************************************************/

# info: we use the tc settings from powerpc-linux.elf
powerpc-darwin.macho%.h : tc_list = powerpc-linux.elf default
powerpc-darwin.macho%.h : tc_bfdname = elf32-powerpc

powerpc-darwin.macho-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

powerpc-darwin.macho-fold.h : tmp/$$T.o tmp/powerpc-darwin.macho-main.o
	$(call tc,ld) --no-warn-mismatch --strip-all --oformat binary -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	chmod a-x tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

powerpc-darwin.macho-upxmain.h: powerpc-darwin.macho-upxmain.exe
	$(call tc,bin2h) $< $@

tmp/powerpc-darwin.macho-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)

tmp/powerpc-darwin.macho-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)


# /***********************************************************************
# // powerpc-darwin.dylib
# ************************************************************************/

# info: we use the tc settings from powerpc-linux.elf
powerpc-darwin.dylib%.h : tc_list = powerpc-linux.elf default
powerpc-darwin.dylib%.h : tc_bfdname = elf32-powerpc

powerpc-darwin.dylib-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@


# /***********************************************************************
# // powerpc-linux.elf
# ************************************************************************/

powerpc-linux.elf%.h : tc_list = powerpc-linux.elf default
powerpc-linux.elf%.h : tc_bfdname = elf32-powerpc

tc.powerpc-linux.elf.gcc  = powerpc.405-linux-gcc-3.4.5 -m32 -mbig-endian -mtune=powerpc -nostdinc -MMD -MT $@
tc.powerpc-linux.elf.gcc += -fno-exceptions -fno-asynchronous-unwind-tables   # -fno-stack-protector
tc.powerpc-linux.elf.gcc += -Wall -W -Wcast-align -Wcast-qual -Wstrict-prototypes -Wwrite-strings -Werror

powerpc-linux.elf-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

powerpc-linux.elf-fold.h : tmp/$$T.o tmp/powerpc-linux.elf-main.o $(srcdir)/src/$$T.lds
	$(call tc,ld) --strip-all -T $(srcdir)/src/$T.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	$(call tc,f-objstrip,tmp/$T.bin)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

tmp/powerpc-linux.elf-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)

tmp/powerpc-linux.elf-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)


# /***********************************************************************
# // powerpc-linux.kernel.vmlinux
# // powerpc-linux.kernel.vmlinux-head
# ************************************************************************/

powerpc-linux.kernel.vmlinu%.h : tc_list = powerpc-linux.kernel default
powerpc-linux.kernel.vmlinu%.h : tc_bfdname = elf32-powerpc

tc.powerpc-linux.kernel.gcc  = $(tc.powerpc-linux.elf.gcc)

powerpc-linux.kernel.vmlinu%.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@

powerpc-linux.kernel.vmlinux-head.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.o
	$(call tc,objcopy) --output-target binary --only-section .text tmp/$T.o tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@


# /***********************************************************************
# // powerpc64-darwin.dylib
# ************************************************************************/

# info: we use the tc settings from powerpc64le-linux.elf
powerpc64-darwin.dylib%.h : tc_list = powerpc64-linux.elf default
powerpc64-darwin.dylib%.h : tc_bfdname = elf64-powerpc

powerpc64-darwin.dylib-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@


# /***********************************************************************
# // powerpc64-darwin.macho
# ************************************************************************/

# info: we use the tc settings from powerpc64le-linux.elf
# Omit -Os because it references _savegpr0_28 and _restgpr0_28 (and *_16 and *_27)
# only 3 times, so sharing the code of those routines saves almost nothing
# (and we would have to write them ourselves.)
# Besides, PowerPC has 31.5 registers (r0 cannot index), which is plenty.
powerpc64-darwin.macho%.h : tc_list = powerpc64-darwin.macho powerpc64-linux.elf default
powerpc64-darwin.macho%.h : tc_bfdname = elf64-powerpc

tc.powerpc64-darwin.macho.ld = multiarch-ld-2.27 -b $(tc_bfdname)

powerpc64-darwin.macho-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

powerpc64-darwin.macho-fold.h : tmp/$$T.o tmp/powerpc64-darwin.macho-main.o
	$(call tc,ld) --no-warn-mismatch --strip-all --oformat binary -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	chmod a-x tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

tmp/powerpc64-darwin.macho-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)

# Omit -Os because it references _savegpr0_28 and _restgpr0_28 (and *_16 and *_27)
# only 3 times, so sharing the code of those routines saves almost nothing
# (and we would have to write them ourselves.)
# Besides, PowerPC has 31.5 registers (r0 cannot index), which is plenty.
tmp/powerpc64-darwin.macho-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c -O $< -o $@
	$(call tc,f-objstrip,$@)


# /***********************************************************************
# // powerpc64le-linux.elf
# ************************************************************************/

powerpc64le-linux.elf%.h : tc_list = powerpc64le-linux.elf default
powerpc64le-linux.elf%.h : tc_bfdname = elf64-powerpcle

tc.powerpc64le-linux.elf.gcc  = powerpc64-linux-gcc-4.9.2 -m64 -mlittle-endian -nostdinc -MMD -MT $@
tc.powerpc64le-linux.elf.gcc += -fno-exceptions -fno-asynchronous-unwind-tables -fno-stack-protector
tc.powerpc64le-linux.elf.gcc += -Wall -W -Wcast-align -Wcast-qual -Wstrict-prototypes -Wwrite-strings -Werror

tc.powerpc64le-linux.elf.ld      = powerpc64-linux-ld-2.25 -b $(tc_bfdname)
tc.powerpc64le-linux.elf.objcopy = multiarch-objcopy-2.27 -F $(tc_bfdname)
tc.powerpc64le-linux.elf.objdump = multiarch-objdump-2.27 -b $(tc_bfdname)

powerpc64le-linux.elf-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

powerpc64le-linux.elf-fold.h : tmp/$$T.o tmp/powerpc64le-linux.elf-main.o $(srcdir)/src/$$T.lds
	$(call tc,ld) --strip-all -T $(srcdir)/src/$T.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	$(call tc,f-objstrip,tmp/$T.bin)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

tmp/powerpc64le-linux.elf-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)

tmp/powerpc64le-linux.elf-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c -Os $< -o $@
	$(call tc,f-objstrip,$@)


powerpc64-linux.elf%.h : tc_list = powerpc64-linux.elf default
powerpc64-linux.elf%.h : tc_bfdname = elf64-powerpc

tc.powerpc64-linux.elf.gcc  = powerpc64-linux-gcc-4.9.2 -m64 -mbig-endian -nostdinc -MMD -MT $@
tc.powerpc64-linux.elf.gcc += -fno-exceptions -fno-asynchronous-unwind-tables -fno-stack-protector
tc.powerpc64-linux.elf.gcc += -mno-longcall -mno-toc
tc.powerpc64-linux.elf.gcc += -Wall -W -Wcast-align -Wcast-qual -Wstrict-prototypes -Wwrite-strings -Werror

tc.powerpc64-linux.elf.ld      = powerpc64-linux-ld-2.25 -b $(tc_bfdname)
tc.powerpc64-linux.elf.objcopy = multiarch-objcopy-2.27 -F $(tc_bfdname)
tc.powerpc64-linux.elf.objdump = multiarch-objdump-2.27 -b $(tc_bfdname)

powerpc64-linux.elf-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h) tmp/$T.bin $@

powerpc64-linux.elf-fold.h : tmp/$$T.o tmp/powerpc64-linux.elf-main.o $(srcdir)/src/$$T.lds
	$(call tc,ld) --strip-all -T $(srcdir)/src/$T.lds -Map tmp/$T.map $(filter %.o,$^) -o tmp/$T.bin
	$(call tc,f-objstrip,tmp/$T.bin)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@

tmp/powerpc64-linux.elf-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o $@
	$(call tc,f-objstrip,$@)

tmp/powerpc64-linux.elf-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c -Os $< -o $@
	$(call tc,f-objstrip,$@)


# /***********************************************************************
# // powerpc64le-linux.kernel.vmlinux
# // powerpc64le-linux.kernel.vmlinux-head
# ************************************************************************/

powerpc64le-linux.kernel.vmlinu%.h : tc_list = powerpc64le-linux.kernel default
powerpc64le-linux.kernel.vmlinu%.h : tc_bfdname = elf64-powerpcle

tc.powerpc64le-linux.kernel.gcc  = $(tc.powerpc64le-linux.elf.gcc)

powerpc64le-linux.kernel.vmlinu%.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@

powerpc64le-linux.kernel.vmlinux-head.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.o
	$(call tc,objcopy) --output-target binary --only-section .text tmp/$T.o tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@


powerpc64-linux.kernel.vmlinu%.h : tc_list = powerpc64-linux.kernel default
powerpc64-linux.kernel.vmlinu%.h : tc_bfdname = elf64-powerpc

tc.powerpc64-linux.kernel.gcc  = $(tc.powerpc64-linux.elf.gcc)

powerpc64-linux.kernel.vmlinu%.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,f-embed_objinfo,tmp/$T.bin)
	$(call tc,bin2h-c) tmp/$T.bin $@

powerpc64-linux.kernel.vmlinux-head.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.o
	$(call tc,objcopy) --output-target binary --only-section .text tmp/$T.o tmp/$T.bin
	$(call tc,bin2h) tmp/$T.bin $@


# /***********************************************************************
# // dependencies
# ************************************************************************/

ifneq ($(strip $(STUBS)),)
# FIXME: we want a dependency-only prerequisite here
$(STUBS): tmp/.tmp-stamp $(MAKEFILE_LIST)
$(STUBS): $(top_srcdir)/src/stub/scripts/bin2h.py
$(STUBS): $(top_srcdir)/src/stub/scripts/xstrip.py
endif
-include tmp/*.d

ifneq ($(strip $(STUBS)),)
.DELETE_ON_ERROR: $(STUBS)
endif


# /***********************************************************************
# // extra-all
# // extra-clean
# ************************************************************************/

ifndef EXTRA_MAKEFILES
EXTRA_MAKEFILES :=
##ifneq ($(filter extra-%,$(MAKECMDGOALS)),)
EXTRA_MAKEFILES += $(sort $(wildcard src/arch/*/Makefile.extra))
EXTRA_MAKEFILES += $(sort $(wildcard src/arch/*/*/Makefile.extra))
EXTRA_MAKEFILES += $(sort $(wildcard src/arch/*/*/*/Makefile.extra))
####EXTRA_MAKEFILES += $(shell find src/arch -name Makefile.extra 2>/dev/null)
##endif
endif

ifneq ($(strip $(EXTRA_MAKEFILES)),)
EXTRA_MAKEFILES.targets :=
EXTRA_MAKEFILES.targets += $(addsuffix .~all,$(sort $(EXTRA_MAKEFILES)))
EXTRA_MAKEFILES.targets += $(addsuffix .~clean,$(sort $(EXTRA_MAKEFILES)))

$(EXTRA_MAKEFILES.targets): MAKEFLAGS += --no-print-directory
$(EXTRA_MAKEFILES.targets): $$(basename $$@)
	$(MAKE) -C $(dir $@) -f $(basename $(notdir $@)) $(subst .~,,$(suffix $@))

extra-all:   $$(filter %.~all,$$(EXTRA_MAKEFILES.targets))
extra-clean: $$(filter %.~clean,$$(EXTRA_MAKEFILES.targets))

.PHONY: extra-all extra-clean $(EXTRA_MAKEFILES.targets)
endif

# vim:set ts=8 sw=8 noet:
