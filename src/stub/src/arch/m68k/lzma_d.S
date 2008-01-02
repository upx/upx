/*
;  lzma_d.S --
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

; decompress from a3 to d4
;   note: must preserve d4 and a5-a7
;
; On entry:
;   a3  src pointer
;   d4  dest pointer
;
; On exit:
;   d3.l = 0
*/


section lzma.init
        move.l  sp,d7                   // save stack
        lea     lzma_stack_adjust:w(sp),sp

        addq.l  #2,a3                   // skip 2 bytes for properties

        move.l  sp,a2

        // push 32 bytes on stack
        pea     (a2)                    // &outSizeProcessed
        move.l  #lzma_u_len,-(sp)       // outSize
        move.l  d4,-(sp)                // out
        pea     4(a2)                   // &inSizeProcessed
        move.l  #lzma_c_len,-(sp)       // inSize
        move.l  a3,-(sp)                // in
        pea     8(a2)                   // &CLzmaDecoderState
        move.l  d0,-(sp)                // dummy return address (instead of call)

        // hardwired LzmaDecodeProperties()
        move.l  #lzma_properties,8(a2)   // lc, lp, pb, dummy


section lzma.fast
#include "m68000/lzma_d_cf.S"


section lzma.small
#include "m68000/lzma_d_cs.S"


section lzma.finish
        moveq.l #0,d3

        //add.l   4(a2),a3                // inSizeProcessed
        //add.l   (a2),d4                 // outSizeProcessed

#if 0
        move.l  d7,sp                   // restore stack
#else

// Clear dirty stack (also see m68000/tmp/lzma_d_c[fs].out.disasm)
//     32 bytes pushed above
//     68 bytes local stack in LzmaDecode
//     40 bytes for saving 10 registers in LzmaDecode
//     12 bytes for calling __mulsi3
//    152 *TOTAL* (round up by 16 to 160)

        lea     -160(a2),sp
.clearstack2:
        move.l  d3,(sp)+
        move.l  d3,(sp)+
        move.l  d3,(sp)+
        move.l  d3,(sp)+
        cmp.l   sp,d7
        bnes    .clearstack2
#endif


// vi:ts=8:et

