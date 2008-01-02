/* arm_nrv2b_d32.S -- ARM decompressor for NRV2B

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
#define g1b  r6
#define wrnk r7  /* 0xd00  M2_MAX_OFFSET before "wrinkle" */

/* r12 ("ip") is assumed to be a scratch register. */

#define GETBIT blx g1b

#define getnextb(reg) GETBIT; adc reg,reg
#define   jnextb0     GETBIT; bcc
#define   jnextb1     GETBIT; bcs

#ifndef PURE_THUMB
ucl_nrv2b_decompress_32: .globl ucl_nrv2b_decompress_32  @ ARM mode
        .type ucl_nrv2b_decompress_32, %function
/* error = (*)(char const *src, int len_src, char *dst, int *plen_dst) */
        adr r12,1+.thumb_nrv2b_d32  @ load pc-relative address
        bx  r12  @ enter THUMB mode
        .code 16  @ THUMB mode
        .thumb_func
#endif

.thumb_nrv2b_d32:
        add r1,len,src  @ r1= eof_src;
        push {r1,r2,r3, r4,r5,r6,r7, lr}
        mov bits,#1; neg off,bits  @ off= -1 initial condition
        lsl bits,#31  @ 1<<31: refill next time
        mov wrnk,#0xd
        lsl wrnk,#8  @ 0xd00
        adr g1b,get1b  @ load pc-relative address
        b top_n2b

eof_n2b:
        pop {r1,r3,r4}  @ r1= eof_src; r3= orig_dst; r4= plen_dst
        sub src,r1  @ 0 if actual src length equals expected length
        sub dst,r3  @ actual dst length
        str dst,[r4]
        pop {r4,r5,r6,r7, pc}  @ return

lit_n2b:
        ldrb tmp,[src]; add src,#1
        strb tmp,[dst]; add dst,#1
top_n2b:
        jnextb1 lit_n2b

        mov len,#1  @ the msb
getoff_n2b:  @ ss11 len= [2..)
        getnextb(len)
        jnextb0 getoff_n2b

        sub tmp,len,#3  @ set Carry
        mov len,#0  @ Carry unaffected
        blo offprev_n2b  @ ss11 returned 2
        lsl tmp,#8
        ldrb off,[src]; add src,#1  @ low 8 bits
        orr  off,tmp
        mvn  off,off; beq eof_n2b  @ off= ~off
offprev_n2b:  @ In: 0==len
        getnextb(len); getnextb(len); bne plus1_n2b  @ two bits; 1,2,3 ==> 2,3,4

        mov len,#1  @ the msb
getlen_n2b:  @ ss11 len= [2..)
        getnextb(len)
        jnextb0 getlen_n2b

        add len,#2  @ [2..) ==> [4..);
plus1_n2b:
        add len,#1  @ 1,2,3 ==> 2,3,4; [4..) ==> [5..)
/* 'cmn': add the inputs, set condition codes, discard the sum */
        cmn off,wrnk; bcs near_n2b  @ within M2_MAX_OFFSET
        add len,#1  @ too far away, so minimum match length is 3
near_n2b:
        ldrb tmp,[dst]  @ force cacheline allocate
copy_n2b:
        ldrb tmp,[dst,off]
        strb tmp,[dst]; add dst,#1
        sub len,#1; bne copy_n2b
        b top_n2b

        .code 32  @ ARM mode
get1b:  @ Out: Carry= next_bit
        adds bits,bits,bits  @ shift up, set Carry
        bxne lr  @ return if reload is not needed
/* In: Carry set (unchanged until final adcs) */
        ldrb bits,[src],#1
        ldrb tmp, [src],#1; orr bits,bits,tmp,lsl #1*8
        ldrb tmp, [src],#1; orr bits,bits,tmp,lsl #2*8
        ldrb tmp, [src],#1; orr bits,bits,tmp,lsl #3*8
        adcs bits,bits,bits  @ Carry= next_bit; set bit0 flag from CarryIn
        bx lr
        .code 16  @ THUMB mode

#ifndef PURE_THUMB
        .size ucl_nrv2b_decompress_32, .-ucl_nrv2b_decompress_32
#endif

/*
vi:ts=8:et:nowrap
 */

