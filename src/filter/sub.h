/* sub.h -- simple delta filter

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

   Markus F.X.J. Oberhumer   Laszlo Molnar
   markus@oberhumer.com      ml1050@cdata.tvnet.hu
 */



/*************************************************************************
//
**************************************************************************/

#define SUB(f, N) \
    upx_byte *b = f->buf; \
    unsigned l = f->buf_len; \
    int i; \
    unsigned char d[N]; \
    \
    i = N - 1; do d[i] = 0; while (--i >= 0); \
    \
    i = N - 1; \
    do { \
        *b -= d[i]; \
        d[i] += *b++; \
        if (--i < 0) \
            i = N - 1; \
    } while (--l > 0); \
    f->calls = f->buf_len - 1; \
    return 0;


#define ADD(f, N) \
    upx_byte *b = f->buf; \
    unsigned l = f->buf_len; \
    int i; \
    unsigned char d[N]; \
    \
    i = N - 1; do d[i] = 0; while (--i >= 0); \
    \
    i = N - 1; \
    do { \
        d[i] += *b; \
        *b++ = d[i]; \
        if (--i < 0) \
            i = N - 1; \
    } while (--l > 0); \
    f->calls = f->buf_len - 1; \
    return 0;


#define SCAN(f, N) \
    f->calls = f->buf_len - 1; \
    return 0;


// filter
static int f_sub1(Filter *f)
{
    SUB(f, 1)
}

static int f_sub2(Filter *f)
{
    SUB(f, 2)
}

static int f_sub3(Filter *f)
{
    SUB(f, 3)
}

static int f_sub4(Filter *f)
{
    SUB(f, 4)
}


// unfilter
static int u_sub1(Filter *f)
{
    ADD(f, 1)
}

static int u_sub2(Filter *f)
{
    ADD(f, 2)
}

static int u_sub3(Filter *f)
{
    ADD(f, 3)
}

static int u_sub4(Filter *f)
{
    ADD(f, 4)
}


// scan
static int s_sub1(Filter *f)
{
    SCAN(f, 1)
}

static int s_sub2(Filter *f)
{
    SCAN(f, 2)
}

static int s_sub3(Filter *f)
{
    SCAN(f, 3)
}

static int s_sub4(Filter *f)
{
    SCAN(f, 4)
}


#undef SUB
#undef ADD
#undef SCAN


/*
vi:ts=4:et:nowrap
*/

