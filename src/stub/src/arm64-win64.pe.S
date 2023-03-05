/* arm64-win64.pe.S --
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2023 Laszlo Molnar
*  Copyright (C) 2000-2023 John F. Reiser
*  All Rights Reserved.
*
*  UPX and the UCL library are free software; you can redistribute them
*  and/or modify them under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; see the file COPYING.
*  If not, write to the Free Software Foundation, Inc.,
*  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
*  Markus F.X.J. Oberhumer              Laszlo Molnar
*  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
*
*  John F. Reiser
*  <jreiser@users.sourceforge.net>
*/

#include "arch/arm64/v8/macros.S"

// =============
// ============= DECOMPRESSION
// =============

section NRV_HEAD
        // empty ???
#define M_NRV2B_LE32    2
#define M_NRV2D_LE32    5
#define M_NRV2E_LE32    8
#define M_CL1B_LE32     11
#define M_LZMA          14

#define xsrc    x0
#define xsrclen w1
#define xdst    x2
#define xdstlen x3
#define methb   w4

#define NO_METHOD_CHECK 1

off     .req w5
section NRV2B
#include "arch/arm64/v8/nrv2b_d32.S"

section NRV2D
#include "arch/arm64/v8/nrv2d_d32.S"

section NRV2E
#include "arch/arm64/v8/nrv2e_d32.S"


section LZMA_HEAD

#include "arch/arm64/v8/lzma_d.S"

section LZMA_TAIL


// vim:set ts=8 sw=8 et:
