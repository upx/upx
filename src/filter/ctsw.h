/* ctsw.h -- call-/swaptrick filter

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



/*************************************************************************
// 16-bit call-/swaptrick ("naive")
**************************************************************************/

#define CTSW16(f, cond1, cond2, addvalue, get, set) \
    upx_byte *b = f->buf; \
    upx_byte *b_end = b + f->buf_len - 3; \
    do { \
        if (cond1) \
        { \
            b += 1; \
            unsigned a = (unsigned) (b - f->buf); \
            f->lastcall = a; \
            set(b, get(b) + (addvalue)); \
            f->calls++; \
            b += 2 - 1; \
        } \
        else if (cond2) \
        { \
            b += 1; \
            unsigned a = (unsigned) (b - f->buf); \
            f->lastcall = a; \
            set(b, get(b)); \
            f->calls++; \
            b += 2 - 1; \
        } \
    } while (++b < b_end); \
    if (f->lastcall) f->lastcall += 2; \
    return 0;


// filter
static int f_ctsw16_e8_e9(Filter *f)
{
    CTSW16(f, (*b == 0xe8), (*b == 0xe9), a + f->addvalue, get_le16, set_be16)
}

static int f_ctsw16_e9_e8(Filter *f)
{
    CTSW16(f, (*b == 0xe9), (*b == 0xe8), a + f->addvalue, get_le16, set_be16)
}


// unfilter
static int u_ctsw16_e8_e9(Filter *f)
{
    CTSW16(f, (*b == 0xe8), (*b == 0xe9), 0 - a - f->addvalue, get_be16, set_le16)
}

static int u_ctsw16_e9_e8(Filter *f)
{
    CTSW16(f, (*b == 0xe9), (*b == 0xe8), 0 - a - f->addvalue, get_be16, set_le16)
}


// scan
static int s_ctsw16_e8_e9(Filter *f)
{
    CTSW16(f, (*b == 0xe8), (*b == 0xe9), a + f->addvalue, get_le16, set_dummy)
}

static int s_ctsw16_e9_e8(Filter *f)
{
    CTSW16(f, (*b == 0xe9), (*b == 0xe8), a + f->addvalue, get_le16, set_dummy)
}


#undef CTSW16


/*************************************************************************
// 32-bit call-/swaptrick ("naive")
**************************************************************************/

#define CTSW32(f, cond1, cond2, addvalue, get, set) \
    upx_byte *b = f->buf; \
    upx_byte *b_end = b + f->buf_len - 5; \
    do { \
        if (cond1) \
        { \
            b += 1; \
            unsigned a = (unsigned) (b - f->buf); \
            f->lastcall = a; \
            set(b, get(b) + (addvalue)); \
            f->calls++; \
            b += 4 - 1; \
        } \
        else if (cond2) \
        { \
            b += 1; \
            unsigned a = (unsigned) (b - f->buf); \
            f->lastcall = a; \
            set(b, get(b)); \
            f->calls++; \
            b += 4 - 1; \
        } \
    } while (++b < b_end); \
    if (f->lastcall) f->lastcall += 4; \
    return 0;


// filter
static int f_ctsw32_e8_e9(Filter *f)
{
    CTSW32(f, (*b == 0xe8), (*b == 0xe9), a + f->addvalue, get_le32, set_be32)
}

static int f_ctsw32_e9_e8(Filter *f)
{
    CTSW32(f, (*b == 0xe9), (*b == 0xe8), a + f->addvalue, get_le32, set_be32)
}


// unfilter
static int u_ctsw32_e8_e9(Filter *f)
{
    CTSW32(f, (*b == 0xe8), (*b == 0xe9), 0 - a - f->addvalue, get_be32, set_le32)
}

static int u_ctsw32_e9_e8(Filter *f)
{
    CTSW32(f, (*b == 0xe9), (*b == 0xe8), 0 - a - f->addvalue, get_be32, set_le32)
}


// scan
static int s_ctsw32_e8_e9(Filter *f)
{
    CTSW32(f, (*b == 0xe8), (*b == 0xe9), a + f->addvalue, get_le32, set_dummy)
}

static int s_ctsw32_e9_e8(Filter *f)
{
    CTSW32(f, (*b == 0xe9), (*b == 0xe8), a + f->addvalue, get_le32, set_dummy)
}


#undef CTSW32


/*
vi:ts=4:et:nowrap
*/

