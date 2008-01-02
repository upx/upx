/*
;  lzma_d.ash -- 16-bit assembly
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


; ------------- DECOMPRESSION -------------

; Input:
;   ds:si - source
;   es:di - dest
;   cld

; Output:
*/


/*************************************************************************
//
**************************************************************************/

// init
section LZMA_DEC00
        //.byte   0xcc

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
        jnzs    1b
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

        add     bx, 4
        push    bx                      // &state __near                [bp + 4]

        // enter small memory model
        push    ds                      // save ds and dummy for call
        push    ss
        pop     ds

        // fill properties: lc lp pb
        movw    [bx],   offset lzma_properties          // lc lp
        movb    [bx+2], offset lzma_properties_hi       // pb

    ignore_reloc_overflow lzma_u_len
    ignore_reloc_overflow lzma_c_len
    ignore_reloc_overflow lzma_properties


#define ret
section LZMA_DEC10
.arch   i8086, nojumps
#define SMALL 1
#include "lzma_m.h"
#include "lzma_d_cs.S"
#undef SMALL
section LZMA_DEC20
.arch   i8086, nojumps
#define FAST 1
#include "lzma_m.h"
#include "lzma_d_cf.S"
#undef FAST
.arch   i8086, jumps
#undef ret


// cleanup
section LZMA_DEC30
        pop     ds

section LZMA_DEC31
        // clear dirty stack
        mov     sp, bp
        mov     bx, offset clear_dirty_stack_low
        xor     ax, ax
1:      push    ax
        cmp     sp, bx
        jnzs    1b

section LZMA_DEC32
        mov     sp, bp
        mov     di, offset lzma_u_len
    ignore_reloc_overflow lzma_u_len

section LZMA_DEC33
        pop     ax
        push    ax
        add     ax, offset lzma_u_len_segment
        mov     es, ax


// vi:ts=4:et
