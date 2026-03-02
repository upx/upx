/*
;  arm.v4a-expand.S -- decompressors for arm.v4a
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) Laszlo Molnar
;  Copyright (C) John F. Reiser
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
    PUSH4(fx_src,fx_dst,fx_dstlen,lr)  // MATCH_95 params to unfilter and sync_cache

// Input to de-compressor:
xsrc    .req x0
xsrclen .req w1
xdst    .req x2
xdstlen .req x3
meth    .req w4
    ldrb meth,[fx_src,#b_method]
    mov xdstlen,fx_dstlen  // arg4
    mov xdst,fx_dst  // arg3
    ldr xsrclen,[xsrc,#sz_cpr]  // arg2
    add xsrc,fx_src,#sz_binfo // arg1
    call decompress
    mov x3,x0  // save retval

    POP4(x2,x0,x1,lr)  // MATCH_95  fx_src,fx_dst,fx_dstlen,lr
    ldr x1,[x1]  // actual length used by decompressor
    PUSH3(x0,x1,x3)  // MATCH_96  params for sync_cache
    ldrb w3,[x2,#b_ftid]
    ldrb w2,[x2,#b_cto8]
    cbz w3,no_unf
#include "arch/arm64/v8/bxx.S"  // unfilter code; args in registers, fall-through return
no_unf:

    POP2(x0,x1)  // MATCH_96  dst, len
    add x1,x1,x0  // lo, hi
    sync_cache  // in macros.S
    POP1(x0)  // MATCH_96  retval from decompress
    ret
    .unreq fx_src
    .unreq fx_dst
    .unreq fx_dstlen
    .unreq xsrc
    .unreq xsrclen
    .unreq xdst
    .unreq xdstlen

decompress:  // (src *, cpr_len, dst *, &dstlen);
//  sections NRV2B, etc, inserted here by addLoader() from ::buildLinuxLoader()

  section EXP_TAIL
// Fall through: daisy chain had no matching method
        mov x0,#-1
        mov w1,meth
        bkpt  // EXP_TAIL daisy chain fail

        src .req x0
        dst .req x2
        dstw .req w2

  .globl eof

// sync_cache is done in tail of f_expand, after possible unfilter
// NYI: eof_n2b, eof_n2d, eof_n2e should be unified.
eof_n2b: // .globl eof_n2b .type eof_n2b,%function
eof:  // MATCH_90  end of a compressed extent; need sync_cache after unfilter
        ldr x3,[sp,#0*NBPW]  // &input_eof
        sub x0,src,x3  // src -= eof;  // return 0: good; else: bad
        ldr x1,[sp,#1*NBPW]  // original dst
        sub dst,dst,x1  // dst -= original dst; actual length of output
        ldr x1,[sp,#2*NBPW]  // &dstlen
        str dstw,[x1]  // actual length used at dst  XXX: 4GB
        ldr lr,[sp,#3*NBPW]
        add sp,sp,#4*NBPW
        ret


// WINDOWS_BACK compatibility seems to be broken
// if POP3 replaces POP2+POP1 (MATCH_92, MATCH_91)
srclim .req x7
eof_n2d: // .globl eof_n2d
eof_n2e: // .globl eof_n2e
        POP2(x3,x4)  // MATCH_92  r3= orig_dst; r4= plen_dst
        SUB2(src,srclim)  // 0 if actual src length equals expected length
        SUB2(dst,x3)  // actual dst length
        str dstw,[x4]
        POP1(lr)  // MATCH_91
        ret

    .unreq src
    .unreq dst
    .unreq dstw
    .unreq srclim

eof_lzma: .globl eof_lzma
        POP4(x2,x3, fp,lr)  // MATCH_94  x2= orig_dst; x3= plen_dst
        ret

upx_mmap_and_fd: .globl upx_mmap_and_fd
    // UMF_LINUX goes here

#define M_NRV2B_LE32    2
#define M_NRV2B_8    3
#define M_NRV2D_LE32    5
#define M_NRV2D_8    6
#define M_NRV2E_LE32    8
#define M_NRV2E_8    9
#define M_CL1B_LE32     11
#define M_LZMA          14

#define NO_METHOD_CHECK 1  /* subsumed here by daisy chain */

  section NRV2E
    cmp meth,#M_NRV2E_LE32; bne not_nrv2e
#include "arch/arm64/v8/nrv2e_d32.S"
not_nrv2e:

  section NRV2D
    cmp meth,#M_NRV2D_LE32; bne not_nrv2d
#include "arch/arm64/v8/nrv2d_d32.S"
not_nrv2d:

  section NRV2B
    cmp meth,#M_NRV2B_LE32; bne not_nrv2b
#include "arch/arm64/v8/nrv2b_d32.S"
not_nrv2b:

  section LZMA_DAISY
    cmp meth,#M_LZMA; bne not_lzma
#include "arch/arm64/v8/lzma_d.S"
not_lzma:

section ZSTD_DAISY
    .balign 4
#define M_ZSTD 16
    cmp meth,#M_ZSTD; bne not_zstd
    stp x3,lr,[sp,#-16]!
    uxtw x5,w1
    ldr w4,[x3]
    mov x3,x5
    mov x5,x0
    mov x0,x2
    uxtw x1,w4
    mov x2,x5
    bl upx_zstd_decompress
    ldp x3,lr,[sp],#16
    str w0,[x3]
    mov x0,#0
    ret
not_zstd:
