/* lzma_d.S -- arm64 decompressor for LZMA

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2018 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2018 Laszlo Molnar
   Copyright (C) 2000-2018 John F. Reiser
   All Rights Reserved.

   UPX and the UCL library are free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer              Laszlo Molnar
   <markus@oberhumer.com>               <ezerotven+github@gmail.com>

   John F. Reiser
   <jreiser@users.sourceforge.net>
*/

#define section .section

  section LZMA_ELF00
//decompress:  // (uchar const *src, size_t lsrc, uchar *dst, u32 &ldst, uint method)
/* Arguments according to calling convention */
src .req x0
#define lsrc w1
dst .req x2
#define ldst x3  /* Out: actually a reference: &len_dst */
#define meth w4

#define M_LZMA          14
        cmp meth,#M_LZMA; bne not_lzma

        add x1,src,len,uxtw  // x1= eof_src;
        mov x4,#0
        PUSH4(x1,x2,x3, lr)

        PUSH2(x4,x4)  // clear CLzmaDecoderState, inSizeProcessed, outSizeProcessed
State= 0
#define a0 x0  /* &CLzmaDecoderState */
        add a0,sp,#State
#define a1 x1  /* inp */
#define a2 w2  /* inSize */
#define a3 x3  /* &inSizeProcessed */
        add a3,sp,#8
#define a4 x4  /* outp */
#define a5 w5  /* outSize */
#define a6 x6  /* &outSizeProcessed */
        add a6,sp,#12

#define t0  w7
#define t1  w8
#define t1x x8

#define fp  x29

//The target is:
//LzmaDecode(  // from lzmaSDK/C/7zip/Compress/LZMA_C/LzmaDecode.h
//      a0= &CLzmaDecoderState,
//      a1= inp,  a2= inSize,  a3= &inSizeProcessed,
//      a4= outp, a5= outSize, a6= &outSizeProcessed
//)
#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE   768

        ldrb t1,[src,#0]  // first byte, replaces LzmaDecodeProperties()
        mov t0,#2*LZMA_LIT_SIZE
        lsr t1,t1,#3  // lit_context_bits + lit_pos_bits
        lslv t0,t0,t1  // 2*LZMA_LIT_SIZE << (lit_context_bits + lit_pos_bits)
        mov fp,sp
#define W 4  /* even #bits to round up so that 8 bits span all the 1's */
        add t0,t0,#((~(~0<<W) + 2*LZMA_BASE_SIZE)>>W)<<W

        mov t1x,sp
        sub sp,sp,t0,uxtw

1:  // clear inSizeProcessed and CLzmaDecoderState
        stp x4,x4,[x3],#2*8
        cmp x3,t1x
        blo 1b

        mov a6,ldst  // &outSizeProcessed
        mov a5,t0  // outSize
        mov a4,dst  // outp
        sub w2,lsrc,#2  // inSize
        mov a1,src  // inp

        ldrb t0,[a1],#1  // first byte, replaces LzmaDecodeProperties()
        and  t0,t0,#7  // posBits
        strb t0,[sp,#2 + State]
        ldrb t0,[a1],#1  // second byte, replaces LzmaDecodeProperties()
        lsr  t1,t0,#4  // lit_pos_bits
        strb t1,[sp,#1 + State]
        and  t0,t0,#0xf  // lib_context_bits
        strb t0,[sp,#0 + State]

        add a0,sp,#State
        bl 1f  // the call
        mov sp,fp

eof_lzma:
        POP4(x1,x3,x4, lr)  // x1= eof_src; r3= orig_dst; r4= plen_dst
        SUB2(src,x1)  // 0 if actual src length equals expected length
        SUB2(dst,x3)  // actual dst length
        str dstw,[x4]
        mov x4,x0  // save result value

        mov x0,x3  // orig_dst
        add x1,x3,dst  // orig_dst + dst_len
cache_lzma:
        dc cvau,x0  // Clean by VA to point of Unification
        ic ivau,x0  // Invalidate by VA to point of Unification
        add x0,x0,#64  // next line
        cmp x0,x1; blo cache_lzma

        mov x0,x4  // result value
        ret

#undef t0
#undef t1
#undef t1x

#undef lsrc
#undef ldst
#undef meth

#undef a0
#undef a1
#undef a2
#undef a3
#undef a4
#undef a5
#undef a6

1:  // entry to C-lang LzmaDecode

  section LZMA_DEC20
#include "lzma_d_cf.S"

  section LZMA_DEC10
#if 0  /*{*/
#include "lzma_d_cs.S"
#else  /*}{*/
#define PARAMETER_STYLE 3
#include "lzma_d-arm.S"
#endif  /*}*/

  section LZMA_DEC30

not_lzma:

// vi:ts=8:et

