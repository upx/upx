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

XSPAN_NAMESPACE_BEGIN

/*************************************************************************
// PtrOrSpanOrNull
//
// invariants:
//   ptr can be null
//   if ptr != null && base != null then ptr is valid in base
**************************************************************************/

template <class T>
struct PtrOrSpanOrNull {
private:
#define CSelf PtrOrSpanOrNull
    typedef CSelf<T> Self;
    // core config
#if __cplusplus >= 201103L
    static constexpr bool configRequirePtr = false;
    static constexpr bool configRequireBase = false;
#else
    enum { configRequirePtr = false };
    enum { configRequireBase = false };
#endif

#include "xspan_impl_common.h"
public:
    // constructors from pointers
    CSelf(pointer first) : ptr(first), base(nullptr), size_in_bytes(0) { assertInvariants(); }

    // constructors
    CSelf(const Self &other)
        : ptr(other.ptr), base(other.base), size_in_bytes(other.size_in_bytes) {
        assertInvariants();
    }
    template <class U>
    CSelf(const CSelf<U> &other, XSPAN_REQUIRES_CONVERTIBLE_A)
        : ptr(other.ptr), base(other.base), size_in_bytes(other.size_in_bytes) {
        assertInvariants();
    }

    // constructors from Span friends
    template <class U>
    CSelf(const PtrOrSpan<U> &other, XSPAN_REQUIRES_CONVERTIBLE_A)
        : ptr(other.ptr), base(other.base), size_in_bytes(other.size_in_bytes) {
        assertInvariants();
    }
    template <class U>
    CSelf(const Span<U> &other, XSPAN_REQUIRES_CONVERTIBLE_A)
        : ptr(other.ptr), base(other.base), size_in_bytes(other.size_in_bytes) {
        assertInvariants();
    }

    // assignment from Span friends
    template <class U>
    XSPAN_REQUIRES_CONVERTIBLE_R(Self &)
    operator=(const PtrOrSpan<U> &other) {
        return assign(Self(other));
    }
    template <class U>
    XSPAN_REQUIRES_CONVERTIBLE_R(Self &)
    operator=(const Span<U> &other) {
        return assign(Self(other));
    }

    // nullptr
    forceinline CSelf(std::nullptr_t) noexcept : ptr(nullptr), base(nullptr), size_in_bytes(0) {}
    forceinline Self &operator=(std::nullptr_t) noexcept {
        ptr = nullptr;
        return *this;
    }
#undef CSelf
};

// raw_bytes overload
template <class T>
inline typename PtrOrSpanOrNull<T>::pointer raw_bytes(const PtrOrSpanOrNull<T> &a,
                                                      size_t size_in_bytes) {
    return a.raw_bytes(size_in_bytes);
}
template <class T>
inline typename PtrOrSpanOrNull<T>::pointer raw_index_bytes(const PtrOrSpanOrNull<T> &a,
                                                            size_t index, size_t size_in_bytes) {
    typedef typename PtrOrSpanOrNull<T>::element_type element_type;
    if very_unlikely (a.raw_ptr() == nullptr)
        throwInternalError("raw_index_bytes unexpected NULL ptr");
    return a.raw_bytes(mem_size(sizeof(element_type), index, size_in_bytes)) + index;
}

/*************************************************************************
//
**************************************************************************/

XSPAN_NAMESPACE_END

#if !XSPAN_CONFIG_ENABLE_IMPLICIT_CONVERSION || 1

#define C XSPAN_NS(PtrOrSpanOrNull)
#define D XSPAN_NS(PtrOrSpan)
#define E XSPAN_NS(Span)
#include "xspan_fwd.h"
#undef C
#undef D
#undef E

#endif

/* vim:set ts=4 sw=4 et: */
