#
# UPX stub Makefile (GNU make)
#
# see http://upx.sourceforge.net/download/tools/
# for required build tools
#

MAKEFLAGS += -rR
SHELL = /bin/sh

ifeq ($(strip $(UCLDIR)),)
# change this to reflect where the UCL library is
UCLDIR = $(HOME)/local/src/ucl-1.03
endif
ifeq ($(wildcard $(UCLDIR)/upx),)
$(error Please set UCLDIR in Makefile or environment)
endif

# update path for our special stub build tools
ifneq ($(wildcard $(HOME)/local/bin/bin-upx),)
export PATH := $(HOME)/local/bin/bin-upx:$(PATH)
endif


# -------------------------------------------------------
# You should not have to change anything below this line.
# -------------------------------------------------------

srcdir = .
top_srcdir = ../..


# These are the files we want to create.
STUBS = \
	l_armpe.h \
	l_com.h \
	l_djgpp2.h stubify.h \
	l_exe.h \
	l_ps1b.h l_ps1c.h \
	l_sys.h \
	l_t_n2b.h l_t_n2bs.h l_t_n2d.h l_t_n2ds.h l_t_n2e.h l_t_n2es.h \
	l_tmt.h \
	l_wcle.h \
	l_w32pe.h \
	l_lx_exec86.h fold_exec86.h \
	l_lx_elf86.h  fold_elf86.h \
	l_lx_sh86.h   fold_sh86.h \
	l_lx_pti86.h  fold_pti86.h \
	l_lx_elfppc32.h fold_elfppc32.h \
	l_lx_elf64amd.h fold_elf64amd.h \
	l_mac_ppc32.h fold_machppc32.h \
	l_vmlinz.h l_vmlinx.h

# experimental:
ifneq ($(strip $(wildcard $(srcdir)/l_ext2.asm)),)
STUBS += l_ext2.h
endif


# util var for use in the rules - basename of the current target
override T = $(basename $@)

ifneq ($(strip $(USE_MAKEFILE_DEPS)),)
MAKEFILE_DEPS = $(MAKEFILE_LIST)
endif


# /***********************************************************************
# // source directories
# ************************************************************************/

UCL_UPX  = $(UCLDIR)/upx
UCL_I386 = $(UCLDIR)/upx/i386
UCL_M68K = $(UCLDIR)/upx/m68k
UCL_MR3K = $(UCLDIR)/upx/mr3k

.SUFFIXES:
.SUFFIXES: .asm .ash .asx .asy .bin .c .h .s

vpath %.ash $(UCL_I386)
vpath %.ash $(UCL_M68K)
vpath %.ash $(UCL_MR3K)


# /***********************************************************************
# // tools
# ************************************************************************/

NASM     = nasm -O99 -w+macro-params -w+macro-selfref -w+number-overflow -w+orphan-labels
NASM    += -I$(srcdir)/

APP_I386 = perl -w $(srcdir)/scripts/app_i386.pl
BIN2H    = perl -w $(srcdir)/scripts/bin2h.pl
##BIN2H    = python  $(srcdir)/scripts/bin2h.py
BRANDELF = perl -w $(srcdir)/scripts/brandelf.pl
O2BIN    = perl -w $(srcdir)/scripts/o2bin.pl
##STRIPELF = perl -w $(srcdir)/scripts/stripelf.pl


###
### i386-linux
###

LD_LINUX_I386 = ld-2.13.2 -m elf_i386
LD_LINUX_I386 = ld -m elf_i386
OBJCOPY_LINUX_I386 = objcopy -F elf32-i386
OBJSTRIP_LINUX_I386 = $(OBJCOPY_LINUX_I386) -R .comment -R .note
STRIPELF_LINUX_I386 = ./util/sstrip/i386-linux-sstrip

# gcc 2.7.2.3 (fold_*86.bin: 1905 / 1011 / 1791 / 1406)
CC_LINUX_I386_GCC272  = gcc-2.72 -m386 -O2 -MMD
CC_LINUX_I386_GCC272 += -malign-functions=0 -malign-jumps=0 -malign-loops=0

# gcc 2.95.3 (fold_*86.bin: 1773 / 995 / 1664 / 1317)
CC_LINUX_I386_GCC295  = gcc-2.95.3 -march=i386 -mcpu=k6 -fno-exceptions -MMD
CC_LINUX_I386_GCC295 += -Os -fno-omit-frame-pointer
CC_LINUX_I386_GCC295 += -malign-functions=0 -malign-jumps=0 -malign-loops=0
CC_LINUX_I386_GCC295 += -Werror

# gcc 3.4.5 (fold_*86.bin: 1651 / 947 / 1537 / 1190)
CC_LINUX_I386_GCC34  = gcc-3.4.5
CC_LINUX_I386_GCC34 += -m32 -march=i386 -mtune=k6 -fno-exceptions -MMD
CC_LINUX_I386_GCC34 += -Os -fno-omit-frame-pointer
CC_LINUX_I386_GCC34 += -momit-leaf-frame-pointer
CC_LINUX_I386_GCC34 += -fno-align-functions -fno-align-jumps -fno-align-labels -fno-align-loops
CC_LINUX_I386_GCC34 += -mpreferred-stack-boundary=2
CC_LINUX_I386_GCC34 += -fweb
CC_LINUX_I386_GCC34 += -Werror

# gcc 4.1.0 (fold_*86.bin: 1638 / 931 / 1514 / 1137)
# gcc 4.0.3 (fold_*86.bin: 1731 / 995 / 1625 / 1195)
CC_LINUX_I386_GCC41  = gcc-4.1.0
CC_LINUX_I386_GCC41 += -m32 -march=i386 -mtune=k6 -fno-exceptions -MMD
CC_LINUX_I386_GCC41 += -Os -fno-omit-frame-pointer
CC_LINUX_I386_GCC41 += -momit-leaf-frame-pointer
CC_LINUX_I386_GCC41 += -fno-align-functions -fno-align-jumps -fno-align-labels -fno-align-loops
CC_LINUX_I386_GCC41 += -mpreferred-stack-boundary=2
CC_LINUX_I386_GCC41 += -fweb
CC_LINUX_I386_GCC41 += -Werror

CC_LINUX_I386  = $(CC_LINUX_I386_GCC272)
CC_LINUX_I386  = $(CC_LINUX_I386_GCC295)
CC_LINUX_I386  = $(CC_LINUX_I386_GCC34)
CC_LINUX_I386  = $(CC_LINUX_I386_GCC41)
CC_LINUX_I386 += -nostdinc
CC_LINUX_I386 += -Wall -W -Wcast-align -Wcast-qual -Wwrite-strings
CC_LINUX_I386 += -funsigned-char


###
### MIPS R3000
###

CPP_MR3K = gcc-2.95.3 -nostdinc -I$(UCL_UPX) -I$(srcdir) -E -x assembler-with-cpp -Wall -Wp,-P,-C,-traditional
APP_MR3K = perl -w $(srcdir)/scripts/app_mr3k.pl
ASM_MR3K = asm5900 --nologo -q


###
### Motorola 68000
###

CPP_M68K  = $(CPP_MR3K)
APP_M68K  = perl -w $(srcdir)/scripts/app_m68k.pl
ifeq (1,1)
  # a68k 68000-assembler
  CPP_M68K += -D__A68K__
  ASM_M68K  = a68k -q -x
else
  # asl 68000-assembler
  CPP_M68K += -D__ASL__
  ASM_M68K  = sh $(srcdir)/scripts/asl_m68k.sh
endif


###
### PowerPC 32
###

GCC_PPC32     := false
LD_PPC32      := false
OBJCOPY_PPC32 := false
OBJSTRIP_PPC32 = $(OBJCOPY_PPC32) -R .comment -R .note

d = /home2/crosstool/powerpc-750-linux-gnu/gcc-3.4.1-glibc-20040827/bin
ifneq ($(wildcard $d),)
  GCC_PPC32     := $d/powerpc-750-linux-gnu-gcc -m32 -nostdinc -MMD
  LD_PPC32      := $d/powerpc-750-linux-gnu-ld
  OBJCOPY_PPC32 := $d/powerpc-750-linux-gnu-objcopy
endif
d = /opt/cc-i386-linux/crosstool/powerpc-750-linux-gnu/gcc-3.4.3.20050210-glibc-2.2.5/bin
ifneq ($(wildcard $d),)
  GCC_PPC32     := $d/powerpc-750-linux-gnu-gcc -m32 -nostdinc -MMD
  LD_PPC32      := $d/powerpc-750-linux-gnu-ld
  OBJCOPY_PPC32 := $d/powerpc-750-linux-gnu-objcopy
endif


###
### AMD x86_64
###

GCC_AMD64     := false
LD_AMD64      := false
OBJCOPY_AMD64 := false
OBJSTRIP_AMD64 = $(OBJCOPY_AMD64) -R .comment -R .note

d = /home2/crosstool/gcc-4.0.1-glibc-2.3.5/x86_64-unknown-linux-gnu/bin
ifneq ($(wildcard $d),)
  GCC_AMD64     := $d/x86_64-unknown-linux-gnu-gcc -m64 -nostdinc -MMD
  LD_AMD64      := $d/x86_64-unknown-linux-gnu-ld -m elf_x86_64
  OBJCOPY_AMD64 := $d/x86_64-unknown-linux-gnu-objcopy -F elf64-x86-64
endif
d = /opt/cc-i386-linux/crosstool/x86_64-unknown-linux-gnu/gcc-3.4.3.20050507-glibc-2.3.5/bin
ifneq ($(wildcard $d),)
  GCC_AMD64     := $d/x86_64-unknown-linux-gnu-gcc -m64 -nostdinc -MMD
  LD_AMD64      := $d/x86_64-unknown-linux-gnu-ld -m elf_x86_64
  OBJCOPY_AMD64 := $d/x86_64-unknown-linux-gnu-objcopy -F elf64-x86-64
endif


###
### ARM-PE-WINCE
###

GCC_WINCE	:=  arm-wince-pe-gcc -Os
LD_WINCE	:=  arm-wince-pe-ld
OBJCOPY_WINCE	:=  arm-wince-pe-objcopy
BIN2H_WINCE	:=  perl -ne 'print "db\t", join(",", map { sprintf "%\#02x", $$_ } unpack("C*", $$_)), "\n"'


# /***********************************************************************
# // main targets
# ************************************************************************/

.PHONY: default all stubs mostlyclean clean distclean maintainer-clean ident strings

default:
	@echo "UPX info: type 'make all' if you have all the required build tools."

all: stubs ## upxb upxd
	@echo "timestamp" > stamp-h

stubs: $(STUBS)


mostlyclean:
	-rm -f *.bin *.bkp *.d *.i *.lst *.map stubify.exe
	-rm -f *.o *.asx *.asy

clean: mostlyclean
ifneq ($(strip $(wildcard stamp-h)),)
	-rm -f l_*.h $(STUBS)
endif

distclean: mostlyclean
	@-rm -f stamp-h

maintainer-clean: mostlyclean
	-rm -f l_*.h $(STUBS)
	@-rm -f stamp-h


ident: all
	ident *.bin

strings: all
	strings *.bin


# /***********************************************************************
# // rules
# ************************************************************************/

%.asx : %.asm
	$(APP_I386) $< $@

%.asy : %.ash
	$(APP_I386) $< $@


stubify.h: stub.asm
	djasm --gmtime=1070220810 --iname=stub.asm --oname=stub.h $< stubify.exe
	$(BIN2H) stubify.exe stubify_stub $@ -q

l_com.h: l_com.asx
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin nrv2b_loader $@

l_djgpp2.h: l_djgpp2.asx
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin nrv_loader $@

l_exe.h: l_exe.asx
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin nrv_loader $@

l_sys.h: l_sys.asx
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin nrv2b_loader $@

l_tmt.h: l_tmt.asx
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin nrv_loader $@

l_ext2.h: l_ext2.asx
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin nrv_loader $@

l_vmlinz.h: l_vmlinz.asx
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin nrv_loader $@

l_vmlinx.h: l_vmlinx.asx
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin nrv_loader $@

l_vxd.h: l_vxd.asx
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin nrv_loader $@

l_wcle.h: l_wcle.asx
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin nrv_loader $@

l_w32pe.h: l_w32pe.asx
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin nrv_loader $@

# /***********************************************************************
# // atari/tos rules
# ************************************************************************/

l_t_n2b.h: l_tos.s
	$(CPP_M68K) -DNRV2B -o $T.i $<
	$(ASM_M68K) $T.i
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2b_loader $@

l_t_n2bs.h: l_tos.s
	$(CPP_M68K) -DNRV2B -DSMALL -o $T.i $<
	$(ASM_M68K) $T.i
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2b_loader_small $@

l_t_n2d.h: l_tos.s
	$(CPP_M68K) -DNRV2D -o $T.i $<
	$(ASM_M68K) $T.i
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2d_loader $@

l_t_n2ds.h: l_tos.s
	$(CPP_M68K) -DNRV2D -DSMALL -o $T.i $<
	$(ASM_M68K) $T.i
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2d_loader_small $@

l_t_n2e.h: l_tos.s
	$(CPP_M68K) -DNRV2E -o $T.i $<
	$(ASM_M68K) $T.i
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2e_loader $@

l_t_n2es.h: l_tos.s
	$(CPP_M68K) -DNRV2E -DSMALL -o $T.i $<
	$(ASM_M68K) $T.i
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2e_loader_small $@


# /***********************************************************************
# // ps1/exe
# ************************************************************************/

l_ps1b.h: l_ps1.asm
	$(CPP_MR3K) -DCDBOOT -o $T.asx $<
	$(APP_MR3K) $T.asx $T.asy
	$(ASM_MR3K) $T.asy -o$T.bin -l$T.lst
	$(BIN2H) $T.bin nrv_boot_loader $@

l_ps1c.h: l_ps1.asm
	$(CPP_MR3K) -o $T.asx $<
	$(APP_MR3K) $T.asx $T.asy
	$(ASM_MR3K) $T.asy -o$T.bin -l$T.lst
	$(BIN2H) $T.bin nrv_con_loader $@


# /***********************************************************************
# // linux rules (exec, elf, sh, sep)
# ************************************************************************/

upx_itoa.o: upx_itoa.asm
	$(NASM) -f elf -o $@ $<
	$(OBJSTRIP_LINUX_I386) $@

l_lx_elf86.h: l_lx_elf86.asx
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin linux_i386elf_loader $@

l_lx_exec86.h: l_lx_exec86.asx
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin linux_i386exec_loader $@

l_lx_sh86.h: l_lx_sh86.asx
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin linux_i386sh_loader $@

l_lx_pti86.h: l_lx_pti86.asx
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin linux_i386pti_loader $@

l_mac_ppc32.h: l_mac_ppc32.S ppc_d_nrv2e.S
	$(GCC_PPC32) -c $<
	$(OBJSTRIP_PPC32) $T.o
	$(LD_PPC32) -o $T.bin --oformat binary $T.o
	$(BIN2H) $T.bin l_mac_ppc32_loader $@

l_lx_elfppc32.h: l_lx_elfppc32.S ppc_d_nrv2e.S
	$(GCC_PPC32) -c $<
	$(OBJSTRIP_PPC32) $T.o
	$(LD_PPC32) -o $T.bin --oformat binary $T.o
	$(BIN2H) $T.bin linux_elfppc32_loader $@

l_lx_elf64amd.h: l_lx_elf64amd.S amd_d_nrv2e.S
	$(GCC_AMD64) -c $<
	$(OBJSTRIP_AMD64) $T.o
	$(LD_AMD64) -o $T.bin --oformat binary $T.o
	$(BIN2H) $T.bin linux_elf64amd_loader $@

l_lx_elf.o: l_lx_elf.c linux.hh $(MAKEFILE_DEPS)
	$(CC_LINUX_I386) -c $<
	$(OBJSTRIP_LINUX_I386) $@

l_lx_exec.o: l_lx_exec.c linux.hh $(MAKEFILE_DEPS)
	$(CC_LINUX_I386) -c $<
	$(OBJSTRIP_LINUX_I386) $@

l_lx_sep.o: l_lx_sep.c linux.hh $(MAKEFILE_DEPS)
	$(CC_LINUX_I386) -c $<
	$(OBJSTRIP_LINUX_I386) $@

l_lx_sh.o: l_lx_sh.c linux.hh $(MAKEFILE_DEPS)
	$(CC_LINUX_I386) -c $<
	$(OBJSTRIP_LINUX_I386) $@

l_lx_pti.o: l_lx_pti.c linux.hh $(MAKEFILE_DEPS)
	$(CC_LINUX_I386) -c $<
	$(OBJSTRIP_LINUX_I386) $@

fold_elf86.o: fold_elf86.asm
	$(NASM) -f elf -o $@ $<
	$(OBJSTRIP_LINUX_I386) $@

fold_elf86.h: l_lx_elf.o fold_elf86.o l_lx_elf86.lds
	$(LD_LINUX_I386) -T $(srcdir)/l_lx_elf86.lds -Map $T.map -o $T.bin $T.o l_lx_elf.o
	chmod a-x $T.bin
	$(OBJSTRIP_LINUX_I386) $T.bin
	$(STRIPELF_LINUX_I386) $T.bin
	$(BRANDELF) $T.bin
	$(BIN2H) $T.bin linux_i386elf_fold $@

m_lx_elfppc32.o: m_lx_elfppc32.c $(MAKEFILE_DEPS)
	$(GCC_PPC32) -c -Os $<
	$(OBJSTRIP_PPC32) $@

a_lx_elf64amd.o: a_lx_elf64.c $(MAKEFILE_DEPS)
	$(GCC_AMD64) -c -Os -o $@ $<
	$(OBJSTRIP_AMD64) $@

fold_elfppc32.o: fold_elfppc32.S ppc_bxx.S $(MAKEFILE_DEPS)
	$(GCC_PPC32) -c $<
	$(OBJSTRIP_PPC32) $@

fold_elf64amd.o: fold_elf64amd.S amd_bxx.S $(MAKEFILE_DEPS)
	$(GCC_AMD64) -c $<
	$(OBJSTRIP_AMD64) $@

fold_elfppc32.h: m_lx_elfppc32.o fold_elfppc32.o l_lx_elfppc32.lds
	$(LD_PPC32) -T $(srcdir)/l_lx_elfppc32.lds -Map $T.map -o $T.bin --strip-all \
		fold_elfppc32.o m_lx_elfppc32.o
	$(BIN2H) $T.bin linux_elfppc32_fold $@

fold_elf64amd.h: a_lx_elf64amd.o fold_elf64amd.o l_lx_elf64amd.lds amd_bxx.S
	$(LD_AMD64) -T $(srcdir)/l_lx_elf64amd.lds -Map $T.map -o $T.bin --strip-all \
		fold_elf64amd.o a_lx_elf64amd.o
	$(STRIPELF_LINUX_I386) $T.bin
	$(BIN2H) $T.bin linux_elf64amd_fold $@

fold_exec86.o: fold_exec86.asm
	$(NASM) -f elf -o $@ $<
	$(OBJSTRIP_LINUX_I386) $@

fold_exec86.h: l_lx_exec.o upx_itoa.o fold_exec86.o l_lx_exec86.lds
	$(LD_LINUX_I386) -T $(srcdir)/l_lx_exec86.lds -Map $T.map -o $T.bin $T.o l_lx_exec.o upx_itoa.o
	chmod a-x $T.bin
	$(OBJSTRIP_LINUX_I386) $T.bin
	$(STRIPELF_LINUX_I386) $T.bin
	$(BRANDELF) $T.bin
	$(BIN2H) $T.bin linux_i386exec_fold $@

fold_sh86.o: fold_sh86.asm
	$(NASM) -f elf -o $@ $<
	$(OBJSTRIP_LINUX_I386) $@

fold_sh86.h: l_lx_sh.o fold_sh86.o l_lx_sh86.lds
	$(LD_LINUX_I386) -T $(srcdir)/l_lx_sh86.lds -Map $T.map -o $T.bin $T.o l_lx_sh.o
	chmod a-x $T.bin
	$(OBJSTRIP_LINUX_I386) $T.bin
	$(STRIPELF_LINUX_I386) $T.bin
	$(BRANDELF) $T.bin
	$(BIN2H) $T.bin linux_i386sh_fold $@

fold_pti86.o: fold_pti86.asm
	$(NASM) -f elf -i$(UCL_I386)/ -o $@ $<
	$(OBJSTRIP_LINUX_I386) $@

fold_pti86.h: l_lx_pti.o fold_pti86.o l_lx_pti86.lds
	$(LD_LINUX_I386) -T $(srcdir)/l_lx_pti86.lds -Map $T.map -o $T.bin $T.o l_lx_pti.o
	chmod a-x $T.bin
	$(OBJSTRIP_LINUX_I386) $T.bin
	$(STRIPELF_LINUX_I386) $T.bin
	$(BRANDELF) $T.bin
	$(BIN2H) $T.bin linux_i386pti_fold $@

fold_machppc32.h: m_mac_mach32.o fold_machppc32.o
	$(LD_PPC32) -Map $T.map -o $T.bin --oformat binary fold_machppc32.o m_mac_mach32.o
	chmod a-x $T.bin
	$(BIN2H) $T.bin fold_machppc32 $@

m_mac_mach32.o: m_mac_mach32.c $(MAKEFILE_DEPS)
	$(GCC_PPC32) -Os -c $<
	$(OBJSTRIP_PPC32) $@

fold_machppc32.o: fold_machppc32.S ppc_bxx.S $(MAKEFILE_DEPS)
	$(GCC_PPC32) -c $<
	$(OBJSTRIP_PPC32) $@

upxb: l_lx_sep.o l_lx_sep86.asm
	$(NASM) -f elf -i$(UCL_I386)/ -dNRV2B -o $T.o l_lx_sep86.asm
	$(OBJSTRIP_LINUX_I386) $T.o
	$(LD_LINUX_I386) -T $(srcdir)/l_lx_sep86.lds -Map $T.map -o $@ $T.o l_lx_sep.o
	$(OBJSTRIP_LINUX_I386) $@
	$(STRIPELF_LINUX_I386) $@
	$(BRANDELF) $@

upxd: l_lx_sep.o l_lx_sep86.asm
	$(NASM) -f elf -i$(UCL_I386)/ -dNRV2D -o $T.o l_lx_sep86.asm
	$(OBJSTRIP_LINUX_I386) $T.o
	$(LD_LINUX_I386) -T $(srcdir)/l_lx_sep86.lds -Map $T.map -o $@ $T.o l_lx_sep.o
	$(OBJSTRIP_LINUX_I386) $@
	$(STRIPELF_LINUX_I386) $@
	$(BRANDELF) $@

l_armpe.h: l_armpe.asx l_armpe_s.S l_armpe_c.c
	$(GCC_WINCE) -c l_armpe_s.S l_armpe_c.c
	$(LD_WINCE) -nostdlib -o l_armpe_.out l_armpe_s.o l_armpe_c.o
	$(OBJCOPY_WINCE) --only-section .text -O binary l_armpe_.out l_armpe_.bin
	$(BIN2H_WINCE) <l_armpe_.bin >l_armpe_.ah
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin nrv_loader $@


# /***********************************************************************
# // dependencies
# ************************************************************************/

-include *.d

DEPS1 = header.ash macros.ash ident.ash ident_n.ash ident_s.ash
DEPS2 = header.asy macros.asy
DEPS3 = n2b_d32.asy n2d_d32.asy n2e_d32.asy cl1_d32.asy

$(STUBS):       $(srcdir)/scripts/bin2h.pl

l_armpe.h:      $(DEPS2) $(DEPS3)
l_com.h:        n2b_d16.asy  $(DEPS2)
l_djgpp2.h:     $(DEPS2) $(DEPS3)
l_exe.h:        n2b_d8e.asy  n2d_d8e.asy  n2e_d8e.asy  $(DEPS2)
l_sys.h:        n2b_d16.asy  $(DEPS2)
l_t_n2b.h:      n2b_d.ash    bits.ash  $(DEPS1) ../version.h
l_t_n2bs.h:     n2b_d.ash    bits.ash  $(DEPS1) ../version.h
l_t_n2d.h:      n2d_d.ash    bits.ash  $(DEPS1) ../version.h
l_t_n2ds.h:     n2d_d.ash    bits.ash  $(DEPS1) ../version.h
l_t_n2e.h:      n2e_d.ash    bits.ash  $(DEPS1) ../version.h
l_t_n2es.h:     n2e_d.ash    bits.ash  $(DEPS1) ../version.h
l_tmt.h:        $(DEPS2) $(DEPS3)
l_vmlinz.h:     $(DEPS2) $(DEPS3)
l_vmlinx.h:     $(DEPS2) $(DEPS3)
l_vxd.h:        $(DEPS2) $(DEPS3)
l_wcle.h:       $(DEPS2) $(DEPS3)
l_w32pe.h:      $(DEPS2) $(DEPS3)

l_lx_elf86.h:   l_lx_elf86.asm  macros.ash macros.asy $(DEPS3)
l_lx_exec86.h:  l_lx_exec86.asm macros.ash macros.asy $(DEPS3)
l_lx_sh86.h:    l_lx_sh86.asm   macros.ash macros.asy $(DEPS3)


# FIXME: what is this ???
upxrun: start-interp.o interp.o
	gcc -nostartfiles -nostdlib -o $@ -Ttext=0x10000 $^
start-interp.o: start-interp.S
	gcc -c $<
interp.o: interp.c
	gcc -c -g $<


# vi:nowrap
