/* membuffer.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2023 Laszlo Molnar
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

protected:
    pointer ptr;
    size_type size_in_bytes;

public:
    explicit inline MemBufferBase() noexcept : ptr(nullptr), size_in_bytes(0) {}
    forceinline ~MemBufferBase() noexcept {}

    // IMPORTANT NOTE: automatic conversion to underlying pointer
    // HINT: for fully bound-checked pointer use XSPAN_S from xspan.h
    operator pointer() const noexcept { return ptr; }

    // membuffer + n -> pointer
    template <class U>
    typename std::enable_if<std::is_integral<U>::value, pointer>::type operator+(U n) const {
        size_t bytes = mem_size(sizeof(T), n); // check mem_size
        return raw_bytes(bytes) + n;           // and check bytes
    }
private:
    // membuffer - n -> pointer; not allowed - use raw_bytes() if needed
    template <class U>
    typename std::enable_if<std::is_integral<U>::value, pointer>::type operator-(U n) const
        DELETED_FUNCTION;

public: // raw access
    pointer raw_ptr() const noexcept { return ptr; }
    size_type raw_size_in_bytes() const noexcept { return size_in_bytes; }

    pointer raw_bytes(size_t bytes) const {
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
    MemBufferBase<T> *operator&() const noexcept DELETED_FUNCTION;
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
    explicit MemBuffer(upx_uint64_t bytes);
    ~MemBuffer() noexcept;

    static unsigned getSizeForCompression(unsigned uncompressed_size, unsigned extra = 0);
    static unsigned getSizeForDecompression(unsigned uncompressed_size, unsigned extra = 0);

    void alloc(upx_uint64_t bytes);
    void allocForCompression(unsigned uncompressed_size, unsigned extra = 0);
    void allocForDecompression(unsigned uncompressed_size, unsigned extra = 0);

    void dealloc() noexcept;
    void checkState() const;
    unsigned getSize() const noexcept { return size_in_bytes; }

    // explicit conversion
    void *getVoidPtr() noexcept { return (void *) ptr; }
    const void *getVoidPtr() const noexcept { return (const void *) ptr; }

    // util
    void fill(unsigned off, unsigned len, int value);
    forceinline void clear(unsigned off, unsigned len) { fill(off, len, 0); }
    forceinline void clear() { fill(0, size_in_bytes, 0); }

    // If the entire range [skip, skip+take) is inside the buffer,
    // then return &ptr[skip]; else throwCantPack(sprintf(errfmt, skip, take)).
    // This is similar to BoundedPtr, except only checks once.
    // skip == offset, take == size_in_bytes
    forceinline pointer subref(const char *errfmt, size_t skip, size_t take) {
        return (pointer) subref_impl(errfmt, skip, take);
    }

private:
    void *subref_impl(const char *errfmt, size_t skip, size_t take);

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

    // disable copy and move
    MemBuffer(const MemBuffer &) DELETED_FUNCTION;
    MemBuffer &operator=(const MemBuffer &) DELETED_FUNCTION;
#if __cplusplus >= 201103L
    MemBuffer(MemBuffer &&) noexcept DELETED_FUNCTION;
    MemBuffer &operator=(MemBuffer &&) noexcept DELETED_FUNCTION;
#endif
    // disable dynamic allocation
    ACC_CXX_DISABLE_NEW_DELETE
};

/* vim:set ts=4 sw=4 et: */
