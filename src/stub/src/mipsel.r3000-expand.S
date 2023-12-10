/*
;  mipsel.r3000-expand.S -- decompressors for arm.v4a
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

NBPW= 4
#ifndef _MR3K_MACROS_ASH
#include "arch/mips/r3000/macros.ash"
#endif

  section EXP_HEAD

sz_unc= 0
sz_cpr= 4
b_method= 8
b_ftid=   9
b_cto8=  10
b_extra= 11
sz_binfo= 12

f_expand: .globl f_expand  # f_expand(b_info *, dst, &dstlen)
    break


decompress:   # (src *, cpr_len, dst *, &dstlen);

  section EXP_TAIL
    break

#define M_NRV2B_LE32    2
#define M_NRV2B_8    3
#define M_NRV2D_LE32    5
#define M_NRV2D_8    6
#define M_NRV2E_LE32    8
#define M_NRV2E_8    9
#define M_CL1B_LE32     11
#define M_LZMA          14

#define NO_METHOD_CHECK 1

  section NRV2E
#include "arch/mips/r3000/nrv2e_d.ash"

  section NRV2D
#include "arch/mips/r3000/nrv2d_d.ash"

  section NRV2B
#include "arch/mips/r3000/nrv2b_d.ash"

/* lzma has its own 'section's */
#include "arch/mips/r3000/lzma_d.S"

