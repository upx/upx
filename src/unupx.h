/* unupx.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2000 Laszlo Molnar
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

   Markus F.X.J. Oberhumer                   Laszlo Molnar
   markus.oberhumer@jk.uni-linz.ac.at        ml1050@cdata.tvnet.hu
 */


#ifndef __UPX_UNUPX_H
#define __UPX_UNUPX_H


/*************************************************************************
// integral and pointer types
**************************************************************************/

#ifndef upx_byte
typedef int upx_int;
typedef unsigned upx_uint;
typedef int upx_int32;
typedef unsigned upx_uint32;

#define upx_byte unsigned char
#define upx_bytep upx_byte *
#define upx_voidp void *
#define upx_uintp upx_uint *
#endif


/*************************************************************************
// calling conventions
**************************************************************************/

#ifndef __UPX_CDECL
#define __UPX_CDECL
#endif
#ifndef __UPX_ENTRY
#define __UPX_ENTRY __UPX_CDECL
#endif
#define UPX_EXTERN(x)           extern "C" x __UPX_ENTRY
#define UPX_EXTERN_CDECL(x)     extern "C" x __UPX_CDECL


/*************************************************************************
// constants
**************************************************************************/

/* Executable formats. Note: big endian types are >= 128 */
#define UPX_F_DOS_COM           1
#define UPX_F_DOS_SYS           2
#define UPX_F_DOS_EXE           3
#define UPX_F_DJGPP2_COFF       4
#define UPX_F_WC_LE             5
#define UPX_F_VXD_LE            6
#define UPX_F_DOS_EXEH          7               /* OBSOLETE */
#define UPX_F_TMT_ADAM          8
#define UPX_F_W32_PE            9
#define UPX_F_LINUX_i386        10
#define UPX_F_BVMLINUX_i386     11
#define UPX_F_LINUX_ELF_i386    12
#define UPX_F_LINUX_SEP_i386    13
#define UPX_F_LINUX_SH_i386     14
#define UPX_F_ATARI_TOS         129
#define UPX_F_SOLARIS_SPARC     130


#define UPX_MAGIC_LE32      0x21585055          /* "UPX!" */


/*************************************************************************
// prototypes
**************************************************************************/


#endif /* already included */


/*
vi:ts=4:et
*/

