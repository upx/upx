/*
;  lzma_d.S -- 32-bit PowerPC assembly
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 2006-2017 Markus Franz Xaver Johannes Oberhumer
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

#include "ppc_regs.h"
retaddr = 2*8  // (sp,cr,pc, xx,yy,zz) save area per calling convention

#define section .section

  section LZMA_ELF00
//decompress:  // (uchar const *src, size_t lsrc, uchar *dst, u32 &ldst, uint method)
/* Arguments according to calling convention */
#define src  a0
#define lsrc a1
#define dst  a2
#define ldst a3  /* Out: actually a reference: &len_dst */
#define meth a4

////  teq r0,r0  // debugging

#define M_LZMA          14
        cmplwi cr0,meth,M_LZMA
        bne   cr0,not_lzma
        stdu sp,-6*8(sp)  // defend against callers who omit the linkage area
        mflr r0

//LzmaDecode(  // from lzmaSDK/C/7zip/Compress/LZMA_C/LzmaDecode.h
//      a0= &CLzmaDecoderState,
//      a1= inp,  a2= inSize,  a3= &inSizeProcessed,
//      a4= outp, a5= outSize, a6= &outSizeProcessed
//)
        mr    a6,ldst  // &outSizeProcessed
        lwz   a5,0(ldst)  // outSize
        mr    a4,dst  // outp

        addi  a2,lsrc,-2  // inSize
        la    a1,2(src)  // inp

        std r0,retaddr(sp)  // save return address in caller's frame
        lbz r0,0(src)  // first byte, replaces LzmaDecodeProperties()
        rldicl t1,r0,64-3,3  // t1= (r0>>3)==(lit_context-bits + lit_pos_bits)
        clrldi t0,r0,64-3    // t0= (7& t0)==pos_bits

#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE   768
#define szSizeT 8

        li   a0,-2*LZMA_LIT_SIZE
        sld  a0,a0,t1  // -2*LZMA_LIT_SIZE << (lit_context_bits + lit_pos_bits)
        addi a0,a0,-4*8 -(szSizeT +8) - 2*LZMA_BASE_SIZE
// alloca{sp,ra,orig_dst,&outSizeProcessed, inSizeProcessed,*_bits, CLzmaDecoderState}
        mr a3,sp
        add sp,sp,a0
        rldicr sp,sp,0,64-6  // (1<<6) byte align

        li r0,0
        mr a0,a3  // old sp
        std r0,0(a6)  // outSizeProcessed= 0;
1:
        stdu r0,-8(a0)  // clear CLZmaDecoderState on stack
        cmpld cr0,sp,a0  // compare logical ==> compare unsigned
        blt cr0,1b
        std a3,0(sp)  // frame chain

        lbz r0,-1(a1)  // second byte, replaces LzmaDecodeProperties()
        la a3,4*8        (sp)  // &inSizeProcessed
        la a0,4*8+szSizeT(sp)  // &CLzmaDecoderState
        std a4,2*8(sp)  // outp
        std a6,3*8(sp)  // &outSizeProcessed
        rldicl t1,r0,64-4,4  // t1= (r0>>4)==lit_pos_bits
        clrldi r0,r0,64-4    // r0= (0xf& r0)==lit_context_bits
        stb t0,2(a0)  // pos_bits
        stb t1,1(a0)  // lit_pos_bits
        stb r0,0(a0)  // lit_context_bits
        stdu sp,-6*8(sp)  // defend against callers who omit the linkage area

  section LZMA_DEC10
#include "lzma_d_cs.S"

  section LZMA_DEC20
        bl   lzma_dec20
        b    eof_dec20
lzma_dec20:
#include "lzma_d_cf.S"
eof_dec20:

  section LZMA_DEC30  // Out: a0= retval; a2= &hi stored; a4= &lo stored; lr= retaddr
        la   sp,6*8(sp) // restore sa
        ld   a2,3*8(sp)  // &outSizeProcessed
        ld   a4,2*8(sp)  // &lowest byte stored
        ld   a2,0(a2)  // outSizeProcessed
        ld   sp,0(sp)  // old sp
        add  a2,a2,a4  // &next byte to store
        ld r0,retaddr(sp)
         addi a2,a2,-1  // &highest byte stored
        ld   sp,0(sp)   // un-defend
        mtlr r0

not_lzma:

// vi:ts=8:et

