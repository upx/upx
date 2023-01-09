/*
;  amd64-expand.S -- decompressors for amd64
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2021 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2021 Laszlo Molnar
;  Copyright (C) 2000-2021 John F. Reiser
;  All Rights Reserved.
;
;  UPX and the UCL library are free software; you can redistribute them
;  and/or modify them under the terms of the GNU General Public License as
;  published by the Free Software Foundation; either version 2 of
;  the License, or (at your option) any later version.
;
;  This program is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this program; see the file COPYING.
;  If not, write to the Free Software Foundation, Inc.,
;  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
;
;  Markus F.X.J. Oberhumer              Laszlo Molnar
;  <markus@oberhumer.com>               <ezerotven+github@gmail.com>
;
;  John F. Reiser
;  <jreiser@users.sourceforge.net>
;
*/

#include "arch/amd64/regs.h"
NBPW= 8

.macro          section name
                .section \name,"ax"
                .code64
.endm

/* AMD64 branch prediction is much worse if there are more than 3 branches
   per 16-byte block.  The jnextb would suffer unless inlined.  getnextb is OK
   using closed subroutine to save space, and should be OK on cycles because
   CALL+RET should be predicted.  getnextb could partially expand, using closed
   subroutine only for refill.
*/
/* jump on next bit {0,1} with prediction {y==>likely, n==>unlikely} */
/* Prediction omitted for now. */
/* On refill: prefetch next byte, for latency reduction on literals and offsets. */
#define jnextb0np jnextb0yp
#define jnextb0yp GETBITp; jnc
#define jnextb1np jnextb1yp
#define jnextb1yp GETBITp; jc
#define GETBITp \
        addl bits,bits; jnz 0f; \
        movl (%rsi),bits; sub $-4,%rsi; \
        adcl bits,bits; movb (%rsi),%dl; \
0:
/* Same, but without prefetch (not useful for length of match.) */
#define jnextb0n jnextb0y
#define jnextb0y GETBIT; jnc
#define jnextb1n jnextb1y
#define jnextb1y GETBIT; jc
#define GETBIT \
        addl bits,bits; jnz 0f; \
        movl (%rsi),bits; sub $-4,%rsi; \
        adcl bits,bits; \
0:

/* rotate next bit into bottom bit of reg */
#define getnextbp(reg) GETBITp; adcl reg,reg
#define getnextb(reg)  getnextbp(reg)

// /*************************************************************************
//   C-callable decompressor  f_expand(src, srclen, dst, &dstlen, method)
// **************************************************************************/

  section EXP_HEAD
        push %rbp; mov %rsp,%rbp; push %rbx  // MATCH_50  C saved registers

// Calling sequence registers
#define src    %arg1
#define srclen %arg2
#define dst    %arg3
#define dstlen %arg4
#define meth   %arg5d
#define methb  %arg5b
#define         arg5d r8d

// If unfilter will be needed, then splice a call to unfilter after de-compress
        movl meth,%eax; movzbl %ah,%eax; test %eax,%eax; jz no_unf
        push %rax  // ftid
        movl meth,%eax; shr $16,%eax; push %rax  // cto8 and extra
        push (dstlen)
        push dst
        call no_unf
        pop %rdi; pop %rsi; pop %rdx; pop %rcx  // (*f_unf)(xo->buf, out_len, h.b_cto8, h.b_ftid);
#include "arch/amd64/bxx.S"  // actual unfilter code
        pop %rbx; pop %rbp
        ret
no_unf:

/* Working registers */
#define off  %eax  /* XXX: 2GB */
#define len  %ecx  /* XXX: 2GB */
#define lenq %rcx
#define bits %ebx
#define displ %ebp
#define dispq %rbp

        push dstlen  // MATCH_51
        add src,srclen; push srclen  // MATCH_52  src_EOF
        mov src,%rsi  // hardware src for movsb, lodsb
        push dst  // MATCH_53  dst_orig
        mov dst,%rdi  // hardware dst for movsb
        xor bits,bits  // empty; force refill
        xor len,len  // create loop invariant
        orq $(~0),dispq  // -1: initial displacement
        jmp setup

refill:
        movl (%rsi),bits; sub $-4,%rsi  // next 32 bits; set Carry
        adcl bits,bits  // LSB= 1 (CarryIn); CarryOut= next bit
        movb (%rsi),%dl  // speculate: literal, or bottom 8 bits of offset
        rep; ret
getbit:
        addl bits,bits; jz refill  // Carry= next bit
        rep; ret

copy:  // In: len, %rdi, dispq;  Out: 0==len, %rdi, dispq;  trashes %rax, %rdx
        lea (%rdi,dispq),%rax; cmpl $5,len  // <=3 is forced
        movb (%rax),%dl; jbe copy1  // <=5 for better branch predict
        cmpl $-4,displ;   ja  copy1  // 4-byte chunks would overlap
        subl $4,len  // adjust for termination cases
copy4:
        movl (%rax),%edx; add $4,      %rax; subl $4,len
        movl %edx,(%rdi); lea  4(%rdi),%rdi; jnc copy4
        addl $4,len; movb (%rax),%dl; jz copy0
copy1:
        inc %rax; movb %dl,(%rdi); dec len
            movb (%rax),%dl
                lea 1(%rdi),%rdi;  jnz copy1
copy0:
        rep; ret

setup:
        cld

  section EXP_TAIL
eof:
        pop %rax  // MATCH_53 dst_orig
        sub %rax,%rdi  // dst -= original dst
        pop %rax  // MATCH_52 src_EOF
        pop %rcx  // MATCH_51 &dstlen
        movl %edi,(%rcx)  // actual length used at dst  XXX: 4GB
        sub %rsi,%rax  // src -= eof;  // return 0: good; else: bad

        pop %rbx; pop %rbp
        ret

#define M_NRV2B_LE32    2
#define M_NRV2D_LE32    5
#define M_NRV2E_LE32    8
#define M_CL1B_LE32     11
#define M_LZMA          14

  section NRV2E
#include "arch/amd64/nrv2e_d.S"

  section NRV2D
#include "arch/amd64/nrv2d_d.S"

  section NRV2B
#include "arch/amd64/nrv2b_d.S"

#undef off
#undef len
#undef bits
#undef displ
#undef dispq

#undef src
#undef dst
#undef meth

/* lzma has its own 'section's */
#include "arch/amd64/lzma_d.S"

