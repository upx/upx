/* compress.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2004 Laszlo Molnar
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
 */


#include "conf.h"


#if defined(WITH_NRV)
#  include "compress_nrv.ch"
#elif defined(WITH_UCL)
#  define upx_adler32       upx_adler32
//#  define upx_crc32         upx_crc32
#  define upx_compress      upx_compress
#  define upx_decompress    upx_decompress
#  if (UPX_VERSION_HEX >= 0x019000)
#    define ALG_NRV2E
#    define upx_test_overlap  upx_test_overlap
#  endif
#  include "compress.ch"
#else
#  error
#endif


/*
vi:ts=4:et:nowrap
*/

