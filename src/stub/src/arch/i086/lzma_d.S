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
        mov     cx, ax      // cx: save ax
        mov     ax, dx
        mul     bx
        xchg    ax, cx      // save high-word result, get orig ax
        // mult low-word
        mul     bx          // dx:ax := ax * bx
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
        //xor     cx, cx      // FIXME - do we need this ?
.endm


// shld: dx:ax >>= 11
.macro  M_shrd_11
#if 1
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
        mov     bx, dx
        mov     cl, 11
        shr     dx, cl
        shr     ax, cl
        mov     cl, 5
        shl     bx, cl
        or      ax, bx
#endif
        //xor     cx, cx      // FIXME - do we need this ?
.endm


#if 0
// shld: dx:ax <<= cl; trashes register "r1" (bx, di, si or bp)
//   REQUIRED: 0 <= cl < 32
// FIXME - this does not work yet
.macro  M_shld r1
        local   L1
        mov     r1, ax      // save ax
        shl     dx, cl
        shl     ax, cl
        neg     cl
        and     cl, 15      // the 8086 uses all eight bits of the shift count
        shr     r1, cl
        or      dx, r1
.endm
#endif


/*************************************************************************
// support code (see cleanasm.py)
**************************************************************************/

section LZMA_DEC99


/*************************************************************************
//
**************************************************************************/

// init
section LZMA_DEC00
        //.byte   0xcc

        push    ds
        mov     bp, sp
#if 1
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
