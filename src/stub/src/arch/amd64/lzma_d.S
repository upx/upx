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
*/

#define section .section

section LZMA_ELF00
//decompress:  // (uchar const *src, size_t lsrc, uchar *dst, u32 &ldst, uint method)
/* Arguments according to calling convention */
#define src  %arg1  /* %rdi */
#define lsrc %arg2  /* %rsi */
#define dst  %arg3  /* %rdx */
#define ldst %arg4  /* %rcx */ /* Out: actually a reference: &len_dst */
#define meth %arg5l /* %r8  */
#define methb %arg5b

#define M_LZMA          14
        cmpb $ M_LZMA,methb; jne not_lzma
                // C calling convention
        pushq %rbp; movq %rsp,%rbp
        pushq %rbx

//LzmaDecode(  // from lzmaSDK/C/7zip/Compress/LZMA_C/LzmaDecode.h
//      %arg1= &CLzmaDecoderState,
//      %arg2= in,  %arg3l= inSize, %arg4= &inSizeProcessed,
//      %arg5= out, %arg6l= outSize, 8+%esp/ &outSizeProcessed
//)

#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE   768
#define szSizeT 8

        movq dst,%arg5  // outp
        movq lsrc,%arg3  // inSize
        movq  src,%arg2  // in
        movq ldst,%r10

        lodsb; decl %arg3l  // first byte, replaces LzmaDecodeProperties()
        movb %al,%cl  // cl= ((lit_context_bits + lit_pos_bits)<<3) | pos_bits
        andb $6,%al  // al= pos_bits
        shrb $3,%cl  // cl= lit_context_bits + lit_pos_bits

        movq $-LZMA_LIT_SIZE,%rbx
        shlq %cl,%rbx
// inSizeProcessed, outSizeProcessed, *_bits, CLzmaDecoderState
        leaq -(2*szSizeT +4) - 2*LZMA_BASE_SIZE(%rsp,%rbx,2), %rbx
        andq $~0<<6,%rbx  // 64-byte align
1:
        pushq $0
        cmpq %rbx,%rsp
        jne 1b

        pushq %rbx; addq $ szSizeT,%rbx  // &outSizeProcessed
        movl (%r10),%arg6l  // &outSize  XXX: 4GB
        movq %rbx,%arg4; addq $ szSizeT,%rbx  // &inSizeProcessed

        movb %al,2(%rbx)  // store pos_bits
        lodsb; decl %arg3l  // second byte, replaces LzmaDecodeProperties()
        movb %al,%cl  // cl= (lit_pos_bits<<4) | lit_context_bits
        andb $0xf,%al; movb %al, (%rbx)  // store lit_context_bits
        shrb  $4, %cl; movb %cl,1(%rbx)  // store lit_pos_bits

        movq %rbx,%arg1  // &CLzmaDecoderState
        pushq %rax  // return address slot (dummy CALL)

section LZMA_DEC10
#include "lzma_d_cs.S"

section LZMA_DEC20
#include "lzma_d_cf.S"


//
// cleanup
section LZMA_DEC30

        movl -1*szSizeT(%rbx),%eax; addq %rax,%rsi  // inSizeProcessed
        movl -2*szSizeT(%rbx),%eax; addq %rax,%rdi  // outSizeProcessed
        xorq %rax,%rax

        leaq -256(%rsp),%rcx
        movq %rbp,%rsp  // restore stack
1:
        pushq %rax
        cmpq %rcx,%rsp
        jne 1b

        movq -8(%rbp),%rbx
        xorq %rcx,%rcx
        leave
not_lzma:

// vi:ts=8:et

