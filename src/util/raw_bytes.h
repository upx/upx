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

/*************************************************************************
// raw_bytes() - get underlying memory from checked buffers/pointers.
// This is overloaded by various utility classes like MemBuffer and XSpan.
//
// Note that the pointer type is retained, the "_bytes" hints size_in_bytes
**************************************************************************/

// default: for any regular pointer, raw_bytes() is just the pointer itself
template <class T>
inline
    typename std::enable_if<std::is_pointer<T>::value && !upx::is_bounded_array<T>::value, T>::type
    raw_bytes(T ptr, size_t size_in_bytes) {
    if (size_in_bytes > 0) {
        if very_unlikely (ptr == nullptr)
            throwCantPack("raw_bytes unexpected NULL ptr");
        if very_unlikely (__acc_cte(VALGRIND_CHECK_MEM_IS_ADDRESSABLE(ptr, size_in_bytes) != 0))
            throwCantPack("raw_bytes valgrind-check-mem");
    }
    return ptr;
}

// default: for any regular pointer, raw_index_bytes() is just "pointer + index"
// NOTE: index == number of elements, *NOT* size in bytes!
template <class T>
inline typename std::enable_if<std::is_pointer<T>::value && !upx::is_bounded_array<T>::value &&
                                   !std::is_void<typename std::remove_pointer<T>::type>::value,
                               T>::type
raw_index_bytes(T ptr, size_t index, size_t size_in_bytes) {
    typedef typename std::remove_pointer<T>::type element_type;
    if very_unlikely (ptr == nullptr)
        throwCantPack("raw_index_bytes unexpected NULL ptr");
    size_in_bytes = mem_size(sizeof(element_type), index, size_in_bytes); // assert size
    if very_unlikely (__acc_cte(VALGRIND_CHECK_MEM_IS_ADDRESSABLE(ptr, size_in_bytes) != 0))
        throwCantPack("raw_index_bytes valgrind-check-mem");
    UNUSED(size_in_bytes);
    return ptr + index;
}

// same for bounded arrays
template <class T, size_t N>
inline T *raw_bytes(T (&array)[N], size_t size_in_bytes) {
    typedef T element_type;
    if very_unlikely (size_in_bytes > mem_size(sizeof(element_type), N))
        throwCantPack("raw_bytes out of range");
    return array;
}

template <class T, size_t N>
inline T *raw_index_bytes(T (&array)[N], size_t index, size_t size_in_bytes) {
    typedef T element_type;
    return raw_bytes(array, mem_size(sizeof(element_type), index, size_in_bytes)) + index;
}

/* vim:set ts=4 sw=4 et: */
