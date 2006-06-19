/* lzma_d.c --

   This file is part of the UPX executable compressor.

   Copyright (C) 2006-2006 Markus Franz Xaver Johannes Oberhumer
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


/*************************************************************************
//
**************************************************************************/

#define ACC_LIBC_NAKED
#define ACC_OS_FREESTANDING
#include "../../../../../miniacc.h"

#undef _LZMA_IN_CB
#undef _LZMA_OUT_READ
#undef _LZMA_PROB32
#undef _LZMA_LOC_OPT

#if 0

#include "C/7zip/Compress/LZMA_C/LzmaDecode.h"
#include "C/7zip/Compress/LZMA_C/LzmaDecode.c"
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(CLzmaDecoderState) == 16)

#else

#define CLzmaDecoderState   CLzmaDecoderState_dummy
#define LzmaDecode          LzmaDecode_dummy
#include "C/7zip/Compress/LZMA_C/LzmaDecode.h"
#undef CLzmaDecoderState
#undef LzmaDecode
typedef struct {
    struct { unsigned char lc, lp, pb, dummy; } Properties;
    CProb Probs[6];
//    CProb *Probs;
} CLzmaDecoderState;
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(CLzmaDecoderState) == 16)
#define CLzmaDecoderState   const CLzmaDecoderState
#include "C/7zip/Compress/LZMA_C/LzmaDecode.c"

#endif


/*
vi:ts=4:et:nowrap
*/

