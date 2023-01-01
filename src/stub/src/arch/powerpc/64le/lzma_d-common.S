/*
;  lzma_d.S -- 64-bit PowerPC assembly
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 2006-2023 Markus Franz Xaver Johannes Oberhumer
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
// http://refspecs.linuxfoundation.org/ELF/ppc64/PPC-elf64abi.html#STACK
SZ_LINK= 6*8  // (sp,cr,lr, xx,yy,zz) save area per calling convention
SZ_PSAV= 8*8  // for spilling a0-a7 if necessary
retaddr = 2*8  // lr === pc

#define section .section

  section LZMA_ELF00
//decompress:  // (uchar const *src, size_t lsrc, uchar *dst, u64 &ldst, uint method)
// Our 4th parameter points to 64 bits; LzmaDecode deals in 32 bits.
// The controlling file is src/lzma-sdk/C/7zip/Compress/LZMA_C/LzmaTypes.h
// (LZMA SDK 4.40 2006-05-01) where the default is
//      typedef UInt32 SizeT;
// unless _LZMA_SYSTEM_SIZE_T is defined.  Only the 32 bit SizeT has been tested!
// [Check: cd src/stub/src/arch/powerpc/64le; make -f Makefile.extra ]

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
        stdu sp,-(SZ_LINK+SZ_PSAV)(sp)  // FR_10  defend against callers who omit the linkage area
        mflr r0

//Prepare for call to:
//LzmaDecode(  // from lzmaSDK/C/7zip/Compress/LZMA_C/LzmaDecode.h
//      a0= &CLzmaDecoderState,
//      a1= inp,  a2= inSize,  a3= &inSizeProcessed,
//      a4= outp, a5= outSize, a6= &outSizeProcessed
//)
        la    a6,BIG_ENDIAN*4(ldst)  // &outSizeProcessed
        lwz   a5,BIG_ENDIAN*4(ldst)  // outSize  XXX 32 BITS
        mr    a4,dst  // outp

        addi  a2,lsrc,-2  // inSize
        la    a1,2(src)  // inp

        lbz t2,0(src)  // first byte, replaces LzmaDecodeProperties()
        std r0,retaddr(sp)  // save return address in caller's frame
        rldicl t1,t2,64-3,3  // t1= (t2>>3)==(lit_context-bits + lit_pos_bits)
        clrldi t2,t2,64-3    // t2= (7& t2)==pos_bits

#define LZMA_BASE_SIZE 1846
#define LZMA_LIT_SIZE   768
#define szSizeT 8

        li   a0,-2*LZMA_LIT_SIZE
        sld  a0,a0,t1  // -2*LZMA_LIT_SIZE << (lit_context_bits + lit_pos_bits)
        addi a0,a0,-6*8 - 2*LZMA_BASE_SIZE
// alloca{sp,ra,orig_dst,&outSizeProcessed,slot(inSizeProcessed),*_bits, CLzmaDecoderState}
        mr a3,sp
        add sp,sp,a0  // FR_11  allocate
        clrrdi sp,sp,6  // (1<<6) byte align (round down)

        li r0,0
        mr a0,a3  // old sp
        stw r0,0(a6)  // outSizeProcessed= 0;  XXX 32 BITS
1:
        stdu r0,-8(a0)  // clear CLzmaDecoderState on stack
        cmpld cr0,sp,a0  // compare logical ==> compare unsigned
        blt cr0,1b
        std a3,0(sp)  // FR_11  chain

        lbz r0,-1(a1)  // second byte, replaces LzmaDecodeProperties()
        la a3,4*8 + BIG_ENDIAN*4(sp)  // &inSizeProcessed  XXX 32 BITS
        la a0,5*8               (sp)  // &CLzmaDecoderState
        stb t2,2(a0)  // pos_bits
        std a4,2*8(sp)  // outp
        std a6,3*8(sp)  // &outSizeProcessed
        rldicl t1,r0,64-4,4  // t1= (r0>>4)==lit_pos_bits
        clrldi r0,r0,64-4    // r0= (0xf& r0)==lit_context_bits
        stb t1,1(a0)  // lit_pos_bits
        stb r0,0(a0)  // lit_context_bits
        stdu sp,-(SZ_LINK+SZ_PSAV)(sp)  // FR_12  defend against callers who omit the linkage area

  section LZMA_DEC30  // Out: a0= retval; a2= &hi stored; a4= &lo stored; lr= retaddr
        la   sp,SZ_LINK+SZ_PSAV(sp) // FR_12  un-defend
        ld   a2,3*8(sp)  // &outSizeProcessed
        ld   a4,2*8(sp)  // &lowest byte stored
        lwz  a2,0(a2)  // outSizeProcessed  XXX 32 BITS
        ld   sp,0(sp)  // FR_11  de-alloc CLzmaDecoderState, incl. array of probabilities
        add  a2,a2,a4  // &next byte to store
        ld r0,retaddr(sp)
         addi a2,a2,-1  // &highest byte stored
        ld   sp,0(sp)   // FR_10  un-allocate and un-chain
        mtlr r0

not_lzma:

// vi:ts=8:et

