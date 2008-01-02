/* nrv2b_d8.S -- ARM decompressor for NRV2B

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

#define src  r0
#define len  r1
#define dst  r2
#define tmp  r3
#define bits r4
#define off  r5

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

#undef  GETBIT
#define GETBIT  bl get1_n2b

#define getnextb(reg) GETBIT; ADC2S(reg,reg) /* Set Condition Codes on result */
#define   jnextb0     GETBIT; bcc
#define   jnextb1     GETBIT; bcs

ucl_nrv2b_decompress_8: .globl ucl_nrv2b_decompress_8  // ARM mode
        .type ucl_nrv2b_decompress_8, %function
/* error = (*)(char const *src, int len_src, char *dst, int *plen_dst) */
        add r1,len,src  // r1= eof_src;
        PUSH {r1,r2,r3, r4,r5, lr}
        mvn off,#~-1  // off= -1 initial condition
        mov bits,#1<<31  // refill next time
        b top_n2b

eof_n2b:
        POP {r1,r3,r4}  // r1= eof_src; r3= orig_dst; r4= plen_dst
        SUB2(src,r1)  // 0 if actual src length equals expected length
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

        POP {r4,r5, pc}  // return

get1_n2b:
        ADD2S(bits,bits)  // shift left 1 bit, set condition codes
        movne pc,lr  // conditionally return CarryOut if rest is non-empty
get8_n2b:  // In: Carry set [from adding 0x80000000 (1<<31) to itself]
        LDRB3(bits,[src],#1)  // zero-extend next byte
        ADC2(bits,bits)  // double and insert CarryIn as low bit
        movs bits,bits,lsl #24  // move to top byte, and set CarryOut from old bit 8
        mov pc,lr

ss11_n2b:  // return len= [2..)
        mov len,#1  // the msb
        mov ip,lr  // outer ret.addr
1:
        getnextb(len)
        jnextb0 1b
        mov pc,ip  // outer ret

lit_n2b:
        LDRB3(tmp,[src],#1)
        STRB3(tmp,[dst],#1)
top_n2b:
        jnextb1 lit_n2b

        bl ss11_n2b  // len= [2..)
        subs tmp,len,#3  // set Carry
        mov len,#0  // Carry unaffected
        blo offprev_n2b  // ss11 returned 2
        LDRB3(off,[src],#1)  // low 8 bits
        orr  off,off,tmp,lsl #8
        mvns off,off; beq eof_n2b  // off= ~off
offprev_n2b:  // In: 0==len
        getnextb(len); getnextb(len); bne 1f  // two bits; 1,2,3 ==> 2,3,4
        bl ss11_n2b  // len= [2..)
        ADD2(len,#2)  // [2..) ==> [4..);
1:
/* 'cmn': add the inputs, set condition codes, discard the sum */
        cmn off,#0xd<<8  // within M2_MAX_OFFSET
        addcc len,len,#1  // too far away, so minimum match length is 3
copy_n2b:  // copy 1+len bytes
        ldrb  tmp,[dst,off]; SUB2S(len,#1)
        STRB3(tmp,[dst],#1); bhs copy_n2b
        b top_n2b  // exit with -1==len

        .size ucl_nrv2b_decompress_8, .-ucl_nrv2b_decompress_8
/*
vi:ts=8:et:nowrap
 */

