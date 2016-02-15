/* lzma_d.S -- ARM decompressor for LZMA

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2015 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2015 Laszlo Molnar
   Copyright (C) 2000-2015 John F. Reiser
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
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>

   John F. Reiser
   <jreiser@users.sourceforge.net>
*/

#define section .section

  section LZMA_ELF00
//decompress:  // (uchar const *src, size_t lsrc, uchar *dst, u32 &ldst, uint method)
/* Arguments according to calling convention */
#define src  x0
#define lsrc w1
#define dst  x2
#define ldst x3  /* Out: actually a reference: &len_dst */
#define meth w4

  // bkpt  // debugging

#define M_LZMA          14
        cmp meth,#M_LZMA; bne not_lzma

#if defined(LINUX_ARM_CACHEFLUSH)||defined(DARWIN_ARM_CACHEFLUSH)  /*{*/
        PUSH4(dst,ldst, fp,lr)  // dst,ldst for cache flush
#else  /*}{*/
        PUSH2(          fp,lr)
#endif  /*}*/

#define a0 x0
#define a1 x1
#define a2 w2
#define a3 x3
#define a4 x4  /* outp */
#define a5 w5  /* outSize */
#define a6 x6  /* &outSizeProcessed */
#define inSzP      3*4  /*   inSizeprocessed */
#define State      4*4  /* CLzmaDecoderState */
#define t0 w7
#define t1 w8
#define t1x x8

//LzmaDecode(  // from lzmaSDK/C/7zip/Compress/LZMA_C/LzmaDecode.h
//      a0= &CLzmaDecoderState,
//      a1= inp,  a2= inSize,  a3= &inSizeProcessed,
//      a4= outp, a5= outSize, a6= &outSizeProcessed
//)
#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE   768

        ldrb t0,[src,#0]  // first byte, replaces LzmaDecodeProperties()
        mov t1,#2*LZMA_LIT_SIZE
        lsr t0,t0,#3  // lit_context_bits + lit_pos_bits
        lsl t1,t1,t0  // 2*LZMA_LIT_SIZE << (lit_context_bits + lit_pos_bits)
        mov t0,sp
#define W 4  /* even #bits to round up so that 8 bits span all the 1's */
        add t1,t1,#((~(~0<<W) + State + 2*LZMA_BASE_SIZE)>>W)<<W
        sub sp,sp,t1  // alloca

        ldr t1,[ldst]
        str ldst,a6  // &outSizeProcessed
        str t1,  a5  // outSize
        str dst, a4   // outp

        add r3,sp,#inSzP
        mov t1x,#0
1:  // clear inSizeProcessed and CLzmaDecoderState
        str t1x,[r3],#8
        cmp r3,t0
        blo 1b

        add a3,sp,#inSzP // &inSizeProcessed
        sub a2,lsrc,#2  // inSize
        mov a1, src  // inp

        ldrb t1,[a1],#1  // first byte, replaces LzmaDecodeProperties()
        and  t1,t1,#7  // posBits
        strb t1,[sp,#2 + State]
        ldrb t1,[a1],#1  // second byte, replaces LzmaDecodeProperties()
        mov  a0,t1,LSR #4  // lit_pos_bits
        strb a0,[sp,#1 + State]
        and  t1,t1,#0xf  // lib_context_bits
        strb t1,[sp,#0 + State]

        add a0,sp,#State
        bl 1f  // the call
        mov sp,t0  // un-alloca

#if defined(LINUX_ARM_CACHEFLUSH)  /*{*/
        mov w3,w0  // save result value
        POP2(x0,x1)  // dst, ldst
        ldr x1,[x1]  // ldst by reference
        add x1,x1,x0  // just beyond what was written
        mov w2,#0
        do_sys2 __ARM_NR_cacheflush  // decompressed region
        mov x0,x3  // result value
#endif  /*}*/
#if defined(DARWIN_ARM_CACHEFLUSH)  /*{*/
        mov w4,w0  // save result value
        POP2(x0,x1)  // dst, ldst
        ldr x1,[x1]  // ldst by reference
        PUSH2(x0,x1); do_dcache_flush
        POP2 (x0,x1); do_icache_invalidate
        mov x0,x4  // result value
#endif  /*}*/

        POP {fp,pc}
1:

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

