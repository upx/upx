/* powerpc64le-linux.elf-entry.S -- Linux program entry point & decompressor (Elf binary)
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2017 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2017 Laszlo Molnar
*  Copyright (C) 2000-2017 John F. Reiser
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

#include "arch/powerpc/64le/macros.S"
#include "arch/powerpc/64le/ppc_regs.h"

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

PROT_READ=  1
PROT_WRITE= 2
PROT_EXEC=  4

MAP_PRIVATE= 2
MAP_FIXED=     0x10
MAP_ANONYMOUS= 0x20

PAGE_SHIFT= 16
PAGE_SIZE = -(~0<<PAGE_SHIFT)

/* /usr/include/asm-ppc/unistd.h */
__NR_write = 4
__NR_exit  = 1
__NR_mmap  = 90

  section ELFMAINX
_start: .globl _start
        call main  // must be exactly 1 instruction; link_register= &decompress

/* Returns 0 on success; non-zero on failure. */
decompress:  // (uchar const *src, size_t lsrc, uchar *dst, size_t &ldst, uint method)

  section NRV_HEAD
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

  section NRV2E
#include "arch/powerpc/64le/nrv2e_d.S"

  section NRV2D
#include "arch/powerpc/64le/nrv2d_d.S"

  section NRV2B
#include "arch/powerpc/64le/nrv2b_d.S"

#include "arch/powerpc/64le/lzma_d.S"

  section NRV_TAIL
eof_nrv:
#define dst0 a4
#define tmp a1
        ld dst0,0(ldst)  // original dst
        mtlr t3  // return address
        subf a0,lsrc,src
        subf tmp,dst0,dst  // -1+ dst length
        addi a0,a0,1  // return 0: good; else: bad  [+1: correct for lbzu]
        addi tmp,tmp,1  // dst length
        std  tmp,0(ldst)
#undef tmp

// CACHELINE=32 is the observed minimum line size of any cache.
// Some caches may have larger lines, but it is cumbersome to lookup
// {AT_DCACHEBSIZE, AT_ICACHEBSIZE, AT_UCACHEBSIZE: /usr/include/elf.h},
// then save the correct size in a variable {where to put it?}, or to modify
// the two instructions here.  If a cache has larger lines, then we expect
// that the second dcbst (or icbi) on a the same line will be fast.
// If not, then too bad.

  section CFLUSH  // In: a2=dst= &highest stored byte; a4=dst0= &lowest stored byte
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
msg_SELinux:
        call L72
L70:
        .asciz "PROT_EXEC|PROT_WRITE failed.\n"
L71:
        // IDENTSTR goes here

  section ELFMAINZ
L72:
        li   a2,L71 - L70  // length
        mflr a1    // message text
        li   a0,2  // fd stderr
        li 0,__NR_write; sc
die:
        li a0,127
        li 0,__NR_exit; sc

        /* Decompress the rest of this loader, and jump to it. */
unfold:
        mflr r30  // &{ b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...}

        li a5,0  // off_t
        li a4,-1  // fd; cater to *BSD for MAP_ANON
        lwz  a0,sz_cpr(r30)        // size < 1 Page
        li a3,MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS
        li a2,PROT_READ | PROT_WRITE | PROT_EXEC
//        li a1,PAGE_SIZE
        lis        a1,1
        add  a0,a0,r30
        li 0,__NR_mmap
//          addi a0,a0,sz_b_info+PAGE_SIZE-1
        addi a0,a0,sz_b_info-1
        add  a0,a0,a1
        rlwinm a0,a0,0,0,31-PAGE_SHIFT  // next page boundary after fold
        sc; bso- msg_SELinux  // Branch if SummaryOverflow (failure)
0:
        mtctr r31
        lwz r0,sz_unc(r30)        // size uncompressed
        lbz meth,b_method(r30)
        la ldst,31*8(sp)  // &slot on stack
        std  r0,31*8(sp)  // lzma uses for EOF
        mr dst,a0
        mtlr   a0  // &continuation
        lwz lsrc,sz_cpr(r30)        // size of compressed
        addi src,r30,sz_b_info
        la sp,-6*8(sp)  // (sp,pc,cr, xx,yy,zz) save area per calling convention
        bctr  // goto decomrpess; return to link register (mmap'ed page)

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

main:
////  teq r0,r0  // debugging
        stdu r1,-32*8(sp)  // allocate space (keeping 0 mod 16), save r1
//        stm  r2,8(sp) // save registers r2 thru r31 - 32bits save
        std  2,2*8-8(sp)
        std  3,3*8-8(sp)
        std  4,4*8-8(sp)
        std  5,5*8-8(sp)
        std  6,6*8-8(sp)
        std  7,7*8-8(sp)
        std  8,8*8-8(sp)
        std  9,9*8-8(sp)
        std  10,10*8-8(sp)
        std  11,11*8-8(sp)
        std  12,12*8-8(sp)
        std  13,13*8-8(sp)
        std  14,14*8-8(sp)
        std  15,15*8-8(sp)
        std  16,16*8-8(sp)
        std  17,17*8-8(sp)
        std  18,18*8-8(sp)
        std  19,19*8-8(sp)
        std  20,20*8-8(sp)
        std  21,21*8-8(sp)
        std  22,22*8-8(sp)
        std  23,23*8-8(sp)
        std  24,24*8-8(sp)
        std  25,25*8-8(sp)
        std  26,26*8-8(sp)
        std  27,27*8-8(sp)
        std  28,28*8-8(sp)
        std  29,29*8-8(sp)
        std  30,30*8-8(sp)
        std  31,31*8-8(sp)
        mflr r31  // &decompress
        call unfold
        /* { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...} */

/* vim:set ts=8 sw=8 et: */
