/* system_defs.h -- system defines

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2024 Markus Franz Xaver Johannes Oberhumer
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

#pragma once

#if !defined(_FILE_OFFSET_BITS)
#define _FILE_OFFSET_BITS 64
#endif

// these are needed for some older glibc/mingw versions
#if !defined(__STDC_CONSTANT_MACROS)
#define __STDC_CONSTANT_MACROS 1
#endif
#if !defined(__STDC_FORMAT_MACROS)
#define __STDC_FORMAT_MACROS 1
#endif
#if !defined(__STDC_LIMIT_MACROS)
#define __STDC_LIMIT_MACROS 1
#endif

#if !defined(__USE_MINGW_ANSI_STDIO)
#if defined(_WIN32) && defined(__MINGW32__) && (defined(__clang__) || defined(__GNUC__))
#define __USE_MINGW_ANSI_STDIO 1
#endif
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
// disable silly warnings about using "deprecated" POSIX functions like fopen()
#if !defined(_CRT_NONSTDC_NO_DEPRECATE)
#define _CRT_NONSTDC_NO_DEPRECATE 1
#endif
#if !defined(_CRT_NONSTDC_NO_WARNINGS)
#define _CRT_NONSTDC_NO_WARNINGS 1
#endif
#if !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE 1
#endif
#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#endif // _WIN32

/* vim:set ts=4 sw=4 et: */
