/* ppc_d_nrv2e.S -- PowerPC decompressor for NRV2E

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
   Copyright (C) 2000-2008 John F. Reiser
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
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>

   John F. Reiser
   <jreiser@users.sourceforge.net>
*/

#define M_NRV2E_LE32    8
        dcbtst 0,dst  // prime dcache for store
        mflr t3  // return address

        cmpli cr0,meth,M_NRV2E_LE32
        bne   cr0,not_nrv2e

        stw dst,0(ldst)  // original dst
        add lsrc,lsrc,src  // input eof

        lis hibit,0x8000  // 0x80000000 for detecting next bit
        lis  bits,0x8000 // prepare for first load
        addi src,src,-1  // prepare for 'lbzu'
        addi dst,dst,-1  // prepare for 'stbu'
        li disp,-1  // initial displacement
        b bot_n2e

#undef  jnextb0y
#undef  jnextb0n
#undef  jnextb1y
#undef  jnextb1n
/* jump on next bit, with branch prediction: y==>likely; n==>unlikely
   cr0 is set by the cmpl ["compare logical"==>unsigned]:
    lt  next bit is 0
    gt  next bit is 1
    eq  must load next 32 bits from memory

   beql-: branch and link [call subroutine] if cr0 is eq, unlikely
*/
#define jnextb0y  cmpl 0,bits,hibit; add bits,bits,bits; beql- get32; blt+
#define jnextb0n  cmpl 0,bits,hibit; add bits,bits,bits; beql- get32; blt-
#define jnextb1y  cmpl 0,bits,hibit; add bits,bits,bits; beql- get32; bgt+
#define jnextb1n  cmpl 0,bits,hibit; add bits,bits,bits; beql- get32; bgt-

#undef  getnextb
/* rotate next bit into bottom bit of reg */
#define getnextb(reg)              addc. bits,bits,bits; beql- get32; adde reg,reg,reg

get32:
                // fetch 4 bytes unaligned and LITTLE ENDIAN
#if 0  /*{ clean; but 4 instr larger, and 3 cycles longer */
        lbz bits,1(src)  // lo8
        lbz   t0,2(src); rlwimi bits,t0, 8,16,23
        lbz   t0,3(src); rlwimi bits,t0,16, 8,15
        lbzu  t0,4(src); rlwimi bits,t0,24, 0, 7
#else  /*}{ pray for no unalignment trap or slowdown */
        li bits,1  // compensate for 'lbzu'
        lwbrx bits,bits,src  // bits= fetch_le32(bits+src)
        addi src,src,4
#endif  /*}*/

        cmpl    0,bits,hibit  // cr0  for   jnextb
        addc bits,bits,bits  // CArry for getnextb
        ori  bits,bits,1  // the flag bit
        ret

lit_n2e:
#define tmp len
        lbzu tmp,1(src)  // tmp= *++src;
        stbu tmp,1(dst)  // *++dst= tmp;
#undef tmp
top_n2e:
        jnextb1y lit_n2e
        li off,1
        b getoff_n2e

off_n2e:
        addi off,off,-1
        getnextb(off)
getoff_n2e:
        getnextb(off)
        jnextb0n off_n2e

        li len,0
        addic. off,off,-3  // CArry set [and ignored], but no 'addi.'
        rlwinm off,off,8,0,31-8  // off<<=8;
        blt- offprev_n2e
        lbzu t0,1(src)
        nor. disp,off,t0  // disp = -(1+ (off|t0));
        srawi disp,disp,1  // shift off low bit (sets CArry; ignored)
        beq- eof_nrv
        andi. t0,t0,1  // complement of low bit of unshifted disp
        beq+ lenlast_n2e // low bit was 1
        b lenmore_n2e    // low bit was 0

offprev_n2e:
        jnextb1y lenlast_n2e
lenmore_n2e:
        li len,1  // 1: "the msb"
        jnextb1y lenlast_n2e
len_n2e:
        getnextb(len)
        jnextb0n len_n2e
        addi len,len,6-2-2
        b gotlen_n2e

lenlast_n2e:
        getnextb(len)  // 0,1,2,3
gotlen_n2e:
#define tmp off
        subfic tmp,disp,(~0)+(-0x500)  // want CArry only
#undef tmp
        addi  len,len,2
        addze len,len  // len += (disp < -0x500);

#define back off
        add back,disp,dst  // point back to match in dst
        mtctr len
short_n2e:
#define tmp len
        lbzu tmp,1(back)
        stbu tmp,1(dst)
#undef tmp
        bdnz+ short_n2e
bot_n2e:
/* This "prefetch for store" is simple, small, and effective.  Matches
   usually occur more frequently than once per 128 bytes, but G4 line size
   is only 32 bytes anyway.  Assume that an 'unnecessary' dcbtst costs only
   about as much as a hit.  The counter register is free at top_n2e, so we could
   pace the dcbtst optimally; but that takes 7 or 8 instructions of space.
*/
        li back,2*SZ_DLINE
        dcbtst back,dst  // 2 lines ahead [-1 for stbu]
        dcbt   back,src  // jump start auto prefetch at page boundary
/* Auto prefetch for Read quits at page boundary; needs 2 misses to restart. */
        b top_n2e
#undef back

not_nrv2e:

// vi:ts=8:et
