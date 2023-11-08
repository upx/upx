/* headers.h -- include system headers

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

#if !(__cplusplus + 0 >= 201703L)
#error "C++17 is required"
#endif

// sanity check
#if defined(__ILP32) || defined(__ILP32__)
static_assert(sizeof(int) == 4);
static_assert(sizeof(long) == 4);
static_assert(sizeof(void *) == 4);
#endif
#if defined(__LP64) || defined(__LP64__)
static_assert(sizeof(int) == 4);
static_assert(sizeof(long) == 8);
static_assert(sizeof(void *) == 8);
#endif
#if defined(_WIN32)
static_assert(sizeof(int) == 4);
static_assert(sizeof(long) == 4);
#if !defined(_WIN64)
static_assert(sizeof(void *) == 4);
#endif
#endif
#if defined(_WIN64)
static_assert(sizeof(int) == 4);
static_assert(sizeof(long) == 4);
static_assert(sizeof(void *) == 8);
#endif

#if !defined(_FILE_OFFSET_BITS)
#define _FILE_OFFSET_BITS 64
#endif
#if defined(_WIN32) && defined(__MINGW32__) && defined(__GNUC__)
#if !defined(_USE_MINGW_ANSI_STDIO)
#define _USE_MINGW_ANSI_STDIO 1
#endif
#endif
#if defined(_WIN32)
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

// ACC and C system headers
#ifndef ACC_CFG_USE_NEW_STYLE_CASTS
#define ACC_CFG_USE_NEW_STYLE_CASTS 1
#endif
#define ACC_CFG_PREFER_TYPEOF_ACC_INT32E_T ACC_TYPEOF_INT
#define ACC_CFG_PREFER_TYPEOF_ACC_INT64E_T ACC_TYPEOF_LONG_LONG
#include "miniacc.h"

// disable some pedantic warnings
#if (ACC_CC_MSC)
#pragma warning(disable : 4127) // W4: conditional expression is constant
#pragma warning(disable : 4244) // W3: conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable : 4267) // W3: conversion from 'size_t' to 'type', possible loss of data
#pragma warning(disable : 4820) // W4: padding added after data member
#endif

#undef snprintf
#undef vsnprintf
#define HAVE_STDINT_H       1
#define ACC_WANT_ACC_INCD_H 1
#define ACC_WANT_ACC_INCE_H 1
#define ACC_WANT_ACC_LIB_H  1
#define ACC_WANT_ACC_CXX_H  1
#include "miniacc.h"
#if (ACC_CC_MSC)
#include <intrin.h>
#endif

// C++ system headers
#include <cstddef>
#include <exception>
#include <new>
#include <type_traits>

// C++ multithreading (UPX currently does not use multithreading)
#ifndef WITH_THREADS
#define WITH_THREADS 0
#endif
#if __STDC_NO_ATOMICS__
#undef WITH_THREADS
#endif
#if WITH_THREADS
#include <atomic>
#include <mutex>
#endif

// sanitizers
#if !defined(__SANITIZE_ADDRESS__) && defined(__has_feature)
#if __has_feature(address_sanitizer)
#define __SANITIZE_ADDRESS__ 1
#endif
#endif
#if !defined(__SANITIZE_MEMORY__) && defined(__has_feature)
#if __has_feature(memory_sanitizer)
#define __SANITIZE_MEMORY__ 1
#endif
#endif

// UPX vendor git submodule headers
#include <doctest/doctest/parts/doctest_fwd.h>
#if WITH_BOOST_PFR
#include <sstream>
#include <boost/pfr/io.hpp>
#endif
#if WITH_RANGELESS_FN
#include <rangeless/include/fn.hpp>
#endif
#ifndef WITH_VALGRIND
#define WITH_VALGRIND 1
#endif
#if defined(__SANITIZE_ADDRESS__) || defined(__SANITIZE_MEMORY__) || defined(_WIN32) ||            \
    !defined(__GNUC__)
#undef WITH_VALGRIND
#endif
#if WITH_VALGRIND
#include <valgrind/include/valgrind/memcheck.h>
#endif

// IMPORTANT: unconditionally enable assertions
#undef NDEBUG
#include <assert.h>

#if (ACC_OS_CYGWIN || ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_EMX || ACC_OS_OS2 || ACC_OS_OS216 ||  \
     ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)
#if defined(INVALID_HANDLE_VALUE) || defined(MAKEWORD) || defined(RT_CURSOR)
#error "something pulled in <windows.h>"
#endif
#endif

#ifdef WANT_WINDOWS_LEAN_H
#if defined(_WIN32) || defined(__CYGWIN__)
#include "util/windows_lean.h"
#endif
#endif

/* vim:set ts=4 sw=4 et: */
