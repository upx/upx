/*
;  lzma_d.S -- 64-bit assembly
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 2006-2006 Markus Franz Xaver Johannes Oberhumer
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
;  Markus F.X.J. Oberhumer
;  <markus@oberhumer.com>
;  http://www.oberhumer.com/opensource/upx/
;


; ------------- DECOMPRESSION -------------

; Input:
;   %rsi - source
;   %rdi - dest
;   cld

; Output:
;   %rax - 0
;   %rcx - 0
*/

#define section .section

//
// init
section LZMA_DEC00

// %rbx = alloca('UPXa');

        mov %rsp,%rbp  // save stack

        lea UPXa(%rsp),%rbx
        xor %rax, %rax
1:
        push %rax
        cmp %rbx,%rsp
        jne 1b


        addq $2,%rsi  // skip 2 bytes for properties

        pushq %rbx; addq $4,%rbx  // &outSizeProcessed
        pushq $ UPXb  // outSize  XXX: 2GB
        pushq %rdi  // out
        pushq %rbx; addq $4,%rbx  // &inSizeProcessed
        pushq $ UPXc  // inSize  XXX: 2GB
        pushq %rsi  // in
        pushq %rbx  // &CLzmaDecoderState
        pushq %rax  // fake return address for call

        // hardwired LzmaDecodeProperties()
        movl $ UPXd,(%rbx)  // lc, lp, pb, dummy


section LZMA_ELF00

#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE   768

#ifndef O_OUTS  // ELF defines them, others do not care
#define O_OUTS 0
#define O_INS  0
#endif

        movq %rsp,%rbp  // save stack
        movl O_INS(%rbp),%edx  // inSize  XXX: 4GB

        lodsb; subl $1,%edx  // first byte, replaces LzmaDecodeProperties()
        movb %al,%cl  // cl= ((lit_context_bits + lit_pos_bits)<<3) | pos_bits
        andb $6,%al  // al= pos_bits
        shrb $3,%cl  // cl= lit_context_bits + lit_pos_bits

        movq $-LZMA_LIT_SIZE,%rbx
        shlq %cl,%rbx
// inSizeProcessed, outSizeProcessed, *_bits, CLzmaDecoderState
        leaq -(2*4 +4) - 2*LZMA_BASE_SIZE(%rsp,%rbx,2), %rbx
        andq $~0<<6,%rbx  // 64-byte align
.elf_clearstack1:
        pushq $0
        cmpq %rbx,%rsp
        jne .elf_clearstack1

        pushq %rbx; addq $8,%rbx  // &outSizeProcessed
        movl O_OUTS(%rbp),%ecx  // &outSize  XXX: 2GB
        pushq (%rcx)  // outSize
        pushq %rdi  // out
        pushq %rbx; addq $8,%rbx  // &inSizeProcessed

        movb %al,2(%rbx)  // store pos_bits
        lodsb; subl $1,%edx  // second byte, replaces LzmaDecodeProperties()
        movb %al,%cl  // cl= (lit_pos_bits<<4) | lit_context_bits
        andb $0xf,%al; movb %al, (%rbx)  // store lit_context_bits
        shrb  $4, %cl; movb %cl,1(%rbx)  // store lit_pos_bits

        pushq %rdx  // inSize -2
        pushq %rsi  // in
        pushq %rbx  // &CLzmaDecoderState
        pushq %rax  // return address slot (dummy CALL)

section LZMA_DEC10
#include "lzma_d_cs.S"

section LZMA_DEC20
#include "lzma_d_cf.S"


//
// cleanup
section LZMA_DEC30

        addq -1*8(%rbx),%rsi  // inSizeProcessed
        addq -2*8(%rbx),%rdi  // outSizeProcessed
        xorq %rax,%rax

        leaq -256(%rsp),%rcx
        movq %rbp,%rsp  // restore stack
1:
        pushq %rax
        cmpq %rcx,%rsp
        jne 1b

        movq %rbp,%rsp  // restore stack
        xorq %rcx,%rcx


// vi:ts=8:et

