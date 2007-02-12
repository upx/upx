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
// FIXME: need optimized version here
        movw    dx,ds
        mov     bx,0x1
        xor     cx,cx
        M_WCC_PIA
        mov     WORD PTR [bp-12],ax
        movw    ds,dx
        ret


#if 1
.macro lzma_WCC_PIA_V02
// FIXME: need optimized version here
        movw    dx,ds
        mov     bx,0x1
        xor     cx,cx
        M_WCC_PIA
.endm
#define WCC_PIA_V02 lzma_WCC_PIA_V02
#endif


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


#if 1
// FIXME: do we actually need the PTS result ??
#define WCC_PTS     M_WCC_PTS
#else
#define WCC_PTS     xor ax,ax; xor dx,dx
#endif


__U4M_V02:
        mov     bx,WORD PTR es:[bx]
        mov     ax,WORD PTR [bp-102]
        mov     dx,WORD PTR [bp-100]
// FIXME: need optimized version here (cx = 0)
        xor     cx,cx
        M_WCC_U4M
        mov     WORD PTR [bp-10],ax
        mov     WORD PTR [bp-6],dx
        ret


__U4M_V01:
// FIXME: need optimized version here (cx = 0)
        xor     cx,cx
        M_WCC_U4M
        ret


/*************************************************************************
//
**************************************************************************/


// init
section LZMA_DEC00
//        .byte   0xcc
        mov     bp, sp
        lea     bx, [bp + lzma_stack_adjust]
        xor     ax, ax
.clearstack:
        push    ax
        cmp     sp, bx
        jnz     .clearstack

        inc     si
        inc     si

        push    ss                      // outSizeProcessed
        push    bx
        mov     ax, offset lzma_u_len_hi
        push    ax
        mov     ax, offset lzma_u_len
        push    ax                      // outSize
        push    es
        push    di                      // out

        add     bx, 4
        push    ss                      // inSizeProcessed
        push    bx
        mov     ax, offset lzma_c_len_hi
        push    ax
        mov     ax, offset lzma_c_len
        push    ax                      // inSize
        push    ds
        push    si                      // in

        add     bx, 4
        push    ss
        push    bx
        mov     ax, offset lzma_properties_hi
        mov     ss:[bx + 2], ax
        mov     ax, offset lzma_properties
        mov     ss:[bx], ax
        call    LZMA_DEC10
//        .byte   0xcc

        mov     sp, bp
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
        mov     di, offset lzma_u_len

section LZMA_DEC31
        pop     ax
        push    ax
        add     ax, offset lzma_u_len_segment
        mov     es, ax


#undef WCC_PIA_V02
#undef WCC_PTS

// vi:ts=8:et

