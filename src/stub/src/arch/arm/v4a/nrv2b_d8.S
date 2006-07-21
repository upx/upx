/* nrv2b_d8.S -- ARM decompressor for NRV2B

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2006 Laszlo Molnar
   Copyright (C) 2000-2006 John F. Reiser
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
   <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>

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
#define GETBIT  ADD2S(bits,bits); bleq get8_n2b

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
        POP {r4,r5, pc}  // return

get8_n2b:  // In: Carry set [from adding 0x80000000 (1<<31) to itself]
        LDRB3(bits,[src],#1)  // zero-extend next byte
        adc  bits,bits,bits  // double and insert CarryIn as low bit
        movs bits,bits,lsl #24  // move to top byte, and set CarryOut from old bit 8
        mov pc,lr

lit_n2b:
        LDRB3(tmp,[src],#1)
        STRB3(tmp,[dst],#1)
top_n2b:
        jnextb1 lit_n2b

        mov len,#1  // the msb
getoff_n2b:  // ss11 len= [2..)
        getnextb(len)
        jnextb0 getoff_n2b

        sub tmp,len,#3  // set Carry
        mov len,#0  // Carry unaffected
        blo offprev_n2b  // ss11 returned 2
        LDRB3(off,[src],#1)  // low 8 bits
        orr  off,off,tmp,lsl #8
        mvns off,off; beq eof_n2b  // off= ~off
offprev_n2b:  // In: 0==len
        getnextb(len); getnextb(len); bne plus1_n2b  // two bits; 1,2,3 ==> 2,3,4

        mov len,#1  // the msb
getlen_n2b:  // ss11 len= [2..)
        getnextb(len)
        jnextb0 getlen_n2b
        ADD2(len,#2)  // [2..) ==> [4..);
plus1_n2b:
        ADD2(len,#1)  // 1,2,3 ==> 2,3,4; [4..) ==> [5..)
/* 'cmn': add the inputs, set condition codes, discard the sum */
        cmn off,#0xd<<8  // within M2_MAX_OFFSET
        addcc len,len,#1  // too far away, so minimum match length is 3
        ldrb tmp,[dst]  // force cacheline allocate
copy_n2b:
        ldrb tmp,[dst,off]
        STRB3(tmp,[dst],#1)
        SUB2S(len,#1); bne copy_n2b
        b top_n2b

        .size ucl_nrv2b_decompress_8, .-ucl_nrv2b_decompress_8
/*
vi:ts=8:et:nowrap
 */

