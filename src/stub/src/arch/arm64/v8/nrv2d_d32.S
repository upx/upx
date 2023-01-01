/* nrv2d_d32.S -- ARM64 decompressor for NRV2D

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2023 Laszlo Molnar
   Copyright (C) 2000-2023 John F. Reiser
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
#include "macros.S"

#define SAFE 0  /* 1 for src+dst bounds checking: cost 76 bytes */

#define lr   x30

#define src  x0
#define len  w1  /* overlaps 'cnt' */
#define dst  x2
#define dstw w2
#define tmp  w3
#define tmpx x3
#define bits w4
#define off  w5
/*           r6  UNUSED in ARM code unless DEBUG mode */
#define srclim x7
#if 1==SAFE  /*{*/
#define dstlim x12  /* FIXME */
#endif  /*}*/

#define cnt  w1  /* overlaps 'len' while reading an offset */

/* macros reduce "noise" when comparing this ARM code to corresponding THUMB code */
#define ADD2( dst,src) add  dst,dst,src
#define ADD2S(dst,src) adds dst,dst,src
#define ADC2( dst,src) adc  dst,dst,src
#define ADC2S(dst,src) adcs dst,dst,src
#define SUB2( dst,src) sub  dst,dst,src
#define SUB2S(dst,src) subs dst,dst,src
#define LDR3( reg,psrc,incr) ldr  reg,psrc,incr
#define LDRB3(reg,psrc,incr) ldrb reg,psrc,incr
#define STRB3(reg,pdst,incr) strb reg,pdst,incr

#if 1==SAFE  /*{*/
#define CHECK_SRC  cmp srclim,src; bls bad_src_n2d /* Out: 1==Carry for get32_n2d */
#define CHECK_DST  cmp dst,dstlim; bhs bad_dst_n2d
#else  /*}{*/
#define CHECK_SRC  /*empty*/
#define CHECK_DST  /*empty*/
#endif  /*}*/

#if 0  /*{ DEBUG only: check newly-decompressed against original dst */
#define CHECK_BYTE \
   ldrb  w6,[dst]; \
   cmp   w6,tmp; beq 0f; bkpt; 0:
#else  /*}{*/
#define CHECK_BYTE  /*empty*/
#endif  /*}*/

#undef  GETBIT
#define GETBIT bl get1_n2d

#undef  getnextb
#define getnextb(reg) GETBIT; ADC2S(reg,reg) /* Out: condition code changed */
#define   jnextb0     GETBIT; bcc
#define   jnextb1     GETBIT; bcs

ucl_nrv2d_decompress_32: .globl ucl_nrv2d_decompress_32  // ARM mode
        .type ucl_nrv2d_decompress_32, %function
/* error = (*)(char const *src, int len_src, char *dst, int *plen_dst)
   Actual decompressed length is stored through plen_dst.
   For SAFE mode: at call, *plen_dst must be allowed length of output buffer.
*/
        PUSH3(x2,x3, lr)
#define sp_DST0 0  /* stack offset of original dst */
        add srclim,src,len,uxtw  // srclim= eof_src;
#if 1==SAFE  /*{*/
        ldr tmp,[x3]  // len_dst
        add dstlim,dst,tmp,uxtw
#endif  /*}*/
        mov off,#-1  // off= -1 initial condition
        mov bits,#1<<31  // refill next time
        b top_n2d

#if 1==SAFE  /*{*/
bad_dst_n2d:  # return value will be 2
bkpt
        add src,srclim,#1
bad_src_n2d:  # return value will be 1
        ADD2(src,#1)
#endif  /*}*/
eof_n2d:
        POP2(x3,x4)  // r3= orig_dst; r4= plen_dst
        SUB2(src,srclim)  // 0 if actual src length equals expected length
        SUB2(dst,x3)  // actual dst length
        str dstw,[x4]
        mov x5,x0  // save result value

        mov x0,x3  // orig_dst
        add x1,x3,dst  // orig_dst + dst_len
cache_n2d:
        clear_cache  // x0= lo; x1= hi; uses x2-x4

        mov x0,x5  // result value
        POP1(lr)
        ret

get1_n2d:
        ADD2S(bits,bits); cbz bits,get32_n2d; ret
get32_n2d:  // In: Carry set [from adding 0x80000000 (1<<31) to itself]
        CHECK_SRC; LDR3(bits,[src],#4)
        ADC2S(bits,bits)  // left shift one bit with CarryIn and CarryOut
        ret

lit_n2d:
        CHECK_SRC; LDRB3(tmp,[src],#1)
        CHECK_BYTE
        CHECK_DST; STRB3(tmp,[dst],#1)
top_n2d:
        jnextb1 lit_n2d
        mov cnt,#1; b getoff_n2d

off_n2d:
        SUB2(cnt,#1)
        getnextb(cnt)
getoff_n2d:
        getnextb(cnt)
        jnextb0 off_n2d

        subs tmp,cnt,#3  // set Carry
        mov len,#0  // Carry unaffected
        blo offprev_n2d  // cnt was 2; tests Carry only
        CHECK_SRC; LDRB3(off,[src],#1)  // low 7+1 bits
        orr  off,off,tmp,lsl #8
        mvn off,off; cbz off,eof_n2d  // off= ~off
        lsl tmp,off,#31; lsr off,off,#1; adds tmp,tmp,tmp
        b len_n2d -4 // CHEAT [getnextb ends in ADC2(reg,reg)]

offprev_n2d:
        getnextb(len)
len_n2d:
        getnextb(len); bne gotlen_n2d  // 1..3  getnextb() must set Condition Code
        mov len,#1  // begin ss11
lenmore_n2d:
        getnextb(len)
        jnextb0 lenmore_n2d
        ADD2(len,#2)  // 2.. ==> 4..
gotlen_n2d:  // 'cmn': add the inputs, set condition codes, discard the sum
        ADD2(len,#1)  // 1..3 ==> 2..4;  4.. ==> 5..
        cmn off,#5<<8  // displ<=M2_MAX_OFFSET ==> no increment
        csinc len,len,len,cc  // too far away, so minimum match length is 3
#if 1==SAFE  /*{*/
        ldr tmp,[sp,#sp_DST0]
        SUB2( tmp,dst)
        SUB2S(tmp,off); bhi bad_dst_n2d  // reaching back too far

        add tmp,dst,cnt
        cmp tmp,dstlim; bhi bad_dst_n2d  // too much output
#endif  /*}*/
        ldrb tmp,[dst]  // force cacheline allocate
copy_n2d:
        ldrb tmp,[dst,off,sxtw]
        CHECK_BYTE
        STRB3(tmp,[dst],#1)
        SUB2S(len,#1); bne copy_n2d
        b top_n2d

        .size ucl_nrv2d_decompress_32, .-ucl_nrv2d_decompress_32

/*
vi:ts=8:et:nowrap
 */

