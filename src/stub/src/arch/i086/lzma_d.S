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
;   ds:si - source
;   es:di - dest
;   cld

; Output:
*/


/*************************************************************************
// support macros
**************************************************************************/

// huge pointer diff: dx:ax = dx:ax - cx:bx
// !!! this version does nothing !!!
.macro M_aNahdiff
        add     sp, 8
.endm


// umul32: dx:ax = dx:ax * 00:bx
.macro  M_LMUL_dxax_00bx
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
.macro  M_LMUL_dxax_00bx_ptr
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
.macro  M_LMUL_axcx_00bx
        // mult high-word
        mul     bx
        xchg    ax, cx      // save high-word result, get low
        // mult low-word
        mul     bx
        // add high-word
        add     dx, cx      // add high-word result
.endm


// shld: dx:ax <<= 8
.macro  M_shld_8
        mov     dh, dl
        mov     dl, ah
        mov     ah, al
        xor     al, al
.endm

.macro  M_shld_8_bp h l
        mov     dx, word ptr[bp+h]
        mov     ax, word ptr[bp+l]
        M_shld_8
        mov     word ptr[bp+h], dx
        mov     word ptr[bp+l], ax
.endm


// shld: dx:ax >>= 11
.macro  M_shrd_11
#if 0
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
#else
    // WARNING: this trashes "bx" !
        mov     cl, 11
        mov     bx, dx      // save dx
        shr     ax, cl
        shr     dx, cl
        mov     cl, 5       // cl = 16 - cl
        shl     bx, cl
        or      ax, bx
#endif
.endm

.macro  M_shrd_11_bp h l
        mov     dx, word ptr[bp+h]
        mov     ax, word ptr[bp+l]
        M_shrd_11
        mov     word ptr[bp+h], dx
        mov     word ptr[bp+l], ax
.endm

.macro  M_shrd_11_disi_bp h l
        mov     dx, di
        mov     ax, si
        M_shrd_11
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
        shr     ax, cl
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


/*************************************************************************
// support code (see cleanasm.py)
**************************************************************************/

section LZMA_DEC99

.macro  M_PIA
        M_WCC_PIA
.endm
.macro  M_PIA1
#if 1
        local   L1
        inc     ax
        jnes    L1
        //add     dx, __AHINCR
        add     dh, __AHINCR >> 8
L1:
#else
        add     ax, 1
        sbb     bl, bl
        and     bl, __AHINCR >> 8
        add     dh, bl
#endif
.endm
.macro  M_PTC
        M_WCC_PTC
.endm
.macro  M_PTC_JNE l
        cmp     ax, bx
        jnes    l
        cmp     dx, cx
        jnes    l
.endm
.macro  M_PTS
.endm


/*************************************************************************
//
**************************************************************************/

// init
section LZMA_DEC00
        //.byte   0xcc

        push    ds
        mov     bp, sp
#if 0
        // DEBUG - check for enough stack
        lea     bx, [bp + lzma_stack_adjust - 256]
        cmp     bp, bx
1:      jcs     1b
#endif
        lea     bx, [bp + lzma_stack_adjust]

#if 0
        // clear stack
        xor     ax, ax
1:      push    ax
        cmp     sp, bx
        jnz     1b
#else
        mov     sp, bx
#endif

        inc     si
        inc     si

        push    bx                      // &outSizeProcessed __near     [bp + 24]
        mov     ax, offset lzma_u_len_hi // outSize
        push    ax                      //                              [bp + 22]
        mov     ax, offset lzma_u_len
        push    ax                      //                              [bp + 20]
        push    es                      // out                          [bp + 18]
        push    di                      //                              [bp + 16]

        add     bx, 4
        push    bx                      // &inSizeProcessed __near      [bp + 14]
        mov     ax, offset lzma_c_len_hi // inSize
        push    ax                      //                              [bp + 12]
        mov     ax, offset lzma_c_len
        push    ax                      //                              [bp + 10]
        push    ds                      // in                           [bp + 8]
        push    si                      //                              [bp + 6]

        // enter small model
        push    ss
        pop     ds

        add     bx, 4
        push    bx                      // &state __near                [bp + 4]
        mov     ax, offset lzma_properties_hi
        mov     [bx + 2], ax
        mov     ax, offset lzma_properties
        mov     [bx], ax

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
#if 0
        // clear dirty stack
        lea     bx, [bp + lzma_stack_adjust - 256]
        mov     sp, bp
        xor     ax, ax
1:      push    ax
        cmp     sp, bx
        jnz     1b
#endif
        mov     sp, bp
        pop     ds
        mov     di, offset lzma_u_len

section LZMA_DEC31
        pop     ax
        push    ax
        add     ax, offset lzma_u_len_segment
        mov     es, ax


// vi:ts=4:et
