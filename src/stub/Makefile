#
# UPX stub Makefile (GNU make)
#
# see http://upx.sourceforge.net/download/tools/
# for required build tools
#

ifeq ($(strip $(UCLDIR)),)
# change this to reflect where the UCL library is
UCLDIR = $(HOME)/local/src/ucl-1.03
endif
ifeq ($(wildcard $(UCLDIR)/upx),)
$(error Please set UCLDIR in Makefile or environment)
endif


# -------------------------------------------------------
# You should not have to change anything below this line.
# -------------------------------------------------------

SHELL = /bin/sh

srcdir = .
top_srcdir = ../..


# These are the files we want to create.
STUBS = \
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
	l_mac_ppc32.h fold_machppc32.h \
	l_vmlinz.h l_vmlinx.h

# experimental:
ifneq ($(strip $(wildcard $(srcdir)/l_ext2.asm)),)
STUBS += l_ext2.h
endif


# util var for use in the rules - basename of the current target
override T = $(basename $@)


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

NASM     = nasm -O15 -w+macro-params -w+macro-selfref -w+number-overflow -w+orphan-labels
NASM    += -I$(srcdir)/

APP_I386 = perl -w $(srcdir)/scripts/app_i386.pl
BIN2H    = perl -w $(srcdir)/scripts/bin2h.pl
##BIN2H    = python  $(srcdir)/scripts/bin2h.py
BRANDELF = perl -w $(srcdir)/scripts/brandelf.pl
O2BIN    = perl -w $(srcdir)/scripts/o2bin.pl
##STRIPELF = perl -w $(srcdir)/scripts/stripelf.pl
STRIPELF = ./util/sstrip/sstrip


###
### linux/i386
###

LD_LINUX_I386 = ld-2.13.2
LD_LINUX_I386 = ld
OBJCOPY_LINUX_I386 = objcopy

# gcc 2.7.2.3 (fold_*86.bin: 1839 / 1011 / xxxx / 1390)
CC_LINUX_I386_GCC272  = gcc-2.72 -m386 -O2 -MMD
CC_LINUX_I386_GCC272 += -malign-functions=0 -malign-jumps=0 -malign-loops=0

# gcc 2.95.3 (fold_*86.bin: 1721 / 995 / xxxx / 1321)
CC_LINUX_I386_GCC295  = gcc-2.95.3 -march=i386 -mcpu=k6 -fno-exceptions -MMD
CC_LINUX_I386_GCC295 += -Os -fno-omit-frame-pointer
CC_LINUX_I386_GCC295 += -malign-functions=0 -malign-jumps=0 -malign-loops=0
CC_LINUX_I386_GCC295 += -Werror

# gcc 3.4.3 (fold_*86.bin: 1624 / 931 / 1544 / 1207)
CC_LINUX_I386_GCC34  = gcc-3.4.3
CC_LINUX_I386_GCC34 += -m32 -march=i386 -mtune=k6 -fno-exceptions -MMD
CC_LINUX_I386_GCC34 += -Os -fno-omit-frame-pointer
CC_LINUX_I386_GCC34 += -momit-leaf-frame-pointer
CC_LINUX_I386_GCC34 += -fno-align-functions -fno-align-jumps -fno-align-labels -fno-align-loops
CC_LINUX_I386_GCC34 += -mpreferred-stack-boundary=2
CC_LINUX_I386_GCC34 += -fweb
CC_LINUX_I386_GCC34 += -Werror

CC_LINUX_I386  = $(CC_LINUX_I386_GCC272)
CC_LINUX_I386  = $(CC_LINUX_I386_GCC295)
CC_LINUX_I386  = $(CC_LINUX_I386_GCC34)
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

GCC_PPC32 := false
LD_PPC32  := false

d = /home2/crosstool/powerpc-750-linux-gnu/gcc-3.4.1-glibc-20040827/bin
ifneq ($(wildcard $d),)
  GCC_PPC32 := $d/powerpc-750-linux-gnu-gcc -m32
  LD_PPC32  := $d/powerpc-750-linux-gnu-ld
endif
d = /opt/cc-i386-linux/crosstool/powerpc-750-linux-gnu/gcc-3.4.3.20050210-glibc-2.2.5/bin
ifneq ($(wildcard $d),)
  GCC_PPC32 := $d/powerpc-750-linux-gnu-gcc -m32
  LD_PPC32  := $d/powerpc-750-linux-gnu-ld
endif


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
	$(OBJCOPY_LINUX_I386) -R .comment -R .note $@

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
	$(LD_PPC32) -o $T.bin --oformat binary $T.o
	$(BIN2H) $T.bin l_mac_ppc32_loader $@

l_lx_elfppc32.h: l_lx_elfppc32.S ppc_d_nrv2e.S
	$(GCC_PPC32) -c $<
	$(LD_PPC32) -o $T.bin --oformat binary $T.o
	$(BIN2H) $T.bin linux_elfppc32_loader $@

l_lx_elf.o: l_lx_elf.c linux.hh
	$(CC_LINUX_I386) -c $<
	$(OBJCOPY_LINUX_I386) -R .comment -R .note $@

l_lx_exec.o: l_lx_exec.c linux.hh
	$(CC_LINUX_I386) -c $<
	$(OBJCOPY_LINUX_I386) -R .comment -R .note $@

l_lx_sep.o: l_lx_sep.c linux.hh
	$(CC_LINUX_I386) -c $<
	$(OBJCOPY_LINUX_I386) -R .comment -R .note $@

l_lx_sh.o: l_lx_sh.c linux.hh
	$(CC_LINUX_I386) -c $<
	$(OBJCOPY_LINUX_I386) -R .comment -R .note $@

l_lx_pti.o: l_lx_pti.c linux.hh
	$(CC_LINUX_I386) -c $<

fold_elf86.o: fold_elf86.asm
	$(NASM) -f elf -o $@ $<
	$(OBJCOPY_LINUX_I386) -R .comment -R .note $@

fold_elf86.h: l_lx_elf.o fold_elf86.o l_lx_elf86.lds
	$(LD_LINUX_I386) -T $(srcdir)/l_lx_elf86.lds -Map $T.map -o $T.bin $T.o l_lx_elf.o
	chmod a-x $T.bin
	$(OBJCOPY_LINUX_I386) -S -R .comment -R .note $T.bin
	$(STRIPELF) $T.bin
	$(BRANDELF) $T.bin
	$(BIN2H) $T.bin linux_i386elf_fold $@

m_lx_elfppc32.o: m_lx_elfppc32.c
	$(GCC_PPC32) -Os -o $@ -c $<

fold_elfppc32.o: fold_elfppc32.S ppc_bxx.S
	$(GCC_PPC32) -c $<

fold_elfppc32.h: m_lx_elfppc32.o fold_elfppc32.o l_lx_elfppc32.lds
	$(LD_PPC32) -T $(srcdir)/l_lx_elfppc32.lds -Map $T.map -o $T.bin --strip-all \
		fold_elfppc32.o m_lx_elfppc32.o
	$(BIN2H) $T.bin linux_elfppc32_fold $@

fold_exec86.o: fold_exec86.asm
	$(NASM) -f elf -o $@ $<
	$(OBJCOPY_LINUX_I386) -R .comment -R .note $@

fold_exec86.h: l_lx_exec.o upx_itoa.o fold_exec86.o l_lx_exec86.lds
	$(LD_LINUX_I386) -T $(srcdir)/l_lx_exec86.lds -Map $T.map -o $T.bin $T.o l_lx_exec.o upx_itoa.o
	chmod a-x $T.bin
	$(OBJCOPY_LINUX_I386) -S -R .comment -R .note $T.bin
	$(STRIPELF) $T.bin
	$(BRANDELF) $T.bin
	$(BIN2H) $T.bin linux_i386exec_fold $@

fold_sh86.o: fold_sh86.asm
	$(NASM) -f elf -o $@ $<
	$(OBJCOPY_LINUX_I386) -R .comment -R .note $@

fold_sh86.h: l_lx_sh.o fold_sh86.o l_lx_sh86.lds
	$(LD_LINUX_I386) -T $(srcdir)/l_lx_sh86.lds -Map $T.map -o $T.bin $T.o l_lx_sh.o
	chmod a-x $T.bin
	$(OBJCOPY_LINUX_I386) -S -R .comment -R .note $T.bin
	$(STRIPELF) $T.bin
	$(BRANDELF) $T.bin
	$(BIN2H) $T.bin linux_i386sh_fold $@

fold_pti86.o: fold_pti86.asm
	$(NASM) -i$(UCL_I386)/ -f elf -o $@ $<

fold_pti86.h: l_lx_pti.o fold_pti86.o l_lx_pti86.lds
	$(LD_LINUX_I386) -T $(srcdir)/l_lx_pti86.lds -Map $T.map -o $T.bin $T.o l_lx_pti.o
	chmod a-x $T.bin
	$(OBJCOPY_LINUX_I386) -S -R .comment -R .note $T.bin
	$(STRIPELF) $T.bin
	$(BRANDELF) $T.bin
	$(BIN2H) $T.bin linux_i386pti_fold $@

fold_machppc32.h: m_mac_mach32.o fold_machppc32.o
	$(LD_PPC32) -Map $T.map -o $T.bin --oformat binary fold_machppc32.o m_mac_mach32.o
	chmod a-x $T.bin
	$(BIN2H) $T.bin fold_machppc32 $@

m_mac_mach32.o: m_mac_mach32.c
	$(GCC_PPC32) -Os -c $<

fold_machppc32.o: fold_machppc32.S ppc_bxx.S
	$(GCC_PPC32) -c $<

upxb: l_lx_sep.o l_lx_sep86.asm
	$(NASM) -i$(UCL_I386)/ -f elf -dNRV2B -o $T.o l_lx_sep86.asm
	$(LD_LINUX_I386) -T $(srcdir)/l_lx_sep86.lds -Map $T.map -o $@ $T.o l_lx_sep.o
	$(OBJCOPY_LINUX_I386) -S -R .comment -R .note $@
	$(STRIPELF) $@
	$(BRANDELF) $@

upxd: l_lx_sep.o l_lx_sep86.asm
	$(NASM) -i$(UCL_I386)/ -f elf -dNRV2D -o $T.o l_lx_sep86.asm
	$(LD_LINUX_I386) -T $(srcdir)/l_lx_sep86.lds -Map $T.map -o $@ $T.o l_lx_sep.o
	$(OBJCOPY_LINUX_I386) -S -R .comment -R .note $@
	$(STRIPELF) $@
	$(BRANDELF) $@


# /***********************************************************************
# // dependencies
# ************************************************************************/

DEPS_FILES := $(wildcard *.d)
ifneq ($(strip $(DEPS_FILES)),)
  include $(DEPS_FILES)
endif

DEPS1 = header.ash macros.ash ident.ash ident_n.ash ident_s.ash
DEPS2 = header.asy macros.asy
DEPS3 = n2b_d32.asy n2d_d32.asy n2e_d32.asy cl1_d32.asy

$(STUBS):       $(srcdir)/scripts/bin2h.pl

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

.NOEXPORT:

upxrun: start-interp.o interp.o
	gcc -nostartfiles -nostdlib -o upxrun -Ttext=0x10000 \
		start-interp.o interp.o

start-interp.o: start-interp.S
	gcc -c $<

interp.o: interp.c
	gcc -c -g $<

# vi:nowrap
