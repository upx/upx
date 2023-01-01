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

#if WITH_SPAN

#if 1
#define SPAN_NAMESPACE_NAME XSpan
#define SPAN_NAMESPACE_BEGIN namespace SPAN_NAMESPACE_NAME {
#define SPAN_NAMESPACE_END }
#define SPAN_NS(x) SPAN_NAMESPACE_NAME ::x
#else
#define SPAN_NAMESPACE_BEGIN /*empty*/
#define SPAN_NAMESPACE_END   /*empty*/
#define SPAN_NS(x) ::x
#endif

SPAN_NAMESPACE_BEGIN

// HINT: set env-var "UPX_DEBUG_DOCTEST_DISABLE=1" for improved debugging experience
__acc_noinline void span_fail_nullptr();
__acc_noinline void span_fail_nullbase();
__acc_noinline void span_fail_not_same_base();
__acc_noinline void span_fail_range_nullptr();
__acc_noinline void span_fail_range_nullbase();
__acc_noinline void span_fail_range_range();
void span_check_range(const void *p, const void *base, ptrdiff_t size_in_bytes);

// help constructor to distinguish between number of elements and bytes
struct SpanCount {
    explicit SpanCount(size_t n) : count(n) {}
    size_t count; // public
};
struct SpanSizeInBytes {
    explicit SpanSizeInBytes(size_t bytes) : size_in_bytes(bytes) {}
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

#ifndef span_mem_size_impl
template <class T>
inline size_t span_mem_size_impl(size_t n) {
#ifdef UPX_VERSION_HEX
    // check for overflow and sane limits
    return mem_size(sizeof(T), n);
#else
    return sizeof(T) * n;
#endif
}
#endif

template <class T>
inline size_t span_mem_size(size_t n) {
    return span_mem_size_impl<typename TypeForSizeOf<T>::type>(n);
}

template <class T>
inline void span_mem_size_assert_ptrdiff(ptrdiff_t n) {
    if (n >= 0)
        (void) span_mem_size<T>((size_t) n);
    else
        (void) span_mem_size<T>((size_t) -n);
}

#if 0
template <class From, class To>
struct Span_is_convertible : public std::is_convertible<From *, To *> {};
#else

namespace detail {
template <class T, class U>
struct Span_void_to_T {
    typedef U type;
};
template <class T>
struct Span_void_to_T<T, void> {
    typedef typename std::remove_const<T>::type type;
};
template <class T>
struct Span_void_to_T<T, const void> {
    // typedef typename std::add_const<T>::type type;
    typedef T type;
};

template <class From, class To>
struct Span_ptr_is_convertible : public std::false_type {};
template <class T>
struct Span_ptr_is_convertible<T, T> : public std::true_type {};
template <class T>
struct Span_ptr_is_convertible<T, const T> : public std::true_type {};
} // namespace detail

template <class From, class To>
struct Span_is_convertible
    : public detail::Span_ptr_is_convertible<From,
                                             typename detail::Span_void_to_T<From, To>::type> {};
#endif

#if 1
// char => char
ACC_COMPILE_TIME_ASSERT_HEADER((Span_is_convertible<char, char>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((Span_is_convertible<char, const char>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((Span_is_convertible<const char, const char>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((!Span_is_convertible<const char, char>::value))

// void => void
ACC_COMPILE_TIME_ASSERT_HEADER((Span_is_convertible<void, void>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((Span_is_convertible<void, const void>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((Span_is_convertible<const void, const void>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((!Span_is_convertible<const void, void>::value))

// char => void
ACC_COMPILE_TIME_ASSERT_HEADER((Span_is_convertible<char, void>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((Span_is_convertible<char, const void>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((Span_is_convertible<const char, const void>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((!Span_is_convertible<const char, void>::value))

// void => char
ACC_COMPILE_TIME_ASSERT_HEADER((!Span_is_convertible<void, char>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((!Span_is_convertible<void, const char>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((!Span_is_convertible<const void, const char>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((!Span_is_convertible<const void, char>::value))

// char => int
ACC_COMPILE_TIME_ASSERT_HEADER(!(Span_is_convertible<char, int>::value))
ACC_COMPILE_TIME_ASSERT_HEADER(!(Span_is_convertible<char, const int>::value))
ACC_COMPILE_TIME_ASSERT_HEADER(!(Span_is_convertible<const char, const int>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((!Span_is_convertible<const char, int>::value))
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

class SpanInternalDummyArg; // not implemented

SPAN_NAMESPACE_END

#ifndef SPAN_DELETED_FUNCTION
#define SPAN_DELETED_FUNCTION = delete
#endif
#define SPAN_REQUIRES_CONVERTIBLE_UT(T, U, RType)                                                  \
    typename std::enable_if<SPAN_NS(Span_is_convertible) < U, T>::value, RType > ::type
#define SPAN_REQUIRES_CONVERTIBLE_ANY_DIRECTION(T, U, RType)                                       \
    typename std::enable_if<SPAN_NS(Span_is_convertible) < U, T>::value ||                         \
        SPAN_NS(Span_is_convertible)<T, U>::value,                                                 \
        RType > ::type
// note: these use "T" and "U"
#define SPAN_REQUIRES_CONVERTIBLE_R(RType) SPAN_REQUIRES_CONVERTIBLE_UT(T, U, RType)
#define SPAN_REQUIRES_CONVERTIBLE_A                                                                \
    SPAN_REQUIRES_CONVERTIBLE_R(SPAN_NS(SpanInternalDummyArg) *) = nullptr
#define SPAN_REQUIRES_CONVERTIBLE_T SPAN_REQUIRES_CONVERTIBLE_R(SPAN_NS(SpanInternalDummyArg) *)
// note: these use "T" and "U"
#define SPAN_REQUIRES_SIZE_1_R(RType)                                                              \
    typename std::enable_if<SPAN_NS(Span_is_convertible) < U, T>::value &&SPAN_NS(                 \
        ValueForSizeOf)<T>::value == 1 &&                                                          \
        SPAN_NS(ValueForSizeOf)<U>::value == 1,                                                    \
        RType > ::type
#define SPAN_REQUIRES_SIZE_1_A SPAN_REQUIRES_SIZE_1_R(SPAN_NS(SpanInternalDummyArg) *) = nullptr

#include "xspan_impl_ptr_or_null.h"
#include "xspan_impl_ptr_or_span.h"
#include "xspan_impl_span.h"
#include "xspan_impl_ptr.h"
#undef SPAN_REQUIRES_CONVERTIBLE_UT
#undef SPAN_REQUIRES_CONVERTIBLE_ANY_DIRECTION
#undef SPAN_REQUIRES_CONVERTIBLE_A
#undef SPAN_REQUIRES_CONVERTIBLE_R
#undef SPAN_REQUIRES_CONVERTIBLE_T
#undef SPAN_REQUIRES_SIZE_1_A
#undef SPAN_REQUIRES_SIZE_1_R

#endif // WITH_SPAN

/* vim:set ts=4 sw=4 et: */
