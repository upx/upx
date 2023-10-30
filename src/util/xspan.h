/* xspan -- a minimally invasive checked memory smart pointer

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

/*************************************************************************
// config and implementation
**************************************************************************/

#ifndef WITH_XSPAN
#define WITH_XSPAN 2
#endif

#if WITH_XSPAN

// automatic conversion to underlying pointer; do NOT enable this config as this
// defeats the main purpose of a checked pointer => use raw_bytes() as needed;
// and see xspan_fwd.h how to make this more convenient
#ifndef XSPAN_CONFIG_ENABLE_IMPLICIT_CONVERSION
#define XSPAN_CONFIG_ENABLE_IMPLICIT_CONVERSION 0
#endif
// allow automatic conversion PtrOrSpanOrNull => PtrOrSpan => Span (with run-time checks)
// choose between compile-time safety vs. possible run-time exceptions
#ifndef XSPAN_CONFIG_ENABLE_SPAN_CONVERSION
#define XSPAN_CONFIG_ENABLE_SPAN_CONVERSION 1
#endif

#include "xspan_impl.h"

#ifdef XSPAN_NAMESPACE_NAME
// types to help the constructor to distinguish between number of elements and bytes
using XSPAN_NAMESPACE_NAME::XSpanCount;
using XSPAN_NAMESPACE_NAME::XSpanSizeInBytes;
// actual classes
using XSPAN_NAMESPACE_NAME::Ptr;
using XSPAN_NAMESPACE_NAME::PtrOrSpan;
using XSPAN_NAMESPACE_NAME::PtrOrSpanOrNull;
using XSPAN_NAMESPACE_NAME::Span;
// support functions
using XSPAN_NAMESPACE_NAME::raw_bytes;       // overloaded for all classes
using XSPAN_NAMESPACE_NAME::raw_index_bytes; // overloaded for all classes
#endif

#endif // WITH_XSPAN

/*************************************************************************
// usage
//
// PtrOrSpanOrNull  invariants: ptr is checked if ptr != null && base != null
// PtrOrSpan        invariants: ptr is checked if base != null; ptr != null
// Span             invariants: ptr is checked; ptr != null; base != null
//
// Ptr              invariants: none; this is just a no-op pointer wrapper
**************************************************************************/

#if WITH_XSPAN >= 2

// fully checked

// types
#define XSPAN_0(type) PtrOrSpanOrNull<type>
#define XSPAN_P(type) PtrOrSpan<type>
#define XSPAN_S(type) Span<type>

// create a value
#define XSPAN_0_MAKE(type, first, ...) (XSPAN_0(type)((first), ##__VA_ARGS__))
#define XSPAN_P_MAKE(type, first, ...) (XSPAN_P(type)((first), ##__VA_ARGS__))
#define XSPAN_S_MAKE(type, first, ...) (XSPAN_S(type)((first), ##__VA_ARGS__))

// define a variable
#define XSPAN_0_VAR(type, var, first, ...) XSPAN_0(type) var((first), ##__VA_ARGS__)
#define XSPAN_P_VAR(type, var, first, ...) XSPAN_P(type) var((first), ##__VA_ARGS__)
#define XSPAN_S_VAR(type, var, first, ...) XSPAN_S(type) var((first), ##__VA_ARGS__)

// cast to a different type (creates a new value)
#define XSPAN_TYPE_CAST(type, x) ((x).type_cast<type>())
// poison a pointer: point to a non-null invalid address
#define XSPAN_INVALIDATE(x)      ((x).invalidate())

#elif WITH_XSPAN >= 1

// unchecked - just a no-op pointer wrapper, no extra functionality

// types
#define XSPAN_0(type)                      Ptr<type>
#define XSPAN_P(type)                      Ptr<type>
#define XSPAN_S(type)                      Ptr<type>

// create a value
#define XSPAN_0_MAKE(type, first, ...)     (XSPAN_0(type)((first)))
#define XSPAN_P_MAKE(type, first, ...)     (XSPAN_P(type)((first)))
#define XSPAN_S_MAKE(type, first, ...)     (XSPAN_S(type)((first)))

// define a variable
#define XSPAN_0_VAR(type, var, first, ...) XSPAN_0(type) var((first))
#define XSPAN_P_VAR(type, var, first, ...) XSPAN_P(type) var((first))
#define XSPAN_S_VAR(type, var, first, ...) XSPAN_S(type) var((first))

// cast to a different type (creates a new value)
#define XSPAN_TYPE_CAST(type, x)           ((x).type_cast<type>())
// poison a pointer: point to a non-null invalid address
#define XSPAN_INVALIDATE(x)                ((x).invalidate())

#else // WITH_XSPAN

// unchecked regular pointers

// helper for implicit pointer conversions and MemBuffer overloads
template <class R, class T>
inline R *xspan_make_helper__(T *first) noexcept {
    return first; // IMPORTANT: no cast here to detect bad usage
}
template <class R>
inline R *xspan_make_helper__(std::nullptr_t /*first*/) noexcept {
    return nullptr;
}
template <class R>
inline R *xspan_make_helper__(MemBuffer &mb) noexcept {
    return (R *) membuffer_get_void_ptr(mb);
}

// types
#define XSPAN_0(type)                      type *
#define XSPAN_P(type)                      type *
#define XSPAN_S(type)                      type *

// create a value
#define XSPAN_0_MAKE(type, first, ...)     (xspan_make_helper__<type>((first)))
#define XSPAN_P_MAKE(type, first, ...)     (xspan_make_helper__<type>((first)))
#define XSPAN_S_MAKE(type, first, ...)     (xspan_make_helper__<type>((first)))

// define a variable
#define XSPAN_0_VAR(type, var, first, ...) type *var = XSPAN_0_MAKE(type, (first))
#define XSPAN_P_VAR(type, var, first, ...) type *var = XSPAN_P_MAKE(type, (first))
#define XSPAN_S_VAR(type, var, first, ...) type *var = XSPAN_S_MAKE(type, (first))

// cast to a different type (creates a new value)
#define XSPAN_TYPE_CAST(type, x)           (upx::ptr_reinterpret_cast<type *>(x))
// poison a pointer: point to a non-null invalid address
#define XSPAN_INVALIDATE(x)                ptr_invalidate_and_poison(x)

#endif // WITH_XSPAN

/*************************************************************************
// nicer names
**************************************************************************/

#if 1
// types
#define SPAN_0          XSPAN_0
#define SPAN_P          XSPAN_P
#define SPAN_S          XSPAN_S
// create a value
#define SPAN_0_MAKE     XSPAN_0_MAKE
#define SPAN_P_MAKE     XSPAN_P_MAKE
#define SPAN_S_MAKE     XSPAN_S_MAKE
// define a variable
#define SPAN_0_VAR      XSPAN_0_VAR
#define SPAN_P_VAR      XSPAN_P_VAR
#define SPAN_S_VAR      XSPAN_S_VAR
// cast to a different type (creates a new value)
#define SPAN_TYPE_CAST  XSPAN_TYPE_CAST
// poison a pointer: point to a non-null invalid address
#define SPAN_INVALIDATE XSPAN_INVALIDATE
#endif

/* vim:set ts=4 sw=4 et: */
