/* membuffer.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2024 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2024 Laszlo Molnar
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

   Markus F.X.J. Oberhumer              Laszlo Molnar
   <markus@oberhumer.com>               <ezerotven+github@gmail.com>
 */

#pragma once

// A MemBuffer allocates memory on the heap, and automatically
// gets destructed when leaving scope or on exceptions.

/*************************************************************************
// MemBufferBase
// provides some base functionality for treating a MemBuffer as a pointer
**************************************************************************/

template <class T>
class MemBufferBase {
public:
    typedef T element_type;
    typedef typename std::add_lvalue_reference<T>::type reference;
    typedef typename std::add_pointer<T>::type pointer;
    typedef unsigned size_type; // limited by UPX_RSIZE_MAX
    typedef pointer iterator;
    typedef typename std::add_pointer<const T>::type const_iterator;
protected:
    static const size_t element_size = sizeof(element_type);

protected:
    pointer ptr;
    size_type size_in_bytes;

public:
    explicit inline MemBufferBase() noexcept : ptr(nullptr), size_in_bytes(0) {}
    forceinline ~MemBufferBase() noexcept {}

    // IMPORTANT NOTE: automatic conversion to underlying pointer
    // HINT: for fully bound-checked pointer use XSPAN_S from xspan.h
    operator pointer() const noexcept { return ptr; }

    // array access
    reference operator[](ptrdiff_t i) const may_throw {
        // NOTE: &array[SIZE] is *not* legal for containers like std::vector and MemBuffer !
        if very_unlikely (i < 0 || mem_size(element_size, i) + element_size > size_in_bytes)
            throwCantPack("MemBuffer invalid array index %td (%u bytes)", i, size_in_bytes);
        return ptr[i];
    }
    // dereference
    reference operator*() const DELETED_FUNCTION;
    // arrow operator
    pointer operator->() const DELETED_FUNCTION;

    iterator begin() const may_throw {
        if very_unlikely (ptr == nullptr)
            throwCantPack("MemBuffer begin() unexpected NULL ptr");
        return ptr;
    }
    const_iterator cbegin() const may_throw {
        if very_unlikely (ptr == nullptr)
            throwCantPack("MemBuffer cbegin() unexpected NULL ptr");
        return ptr;
    }
    iterator end() const may_throw {
        if very_unlikely (ptr == nullptr)
            throwCantPack("MemBuffer end() unexpected NULL ptr");
        return ptr + size_in_bytes / element_size;
    }
    const_iterator cend() const may_throw {
        if very_unlikely (ptr == nullptr)
            throwCantPack("MemBuffer cend() unexpected NULL ptr");
        return ptr + size_in_bytes / element_size;
    }

    // membuffer + n -> pointer
    template <class U>
    typename std::enable_if<std::is_integral<U>::value, pointer>::type operator+(U n) const
        may_throw {
        size_t bytes = mem_size(element_size, n); // check mem_size
        return raw_bytes(bytes) + n;              // and check bytes
    }
private:
    // membuffer - n -> pointer; not allowed - use raw_bytes() if needed
    template <class U>
    typename std::enable_if<std::is_integral<U>::value, pointer>::type operator-(U n) const
        DELETED_FUNCTION;

public: // raw access
    pointer raw_ptr() const noexcept { return ptr; }
    size_type raw_size_in_bytes() const noexcept { return size_in_bytes; }

    pointer raw_bytes(size_t bytes) const may_throw {
        if (bytes > 0) {
            if very_unlikely (ptr == nullptr)
                throwCantPack("MemBuffer raw_bytes unexpected NULL ptr");
            if very_unlikely (bytes > size_in_bytes)
                throwCantPack("MemBuffer raw_bytes invalid size");
        }
        return ptr;
    }

private:
    // disable taking the address => force passing by reference
    // [I'm not too sure about this design decision, but we can always allow it if needed]
    UPX_CXX_DISABLE_ADDRESS(MemBufferBase)
};

/*************************************************************************
// MemBufferBase global overloads
**************************************************************************/

// global operators
#if ALLOW_INT_PLUS_MEMBUFFER
// rewrite "n + membuffer" to "membuffer + n" (member function) so that this will get checked above
template <class T, class U>
inline typename std::enable_if<std::is_integral<U>::value, typename MemBufferBase<T>::pointer>::type
operator+(U n, const MemBufferBase<T> &mbb) {
    return mbb + n;
}
#else
// not allowed
template <class T, class U>
inline typename std::enable_if<std::is_integral<U>::value, typename MemBufferBase<T>::pointer>::type
operator+(U n, const MemBufferBase<T> &mbb) DELETED_FUNCTION;
#endif

// raw_bytes overload
template <class T>
inline typename MemBufferBase<T>::pointer raw_bytes(const MemBufferBase<T> &mbb,
                                                    size_t size_in_bytes) {
    return mbb.raw_bytes(size_in_bytes);
}
template <class T>
inline typename MemBufferBase<T>::pointer raw_index_bytes(const MemBufferBase<T> &mbb, size_t index,
                                                          size_t size_in_bytes) {
    typedef typename MemBufferBase<T>::element_type element_type;
    return mbb.raw_bytes(mem_size(sizeof(element_type), index, size_in_bytes)) + index;
}

// some more global overloads using a checked raw_bytes() call
#if __cplusplus >= 201703L
#define XSPAN_REQUIRES_CONVERTIBLE_ANY_DIRECTION(A, B, RType)                                      \
    std::enable_if_t<std::is_convertible_v<A *, B *> || std::is_convertible_v<B *, A *>, RType>
#elif __cplusplus >= 201103L
#define XSPAN_REQUIRES_CONVERTIBLE_ANY_DIRECTION(A, B, RType)                                      \
    typename std::enable_if<                                                                       \
        std::is_convertible<A *, B *>::value || std::is_convertible<B *, A *>::value, RType>::type
#else
#define XSPAN_REQUIRES_CONVERTIBLE_ANY_DIRECTION(A, B, RType)                                      \
    typename std::enable_if<std::is_same<A, B>::value, RType>::type
#endif
#define C                        MemBufferBase
#define XSPAN_FWD_C_IS_MEMBUFFER 1
#if WITH_XSPAN >= 1
#define D XSPAN_NS(Ptr)
#endif
#include "xspan_fwd.h"
#undef XSPAN_FWD_C_IS_MEMBUFFER
#undef C
#undef D
#undef XSPAN_REQUIRES_CONVERTIBLE_ANY_DIRECTION

/*************************************************************************
// MemBuffer
**************************************************************************/

class MemBuffer final : public MemBufferBase<byte> {
public:
    explicit inline MemBuffer() noexcept : MemBufferBase<byte>() {}
    explicit MemBuffer(upx_uint64_t bytes) may_throw;
    ~MemBuffer() noexcept;

    static unsigned getSizeForCompression(unsigned uncompressed_size, unsigned extra = 0) may_throw;
    static unsigned getSizeForDecompression(unsigned uncompressed_size, unsigned extra = 0)
        may_throw;

    void alloc(upx_uint64_t bytes) may_throw;
    void allocForCompression(unsigned uncompressed_size, unsigned extra = 0) may_throw;
    void allocForDecompression(unsigned uncompressed_size, unsigned extra = 0) may_throw;

    void dealloc() noexcept;
    void checkState() const may_throw;

    // explicit conversion
    void *getVoidPtr() noexcept { return (void *) ptr; }
    const void *getVoidPtr() const noexcept { return (const void *) ptr; }
    unsigned getSizeInBytes() const noexcept { return size_in_bytes; }
    unsigned getSize() const noexcept { return size_in_bytes; } // note: element_size == 1

    // util
    noinline void fill(unsigned off, unsigned len, int value) may_throw;
    forceinline void clear(unsigned off, unsigned len) may_throw { fill(off, len, 0); }
    forceinline void clear() may_throw { fill(0, size_in_bytes, 0); }

    // If the entire range [skip, skip+take) is inside the buffer,
    // then return &ptr[skip]; else throwCantPack(sprintf(errfmt, skip, take)).
    // This is similar to BoundedPtr, except only checks once.
    // skip == offset, take == size_in_bytes
    forceinline pointer subref(const char *errfmt, size_t skip, size_t take) may_throw {
        return (pointer) subref_impl(errfmt, skip, take);
    }

private:
    void *subref_impl(const char *errfmt, size_t skip, size_t take) may_throw;

    // static debug stats
    struct Stats {
        upx_std_atomic(upx_uint32_t) global_alloc_counter;
        upx_std_atomic(upx_uint32_t) global_dealloc_counter;
#if WITH_THREADS
        // avoid link errors on some 32-bit platforms: undefined reference to __atomic_fetch_add_8
        upx_std_atomic(size_t) global_total_bytes; // stats may overflow on 32-bit systems
        upx_std_atomic(size_t) global_total_active_bytes;
#else
        upx_std_atomic(upx_uint64_t) global_total_bytes;
        upx_std_atomic(upx_uint64_t) global_total_active_bytes;
#endif
    };
    static Stats stats;
#if DEBUG
    // debugging aid
    struct Debug {
        void *last_return_address_alloc;
        void *last_return_address_dealloc;
        void *last_return_address_fill;
        void *last_return_address_subref;
        Debug() noexcept { mem_clear(this); }
    };
    Debug debug;
#endif

    UPX_CXX_DISABLE_COPY_MOVE(MemBuffer)             // disable copy and move
    UPX_CXX_DISABLE_NEW_DELETE_NO_VIRTUAL(MemBuffer) // disable dynamic allocation
};

/* vim:set ts=4 sw=4 et: */
