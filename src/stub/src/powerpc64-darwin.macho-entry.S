/* powerpc64-darwin.macho-entry.S -- program entry point & decompressor (PowerPC64 Mach-o)
 *
 *  This file is part of the UPX executable compressor.
 *
 *  Copyright (C) 2005-2023 John F. Reiser
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
 *  John F. Reiser
 *  <jreiser@users.sourceforge.net>
 *
 */

#ifndef BIG_ENDIAN  //{
#define BIG_ENDIAN 1  /* Apple on PowerPC* is BIG_ENDIAN */
#endif  //}
NBPW= 8  // Number of Bytes Per Word

#include "arch/powerpc/64/macros.S"
#include "arch/powerpc/64/ppc_regs.h"
LINKAREA= 6*8

  section MACOS000
_start: .globl _start
        call main  # must be exactly 1 instruction; link_register= &decompress

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
#include "arch/powerpc/64/nrv2e_d.S"

  section NRV2D
#include "arch/powerpc/64/nrv2d_d.S"

  section NRV2B
#include "arch/powerpc/64/nrv2b_d.S"

#include "arch/powerpc/64/lzma_d.S"

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
        cmpld cr0,dst0,dst  // did we cover the highest-addressed byte?
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
sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

/* Decompress the rest of this loader, and jump to it. */
unfold:
        mflr t1  # -(L105-L100)+ &{ b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...}
        //lwz t2,4(t1)  // O_BINFO
        //stdu t2,-8(sp)

        lwz lsrc,L105-L100+sz_cpr(t1)
        mtctr lsrc  # length to copy (and decompress)
        lwz ldst,L105-L100+sz_unc(t1)
        lbz meth,L105-L100+b_method(t1)
        add  dst,lsrc,t1
        addi dst,dst,L105-L100+sz_b_info
        add  src,ldst,t1
        addi src,src,GAP+64  # defend against prefetch and overlap
movup:  # descending copy moves folded_loader to higher address
        lbzu r0,-1(dst)
        stbu r0,-1(src)
        bdnz+ movup  # typical count is about 0x4cb(1227) bytes

        mtctr r31  # &decompress
        addi dst,t1,GAP  # &unfolded result
        std ldst,-8(sp)  # LZMA needs for EOF
        la  ldst,-8(sp)  # &sz_result
        stdu sp,-SZ_FRAME(sp)  // ABI convention (for lzma compiled by gcc)
        bctr  # call decompress: branch to counter register, return to link register

main:
////    teq r0,r0  // debugging
        mflr r31  # r31= &decompress
        call unfold
L100: GAP= 128  # > farthest_prefetch; must match ../p_mach.cpp
        b GAP+L100  # 'isync' has trouble on Macintosh G4?
        .long 0  // O_BINFO
L105:
        /* { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...} */

/* vim:set ts=8 sw=8 et: */
