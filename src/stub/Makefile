#
# UPX stub Makefile - needs GNU make 3.81 or better
#
# see http://upx.sourceforge.net/download/tools/
# for required build tools
#
# also please read README.SRC
#

MAKEFLAGS += -rR
.SUFFIXES:
.SECONDEXPANSION:
export SHELL = /bin/sh
export LC_ALL = C

ifneq ($(findstring $(firstword $(MAKE_VERSION)),3.79 3.79.1 3.80),)
$(error GNU make 3.81 or better is required)
endif

# update $PATH for our special stub build tools
ifneq ($(wildcard $(HOME)/local/bin/bin-upx/.),)
export PATH := $(HOME)/local/bin/bin-upx:$(PATH)
endif
ifneq ($(wildcard $(HOME)/bin/bin-upx/.),)
export PATH := $(HOME)/bin/bin-upx:$(PATH)
endif


# /***********************************************************************
# //
# ************************************************************************/

ifndef srcdir
srcdir := $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))
srcdir := $(shell echo '$(srcdir)' | sed 's,/*$$,,')
endif

ifndef STUBS
STUBS += amd64-linux.elf-entry.h
STUBS += amd64-linux.elf-fold.h
STUBS += arm-linux.elf-entry.h
STUBS += arm-linux.elf-fold.h
STUBS += arm.v4a-wince.pe.h
STUBS += arm.v4t-wince.pe.h
STUBS += i086-dos16.com.h
STUBS += i086-dos16.exe.h
STUBS += i086-dos16.sys.h
STUBS += i386-dos32.djgpp2.h
STUBS += i386-dos32.djgpp2-stubify.h
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
STUBS += i386-linux.kernel.vmlinux.h
STUBS += i386-linux.kernel.vmlinuz.h
STUBS += i386-win32.pe.h
STUBS += m68k-atari.tos-nrv2b.h
STUBS += m68k-atari.tos-nrv2b.small.h
STUBS += m68k-atari.tos-nrv2d.h
STUBS += m68k-atari.tos-nrv2d.small.h
STUBS += m68k-atari.tos-nrv2e.h
STUBS += m68k-atari.tos-nrv2e.small.h
STUBS += mipsel.r3000-ps1-boot.h
STUBS += mipsel.r3000-ps1-console.h
STUBS += powerpc-darwin.macho-entry.h
STUBS += powerpc-darwin.macho-fold.h
STUBS += powerpc-linux.elf-entry.h
STUBS += powerpc-linux.elf-fold.h
endif


ifndef default.targets
ifeq ($(wildcard .all-stamp),)
default.targets =
default: $$(default.targets)
	@echo "UPX info: type 'make all' if you have all the required build tools."
else
default.targets = all
default: $$(default.targets)
endif
endif

all.targets ?= tmp/.tmp-stamp .all-stamp
all: $$(all.targets)
tmp/.tmp-stamp:
	@mkdir -p $(dir $@)
	@echo "timestamp" > $@
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
	rm -f $(STUBS)
	rm -rf tmp

.PHONY: default all mostlyclean clean distclean maintainer-clean


# util var for use in the rules - basename of the current target
override T = $(basename $(notdir $@))


# /***********************************************************************
# // setup toolchain globals
# ************************************************************************/

# default settings for $(tc_list)
tc.default.bin2h      = python $(srcdir)/scripts/bin2h.py
tc.default.brandelf   = perl -w $(srcdir)/scripts/brandelf.pl
tc.default.djasm      = djasm
tc.default.gpp_inc    = python $(srcdir)/scripts/gpp_inc.py
tc.default.m-objcopy  = multiarch-objcopy-2.17
tc.default.m-objdump  = multiarch-objdump-2.17
tc.default.m-ld       = multiarch-ld-2.17
tc.default.m-nm       = multiarch-nm-2.17
tc.default.m-readelf  = multiarch-readelf-2.17
tc.default.nasm       = nasm
tc.default.nasm      += -I$(srcdir)/ -I$(srcdir)/src/
tc.default.nasm      += -O99 -w+macro-params -w+macro-selfref -w+number-overflow -w+orphan-labels
tc.default.nasm      += -Dstub_$(subst .,_,$(subst -,_,$(basename $(notdir $@))))=1
tc.default.o2bin      = perl $(srcdir)/scripts/o2bin.pl
tc.default.pp-asm     = i386-linux-gcc-2.95.3 -E -nostdinc -x assembler-with-cpp -Wall -Wp,-P,-C,-traditional
tc.default.pp-nasm    = $(tc.default.gpp_inc) --mode=nasm -I$(srcdir)/ -I$(srcdir)/src/
tc.default.sstrip     = sstrip

# some common settings for $(tc_list)
tc.arch-i086.app-nasm  = perl -w $(srcdir)/src/arch/i086/app-nasm.pl
tc.arch-i086.gcc       = i386-linux-gcc-3.4.6 -m32 -march=i386 -nostdinc -MMD
tc.arch-i386.app-nasm  = perl -w $(srcdir)/src/arch/i386/app-nasm.pl
tc.arch-i386.gcc       = i386-linux-gcc-3.4.6 -m32 -march=i386 -nostdinc -MMD


# slightly tricky make stuff follows

# enumerate the names of all variables that will get tested (from basename and $(tc_list))
__tc_varlist   = tc.$(basename $(notdir $@)).$1 $(foreach v,$(tc_list),tc.$v.$1)
# return the name of the first variable that is not empty
__tc_varsearch = $(firstword $(foreach v,$1,$(if $($v),$v,)))
# error sentinel for missing commands
__tc_FALSE     = false tc_FALSE: '$1' '$@' '$<' '$(tc_list)'
# expand the first variable that is not empty
tc             = $($(call __tc_varsearch,$(call __tc_varlist,$1) __tc_FALSE))


# clear some vars, just in case
CPPFLAGS =
CPLAGS =
PP_FLAGS =
IDENT_NAME =
IDENT_PREFIX =
IDENT_SUFFIX =


# /***********************************************************************
# // amd64-linux.elf
# ************************************************************************/

amd64-linux.elf% : tc_list = amd64-linux.elf default

tc.amd64-linux.elf.gcc      = x86_64-unknown-linux-gnu-gcc-3.4.4 -m64 -nostdinc -MMD
tc.amd64-linux.elf.gcc     += -fno-exceptions -fno-asynchronous-unwind-tables
tc.amd64-linux.elf.gcc     += -Wall -W -Wcast-align -Wcast-qual -Wwrite-strings -Werror
##tc.amd64-linux.elf.ld       = x86_64-unknown-linux-gnu-ld -m elf_x86_64
##tc.amd64-linux.elf.objcopy  = x86_64-unknown-linux-gnu-objcopy
##tc.amd64-linux.elf.objdump  = x86_64-unknown-linux-gnu-objdump
tc.amd64-linux.elf.ld       = $(call tc,m-ld) -b elf64-x86-64
tc.amd64-linux.elf.objcopy  = $(call tc,m-objcopy) -I elf64-x86-64
tc.amd64-linux.elf.objdump  = $(call tc,m-objdump) -b elf64-x86-64
tc.amd64-linux.elf.objstrip = $(call tc,objcopy) -R .comment -R .note

amd64-linux.elf-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o tmp/$T.o
	$(call tc,objstrip) tmp/$T.o
	$(call tc,ld)  --oformat binary tmp/$T.o -o tmp/$T.bin
	$(call tc,bin2h) --ident=linux_elf64amd_loader tmp/$T.bin $@

amd64-linux.elf-fold.h : tmp/$$T.o tmp/amd64-linux.elf-main.o
	$(call tc,ld) -T $(srcdir)/src/$T.lds -Map tmp/$T.map -o tmp/$T.bin --strip-all $(filter %.o,$^)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,bin2h) --ident=linux_elf64amd_fold tmp/$T.bin $@

tmp/amd64-linux.elf-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o $@
	$(call tc,objstrip) $@

tmp/amd64-linux.elf-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c -Os $< -o $@
	$(call tc,objstrip) $@


# /***********************************************************************
# // arm-linux.elf
# ************************************************************************/

arm-linux.elf% : tc_list = arm-linux.elf default

tc.arm-linux.elf.gcc      = arm-9tdmi-linux-gnu-gcc-3.4.5 -march=armv4 -nostdinc -MMD
tc.arm-linux.elf.gcc     += -fno-exceptions -fno-asynchronous-unwind-tables
tc.arm-linux.elf.gcc     += -Wall -W -Wcast-align -Wcast-qual -Wwrite-strings -Werror
##tc.arm-linux.elf.ld       = arm-9tdmi-linux-gnu-ld
##tc.arm-linux.elf.objcopy  = arm-9tdmi-linux-gnu-objcopy
##tc.arm-linux.elf.objdump  = arm-9tdmi-linux-gnu-objdump
tc.arm-linux.elf.ld       = $(call tc,m-ld) -b elf32-littlearm
tc.arm-linux.elf.objcopy  = $(call tc,m-objcopy) -I elf32-littlearm
tc.arm-linux.elf.objdump  = $(call tc,m-objdump) -b elf32-littlearm
tc.arm-linux.elf.objstrip = $(call tc,objcopy) -R .comment -R .note

arm-linux.elf-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -nostdlib $< -o tmp/$T.out
	$(call tc,objcopy) --only-section .text -O binary tmp/$T.out tmp/$T.bin
	$(call tc,bin2h) --ident=linux_elf32arm_loader tmp/$T.bin $@

arm-linux.elf-fold.h : tmp/$$T.o tmp/arm-linux.elf-main.o
	$(call tc,ld) -T $(srcdir)/src/$T.lds -Map tmp/$T.map -o tmp/$T.bin --strip-all $(filter %.o,$^)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,bin2h) --ident=linux_elf32arm_fold tmp/$T.bin $@

tmp/arm-linux.elf-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o $@
	$(call tc,objstrip) $@

tmp/arm-linux.elf-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c -Os $< -o $@
	$(call tc,objstrip) $@


# /***********************************************************************
# // arm.v4a-wince.pe
# // arm.v4t-wince.pe
# ************************************************************************/

# info: we use the tc settings from arm-linux.elf
arm.v4a-wince.pe% : tc_list = arm-linux.elf default
arm.v4t-wince.pe% : tc_list = arm-linux.elf default

arm.v4a-wince.pe.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -march=armv4 -nostdlib $< -o tmp/$T.out
	$(call tc,objcopy) --only-section .text -O binary tmp/$T.out tmp/$T.bin
	$(call tc,bin2h) --ident=nrv_loader_arm tmp/$T.bin $@

arm.v4t-wince.pe.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -march=armv4t -nostdlib $< -o tmp/$T.out
	$(call tc,objcopy) --only-section .text -O binary tmp/$T.out tmp/$T.bin
	$(call tc,bin2h) --ident=nrv_loader_thumb tmp/$T.bin $@


# /***********************************************************************
# // i086-dos16.com
# ************************************************************************/

i086-dos16.com% : tc_list = arch-i086 default

i086-dos16.com.h : $(srcdir)/src/$$T.asm
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,m-objdump) -trwh tmp/$T.bin >> tmp/$T.bin
	$(call tc,bin2h) --ident=nrv2b_loader tmp/$T.bin $@


# /***********************************************************************
# // i086-dos16.exe
# ************************************************************************/

i086-dos16.exe% : tc_list = arch-i086 default

i086-dos16.exe.h : $(srcdir)/src/$$T.asm
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,m-objdump) -trwh tmp/$T.bin >> tmp/$T.bin
	$(call tc,bin2h) --ident=nrv_loader tmp/$T.bin $@


# /***********************************************************************
# // i086-dos16.sys
# ************************************************************************/

i086-dos16.sys% : tc_list = arch-i086 default

i086-dos16.sys.h : $(srcdir)/src/$$T.asm
	$(call tc,gcc) -c -x assembler-with-cpp $< -o tmp/$T.bin
	$(call tc,m-objdump) -trwh tmp/$T.bin >> tmp/$T.bin
	$(call tc,bin2h) --ident=nrv2b_loader tmp/$T.bin $@


# /***********************************************************************
# // i386-dos32.djgpp2
# ************************************************************************/

i386-dos32.djgpp2% : tc_list = arch-i386 default

i386-dos32.djgpp2.h : $(srcdir)/src/$$T.asm
	$(call tc,pp-nasm) --MMD=$@ $< -o tmp/$T.tmp1
	$(call tc,app-nasm) tmp/$T.tmp1 tmp/$T.tmp2
	$(call tc,nasm) -f bin -l tmp/$T.bin.lst tmp/$T.tmp2 -o tmp/$T.bin
	$(call tc,bin2h) --ident=nrv_loader tmp/$T.bin $@

i386-dos32.djgpp2-stubify.h : $(srcdir)/src/$$T.asm
	$(call tc,djasm) --outtype=exe --gmtime=1070220810 --inname=stub.asm --outname=stub.h $< tmp/$T.bin
	$(call tc,bin2h) --ident=stubify_stub -q tmp/$T.bin $@


# /***********************************************************************
# // i386-dos32.tmt
# ************************************************************************/

i386-dos32.tmt% : tc_list = arch-i386 default

i386-dos32.tmt.h : $(srcdir)/src/$$T.asm
	$(call tc,pp-nasm) --MMD=$@ $< -o tmp/$T.tmp1
	$(call tc,app-nasm) tmp/$T.tmp1 tmp/$T.tmp2
	$(call tc,nasm) -f bin -l tmp/$T.bin.lst tmp/$T.tmp2 -o tmp/$T.bin
	$(call tc,bin2h) --ident=nrv_loader tmp/$T.bin $@


# /***********************************************************************
# // i386-dos32.watcom.le
# ************************************************************************/

i386-dos32.watcom.le% : tc_list = arch-i386 default

i386-dos32.watcom.le.h : $(srcdir)/src/$$T.asm
	$(call tc,pp-nasm) --MMD=$@ $< -o tmp/$T.tmp1
	$(call tc,app-nasm) tmp/$T.tmp1 tmp/$T.tmp2
	$(call tc,nasm) -f bin -l tmp/$T.bin.lst tmp/$T.tmp2 -o tmp/$T.bin
	$(call tc,bin2h) --ident=nrv_loader tmp/$T.bin $@


# /***********************************************************************
# // i386-linux.elf
# ************************************************************************/

i386-linux.elf% : tc_list = i386-linux.elf arch-i386 default

tc.i386-linux.elf.gcc      = i386-linux-gcc-3.4.6 -m32 -march=i386 -nostdinc -MMD
tc.i386-linux.elf.gcc     += -fno-exceptions -fno-asynchronous-unwind-tables
tc.i386-linux.elf.gcc     += -Wall -W -Wcast-align -Wcast-qual -Wwrite-strings -Werror
tc.i386-linux.elf.gcc     += -march=i386 -mtune=k6
tc.i386-linux.elf.gcc     += -Os -fno-omit-frame-pointer
tc.i386-linux.elf.gcc     += -momit-leaf-frame-pointer
tc.i386-linux.elf.gcc     += -fno-align-functions -fno-align-jumps -fno-align-labels -fno-align-loops
tc.i386-linux.elf.gcc     += -mpreferred-stack-boundary=2
tc.i386-linux.elf.gcc     += -fweb
##tc.i386-linux.elf.ld       = i386-linux-ld-2.16.1
##tc.i386-linux.elf.objcopy  = i386-linux-objcopy-2.16.1
##tc.i386-linux.elf.objdump  = i386-linux-objdump
tc.i386-linux.elf.ld       = $(call tc,m-ld) -b elf32-i386
tc.i386-linux.elf.objcopy  = $(call tc,m-objcopy) -I elf32-i386
tc.i386-linux.elf.objdump  = $(call tc,m-objdump) -b elf32-i386
tc.i386-linux.elf.objstrip = $(call tc,objcopy) -R .comment -R .note

i386-linux.elf-entry.h : $(srcdir)/src/$$T.asm
	$(call tc,pp-nasm) $< -o tmp/$T.tmp1
	$(call tc,app-nasm) tmp/$T.tmp1 tmp/$T.tmp2
	$(call tc,nasm) -f bin -l tmp/$T.bin.lst tmp/$T.tmp2 -o tmp/$T.bin
	$(call tc,bin2h) --ident=linux_i386elf_loader tmp/$T.bin $@

i386-linux.elf-fold.h : tmp/$$T.o tmp/i386-linux.elf-main.o
	$(call tc,ld) -T $(srcdir)/src/$T.lds -Map tmp/$T.map -o tmp/$T.bin --strip-all $(filter %.o,$^)
	$(call tc,objstrip) tmp/$T.bin
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,brandelf) tmp/$T.bin
	$(call tc,bin2h) --ident=linux_i386elf_fold tmp/$T.bin $@

tmp/i386-linux.elf-fold.o : $(srcdir)/src/$$T.asm
	$(call tc,nasm) -f elf -l $@.lst $< -o $@
	$(call tc,objstrip) $@

tmp/i386-linux.elf-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c $< -o $@
	$(call tc,objstrip) $@


# /***********************************************************************
# // i386-linux.elf.execve
# ************************************************************************/

# note: tc_list settings are inherited from i386-linux.elf

i386-linux.elf.execve-entry.h : $(srcdir)/src/$$T.asm
	$(call tc,pp-nasm) --MMD=$@ $< -o tmp/$T.tmp1
	$(call tc,app-nasm) tmp/$T.tmp1 tmp/$T.tmp2
	$(call tc,nasm) -f bin -l tmp/$T.bin.lst tmp/$T.tmp2 -o tmp/$T.bin
	$(call tc,bin2h) --ident=linux_i386exec_loader tmp/$T.bin $@

i386-linux.elf.execve-fold.h : tmp/$$T.o tmp/i386-linux.elf.execve-main.o tmp/i386-linux.elf.execve-upx_itoa.o
	$(call tc,ld) -T $(srcdir)/src/$T.lds -Map tmp/$T.map -o tmp/$T.bin --strip-all $(filter %.o,$^)
	$(call tc,objstrip) tmp/$T.bin
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,brandelf) tmp/$T.bin
	$(call tc,bin2h) --ident=linux_i386exec_fold tmp/$T.bin $@

tmp/i386-linux.elf.execve-fold.o : $(srcdir)/src/$$T.asm
	$(call tc,nasm) -f elf -l $@.lst $< -o $@
	$(call tc,objstrip) $@

tmp/i386-linux.elf.execve-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c $< -o $@
	$(call tc,objstrip) $@

tmp/i386-linux.elf.execve-upx_itoa.o: $(srcdir)/src/$$T.asm
	$(call tc,nasm) -f elf -l $@.lst $< -o $@
	$(call tc,objstrip) $@


# /***********************************************************************
# // i386-linux.elf.interp
# ************************************************************************/

# note: tc_list settings are inherited from i386-linux.elf

i386-linux.elf.interp-entry.h : $(srcdir)/src/$$T.asm
	$(call tc,pp-nasm) --MMD=$@ $< -o tmp/$T.tmp1
	$(call tc,app-nasm) tmp/$T.tmp1 tmp/$T.tmp2
	$(call tc,nasm) -f bin -l tmp/$T.bin.lst tmp/$T.tmp2 -o tmp/$T.bin
	$(call tc,bin2h) --ident=linux_i386pti_loader tmp/$T.bin $@

i386-linux.elf.interp-fold.h : tmp/$$T.o tmp/i386-linux.elf.interp-main.o
	$(call tc,ld) -T $(srcdir)/src/$T.lds -Map tmp/$T.map -o tmp/$T.bin --strip-all $(filter %.o,$^)
	$(call tc,objstrip) tmp/$T.bin
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,brandelf) tmp/$T.bin
	$(call tc,bin2h) --ident=linux_i386pti_fold tmp/$T.bin $@

tmp/i386-linux.elf.interp-fold.o : $(srcdir)/src/$$T.asm
	$(call tc,nasm) -f elf -l $@.lst $< -o $@
	$(call tc,objstrip) $@

tmp/i386-linux.elf.interp-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c $< -o $@
	$(call tc,objstrip) $@


# /***********************************************************************
# // i386-linux.elf.shell
# ************************************************************************/

# note: tc_list settings are inherited from i386-linux.elf

i386-linux.elf.shell-entry.h : $(srcdir)/src/$$T.asm
	$(call tc,pp-nasm) --MMD=$@ $< -o tmp/$T.tmp1
	$(call tc,app-nasm) tmp/$T.tmp1 tmp/$T.tmp2
	$(call tc,nasm) -f bin -l tmp/$T.bin.lst tmp/$T.tmp2 -o tmp/$T.bin
	$(call tc,bin2h) --ident=linux_i386sh_loader tmp/$T.bin $@

i386-linux.elf.shell-fold.h : tmp/$$T.o tmp/i386-linux.elf.shell-main.o
	$(call tc,ld) -T $(srcdir)/src/$T.lds -Map tmp/$T.map -o tmp/$T.bin --strip-all $(filter %.o,$^)
	$(call tc,objstrip) tmp/$T.bin
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,brandelf) tmp/$T.bin
	$(call tc,bin2h) --ident=linux_i386sh_fold tmp/$T.bin $@

tmp/i386-linux.elf.shell-fold.o : $(srcdir)/src/$$T.asm
	$(call tc,nasm) -f elf -l $@.lst $< -o $@
	$(call tc,objstrip) $@

tmp/i386-linux.elf.shell-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c $< -o $@
	$(call tc,objstrip) $@


# /***********************************************************************
# // i386-linux.kernel.vmlinux
# // i386-linux.kernel.vmlinuz
# ************************************************************************/

i386-linux.kernel.vmlin% : tc_list = arch-i386 default

i386-linux.kernel%.h : $(srcdir)/src/$$T.asm
	$(call tc,pp-nasm) --MMD=$@ $< -o tmp/$T.tmp1
	$(call tc,app-nasm) tmp/$T.tmp1 tmp/$T.tmp2
	$(call tc,nasm) -f bin -l tmp/$T.bin.lst tmp/$T.tmp2 -o tmp/$T.bin
	$(call tc,bin2h) --ident=nrv_loader tmp/$T.bin $@


# /***********************************************************************
# // i386-win32.pe
# ************************************************************************/

i386-win32.pe% : tc_list = arch-i386 default

i386-win32.pe.h : $(srcdir)/src/$$T.asm
	$(call tc,pp-nasm) --MMD=$@ $< -o tmp/$T.tmp1
	$(call tc,app-nasm) tmp/$T.tmp1 tmp/$T.tmp2
	$(call tc,nasm) -f bin -l tmp/$T.bin.lst tmp/$T.tmp2 -o tmp/$T.bin
	$(call tc,bin2h) --ident=nrv_loader tmp/$T.bin $@


# /***********************************************************************
# // m68k-atari.tos
# ************************************************************************/

m68k-atari.tos-%.h : tc_list = m68k-atari.tos default

tc.m68k-atari.tos.app-a68k  = perl -w $(srcdir)/src/arch/m68k/app-a68k.pl
tc.m68k-atari.tos.asm-a68k  = a68k

m68k-atari.tos-nrv%.h : $(srcdir)/src/m68k-atari.tos.asm
	# call gpp_inc to generate .d file
	$(call tc,gpp_inc) --mode=c --MMD=$@ --MF=tmp/$T.i.d $< -o /dev/null
	$(call tc,pp-asm) -D__A68K__ $(PP_FLAGS) $< -o tmp/$T.i
	$(call tc,asm-a68k) -q -ltmp/$T.o.lst tmp/$T.i -otmp/$T.o
	$(call tc,o2bin) tmp/$T.o tmp/$T.bin 'UPX1' 'UPX9'
	$(call tc,bin2h) --ident=$(IDENT_PREFIX)loader$(IDENT_SUFFIX) tmp/$T.bin $@

m68k-atari.tos-nrv2b% :        PP_FLAGS     = -DNRV2B
m68k-atari.tos-nrv2d% :        PP_FLAGS     = -DNRV2D
m68k-atari.tos-nrv2e% :        PP_FLAGS     = -DNRV2E
m68k-atari.tos-nrv2%.small.h : PP_FLAGS    += -DSMALL
m68k-atari.tos-nrv2b% :        IDENT_PREFIX = nrv2b_
m68k-atari.tos-nrv2d% :        IDENT_PREFIX = nrv2d_
m68k-atari.tos-nrv2e% :        IDENT_PREFIX = nrv2e_
m68k-atari.tos-nrv2%.small.h : IDENT_SUFFIX = _small


# /***********************************************************************
# // mipsel.r3000-ps1-boot
# // mipsel.r3000-ps1-console
# ************************************************************************/

mipsel.r3000-ps1-%.h : tc_list = mipsel.r3000-ps1 default

tc.mipsel.r3000-ps1.app-asm5900 = perl -w $(srcdir)/src/arch/mips/mipsel.r3000/app-asm5900.pl
tc.mipsel.r3000-ps1.asm5900     = asm5900

mipsel.r3000-ps1-%.h : $(srcdir)/src/mipsel.r3000-ps1.asm
	# call gpp_inc to generate .d file
	$(call tc,gpp_inc) --mode=c --MMD=$@ --MF=tmp/$T.tmp1.d $< -o /dev/null
	$(call tc,pp-asm) $(PP_FLAGS) $< -o tmp/$T.tmp1
	$(call tc,app-asm5900) tmp/$T.tmp1 tmp/$T.tmp2
	$(call tc,asm5900) --nologo -q -ltmp/$T.bin.lst tmp/$T.tmp2 -otmp/$T.bin
	$(call tc,bin2h) --ident=$(IDENT_NAME) tmp/$T.bin $@

mipsel.r3000-ps1-boot.h:    PP_FLAGS  += -DCDBOOT
mipsel.r3000-ps1-boot.h:    IDENT_NAME = nrv_boot_loader
mipsel.r3000-ps1-console.h: IDENT_NAME = nrv_con_loader


# /***********************************************************************
# // powerpc-linux.elf
# ************************************************************************/

powerpc-linux.elf% : tc_list = powerpc-linux.elf default

tc.powerpc-linux.elf.gcc      = powerpc-750-linux-gnu-gcc-3.4.4 -mcpu=405 -nostdinc -MMD
tc.powerpc-linux.elf.gcc     += -fno-exceptions -fno-asynchronous-unwind-tables
tc.powerpc-linux.elf.gcc     += -Wall -W -Wcast-align -Wcast-qual -Wwrite-strings -Werror
##tc.powerpc-linux.elf.ld       = powerpc-750-linux-gnu-ld
##tc.powerpc-linux.elf.objcopy  = powerpc-750-linux-gnu-objcopy
##tc.powerpc-linux.elf.objdump  = powerpc-750-linux-gnu-objdump
tc.powerpc-linux.elf.ld       = $(call tc,m-ld) -b elf32-powerpc
tc.powerpc-linux.elf.objcopy  = $(call tc,m-objcopy) -I elf32-powerpc
tc.powerpc-linux.elf.objdump  = $(call tc,m-objdump) -b elf32-powerpc
tc.powerpc-linux.elf.objstrip = $(call tc,objcopy) -R .comment -R .note

powerpc-linux.elf-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o tmp/$T.o
	$(call tc,objstrip) tmp/$T.o
	$(call tc,ld)  --oformat binary tmp/$T.o -o tmp/$T.bin
	$(call tc,bin2h) --ident=linux_elfppc32_loader tmp/$T.bin $@

powerpc-linux.elf-fold.h : tmp/$$T.o tmp/powerpc-linux.elf-main.o
	$(call tc,ld) -T $(srcdir)/src/$T.lds -Map tmp/$T.map -o tmp/$T.bin --strip-all $(filter %.o,$^)
	$(call tc,sstrip) tmp/$T.bin
	$(call tc,bin2h) --ident=linux_elfppc32_fold tmp/$T.bin $@

tmp/powerpc-linux.elf-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o $@
	$(call tc,objstrip) $@

tmp/powerpc-linux.elf-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c -Os $< -o $@
	$(call tc,objstrip) $@


# /***********************************************************************
# // powerpc-darwin.macho
# ************************************************************************/

# info: we use the tc settings from powerpc-linux.elf
powerpc-darwin.macho% : tc_list = powerpc-linux.elf default

powerpc-darwin.macho-entry.h : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o tmp/$T.o
	$(call tc,objstrip) tmp/$T.o
	$(call tc,ld)  --oformat binary tmp/$T.o -o tmp/$T.bin
	$(call tc,bin2h) --ident=l_mac_ppc32_loader tmp/$T.bin $@

powerpc-darwin.macho-fold.h : tmp/$$T.o tmp/powerpc-darwin.macho-main.o
	$(call tc,ld) --oformat binary -Map tmp/$T.map -o tmp/$T.bin --strip-all $(filter %.o,$^)
	$(call tc,bin2h) --ident=fold_machppc32 tmp/$T.bin $@

tmp/powerpc-darwin.macho-fold.o : $(srcdir)/src/$$T.S
	$(call tc,gcc) -c $< -o $@
	$(call tc,objstrip) $@

tmp/powerpc-darwin.macho-main.o : $(srcdir)/src/$$T.c
	$(call tc,gcc) -c -Os $< -o $@
	$(call tc,objstrip) $@


# /***********************************************************************
# // dependencies
# ************************************************************************/

ifneq ($(STUBS),)
# FIXME: we want a dependency-only-prerequisite here
##$(STUBS): | tmp/.tmp-stamp
##$(STUBS): $(MAKEFILE_LIST)
endif
-include tmp/*.d


# vi:ts=8:nowrap
