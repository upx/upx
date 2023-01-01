/* bptr.h --

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
#ifndef UPX_BPTR_H__
#define UPX_BPTR_H__ 1

#if WITH_SPAN >= 2
#error "this file is deprecated, please use xspan.h instead"
#endif

/*************************************************************************
// BoundedPtr
**************************************************************************/

template <class T>
class BoundedPtr {
public:
    typedef T element_type;
    typedef typename std::add_pointer<T>::type pointer;

    ~BoundedPtr() {}

    BoundedPtr(void *base, size_t size_in_bytes, T *ptr = nullptr)
        : ptr_(ptr), base_(base), size_in_bytes_(0) {
        assert(base_ != nullptr);
        size_in_bytes_ = mem_size(1, size_in_bytes);
        check();
    }

    // assignment
    BoundedPtr &operator=(const BoundedPtr &other) {
        assert(base_ == other.base_);
        assert(size_in_bytes_ == other.size_in_bytes_);
        ptr_ = other.ptr_;
        check();
        return *this;
    }
    BoundedPtr &operator=(T *other) {
        ptr_ = other;
        check();
        return *this;
    }

    // dereference
    T &operator*() {
        checkNULL();
        checkRange(ptr_ + 1);
        return *ptr_;
    }
    const T &operator*() const {
        checkNULL();
        checkRange(ptr_ + 1);
        return *ptr_;
    }

    operator T *() { return ptr_; }
    operator const T *() const { return ptr_; }

    BoundedPtr &operator+=(size_t n) {
        checkNULL();
        ptr_ += n;
        checkRange();
        return *this;
    }
    BoundedPtr &operator-=(size_t n) {
        checkNULL();
        ptr_ -= n;
        checkRange();
        return *this;
    }
    BoundedPtr &operator++(void) {
        checkNULL();
        ptr_ += 1;
        checkRange();
        return *this;
    }

    T *raw_bytes(size_t bytes) const {
        checkNULL();
        if (bytes > 0)
            checkRange((const char *) (const void *) ptr_ + bytes);
        return ptr_;
    }

private:
    void checkNULL() const {
        if __acc_very_unlikely (!ptr_)
            throwCantUnpack("unexpected NULL pointer; take care!");
    }
    __acc_forceinline void checkRange() const { checkRange(ptr_, base_, size_in_bytes_); }
    __acc_forceinline void checkRange(const void *p) const { checkRange(p, base_, size_in_bytes_); }
    static void checkRange(const void *ptr, const void *base, size_t size_in_bytes) {
        size_t off = (const char *) ptr - (const char *) base;
        if __acc_very_unlikely (off > size_in_bytes)
            throwCantUnpack("pointer out of range; take care!");
    }
    void check() const { // check ptr_ invariant: either NULL or valid checkRange()
        if (ptr_ != nullptr)
            checkRange();
    }

    T *ptr_;
    void *base_;
    size_t size_in_bytes_;

    // disable copy
    BoundedPtr(const BoundedPtr &) = delete;
    // disable dynamic allocation
    ACC_CXX_DISABLE_NEW_DELETE

    // disable taking the address => force passing by reference
    // [I'm not too sure about this design decision, but we can always allow it if needed]
    BoundedPtr<T> *operator&() const = delete;
};

// raw_bytes overload
template <class T>
inline typename BoundedPtr<T>::pointer raw_bytes(const BoundedPtr<T> &a, size_t size_in_bytes) {
    return a.raw_bytes(size_in_bytes);
}
template <class T>
inline typename BoundedPtr<T>::pointer raw_index_bytes(const BoundedPtr<T> &a, size_t index,
                                                       size_t size_in_bytes) {
    typedef typename BoundedPtr<T>::element_type element_type;
    return raw_bytes(a, mem_size(sizeof(element_type), index, size_in_bytes)) + index;
}

#endif /* already included */

/* vim:set ts=4 sw=4 et: */
