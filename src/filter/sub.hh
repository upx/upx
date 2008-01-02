/* sub.hh -- simple delta filter

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
//
**************************************************************************/

#define SUB(f, N, T, get, set) \
    upx_byte *b = f->buf; \
    unsigned l = f->buf_len / sizeof(T); \
    int i; \
    T d[N]; \
    \
    i = N - 1; do d[i] = 0; while (--i >= 0); \
    \
    i = N - 1; \
    do { \
        T delta = (T) (get(b) - d[i]); \
        set(b, delta); \
        d[i] = (T) (d[i] + delta); \
        b += sizeof(T); \
        if (--i < 0) \
            i = N - 1; \
    } while (--l > 0); \
    f->calls = (f->buf_len / sizeof(T)) - N; \
    assert((int)f->calls > 0); \
    return 0;


#define ADD(f, N, T, get, set) \
    upx_byte *b = f->buf; \
    unsigned l = f->buf_len / sizeof(T); \
    int i; \
    T d[N]; \
    \
    i = N - 1; do d[i] = 0; while (--i >= 0); \
    \
    i = N - 1; \
    do { \
        d[i] = (T) (d[i] + get(b)); \
        set(b, d[i]); \
        b += sizeof(T); \
        if (--i < 0) \
            i = N - 1; \
    } while (--l > 0); \
    f->calls = (f->buf_len / sizeof(T)) - N; \
    assert((int)f->calls > 0); \
    return 0;


#define SCAN(f, N, T, get, set) \
    f->calls = (f->buf_len / sizeof(T)) - N; \
    assert((int)f->calls > 0); \
    return 0;



/*
vi:ts=4:et:nowrap
*/

