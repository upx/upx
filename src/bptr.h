/* bptr.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
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
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
 */


#ifndef __UPX_BPTR_H
#define __UPX_BPTR_H


/*************************************************************************
// BoundedPtr
**************************************************************************/

template <class T>
class BoundedPtr
{
//    typedef BoundedPtr<T> Self;
public:
    typedef T* StoredType;
    typedef T* PointerType;
    typedef T& ReferenceType;

    ~BoundedPtr() { }

    explicit BoundedPtr(void* base, unsigned size, T* ptr=0)
        : ptr_(ptr), base_(base), size_(size) { check(); }
    BoundedPtr& operator= (const BoundedPtr& other) {
        assert(base_ == other.base_); assert(size_ == other.size_);
        ptr_ = other.ptr_; check(); return *this;
    }
    BoundedPtr& operator= (T* other) {
        ptr_ = other; check(); return *this;
    }

    operator T* () { check(); return ptr_; }
    operator const T* () const { check(); return ptr_; }

    BoundedPtr& operator += (size_t n) {
        checkStrict(); ptr_ += n; checkStrict(); return *this;
    }
    BoundedPtr& operator -= (size_t n) {
        checkStrict(); ptr_ -= n; checkStrict(); return *this;
    }
    BoundedPtr& operator ++ (void) {
        checkStrict(); ptr_ += 1; checkStrict(); return *this;
    }
//    T* operator ++ (int) {
//        T* p = ptr_; checkStrict(); ptr_ += 1; checkStrict(); return p;
//    }
//    BoundedPtr& operator -- (void) {
//        checkStrict(); ptr_ -= 1; checkStrict(); return *this;
//    }


private:
    void checkNULL() const {
        if (!ptr_)
            throwCantUnpack("unexpected NULL pointer; take care!");
    }
    void checkRange(size_t extra=0) const {
        size_t off = (const char *) ptr_ - (const char *) base_;
        if (off > size_ || off + extra > size_)
            throwCantUnpack("pointer out of range; take care!");
    }
    void checkStrict(size_t extra=0) const {
        checkNULL();
        checkRange(extra);
    }
    void check(size_t extra=0) const {
        if (ptr_) checkRange(extra);
    }

    T* ptr_;
    void* base_;
    size_t size_;

    // disable copy
    BoundedPtr(const BoundedPtr&); // {}
//    BoundedPtr& operator= (const BoundedPtr&); // { return *this; }

    // disable dynamic allocation
    DISABLE_NEW_DELETE
};


#endif /* already included */


/*
vi:ts=4:et
*/

