#
# UPX stub Makefile (GNU make)
#

ifeq ($(strip $(UCLDIR)),)
# change this to reflect where the UCL library is
UCLDIR = $(HOME)/local/src/ucl-0.91
endif


# -------------------------------------------------------
# You should not have to change anything below this line.
# -------------------------------------------------------

SHELL = /bin/sh

top_srcdir = ../..


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
	l_lx_n2b.h l_lx_n2d.h \
	l_le_n2b.h l_le_n2d.h \
	l_sh_n2b.h l_sh_n2d.h


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

NASM     = nasm -w+macro-params -w+orphan-labels

APP      = perl -w scripts/app.pl
BIN2H    = perl -w scripts/bin2h.pl
BRANDELF = perl -w scripts/brandelf.pl
O2BIN    = perl -w scripts/o2bin.pl
SETFOLD  = /bin/sh scripts/setfold.sh
##STRIPELF = perl -w scripts/stripelf.pl
STRIPELF = ./util/sstrip/sstrip

# Preprocessor for a68k assembler.
CPP_M68K  = gcc -I$(UCL_UPX) -E -x assembler-with-cpp -Wall -Wp,-P,-C,-traditional

# Use gcc 2.95.2 for smallest code.
CC_LINUX_CFLAGS = -Wall -W -Wcast-align -Wcast-qual -Wwrite-strings
CC_LINUX_CFLAGS += -funsigned-char
###CC_LINUX_CFLAGS += -fwritable-strings -save-temps
CC_LINUX = gcc272 -O2 -m386                  -malign-functions=0 -malign-jumps=0 -malign-loops=0 $(CC_LINUX_CFLAGS)
CC_LINUX = gcc    -Os -march=i386 -mcpu=i386 -malign-functions=0 -malign-jumps=0 -malign-loops=0 $(CC_LINUX_CFLAGS)
# Specifying -mcpu=i586 inhibits use of 'leave', which costs 2 bytes per subr
#CC_LINUX =gcc    -Os -march=i386 -mcpu=i586 -malign-functions=0 -malign-jumps=0 -malign-loops=0 $(CC_LINUX_CFLAGS)


# /***********************************************************************
# // main targets
# ************************************************************************/

.PHONY: default all stubs mostlyclean clean distclean maintainer-clean ident strings

default:
	@echo "UPX info: type 'make all' if you have all the needed build tools."

all: stubs upxb upxd

stubs: $(STUBS)

mostlyclean:
	-rm -f *~ *.bin *.bkp *.i *.lst *.map

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
	djasm $< $@

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

l_t_n2b.h: l_tos.s
	$(CPP_M68K) -D__A68K__ -DNRV2B -o $T.i $<
	a68k -q -x $T.i
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2b_loader $@

l_t_n2bs.h: l_tos.s
	$(CPP_M68K) -D__A68K__ -DNRV2B -DSMALL -o $T.i $<
	a68k -q -x $T.i
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2b_loader_small $@

l_t_n2d.h: l_tos.s
	$(CPP_M68K) -D__A68K__ -DNRV2D -o $T.i $<
	a68k -q -x $T.i
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2d_loader $@

l_t_n2ds.h: l_tos.s
	$(CPP_M68K) -D__A68K__ -DNRV2D -DSMALL -o $T.i $<
	a68k -q -x $T.i
	$(O2BIN) $T.o $T.bin 'UPX1' 'UPX9'
	$(BIN2H) $T.bin nrv2d_loader_small $@

l_vxd.h: l_vxd.asm
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin nrv_loader $@

l_wcle.h: l_wcle.asx
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin nrv_loader $@

l_w32pe.h: l_w32pe.asx
	$(NASM) -f bin -o $T.bin $<
	$(BIN2H) $T.bin nrv_loader $@


# /***********************************************************************
# // linux rules (exec, elf, sh, sep)
# ************************************************************************/

l_lx_n2b.h: l_lx_exec.c l_xe_n2b.o l_lx_exec86.lds Makefile
	$(CC_LINUX) -DNRV2B -o $T.o -c $<
	ld -T l_lx_exec86.lds -Map $T.map -o $T.bin \
		l_xe_n2b.o $T.o
	$(SETFOLD) $T.bin
	$(BRANDELF) $T.bin
	$(BIN2H) $T.bin linux_i386exec_nrv2b_loader $@

l_le_n2b.h: l_lx_elf.c l_6e_n2b.o l_lx_elf86.lds
	$(CC_LINUX) -DNRV2B -o $T.o -c $<
	ld -T l_lx_elf86.lds -Map $T.map -o $T.bin \
		l_6e_n2b.o $T.o
	$(SETFOLD) $T.bin
	$(BRANDELF) $T.bin
	$(BIN2H) $T.bin linux_i386elf_nrv2b_loader $@

l_sh_n2b.h: l_lx_sh.c l_6h_n2b.o l_lx_sh86.lds
	$(CC_LINUX) -DNRV2B -o $T.o -c $<
	ld -T l_lx_sh86.lds -Map $T.map -o $T.bin \
		l_6h_n2b.o $T.o
	$(SETFOLD) $T.bin
	$(BRANDELF) $T.bin
	$(BIN2H) $T.bin linux_i386sh_nrv2b_loader $@

l_xe_n2b.o: l_lx_exec86.asm
	$(NASM) -i$(UCL_I386)/ -f elf -dNRV2B -o $@ $<

l_6e_n2b.o: l_lx_elf86.asm
	$(NASM) -i$(UCL_I386)/ -f elf -dNRV2B -o $@ $<

l_6h_n2b.o: l_lx_sh86.asm
	$(NASM) -i$(UCL_I386)/ -f elf -dNRV2B -o $@ $<


l_lx_n2d.h: l_lx_exec.c l_xe_n2d.o l_lx_exec86.lds
	$(CC_LINUX) -DNRV2D -o $T.o -c $<
	ld -T l_lx_exec86.lds -Map $T.map -o $T.bin \
		l_xe_n2d.o $T.o
	$(SETFOLD) $T.bin
	$(BRANDELF) $T.bin
	$(BIN2H) $T.bin linux_i386exec_nrv2d_loader $@

l_le_n2d.h: l_lx_elf.c l_6e_n2d.o l_lx_elf86.lds
	$(CC_LINUX) -DNRV2D -o $T.o -c $<
	ld -T l_lx_elf86.lds -Map $T.map -o $T.bin \
		l_6e_n2d.o $T.o
	$(SETFOLD) $T.bin
	$(BRANDELF) $T.bin
	$(BIN2H) $T.bin linux_i386elf_nrv2d_loader $@

l_sh_n2d.h: l_lx_sh.c l_6h_n2d.o l_lx_sh86.lds
	$(CC_LINUX) -DNRV2D -o $T.o -c $<
	ld -T l_lx_sh86.lds -Map $T.map -o $T.bin \
		l_6h_n2d.o $T.o
	$(SETFOLD) $T.bin
	$(BRANDELF) $T.bin
	$(BIN2H) $T.bin linux_i386sh_nrv2d_loader $@

l_xe_n2d.o: l_lx_exec86.asm
	$(NASM) -i$(UCL_I386)/ -f elf -dNRV2D -o $@ $<

l_6e_n2d.o: l_lx_elf86.asm
	$(NASM) -i$(UCL_I386)/ -f elf -dNRV2D -o $@ $<

l_6h_n2d.o: l_lx_sh86.asm
	$(NASM) -i$(UCL_I386)/ -f elf -dNRV2D -o $@ $<

l_lx_sep.o: l_lx_sep.c
	$(CC_LINUX) -c $<

upxb: l_lx_sep.o l_lx_sep86.asm
	$(NASM) -i$(UCL_I386)/ -f elf -dNRV2B -o $T.o l_lx_sep86.asm
	ld -T l_lx_sep86.lds -Map $T.map -o $@ $T.o l_lx_sep.o
	objcopy -S -R .comment -R .note $@
	$(STRIPELF) $@
	$(BRANDELF) $@

upxd: l_lx_sep.o l_lx_sep86.asm
	$(NASM) -i$(UCL_I386)/ -f elf -dNRV2D -o $T.o l_lx_sep86.asm
	ld -T l_lx_sep86.lds -Map $T.map -o $@ $T.o l_lx_sep.o
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
l_t_n2b.h:      n2b_d.ash    bits.ash  $(DEPS1)
l_t_n2bs.h:     n2b_d.ash    bits.ash  $(DEPS1)
l_t_n2d.h:      n2d_d.ash    bits.ash  $(DEPS1)
l_t_n2ds.h:     n2d_d.ash    bits.ash  $(DEPS1)
l_tmt.h:        n2b_d32.asy  n2d_d32.asy  $(DEPS2)
l_vxd.h:        n2b_d32.asy  n2d_d32.asy  $(DEPS2)
l_wcle.h:       n2b_d32.asy  n2d_d32.asy  $(DEPS2)
l_w32pe.h:      n2b_d32.asy  n2d_d32.asy  $(DEPS2)

l_xe_n2b.o:     n2b_d32.ash  $(DEPS1)
l_6e_n2b.o:     n2b_d32.ash  $(DEPS1)
l_6h_n2b.o:     n2b_d32.ash  $(DEPS1)

l_xe_n2d.o:     n2d_d32.ash  $(DEPS1)
l_6e_n2d.o:     n2d_d32.ash  $(DEPS1)
l_6h_n2d.o:     n2d_d32.ash  $(DEPS1)

l_lx_n2b.h:     linux.hh
l_lx_n2d.h:     linux.hh
l_le_n2b.h:     linux.hh
l_le_n2d.h:     linux.hh
l_sh_n2b.h:     linux.hh
l_sh_n2d.h:     linux.hh
upxb:           linux.hh
upxd:           linux.hh

.NOEXPORT:

# vi:nowrap
