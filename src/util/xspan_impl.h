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

#if WITH_XSPAN

#if 1
#define XSPAN_NAMESPACE_NAME XSpan
#define XSPAN_NAMESPACE_BEGIN namespace XSPAN_NAMESPACE_NAME {
#define XSPAN_NAMESPACE_END }
#define XSPAN_NS(x) XSPAN_NAMESPACE_NAME ::x
#else
#define XSPAN_NAMESPACE_BEGIN /*empty*/
#define XSPAN_NAMESPACE_END   /*empty*/
#define XSPAN_NS(x) ::x
#endif

XSPAN_NAMESPACE_BEGIN

// HINT: set env-var "UPX_DEBUG_DOCTEST_DISABLE=1" for improved debugging experience
noinline void xspan_fail_nullptr();
noinline void xspan_fail_nullbase();
noinline void xspan_fail_not_same_base();
noinline void xspan_fail_range_nullptr();
noinline void xspan_fail_range_nullbase();
noinline void xspan_fail_range_range();
void xspan_check_range(const void *p, const void *base, ptrdiff_t size_in_bytes);

// help constructor to distinguish between number of elements and bytes
struct XSpanCount {
    explicit XSpanCount(size_t n) : count(n) {}
    size_t count; // public
};
struct XSpanSizeInBytes {
    explicit XSpanSizeInBytes(size_t bytes) : size_in_bytes(bytes) {}
    size_t size_in_bytes; // public
};

template <class T>
struct TypeForSizeOf {
    typedef T type;
};
template <>
struct TypeForSizeOf<void> {
    typedef char type;
};
template <>
struct TypeForSizeOf<const void> {
    typedef const char type;
};

template <class T>
struct ValueForSizeOf {
    static const size_t value = sizeof(typename TypeForSizeOf<T>::type);
};

ACC_COMPILE_TIME_ASSERT_HEADER(ValueForSizeOf<char>::value == 1)
ACC_COMPILE_TIME_ASSERT_HEADER(ValueForSizeOf<const char>::value == 1)
ACC_COMPILE_TIME_ASSERT_HEADER(ValueForSizeOf<void>::value == 1)
ACC_COMPILE_TIME_ASSERT_HEADER(ValueForSizeOf<const void>::value == 1)
ACC_COMPILE_TIME_ASSERT_HEADER(ValueForSizeOf<int>::value == 4)

#ifndef xspan_mem_size_impl
template <class T>
inline size_t xspan_mem_size_impl(size_t n) {
#ifdef UPX_VERSION_HEX
    // check for overflow and sane limits
    return mem_size(sizeof(T), n);
#else
    return sizeof(T) * n;
#endif
}
#endif

template <class T>
inline size_t xspan_mem_size(size_t n) {
    return xspan_mem_size_impl<typename TypeForSizeOf<T>::type>(n);
}

template <class T>
inline void xspan_mem_size_assert_ptrdiff(ptrdiff_t n) {
    if (n >= 0)
        (void) xspan_mem_size<T>((size_t) n);
    else
        (void) xspan_mem_size<T>((size_t) -n);
}

#if 0
template <class From, class To>
struct XSpan_is_convertible : public std::is_convertible<From *, To *> {};
#else

namespace detail {
template <class T, class U>
struct XSpan_void_to_T {
    typedef U type;
};
template <class T>
struct XSpan_void_to_T<T, void> {
    typedef typename std::remove_const<T>::type type;
};
template <class T>
struct XSpan_void_to_T<T, const void> {
    // typedef typename std::add_const<T>::type type;
    typedef T type;
};

template <class From, class To>
struct XSpan_ptr_is_convertible : public std::false_type {};
template <class T>
struct XSpan_ptr_is_convertible<T, T> : public std::true_type {};
template <class T>
struct XSpan_ptr_is_convertible<T, const T> : public std::true_type {};
} // namespace detail

template <class From, class To>
struct XSpan_is_convertible
    : public detail::XSpan_ptr_is_convertible<From,
                                              typename detail::XSpan_void_to_T<From, To>::type> {};
#endif

#if DEBUG
// char => char
ACC_COMPILE_TIME_ASSERT_HEADER((XSpan_is_convertible<char, char>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((XSpan_is_convertible<char, const char>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((XSpan_is_convertible<const char, const char>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((!XSpan_is_convertible<const char, char>::value))
// void => void
ACC_COMPILE_TIME_ASSERT_HEADER((XSpan_is_convertible<void, void>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((XSpan_is_convertible<void, const void>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((XSpan_is_convertible<const void, const void>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((!XSpan_is_convertible<const void, void>::value))
// char => void
ACC_COMPILE_TIME_ASSERT_HEADER((XSpan_is_convertible<char, void>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((XSpan_is_convertible<char, const void>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((XSpan_is_convertible<const char, const void>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((!XSpan_is_convertible<const char, void>::value))
// void => char
ACC_COMPILE_TIME_ASSERT_HEADER((!XSpan_is_convertible<void, char>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((!XSpan_is_convertible<void, const char>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((!XSpan_is_convertible<const void, const char>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((!XSpan_is_convertible<const void, char>::value))
// char => int
ACC_COMPILE_TIME_ASSERT_HEADER(!(XSpan_is_convertible<char, int>::value))
ACC_COMPILE_TIME_ASSERT_HEADER(!(XSpan_is_convertible<char, const int>::value))
ACC_COMPILE_TIME_ASSERT_HEADER(!(XSpan_is_convertible<const char, const int>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((!XSpan_is_convertible<const char, int>::value))
#endif

/*************************************************************************
// PtrOrSpanOrNull
// PtrOrSpan
// Span
// Ptr
**************************************************************************/

// forward declarations

template <class T>
struct PtrOrSpanOrNull;
template <class T>
struct PtrOrSpan;
template <class T>
struct Span;
template <class T>
struct Ptr;

class XSpanInternalDummyArg; // not implemented

XSPAN_NAMESPACE_END

#ifndef XSPAN_DELETED_FUNCTION
#define XSPAN_DELETED_FUNCTION = delete
#endif
#define XSPAN_REQUIRES_CONVERTIBLE_ONE_DIRECTION(From, To, RType)                                  \
    typename std::enable_if<XSPAN_NS(XSpan_is_convertible) < From, To>::value, RType > ::type
#define XSPAN_REQUIRES_CONVERTIBLE_ANY_DIRECTION(A, B, RType)                                      \
    typename std::enable_if<XSPAN_NS(XSpan_is_convertible) < A, B>::value ||                       \
        XSPAN_NS(XSpan_is_convertible)<B, A>::value,                                               \
        RType > ::type
// note: these use "T" and "U"
#define XSPAN_REQUIRES_CONVERTIBLE_R(RType) XSPAN_REQUIRES_CONVERTIBLE_ONE_DIRECTION(U, T, RType)
#define XSPAN_REQUIRES_CONVERTIBLE_A                                                               \
    XSPAN_REQUIRES_CONVERTIBLE_R(XSPAN_NS(XSpanInternalDummyArg) *) = nullptr
#define XSPAN_REQUIRES_CONVERTIBLE_T XSPAN_REQUIRES_CONVERTIBLE_R(XSPAN_NS(XSpanInternalDummyArg) *)
// note: these use "T" and "U"
#define XSPAN_REQUIRES_SIZE_1_R(RType)                                                             \
    typename std::enable_if<XSPAN_NS(XSpan_is_convertible) < U, T>::value &&XSPAN_NS(              \
        ValueForSizeOf)<T>::value == 1 &&                                                          \
        XSPAN_NS(ValueForSizeOf)<U>::value == 1,                                                   \
        RType > ::type
#define XSPAN_REQUIRES_SIZE_1_A XSPAN_REQUIRES_SIZE_1_R(XSPAN_NS(XSpanInternalDummyArg) *) = nullptr

#include "xspan_impl_ptr_or_null.h"
#include "xspan_impl_ptr_or_span.h"
#include "xspan_impl_span.h"
#include "xspan_impl_ptr.h"
#undef XSPAN_REQUIRES_CONVERTIBLE_ONE_DIRECTION
#undef XSPAN_REQUIRES_CONVERTIBLE_ANY_DIRECTION
#undef XSPAN_REQUIRES_CONVERTIBLE_A
#undef XSPAN_REQUIRES_CONVERTIBLE_R
#undef XSPAN_REQUIRES_CONVERTIBLE_T
#undef XSPAN_REQUIRES_SIZE_1_A
#undef XSPAN_REQUIRES_SIZE_1_R

#endif // WITH_XSPAN

/* vim:set ts=4 sw=4 et: */
