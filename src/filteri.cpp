/* filteri.cpp -- filter implementation (low-level)

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2000 Laszlo Molnar

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
// 16-bit calltrick ("naive")
**************************************************************************/

#define CT16(f, cond, addvalue, get, set) \
    upx_byte *b = f->buf; \
    upx_byte *b_end = b + f->buf_len - 3; \
    do { \
        if (cond) \
        { \
            b += 1; \
            unsigned a = (unsigned) (b - f->buf); \
            f->lastcall = a; \
            set(b, get(b) + (addvalue)); \
            f->calls++; \
            b += 2 - 1; \
        } \
    } while (++b < b_end); \
    if (f->lastcall) f->lastcall += 2; \
    return 0;



// filter: e8, e9, e8e9
static int f_ct16_e8(Filter *f)
{
    CT16(f, (*b == 0xe8), a + f->addvalue, get_le16, set_le16)
}

static int f_ct16_e9(Filter *f)
{
    CT16(f, (*b == 0xe9), a + f->addvalue, get_le16, set_le16)
}

static int f_ct16_e8e9(Filter *f)
{
    CT16(f, (*b == 0xe8 || *b == 0xe9), a + f->addvalue, get_le16, set_le16)
}


// unfilter: e8, e9, e8e9
static int u_ct16_e8(Filter *f)
{
    CT16(f, (*b == 0xe8), 0 - a - f->addvalue, get_le16, set_le16)
}

static int u_ct16_e9(Filter *f)
{
    CT16(f, (*b == 0xe9), 0 - a - f->addvalue, get_le16, set_le16)
}

static int u_ct16_e8e9(Filter *f)
{
    CT16(f, (*b == 0xe8 || *b == 0xe9), 0 - a - f->addvalue, get_le16, set_le16)
}


// scan: e8, e9, e8e9
static int s_ct16_e8(Filter *f)
{
    CT16(f, (*b == 0xe8), 0 - a - f->addvalue, get_le16, set_dummy)
}

static int s_ct16_e9(Filter *f)
{
    CT16(f, (*b == 0xe9), 0 - a - f->addvalue, get_le16, set_dummy)
}

static int s_ct16_e8e9(Filter *f)
{
    CT16(f, (*b == 0xe8 || *b == 0xe9), 0 - a - f->addvalue, get_le16, set_dummy)
}


// filter: e8, e9, e8e9 with bswap le->be
static int f_ct16_e8_bswap_le(Filter *f)
{
    CT16(f, (*b == 0xe8), a + f->addvalue, get_le16, set_be16)
}

static int f_ct16_e9_bswap_le(Filter *f)
{
    CT16(f, (*b == 0xe9), a + f->addvalue, get_le16, set_be16)
}

static int f_ct16_e8e9_bswap_le(Filter *f)
{
    CT16(f, (*b == 0xe8 || *b == 0xe9), a + f->addvalue, get_le16, set_be16)
}


// unfilter: e8, e9, e8e9 with bswap le->be
static int u_ct16_e8_bswap_le(Filter *f)
{
    CT16(f, (*b == 0xe8), 0 - a - f->addvalue, get_be16, set_le16)
}

static int u_ct16_e9_bswap_le(Filter *f)
{
    CT16(f, (*b == 0xe9), 0 - a - f->addvalue, get_be16, set_le16)
}

static int u_ct16_e8e9_bswap_le(Filter *f)
{
    CT16(f, (*b == 0xe8 || *b == 0xe9), 0 - a - f->addvalue, get_be16, set_le16)
}


// scan: e8, e9, e8e9 with bswap le->be
static int s_ct16_e8_bswap_le(Filter *f)
{
    CT16(f, (*b == 0xe8), 0 - a - f->addvalue, get_be16, set_dummy)
}

static int s_ct16_e9_bswap_le(Filter *f)
{
    CT16(f, (*b == 0xe9), 0 - a - f->addvalue, get_be16, set_dummy)
}

static int s_ct16_e8e9_bswap_le(Filter *f)
{
    CT16(f, (*b == 0xe8 || *b == 0xe9), 0 - a - f->addvalue, get_be16, set_dummy)
}


// filter: e8, e9, e8e9 with bswap be->le
static int f_ct16_e8_bswap_be(Filter *f)
{
    CT16(f, (*b == 0xe8), a + f->addvalue, get_be16, set_le16)
}

static int f_ct16_e9_bswap_be(Filter *f)
{
    CT16(f, (*b == 0xe9), a + f->addvalue, get_be16, set_le16)
}

static int f_ct16_e8e9_bswap_be(Filter *f)
{
    CT16(f, (*b == 0xe8 || *b == 0xe9), a + f->addvalue, get_be16, set_le16)
}


// unfilter: e8, e9, e8e9 with bswap be->le
static int u_ct16_e8_bswap_be(Filter *f)
{
    CT16(f, (*b == 0xe8), 0 - a - f->addvalue, get_le16, set_be16)
}

static int u_ct16_e9_bswap_be(Filter *f)
{
    CT16(f, (*b == 0xe9), 0 - a - f->addvalue, get_le16, set_be16)
}

static int u_ct16_e8e9_bswap_be(Filter *f)
{
    CT16(f, (*b == 0xe8 || *b == 0xe9), 0 - a - f->addvalue, get_le16, set_be16)
}


// scan: e8, e9, e8e9 with bswap be->le
static int s_ct16_e8_bswap_be(Filter *f)
{
    CT16(f, (*b == 0xe8), 0 - a - f->addvalue, get_le16, set_dummy)
}

static int s_ct16_e9_bswap_be(Filter *f)
{
    CT16(f, (*b == 0xe9), 0 - a - f->addvalue, get_le16, set_dummy)
}

static int s_ct16_e8e9_bswap_be(Filter *f)
{
    CT16(f, (*b == 0xe8 || *b == 0xe9), 0 - a - f->addvalue, get_le16, set_dummy)
}


#undef CT16


/*************************************************************************
// 32-bit calltrick ("naive")
**************************************************************************/

#define CT32(f, cond, addvalue, get, set) \
    upx_byte *b = f->buf; \
    upx_byte *b_end = b + f->buf_len - 5; \
    do { \
        if (cond) \
        { \
            b += 1; \
            unsigned a = (unsigned) (b - f->buf); \
            f->lastcall = a; \
            set(b, get(b) + (addvalue)); \
            f->calls++; \
            b += 4 - 1; \
        } \
    } while (++b < b_end); \
    if (f->lastcall) f->lastcall += 4; \
    return 0;


// filter: e8, e9, e8e9
static int f_ct32_e8(Filter *f)
{
    CT32(f, (*b == 0xe8), a + f->addvalue, get_le32, set_le32)
}

static int f_ct32_e9(Filter *f)
{
    CT32(f, (*b == 0xe9), a + f->addvalue, get_le32, set_le32)
}

static int f_ct32_e8e9(Filter *f)
{
    CT32(f, (*b == 0xe8 || *b == 0xe9), a + f->addvalue, get_le32, set_le32)
}


// unfilter: e8, e9, e8e9
static int u_ct32_e8(Filter *f)
{
    CT32(f, (*b == 0xe8), 0 - a - f->addvalue, get_le32, set_le32)
}

static int u_ct32_e9(Filter *f)
{
    CT32(f, (*b == 0xe9), 0 - a - f->addvalue, get_le32, set_le32)
}

static int u_ct32_e8e9(Filter *f)
{
    CT32(f, (*b == 0xe8 || *b == 0xe9), 0 - a - f->addvalue, get_le32, set_le32)
}


// scan: e8, e9, e8e9
static int s_ct32_e8(Filter *f)
{
    CT32(f, (*b == 0xe8), 0 - a - f->addvalue, get_le32, set_dummy)
}

static int s_ct32_e9(Filter *f)
{
    CT32(f, (*b == 0xe9), 0 - a - f->addvalue, get_le32, set_dummy)
}

static int s_ct32_e8e9(Filter *f)
{
    CT32(f, (*b == 0xe8 || *b == 0xe9), 0 - a - f->addvalue, get_le32, set_dummy)
}


// filter: e8, e9, e8e9 with bswap le->be
static int f_ct32_e8_bswap_le(Filter *f)
{
    CT32(f, (*b == 0xe8), a + f->addvalue, get_le32, set_be32)
}

static int f_ct32_e9_bswap_le(Filter *f)
{
    CT32(f, (*b == 0xe9), a + f->addvalue, get_le32, set_be32)
}

static int f_ct32_e8e9_bswap_le(Filter *f)
{
    CT32(f, (*b == 0xe8 || *b == 0xe9), a + f->addvalue, get_le32, set_be32)
}


// unfilter: e8, e9, e8e9 with bswap le->be
static int u_ct32_e8_bswap_le(Filter *f)
{
    CT32(f, (*b == 0xe8), 0 - a - f->addvalue, get_be32, set_le32)
}

static int u_ct32_e9_bswap_le(Filter *f)
{
    CT32(f, (*b == 0xe9), 0 - a - f->addvalue, get_be32, set_le32)
}

static int u_ct32_e8e9_bswap_le(Filter *f)
{
    CT32(f, (*b == 0xe8 || *b == 0xe9), 0 - a - f->addvalue, get_be32, set_le32)
}


// scan: e8, e9, e8e9 with bswap le->be
static int s_ct32_e8_bswap_le(Filter *f)
{
    CT32(f, (*b == 0xe8), 0 - a - f->addvalue, get_be32, set_dummy)
}

static int s_ct32_e9_bswap_le(Filter *f)
{
    CT32(f, (*b == 0xe9), 0 - a - f->addvalue, get_be32, set_dummy)
}

static int s_ct32_e8e9_bswap_le(Filter *f)
{
    CT32(f, (*b == 0xe8 || *b == 0xe9), 0 - a - f->addvalue, get_be32, set_dummy)
}


// filter: e8, e9, e8e9 with bswap be->le
static int f_ct32_e8_bswap_be(Filter *f)
{
    CT32(f, (*b == 0xe8), a + f->addvalue, get_be32, set_le32)
}

static int f_ct32_e9_bswap_be(Filter *f)
{
    CT32(f, (*b == 0xe9), a + f->addvalue, get_be32, set_le32)
}

static int f_ct32_e8e9_bswap_be(Filter *f)
{
    CT32(f, (*b == 0xe8 || *b == 0xe9), a + f->addvalue, get_be32, set_le32)
}


// unfilter: e8, e9, e8e9 with bswap be->le
static int u_ct32_e8_bswap_be(Filter *f)
{
    CT32(f, (*b == 0xe8), 0 - a - f->addvalue, get_le32, set_be32)
}

static int u_ct32_e9_bswap_be(Filter *f)
{
    CT32(f, (*b == 0xe9), 0 - a - f->addvalue, get_le32, set_be32)
}

static int u_ct32_e8e9_bswap_be(Filter *f)
{
    CT32(f, (*b == 0xe8 || *b == 0xe9), 0 - a - f->addvalue, get_le32, set_be32)
}


// scan: e8, e9, e8e9 with bswap be->le
static int s_ct32_e8_bswap_be(Filter *f)
{
    CT32(f, (*b == 0xe8), 0 - a - f->addvalue, get_le32, set_dummy)
}

static int s_ct32_e9_bswap_be(Filter *f)
{
    CT32(f, (*b == 0xe9), 0 - a - f->addvalue, get_le32, set_dummy)
}

static int s_ct32_e8e9_bswap_be(Filter *f)
{
    CT32(f, (*b == 0xe8 || *b == 0xe9), 0 - a - f->addvalue, get_le32, set_dummy)
}


#undef CT32

#undef set_dummy


/*************************************************************************
// 32-bit calltrick with cto ("clever")
//
// This version is more sophisticated because it only
// tries to change actual calls and/or jumps.
**************************************************************************/

#if 1
// use Laszlo's implementation
#include "fcto_ml.ch"
#else
// use Marco's implementation
#include "fcto_mfx.ch"
#endif


/*************************************************************************
// database for use in class Filter
**************************************************************************/

const FilterImp::FilterEntry FilterImp::filters[] = {
    // no filter
    { 0x00, 0,          0, NULL, NULL, NULL },
    // 16-bit calltrick
    { 0x01, 4,          0, f_ct16_e8, u_ct16_e8, s_ct16_e8,},
    { 0x02, 4,          0, f_ct16_e9, u_ct16_e9, s_ct16_e9 },
    { 0x03, 4,          0, f_ct16_e8e9, u_ct16_e8e9, s_ct16_e8e9 },
    { 0x04, 4,          0, f_ct16_e8_bswap_le, u_ct16_e8_bswap_le, s_ct16_e8_bswap_le },
    { 0x05, 4,          0, f_ct16_e9_bswap_le, u_ct16_e9_bswap_le, s_ct16_e9_bswap_le },
    { 0x06, 4,          0, f_ct16_e8e9_bswap_le, u_ct16_e8e9_bswap_le, s_ct16_e8e9_bswap_le },
    { 0x07, 4,          0, f_ct16_e8_bswap_be, u_ct16_e8_bswap_be, s_ct16_e8_bswap_be },
    { 0x08, 4,          0, f_ct16_e9_bswap_be, u_ct16_e9_bswap_be, s_ct16_e9_bswap_be },
    { 0x09, 4,          0, f_ct16_e8e9_bswap_be, u_ct16_e8e9_bswap_be, s_ct16_e8e9_bswap_be },
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
    // 32-bit cto calltrick
    { 0x21, 6, 0x00ffffff, f_cto32_e8, u_cto32_e8, s_cto32_e8 },
    { 0x22, 6, 0x00ffffff, f_cto32_e9, u_cto32_e9, s_cto32_e9 },
    { 0x23, 6, 0x00ffffff, f_cto32_e8e9, u_cto32_e8e9, s_cto32_e8e9 },
    { 0x24, 6, 0x00ffffff, f_cto32_e8_bswap_le, u_cto32_e8_bswap_le, s_cto32_e8_bswap_le },
    { 0x25, 6, 0x00ffffff, f_cto32_e9_bswap_le, u_cto32_e9_bswap_le, s_cto32_e9_bswap_le },
    { 0x26, 6, 0x00ffffff, f_cto32_e8e9_bswap_le, u_cto32_e8e9_bswap_le, s_cto32_e8e9_bswap_le },
    { 0x27, 6, 0x00ffffff, f_cto32_e8_bswap_be, u_cto32_e8_bswap_be, s_cto32_e8_bswap_be },
    { 0x28, 6, 0x00ffffff, f_cto32_e9_bswap_be, u_cto32_e9_bswap_be, s_cto32_e9_bswap_be },
    { 0x29, 6, 0x00ffffff, f_cto32_e8e9_bswap_be, u_cto32_e8e9_bswap_be, s_cto32_e8e9_bswap_be },
};

const int FilterImp::n_filters = HIGH(filters);


/*
vi:ts=4:et:nowrap
*/

