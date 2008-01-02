/* filteri.cpp -- filter implementation (low-level)

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
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
 */


#include "conf.h"
#include "filter.h"

static unsigned
umin(unsigned const a, unsigned const b)
{
    return (a<=b) ? a : b;
}


#define set_dummy(p, v)     ((void)0)
#define get_8(p)            (*(p))
#define set_8(p, v)         (*(p) = (v))


/*************************************************************************
// util
**************************************************************************/

#include "filter/getcto.h"


/*************************************************************************
// simple filters: calltrick / swaptrick / delta / ...
**************************************************************************/

#include "filter/ct.h"
#include "filter/sw.h"
#include "filter/ctsw.h"

#include "filter/sub8.h"
#include "filter/sub16.h"
#include "filter/sub32.h"


/*************************************************************************
// cto calltrick
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
// cto calltrick with jmp
**************************************************************************/

#define COND(b,x,lastcall) (b[x] == 0xe8 || b[x] == 0xe9)
#define F                       f_ctoj32_e8e9_bswap_le
#define U                       u_ctoj32_e8e9_bswap_le
#include "filter/ctoj.h"
#define F                       s_ctoj32_e8e9_bswap_le
#include "filter/ctoj.h"
#undef COND


/*************************************************************************
// cto calltrick with jmp, optional jcc
**************************************************************************/

#define COND1(b,x)     (b[x] == 0xe8 || b[x] == 0xe9)
#define COND2(b,x,lc)  (lc!=(x) && 0xf==b[(x)-1] && 0x80<=b[x] && b[x]<=0x8f)
#define COND(b,x,lc,id) (COND1(b,x) || ((9<=(0xf&(id))) && COND2(b,x,lc)))
#define F                       f_ctok32_e8e9_bswap_le
#define U                       u_ctok32_e8e9_bswap_le
#include "filter/ctok.h"
#define F                       s_ctok32_e8e9_bswap_le
#include "filter/ctok.h"
#undef COND
#undef COND2
#undef COND1


/*************************************************************************
// cto calltrick with jmp and jcc and relative renumbering
**************************************************************************/

#define COND_CALL(which,b,x) ((which = 0), b[x] == 0xe8)
#define COND_JMP( which,b,x) ((which = 1), b[x] == 0xe9)
#define COND_JCC( which,b,lastcall,x,y,z) ((which = 2), \
         (lastcall!=(x) && 0xf==b[y] && 0x80<=b[z] && b[z]<=0x8f))
#define COND1(which,b,x) (COND_CALL(which,b,x) || COND_JMP(which,b,x))
#define COND2(which,b,lastcall,x,y,z) COND_JCC(which,b,lastcall,x,y,z)

#define CONDF(which,b,x,lastcall) \
    (COND1(which,b,x) || COND2(which,b,lastcall,x,(x)-1, x   ))
#define CONDU(which,b,x,lastcall) \
    (COND1(which,b,x) || COND2(which,b,lastcall,x, x   ,(x)-1))

#define F                       f_ctojr32_e8e9_bswap_le
#define U                       u_ctojr32_e8e9_bswap_le
#include "filter/ctojr.h"
#define F                       s_ctojr32_e8e9_bswap_le
#include "filter/ctojr.h"

#undef CONDU
#undef CONDF
#undef COND2
#undef COND1
#undef COND_JCC
#undef COND_JMP
#undef COND_CALL


/*************************************************************************
// PowerPC branch [incl. call] trick
**************************************************************************/

#define COND(b,x) (18==(get_be32(b+x)>>26))
#define F                       f_ppcbxx
#define U                       u_ppcbxx
#include "filter/ppcbxx.h"
#define F                       s_ppcbxx
#include "filter/ppcbxx.h"
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

    // 32-bit cto calltrick with jmp
    { 0x36, 6, 0x00ffffff, f_ctoj32_e8e9_bswap_le, u_ctoj32_e8e9_bswap_le, s_ctoj32_e8e9_bswap_le },

    // 32-bit calltrick with jmp, optional jcc; runtime can unfilter more than one block
    { 0x46, 6, 0x00ffffff, f_ctok32_e8e9_bswap_le, u_ctok32_e8e9_bswap_le, s_ctok32_e8e9_bswap_le },
    { 0x49, 6, 0x00ffffff, f_ctok32_e8e9_bswap_le, u_ctok32_e8e9_bswap_le, s_ctok32_e8e9_bswap_le },

    // 24-bit calltrick for arm
    { 0x50, 8, 0x01ffffff, f_ct24arm_le, u_ct24arm_le, s_ct24arm_le },
    { 0x51, 8, 0x01ffffff, f_ct24arm_be, u_ct24arm_be, s_ct24arm_be },

    // 32-bit cto calltrick with jmp and jcc(swap 0x0f/0x8Y) and relative renumbering
    { 0x80, 8, 0x00ffffff, f_ctojr32_e8e9_bswap_le, u_ctojr32_e8e9_bswap_le, s_ctojr32_e8e9_bswap_le },
    { 0x81, 8, 0x00ffffff, f_ctojr32_e8e9_bswap_le, u_ctojr32_e8e9_bswap_le, s_ctojr32_e8e9_bswap_le },
    { 0x82, 8, 0x00ffffff, f_ctojr32_e8e9_bswap_le, u_ctojr32_e8e9_bswap_le, s_ctojr32_e8e9_bswap_le },
    { 0x83, 8, 0x00ffffff, f_ctojr32_e8e9_bswap_le, u_ctojr32_e8e9_bswap_le, s_ctojr32_e8e9_bswap_le },
    { 0x84, 8, 0x00ffffff, f_ctojr32_e8e9_bswap_le, u_ctojr32_e8e9_bswap_le, s_ctojr32_e8e9_bswap_le },
    { 0x85, 8, 0x00ffffff, f_ctojr32_e8e9_bswap_le, u_ctojr32_e8e9_bswap_le, s_ctojr32_e8e9_bswap_le },
    { 0x86, 8, 0x00ffffff, f_ctojr32_e8e9_bswap_le, u_ctojr32_e8e9_bswap_le, s_ctojr32_e8e9_bswap_le },
    { 0x87, 8, 0x00ffffff, f_ctojr32_e8e9_bswap_le, u_ctojr32_e8e9_bswap_le, s_ctojr32_e8e9_bswap_le },

    // simple delta filter
    { 0x90, 2,          0, f_sub8_1, u_sub8_1, s_sub8_1 },
    { 0x91, 3,          0, f_sub8_2, u_sub8_2, s_sub8_2 },
    { 0x92, 4,          0, f_sub8_3, u_sub8_3, s_sub8_3 },
    { 0x93, 5,          0, f_sub8_4, u_sub8_4, s_sub8_4 },

    { 0xa0,99,          0, f_sub16_1, u_sub16_1, s_sub16_1 },
    { 0xa1,99,          0, f_sub16_2, u_sub16_2, s_sub16_2 },
    { 0xa2,99,          0, f_sub16_3, u_sub16_3, s_sub16_3 },
    { 0xa3,99,          0, f_sub16_4, u_sub16_4, s_sub16_4 },

    { 0xb0,99,          0, f_sub32_1, u_sub32_1, s_sub32_1 },
    { 0xb1,99,          0, f_sub32_2, u_sub32_2, s_sub32_2 },
    { 0xb2,99,          0, f_sub32_3, u_sub32_3, s_sub32_3 },
    { 0xb3,99,          0, f_sub32_4, u_sub32_4, s_sub32_4 },

    // PowerPC branch+call trick
    { 0xd0, 8,          0, f_ppcbxx, u_ppcbxx, s_ppcbxx },
};

const int FilterImp::n_filters = TABLESIZE(filters);


/*
vi:ts=4:et:nowrap
*/

