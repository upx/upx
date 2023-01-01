/* powerpc64-darwin.dylib-entry.S -- program entry point & decompress (PowerPC64 dylib)
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
decompress:
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

#undef off
#undef len
#undef bits
#undef disp

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
sz_l_info= 12
sz_p_info= 12
sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8
  b_ftid=   9
  b_cto8=  10

// register numbers during entry
#define f_unc 31
#define f_uini 30
#define l_unm 29
#define a_unm 28
#define r_unc 27
#define r_cpr 26
#define s_unc 25
#define s_cpr 24
#define l_unc 23
#define l_cpr 22
#define t_h   21  /* temporary */

PROT_NONE  =0x00
PROT_READ  =0x01
PROT_WRITE =0x02
PROT_EXEC  =0x04

MAP_SHARED  =0x1
MAP_PRIVATE =0x2
MAP_ANON    =0x1000

SYS_mmap    =197
SYS_munmap=   73
SYS_mprotect= 74

main2:
    //teq r0,r0  // debugging
        stdu r2,-8*(1+ 32-a0)(sp)  # retaddr
//        stmw a0,4*1(sp)
        std  3,3*8-8*2(sp)
        std  4,4*8-8*2(sp)
        std  5,5*8-8*2(sp)
        std  6,6*8-8*2(sp)
        std  7,7*8-8*2(sp)
        std  8,8*8-8*2(sp)
        std  9,9*8-8*2(sp)
        std  10,10*8-8*2(sp)
        std  11,11*8-8*2(sp)
        std  12,12*8-8*2(sp)
        std  13,13*8-8*2(sp)
        std  14,14*8-8*2(sp)
        std  15,15*8-8*2(sp)
        std  16,16*8-8*2(sp)
        std  17,17*8-8*2(sp)
        std  18,18*8-8*2(sp)
        std  19,19*8-8*2(sp)
        std  20,20*8-8*2(sp)
        std  21,21*8-8*2(sp)
        std  22,22*8-8*2(sp)
        std  23,23*8-8*2(sp)
        std  24,24*8-8*2(sp)
        std  25,25*8-8*2(sp)
        std  26,26*8-8*2(sp)
        std  27,27*8-8*2(sp)
        std  28,28*8-8*2(sp)
        std  29,29*8-8*2(sp)
        std  30,30*8-8*2(sp)
        std  31,31*8-8*2(sp)

        mflr f_unc  # f_unc= &decompress
        lwz  t_h, -4*1(f_unc)  # "call main" at _start
        lwz  l_unm,-4*1+ _start - decompress(f_unc)  # 4+ offset(_start)
        rlwinm t_h,t_h,0,6,29  # 4+ main - decompress
        add  l_unm,l_unm,t_h  # offset(main); ASSUMES (8+_start)==decompress
        addi t_h,t_h,-4   # main - decompress

        li  a0,0  # addr
        mr  a1,l_unm  # length for munmap
        li  a2,PROT_READ|PROT_WRITE
        li  a3,MAP_ANON|MAP_PRIVATE
        li  a4,-1
        li  a5,0  # hi32(offset)
        li  a6,0  # lo32(offset)
        li  0,SYS_mmap
        sc
        li a0,-1  # failure
        mr a_unm,a0  # address for munmap


        li   a2,main - movup2
        mtctr a2
        add  a1,a0 ,l_unm  # lwa(dst); new_page + offset(main)
        add  a0,t_h,f_unc  # lwa(src); &main
movup1:  # descending copy [moveup2, main)
        lbzu r0,-1(a0)
        stbu r0,-1(a1)
        bdnz+ movup1

        subf a2,a2,l_unm  # offset(movup2)
        mtlr a1  # &copied movup2
        mtctr a2  # offset(movup2)
        blr  # goto the copied code

movup2:  # descending copy [base, movup2)
        lbzu r0,-1(a0)
        stbu r0,-1(a1)
        bdnz+ movup2

        lwz  f_uini,-4*4+ _start - decompress(f_unc)  # offset(user_init_fn)
        subf f_unc,a0,f_unc
        add  f_unc,a1,f_unc  # relocated decompress
        add  f_uini,f_uini,a0

        lwz  t1,-4*3+ _start - decompress(f_unc)  # offset(b_info)
        add  r_cpr,a1,t1  # &b_info
        add  r_unc,a0,t1  # &b_info
        addi r_unc,r_unc,-sz_l_info -sz_p_info

        // skip compressed Mach headers
        lwz  t1,sz_cpr(r_cpr)
        addi r_cpr,r_cpr,sz_b_info
        add  r_cpr,r_cpr,t1
dy_uncpr:
        mr s_cpr,r_cpr
        mr s_unc,r_unc
        addi a0,r_cpr,sz_unc
        call get4; beq dy_done
        add r_unc,r_unc,a0
        mr l_unc,a0
        addi a0,r_cpr,sz_cpr
        call get4
                                        add r_cpr,r_cpr,a0
        mr l_cpr,a0
        addi r_cpr,r_cpr,sz_b_info

        stdu l_unc,-8(sp)  # keep stack 8-byte aligned
        mtlr f_unc
        addi a0,s_cpr,sz_b_info  # src
        mr a1,l_cpr
        mr a2,s_unc  # dst
        mr a3,sp  # &l_dst
        lbz a4,b_method(s_cpr)
        stdu sp,-SZ_FRAME(sp)
        blrl  # uncompress
        la sp,8+SZ_FRAME(sp)
                // FIXME: check status

        lbz a3,b_ftid(s_cpr)
        cmplwi cr0,a3,0
        beq dy_uncpr
        lbz a2,b_cto8(s_cpr)
        ld a1,sz_unc(s_cpr)
        mr  a0,s_unc
        bl unfilter
        b dy_uncpr

dy_done:
        bl dy_done2
dy_done1:  # escape hatch
        sc  # munmap
        li a0,~0  # failure
//        lmw r0,0(sp)
        ld  2,2*8-8*2(sp)
        ld  3,3*8-8*2(sp)
        ld  4,4*8-8*2(sp)
        ld  5,5*8-8*2(sp)
        ld  6,6*8-8*2(sp)
        ld  7,7*8-8*2(sp)
        ld  8,8*8-8*2(sp)
        ld  9,9*8-8*2(sp)
        ld  10,10*8-8*2(sp)
        ld  11,11*8-8*2(sp)
        ld  12,12*8-8*2(sp)
        ld  13,13*8-8*2(sp)
        ld  14,14*8-8*2(sp)
        ld  15,15*8-8*2(sp)
        ld  16,16*8-8*2(sp)
        ld  17,17*8-8*2(sp)
        ld  18,18*8-8*2(sp)
        ld  19,19*8-8*2(sp)
        ld  20,20*8-8*2(sp)
        ld  21,21*8-8*2(sp)
        ld  22,22*8-8*2(sp)
        ld  23,23*8-8*2(sp)
        ld  24,24*8-8*2(sp)
        ld  25,25*8-8*2(sp)
        ld  26,26*8-8*2(sp)
        ld  27,27*8-8*2(sp)
        ld  28,28*8-8*2(sp)
        ld  29,29*8-8*2(sp)
        ld  30,30*8-8*2(sp)
        ld  31,31*8-8*2(sp)
        addi sp,sp,8*(32-r0)
        mtlr r0  # &continuation in dyld
        bctr  # goto user_init_function
dy_done2:
        li r0,(dy_done2 - dy_done1)/4
        mflr a0
        la a0,dy_done2 - dy_done1(a0)
        mtctr r0
dy_done3:
        lwzu r0,-4(a0)
        stwu r0,-4(s_unc)
        bdnz+ dy_done3

        mtlr s_unc
        mtctr f_uini  # user_init_function
        mr a0,a_unm
        mr a1,l_unm
        li  0,SYS_munmap
        blr  # goto relocated dy_done1

get4:
        lbz t1,3(a0)
        lbz t2,2(a0)
        rlwimi t1,t2, 8,16,23
        lbz t2,1(a0)
        rlwimi t1,t2,16, 8,15
        lbz t2,0(a0)
        rlwimi t1,t2,24, 0, 7
        mr. a0,t1  # set condition codes
        blr

unfilter:
#include "arch/powerpc/64/bxx.S"

main:
        b main2
dy_top:
len_top  = dy_top - main

/* vim:set ts=8 sw=8 et: */
