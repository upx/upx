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

SPAN_NAMESPACE_BEGIN

/*************************************************************************
// PtrOrSpan
//
// invariants:
//   ptr != null
//   if base != null then ptr is valid in base
**************************************************************************/

template <class T>
struct PtrOrSpan {
private:
#define CSelf PtrOrSpan
    typedef CSelf<T> Self;
    // core config
    enum { configRequirePtr = true };
    enum { configRequireBase = false };

#include "xspan_impl_common.h"
public:
    // constructors from pointers
    CSelf(pointer first) : ptr(makePtr(first)), base(nullptr), size_in_bytes(0) {
        assertInvariants();
    }

    // constructors
    CSelf(const Self &other)
        : ptr(other.ensurePtr()), base(other.base), size_in_bytes(other.size_in_bytes) {
        assertInvariants();
    }
    template <class U>
    CSelf(const CSelf<U> &other, SPAN_REQUIRES_CONVERTIBLE_A)
        : ptr(other.ensurePtr()), base(other.base), size_in_bytes(other.size_in_bytes) {
        assertInvariants();
    }

    // constructors from Span friends
    template <class U>
    CSelf(const Span<U> &other, SPAN_REQUIRES_CONVERTIBLE_A)
        : ptr(other.ensurePtr()), base(other.base), size_in_bytes(other.size_in_bytes) {
        assertInvariants();
    }
#if SPAN_CONFIG_ENABLE_SPAN_CONVERSION
    template <class U>
    CSelf(const PtrOrSpanOrNull<U> &other, SPAN_REQUIRES_CONVERTIBLE_A)
        : ptr(other.ensurePtr()), base(other.base), size_in_bytes(other.size_in_bytes) {
        assertInvariants();
    }
#endif

    // assignment from Span friends
    // TODO: use Unchecked to avoid double checks in both constructor and assignment
    template <class U>
    SPAN_REQUIRES_CONVERTIBLE_R(Self &)
    operator=(const Span<U> &other) {
        return assign(Self(other));
    }
#if SPAN_CONFIG_ENABLE_SPAN_CONVERSION
    template <class U>
    SPAN_REQUIRES_CONVERTIBLE_R(Self &)
    operator=(const PtrOrSpanOrNull<U> &other) {
        if (other.base == nullptr)
            return assign(Self(other.ptr, size_in_bytes, base));
        return assign(Self(other.ptr, other.size_in_bytes, other.base));
    }
#endif

    // nullptr
    CSelf(std::nullptr_t) SPAN_DELETED_FUNCTION;
    CSelf(std::nullptr_t, SpanSizeInBytes, const void *) SPAN_DELETED_FUNCTION;
    CSelf(std::nullptr_t, SpanCount, const void *) SPAN_DELETED_FUNCTION;
    CSelf(std::nullptr_t, size_type, const void *) SPAN_DELETED_FUNCTION;
    Self &operator=(std::nullptr_t) SPAN_DELETED_FUNCTION;
#if 0
    // don't enable, this prevents generic usage
    bool operator==(std::nullptr_t) const SPAN_DELETED_FUNCTION;
    bool operator!=(std::nullptr_t) const SPAN_DELETED_FUNCTION;
#else
    bool operator==(std::nullptr_t) const {
        assertInvariants();
        return false;
    }
    bool operator!=(std::nullptr_t) const {
        assertInvariants();
        return true;
    }
#endif
#undef CSelf
};

// raw_bytes overload
template <class T>
inline typename PtrOrSpan<T>::pointer raw_bytes(const PtrOrSpan<T> &a, size_t size_in_bytes) {
    return a.raw_bytes(size_in_bytes);
}
template <class T>
inline typename PtrOrSpan<T>::pointer raw_index_bytes(const PtrOrSpan<T> &a, size_t index,
                                                      size_t size_in_bytes) {
    typedef typename PtrOrSpan<T>::element_type element_type;
    return raw_bytes(a, mem_size(sizeof(element_type), index, size_in_bytes)) + index;
}

/*************************************************************************
//
**************************************************************************/

SPAN_NAMESPACE_END

#if !SPAN_CONFIG_ENABLE_IMPLICIT_CONVERSION || 1

#define C SPAN_NS(PtrOrSpan)
#define D SPAN_NS(PtrOrSpanOrNull)
#define E SPAN_NS(Span)
#include "xspan_fwd.h"
#undef C
#undef D
#undef E

#endif

/* vim:set ts=4 sw=4 et: */
