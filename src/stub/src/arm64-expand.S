/*
;  arm.v4a-expand.S -- decompressors for arm.v4a
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

NBPW= 8
#include "arch/arm64/v8/macros.S"

.macro call subr; bl \subr; .endm

  section EXP_HEAD

sz_unc= 0
sz_cpr= 4
b_method= 8
b_ftid=   9
b_cto8=  10
b_extra= 11
sz_binfo= 12

f_expand: .globl f_expand // f_expand(b_info *, dst, &dstlen)
// Supervisor for de-compression, un-filter, and sync_cache
// Input to supervisor:
fx_src    .req x0
fx_dst    .req x1
fx_dstlen .req x2
    PUSH4(fx_src,fx_dst,fx_dstlen,lr)  // params to unfilter and sync_cache

// Input to de-compressor:
xsrc    .req x0
xsrclen .req w1
xdst    .req x2
xdstlen .req x3
methb   .req w4
    ldrb methb,[fx_src,#b_method]
    mov xdstlen,fx_dstlen  // arg4
    mov xdst,fx_dst  // arg3
    ldr xsrclen,[xsrc,#sz_cpr]  // arg2
    add xsrc,fx_src,#sz_binfo // arg1
    call decompress
    mov x3,x0  // save retval

    POP4(x2,x0,x1,lr)  // fx_src,fx_dst,fx_dstlen,lr
    ldr x1,[x1]  // actual length used by decompressor
    PUSH3(x0,x1,x3)  // params for sync_cache
    ldrb w3,[x2,#b_ftid]
    ldrb w2,[x2,#b_cto8]
    cbz w3,no_unf
#include "arch/arm64/v8/bxx.S"  // unfilter code; args in registers, fall-through return
no_unf:

    POP2(x0,x1)  // dst, len
    add x1,x1,x0  // lo, hi
    sync_cache  // in macros.S
    POP1(x0)  // retval from decompress
    ret
    .unreq fx_src
    .unreq fx_dst
    .unreq fx_dstlen
    .unreq xsrc
    .unreq xsrclen
    .unreq xdst
    .unreq xdstlen
    .unreq methb

decompress:  // (src *, cpr_len, dst *, &dstlen);
//  sections NRV2B, etc, inserted here by addLoader() from ::buildLinuxLoader()

  section EXP_TAIL
        src .req x0
        dst .req x2
  .type eof,function
  .globl eof
eof:  // end of a compressed extent; need sync_cache
        ldr x3,[sp],#NBPW  // &input_eof
        sub x3,x0,x3  // src -= eof;  // return 0: good; else: bad
        ldr x0,[sp],#NBPW  // original dst
        ldr x1,[sp],#NBPW; sub x1,x2,x1  // dst -= original dst
        ldr x3,[sp],#NBPW; str dst,[x3]  // actual length used at dst  XXX: 4GB
        ret
    .unreq src
    .unreq dst

#define M_NRV2B_LE32    2
#define M_NRV2B_8    3
#define M_NRV2D_LE32    5
#define M_NRV2D_8    6
#define M_NRV2E_LE32    8
#define M_NRV2E_8    9
#define M_CL1B_LE32     11
#define M_LZMA          14

// FIXME: will need extra parameter for multi-method decompression
#define NO_METHOD_CHECK 1

  section NRV2E
#include "arch/arm64/v8/nrv2e_d32.S"

  section NRV2D
#include "arch/arm64/v8/nrv2d_d32.S"

  section NRV2B
#include "arch/arm64/v8/nrv2b_d32.S"

/* lzma has its own 'section's */
#include "arch/arm64/v8/lzma_d.S"

