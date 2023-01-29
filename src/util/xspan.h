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
// choose between compile-time safety vs. possible run-time errors
#ifndef XSPAN_CONFIG_ENABLE_SPAN_CONVERSION
#define XSPAN_CONFIG_ENABLE_SPAN_CONVERSION 1
#endif

#include "xspan_impl.h"

#ifdef XSPAN_NAMESPACE_NAME
// help constructor to distinguish between number of elements and bytes
using XSPAN_NAMESPACE_NAME::XSpanCount;
using XSPAN_NAMESPACE_NAME::XSpanSizeInBytes;
// actual classes
using XSPAN_NAMESPACE_NAME::Ptr;
using XSPAN_NAMESPACE_NAME::PtrOrSpan;
using XSPAN_NAMESPACE_NAME::PtrOrSpanOrNull;
using XSPAN_NAMESPACE_NAME::Span;
// util
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

#define XSPAN_0(type) PtrOrSpanOrNull<type>
#define XSPAN_P(type) PtrOrSpan<type>
#define XSPAN_S(type) Span<type>

// create a value
#define XSPAN_0_MAKE(type, first, ...) (XSPAN_0(type)(first, ##__VA_ARGS__))
#define XSPAN_P_MAKE(type, first, ...) (XSPAN_P(type)(first, ##__VA_ARGS__))
#define XSPAN_S_MAKE(type, first, ...) (XSPAN_S(type)(first, ##__VA_ARGS__))

// define a variable
#define XSPAN_0_VAR(type, var, first, ...) XSPAN_0(type) var(first, ##__VA_ARGS__)
#define XSPAN_P_VAR(type, var, first, ...) XSPAN_P(type) var(first, ##__VA_ARGS__)
#define XSPAN_S_VAR(type, var, first, ...) XSPAN_S(type) var(first, ##__VA_ARGS__)

#elif WITH_XSPAN >= 1

// unchecked - just a no-op pointer wrapper, no extra functionality

#define XSPAN_0(type) Ptr<type>
#define XSPAN_P(type) Ptr<type>
#define XSPAN_S(type) Ptr<type>

// create a value
#define XSPAN_0_MAKE(type, first, ...) (XSPAN_0(type)(first))
#define XSPAN_P_MAKE(type, first, ...) (XSPAN_P(type)(first))
#define XSPAN_S_MAKE(type, first, ...) (XSPAN_S(type)(first))

// define a variable
#define XSPAN_0_VAR(type, var, first, ...) XSPAN_0(type) var(first)
#define XSPAN_P_VAR(type, var, first, ...) XSPAN_P(type) var(first)
#define XSPAN_S_VAR(type, var, first, ...) XSPAN_S(type) var(first)

#else // WITH_XSPAN

// unchecked regular pointers

// helper for implicit pointer conversions and MemBuffer overloads
template <class R, class T>
inline R *xspan_make_helper__(R * /*dummy*/, T *first) {
    return first; // IMPORTANT: no cast here to detect bad usage
}
template <class R>
inline R *xspan_make_helper__(R * /*dummy*/, std::nullptr_t /*first*/) {
    return nullptr;
}
template <class R>
inline R *xspan_make_helper__(R * /*dummy*/, MemBuffer &first) {
    return (R *) membuffer_get_void_ptr(first);
}

#define XSPAN_0(type) type *
#define XSPAN_P(type) type *
#define XSPAN_S(type) type *

// create a value
#define XSPAN_0_MAKE(type, first, ...) (xspan_make_helper__((type *) nullptr, first))
#define XSPAN_P_MAKE(type, first, ...) (xspan_make_helper__((type *) nullptr, first))
#define XSPAN_S_MAKE(type, first, ...) (xspan_make_helper__((type *) nullptr, first))

// define a variable
#define XSPAN_0_VAR(type, var, first, ...) type *var = XSPAN_0_MAKE(type, first)
#define XSPAN_P_VAR(type, var, first, ...) type *var = XSPAN_P_MAKE(type, first)
#define XSPAN_S_VAR(type, var, first, ...) type *var = XSPAN_S_MAKE(type, first)

#endif // WITH_XSPAN

#if 1
// nicer names
#define SPAN_0 XSPAN_0
#define SPAN_P XSPAN_P
#define SPAN_S XSPAN_S
#define SPAN_0_MAKE XSPAN_0_MAKE
#define SPAN_P_MAKE XSPAN_P_MAKE
#define SPAN_S_MAKE XSPAN_S_MAKE
#define SPAN_0_VAR XSPAN_0_VAR
#define SPAN_P_VAR XSPAN_P_VAR
#define SPAN_S_VAR XSPAN_S_VAR
#endif

/*************************************************************************
// raw_bytes() - get underlying memory from checked buffers/pointers.
// This is overloaded by various utility classes like BoundedPtr,
// MemBuffer and XSpan.
//
// Note that the pointer type is retained, the "_bytes" hints size_in_bytes
**************************************************************************/

// default: for any regular pointer, raw_bytes() is just the pointer itself
template <class T>
inline
    typename std::enable_if<std::is_pointer<T>::value && !std_is_bounded_array<T>::value, T>::type
    raw_bytes(T ptr, size_t size_in_bytes) {
    if (size_in_bytes > 0) {
        if very_unlikely (ptr == nullptr)
            throwInternalError("raw_bytes unexpected NULL ptr");
        if very_unlikely (__acc_cte(VALGRIND_CHECK_MEM_IS_ADDRESSABLE(ptr, size_in_bytes) != 0))
            throwInternalError("raw_bytes valgrind-check-mem");
    }
    return ptr;
}

// default: for any regular pointer, raw_index_bytes() is just "pointer + index"
// NOTE: index == number of elements, *NOT* size in bytes!
template <class T>
inline
    typename std::enable_if<std::is_pointer<T>::value && !std_is_bounded_array<T>::value, T>::type
    raw_index_bytes(T ptr, size_t index, size_t size_in_bytes) {
    typedef typename std::remove_pointer<T>::type element_type;
    if very_unlikely (ptr == nullptr)
        throwInternalError("raw_index_bytes unexpected NULL ptr");
    size_in_bytes = mem_size(sizeof(element_type), index, size_in_bytes); // assert size
    if very_unlikely (__acc_cte(VALGRIND_CHECK_MEM_IS_ADDRESSABLE(ptr, size_in_bytes) != 0))
        throwInternalError("raw_index_bytes valgrind-check-mem");
    UNUSED(size_in_bytes);
    return ptr + index;
}

// same for bounded arrays
template <class T, size_t N>
inline T *raw_bytes(T (&a)[N], size_t size_in_bytes) {
    typedef T element_type;
    if very_unlikely (size_in_bytes > mem_size(sizeof(element_type), N))
        throwInternalError("raw_bytes out of range");
    return a;
}

template <class T, size_t N>
inline T *raw_index_bytes(T (&a)[N], size_t index, size_t size_in_bytes) {
    typedef T element_type;
    return raw_bytes(a, mem_size(sizeof(element_type), index, size_in_bytes)) + index;
}

/* vim:set ts=4 sw=4 et: */
