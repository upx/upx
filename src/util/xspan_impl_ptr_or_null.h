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

SPAN_NAMESPACE_BEGIN

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
    enum { configRequirePtr = false };
    enum { configRequireBase = false };

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
    CSelf(const CSelf<U> &other, SPAN_REQUIRES_CONVERTIBLE_A)
        : ptr(other.ptr), base(other.base), size_in_bytes(other.size_in_bytes) {
        assertInvariants();
    }

    // constructors from Span friends
    template <class U>
    CSelf(const PtrOrSpan<U> &other, SPAN_REQUIRES_CONVERTIBLE_A)
        : ptr(other.ptr), base(other.base), size_in_bytes(other.size_in_bytes) {
        assertInvariants();
    }
    template <class U>
    CSelf(const Span<U> &other, SPAN_REQUIRES_CONVERTIBLE_A)
        : ptr(other.ptr), base(other.base), size_in_bytes(other.size_in_bytes) {
        assertInvariants();
    }

    // assignment from Span friends
    template <class U>
    SPAN_REQUIRES_CONVERTIBLE_R(Self &)
    operator=(const PtrOrSpan<U> &other) {
        return assign(Self(other));
    }
    template <class U>
    SPAN_REQUIRES_CONVERTIBLE_R(Self &)
    operator=(const Span<U> &other) {
        return assign(Self(other));
    }

    // nullptr
    CSelf(std::nullptr_t) : ptr(nullptr), base(nullptr), size_in_bytes(0) {}
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
    return raw_bytes(a, mem_size(sizeof(element_type), index, size_in_bytes)) + index;
}

/*************************************************************************
//
**************************************************************************/

SPAN_NAMESPACE_END

#if !SPAN_CONFIG_ENABLE_IMPLICIT_CONVERSION || 1

#define C SPAN_NS(PtrOrSpanOrNull)
#define D SPAN_NS(PtrOrSpan)
#define E SPAN_NS(Span)
#include "xspan_fwd.h"
#undef C
#undef D
#undef E

#endif

/* vim:set ts=4 sw=4 et: */
