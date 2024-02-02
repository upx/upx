/* dt_impl.cpp -- doctest support code implementation

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

/*************************************************************************
// doctest support code implementation
**************************************************************************/

// libc++ hardenining
#if defined(__clang__) && defined(__clang_major__) && (__clang_major__ + 0 >= 18)
#if DEBUG
#define _LIBCPP_HARDENING_MODE _LIBCPP_HARDENING_MODE_DEBUG
#else
#define _LIBCPP_HARDENING_MODE _LIBCPP_HARDENING_MODE_EXTENSIVE
#endif
#endif
#if defined(__clang__) && defined(__clang_major__) && (__clang_major__ + 0 < 18)
#if DEBUG
#define _LIBCPP_ENABLE_ASSERTIONS 1
#endif
#endif

#if defined(__has_include)
#if __has_include(<features.h>)
#include <features.h> // for __GLIBC__
#endif
#endif
// aligned_alloc() was added in glibc-2.16
#if defined(__ELF__) && (__GLIBC__ + 0 == 2) && (__GLIBC_MINOR__ + 0 < 16)
#define _LIBCPP_HAS_NO_LIBRARY_ALIGNED_ALLOCATION
#endif

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

#if !defined(UPX_DOCTEST_CONFIG_MULTITHREADING) && !(WITH_THREADS)
#ifndef DOCTEST_CONFIG_NO_MULTITHREADING
#define DOCTEST_CONFIG_NO_MULTITHREADING
#endif
#endif

#if defined(__clang__) && defined(__FAST_MATH__) && defined(__INTEL_LLVM_COMPILER)
// warning: comparison with NaN always evaluates to false in fast floating point modes
#pragma clang diagnostic ignored "-Wtautological-constant-compare"
#endif

#include <doctest/doctest/parts/doctest.cpp>

#endif // DOCTEST_CONFIG_DISABLE

/* vim:set ts=4 sw=4 et: */
