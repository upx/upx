/* nrv2d_d8.S -- ARM decompressor for NRV2D

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
   Copyright (C) 2000-2008 John F. Reiser
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
#define SAFE 0  /* 1 for src+dst bounds checking: cost 76 bytes */

#define src  r0
#define len  r1  /* overlaps 'cnt' */
#define dst  r2
#define tmp  r3
#define bits r4
#define off  r5
/*           r6  UNUSED in ARM code unless DEBUG mode */
#define srclim r7
#if 1==SAFE  /*{*/
#define dstlim r12
#endif  /*}*/

#define cnt  r1  /* overlaps 'len' while reading an offset */

/* macros reduce "noise" when comparing this ARM code to corresponding THUMB code */
#define PUSH stmdb sp!,
#define POP  ldmia sp!,
#define ADD2( dst,src) add  dst,dst,src
#define ADD2S(dst,src) adds dst,dst,src
#define ADC2( dst,src) adc  dst,dst,src
#define ADC2S(dst,src) adcs dst,dst,src
#define SUB2( dst,src) sub  dst,dst,src
#define SUB2S(dst,src) subs dst,dst,src
#define LDRB3(reg,psrc,incr) ldrb reg,psrc,incr
#define STRB3(reg,pdst,incr) strb reg,pdst,incr

#if 1==SAFE  /*{*/
#define CHECK_SRC  cmp srclim,src; bls bad_src_n2d /* Out: 1==Carry for get8_n2d */
#define CHECK_DST  cmp dst,dstlim; bhs bad_dst_n2d
#else  /*}{*/
#define CHECK_SRC  /*empty*/
#define CHECK_DST  /*empty*/
#endif  /*}*/

#if 0  /*{ DEBUG only: check newly-decompressed against original dst */
#define CHECK_BYTE \
   ldrb  r6,[dst]; \
   cmp   r6,tmp; beq 0f; bkpt; 0:
#else  /*}{*/
#define CHECK_BYTE  /*empty*/
#endif  /*}*/

#undef  GETBIT
#define GETBIT  ADD2S(bits,bits); bleq get8_n2d

#undef  getnextb
#define getnextb(reg) GETBIT; ADC2S(reg,reg) /* Out: condition code changed */
#define   jnextb0     GETBIT; bcc
#define   jnextb1     GETBIT; bcs

ucl_nrv2d_decompress_8: .globl ucl_nrv2d_decompress_8  // ARM mode
        .type ucl_nrv2d_decompress_8, %function
/* error = (*)(char const *src, int len_src, char *dst, int *plen_dst)
   Actual decompressed length is stored through plen_dst.
   For SAFE mode: at call, *plen_dst must be allowed length of output buffer.
*/
        PUSH {r2,r3, r4,r5,r6,r7, lr}
#define sp_DST0 0  /* stack offset of original dst */
        add srclim,len,src  // srclim= eof_src;
#if 1==SAFE  /*{*/
        ldr tmp,[r3]  // len_dst
        add dstlim,tmp,dst
#endif  /*}*/
        mvn off,#~-1  // off= -1 initial condition
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
        POP {r3,r4}  // r3= orig_dst; r4= plen_dst
        SUB2(src,srclim)  // 0 if actual src length equals expected length
        SUB2(dst,r3)  // actual dst length
        str dst,[r4]

#if defined(LINUX_ARM_CACHEFLUSH)  /*{*/
/* linux/include/asm-arm/unistd.h */
#define __NR_SYSCALL_BASE       0x900000
#define __ARM_NR_BASE           (__NR_SYSCALL_BASE+0x0f0000)
#define __ARM_NR_cacheflush     (__ARM_NR_BASE+2)
        mov r4,r0  // save result value
        mov r0,r3  // orig_dst
        add r1,r3,dst  // orig_dst + dst_len
        mov r2,#0
        swi __ARM_NR_cacheflush  // decompressed region
        mov r0,r4  // result value
#endif  /*}*/

        POP {r4,r5,r6,r7 ,pc}

get8_n2d:  // In: Carry set [from adding 0x80000000 (1<<31) to itself]
        CHECK_SRC; LDRB3(bits,[src],#1)  // zero-extend next byte
        adc  bits,bits,bits  // double and insert CarryIn as low bit
        movs bits,bits,lsl #24  // move to top byte, and set CarryOut from old bit 8
        mov pc,lr

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
        mvns off,off; beq eof_n2d  // off= ~off
        movs off,off,asr #1
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
        addcc len,len,#1  // too far away, so minimum match length is 3
#if 1==SAFE  /*{*/
        ldr tmp,[sp,#sp_DST0]
        SUB2( tmp,dst)
        SUB2S(tmp,off); bhi bad_dst_n2d  // reaching back too far

        add tmp,dst,cnt
        cmp tmp,dstlim; bhi bad_dst_n2d  // too much output
#endif  /*}*/
        ldrb tmp,[dst]  // force cacheline allocate
copy_n2d:
        ldrb tmp,[dst,off]
        CHECK_BYTE
        STRB3(tmp,[dst],#1)
        SUB2S(len,#1); bne copy_n2d
        b top_n2d

        .size ucl_nrv2d_decompress_8, .-ucl_nrv2d_decompress_8

/*
vi:ts=8:et:nowrap
 */

