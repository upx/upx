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

/*************************************************************************
// A MemBuffer allocates memory on the heap, and automatically
// gets destructed when leaving scope or on exceptions.
**************************************************************************/

// provides some base functionality for treating a MemBuffer as a pointer
template <class T>
class MemBufferBase {
public:
    typedef T element_type;
    typedef typename std::add_lvalue_reference<T>::type reference;
    typedef typename std::add_pointer<T>::type pointer;
    typedef unsigned size_type;

protected:
    pointer ptr;
    size_type size_in_bytes;

public:
    MemBufferBase() noexcept : ptr(nullptr), size_in_bytes(0) {}
    inline ~MemBufferBase() noexcept {}

    // NOTE: implicit conversion to underlying pointer
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
};

class MemBuffer final : public MemBufferBase<byte> {
public:
    MemBuffer() : MemBufferBase<byte>() {}
    explicit MemBuffer(upx_uint64_t bytes);
    ~MemBuffer() noexcept;

    static unsigned getSizeForCompression(unsigned uncompressed_size, unsigned extra = 0);
    static unsigned getSizeForDecompression(unsigned uncompressed_size, unsigned extra = 0);

    void alloc(upx_uint64_t bytes);
    void allocForCompression(unsigned uncompressed_size, unsigned extra = 0);
    void allocForDecompression(unsigned uncompressed_size, unsigned extra = 0);

    void dealloc() noexcept;
    void checkState() const;
    unsigned getSize() const { return size_in_bytes; }

    // explicit conversion
    void *getVoidPtr() { return (void *) ptr; }
    const void *getVoidPtr() const { return (const void *) ptr; }

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
        upx_std_atomic(upx_uint64_t) global_total_bytes;
        upx_std_atomic(upx_uint64_t) global_total_active_bytes;
    };
    static Stats stats;
#if DEBUG
    // debugging aid
    struct Debug {
        void *last_return_address_alloc;
        void *last_return_address_dealloc;
        void *last_return_address_fill;
        void *last_return_address_subref;
        Debug() { memset(this, 0, sizeof(*this)); }
    };
    Debug debug;
#endif

    // disable copy and move
    MemBuffer(const MemBuffer &) DELETED_FUNCTION;
    MemBuffer &operator=(const MemBuffer &) DELETED_FUNCTION;
#if __cplusplus >= 201103L
    MemBuffer(MemBuffer &&) DELETED_FUNCTION;
    MemBuffer &operator=(MemBuffer &&) DELETED_FUNCTION;
#endif
    // disable dynamic allocation
    ACC_CXX_DISABLE_NEW_DELETE

    // disable taking the address => force passing by reference
    // [I'm not too sure about this design decision, but we can always allow it if needed]
    MemBuffer *operator&() const DELETED_FUNCTION;
};

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

// global operators
#if ALLOW_INT_PLUS_MEMBUFFER
// rewrite "n + membuffer" to "membuffer + n" so that this will get checked above
template <class T, class U>
inline typename std::enable_if<std::is_integral<U>::value, typename MemBufferBase<T>::pointer>::type
operator+(U n, const MemBufferBase<T> &mbb) {
    return mbb + n;
}
#else
template <class T, class U>
inline typename std::enable_if<std::is_integral<U>::value, typename MemBufferBase<T>::pointer>::type
operator+(U n, const MemBufferBase<T> &mbb) DELETED_FUNCTION;
#endif

/* vim:set ts=4 sw=4 et: */
