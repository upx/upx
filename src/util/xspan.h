/* xspan -- a minimally invasive checked memory smart pointer

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2022 Markus Franz Xaver Johannes Oberhumer
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

/*************************************************************************
// config and implementation
**************************************************************************/

#ifndef WITH_SPAN
#define WITH_SPAN 2
#endif

#if WITH_SPAN

// automatic conversion to underlying pointer; do NOT enable this config as this
// defeats the main purpose of a checked pointer => use raw_bytes() as needed;
// and see xspan_fwd.h how to make this more convenient
#ifndef SPAN_CONFIG_ENABLE_IMPLICIT_CONVERSION
#define SPAN_CONFIG_ENABLE_IMPLICIT_CONVERSION 0
#endif
// allow automatic conversion PtrOrSpanOrNull => PtrOrSpan => Span (with runtime checks)
// choose between compile-time safety vs. possible run-time errors
#ifndef SPAN_CONFIG_ENABLE_SPAN_CONVERSION
#define SPAN_CONFIG_ENABLE_SPAN_CONVERSION 1
#endif

#include "xspan_impl.h"

#ifdef SPAN_NAMESPACE_NAME
// help constructor to distinguish between number of elements and bytes
using SPAN_NAMESPACE_NAME::SpanCount;
using SPAN_NAMESPACE_NAME::SpanSizeInBytes;
// actual classes
using SPAN_NAMESPACE_NAME::Ptr;
using SPAN_NAMESPACE_NAME::PtrOrSpan;
using SPAN_NAMESPACE_NAME::PtrOrSpanOrNull;
using SPAN_NAMESPACE_NAME::Span;
// util
using SPAN_NAMESPACE_NAME::raw_bytes;       // overloaded for all classes
using SPAN_NAMESPACE_NAME::raw_index_bytes; // overloaded for all classes
#endif

#endif // WITH_SPAN

/*************************************************************************
// usage
//
// PtrOrSpanOrNull  invariants: ptr is checked if ptr != null && base != null
// PtrOrSpan        invariants: ptr is checked if base != null; ptr != null
// Span             invariants: ptr is checked; ptr != null; base != null
//
// Ptr              invariants: none; this is just a no-op pointer wrapper
**************************************************************************/

#if WITH_SPAN >= 2

// fully checked

#define SPAN_0(type) PtrOrSpanOrNull<type>
#define SPAN_P(type) PtrOrSpan<type>
#define SPAN_S(type) Span<type>

// create a value
#define SPAN_0_MAKE(type, first, ...) (SPAN_0(type)(first, ##__VA_ARGS__))
#define SPAN_P_MAKE(type, first, ...) (SPAN_P(type)(first, ##__VA_ARGS__))
#define SPAN_S_MAKE(type, first, ...) (SPAN_S(type)(first, ##__VA_ARGS__))

// define a variable
#define SPAN_0_VAR(type, var, first, ...) SPAN_0(type) var(first, ##__VA_ARGS__)
#define SPAN_P_VAR(type, var, first, ...) SPAN_P(type) var(first, ##__VA_ARGS__)
#define SPAN_S_VAR(type, var, first, ...) SPAN_S(type) var(first, ##__VA_ARGS__)

#elif WITH_SPAN >= 1

// unchecked - just a no-op pointer wrapper, no extra functionality

#define SPAN_0(type) Ptr<type>
#define SPAN_P(type) Ptr<type>
#define SPAN_S(type) Ptr<type>

// create a value
#define SPAN_0_MAKE(type, first, ...) (SPAN_0(type)(first))
#define SPAN_P_MAKE(type, first, ...) (SPAN_P(type)(first))
#define SPAN_S_MAKE(type, first, ...) (SPAN_S(type)(first))

// define a variable
#define SPAN_0_VAR(type, var, first, ...) SPAN_0(type) var(first)
#define SPAN_P_VAR(type, var, first, ...) SPAN_P(type) var(first)
#define SPAN_S_VAR(type, var, first, ...) SPAN_S(type) var(first)

#else

// unchecked regular pointers

// helper for implicit pointer conversions and MemBuffer overloads
template <class R, class T>
inline R *span_make_helper__(R * /*dummy*/, T *first) {
    return first; // IMPORTANT: no cast here to detect bad usage
}
template <class R>
inline R *span_make_helper__(R * /*dummy*/, std::nullptr_t /*first*/) {
    return nullptr;
}
template <class R>
inline R *span_make_helper__(R * /*dummy*/, MemBuffer &first) {
    return (R *) membuffer_get_void_ptr(first);
}

#define SPAN_0(type) type *
#define SPAN_P(type) type *
#define SPAN_S(type) type *

// create a value
#define SPAN_0_MAKE(type, first, ...) (span_make_helper__((type *) nullptr, first))
#define SPAN_P_MAKE(type, first, ...) (span_make_helper__((type *) nullptr, first))
#define SPAN_S_MAKE(type, first, ...) (span_make_helper__((type *) nullptr, first))

// define a variable
#define SPAN_0_VAR(type, var, first, ...) type *var = SPAN_0_MAKE(type, first)
#define SPAN_P_VAR(type, var, first, ...) type *var = SPAN_P_MAKE(type, first)
#define SPAN_S_VAR(type, var, first, ...) type *var = SPAN_S_MAKE(type, first)

#endif // WITH_SPAN

/* vim:set ts=4 sw=4 et: */
