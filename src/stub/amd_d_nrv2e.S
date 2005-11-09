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

SZ_DLINE=128  # size of data cache line in Apple G5

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
#define bits %ebx
#define disp %rbp

	push %rbp; push %rbx
	push ldst
	push dst
        addq src,lsrc; push lsrc  # &input_eof

	movq src,%rsi  # hardware src
	movl $1<<31,%ebx  # force refill
	movq dst,%rdi  # hardware dst
	orq $~0,disp  # -1: initial displacement
	call setup_rdx_n2e
ra_setup_rdx:

/* jump on next bit {0,1} with prediction {y==>likely, n==>unlikely} */
/* Prediction omitted for now. */
#define jnextb0n call *%rdx; jnc
#define jnextb0y call *%rdx; jnc
#define jnextb1n call *%rdx; jc
#define jnextb1y call *%rdx; jc

/* rotate next bit into bottom bit of reg */
#define getnextb(reg) call *%rdx; adcl reg,reg

	/*.align 1<<4  # not effective unless upx pays attention */
get_refill_n2e:  # In: 1==Carry
	movl (%rsi),bits; leaq 4(%rsi),%rsi  # next 32 bits
	adcl bits,bits  # LSB= 1 (CarryIn); CarryOut= next bit
	ret
getbit:
	addl bits,bits; jz get_refill_n2e  # Carry= next bit
	ret

lit_n2e:
	movsb  # *%rdi++ = *%rsi++;
top_n2e:
	call *%rdx
	jc lit_n2e
        movl $1,off
        jmp getoff_n2e

	/*.align 1<<4  # not effective unless upx pays attention */
off_n2e:
        dec off
        getnextb(off)
getoff_n2e:
        getnextb(off)
        jnextb0n off_n2e

	xorl len,len  # len= 0
	subl $3,off; jc offprev_n2e
	shll $8,off
	lodsb  # requires off===%eax
	xorl $~0,off; jz eof_n2e
	sarl off  # Carry= original low bit
	movslq off,disp
	jc lenlast_n2e
	jmp lenmore_n2e

offprev_n2e:
        jnextb1y lenlast_n2e
lenmore_n2e:
        movl $1,len
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
	rep; movsb
	pop %rsi
bot_n2e:
	movb 2*SZ_DLINE(%rdi),%al  # prefetch for store
        jmp top_n2e

setup_rdx_n2e:
	pop %rdx; addq $ getbit - ra_setup_rdx,%rdx
	cmpl $ M_NRV2E_LE32,meth; jne eof_n2e
        jmp bot_n2e

eof_n2e:
	pop %rcx  # &input_eof
	movq %rsi,%rax; subq %rcx,%rax  # src -= eof;  // return 0: good; else: bad
	pop %rdx; subq %rdx,%rdi  # dst -= original dst
	pop %rcx;      movl %edi,(%rcx)  # actual length used at dst  XXX: 4GB
	pop %rbx; pop %rbp
	ret

