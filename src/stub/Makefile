#
# UPX stub Makefile (GNU make)
#
# see http://upx.sourceforge.net/download/tools/
# for required build tools
#

ifeq ($(strip $(UCLDIR)),)
# change this to reflect where the UCL library is
UCLDIR = $(HOME)/local/src/ucl-1.01
endif


# -------------------------------------------------------
# You should not have to change anything below this line.
# -------------------------------------------------------

SHELL = /bin/sh

top_srcdir = ../..
srcdir = .


# These are the files we want to create.
STUBS = \
	l_com.h \
	l_djgpp2.h stubify.h \
	l_exe.h \
	l_ps1.h \
	l_sys.h \
	l_t_n2b.h l_t_n2bs.h l_t_n2d.h l_t_n2ds.h l_t_n2e.h l_t_n2es.h \
	l_tmt.h \
	l_wcle.h \
	l_w32pe.h \
	l_lx_exec86.h fold_exec86.h \
	l_lx_elf86.h  fold_elf86.h \
	l_lx_sh86.h   fold_sh86.h \
	l_vmlinz.h

# experimental:
ifneq ($(strip $(wildcard $(srcdir)/l_ext2.asm)),)
STUBS += l_ext2.h
endif
ifneq ($(strip $(wildcard $(srcdir)/l_tos2.s)),)
STUBS += l_t_x2b.h l_t_x2bs.h l_t_x2d.h l_t_x2ds.h
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

NASM     = nasm -O2 -w+macro-params -w+orphan-labels
NASM    += -i$(srcdir)/

APP_I386 = perl -w $(srcdir)/scripts/app_i386.pl
BIN2H    = perl -w $(srcdir)/scripts/bin2h.pl
BRANDELF = perl -w $(srcdir)/scripts/brandelf.pl
O2BIN    = perl -w $(srcdir)/scripts/o2bin.pl
##STRIPELF = perl -w $(srcdir)/scripts/stripelf.pl
STRIPELF = ./util/sstrip/sstrip

# Compiler for the Linux/386 stubs.
ifeq (1,2)
  CC_LINUX_I386 = gcc272 -m386 -O2
else
  # gcc 2.95.3 seems to produce the smallest code
  # (specifying -mcpu=i586 inhibits use of 'leave', which costs 2 bytes per subr)
  CC_LINUX_I386 = gcc-2.95 -march=i386 -mcpu=i386 -Os -fno-strict-aliasing
endif
CC_LINUX_I386 += -malign-functions=0 -malign-jumps=0 -malign-loops=0
CC_LINUX_I386 += -Werror
CC_LINUX_I386 += -Wall -W -Wcast-align -Wcast-qual -Wwrite-strings
CC_LINUX_I386 += -funsigned-char
###CC_LINUX_I386 += -fwritable-strings -save-temps

# Motorola 68000
CPP_M68K  = gcc -I$(UCL_UPX) -I$(srcdir) -E -x assembler-with-cpp -Wall -Wp,-P,-C,-traditional
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

# MIPS R3000
CPP_MR3K = gcc -I$(UCL_UPX) -I$(srcdir) -E -x assembler-with-cpp -Wall -Wp,-P,-C,-traditional
APP_MR3K = perl -w $(srcdir)/scripts/app_mr3k.pl
ASM_MR3K = asm5900 --nologo -q


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
	-rm -f *~ *.bin *.bkp *.i *.lst *.map stubify.exe
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
	djasm $< stubify.exe
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

# experimental:
l_t_x2b.h: l_tos2.s
	$(CPP_M68K) -DNRV2B -o $T.i $<
	$(APP_M68K) $T.i $T.asx
	$(ASM_M68K) $T.asx
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2b_loader $@

l_t_x2bs.h: l_tos2.s
	$(CPP_M68K) -DNRV2B -DSMALL -o $T.i $<
	$(APP_M68K) $T.i $T.asx
	$(ASM_M68K) $T.asx
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2b_loader_small $@

l_t_x2d.h: l_tos2.s
	$(CPP_M68K) -DNRV2D -o $T.i $<
	$(APP_M68K) $T.i $T.asx
	$(ASM_M68K) $T.asx
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2d_loader $@

l_t_x2ds.h: l_tos2.s
	$(CPP_M68K) -DNRV2D -DSMALL -o $T.i $<
	$(APP_M68K) $T.i $T.asx
	$(ASM_M68K) $T.asx
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2d_loader_small $@


# /***********************************************************************
# // ps1/exe
# ************************************************************************/

l_ps1.h: l_ps1.asm
	$(CPP_MR3K) -o $T.asx $<
	$(APP_MR3K) $T.asx $T.asy
	$(ASM_MR3K) $T.asy -o$T.bin -l$T.lst
	$(BIN2H) $T.bin nrv_loader $@


# /***********************************************************************
# // linux rules (exec, elf, sh, sep)
# ************************************************************************/

l_lx_elf86.h: l_lx_elf86.asx
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin linux_i386elf_loader $@

l_lx_exec86.h: l_lx_exec86.asx
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin linux_i386exec_loader $@

l_lx_sh86.h: l_lx_sh86.asx
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin linux_i386sh_loader $@

l_lx_elf.o: l_lx_elf.c
	$(CC_LINUX_I386) -c $<

fold_elf86.o: fold_elf86.asm
	$(NASM) -f elf -o $@ $<

fold_elf86.h: l_lx_elf.o fold_elf86.o l_lx_elf86.lds
	ld -T $(srcdir)/l_lx_elf86.lds -Map $T.map -o $T.bin $T.o l_lx_elf.o
	objcopy -S -R .comment -R .note $T.bin
	$(STRIPELF) $T.bin
	$(BRANDELF) $T.bin
	$(BIN2H) $T.bin linux_i386elf_fold $@

l_lx_exec.o: l_lx_exec.c
	$(CC_LINUX_I386) -c $<

fold_exec86.o: fold_exec86.asm
	$(NASM) -f elf -o $@ $<

fold_exec86.h: l_lx_exec.o fold_exec86.o l_lx_exec86.lds
	ld -T $(srcdir)/l_lx_exec86.lds -Map $T.map -o $T.bin $T.o l_lx_exec.o
	objcopy -S -R .comment -R .note $T.bin
	$(STRIPELF) $T.bin
	$(BRANDELF) $T.bin
	$(BIN2H) $T.bin linux_i386exec_fold $@

l_lx_sh.o: l_lx_sh.c
	$(CC_LINUX_I386) -c $<

fold_sh86.o: fold_sh86.asm
	$(NASM) -f elf -o $@ $<

fold_sh86.h: l_lx_sh.o fold_sh86.o l_lx_sh86.lds
	ld -T $(srcdir)/l_lx_sh86.lds -Map $T.map -o $T.bin $T.o l_lx_sh.o
	objcopy -S -R .comment -R .note $T.bin
	$(STRIPELF) $T.bin
	$(BRANDELF) $T.bin
	$(BIN2H) $T.bin linux_i386sh_fold $@

l_lx_sep.o: l_lx_sep.c
	$(CC_LINUX_I386) -c $<

upxb: l_lx_sep.o l_lx_sep86.asm
	$(NASM) -i$(UCL_I386)/ -f elf -dNRV2B -o $T.o l_lx_sep86.asm
	ld -T $(srcdir)/l_lx_sep86.lds -Map $T.map -o $@ $T.o l_lx_sep.o
	objcopy -S -R .comment -R .note $@
	$(STRIPELF) $@
	$(BRANDELF) $@

upxd: l_lx_sep.o l_lx_sep86.asm
	$(NASM) -i$(UCL_I386)/ -f elf -dNRV2D -o $T.o l_lx_sep86.asm
	ld -T $(srcdir)/l_lx_sep86.lds -Map $T.map -o $@ $T.o l_lx_sep.o
	objcopy -S -R .comment -R .note $@
	$(STRIPELF) $@
	$(BRANDELF) $@


# /***********************************************************************
# // dependencies
# ************************************************************************/

DEPS1 = header.ash macros.ash ident.ash ident_n.ash ident_s.ash
DEPS2 = header.asy macros.asy

$(STUBS):       $(srcdir)/scripts/bin2h.pl

l_com.h:        n2b_d16.asy  $(DEPS2)
l_djgpp2.h:     n2b_d32.asy  n2d_d32.asy  n2e_d32.asy  $(DEPS2)
l_exe.h:        n2b_d8e.asy  n2d_d8e.asy  n2e_d8e.asy  $(DEPS2)
l_sys.h:        n2b_d16.asy  $(DEPS2)
l_t_n2b.h:      n2b_d.ash    bits.ash  $(DEPS1)
l_t_n2bs.h:     n2b_d.ash    bits.ash  $(DEPS1)
l_t_n2d.h:      n2d_d.ash    bits.ash  $(DEPS1)
l_t_n2ds.h:     n2d_d.ash    bits.ash  $(DEPS1)
l_t_n2e.h:      n2e_d.ash    bits.ash  $(DEPS1)
l_t_n2es.h:     n2e_d.ash    bits.ash  $(DEPS1)
l_tmt.h:        n2b_d32.asy  n2d_d32.asy  n2e_d32.asy  $(DEPS2)
l_vxd.h:        n2b_d32.asy  n2d_d32.asy  n2e_d32.asy  $(DEPS2)
l_wcle.h:       n2b_d32.asy  n2d_d32.asy  n2e_d32.asy  $(DEPS2)
l_w32pe.h:      n2b_d32.asy  n2d_d32.asy  n2e_d32.asy  $(DEPS2)

l_lx_elf86.h:   l_lx_elf86.asm  macros.ash macros.asy
l_lx_exec86.h:  l_lx_exec86.asm macros.ash macros.asy
l_lx_sh86.h:    l_lx_sh86.asm   macros.ash macros.asy

upxb:           linux.hh
upxd:           linux.hh

.NOEXPORT:

# vi:nowrap
