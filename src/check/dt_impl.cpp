/* dt_impl.cpp -- doctest support code implementation

   This file is part of the UPX executable compressor.

   Copyright (C) Markus Franz Xaver Johannes Oberhumer
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

#include "../util/system_defs.h"
#include "../util/system_features.h"

/*************************************************************************
// doctest support code implementation
**************************************************************************/

#define DOCTEST_CONFIG_IMPLEMENT
#define DOCTEST_CONFIG_NO_UNPREFIXED_OPTIONS

#if !defined(DOCTEST_CONFIG_DISABLE)

#if defined(__wasi__)
#define DOCTEST_CONFIG_NO_MULTITHREADING
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#elif defined(__i386__) && defined(__MSDOS__) && defined(__DJGPP__) && defined(__GNUC__)
#define DOCTEST_CONFIG_NO_MULTITHREADING
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#elif defined(__m68k__) && defined(__atarist__) && defined(__GNUC__)
#define DOCTEST_CONFIG_COLORS_NONE
#define DOCTEST_CONFIG_NO_MULTITHREADING
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#pragma GCC diagnostic ignored "-Wshadow"
#endif

#if !(WITH_THREADS) && !defined(UPX_DOCTEST_CONFIG_MULTITHREADING)
#ifndef DOCTEST_CONFIG_NO_MULTITHREADING
#define DOCTEST_CONFIG_NO_MULTITHREADING
#endif
#endif

#if defined(__FAST_MATH__) && defined(__clang__) && (__clang_major__ + 0 > 0)
#if __clang_major__ >= 6
// warning: comparison with NaN always evaluates to false in fast floating point modes
#pragma clang diagnostic ignored "-Wtautological-constant-compare"
#endif
#if defined(__has_warning)
#if __has_warning("-Wnan-infinity-disabled")
// warning: use of NaN is undefined behavior due to the currently enabled floating-point options
#pragma clang diagnostic ignored "-Wnan-infinity-disabled"
#endif
#endif
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1940)
#pragma warning(disable : 5285) // Specializing template std::tuple is forbidden
#endif

// NOLINTBEGIN(bugprone-unintended-char-ostream-output)
#include <doctest/doctest/parts/doctest.cpp>
// NOLINTEND(bugprone-unintended-char-ostream-output)

#endif // DOCTEST_CONFIG_DISABLE

/* vim:set ts=4 sw=4 et: */
