/*
; mips.r3000-expand.S -- decompressors for mips R3000
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2021 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2021 Laszlo Molnar
;  Copyright (C) 2000-2021 John F. Reiser
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
;  <markus@oberhumer.com>               <ezerotven+github@gmail.com>
;
;  John F. Reiser
;  <jreiser@users.sourceforge.net>
;
*/

    .set noreorder  # we handle branch delay sots explicitly
    .set noat  # assumbler should not use $at

NBPW= 4
SZ_DLINE= 128  # size of data cache line in Apple G5
#ifndef BIG_ENDIAN  //{
#define BIG_ENDIAN 1
#endif  //}

#include "arch/mips/r3000/macros.ash"
#include "arch/mips/r3000/bits.ash"

/* These from /usr/include/asm/unistd.h */
__NR_cacheflush = 147+ __NR_Linux

/* asm/cachectl.h */
ICACHE= 1<<0
DCACHE= 1<<1

  section EXP_HEAD

sz_unc= 0
sz_cpr= 4
b_method= 8
b_ftid=   9
b_cto8=  10
b_extra= 11
sz_binfo= 12

f_expand: .globl f_expand // f_expand(b_info *, dst, &dstlen)
    .type f_expand,function
// Supervisor for de-compression, un-filter, and sync_cache
// Input to supervisor:
#define fx_src    a0
#define fx_dst    a1
#define fx_dstlen a2
    PUSH4 fx_src,fx_dst,fx_dstlen,ra  // P_40  params to unfilter and sync_cache

// Input to de-compressor:
#define xsrc    a0
#define xsrclen a1
#define xdst    a2
#define xdstlen a3
#define meth    a4
    lbu meth,b_method(fx_src)  // arg5
    move xdstlen,fx_dstlen  // arg4
    move xdst,fx_dst  // arg3
    move a5,fx_src; bal get4unal
      la a0,sz_cpr(xsrc)
    move xsrclen,v0  // arg2
    call decompress
      addi xsrc,a5,sz_binfo // arg1

    POP3 a2,a0,a1   // P_40  fx_src,fx_dst,fx_dstlen; keep f_expand.retaddr
    lw a1,0(a1)  // actual length used by decompressor
    PUSH3 a0,a1,v0  // P_41  fx_dst,fx_dstlen,retval.f_exp
    lbu a3,b_ftid(a2)
      nop
    beqz a3,no_unf
      lbu a2,b_cto8(a2)
    call unfilter
no_unf:

    POP2 a0,a1   // dst, len
    add a1,a1,a0  // lo, hi
        // synci subsumed by [write() +] mmap() of PROT_EXEC from memfd_create
    POP2 v0,ra  // P41,P40
    ret; nop

//    .unreq fx_src
//    .unreq fx_dst
//    .unreq fx_dstlen
//    .unreq xsrc
//    .unreq xsrclen
//    .unreq xdst
//    .unreq xdstlen
//    .unreq meth

decompress:  // (src *, cpr_len, dst *, &dstlen);
    PUSH1 ra  # P_45
    call go_decompr
      nop
        // lzma subroutine returns here with (0==v0) ==> success
    POP1 ra  # P_45  (1 of 2)
        // nrv2*_d comes here after computing success/fail
ret_decompr: .globl ret_decompr
    ret; nop

#define src  a0
#define lsrc a1
#define dst  a2
#define ldst a3  /* Out: actually a reference: &len_dst */
#define meth a4

eof_nrv: .globl eof_nrv
eof_n2e: .globl eof_n2e
eof_n2d: .globl eof_n2d
eof_n2b: .globl eof_n2b
        POP1 a6  # P_45 (2 of 2) ra for decompress
        lw at,0*NBPW(sp)  # orig &b_info
        move a5,src; bal get4unal
          la a0,sz_cpr(at)
        move lsrc,v0  # expected src len
        move ra,a6

#define tmp at
        lw tmp,1*NBPW(sp)  # orig dst
        lw ldst,2*NBPW(sp)
        subu tmp,dst,tmp
        sw tmp,0(ldst)  # actual len generated

        lw tmp,0*NBPW(sp)  # &b_info
          nop
        subu v0,a5,tmp
        subiu v0,sz_binfo  # actual len consumed
        b ret_decompr
          subu v0,v0,lsrc  # deviation at EOF
#undef tmp

go_decompr:
//  sections NRV2B, etc, inserted here by addLoader() from ::buildLinuxLoader()

  section EXP_TAIL
#define M_NRV2B_LE32    2
#define M_NRV2B_8    3
#define M_NRV2D_LE32    5
#define M_NRV2D_8    6
#define M_NRV2E_LE32    8
#define M_NRV2E_8    9
#define M_CL1B_LE32     11
#define M_LZMA          14

#define  hibit r0  /* holds 0x80000000 during decompress */


unfilter:
//#include "arch/mips/r3000/bxx.S"  // unfilter code; args in registers, fall-through return

        .balign 4
upx_mmap_and_fd: .globl upx_mmap_and_fd
    // UMF_ANDROID or UMF_LINUX must be loaded after EXP_TAIL

// FIXME: will need extra parameter for multi-method decompression
#define NO_METHOD_CHECK 0

#define off  a4
#define len  a5
#define bits a6
#define disp a7

    UCL_init 32,1,0  // 32 bits (not 8), UCL_SMALL, !UCL_FAST
    decomp_done = eof_n2e
  section NRV2E
    li at,M_NRV2E_LE32; bne at,meth,not_nrv2e
#include "arch/mips/r3000/nrv2e_d.ash"
    build nrv2e, full
not_nrv2e:

  section NRV2D
    decomp_done = eof_n2d
    li at,M_NRV2D_LE32; bne at,meth,not_nrv2d
#include "arch/mips/r3000/nrv2d_d.ash"
    build nrv2d, full
not_nrv2d:

  section NRV2B
    decomp_done = eof_n2b
    li at,M_NRV2B_LE32; bne at,meth,not_nrv2b
#include "arch/mips/r3000/nrv2b_d.ash"
    build nrv2b, full
not_nrv2b:

#undef off
#undef len
#undef bits
#undef disp

#undef src
#undef lsrc
#undef dst
#undef ldst

  section LZMA_DAISY
    li at,M_LZMA; bne at,meth,not_lzma
#undef meth
#include "arch/mips/r3000/lzma_d.S"
not_lzma:
