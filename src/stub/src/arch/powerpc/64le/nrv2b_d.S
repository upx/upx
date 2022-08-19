/* ppc_d_nrv2b.S -- ppc64le decompressor for NRV2B

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2022 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2022 Laszlo Molnar
   Copyright (C) 2000-2022 John F. Reiser
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
   <markus@oberhumer.com>               <ezerotven+github@gmail.com>

   John F. Reiser
   <jreiser@users.sourceforge.net>
*/

#define M_NRV2B_LE32    2
        dcbtst 0,dst  // prime dcache for store
        mflr t3  // return address

        cmplwi cr0,meth,M_NRV2B_LE32
        bne   cr0,not_nrv2b

        std dst,0(ldst)  // original dst
        add lsrc,lsrc,src  // input eof

        lis hibit,0x8000  // 0x80000000 for detecting next bit
        sldi hibit,hibit,32        // upper bits
        mr bits,hibit  // prepare for first load
        addi src,src,-1  // prepare for 'lbzu'
        addi dst,dst,-1  // prepare for 'stbu'
        li disp,-1  // initial displacement
        b bot_n2b

#undef  jnextb0y
#undef  jnextb0n
#undef  jnextb1y
#undef  jnextb1n
/* jump on next bit, with branch prediction: y==>likely; n==>unlikely
   cr0 is set by the cmpl ["compare logical"==>unsigned]:
    lt  next bit is 0
    gt  next bit is 1
    eq  must load next 32 bits from memory
*/
#define jnextb0y  call get1; blt+ cr0,
#define jnextb0n  call get1; blt- cr0,
#define jnextb1y  call get1; bgt+ cr0,
#define jnextb1n  call get1; bgt- cr0,

#undef  getnextb
/* rotate next bit into bottom bit of reg; set cr0 based on entire result reg */
#define getnextb(reg) call get1; adde. reg,reg,reg

get2:
        adde  bits,bits,bits  // shift in CArry from 'addc' below
        sldi bits,bits,32-1  // left justify 33-bit result
get1:
        cmpld bits,hibit  // cr0  for   jnextb
        addc  bits,bits,bits  // CArry for getnextb
        bnelr+ cr0  // return if reload not needed; likely 31/32

                // fetch 4 bytes unaligned and LITTLE ENDIAN
#if 0  /*{ clean; but 4 instr larger, and 3 cycles longer */
        lbz bits,1(src)  // lo8
        lbz   t1,2(src); rlwimi bits,t1, 8,16,23
        lbz   t1,3(src); rlwimi bits,t1,16, 8,15
        lbzu  t1,4(src); rlwimi bits,t1,24, 0, 7
#else  /*}{ pray for no unalignment trap or slowdown */
        li bits,1  // compensate for 'lbzu'
#if BIG_ENDIAN  //{
        lwbrx bits,bits,src  // bits= fetch_le32(bits+src)
#else  //}{
        lwzx  bits,bits,src
#endif  //}
        addi src,src,4
#endif  /*}*/
        b get2

lit_n2b:
#define tmp len
        lbzu tmp,1(src)  // tmp= *++src;
        stbu tmp,1(dst)  // *++dst= tmp;
#undef tmp
top_n2b:
        jnextb1y lit_n2b
        li off,1  // "the msb"
offmore_n2b:
        getnextb(off)
        jnextb0n offmore_n2b

        addic. off,off,-3  // CArry set [and ignored], but no 'addi.'
        li len,0
        blt- offprev_n2b
        lbzu t1,1(src)
        slwi off,off,8  // off<<=8;
        nor disp,off,t1  // disp = -(1+ (off|t1));
        slwi. t1,disp,0  // t1= 0xffffffff & disp;
        beq- eof_nrv

offprev_n2b:  // In: 0==len
        getnextb(len); getnextb(len)  // two bits; cr0 set on result
        li off,1; bne- gotlen_n2b  // raw 1,2,3 ==> 2,3,4
        li off,3  // raw 2.. ==> 5..
        li len,1  // "the msb"
lenmore_n2b:
        getnextb(len)
        jnextb0n lenmore_n2b
gotlen_n2b:
        subfic t1,disp,(~0)+(-0xd00)  // want CArry only
        adde len,len,off  // len += off + (disp < -0xd00);

copy:
#define back off
        add back,disp,dst  // point back to match in dst
        mtctr len
short_n2b:
#define tmp len
        lbzu tmp,1(back)
        stbu tmp,1(dst)
#undef tmp
        bdnz+ short_n2b

/* This "prefetch for store" is simple, small, and effective.  Matches
   usually occur more frequently than once per 128 bytes, but G4 line size
   is only 32 bytes anyway.  Assume that an 'unnecessary' dcbtst costs only
   about as much as a hit.  The counter register is free at top_n2b, so we could
   pace the dcbtst optimally; but that takes 7 or 8 instructions of space.
*/
bot_n2b:
        li back,2*SZ_DLINE
        dcbtst back,dst  // 2 lines ahead [-1 for stbu]
        dcbt   back,src  // jump start auto prefetch at page boundary
/* Auto prefetch for Read quits at page boundary; needs 2 misses to restart. */
#undef back
        b top_n2b

not_nrv2b:

// vi:ts=8:et
