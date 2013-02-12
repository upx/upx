/*
;  powerpc-linux.kernel.vmlinux.S -- loader & decompressor for the vmlinux/powerpc format
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2013 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2013 Laszlo Molnar
;  Copyright (C) 2004-2013 John Reiser
;  All Rights Reserved.
;
;  UPX and the UCL library are free software; you can redistribute them
;  and/or modify them under the terms of the GNU General Public License as
;  published by the Free Software Foundation; either version 2 of
;  the License, or (at your option) any later version.
;
;  This program is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this program; see the file COPYING.
;  If not, write to the Free Software Foundation, Inc.,
;  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
;
;  Markus F.X.J. Oberhumer              Laszlo Molnar
;  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
;
;  John Reiser
;  <jreiser@users.sourceforge.net>
*/

#include "arch/powerpc/32/macros.S"
#include "arch/powerpc/32/ppc_regs.h"

// offsets in struct b_info
sz_unc   = 0
sz_cpr   = 4
b_method = 8
b_ftid   = 9
b_cto8   = 10
sz_b_info = 12

// ========== ENTRY POINT

section LINUX000  // adjust parameters; call decompressor
        b uncompress
        b unfilter

section LINUX010  // call unfilter
        .long filter_length
        .long filter_cto

section LINUX020  // adjust return value

// ========== UNFILTER

unfilter:
//section ctok32.00
#include "arch/powerpc/32/bxx.S"

// ========== DECOMPRESSION

section LINUX030  // decompressor
uncompress:

SZ_DLINE=128  # size of data cache line in Apple G5

/* register assingments for NRV algorithms */
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


section NRV2B
#include "arch/powerpc/32/nrv2b_d.S"

section NRV2D
#include "arch/powerpc/32/nrv2d_d.S"

section NRV2E
#include "arch/powerpc/32/nrv2e_d.S"

  section NRV_TAIL
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

#undef hibit

#undef src
#undef lsrc
#undef dst
#undef ldst
#undef meth

#undef off
#undef len
#undef bits
#undef disp

section LZMA
#include "arch/powerpc/32/lzma_d.S"

// ========== IDENT

#include "include/header.S"

// vi:ts=8:et:nowrap
