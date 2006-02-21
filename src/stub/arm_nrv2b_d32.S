/* arm_nrv2b_d32.S -- ARM decompressor for NRV2E

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2004 Laszlo Molnar
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
#define lr2  r6
#define g32  r7

ucl_nrv2b_decompress_32: .globl ucl_nrv2b_decompress_32
		@ ARM mode (char *src, int len_src, char *dst, int *plen_dst)
	add r1,len,src  @ r1= eof_src;
	stmfd sp!,{r1,r2,r3, r4,r5,r6,r7,lr}
	mov bits,#1<<31
	blx hitch_n2b
hitch_n2b_r:
	ldmfd sp!,{r4,r5,r6,r7,pc}

get32:  @ ARM mode; In: Carry set (unchanged until final adc)
	ldrb bits,[src],#1
	ldrb tmp, [src],#1; orr bits,bits,tmp,lsl #1*8
	ldrb tmp, [src],#1; orr bits,bits,tmp,lsl #2*8
	ldrb tmp, [src],#1; orr bits,bits,tmp,lsl #3*8
	adc bits,bits,bits
	bx lr
unhitch_n2b:

#define GETBIT \
   add bits,bits; beq 1f; 0: \
   .subsection 1; \
1: blx getb; b 0b; \
   .subsection 0

#define getnextb(reg) GETBIT; adc reg,reg
#define   jnextb0     GETBIT; bcc
#define   jnextb1     GETBIT; bcs

	.code 16  @ THUMB mode
eof_n2b:
	pop {r1,r3,r4}  @ r1= eof_src; r3= orig_dst; r4= plen_dst
	sub src,r1  @ 0 if src length OK
	sub dst,r3  @ actual dst length
	str dst,[r4]
	sub g32,#get32 - hitch_n2b_r  @ g32= &hitch_n2b_r
	bx g32
hitch_n2b:
	mov g32,lr
	add g32,#get32 - hitch_n2b_r  @ g32= &get32
	b top

lit_n2b:
	ldrb tmp,[src]; add src,#1
	strb tmp,[dst]; add dst,#1
top_n2b:
	jnextb1 lit_n2b

	bl ss11  @ len= offset
	sub len,#3; bcs off_same
	ldrb off,[src]; add src,#1  @ low 8 bits
	lsl len,#8
	orr off,len
	mvn off,off; beq eof_n2b  @ off= ~off
off_same:
	bl ss11  @ len= count -2
	mov tmp,#0xd; lsl tmp,#8
	cmn tmp,len
	mov tmp,#2  @ does not modify Carry
	adc len,tmp  @ len += 2+ (0xd00<=offset)
	ldrb tmp,[dst]  @ force cacheline allocate
copy_n2v:
	ldrb tmp,[dst,off]
	strb tmp,[dst]; add dst,#1
	sub len,#1; bne copy_n2b
	b top_n2b

ss11:
	mov len,#1  @ the msb
	mov lr2,lr  @ save return address
ss11a:
	getnextb(len)
	jnextb0 ss11a
	mov pc,lr2  @ return
