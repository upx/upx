/* filteri.cpp -- filter implementation (low-level)

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2001 Laszlo Molnar
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

   Markus F.X.J. Oberhumer                   Laszlo Molnar
   markus.oberhumer@jk.uni-linz.ac.at        ml1050@cdata.tvnet.hu
 */


#include "conf.h"
#include "filter.h"


#define set_dummy(p, v)     ((void)0)


/*************************************************************************
// calltrick / swaptrick
**************************************************************************/

#include "filter/ct.h"
#include "filter/sw.h"
#include "filter/ctsw.h"


/*************************************************************************
// cto "clever" calltrick
**************************************************************************/

#define COND(b,x)               (b[x] == 0xe8)
#define F                       f_cto32_e8_bswap_le
#define U                       u_cto32_e8_bswap_le
#include "filter/cto.h"
#define F                       s_cto32_e8_bswap_le
#include "filter/cto.h"
#undef COND

#define COND(b,x)               (b[x] == 0xe9)
#define F                       f_cto32_e9_bswap_le
#define U                       u_cto32_e9_bswap_le
#include "filter/cto.h"
#define F                       s_cto32_e9_bswap_le
#include "filter/cto.h"
#undef COND

#define COND(b,x)               (b[x] == 0xe8 || b[x] == 0xe9)
#define F                       f_cto32_e8e9_bswap_le
#define U                       u_cto32_e8e9_bswap_le
#include "filter/cto.h"
#define F                       s_cto32_e8e9_bswap_le
#include "filter/cto.h"
#undef COND


/*************************************************************************
//
**************************************************************************/

#define COND(b,x,lastcall) \
        (b[x] == 0xe8 || b[x] == 0xe9 \
         || (lastcall!=(x) && 0xf==b[(x)-1] && 0x80<=b[x] && b[x]<=0x8f) )
#define F                       f_ctjo32_e8e9_bswap_le
#define U                       u_ctjo32_e8e9_bswap_le
#include "filter/ctjo.h"
#define F                       s_ctjo32_e8e9_bswap_le
#include "filter/ctjo.h"
#undef COND


/*************************************************************************
// database for use in class Filter
**************************************************************************/

const FilterImp::FilterEntry FilterImp::filters[] = {
    // no filter
    { 0x00, 0,          0, NULL, NULL, NULL },
    // 16-bit calltrick
    { 0x01, 4,          0, f_ct16_e8, u_ct16_e8, s_ct16_e8 },
    { 0x02, 4,          0, f_ct16_e9, u_ct16_e9, s_ct16_e9 },
    { 0x03, 4,          0, f_ct16_e8e9, u_ct16_e8e9, s_ct16_e8e9 },
    { 0x04, 4,          0, f_ct16_e8_bswap_le, u_ct16_e8_bswap_le, s_ct16_e8_bswap_le },
    { 0x05, 4,          0, f_ct16_e9_bswap_le, u_ct16_e9_bswap_le, s_ct16_e9_bswap_le },
    { 0x06, 4,          0, f_ct16_e8e9_bswap_le, u_ct16_e8e9_bswap_le, s_ct16_e8e9_bswap_le },
    { 0x07, 4,          0, f_ct16_e8_bswap_be, u_ct16_e8_bswap_be, s_ct16_e8_bswap_be },
    { 0x08, 4,          0, f_ct16_e9_bswap_be, u_ct16_e9_bswap_be, s_ct16_e9_bswap_be },
    { 0x09, 4,          0, f_ct16_e8e9_bswap_be, u_ct16_e8e9_bswap_be, s_ct16_e8e9_bswap_be },
    // 16-bit swaptrick
    { 0x0a, 4,          0, f_sw16_e8, u_sw16_e8, s_sw16_e8 },
    { 0x0b, 4,          0, f_sw16_e9, u_sw16_e9, s_sw16_e9 },
    { 0x0c, 4,          0, f_sw16_e8e9, u_sw16_e8e9, s_sw16_e8e9 },
    // 16-bit call-/swaptrick
    { 0x0d, 4,          0, f_ctsw16_e8_e9, u_ctsw16_e8_e9, s_ctsw16_e8_e9 },
    { 0x0e, 4,          0, f_ctsw16_e9_e8, u_ctsw16_e9_e8, s_ctsw16_e9_e8 },
    // 32-bit calltrick
    { 0x11, 6,          0, f_ct32_e8, u_ct32_e8, s_ct32_e8 },
    { 0x12, 6,          0, f_ct32_e9, u_ct32_e9, s_ct32_e9 },
    { 0x13, 6,          0, f_ct32_e8e9, u_ct32_e8e9, s_ct32_e8e9 },
    { 0x14, 6,          0, f_ct32_e8_bswap_le, u_ct32_e8_bswap_le, s_ct32_e8_bswap_le },
    { 0x15, 6,          0, f_ct32_e9_bswap_le, u_ct32_e9_bswap_le, s_ct32_e9_bswap_le },
    { 0x16, 6,          0, f_ct32_e8e9_bswap_le, u_ct32_e8e9_bswap_le, s_ct32_e8e9_bswap_le },
    { 0x17, 6,          0, f_ct32_e8_bswap_be, u_ct32_e8_bswap_be, s_ct32_e8_bswap_be },
    { 0x18, 6,          0, f_ct32_e9_bswap_be, u_ct32_e9_bswap_be, s_ct32_e9_bswap_be },
    { 0x19, 6,          0, f_ct32_e8e9_bswap_be, u_ct32_e8e9_bswap_be, s_ct32_e8e9_bswap_be },
    // 32-bit swaptrick
    { 0x1a, 6,          0, f_sw32_e8, u_sw32_e8, s_sw32_e8 },
    { 0x1b, 6,          0, f_sw32_e9, u_sw32_e9, s_sw32_e9 },
    { 0x1c, 6,          0, f_sw32_e8e9, u_sw32_e8e9, s_sw32_e8e9 },
    // 32-bit call-/swaptrick
    { 0x1d, 6,          0, f_ctsw32_e8_e9, u_ctsw32_e8_e9, s_ctsw32_e8_e9 },
    { 0x1e, 6,          0, f_ctsw32_e9_e8, u_ctsw32_e9_e8, s_ctsw32_e9_e8 },
    // 32-bit cto calltrick
    { 0x24, 6, 0x00ffffff, f_cto32_e8_bswap_le, u_cto32_e8_bswap_le, s_cto32_e8_bswap_le },
    { 0x25, 6, 0x00ffffff, f_cto32_e9_bswap_le, u_cto32_e9_bswap_le, s_cto32_e9_bswap_le },
    { 0x26, 6, 0x00ffffff, f_cto32_e8e9_bswap_le, u_cto32_e8e9_bswap_le, s_cto32_e8e9_bswap_le },
    // 32-bit cto calltrick + jmp
    { 0x36, 6, 0x00ffffff, f_ctjo32_e8e9_bswap_le, u_ctjo32_e8e9_bswap_le, s_ctjo32_e8e9_bswap_le },
};

const int FilterImp::n_filters = HIGH(filters);


/*
vi:ts=4:et:nowrap
*/

