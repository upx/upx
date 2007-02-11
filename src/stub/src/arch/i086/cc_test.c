/* cc_test.c -- examine compiler-generated library calls

   This file is part of the UPX executable compressor.

   Copyright (C) 2006-2007 Markus Franz Xaver Johannes Oberhumer
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

   Markus F.X.J. Oberhumer
   <mfx@users.sourceforge.net>
 */


#if 0 && defined(__WATCOMC__)
#define __cdecl __watcall
#endif


/*************************************************************************
//
**************************************************************************/

typedef short           int16_t;
typedef unsigned short  uint16_t;
typedef long            int32_t;
typedef unsigned long   uint32_t;

#if 1
// simulated 32-bit pointer
typedef char __huge*    hptr;
typedef long            hptrdiff_t;
typedef unsigned long   hsize_t;
#elif 1
// ignore segment overflow
typedef char __far*     hptr;
typedef long            hptrdiff_t;
typedef unsigned long   hsize_t;
#else
// no segment
typedef char __near*    hptr;
typedef short           hptrdiff_t;
typedef unsigned short  hsize_t;
#endif

// pia - pointer add
hptr __cdecl pia(hptr a, hsize_t d) { return a + d; }

// pis - pointer subtract
hptr __cdecl pis(hptr a, hsize_t d) { return a - d; }

// pts - pointer diff
hptrdiff_t __cdecl pts(hptr a, hptr b) { return a - b; }

// ptc - pointer compare (__PTC sets zero and carry flags)
int __cdecl ptc_eq(hptr a, hptr b) { return a == b; }
int __cdecl ptc_ne(hptr a, hptr b) { return a != b; }
int __cdecl ptc_lt(hptr a, hptr b) { return a < b; }
int __cdecl ptc_le(hptr a, hptr b) { return a <= b; }
int __cdecl ptc_gt(hptr a, hptr b) { return a > b; }
int __cdecl ptc_ge(hptr a, hptr b) { return a >= b; }

// u4m - unsigned multiplication
uint32_t __cdecl u4m(uint32_t a, uint32_t b) { return a * b; }

// i4m - signed multiplication
int32_t  __cdecl i4m(int32_t  a, int32_t  b) { return a * b; }

// just for testing
uint16_t __cdecl u2m(uint16_t a, uint16_t b) { return a * b; }
int16_t  __cdecl i2m(int16_t  a, int16_t  b) { return a * b; }
uint32_t __cdecl u2m4(uint16_t a, uint16_t b) { return a * b; }
int32_t  __cdecl i2m4(int16_t  a, int16_t  b) { return a * b; }


/* vim:set ts=4 et: */
