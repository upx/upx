/* amd_d_nrv2e.S -- AMD64 decompressor for NRV2E

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2004 Laszlo Molnar
   Copyright (C) 2000-2005 John F. Reiser
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

#include "amd_regs.h"

M_NRV2B_LE32=2  # ../conf.h
M_NRV2E_LE32=8

#define ALIGN(n)  .align n

/* Returns 0 on success; non-zero on failure. */
decompress:  # (uchar const *src, size_t lsrc, uchar *dst, size_t &ldst, uint method)

/* Arguments according to calling convention */
#define src  %arg1
#define lsrc %arg2
#define dst  %arg3
#define ldst %arg4  /* Out: actually a reference: &len_dst */
#define meth %arg5l

/* Working registers */
#define off  %eax  /* XXX: 2GB */
#define len  %ecx  /* XXX: 2GB */
#define lenq %rcx
#define bits %ebx
#define disp %rbp

	push %rbp; push %rbx
	push ldst
	push dst
        addq src,lsrc; push lsrc  # &input_eof

	movq src,%rsi  # hardware src for movsb, lodsb
	movq dst,%rdi  # hardware dst for movsb
	subl bits,bits  # empty; force refill
	subl len,len  # create loop invariant
	orq $~0,disp  # -1: initial displacement
	call setup_rdx
ra_setup_rdx:

/* jump on next bit {0,1} with prediction {y==>likely, n==>unlikely} */
/* Prediction omitted for now. */
#define jnextb0n call *%rdx; jnc
#define jnextb0y call *%rdx; jnc
#define jnextb1n call *%rdx; jc
#define jnextb1y call *%rdx; jc

/* rotate next bit into bottom bit of reg */
#define getnextb(reg) call *%rdx; adcl reg,reg

	ALIGN(1<<3)
getbit:
	addl bits,bits; jz refill  # Carry= next bit
	ret
refill:
	movl (%rsi),bits; subq $-4,%rsi  # next 32 bits; set Carry
	adcl bits,bits  # LSB= 1 (CarryIn); CarryOut= next bit
	ret

	ALIGN(1<<3)
lit_n2e:
	movsb  # *%rdi++ = *%rsi++;
top_n2e:
	jnextb1y lit_n2e
	lea 1(lenq),off  # [len= 0] off= 1
        jmp getoff_n2e

off_n2e:
        dec off
        getnextb(off)
getoff_n2e:
        getnextb(off)
        jnextb0n off_n2e

	/*xorl len,len  # len= 0*/
	subl $3,off; jc offprev_n2e
	shll $8,off
	lodsb  # requires off===%eax
	xorl $~0,off; jz eof_n2e
	sarl off  # Carry= original low bit
	movslq off,disp  # XXX: 2GB
	jc lenlast_n2e
	jmp lenmore_n2e

offprev_n2e:
        jnextb1y lenlast_n2e
lenmore_n2e:
        incl len  # len= 1
        jnextb1y lenlast_n2e
len_n2e:
        getnextb(len)
        jnextb0n len_n2e
        addl $6-2-2,len
        jmp gotlen_n2e

lenlast_n2e:
        getnextb(len)  # 0,1,2,3
gotlen_n2e:
	cmpq $-0x500,disp
        adcl $2,len  # len += 2+ (disp < -0x500);
	push %rsi
	leaq (%rdi,disp),%rsi
	rep; movsb  # len
	pop %rsi
bot_n2e:  # In: 0==len
	prefetch 0x7f(%rdi)  # em64t has no prefetchw
        jmp top_n2e


	ALIGN(1<<3)
lit_n2b:
	movsb  # *%rdi++ = %rsi++;
top_n2b:
	jnextb1y lit_n2b
	lea 1(lenq),off  # [len= 0] off= 1
offmore_n2b:
	getnextb(off)
	jnextb0n offmore_n2b

	subl $3,off; jc len_n2b  # use previous offset
	shll $8,off
	lodsb  # requires off===%eax
	xorl $~0,off; jz eof_n2b
	movslq off,disp  # XXX: 2GB
len_n2b:
	lea 1(lenq),off  # [len= 0] off= 1
	getnextb(len); getnextb(len)  # two bits; cc set on result
	jnz gotlen_n2b  # raw 1,2,3 ==> 2,3,4
	movl off,len  # len= 1, the msb
	addl $3-1,off  # raw 2.. ==> 5..
lenmore_n2b:
	getnextb(len)
	jnextb0n lenmore_n2b
gotlen_n2b:
	cmpq $-0xd00,disp
	adcl off,len  # len += off + (disp < -0xd00)
	push %rsi
	leaq (%rdi,disp),%rsi
	rep; movsb  # len
	pop %rsi
bot_n2b:  # In: 0==len
	prefetch 0x7f(%rdi)  # em64t has no prefetchw
        jmp top_n2b

setup_rdx:
	cld
	pop %rdx; addq $ getbit - ra_setup_rdx,%rdx  # %rdx= &getbit
	cmpl $ M_NRV2E_LE32,meth; je bot_n2e
	cmpl $ M_NRV2B_LE32,meth; je bot_n2b
eof_n2e:
eof_n2b:
	pop %rcx  # &input_eof
	movq %rsi,%rax; subq %rcx,%rax  # src -= eof;  // return 0: good; else: bad
	pop %rdx;       subq %rdx,%rdi  # dst -= original dst
	pop %rcx;            movl %edi,(%rcx)  # actual length used at dst  XXX: 4GB
	pop %rbx; pop %rbp
	ret

