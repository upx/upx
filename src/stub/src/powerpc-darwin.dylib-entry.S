/*
 *  powerpc-darwin.dylib-entry.S -- program entry point & decompressor (PowerPC32 dylib)
 *
 *  This file is part of the UPX executable compressor.
 *
 *  Copyright (C) 2005-2009 John F. Reiser
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

#include "arch/powerpc/32/macros.S"
#include "arch/powerpc/32/ppc_regs.h"

/*************************************************************************
// We have been CALLed as a subroutine from dyld; C-language rules apply.
// -4*4+_start: .long offset(user_init_function)
// -3*4+_start: .long offset(b_info of compressed Mach_headers) 
// -2*4+_start: .long length(compressed __TEXT)
// -1*4+_start: .long 8+ total_length  # 8+ number of preceding bytes in file
**************************************************************************/

  section MACOS000
_start: .globl _start
        mflr r2
        call main  # must be exactly 1 instruction; link_register= &decompress
decompressor:
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
#include "arch/powerpc/32/nrv2e_d.S"

  section NRV2D
#include "arch/powerpc/32/nrv2d_d.S"

  section NRV2B
#include "arch/powerpc/32/nrv2b_d.S"

#include "arch/powerpc/32/lzma_d.S"

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

  section ELFMAINY
        // IDENTSTR goes here

  section ELFMAINZ
sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

PROT_NONE  =0x00
PROT_READ  =0x01
PROT_WRITE =0x02
PROT_EXEC  =0x04

MAP_SHARED  =0x1
MAP_PRIVATE =0x2
MAP_ANON    =0x1000

SYS_mmap    =197
SYS_mprotect= 74

main2:
    teq r0,r0  // debugging
        stwu r2,-4*(1+ 32-a0)(sp)  # retaddr
        stmw a0,4*1(sp)
        mflr r31  # r31= &decompressor
        lwz  r29, -4*1(r31)  # "call main" at _start
        lwz  r30,-4*1+ _start - decompressor(r31)  # 4+ offset(_start)
        rlwinm r29,r29,0,6,29  # 4+ main - decompressor
        add  r30,r30,r29  # offset(main); ASSUMES (8+_start)==decompressor
        addi r29,r29,-4   # main - decompressor

        li  a0,0  # addr
        mr  a1,r30  # length
        li  a2,PROT_READ|PROT_WRITE
        li  a3,MAP_ANON|MAP_PRIVATE
        li  a4,-1
        li  a5,0  # hi32(offset)
        li  a6,0  # lo32(offset)
        li  0,SYS_mmap
        sc
          li a0,-1  # failure
    teq r0,r0  // debugging


        li   a2,main - movup2
        mtctr a2
        add  a1,a0 ,r30  # lwa(dst); new_page + offset(main)
        add  a0,r29,r31  # lwa(src); &main
movup1:  # descending copy [moveup2, main)
        lbzu r0,-1(a0)
        stbu r0,-1(a1)
        bdnz+ movup1

        subf a2,a2,r30  # offset(movup2)
        mtlr a1  # &copied movup2
        mtctr a2  # offset(movup2)
        blr  # goto the copied code

movup2:  # descending copy [base, movup2)
        lbzu r0,-1(a0)
        stbu r0,-1(a1)
        bdnz+ movup2

        subf r31,a0,r31
        add  r31,a1,r31  # relocated decompressor

    teq r0,r0
main:
        b main2
dy_top:
len_top  = dy_top - main

/*
vi:ts=8:et:nowrap
*/

