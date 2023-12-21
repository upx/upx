/*
;  mips.r3000-linux.expand.S -- decompressors for mips.r3000-linux
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

#define NBPW 4
#include "arch/mips/r3000/bits.ash"
#include "arch/mips/r3000/macros.ash"

.macro call subr; bal \subr; .endm

  section EXP_HEAD

sz_unc= 0
sz_cpr= 4
b_method= 8
b_ftid=   9
b_cto8=  10
b_extra= 11
sz_binfo= 12

f_expand: .globl f_expand // f_expand(b_info *, dst, &dstlen)
// Supervisor for de-compression, un-filter, and sync_cache
// Input to supervisor:
#define fx_src a0
#define fx_dst a1
#define fx_dstlen a2
    PUSH4 fx_src,fx_dst,fx_dstlen,ra  // params to unfilter and sync_cache

// Input to de-compressor:
#define xsrc    a0
#define xsrclen a1
#define xdst    a2
#define xdstlen a3
#define methb   a4
    lbu methb,b_method(fx_src)
    move xdstlen,fx_dstlen  // arg4
    move xdst,fx_dst  // arg3
    lw xsrclen,sz_cpr(xsrc)  // arg2
    addiu xsrc,fx_src,sz_binfo // arg1
    call decompress
    move a3,v0  // save retval

    POP4 a2,a0,a1,ra   // fx_src,fx_dst,fx_dstlen,ra
    lw a1,(a1)  // actual length used by decompressor
    PUSH3 a0,a1,a3   // params for sync_cache
    lbu a3,b_ftid(a2)
    lbu a2,b_cto8(a2)
    beqz a3,no_unf

    break  # FIXME #include "arch/mips/r3000/bxx.S"  // unfilter code; args in registers, fall-through return
no_unf:

    POP2 a0,a1   // dst, len
    add a1,a1,a0  // lo, hi
    break  // FIXME sync_cache  // in macros.S
    POP1 a0   // retval from decompress
    jr ra
      nop

#undef fx_src
#undef fx_dst
#undef fx_dstlen
#undef xsrc
#undef xsrclen
#undef xdst
#undef xdstlen
#undef methb

decompress:  // (src *, cpr_len, dst *, &dstlen);
//  sections NRV2B, etc, inserted here by addLoader() from ::buildLinuxLoader()

  section EXP_TAIL
  .type eof,function
  .globl eof
eof:  // end of a compressed extent; need sync_cache
        lw a3,0*NBPW(sp)  // &input_eof
        sub a3,a0,a3  // src -= eof;  // return 0: good; else: bad
        lw a0,1*NBPW(sp)  // original dst
        lw a1,2*NBPW(sp); sub a1,a2,a1  // dst -= original dst
        lw a3,3*NBPW(sp); sw dst,(a3)  // actual length used at dst  XXX: 4GB
        jr ra
          addiu sp,4*NBPW
#undef src
#undef dst

#define M_NRV2B_LE32    2
#define M_NRV2B_8    3
#define M_NRV2D_LE32    5
#define M_NRV2D_8    6
#define M_NRV2E_LE32    8
#define M_NRV2E_8    9
#define M_CL1B_LE32     11
#define M_LZMA          14

// FIXME: will need extra parameter for multi-method decompression
#define NO_METHOD_CHECK 1

  section NRV2E
#include "arch/mips/r3000/nrv2e_d.ash"

  section NRV2D
#include "arch/mips/r3000/nrv2d_d.ash"

  section NRV2B
#include "arch/mips/r3000/nrv2b_d.ash"

/* lzma has its own 'section's */
#include "arch/mips/r3000/lzma_d.S"

