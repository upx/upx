/* lzma_d.c --

   This file is part of the UPX executable compressor.

   Copyright (C) 2006-2008 Markus Franz Xaver Johannes Oberhumer
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


/*************************************************************************
//
**************************************************************************/

#define ACC_LIBC_NAKED 1
/*#define ACC_OS_FREESTANDING 1*/
#include "miniacc.h"

#if 0
#undef _LZMA_IN_CB
#undef _LZMA_OUT_READ
#undef _LZMA_PROB32
#undef _LZMA_LOC_OPT
#endif
#if (ACC_ARCH_I086)
#  define Byte  unsigned char
#  define _7ZIP_BYTE_DEFINED 1
#endif
#if !defined(_LZMA_UINT32_IS_ULONG)
#  if defined(__INT_MAX__) && ((__INT_MAX__)+0 == 32767)
#    define _LZMA_UINT32_IS_ULONG 1
#  endif
#endif
#if !defined(_LZMA_NO_SYSTEM_SIZE_T)
#  define _LZMA_NO_SYSTEM_SIZE_T 1
#endif

#if 0

#include "C/7zip/Compress/LZMA_C/LzmaDecode.h"
#if (ACC_ABI_LP64)
#else
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(CLzmaDecoderState) == 16)
#endif
#include "C/7zip/Compress/LZMA_C/LzmaDecode.c"

#else

#define CLzmaDecoderState       CLzmaDecoderState_dummy
#define LzmaDecodeProperties    LzmaDecodeProperties_dummy
#define LzmaDecode              LzmaDecode_dummy
#if (ACC_CC_BORLANDC)
#include "LzmaDecode.h"
#else
#if (WITH_LZMA >= 0x449)
#  include "C/Compress/Lzma/LzmaDecode.h"
#else
#  include "C/7zip/Compress/LZMA_C/LzmaDecode.h"
#endif
#endif
#undef CLzmaDecoderState
#undef LzmaDecodeProperties
#undef LzmaDecode
typedef struct {
    struct { unsigned char lc, lp, pb, dummy; } Properties;
#ifdef _LZMA_PROB32
    CProb Probs[8191];
#else
    CProb Probs[16382];
#endif
} CLzmaDecoderState;
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(CLzmaDecoderState) == 32768u)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(SizeT) >= 4)

#if (ACC_ARCH_I086)
#  define char  char __huge
#elif (ACC_CC_WATCOMC)
#else
#define CLzmaDecoderState   const CLzmaDecoderState
#endif
int LzmaDecodeProperties(CLzmaProperties *, const unsigned char *, int);
int LzmaDecode(CLzmaDecoderState *, const unsigned char *, SizeT, SizeT *, unsigned char *, SizeT, SizeT *);
#if (ACC_CC_BORLANDC)
#include "LzmaDecode.c"
#else
#if (WITH_LZMA >= 0x449)
#  include "C/Compress/Lzma/LzmaDecode.c"
#else
#  include "C/7zip/Compress/LZMA_C/LzmaDecode.c"
#endif
#endif
#undef char
#undef CLzmaDecoderState

#endif


/*
vi:ts=4:et:nowrap
*/

