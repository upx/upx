/* l_test.c --

   This file is part of the UPX executable compressor.

   Copyright (C) 2007-2007 Markus Franz Xaver Johannes Oberhumer
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
   <markus@oberhumer.com>
 */


#include "stub/src/c/lzma_d_c.c"

#if (ACC_ARCH_I086)
#define bytep unsigned char __huge *
typedef unsigned long   uint32_t;
#else
#define bytep unsigned char *
typedef unsigned int    uint32_t;
#endif


#if (ACC_OS_DOS16)
#  if (ACC_CC_BORLANDC || ACC_CC_TURBOC)
     int __cdecl printf(const char *, ...);
     void * __cdecl malloc(unsigned);
     void __far * __cdecl farmalloc(unsigned long);
#    define acc_halloc(x)   ((void __huge *) farmalloc(x))
#  elif (ACC_CC_DMC || ACC_CC_MSC)
#    include <stdio.h>
#    include <malloc.h>
#    define acc_halloc(x)   _halloc(x,1)
#  elif (ACC_CC_WATCOMC)
     int __watcall printf(const char *, ...);
     void * __watcall malloc(unsigned);
     void __huge * __watcall halloc(long, unsigned);
#    define acc_halloc(x)   halloc(x,1)
#  else
#  endif
#else
#  define printf            __builtin_printf
#  define malloc(x)         __builtin_malloc(x)
#  define acc_halloc(x)     malloc(x)
#endif


/*************************************************************************
//
**************************************************************************/

/*
>>> import pylzma; d="\1" + "\0"*131070 + "\2"; print len(d)
>>> c=pylzma.compress(d, eos=0)[5:]; print len(c), map(ord, c)
*/
static const unsigned char c_data[92] = {
0, 0, 128, 65, 72, 1, 140, 46, 188, 80, 161, 51, 135, 75, 212, 2, 20, 181, 241, 145, 230, 34, 107, 72, 201, 86, 118, 176, 70, 120, 214, 184, 247, 212, 250, 132, 59, 160, 44, 112, 185, 177, 245, 126, 103, 190, 14, 145, 73, 36, 148, 246, 166, 58, 41, 192, 68, 167, 144, 98, 122, 42, 61, 195, 135, 248, 98, 136, 254, 191, 96, 21, 192, 75, 86, 63, 228, 231, 15, 70, 52, 239, 169, 194, 249, 109, 126, 11, 123, 48, 0, 0
};



int main()
{
    uint32_t i;
    int r;
    uint32_t src_len = sizeof(c_data);
    uint32_t dst_len = 131072ul;
    uint32_t src_out = 0;
    uint32_t dst_out = 0;
    const bytep src;
    bytep dst;
    CLzmaDecoderState *s;

    printf("Decompress %lu %lu\n", (long)src_len, (long)dst_len);
    s = (CLzmaDecoderState *) malloc(32768u);
    src = c_data;
    dst = (bytep) acc_halloc(dst_len);
    if (!s || !dst) {
        printf("ERROR: Out of memory!\n");
        return 1;
    }
    for (i = 0; i != dst_len; ++i)
        dst[i] = 255;

    s->Properties.lc = 3; s->Properties.lp = 0; s->Properties.pb = 2;
    r = LzmaDecode(s, src, src_len, &src_out, dst, dst_len, &dst_out);

    if (r != 0 || src_out != src_len || dst_out != dst_len)
    {
        printf("ERROR: Decompression error %d %lu %lu\n", r, (long)src_out, (long)dst_out);
        return 1;
    }

    i = 0;
    if (dst[i] != 1)
        goto data_error;
    while (++i != dst_len - 1)
        if (dst[i] != 0)
            goto data_error;
    if (dst[i] != 2)
        goto data_error;

    printf("Decompression test passed. All done.\n");
    return 0;

data_error:
    printf("ERROR: Decompression data error at offset %lu.\n", (long)i);
    return 1;
}


/* vim:set ts=4 et: */
