/*
;  lzma_d.ash -- 16-bit assembly
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 2006-2007 Markus Franz Xaver Johannes Oberhumer
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


; ------------- DECOMPRESSION -------------

; Input:
;   si - source
;   di - dest
;   cld

; Output:
*/


/*************************************************************************
// override generic macros with special versions
**************************************************************************/

// huge pointer diff: dx:ax = dx:ax - cx:bx
// !!! this version does nothing !!!
.macro M_WCC_PTS_lzma
.endm

#define M_WCC_PTS       M_WCC_PTS_lzma


// huge pointer compare: set zero and carry flags: dx:ax cmp cx:bx
// !!! this version does not normalize pointers !!!
.macro M_WCC_PTC_lzma
       local   L1
       cmp     dx, cx
       jnes    L1
       cmp     ax, bx
L1:
.endm

#define M_WCC_PTC       M_WCC_PTC_lzma


// umul32: dx:ax = dx:ax * 00:bx
.macro  M_WCC_U4M_dxax_00bx
        // mult high-word
        xchg    cx, ax      // cx: save ax
        xchg    ax, dx
        mul     bx
        xchg    ax, cx      // save high-word result, get orig ax
        // mult low-word
        mul     bx          // dx:ax := ax * bx
        // add high-word
        add     dx, cx      // add high-word result
.endm

// umul32: dx:ax = ax:cx * 00:bx
.macro  M_WCC_U4M_axcx_00bx
        // mult high-word
        mul     bx
        xchg    ax, cx      // save high-word result, get low
        // mult low-word
        mul     bx
        // add high-word
        add     dx, cx      // add high-word result
.endm


/*************************************************************************
// support code (see cleanasm.py)
**************************************************************************/

section LZMA_DEC99

__PIA_V04:
        mov     bx,WORD PTR [bp-94]
        or      bx,ax
        mov     WORD PTR [bp-8],bx
        mov     WORD PTR [bp-4],dx
__PIA_V03:
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
#if 0
        mov     bx,0x1
        xor     cx,cx
        M_WCC_PIA
        mov     WORD PTR [bp-12],ax
        movw    ds,dx
#else
        // optimized version
        inc     ax
        jnes    .L1
        add     dh, (__AHINCR >> 8)
        movw    ds, dx
.L1:
        mov     WORD PTR [bp-12],ax
#endif
        ret


.macro M_WCC_PIA_V02_lzma
        movw    dx, ds
#if 0
        mov     bx,0x1
        xor     cx,cx
        M_WCC_PIA
#else
        // optimized version
        local   L1
        inc     ax
        jnes    L1
        add     dh, (__AHINCR >> 8)
        movw    ds, dx
L1:
#endif
.endm
#define WCC_PIA_V02     M_WCC_PIA_V02_lzma


__PIA:
        M_WCC_PIA
        ret


__PTC_V01:
        mov     ax,WORD PTR [bp-12]
        movw    dx,ds
        mov     bx,WORD PTR [bp-26]
        mov     cx,WORD PTR [bp-24]
__PTC:
        M_WCC_PTC
        ret


#define WCC_PTS     M_WCC_PTS


__U4M_V02:
#if 0
        mov     bx,WORD PTR es:[bx]
        mov     ax,WORD PTR [bp-102]
        mov     dx,WORD PTR [bp-100]
        xor     cx,cx
        M_WCC_U4M
#else
        // optimized version
        mov     bx,WORD PTR es:[bx]
        mov     cx,WORD PTR [bp-102]
        mov     ax,WORD PTR [bp-100]
        M_WCC_U4M_axcx_00bx
#endif
        mov     WORD PTR [bp-10],ax
        mov     WORD PTR [bp-6],dx
        ret


__U4M_V01:
#if 0
        xor     cx,cx
        M_WCC_U4M
#else
        M_WCC_U4M_dxax_00bx
#endif
        ret


/*************************************************************************
//
**************************************************************************/

// init
section LZMA_DEC00
//        .byte   0xcc
        mov     bp, sp
        lea     bx, [bp + lzma_stack_adjust]
#if 0
        xor     ax, ax
.clearstack:
        push    ax
        cmp     sp, bx
        jnz     .clearstack
#else
        mov     sp, bx
#endif

        inc     si
        inc     si

        push    ss                      // &outSizeProcessed
        push    bx
        mov     ax, offset lzma_u_len_hi // outSize
        push    ax
        mov     ax, offset lzma_u_len
        push    ax
        push    es                      // out
        push    di

        add     bx, 4
        push    ss                      // &inSizeProcessed
        push    bx
        mov     ax, offset lzma_c_len_hi // inSize
        push    ax
        mov     ax, offset lzma_c_len
        push    ax
        push    ds                      // in
        push    si

        add     bx, 4
        push    ss
        push    bx
        mov     ax, offset lzma_properties_hi
        mov     ss:[bx + 2], ax
        mov     ax, offset lzma_properties
        mov     ss:[bx], ax

        call    LZMA_DEC10
        jmp     LZMA_DEC30


section LZMA_DEC10
.arch   i8086, nojumps
#include "lzma_d_cs.S"
section LZMA_DEC20
.arch   i8086, nojumps
#include "lzma_d_cf.S"
.arch   i8086, jumps


// cleanup
section LZMA_DEC30
        mov     sp, bp
        mov     di, offset lzma_u_len

section LZMA_DEC31
        pop     ax
        push    ax
        add     ax, offset lzma_u_len_segment
        mov     es, ax


#undef M_WCC_PTS
#undef M_WCC_PTC
#undef M_WCC_U4M

#undef WCC_PIA_V02
#undef WCC_PTS

// vi:ts=8:et

