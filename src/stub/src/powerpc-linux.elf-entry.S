/*  powerpc-linux.elf-entry.S -- Linux program entry point & decompressor (Elf binary)
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2023 Laszlo Molnar
*  Copyright (C) 2000-2023 John F. Reiser
*  All Rights Reserved.
*
*  UPX and the UCL library are free software; you can redistribute them
*  and/or modify them under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; see the file COPYING.
*  If not, write to the Free Software Foundation, Inc.,
*  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
*  Markus F.X.J. Oberhumer              Laszlo Molnar
*  <markus@oberhumer.com>               <ezerotven+github@gmail.com>
*
*  John F. Reiser
*  <jreiser@users.sourceforge.net>
*/

NBPW= 4  // Number of Bytes Per Word
#include "arch/powerpc/32/macros.S"
#include "arch/powerpc/32/ppc_regs.h"
SZ_FRAME= (2 + 2)*NBPW  // (sp,LR, 2 slots)  0 mod 16

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

szElf32_Ehdr= 13*NBPW
p_memsz= 5*NBPW

e_type= 16
  ET_EXEC= 2
  ET_DYN= 3

AT_NULL= 0  // <elf.h>
AT_PAGESZ= 6
a_type= 0
a_val= NBPW
sz_auxv= 2*NBPW

O_RDONLY= 0

PROT_READ=  1
PROT_WRITE= 2
PROT_EXEC=  4

MAP_PRIVATE= 2
MAP_FIXED=     0x10
MAP_ANONYMOUS= 0x20

PAGE_SHIFT= 12
PAGE_SIZE = -(~0<<PAGE_SHIFT)
PAGE_SHIFT64= 16
PAGE_SIZE64 = -(~0<<PAGE_SHIFT64)


/* /usr/include/asm-ppc/unistd.h */
__NR_exit   = 1
__NR_memfd_create= 360
__NR_mmap   = 90
__NR_mprotect = 125
__NR_munmap = 91
__NR_open  = 5
__NR_write = 4

  section ELFMAINX
sz_pack2= -4+ _start
_start: .globl _start
        call main  // link_register= &f_exp  (&decompress)
f_exp:

/* Returns 0 on success; non-zero on failure. */
decompress:  // (uchar const *src, size_t lsrc, uchar *dst, size_t &ldst, uint method)

SZ_DLINE=128  # size of data cache line in Apple G5

/* PowerPC has no 'cmplis': compare logical [unsigned] immediate shifted [by 16] */
#define  hibit r0  /* holds 0x80000000 during decompress */

#define src  a0
#define lsrc a1
#define dst  a2
#define ldst a3  /* Out: actually a reference: &len_dst */
#define meth a4

#define off  a4
#define len  a5
#define bits a6
#define disp a7

#include "arch/powerpc/32/nrv2b_d.S"

eof_nrv:
#define dst0 a4
#define tmp a1
        lwz dst0,0(ldst)  // original dst
        mtlr t3  // return address
        subf a0,lsrc,src
        subf tmp,dst0,dst  // -1+ dst length
        addi a0,a0,1  // return 0: good; else: bad  [+1: correct for lbzu]
        addi tmp,tmp,1  // dst length
        stw  tmp,0(ldst)
#undef tmp

// CACHELINE=32 is the observed minimum line size of any cache.
// Some caches may have larger lines, but it is cumbersome to lookup
// {AT_DCACHEBSIZE, AT_ICACHEBSIZE, AT_UCACHEBSIZE: /usr/include/elf.h},
// then save the correct size in a variable {where to put it?}, or to modify
// the two instructions here.  If a cache has larger lines, then we expect
// that the second dcbst (or icbi) on a the same line will be fast.
// If not, then too bad.

CACHELINE=32
        ori dst0,dst0,-1+ CACHELINE  // highest addr on cache line
cfl_nrv:
        dcbst  0,dst0  // initiate store (modified) cacheline to memory
        cmpl cr0,dst0,dst  // did we cover the highest-addressed byte?
        icbi   0,dst0  // discard instructions from cacheline
        addi     dst0,dst0,CACHELINE  // highest addr on next line
        blt  cr0,cfl_nrv  // not done yet
#undef dst0
        sync   // wait for all memory operations to finish
        isync  // discard prefetched instructions (if any)
cfl_ret:
        ret

  section ELFMAINY
        // IDENTSTR goes here

  section ELFMAINZ
r_exp=   31
r_fp=    30
r_ADRU=  29
r_LENU=  28
r_fd=    27
r_auxv=  26
r_elf=   25
r_ADRX=  24
r_LENX=  23
r_FLD=   22

r_PMASK= 20
r_obinf= 19

        /* Decompress the rest of this loader, and jump to it. */
unfold:  // IN: r_auxv, r_PMASK
        mflr r_FLD  // LrFLD

//Reserve enough space to decompress the folded code of the stub
        lwz r0,  sz_pack2 - f_exp(r_exp)
        la  r_elf,sz_pack2 - f_exp(r_exp)
        sub r_elf,r_elf,r0  // r_elf=&Elf32_Ehdr of stub
        lwz r_LENU, sz_unc + LBINFO - LrFLD(r_FLD)  // sz_unc of fold

        call 0f; .asciz "upx"; 0:
        mflr a0
        li a1,0
SYS_memfd_create= 360
        li r0,SYS_memfd_create; sc
        mr r_fd,a0

        mr r_fp,sp
        li r0,-2*NBPW
        sub sp,sp,r_LENU
        and sp,sp,r0

        lwz r_obinf,      LOBINFO - LrFLD(r_FLD)  // O_BINFO
// Decompress folded code
        lbz meth,b_method + LBINFO - LrFLD(r_FLD)
        stw  r_LENU,SZ_FRAME+31*NBPW(r_fp)  // lzma uses for EOF
        la ldst,    SZ_FRAME+31*NBPW(r_fp)  // &slot on stack
        mr dst,sp  // dst for unfolding
        lwz lsrc,sz_cpr  + LBINFO - LrFLD(r_FLD)
        la src,sz_b_info + LBINFO - LrFLD(r_FLD)  // folded code
        bctrl  // decompress it
        stw r_PMASK,0(sp)  // forward the actual PAGE_MASK

        mr a0,r_fd
        mr a1,sp
        mr a2,r_LENU
SYS_write= 4
        li r0,SYS_write; sc
        mr sp,r_fp

        li a5,0
        mr a4,r_fd
        li a3,MAP_PRIVATE
        li a2,PROT_WRITE|PROT_EXEC|PROT_READ
        mr a1,r_LENU
        li a0,0
SYS_mmap=      90
        li r0,SYS_mmap; sc

// Use the unfolded code
        addi r0,r_ADRU,2*NBPW  // skip get_page_mask()
        add r_ADRX,r_elf,r_obinf  // compressed data
        mtctr r0
        bctr

// Example code at entrypoint of C-language subroutine:
//      mflr    r0  # r0= return address
//      stwu    sp,-96(sp)  # allocate local frame; chain to previous frame
//      stmw    r14,24(sp)  # save 18 regs r14,r15,...,r31; 4*18 == (96 - 24)
//      stw     r0,100(sp)  # save return address into caller's frame (100 >= 96)
// Example code at exit:
//      lwz     r0,100(sp)  # r0= return address
//      lmw     r14,24(sp)  # restore 18 regs r14,r15,...,r31
//      mtlr    r0  # prepare for indirect jump
//      addi    sp,sp,96  # de-allocate local frame
//      blr  # goto return address

mmapRW0:
        li a5,0  // offset
mmapRW:
        li a2,PROT_READ|PROT_WRITE
mmap:
        li r0,__NR_mmap; sc; bns 0f; teq r0,r0; 0:
        ret

zfind:
        lwz r0,0(a0); addi a0,a0,NBPW
        cmpi cr7,r0,0; bne+ cr7,zfind
        ret

main:
        stwu r1,-(SZ_FRAME+32*NBPW)(sp)  // allocate space (keeping 0 mod 16), chain r1
        stmw r2,  SZ_FRAME+ 2*NBPW - NBPW(sp) // save registers r2 thru r31
        mflr r_exp  // &f_exp (decompress)

        la a0,SZ_FRAME+32*NBPW(sp)  // &argc
        call zfind  // a0= envp
        call zfind  // a0= &Elf32_auxv
        mr r_auxv,a0  // save for folded code

// set r_PMASK by finding actual page size in Elf32_auxv_t
1:
        lwz r0,a_type(a0); lwz a1,a_val(a0); addi a0,a0,sz_auxv
        cmpi cr7,r0,AT_PAGESZ; beq- cr7,2f
        cmpi cr0,r0,AT_NULL;   bne+ cr0,1b
        li a1,PAGE_SIZE  // not found; use default
2:
        neg r_PMASK,a1  // save for folded code

        call unfold
LrFLD:
LOBINFO:
        .long O_BINFO
LBINFO:
        /* { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...} */

/* vim:set ts=8 sw=8 et: */
