/* windows_lean.h -- <windows.h> include wrapper

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
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

// UPX only uses the very basic Windows API
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN 1
#endif
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0400 // _WIN32_WINNT_NT4 aka Windows NT 4
#endif

#if (defined(_MSC_VER) && (_MSC_VER >= 1000 && _MSC_VER < 1200)) && !defined(__clang__)
/* avoid -W4 warnings in <conio.h> */
#pragma warning(disable : 4032)
/* avoid -W4 warnings in <windows.h> */
#pragma warning(disable : 4201 4214 4514)
#endif
#if defined(_MSC_VER) && !defined(__clang__)
/* avoid warnings in some older versions of <windows.h> */
#pragma warning(disable : 5105)
#endif

#if defined(__RSXNT__) && !defined(timeval)
#define timeval win32_timeval /* struct timeval already in <sys/time.h> */
#endif

#include <windows.h>
