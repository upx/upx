#
# UPX stub Makefile (GNU make)
#
# see http://wildsau.idv.uni-linz.ac.at/mfx/download/upx/tools/
# for required support tools
#

ifeq ($(strip $(UCLDIR)),)
# change this to reflect where the UCL library is
UCLDIR = $(HOME)/local/src/ucl-0.92
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
	l_sys.h \
	l_t_n2b.h l_t_n2bs.h l_t_n2d.h l_t_n2ds.h \
	l_tmt.h \
	l_wcle.h \
	l_w32pe.h \
	l_lx_exec86.h fold_exec86.h \
	l_lx_elf86.h  fold_elf86.h \
	l_lx_sh86.h   fold_sh86.h \
	l_vmlinz.h

# experimental:
ifneq ($(strip $(wildcard l_ext2.asm)),)
STUBS += l_ext2.h
endif
ifneq ($(strip $(wildcard l_tos2.s)),)
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

.SUFFIXES:
.SUFFIXES: .asm .ash .asx .asy .bin .c .h .s

vpath %.ash $(UCL_I386)
vpath %.ash $(UCL_M68K)


# /***********************************************************************
# // tools
# ************************************************************************/

NASM     = /usr/topics/asm/nasm-20000903/nasm -O0 -w+macro-params -w+orphan-labels
NASM     = /usr/topics/asm/nasm-20000903/nasm -O2 -w+macro-params -w+orphan-labels
NASM     = nasm -w+macro-params -w+orphan-labels
NASM    += -i$(srcdir)/

APP      = perl -w $(srcdir)/scripts/app.pl
BIN2H    = perl -w $(srcdir)/scripts/bin2h.pl
BRANDELF = perl -w $(srcdir)/scripts/brandelf.pl
O2BIN    = perl -w $(srcdir)/scripts/o2bin.pl
##SETFOLD  = /bin/sh $(srcdir)/scripts/setfold.sh
##STRIPELF = perl -w $(srcdir)/scripts/stripelf.pl
STRIPELF = ./util/sstrip/sstrip

# Use gcc 2.95.2 for smallest code.
CC_LINUX_CFLAGS = -Wall -W -Wcast-align -Wcast-qual -Wwrite-strings
CC_LINUX_CFLAGS += -Werror
CC_LINUX_CFLAGS += -funsigned-char
###CC_LINUX_CFLAGS += -fwritable-strings -save-temps
CC_LINUX = gcc272 -O2 -m386                  -malign-functions=0 -malign-jumps=0 -malign-loops=0 $(CC_LINUX_CFLAGS)
CC_LINUX = gcc    -Os -march=i386 -mcpu=i386 -malign-functions=0 -malign-jumps=0 -malign-loops=0 $(CC_LINUX_CFLAGS)
# Specifying -mcpu=i586 inhibits use of 'leave', which costs 2 bytes per subr
#CC_LINUX = gcc    -Os -march=i386 -mcpu=i586 -malign-functions=0 -malign-jumps=0 -malign-loops=0 $(CC_LINUX_CFLAGS)

ifeq (1,1)
# Preprocessor for the a68k 68000-assembler.
CPP_68K = gcc -I$(UCL_UPX) -E -x assembler-with-cpp -Wall -Wp,-P,-C,-traditional -D__A68K__
##CPP_68K = cpp -I$(UCL_UPX) -x assembler-with-cpp -P -C -traditional -nostdinc -D__A68K__
APP_68K  = perl -w $(srcdir)/scripts/app_68k.pl
ASM_68K = a68k -q -x
else
# Preprocessor for the asl 68000-assembler.
CPP_68K = gcc -I$(UCL_UPX) -E -x assembler-with-cpp -Wall -Wp,-P,-C,-traditional -D__ASL__
APP_68K  = perl -w $(srcdir)/scripts/app_68k.pl
ASM_68K = sh $(srcdir)/scripts/asl_68k.sh
endif


# /***********************************************************************
# // main targets
# ************************************************************************/

.PHONY: default all stubs mostlyclean clean distclean maintainer-clean ident strings

default:
	@echo "UPX info: type 'make all' if you have all the needed build tools."

all: stubs ## upxb upxd

stubs: $(STUBS)

mostlyclean:
	-rm -f *~ *.bin *.bkp *.i *.lst *.map stubify.exe

clean: mostlyclean
	-rm -f *.o *.asx *.asy

distclean: clean

# This command is intended for maintainers to use; it deletes files
# that may require special tools to rebuild.
maintainer-clean: distclean
	-rm -f $(STUBS) upxb upxd

ident: all
	ident *.bin

strings: all
	strings *.bin


# /***********************************************************************
# // rules
# ************************************************************************/

.asm.asx:
	$(APP) $< $@

.ash.asy:
	$(APP) $< $@


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
	$(CPP_68K) -DNRV2B -o $T.i $<
	$(ASM_68K) $T.i
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2b_loader $@

l_t_n2bs.h: l_tos.s
	$(CPP_68K) -DNRV2B -DSMALL -o $T.i $<
	$(ASM_68K) $T.i
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2b_loader_small $@

l_t_n2d.h: l_tos.s
	$(CPP_68K) -DNRV2D -o $T.i $<
	$(ASM_68K) $T.i
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2d_loader $@

l_t_n2ds.h: l_tos.s
	$(CPP_68K) -DNRV2D -DSMALL -o $T.i $<
	$(ASM_68K) $T.i
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2d_loader_small $@

# experimental:
l_t_x2b.h: l_tos2.s
	$(CPP_68K) -DNRV2B -o $T.i $<
	$(APP_68K) $T.i $T.asx
	$(ASM_68K) $T.asx
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2b_loader $@

l_t_x2bs.h: l_tos2.s
	$(CPP_68K) -DNRV2B -DSMALL -o $T.i $<
	$(APP_68K) $T.i $T.asx
	$(ASM_68K) $T.asx
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2b_loader_small $@

l_t_x2d.h: l_tos2.s
	$(CPP_68K) -DNRV2D -o $T.i $<
	$(APP_68K) $T.i $T.asx
	$(ASM_68K) $T.asx
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2d_loader $@

l_t_x2ds.h: l_tos2.s
	$(CPP_68K) -DNRV2D -DSMALL -o $T.i $<
	$(APP_68K) $T.i $T.asx
	$(ASM_68K) $T.asx
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2d_loader_small $@


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
	$(CC_LINUX) -c $<

fold_elf86.o: fold_elf86.asm
	$(NASM) -f elf -o $@ $<

fold_elf86.h: l_lx_elf.o fold_elf86.o l_lx_elf86.lds
	ld -T $(srcdir)/l_lx_elf86.lds -Map $T.map -o $T.bin $T.o l_lx_elf.o
	objcopy -S -R .comment -R .note $T.bin
	./util/sstrip/sstrip $T.bin
	$(BRANDELF) $T.bin
	$(BIN2H) $T.bin linux_i386elf_fold $@

l_lx_exec.o: l_lx_exec.c
	$(CC_LINUX) -c $<

fold_exec86.o: fold_exec86.asm
	$(NASM) -f elf -o $@ $<

fold_exec86.h: l_lx_exec.o fold_exec86.o l_lx_exec86.lds
	ld -T $(srcdir)/l_lx_exec86.lds -Map $T.map -o $T.bin $T.o l_lx_exec.o
	objcopy -S -R .comment -R .note $T.bin
	./util/sstrip/sstrip $T.bin
	$(BRANDELF) $T.bin
	$(BIN2H) $T.bin linux_i386exec_fold $@

l_lx_sh.o: l_lx_sh.c
	$(CC_LINUX) -c $<

fold_sh86.o: fold_sh86.asm
	$(NASM) -f elf -o $@ $<

fold_sh86.h: l_lx_sh.o fold_sh86.o l_lx_sh86.lds
	ld -T $(srcdir)/l_lx_sh86.lds -Map $T.map -o $T.bin $T.o l_lx_sh.o
	objcopy -S -R .comment -R .note $T.bin
	./util/sstrip/sstrip $T.bin
	$(BRANDELF) $T.bin
	$(BIN2H) $T.bin linux_i386sh_fold $@

l_lx_sep.o: l_lx_sep.c
	$(CC_LINUX) -c $<

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

l_com.h:        n2b_d16.asy  $(DEPS2)
l_djgpp2.h:     n2b_d32.asy  n2d_d32.asy  $(DEPS2)
l_exe.h:        n2b_d8e.asy  n2d_d8e.asy  $(DEPS2)
l_sys.h:        n2b_d16.asy  $(DEPS2)
l_t_n2b.h:      n2b_d.ash    bits.ash     $(DEPS1)
l_t_n2bs.h:     n2b_d.ash    bits.ash     $(DEPS1)
l_t_n2d.h:      n2d_d.ash    bits.ash     $(DEPS1)
l_t_n2ds.h:     n2d_d.ash    bits.ash     $(DEPS1)
l_tmt.h:        n2b_d32.asy  n2d_d32.asy  $(DEPS2)
l_ext2.h:       n2b_d32.asy  n2d_d32.asy  $(DEPS2)
l_vmlinz.h:     n2b_d32.asy  n2d_d32.asy  $(DEPS2)
l_vxd.h:        n2b_d32.asy  n2d_d32.asy  $(DEPS2)
l_wcle.h:       n2b_d32.asy  n2d_d32.asy  $(DEPS2)
l_w32pe.h:      n2b_d32.asy  n2d_d32.asy  $(DEPS2)

l_lx_elf86.asx:  l_lx_elf86.asm  macros.ash macros.asy
l_lx_exec86.asx: l_lx_exec86.asm macros.ash macros.asy
l_lx_sh86.asx:    l_lx_sh86.asm  macros.ash macros.asy

upxb:           linux.hh
upxd:           linux.hh

.NOEXPORT:

# vi:nowrap
