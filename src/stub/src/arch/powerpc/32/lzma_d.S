/*
;  lzma_d.S -- 32-bit PowerPC assembly
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 2006-2007 Markus Franz Xaver Johannes Oberhumer
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
retaddr = 2*4  // (sp,cr,pc, xx,yy,zz) save area per calling convention

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
        cmpli cr0,meth,M_LZMA
        bne   cr0,not_lzma
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

        stw r0,retaddr(sp)  // save return address in caller's frame
        lbz r0,0(src)  // first byte, replaces LzmaDecodeProperties()
        rlwinm t1,r0,32-3,3,31  // t1= (r0>>3)==(lit_context-bits + lit_pos_bits)
        rlwinm t0,r0,0,32-3,31  // t0= (7& t0)==pos_bits

#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE   768
#define szSizeT 4

        li   a0,-2*LZMA_LIT_SIZE
        slw  a0,a0,t1  // -2*LZMA_LIT_SIZE << (lit_context_bits + lit_pos_bits)
        addi a0,a0,-2*4 -(szSizeT +4) - 2*LZMA_BASE_SIZE
// alloca{sp,ra, inSizeProcessed,*_bits, CLzmaDecoderState}
        mr a3,sp
        add sp,sp,a0
        rlwinm sp,sp,0,0,32-6  // (1<<6) byte align

        li r0,0
        mr a0,a3  // old sp
        stw r0,0(a6)  // outSizeProcessed= 0;
1:
        stwu r0,-4(a0)  // clear CLZmaDecoderState on stack
        cmpl cr0,sp,a0  // compare logical ==> compare unsigned
        blt cr0,1b
        stw a3,0(sp)  // frame chain

        lbz r0,-1(a1)  // second byte, replaces LzmaDecodeProperties()
        la a3,2*4        (sp)  // &inSizeProcessed
        la a0,2*4+szSizeT(sp)  // &CLzmaDecoderState
        rlwinm t1,r0,32-4,4,31  // t1= (r0>>4)==lit_pos_bits
        rlwinm r0,r0,0,32-4,31  // r0= (0xf& r0)==lit_context_bits
        stb t0,2(a0)  // pos_bits
        stb t1,1(a0)  // lit_pos_bits
        stb r0,0(a0)  // lit_context_bits

  section LZMA_DEC10
#include "lzma_d_cs.S"

  section LZMA_DEC20
#include "lzma_d_cf.S"

  section LZMA_DEC30
        lwz sp,0(sp)  // old sp
        lwz r0,retaddr(sp)
        mtlr r0
        blr

not_lzma:

// vi:ts=8:et

