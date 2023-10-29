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
// Ptr
**************************************************************************/

template <class T>
struct Ptr {
private:
#define CSelf Ptr
    typedef CSelf<T> Self;

public:
    // befriend all
    template <class>
    friend struct CSelf;

    typedef T element_type;
    typedef typename std::add_lvalue_reference<T>::type reference;
    typedef typename std::add_pointer<T>::type pointer;

private:
    pointer ptr;

    // enforce config invariants at constructor time - static functions
    static inline pointer makePtr(pointer p) { return p; }
    // inverse logic for ensuring valid pointers from existing objects
    inline pointer ensurePtr() const { return ptr; }
    // debug
    forceinline void assertInvariants() const noexcept {}

public:
#if XSPAN_CONFIG_ENABLE_IMPLICIT_CONVERSION || 1
    // Ptr always provides automatic conversion to underlying type because
    // it has limited functionality
    operator pointer() const noexcept { return ptr; }
#endif

#if DEBUG
    ~CSelf() noexcept {
        try {
            invalidate();
        } catch (...) {
            std::terminate();
        }
    }
#else
    forceinline ~CSelf() noexcept {}
#endif
    noinline void invalidate() {
        assertInvariants();
        ptr_invalidate_and_poison(ptr); // point to non-null invalid address
        assertInvariants();
    }
    inline CSelf() { assertInvariants(); }

    // constructors from pointers
    CSelf(pointer p) : ptr(makePtr(p)) { assertInvariants(); }
    template <class U>
    CSelf(U *p, XSPAN_REQUIRES_CONVERTIBLE_A) : ptr(makePtr(p)) {
        assertInvariants();
    }

    // constructors
    CSelf(const Self &other) : ptr(other.ptr) { assertInvariants(); }
    template <class U>
    CSelf(const CSelf<U> &other, XSPAN_REQUIRES_CONVERTIBLE_A) : ptr(other.ptr) {
        assertInvariants();
    }

    Self &assign(const Self &other) {
        assertInvariants();
        other.assertInvariants();
        ptr = other.ptr;
        assertInvariants();
        return *this;
    }

    // assignment
    Self &operator=(const Self &other) { return assign(other); }

    // FIXME: this is not called??
    template <class U>
    XSPAN_REQUIRES_CONVERTIBLE_R(Self &)
    operator=(U *other) {
        // assert(0);
        return assign(Self(other));
    }

    // FIXME: this is not called??
    template <class U>
    XSPAN_REQUIRES_CONVERTIBLE_R(Self &)
    operator=(const CSelf<U> &other) {
        // assert(0);
        return assign(Self(other));
    }

    template <class U>
    inline CSelf<U> type_cast() const {
        typedef CSelf<U> R;
        typedef typename R::pointer rpointer;
        return R(upx::ptr_reinterpret_cast<rpointer>(ptr));
    }

    // comparison

    bool operator==(pointer other) const noexcept { return ptr == other; }
    template <class U>
    XSPAN_REQUIRES_CONVERTIBLE_R(bool)
    operator==(U *other) const noexcept {
        return ptr == other;
    }
    template <class U>
    XSPAN_REQUIRES_CONVERTIBLE_R(bool)
    operator==(const Ptr<U> &other) const noexcept {
        return ptr == other.ptr;
    }

    // dereference
    reference operator*() const { return *check_deref(ptr); }

    // array access
    reference operator[](ptrdiff_t i) const { return *check_deref(ptr, i); }

    // arrow operator
    pointer operator->() const { return check_deref(ptr); }

    Self &operator++() {
        ptr = check_add(ptr, 1);
        return *this;
    }
    Self operator++(int) {
        Self tmp = *this;
        ++*this;
        return tmp;
    }
    Self &operator--() {
        ptr = check_add(ptr, -1);
        return *this;
    }
    Self operator--(int) {
        Self tmp = *this;
        --*this;
        return tmp;
    }

    Self &operator+=(ptrdiff_t n) {
        ptr = check_add(ptr, n);
        return *this;
    }
    Self &operator-=(ptrdiff_t n) {
        ptr = check_add(ptr, -n);
        return *this;
    }

    Self operator+(ptrdiff_t n) const {
        pointer p = check_add(ptr, n);
        return Self(p);
    }
    Self operator-(ptrdiff_t n) const {
        pointer p = check_add(ptr, -n);
        return Self(p);
    }

#ifdef UPX_VERSION_HEX
    CSelf(MemBuffer &mb) : ptr((pointer) membuffer_get_void_ptr(mb)) {}
    Self &operator=(MemBuffer &mb) { return assign(Self(mb)); }
#endif

private:
    static forceinline pointer check_deref(pointer p) noexcept { return p; }
    static forceinline pointer check_deref(pointer p, ptrdiff_t n) noexcept { return p + n; }
    static forceinline pointer check_add(pointer p, ptrdiff_t n) noexcept { return p + n; }

public: // raw access
    pointer raw_ptr() const noexcept { return ptr; }

    pointer raw_bytes(size_t bytes) const {
        assertInvariants();
        if (bytes > 0) {
            if __acc_cte (ptr == nullptr)
                xspan_fail_nullptr();
        }
        return ptr;
    }

#undef CSelf
};

// raw_bytes overload
template <class T>
inline typename Ptr<T>::pointer raw_bytes(const Ptr<T> &a, size_t size_in_bytes) {
    return a.raw_bytes(size_in_bytes);
}
template <class T>
inline typename Ptr<T>::pointer raw_index_bytes(const Ptr<T> &a, size_t index,
                                                size_t size_in_bytes) {
    typedef typename Ptr<T>::element_type element_type;
    if very_unlikely (a.raw_ptr() == nullptr)
        throwInternalError("raw_index_bytes unexpected NULL ptr");
    return a.raw_bytes(mem_size(sizeof(element_type), index, size_in_bytes)) + index;
}

/*************************************************************************
//
**************************************************************************/

XSPAN_NAMESPACE_END

#if 1

#define C XSPAN_NS(Ptr)
template <class T>
class MemBufferBase;
#define D MemBufferBase
#include "xspan_fwd.h"
#undef C
#undef D

#endif

/* vim:set ts=4 sw=4 et: */
