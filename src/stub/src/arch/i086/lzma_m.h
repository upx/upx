/*
;  lzma_m.h -- 16-bit assembly
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 2006-2008 Markus Franz Xaver Johannes Oberhumer
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
;  Markus F.X.J. Oberhumer
;  <markus@oberhumer.com>
;  http://www.oberhumer.com/opensource/upx/
;
*/


#ifndef __AHSHIFT

/* DOS real mode */
#define __AHSHIFT        12
#define __AHINCR         (1 << __AHSHIFT)  /* 4096 */


/*************************************************************************
// support macros: PIA, PIS, PTS, PTC
**************************************************************************/

// huge pointer add: dx:ax = dx:ax + cx:bx
.macro  M_PIA_small
        add     ax, bx
        adc     cx, 0
#if 0
        // code size: 8 bytes
        // i086: 2+4+56+3 == 65 clocks
        // i286: 2+2+17+2 == 23 clocks
        mov     bx, cx
        mov     cl, __AHSHIFT
        shl     bx, cl
        add     dx, bx
#else
        // code size: 8 bytes
        // i086: 2+4+20+3 == 29 clocks
        // i286: 2+2+ 9+2 == 15 clocks
        mov     bl, cl
        mov     cl, __AHSHIFT - 8
        shl     bl, cl
        add     dh, bl
#endif
.endm

.macro  M_PIA_fast
        add     ax, bx
        adc     cx, 0
        // code size: 10 bytes
        // i086: 4*2+3 == 11 clocks
        // i286: 4*2+2 == 10 clocks
        shl     cl
        shl     cl
        shl     cl
        shl     cl
        add     dh, cl
.endm


.macro  M_PIA1_small
        // code size: 6 bytes
        // i086: 3+16   == 19 clocks (jump taken)
        // i286: 2+ 7+m ~~ 11 clocks (jump taken)
        local   L1
        inc     ax
        jnes    L1
        //add     dx, __AHINCR
        add     dh, __AHINCR >> 8
L1:
.endm

.macro  M_PIA1_fast
    // WARNING: this trashes "bx" !
        // code size: 10 bytes
        // i086: 3+3+3+3 == 12 clocks
        // i286: 2+2+2+2 ==  8 clocks
        add     ax, 1
        sbb     bl, bl
        and     bl, __AHINCR >> 8
        add     dh, bl
.endm


#if 0
// huge pointer sub: dx:ax = dx:ax - cx:bx
.macro  M_PIS
        sub     ax, bx
        adc     cx, 0
        mov     bx, cx
        mov     cl, __AHSHIFT
        shl     bx, cl
        sub     dx, bx
.endm
#endif


// huge pointer diff: dx:ax = dx:ax - cx:bx
.macro  M_PTS
#if 0
// normalize
// FIXME
// subtract
        sub     ax, bx
        sbb     dx, cx
#endif
.endm


// huge pointer compare: set zero and carry flags: dx:ax cmp cx:bx
.macro  M_PTC
// NOTE: no pointer normalization!
        local   L1
        cmp     dx, cx
        jnes    L1
        cmp     ax, bx
L1:
.endm

.macro  M_PTC_JNE l
        cmp     ax, bx
        jnes    l
        cmp     dx, cx
        jnes    l
.endm


/*************************************************************************
// support macros: U4M, shld, shrd
**************************************************************************/

// umul32: dx:ax = dx:ax * 00:bx
.macro  M_U4M_dxax_00bx
        // mult high-word
        mov     cx, ax      // save ax
        mov     ax, dx
        mul     bx
        xchg    ax, cx      // save high-word result, get saved ax
        // mult low-word
        mul     bx          // dx:ax := ax * bx
        // add high-word
        add     dx, cx      // add high-word result
.endm


// umul32: dx:ax = dx:ax * word ptr [bx]
.macro  M_U4M_dxax_00bx_ptr
        // mult high-word
        mov     cx, ax      // save ax
        mov     ax, dx
        mul     word ptr [bx]
        xchg    ax, cx      // save high-word result, get saved ax
        // mult low-word
        mul     word ptr [bx]
        // add high-word
        add     dx, cx      // add high-word result
.endm


// umul32: dx:ax = ax:cx * 00:bx
.macro  M_U4M_axcx_00bx
        // mult high-word
        mul     bx
        xchg    ax, cx      // save high-word result, get low
        // mult low-word
        mul     bx
        // add high-word
        add     dx, cx      // add high-word result
.endm


// umul32: dx:ax = dx:ax * 0x0600
.macro  M_U4M_dxax_0x0600
#if 0
        // code size: 18 bytes
        // i086: > 140 clocks (mul needs 70 clocks)
        // i286: >  26 clocks (mul needs 13 clocks)
        mov     bx, 0x300
        M_U4M_dxax_00bx
        shl     ax
        rcl     dx
#elif 0
        // code size: 14 bytes
        // i086: > 140 clocks (mul needs 70 clocks)
        // i286: >  26 clocks (mul needs 13 clocks)
        mov     bx, 0x600
        M_U4M_dxax_00bx
#else
        // code size: 16+8 == 24 bytes
        // i086: 18+9 == 27 clocks
        // i286: 16+8 == 24 clocks
    // FIXME: can we further optimize this ?
        shl     ax
        rcl     dx          // dx:ax <<= 1      v * 2
        mov     cx, dx
        mov     bx, ax      // cx:bx = dx:ax    v * 2
        shl     ax
        rcl     dx          // dx:ax <<= 1      v * 4
        add     ax, bx
        adc     dx, cx      // dx:ax += cx:bx   v * 6
        M_shld_8            // dx:ax <<= 8      v * 0x600
#endif
.endm


// shld: dx:ax <<= 8
.macro  M_shld_8
        // code size: 8 bytes
        // i086: 9 clocks
        // i286: 8 clocks
        mov     dh, dl
        mov     dl, ah
        mov     ah, al
        xor     al, al
.endm


// shld: di:si <<= 8; bx and cx are free
.macro  M_shld_disi_8_bxcx
#if 0
        // code size: 9 bytes
        // i086: 2 + 7*(2+2+18) + (2+2+5) == 165 clocks
        // i286: 2 + 7*(2+2+10) + (2+2+4) == 108 clocks
        local   L1
        mov     cx, 8
L1:     shl     si
        rcl     di
        loop    L1
#else
        // code size: 16 bytes
        // i086: 17 clocks
        // i286: 16 clocks
        mov     bx, di
        mov     cx, si
        mov     bh, bl
        mov     bl, ch
        mov     ch, cl
        xor     cl, cl
        mov     di, bx
        mov     si, cx
#endif
.endm


// shld: di:ax <<= 8; bx and cx are free
.macro  M_shld_diax_8_bxcx
#if 0
        // code size: 9 bytes
        // i086: 2 + 7*(2+2+18) + (2+2+5) == 165 clocks
        // i286: 2 + 7*(2+2+10) + (2+2+4) == 108 clocks
        local   L1
        mov     cx, 8
L1:     shl     ax
        rcl     di
        loop    L1
#else
        // code size: 12 bytes
        // i086: 13 clocks
        // i286: 12 clocks
        mov     bx, di
        mov     bh, bl
        mov     bl, ah
        mov     ah, al
        xor     al, al
        mov     di, bx
#endif
.endm


.macro  M_shld_8_bp h l
        mov     dx, word ptr[bp+h]
        mov     ax, word ptr[bp+l]
        M_shld_8
        mov     word ptr[bp+h], dx
        mov     word ptr[bp+l], ax
.endm


// shld: dx:ax <<= cl; trashes cl and register "r1" (bx, di, si or bp)
//   REQUIRED: 0 <= cl <= 15
.macro  M_shld_00_15 r1
        mov     r1, ax      // save ax
        shl     dx, cl
        shl     ax, cl
        sub     cl, 16
        neg     cl          // cl = 16 - cl
        shr     r1, cl
        or      dx, r1
.endm

// shld: dx:ax <<= cl; trashes cl
//   REQUIRED: 16 <= cl <= 32
.macro  M_shld_16_32
        sub     cl, 16
        shl     ax, cl
        mov     dx, ax
        xor     ax, ax
.endm

// shld: dx:ax <<= cl; trashes cl and register "r1" (bx, di, si or bp)
//   REQUIRED: 0 <= cl <= 32
.macro  M_shld r1
        local   L1, L2
        cmp     cl, 16
        jaes    L1
// 0 <= cl <= 15
        M_shld_00_15 r1
        jmps    L2
L1:
// 16 <= cl <= 32
        M_shld_16_32
L2:
.endm


// shld: dx:ax >>= 11
.macro  M_shrd_11_small
    // WARNING: this trashes "bx" !
        // code size: 14 bytes
        // i086: 4+2+52+52+4+28+3 == 145 clocks
        // i286: 2+2+16+16+2+10+2 ==  50 clocks
        mov     cl, 11
        mov     bx, dx      // save dx
        shr     ax, cl
        shr     dx, cl
        mov     cl, 5       // cl = 16 - cl
        shl     bx, cl
        or      ax, bx
.endm

.macro  M_shrd_11_fast
        // code size: 20 bytes
        // i086: 21 clocks
        // i286: 20 clocks
        mov     al, ah
        mov     ah, dl
        mov     dl, dh
        xor     dh, dh
        shr     dx
        rcr     ax
        shr     dx
        rcr     ax
        shr     dx
        rcr     ax
.endm


.macro  M_shrd_11_bp_small h l
        mov     dx, word ptr[bp+h]
        mov     ax, word ptr[bp+l]
        M_shrd_11_small
        mov     word ptr[bp+h], dx
        mov     word ptr[bp+l], ax
.endm

.macro  M_shrd_11_bp_fast h l
        mov     dx, word ptr[bp+h]
        mov     ax, word ptr[bp+l]
        M_shrd_11_fast
        mov     word ptr[bp+h], dx
        mov     word ptr[bp+l], ax
.endm


.macro  M_shrd_11_disi_bp_small h l
        mov     dx, di
        mov     ax, si
        M_shrd_11_small
        mov     word ptr[bp+h], dx
        mov     word ptr[bp+l], ax
.endm

.macro  M_shrd_11_disi_bp_fast h l
        mov     dx, di
        mov     ax, si
        M_shrd_11_fast
        mov     word ptr[bp+h], dx
        mov     word ptr[bp+l], ax
.endm


/*************************************************************************
//
**************************************************************************/

#endif /* ifndef __AHSHIFT */


#undef M_PIA
#undef M_PIA1
#undef M_shrd_11
#undef M_shrd_11_bp
#undef M_shrd_11_disi_bp

#if defined(FAST)
#  define M_PIA                 M_PIA_fast
#  define M_PIA1                M_PIA1_fast
#  define M_shrd_11             M_shrd_11_fast
#  define M_shrd_11_bp          M_shrd_11_bp_fast
#  define M_shrd_11_disi_bp     M_shrd_11_disi_bp_fast
#elif defined(SMALL)
#  define M_PIA                 M_PIA_small
#  define M_PIA1                M_PIA1_small
#  define M_shrd_11             M_shrd_11_small
#  define M_shrd_11_bp          M_shrd_11_bp_small
#  define M_shrd_11_disi_bp     M_shrd_11_disi_bp_small
#else
#  error
#endif


// vi:ts=4:et
