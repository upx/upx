/* bptr.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2016 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2016 Laszlo Molnar
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


#ifndef __UPX_BPTR_H
#define __UPX_BPTR_H 1


/*************************************************************************
// BoundedPtr
**************************************************************************/

template <class T>
class BoundedPtr
{
public:
    //typedef T* StoredType;
    //typedef T* PointerType;
    //typedef T& ReferenceType;

    ~BoundedPtr() { }

    explicit BoundedPtr(void* base, size_t size_in_bytes, T* ptr=0)
        : ptr_(ptr), base_(base), size_in_bytes_(0)
    {
        assert(base_ != NULL);
        size_in_bytes_ = mem_size(1, size_in_bytes);
        check();
    }

    // assignment
    BoundedPtr& operator= (const BoundedPtr& other) {
        assert(base_ == other.base_);
        assert(size_in_bytes_ == other.size_in_bytes_);
        ptr_ = other.ptr_; check(); return *this;
    }
    BoundedPtr& operator= (T* other) {
        ptr_ = other; check(); return *this;
    }

    operator       T* ()       { return ptr_; }
    operator const T* () const { return ptr_; }

    BoundedPtr& operator += (size_t n) {
        checkNULL(); ptr_ += n; checkRange(); return *this;
    }
    BoundedPtr& operator -= (size_t n) {
        checkNULL(); ptr_ -= n; checkRange(); return *this;
    }
    BoundedPtr& operator ++ (void) {
        checkNULL(); ptr_ += 1; checkRange(); return *this;
    }

private:
    void checkNULL() const {
        if __acc_very_unlikely(!ptr_)
            throwCantUnpack("unexpected NULL pointer; take care!");
    }
    void checkRange() const {
        size_t off = (const char *) ptr_ - (const char *) base_;
        if __acc_very_unlikely(off > size_in_bytes_)
            throwCantUnpack("pointer out of range; take care!");
    }
    void check() const { // check ptr_ invariant: either NULL or valid checkRange()
        if (ptr_ != NULL)
            checkRange();
    }

    T* ptr_;
    void* base_;
    size_t size_in_bytes_;

    // disable copy
    BoundedPtr(const BoundedPtr&); // {}
    // disable dynamic allocation
    DISABLE_NEW_DELETE
};


#endif /* already included */

/* vim:set ts=4 sw=4 et: */
