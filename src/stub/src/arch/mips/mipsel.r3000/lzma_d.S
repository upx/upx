/* lzma_d.S -- MIPS R3000 decompressor for LZMA

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2007 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2007 Laszlo Molnar
   Copyright (C) 2000-2007 John F. Reiser
   Copyright (C) 2002-2007 Jens Medoch
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
   <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>

   John F. Reiser                       Jens Medoch
   <jreiser@users.sourceforge.net>      <jssg@users.sourceforge.net>
 */

section     lzma.init
            li      tmp,%lo(lzma_stack_adjust-lzma_args_sz)
            addu    sp,tmp
/*
            move    var,sp
1:
            sw      zero,0(var)
            addiu   tmp,4
            bnez    tmp,1b
            addiu   var,4
*/
            addiu   src,2
            la      inSize,lzma_c_len
            la      outSize,lzma_u_len

            addu    poutSizeProcessed,sp,outSizeProcessed
            addiu   pinSizeprocessed,sp,inSizeProcessed

            la      tmp,lzma_properties
            sw      dst,dst_save(sp)
            addiu   CLzmaDecoderState,sp,lzma_args_sz
            bal     decompressor
            sw      tmp,0(CLzmaDecoderState)
            lw      dst,dst_save(sp)
            lw      outSize,outSizeProcessed(sp)
            li      tmp,%lo(lzma_stack_adjust-lzma_args_sz)
            addu    dst,outSize
            subu    sp,tmp

section     LZMA_DEC10
#include    "lzma_d_cs.S"

section     LZMA_DEC20
#include    "lzma_d_cf.S"


// vi:ts=8:et

