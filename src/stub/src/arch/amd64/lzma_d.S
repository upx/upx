/*
;  lzma_d.S -- 64-bit assembly
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 2006-2008 Markus Franz Xaver Johannes Oberhumer
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

// ELFMAINX has already done this for us:
//      pushq %rbp; push %rbx  // C callable
//      pushq ldst
//      pushq dst
//      addq src,lsrc; push lsrc  // &input_eof

#define M_LZMA          14
        cmpb $ M_LZMA,methb; jne not_lzma

        pushq %rbp; movq %rsp,%rbp  // we use alloca

//LzmaDecode(  // from lzmaSDK/C/7zip/Compress/LZMA_C/LzmaDecode.h
//      %arg1= &CLzmaDecoderState,
//      %arg2= in,  %arg3l= inSize, %arg4= &inSizeProcessed,
//      %arg5= out, %arg6l= outSize, arg7@ 8+%esp/ &outSizeProcessed
//)
        movl (ldst),%arg6l  // &outSize  XXX: 4GB
        movq dst,%arg5  // outp
        movq lsrc,%arg3  // inSize
        leaq 2(src),%arg2; pushq %arg2  // in; save @-8(%rbp) for size calc at eof

        movb (src),%al; decl %arg3l  // first byte, replaces LzmaDecodeProperties()
        movb %al,%cl  // cl= ((lit_context_bits + lit_pos_bits)<<3) | pos_bits
        andb $7,%al  // al= pos_bits
        shrb $3,%cl  // cl= lit_context_bits + lit_pos_bits

#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE   768
#define szSizeT 4

        movq $-LZMA_LIT_SIZE,%rbx
        shlq %cl,%rbx; movb %al,%cl  // %cl= pos_bits
// alloca{inSizeProcessed, outSizeProcessed, *_bits, CLzmaDecoderState}
        leaq -(2*szSizeT +4) - 2*LZMA_BASE_SIZE(%rsp,%rbx,2), %rbx
        andq $~0<<6,%rbx  // 64-byte align
1:
        pushq $0  // clera CLzmaDecoderState on stack
        cmpq %rbx,%rsp
        jne 1b


        pushq %rbx  // &outSizeProcessed [arg7]
        leaq 2*szSizeT(%rbx),%arg1  // &CLzmaDecoderState
        movb -1(%arg2),%cl; decl %arg3l  // second byte, replaces LzmaDecodeProperties()
                       movb %al,2(%arg1)  // store pos_bits
        movb  %cl,%al  // al= (lit_pos_bits<<4) | lit_context_bits
        shrb   $4,%cl; movb %cl,1(%arg1)  // store lit_pos_bits
        andb $0xf,%al; movb %al, (%arg1)  // store lit_context_bits
        leaq -szSizeT(%arg1),%arg4  // &inSizeProcessed

        pushq %rax  // return address slot (dummy CALL)

section LZMA_DEC10
#include "lzma_d_cs.S"

section LZMA_DEC20
#include "lzma_d_cf.S"

section LZMA_DEC30
        movq -1*8(%rbp),%rsi  // src [after header]
        movq  2*8(%rbp),%rdi  // dst
        movl szSizeT(%rbx),%ecx; addq %rcx,%rsi  //  inSizeProcessed
        movl        (%rbx),%edx; addq %rdx,%rdi  // outSizeProcessed
        leave  // movl %ebp,%rsp; popq %rbp
not_lzma:

// vi:ts=8:et

