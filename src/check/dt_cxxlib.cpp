/* dt_cxxlib.cpp -- doctest check

   This file is part of the UPX executable compressor.

   Copyright (C) Markus Franz Xaver Johannes Oberhumer
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

// lots of tests (and probably quite a number of redundant tests)
// modern compilers will optimize away much of this code

#include "../util/system_headers.h"
#include <vector> // std::vector
#include "../conf.h"
#include "../util/membuffer.h"

/*************************************************************************
// xspan codegen
**************************************************************************/

namespace {
template <class T>
struct TestXSpanCG final {
    // create a value
    static noinline XSPAN_0(T) make_span_0_0(T *p, size_t bytes) {
        XSPAN_0(T) r = XSPAN_0_MAKE(T, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        return r;
    }
    static noinline XSPAN_P(T) make_span_p_0(T *p, size_t bytes) {
        XSPAN_P(T) r = XSPAN_0_MAKE(T, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        return r;
    }
    static noinline XSPAN_S(T) make_span_s_0(T *p, size_t bytes) {
        XSPAN_S(T) r = XSPAN_0_MAKE(T, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        return r;
    }

    static noinline XSPAN_0(T) make_span_0_p(T *p, size_t bytes) {
        XSPAN_0(T) r = XSPAN_P_MAKE(T, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        return r;
    }
    static noinline XSPAN_P(T) make_span_p_p(T *p, size_t bytes) {
        XSPAN_P(T) r = XSPAN_P_MAKE(T, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        return r;
    }
    static noinline XSPAN_S(T) make_span_s_p(T *p, size_t bytes) {
        XSPAN_S(T) r = XSPAN_P_MAKE(T, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        return r;
    }

    static noinline XSPAN_0(T) make_span_0_s(T *p, size_t bytes) {
        XSPAN_0(T) r = XSPAN_S_MAKE(T, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        return r;
    }
    static noinline XSPAN_P(T) make_span_p_s(T *p, size_t bytes) {
        XSPAN_P(T) r = XSPAN_S_MAKE(T, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        return r;
    }
    static noinline XSPAN_S(T) make_span_s_s(T *p, size_t bytes) {
        XSPAN_S(T) r = XSPAN_S_MAKE(T, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        return r;
    }

    // define a variable
    static noinline XSPAN_0(T) var_span_0(T *p, size_t bytes) {
        XSPAN_0_VAR(T, r, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        return r;
    }
    static noinline XSPAN_P(T) var_span_p(T *p, size_t bytes) {
        XSPAN_P_VAR(T, r, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        return r;
    }
    static noinline XSPAN_S(T) var_span_s(T *p, size_t bytes) {
        XSPAN_S_VAR(T, r, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        return r;
    }

    // cast to a different type (creates a new value)
    static noinline XSPAN_0(LE32) type_cast_0(T *p, size_t bytes) {
        XSPAN_0_VAR(T, const r, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        return XSPAN_TYPE_CAST(LE32, r);
    }
    static noinline XSPAN_P(LE32) type_cast_p(T *p, size_t bytes) {
        XSPAN_P_VAR(T, const r, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        return XSPAN_TYPE_CAST(LE32, r);
    }
    static noinline XSPAN_S(LE32) type_cast_s(T *p, size_t bytes) {
        XSPAN_S_VAR(T, const r, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        return XSPAN_TYPE_CAST(LE32, r);
    }

    static noinline XSPAN_0(const LE32) type_cast_const_0(T *p, size_t bytes) {
        XSPAN_0_VAR(T, const r, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        return XSPAN_TYPE_CAST(const LE32, r);
    }
    static noinline XSPAN_P(const LE32) type_cast_const_p(T *p, size_t bytes) {
        XSPAN_P_VAR(T, const r, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        return XSPAN_TYPE_CAST(const LE32, r);
    }
    static noinline XSPAN_S(const LE32) type_cast_const_s(T *p, size_t bytes) {
        XSPAN_S_VAR(T, const r, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        return XSPAN_TYPE_CAST(const LE32, r);
    }

    // poison a pointer: point to a non-null invalid address
    static noinline XSPAN_0(T) invalidate_0(T *p, size_t bytes) {
        XSPAN_0_VAR(T, r, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        XSPAN_INVALIDATE(r);
        return r;
    }
    static noinline XSPAN_P(T) invalidate_p(T *p, size_t bytes) {
        XSPAN_P_VAR(T, r, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        XSPAN_INVALIDATE(r);
        return r;
    }
    static noinline XSPAN_S(T) invalidate_s(T *p, size_t bytes) {
        XSPAN_S_VAR(T, r, p, XSpanSizeInBytes(bytes));
        (void) bytes;
        XSPAN_INVALIDATE(r);
        return r;
    }
};
} // namespace

TEST_CASE("xspan codegen") {
    // typedef byte T;
    typedef int T;
    T buf[4] = {0, 1, 2, 3};
    const size_t bytes = acc_vget_acc_hsize_t(sizeof(buf), 0);
    {
        auto r = TestXSpanCG<T>::make_span_0_0(buf, bytes);
        CHECK(r == buf);
    }
    {
        auto r = TestXSpanCG<T>::make_span_p_0(buf, bytes);
        CHECK(r == buf);
    }
    {
        auto r = TestXSpanCG<T>::make_span_s_0(buf, bytes);
        CHECK(r == buf);
    }
    {
        auto r = TestXSpanCG<T>::make_span_0_p(buf, bytes);
        CHECK(r == buf);
    }
    {
        auto r = TestXSpanCG<T>::make_span_p_p(buf, bytes);
        CHECK(r == buf);
    }
    {
        auto r = TestXSpanCG<T>::make_span_s_p(buf, bytes);
        CHECK(r == buf);
    }
    {
        auto r = TestXSpanCG<T>::make_span_0_s(buf, bytes);
        CHECK(r == buf);
    }
    {
        auto r = TestXSpanCG<T>::make_span_p_s(buf, bytes);
        CHECK(r == buf);
    }
    {
        auto r = TestXSpanCG<T>::make_span_s_s(buf, bytes);
        CHECK(r == buf);
    }
    {
        auto r = TestXSpanCG<T>::var_span_0(buf, bytes);
        CHECK(r == buf);
    }
    {
        auto r = TestXSpanCG<T>::var_span_p(buf, bytes);
        CHECK(r == buf);
    }
    {
        auto r = TestXSpanCG<T>::var_span_s(buf, bytes);
        CHECK(r == buf);
    }
    {
        auto r = TestXSpanCG<T>::type_cast_0(buf, bytes);
        CHECK(r == upx::ptr_static_cast<LE32 *>(buf));
    }
    {
        auto r = TestXSpanCG<T>::type_cast_p(buf, bytes);
        CHECK(r == upx::ptr_static_cast<LE32 *>(buf));
    }
    {
        auto r = TestXSpanCG<T>::type_cast_s(buf, bytes);
        CHECK(r == upx::ptr_static_cast<LE32 *>(buf));
    }
    {
        auto r = TestXSpanCG<T>::invalidate_0(buf, bytes);
#if defined(__CHERI__) && defined(__CHERI_PURE_CAPABILITY__)
        (void) r;
#else
        CHECK(r != buf);
        CHECK(r != nullptr);
#endif
    }
    {
        auto r = TestXSpanCG<T>::invalidate_p(buf, bytes);
#if defined(__CHERI__) && defined(__CHERI_PURE_CAPABILITY__)
        (void) r;
#else
        CHECK(r != buf);
        CHECK(r != nullptr);
#endif
    }
    {
        auto r = TestXSpanCG<T>::invalidate_s(buf, bytes);
#if defined(__CHERI__) && defined(__CHERI_PURE_CAPABILITY__)
        (void) r;
#else
        CHECK(r != buf);
        CHECK(r != nullptr);
#endif
    }
    {
        auto r0 = TestXSpanCG<T>::var_span_0(buf, bytes);
        auto r1 = TestXSpanCG<T>::var_span_0(buf + 1, bytes - sizeof(T));
        CHECK(std::is_same_v<decltype(r0 - r1), ptrdiff_t>);
        CHECK((r0 - r1 == -1));
        CHECK((r1 - r0 == 1));
    }
    {
        auto r0 = TestXSpanCG<T>::var_span_p(buf, bytes);
        auto r1 = TestXSpanCG<T>::var_span_p(buf + 1, bytes - sizeof(T));
        CHECK(std::is_same_v<decltype(r0 - r1), ptrdiff_t>);
        CHECK((r0 - r1 == -1));
        CHECK((r1 - r0 == 1));
    }
    {
        auto r0 = TestXSpanCG<T>::var_span_s(buf, bytes);
        auto r1 = TestXSpanCG<T>::var_span_s(buf + 1, bytes - sizeof(T));
        CHECK(std::is_same_v<decltype(r0 - r1), ptrdiff_t>);
        CHECK((r0 - r1 == -1));
        CHECK((r1 - r0 == 1));
    }
    {
        auto r0 = XSPAN_0_MAKE(T, nullptr);
        auto r1 = XSPAN_0_MAKE(T, nullptr);
        CHECK(std::is_same_v<decltype(r0 - r1), ptrdiff_t>);
        CHECK((r0 - r1 == 0));
    }
    {
        auto r0 = XSPAN_0_MAKE(const T, nullptr);
        auto r1 = XSPAN_0_MAKE(T, nullptr);
        CHECK(std::is_same_v<decltype(r0 - r1), ptrdiff_t>);
        CHECK((r0 - r1 == 0));
    }
    {
        auto r0 = XSPAN_0_MAKE(const T, nullptr);
        auto r1 = XSPAN_0_MAKE(const T, XSPAN_0_MAKE(T, nullptr));
        CHECK(std::is_same_v<decltype(r0 - r1), ptrdiff_t>);
        CHECK((r0 - r1 == 0));
    }
    CHECK(TestXSpanCG<T>::make_span_0_0(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::make_span_p_0(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::make_span_s_0(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::make_span_0_p(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::make_span_p_p(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::make_span_s_p(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::make_span_0_s(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::make_span_p_s(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::make_span_s_s(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::var_span_0(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::var_span_p(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::var_span_s(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::type_cast_0(buf, bytes) == upx::ptr_static_cast<LE32 *>(buf));
    CHECK(TestXSpanCG<T>::type_cast_p(buf, bytes) == upx::ptr_static_cast<LE32 *>(buf));
    CHECK(TestXSpanCG<T>::type_cast_s(buf, bytes) == upx::ptr_static_cast<LE32 *>(buf));
    CHECK(TestXSpanCG<T>::type_cast_const_0(buf, bytes) == upx::ptr_static_cast<const LE32 *>(buf));
    CHECK(TestXSpanCG<T>::type_cast_const_p(buf, bytes) == upx::ptr_static_cast<const LE32 *>(buf));
    CHECK(TestXSpanCG<T>::type_cast_const_s(buf, bytes) == upx::ptr_static_cast<const LE32 *>(buf));
#if defined(__CHERI__) && defined(__CHERI_PURE_CAPABILITY__)
#else
    CHECK(TestXSpanCG<T>::invalidate_0(buf, bytes) != buf);
    CHECK(TestXSpanCG<T>::invalidate_p(buf, bytes) != buf);
    CHECK(TestXSpanCG<T>::invalidate_s(buf, bytes) != buf);
#endif
    (void) buf;
    (void) bytes;
}

TEST_CASE("xspan codegen const") {
    typedef const byte T;
    // typedef const int T;
    T buf[4] = {0, 1, 2, 3};
    const size_t bytes = acc_vget_acc_hsize_t(sizeof(buf), 0);
    CHECK(TestXSpanCG<T>::make_span_0_0(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::make_span_p_0(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::make_span_s_0(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::make_span_0_p(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::make_span_p_p(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::make_span_s_p(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::make_span_0_s(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::make_span_p_s(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::make_span_s_s(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::var_span_0(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::var_span_p(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::var_span_s(buf, bytes) == buf);
    CHECK(TestXSpanCG<T>::type_cast_const_0(buf, bytes) == upx::ptr_static_cast<const LE32 *>(buf));
    CHECK(TestXSpanCG<T>::type_cast_const_p(buf, bytes) == upx::ptr_static_cast<const LE32 *>(buf));
    CHECK(TestXSpanCG<T>::type_cast_const_s(buf, bytes) == upx::ptr_static_cast<const LE32 *>(buf));
    (void) buf;
    (void) bytes;
}

/*************************************************************************
// standard C++ library
**************************************************************************/

TEST_CASE("std::vector") {
    constexpr size_t N = 16;
    std::vector<int> v(N);
    CHECK(v.end() - v.begin() == N);
    CHECK(&v[0] == &(*(v.begin())));
    // CHECK(&v[0] + N == &(*(v.end()))); // TODO later: is this legal??
#if defined(_LIBCPP_HARDENING_MODE) && defined(_LIBCPP_HARDENING_MODE_DEBUG) &&                    \
    (_LIBCPP_HARDENING_MODE == _LIBCPP_HARDENING_MODE_DEBUG)
    // unfortunately this does NOT throw but ABORTS
    ////CHECK_THROWS((void) &v[N]);
#endif
    UNUSED(v);
}

/*************************************************************************
// core util: UPX_CXX_DISABLE_xxx, noncopyable
**************************************************************************/

namespace {
template <class TA, class TB, int TC = 0>
struct MyType1 {
    MyType1() noexcept {}
    UPX_CXX_DISABLE_ADDRESS(MyType1)
    UPX_CXX_DISABLE_COPY_MOVE(MyType1)
    UPX_CXX_DISABLE_NEW_DELETE_NO_VIRTUAL(MyType1)
};
template <class TA, class TB, int TC = 0>
struct MyType2 {
    MyType2() noexcept {}
    UPX_CXX_DISABLE_COPY_MOVE(MyType2)
    typedef MyType2<TA, TB, TC> Self;
    UPX_CXX_DISABLE_ADDRESS(Self)
    UPX_CXX_DISABLE_NEW_DELETE_NO_VIRTUAL(Self)
};
template <class TA, class TB, int TC = 0>
struct MyVType1 {
    MyVType1() noexcept {}
    virtual ~MyVType1() noexcept {}
    UPX_CXX_DISABLE_ADDRESS(MyVType1)
    UPX_CXX_DISABLE_COPY_MOVE(MyVType1)
    UPX_CXX_DISABLE_NEW_DELETE(MyVType1)
};
template <class TA, class TB, int TC = 0>
struct MyVType2 {
    MyVType2() noexcept {}
    virtual ~MyVType2() noexcept {}
    UPX_CXX_DISABLE_COPY_MOVE(MyVType2)
    typedef MyVType2<TA, TB, TC> Self;
    UPX_CXX_DISABLE_ADDRESS(Self)
    UPX_CXX_DISABLE_NEW_DELETE(Self)
};
TEST_CASE("UPX_CXX_DISABLE_xxx") {
    MyType1<char, int, 1> dummy1;
    MyType2<char, int, 2> dummy2;
    MyVType1<char, int, 1> vdummy1;
    MyVType2<char, int, 2> vdummy2;
    (void) dummy1;
    (void) dummy2;
    (void) vdummy1;
    (void) vdummy2;
}
} // namespace

namespace test_disable_new_delete {

struct A1 {
    int a;
};
struct A2 {
    int a;
    UPX_CXX_DISABLE_NEW_DELETE_NO_VIRTUAL(A2)
};
struct B1_A1 : public A1 {
    int b;
};
struct B1_A2 : public A2 {
    int b;
};
struct B2_A1 : public A1 {
    int b;
    UPX_CXX_DISABLE_NEW_DELETE_NO_VIRTUAL(B2_A1)
};
struct B2_A2 : public A2 {
    int b;
    UPX_CXX_DISABLE_NEW_DELETE_NO_VIRTUAL(B2_A2)
};

struct X1 {
    virtual ~X1() noexcept {}
    int x;
};
struct X2 {
    virtual ~X2() noexcept {}
    int x;
    UPX_CXX_DISABLE_NEW_DELETE(X2)
};
struct Y1_X1 : public X1 {
    int y;
};
struct Y1_X2 : public X2 {
    int y;
};
struct Y2_X1 : public X1 {
    int y;
    UPX_CXX_DISABLE_NEW_DELETE(Y2_X1)
};
struct Y2_X2 : public X2 {
    int y;
    UPX_CXX_DISABLE_NEW_DELETE(Y2_X2)
};
struct Z1_X1 : public X1 {
    virtual ~Z1_X1() noexcept {}
    int z;
};
struct Z1_X2 : public X2 {
    virtual ~Z1_X2() noexcept {}
    int z;
};
struct Z2_X1 : public X1 {
    virtual ~Z2_X1() noexcept {}
    int z;
    UPX_CXX_DISABLE_NEW_DELETE(Z2_X1)
};
struct Z2_X2 : public X2 {
    virtual ~Z2_X2() noexcept {}
    int z;
    UPX_CXX_DISABLE_NEW_DELETE(Z2_X2)
};

} // namespace test_disable_new_delete

TEST_CASE("upx::noncopyable") {
    struct Test : private upx::noncopyable {
        int v = 1;
    };
    Test t = {};
    CHECK(t.v == 1);
#if (ACC_CC_MSC) // MSVC thinks that Test is not std::is_trivially_copyable; true or compiler bug?
    // @COMPILER_BUG @MSVC_BUG
    t.v = 0;
#else
    mem_clear(&t);
#endif
    CHECK(t.v == 0);
    constexpr Test x = {};
    static_assert(x.v == 1);
}

/*************************************************************************
// <type_traits>
**************************************************************************/

static_assert(!upx::is_bounded_array_v<std::nullptr_t>);
static_assert(!upx::is_bounded_array_v<decltype(nullptr)>);
static_assert(!upx::is_bounded_array_v<void *>);
static_assert(!upx::is_bounded_array_v<int *>);
static_assert(!upx::is_bounded_array_v<const int *>);
static_assert(!upx::is_bounded_array_v<volatile int *>);
static_assert(!upx::is_bounded_array_v<const volatile int *>);
static_assert(upx::is_bounded_array_v<int[1]>);
static_assert(upx::is_bounded_array_v<const int[1]>);
static_assert(upx::is_bounded_array_v<volatile int[1]>);
static_assert(upx::is_bounded_array_v<const volatile int[1]>);
static_assert(upx::is_bounded_array_v<int[1u]>);
static_assert(upx::is_bounded_array_v<const int[1u]>);
static_assert(upx::is_bounded_array_v<volatile int[1u]>);
static_assert(upx::is_bounded_array_v<const volatile int[1u]>);
static_assert(upx::is_bounded_array_v<int[1l]>);
static_assert(upx::is_bounded_array_v<const int[1l]>);
static_assert(upx::is_bounded_array_v<volatile int[1l]>);
static_assert(upx::is_bounded_array_v<const volatile int[1l]>);

static_assert(upx::is_same_all_v<int>);
static_assert(upx::is_same_all_v<int, int>);
static_assert(upx::is_same_all_v<int, int, int>);
static_assert(!upx::is_same_all_v<int, int, char>);
static_assert(!upx::is_same_all_v<int, char>);
static_assert(!upx::is_same_all_v<int, char, char>);
static_assert(!upx::is_same_all_v<int, char, int>);
static_assert(!upx::is_same_all_v<int, char, long>);

static_assert(!upx::is_same_any_v<int>);
static_assert(upx::is_same_any_v<int, int>);
static_assert(upx::is_same_any_v<int, int, int>);
static_assert(upx::is_same_any_v<int, int, char>);
static_assert(!upx::is_same_any_v<int, char>);
static_assert(!upx::is_same_any_v<int, char, char>);
static_assert(upx::is_same_any_v<int, char, int>);
static_assert(!upx::is_same_any_v<int, char, long>);

static_assert(upx::is_same_any_v<ptrdiff_t, int, long, long long>);
static_assert(upx::is_same_any_v<size_t, unsigned, unsigned long, unsigned long long>);
static_assert(upx::is_same_any_v<upx_ptraddr_t, unsigned, unsigned long, unsigned long long>);
#if defined(__CHERI__) && defined(__CHERI_PURE_CAPABILITY__)
static_assert(!upx::is_same_any_v<upx_uintptr_t, unsigned, unsigned long, unsigned long long>);
#else
static_assert(upx::is_same_any_v<upx_uintptr_t, unsigned, unsigned long, unsigned long long>);
#endif

static_assert(std::is_same_v<int, upx::remove_cvref_t<int> >);
static_assert(std::is_same_v<int, upx::remove_cvref_t<const int> >);
static_assert(std::is_same_v<int, upx::remove_cvref_t<int &> >);
static_assert(std::is_same_v<int, upx::remove_cvref_t<const int &> >);
static_assert(std::is_same_v<int, upx::remove_cvref_t<int &&> >);
static_assert(std::is_same_v<int, upx::remove_cvref_t<const int &&> >);
static_assert(std::is_same_v<int *, upx::remove_cvref_t<int *> >);
static_assert(std::is_same_v<int *, upx::remove_cvref_t<int *const> >);
static_assert(std::is_same_v<const int *, upx::remove_cvref_t<const int *> >);
static_assert(std::is_same_v<int *, upx::remove_cvref_t<int *&> >);
static_assert(std::is_same_v<int *, upx::remove_cvref_t<int *const &> >);
static_assert(std::is_same_v<const int *, upx::remove_cvref_t<const int *&> >);
static_assert(std::is_same_v<int *, upx::remove_cvref_t<int *&&> >);
static_assert(std::is_same_v<int *, upx::remove_cvref_t<int *const &&> >);
static_assert(std::is_same_v<const int *, upx::remove_cvref_t<const int *&&> >);
static_assert(std::is_same_v<int[1], upx::remove_cvref_t<int[1]> >);
static_assert(std::is_same_v<int[1], upx::remove_cvref_t<const int[1]> >);

static_assert(std::is_same_v<int, upx::type_identity_t<int> >);
static_assert(std::is_same_v<const int, upx::type_identity_t<const int> >);
static_assert(std::is_same_v<int *, upx::type_identity_t<int *> >);
static_assert(std::is_same_v<int *const, upx::type_identity_t<int *const> >);
static_assert(std::is_same_v<const int *, upx::type_identity_t<const int *> >);
static_assert(std::is_same_v<int &, upx::type_identity_t<int &> >);
static_assert(std::is_same_v<const int &, upx::type_identity_t<const int &> >);
static_assert(std::is_same_v<int &&, upx::type_identity_t<int &&> >);
static_assert(std::is_same_v<const int &&, upx::type_identity_t<const int &&> >);
static_assert(std::is_same_v<int[1], upx::type_identity_t<int[1]> >);
static_assert(std::is_same_v<const int[1], upx::type_identity_t<const int[1]> >);

/*************************************************************************
// <bit>
**************************************************************************/

static_assert(!upx::has_single_bit(0));
static_assert(upx::has_single_bit(1));
static_assert(upx::has_single_bit(2));
static_assert(!upx::has_single_bit(3));
static_assert(upx::has_single_bit(4));

/*************************************************************************
// <algorithm>
**************************************************************************/

static_assert(upx::align_down(0, 4) == 0);
static_assert(upx::align_down(1, 4) == 0);
static_assert(upx::align_down(2, 4) == 0);
static_assert(upx::align_down(3, 4) == 0);
static_assert(upx::align_down(4, 4) == 4);
static_assert(upx::align_down_gap(0, 4) == 0);
static_assert(upx::align_down_gap(1, 4) == 1);
static_assert(upx::align_down_gap(2, 4) == 2);
static_assert(upx::align_down_gap(3, 4) == 3);
static_assert(upx::align_down_gap(4, 4) == 0);
static_assert(upx::align_up(0, 4) == 0);
static_assert(upx::align_up(1, 4) == 4);
static_assert(upx::align_up(2, 4) == 4);
static_assert(upx::align_up(3, 4) == 4);
static_assert(upx::align_up(4, 4) == 4);
static_assert(upx::align_up_gap(0, 4) == 0);
static_assert(upx::align_up_gap(1, 4) == 3);
static_assert(upx::align_up_gap(2, 4) == 2);
static_assert(upx::align_up_gap(3, 4) == 1);
static_assert(upx::align_up_gap(4, 4) == 0);

static_assert(upx::min<upx_int8_t>(1, 2) == 1);
static_assert(upx::min<upx_int16_t>(1, 2) == 1);
static_assert(upx::min<upx_int32_t>(1, 2) == 1);
static_assert(upx::min<upx_int64_t>(1, 2) == 1);
static_assert(upx::min<intmax_t>(1, 2) == 1);
static_assert(upx::max<upx_int8_t>(1, 2) == 2);
static_assert(upx::max<upx_int16_t>(1, 2) == 2);
static_assert(upx::max<upx_int32_t>(1, 2) == 2);
static_assert(upx::max<upx_int64_t>(1, 2) == 2);
static_assert(upx::max<intmax_t>(1, 2) == 2);

static_assert(upx::min(1, 2) == 1);
static_assert(upx::min(1l, 2l) == 1);
static_assert(upx::min(1ll, 2ll) == 1);
static_assert(upx::max(1, 2) == 2);
static_assert(upx::max(1l, 2l) == 2);
static_assert(upx::max(1ll, 2ll) == 2);

static_assert(upx::umin<upx_uint8_t>(1, 2) == 1);
static_assert(upx::umin<upx_uint16_t>(1, 2) == 1);
static_assert(upx::umin<upx_uint32_t>(1, 2) == 1);
static_assert(upx::umin<upx_uint64_t>(1, 2) == 1);
static_assert(upx::umin<uintmax_t>(1, 2) == 1);
static_assert(upx::umax<upx_uint8_t>(1, 2) == 2);
static_assert(upx::umax<upx_uint16_t>(1, 2) == 2);
static_assert(upx::umax<upx_uint32_t>(1, 2) == 2);
static_assert(upx::umax<upx_uint64_t>(1, 2) == 2);
static_assert(upx::umax<uintmax_t>(1, 2) == 2);

static_assert(upx::umin(1u, 2u) == 1);
static_assert(upx::umin(1ul, 2ul) == 1);
static_assert(upx::umin(1ull, 2ull) == 1);
static_assert(upx::umax(1u, 2u) == 2);
static_assert(upx::umax(1ul, 2ul) == 2);
static_assert(upx::umax(1ull, 2ull) == 2);

static_assert(upx::wrapping_add<upx_int8_t>(127, 2) == -127);
static_assert(upx::wrapping_add<upx_int16_t>(32767, 2) == -32767);
static_assert(upx::wrapping_add(2147483647, 2) == -2147483647);
static_assert(upx::wrapping_add(9223372036854775807ll, 2ll) == -9223372036854775807ll);
static_assert(upx::wrapping_sub<upx_int8_t>(-127, 2) == 127);
static_assert(upx::wrapping_sub<upx_int16_t>(-32767, 2) == 32767);
static_assert(upx::wrapping_sub(-2147483647, 2) == 2147483647);
static_assert(upx::wrapping_sub(-9223372036854775807ll, 2ll) == 9223372036854775807ll);

static_assert(upx::wrapping_add<upx_uint8_t>(255, 2) == 1);
static_assert(upx::wrapping_sub<upx_uint8_t>(1, 2) == 255);

/*************************************************************************
// util
**************************************************************************/

// upx::UnsignedSizeOf
static_assert(usizeof(int) == sizeof(int));
static_assert(usizeof('a') == sizeof(char));
static_assert(usizeof("") == 1);
static_assert(usizeof("a") == 2);
static_assert(usizeof("ab") == 3);
static_assert(usizeof(L'a') == sizeof(wchar_t));
static_assert(usizeof(L"") == 1 * sizeof(wchar_t));
static_assert(usizeof(L"a") == 2 * sizeof(wchar_t));
static_assert(usizeof(L"ab") == 3 * sizeof(wchar_t));
static_assert(usizeof(0) == sizeof(int));
static_assert(usizeof(0L) == sizeof(long));
static_assert(usizeof(0LL) == sizeof(long long));
static_assert(usizeof(nullptr) == sizeof(void *));
static_assert(usizeof(sizeof(0)) == sizeof(size_t));
static_assert(usizeof(usizeof(0)) == sizeof(unsigned));
#if 0
// works, but may trigger clang/gcc warnings "-Wunused-value"
static_assert(usizeof((1LL, 1)) == sizeof(int));
static_assert(usizeof((1, 1LL)) == sizeof(long long));
#endif

TEST_CASE("upx::ptr_static_cast 1") {
    // check that we do not trigger any -Wcast-align warnings
    using upx::ptr_static_cast;

    void *vp = nullptr;
    byte *bp = nullptr;
    int *ip = nullptr;
    double *dp = nullptr;

    assert((vp == ptr_static_cast<void *>(vp)));
    assert((vp == ptr_static_cast<void *>(bp)));
    assert((vp == ptr_static_cast<void *>(ip)));
    assert((vp == ptr_static_cast<void *>(dp)));

    assert((bp == ptr_static_cast<byte *>(vp)));
    assert((bp == ptr_static_cast<byte *>(bp)));
    assert((bp == ptr_static_cast<byte *>(ip)));
    assert((bp == ptr_static_cast<byte *>(dp)));

    assert((ip == ptr_static_cast<int *>(vp)));
    assert((ip == ptr_static_cast<int *>(bp)));
    assert((ip == ptr_static_cast<int *>(ip)));
    assert((ip == ptr_static_cast<int *>(dp)));

    assert((dp == ptr_static_cast<double *>(vp)));
    assert((dp == ptr_static_cast<double *>(bp)));
    assert((dp == ptr_static_cast<double *>(ip)));
    assert((dp == ptr_static_cast<double *>(dp)));

    const byte *bc = nullptr;
    const int *ic = nullptr;
    assert((bc == ptr_static_cast<byte *>(bp)));
    assert((bc == ptr_static_cast<const byte *>(bc)));
    assert((bc == ptr_static_cast<byte *>(ip)));
    assert((bc == ptr_static_cast<const byte *>(ic)));
    assert((ic == ptr_static_cast<int *>(bp)));
    assert((ic == ptr_static_cast<const int *>(bc)));
    assert((ic == ptr_static_cast<int *>(ip)));
    assert((ic == ptr_static_cast<const int *>(ic)));
}

TEST_CASE("upx::ptr_static_cast 2") {
    {
        byte *n = nullptr;
        assert_noexcept2(n == nullptr);
        assert_noexcept2(upx::ptr_static_cast<void *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const void *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<int *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const int *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char **>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char **>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char ***>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char ***>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char *const>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char *const>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char *const *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char *const *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char **const *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char **const *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char *const *const *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char *const *const *>(n) == nullptr);
    }
    {
        const int *n = nullptr;
        assert_noexcept2(n == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const void *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const int *>(n) == nullptr);
        // assert_noexcept2(upx::ptr_static_cast<const char **>(n) == nullptr);
        // assert_noexcept2(upx::ptr_static_cast<const char ***>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char *const>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char *const *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char *const *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char **const *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char **const *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char *const *const *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char *const *const *>(n) == nullptr);
    }
    {
        long **n = nullptr;
        assert_noexcept2(n == nullptr);
        assert_noexcept2(upx::ptr_static_cast<void *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const void *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<int *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const int *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char **>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char **>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char ***>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char ***>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char *const>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char *const>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char *const *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char *const *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char **const *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char **const *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char *const *const *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char *const *const *>(n) == nullptr);
    }
    {
        long long *const **n = nullptr;
        assert_noexcept2(n == nullptr);
        assert_noexcept2(upx::ptr_static_cast<void *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const void *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<int *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const int *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char **>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char **>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char ***>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char ***>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char *const>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char *const>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char *const *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char *const *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char **const *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char **const *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<char *const *const *>(n) == nullptr);
        assert_noexcept2(upx::ptr_static_cast<const char *const *const *>(n) == nullptr);
    }
}

TEST_CASE("upx::ptr_static_cast constexpr 1") {
    // check that casts work at compile-time
    using upx::ptr_static_cast;

    constexpr void *vp = nullptr;
    constexpr byte *bp = nullptr;
    constexpr int *ip = nullptr;
    constexpr double *dp = nullptr;
    static_assert((vp == ptr_static_cast<void *>(vp)));
    static_assert((bp == ptr_static_cast<byte *>(bp)));
    static_assert((ip == ptr_static_cast<int *>(ip)));
    static_assert((dp == ptr_static_cast<double *>(dp)));

    constexpr const void *vc = nullptr;
    const constexpr byte *bc = nullptr;
    constexpr const int *ic = nullptr;
    constexpr const double *dc = nullptr;
    static_assert((vc == ptr_static_cast<const void *>(vc)));
    static_assert((bc == ptr_static_cast<const byte *>(bc)));
    static_assert((ic == ptr_static_cast<const int *>(ic)));
    static_assert((dc == ptr_static_cast<const double *>(dc)));

    constexpr void **vpp = nullptr;
    constexpr byte **bpp = nullptr;
    constexpr int **ipp = nullptr;
    constexpr double **dpp = nullptr;
    static_assert((vpp == ptr_static_cast<void **>(vpp)));
    static_assert((bpp == ptr_static_cast<byte **>(bpp)));
    static_assert((ipp == ptr_static_cast<int **>(ipp)));
    static_assert((dpp == ptr_static_cast<double **>(dpp)));

    constexpr const void **vcp = nullptr;
    const constexpr byte **bcp = nullptr;
    constexpr const int **icp = nullptr;
    constexpr const double **dcp = nullptr;
    static_assert((vcp == ptr_static_cast<const void **>(vcp)));
    static_assert((bcp == ptr_static_cast<const byte **>(bcp)));
    static_assert((icp == ptr_static_cast<const int **>(icp)));
    static_assert((dcp == ptr_static_cast<const double **>(dcp)));

    constexpr void *const *vpc = nullptr;
    constexpr byte *const *bpc = nullptr;
    constexpr int *const *ipc = nullptr;
    constexpr double *const *dpc = nullptr;
    static_assert((vpc == ptr_static_cast<void *const *>(vpc)));
    static_assert((bpc == ptr_static_cast<byte *const *>(bpc)));
    static_assert((ipc == ptr_static_cast<int *const *>(ipc)));
    static_assert((dpc == ptr_static_cast<double *const *>(dpc)));

    constexpr const void *const *vcc = nullptr;
    const constexpr byte *const *bcc = nullptr;
    constexpr const int *const *icc = nullptr;
    constexpr const double *const *dcc = nullptr;
    static_assert((vcc == ptr_static_cast<const void *const *>(vcc)));
    static_assert((bcc == ptr_static_cast<const byte *const *>(bcc)));
    static_assert((icc == ptr_static_cast<const int *const *>(icc)));
    static_assert((dcc == ptr_static_cast<const double *const *>(dcc)));
}

TEST_CASE("upx::ptr_static_cast constexpr 2") {
    // check that casts work at compile-time
    using upx::ptr_static_cast;

    constexpr void *vp = nullptr;
    constexpr byte *bp = nullptr;
    constexpr int *ip = nullptr;
    constexpr double *dp = nullptr;
    static_assert((vp == static_cast<void *>(vp)));
    static_assert((vp == static_cast<void *>(bp)));
    static_assert((vp == static_cast<void *>(ip)));
    static_assert((vp == static_cast<void *>(dp)));
    static_assert((vp == ptr_static_cast<void *>(vp)));
    static_assert((vp == ptr_static_cast<void *>(bp)));
    static_assert((vp == ptr_static_cast<void *>(ip)));
    static_assert((vp == ptr_static_cast<void *>(dp)));

    constexpr const void *vc = nullptr;
    const constexpr byte *bc = nullptr;
    constexpr const int *ic = nullptr;
    constexpr const double *dc = nullptr;
    static_assert((vc == static_cast<const void *>(vp)));
    static_assert((vc == static_cast<const void *>(bp)));
    static_assert((vc == static_cast<const void *>(ip)));
    static_assert((vc == static_cast<const void *>(dp)));
    static_assert((vc == ptr_static_cast<const void *>(vp)));
    static_assert((vc == ptr_static_cast<const void *>(dp)));
    static_assert((vc == ptr_static_cast<const void *>(bp)));
    static_assert((vc == ptr_static_cast<const void *>(ip)));
    static_assert((vc == static_cast<const void *>(vc)));
    static_assert((vc == static_cast<const void *>(bc)));
    static_assert((vc == static_cast<const void *>(ic)));
    static_assert((vc == static_cast<const void *>(dc)));
    static_assert((vc == ptr_static_cast<const void *>(vc)));
    static_assert((vc == ptr_static_cast<const void *>(dc)));
    static_assert((vc == ptr_static_cast<const void *>(bc)));
    static_assert((vc == ptr_static_cast<const void *>(ic)));

    // these are invalid:
    //// constexpr char *cc1 = static_cast<char *>(bp);
    //// constexpr char *cc2 = ptr_static_cast<char *>(bp);
    //// constexpr unsigned *uc1 = static_cast<unsigned *>(ip);
    //// constexpr unsigned *uc2 = ptr_static_cast<unsigned *>(ip);
}

#if WITH_THREADS
TEST_CASE("upx::ptr_std_atomic_cast") {
    // pointer-size
    CHECK_EQ(upx::ptr_std_atomic_cast((void **) nullptr), nullptr);
    CHECK_EQ(upx::ptr_std_atomic_cast((uintptr_t *) nullptr), nullptr);
    CHECK_EQ(upx::ptr_std_atomic_cast((upx_uintptr_t *) nullptr), nullptr);
#if 1
    // more fundamental types
    CHECK_EQ(upx::ptr_std_atomic_cast((char *) nullptr), nullptr);
    CHECK_EQ(upx::ptr_std_atomic_cast((short *) nullptr), nullptr);
    CHECK_EQ(upx::ptr_std_atomic_cast((int *) nullptr), nullptr);
    CHECK_EQ(upx::ptr_std_atomic_cast((long *) nullptr), nullptr);
    CHECK_EQ(upx::ptr_std_atomic_cast((ptrdiff_t *) nullptr), nullptr);
    CHECK_EQ(upx::ptr_std_atomic_cast((size_t *) nullptr), nullptr);
    CHECK_EQ(upx::ptr_std_atomic_cast((upx_int8_t *) nullptr), nullptr);
    CHECK_EQ(upx::ptr_std_atomic_cast((upx_int16_t *) nullptr), nullptr);
    CHECK_EQ(upx::ptr_std_atomic_cast((upx_int32_t *) nullptr), nullptr);
#endif
}
#endif // WITH_THREADS

TEST_CASE("upx::atomic_exchange") {
    {
        upx_uintptr_t x = (upx_uintptr_t) 0 - 1;
        upx_uintptr_t y = upx::atomic_exchange(&x, (upx_uintptr_t) 2);
        CHECK_EQ(x, 2);
        CHECK_EQ(y, (upx_uintptr_t) 0 - 1);
        UNUSED(y);
    }
    {
        const int buf[2] = {101, 202};
        const int *ptr_array[2] = {&buf[0], &buf[1]};
        assert_noexcept(*ptr_array[0] == 101 && *ptr_array[1] == 202);
        const int *p = upx::atomic_exchange(&ptr_array[0], ptr_array[1]);
        CHECK_EQ(p, buf + 0);
        assert_noexcept(*ptr_array[0] == 202 && *ptr_array[1] == 202);
        p = upx::atomic_exchange(&ptr_array[1], p);
        CHECK_EQ(p, buf + 1);
        assert_noexcept(*ptr_array[0] == 202 && *ptr_array[1] == 101);
        UNUSED(p);
    }
}

TEST_CASE("upx::ObjectDeleter 1") {
    LE16 *o = nullptr; // object
    LE32 *a = nullptr; // array
    LE64 *m = nullptr; // malloc
    {
        // auto o_deleter = upx::ObjectDeleter(&o, 1);
        upx::ObjectDeleter<LE16> o_deleter = upx::ObjectDeleter(&o, 1);
        o = new LE16;
        assert(o != nullptr);
        auto a_deleter = upx::ArrayDeleter(&a, 1);
        a = New(LE32, 1);
        assert(a != nullptr);
        auto m_deleter = upx::MallocDeleter(&m, 1);
        m = (LE64 *) ::malloc(sizeof(LE64));
        assert(m != nullptr);
    }
    assert(o == nullptr);
    assert(a == nullptr);
    assert(m == nullptr);
    // test "const" versions
    {
        const auto o_deleter = upx::ObjectDeleter(&o, 1);
        o = new LE16;
        assert(o != nullptr);
        const auto a_deleter = upx::ArrayDeleter(&a, 1);
        a = New(LE32, 1);
        assert(a != nullptr);
        const auto m_deleter = upx::MallocDeleter(&m, 1);
        m = (LE64 *) ::malloc(sizeof(LE64));
        assert(m != nullptr);
    }
    assert(o == nullptr);
    assert(a == nullptr);
    assert(m == nullptr);
}

TEST_CASE("upx::ObjectDeleter 2") {
    constexpr size_t N = 2;
    BE16 *o[N]; // multiple objects
    BE32 *a[N]; // multiple arrays
    BE64 *m[N]; // multiple mallocs
    {
        // auto o_deleter = upx::ObjectDeleter(o, 0);
        upx::ObjectDeleter<BE16> o_deleter = upx::ObjectDeleter(o, 0);
        auto a_deleter = upx::ArrayDeleter(a, 0);
        auto m_deleter = upx::MallocDeleter(m, 0);
        for (size_t i = 0; i < N; i++) {
            o[i] = new BE16;
            assert(o[i] != nullptr);
            o_deleter.count += 1;
            a[i] = New(BE32, 1 + i);
            assert(a[i] != nullptr);
            a_deleter.count += 1;
            m[i] = (BE64 *) ::malloc(sizeof(BE64));
            assert(m[i] != nullptr);
            m_deleter.count += 1;
        }
    }
    for (size_t i = 0; i < N; i++) {
        assert(o[i] == nullptr);
        assert(a[i] == nullptr);
        assert(m[i] == nullptr);
    }
}

/*************************************************************************
// namespace upx::compile_time
**************************************************************************/

static_assert(upx::compile_time::string_len("") == 0);
static_assert(upx::compile_time::string_len("a") == 1);
static_assert(upx::compile_time::string_len("ab") == 2);
static_assert(upx::compile_time::string_len("abc") == 3);

static_assert(upx::compile_time::string_eq("", ""));
static_assert(!upx::compile_time::string_eq("a", ""));
static_assert(!upx::compile_time::string_eq("", "a"));
static_assert(upx::compile_time::string_eq("abc", "abc"));
static_assert(!upx::compile_time::string_eq("ab", "abc"));
static_assert(!upx::compile_time::string_eq("abc", "ab"));

static_assert(!upx::compile_time::string_lt("", ""));
static_assert(!upx::compile_time::string_lt("a", ""));
static_assert(upx::compile_time::string_lt("", "a"));
static_assert(!upx::compile_time::string_lt("abc", "abc"));
static_assert(upx::compile_time::string_lt("ab", "abc"));
static_assert(!upx::compile_time::string_lt("abc", "ab"));
static_assert(!upx::compile_time::string_lt("abc", "aba"));
static_assert(upx::compile_time::string_lt("abc", "abz"));

static_assert(upx::compile_time::string_ne("abc", "abz"));
static_assert(!upx::compile_time::string_gt("abc", "abz"));
static_assert(!upx::compile_time::string_ge("abc", "abz"));
static_assert(upx::compile_time::string_le("abc", "abz"));

static_assert(upx::compile_time::mem_eq((const char *) nullptr, (const char *) nullptr, 0));
static_assert(upx::compile_time::mem_eq((const char *) nullptr, (const byte *) nullptr, 0));
static_assert(upx::compile_time::mem_eq((const byte *) nullptr, (const char *) nullptr, 0));
static_assert(upx::compile_time::mem_eq((const byte *) nullptr, (const byte *) nullptr, 0));
static_assert(upx::compile_time::mem_eq("", "", 0));
static_assert(upx::compile_time::mem_eq("abc", "abc", 3));
static_assert(!upx::compile_time::mem_eq("abc", "abz", 3));

static_assert(upx::compile_time::bswap16(0x0102) == 0x0201);
static_assert(upx::compile_time::bswap32(0x01020304) == 0x04030201);
static_assert(upx::compile_time::bswap64(0x0102030405060708ull) == 0x0807060504030201ull);

namespace {
struct alignas(1) TestCT final {
    byte d[8]; // public data

    static constexpr TestCT makeBE16(upx_uint16_t v) noexcept {
        TestCT x = {};
        upx::compile_time::set_be16(x.d, v);
        return x;
    }
    static constexpr TestCT makeBE24(upx_uint32_t v) noexcept {
        TestCT x = {};
        upx::compile_time::set_be24(x.d, v);
        return x;
    }
    static constexpr TestCT makeBE32(upx_uint32_t v) noexcept {
        TestCT x = {};
        upx::compile_time::set_be32(x.d, v);
        return x;
    }
    static constexpr TestCT makeBE64(upx_uint64_t v) noexcept {
        TestCT x = {};
        upx::compile_time::set_be64(x.d, v);
        return x;
    }
    static constexpr TestCT makeLE16(upx_uint16_t v) noexcept {
        TestCT x = {};
        upx::compile_time::set_le16(x.d, v);
        return x;
    }
    static constexpr TestCT makeLE24(upx_uint32_t v) noexcept {
        TestCT x = {};
        upx::compile_time::set_le24(x.d, v);
        return x;
    }
    static constexpr TestCT makeLE32(upx_uint32_t v) noexcept {
        TestCT x = {};
        upx::compile_time::set_le32(x.d, v);
        return x;
    }
    static constexpr TestCT makeLE64(upx_uint64_t v) noexcept {
        TestCT x = {};
        upx::compile_time::set_le64(x.d, v);
        return x;
    }

    // run-time
    static noinline upx_uint16_t noinline_bswap16(upx_uint16_t v) noexcept {
        return upx::compile_time::bswap16(v);
    }
    static noinline upx_uint32_t noinline_bswap32(upx_uint32_t v) noexcept {
        return upx::compile_time::bswap32(v);
    }
    static noinline upx_uint64_t noinline_bswap64(upx_uint64_t v) noexcept {
        return upx::compile_time::bswap64(v);
    }

    static noinline upx_uint16_t noinline_get_be16(const byte *p) noexcept {
        return upx::compile_time::get_be16(p);
    }
    static noinline upx_uint32_t noinline_get_be24(const byte *p) noexcept {
        return upx::compile_time::get_be24(p);
    }
    static noinline upx_uint32_t noinline_get_be32(const byte *p) noexcept {
        return upx::compile_time::get_be32(p);
    }
    static noinline upx_uint64_t noinline_get_be64(const byte *p) noexcept {
        return upx::compile_time::get_be64(p);
    }
    static noinline upx_uint16_t noinline_get_le16(const byte *p) noexcept {
        return upx::compile_time::get_le16(p);
    }
    static noinline upx_uint32_t noinline_get_le24(const byte *p) noexcept {
        return upx::compile_time::get_le24(p);
    }
    static noinline upx_uint32_t noinline_get_le32(const byte *p) noexcept {
        return upx::compile_time::get_le32(p);
    }
    static noinline upx_uint64_t noinline_get_le64(const byte *p) noexcept {
        return upx::compile_time::get_le64(p);
    }

    static noinline void noinline_set_be16(byte * p, upx_uint16_t v) noexcept {
        upx::compile_time::set_be16(p, v);
    }
    static noinline void noinline_set_be24(byte * p, upx_uint32_t v) noexcept {
        upx::compile_time::set_be24(p, v);
    }
    static noinline void noinline_set_be32(byte * p, upx_uint32_t v) noexcept {
        upx::compile_time::set_be32(p, v);
    }
    static noinline void noinline_set_be64(byte * p, upx_uint64_t v) noexcept {
        upx::compile_time::set_be64(p, v);
    }
    static noinline void noinline_set_le16(byte * p, upx_uint16_t v) noexcept {
        upx::compile_time::set_le16(p, v);
    }
    static noinline void noinline_set_le24(byte * p, upx_uint32_t v) noexcept {
        upx::compile_time::set_le24(p, v);
    }
    static noinline void noinline_set_le32(byte * p, upx_uint32_t v) noexcept {
        upx::compile_time::set_le32(p, v);
    }
    static noinline void noinline_set_le64(byte * p, upx_uint64_t v) noexcept {
        upx::compile_time::set_le64(p, v);
    }

    static noinline bool noinline_has_single_bit(upx_ptraddr_t p) noexcept {
        return upx::has_single_bit(p);
    }

    static noinline upx_ptraddr_t noinline_align_down(upx_ptraddr_t p, upx_ptraddr_t a) noexcept {
        return upx::align_down(p, a);
    }
    static noinline upx_ptraddr_t noinline_align_down_gap(upx_ptraddr_t p, upx_ptraddr_t a)
        noexcept {
        return upx::align_down_gap(p, a);
    }
    static noinline upx_ptraddr_t noinline_align_up(upx_ptraddr_t p, upx_ptraddr_t a) noexcept {
        return upx::align_up(p, a);
    }
    static noinline upx_ptraddr_t noinline_align_up_gap(upx_ptraddr_t p, upx_ptraddr_t a) noexcept {
        return upx::align_up_gap(p, a);
    }

    static noinline upx_ptraddr_t noinline_align_down_16(upx_ptraddr_t p) noexcept {
        return upx::align_down(p, upx_ptraddr_t(16));
    }
    static noinline upx_ptraddr_t noinline_align_down_gap_16(upx_ptraddr_t p) noexcept {
        return upx::align_down_gap(p, upx_ptraddr_t(16));
    }
    static noinline upx_ptraddr_t noinline_align_up_16(upx_ptraddr_t p) noexcept {
        return upx::align_up(p, upx_ptraddr_t(16));
    }
    static noinline upx_ptraddr_t noinline_align_up_gap_16(upx_ptraddr_t p) noexcept {
        return upx::align_up_gap(p, upx_ptraddr_t(16));
    }

    static noinline byte *noinline_ptr_align_down(byte * p, size_t a) noexcept {
        return ptr_align_down(p, a);
    }
    static noinline byte *noinline_ptr_align_up(byte * p, size_t a) noexcept {
        return ptr_align_up(p, a);
    }

    static noinline byte *noinline_ptr_align_down_16(byte * p) noexcept {
        return ptr_align_down(p, 16);
    }
    static noinline byte *noinline_ptr_align_up_16(byte * p) noexcept {
        return ptr_align_up(p, 16);
    }
};
static_assert(sizeof(TestCT) == 8);
static_assert(alignof(TestCT) == 1);

static forceinline upx_int64_t get_me8_int64(const byte *p) noexcept {
    upx_int8_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}
static forceinline upx_uint64_t get_me8_uint64(const byte *p) noexcept {
    upx_int8_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}
static forceinline upx_int64_t get_ne8_int64(const byte *p) noexcept {
    upx_uint8_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}
static forceinline upx_uint64_t get_ne8_uint64(const byte *p) noexcept {
    upx_uint8_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}
static forceinline upx_int64_t get_me16_int64(const byte *p) noexcept {
    upx_int16_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}
static forceinline upx_uint64_t get_me16_uint64(const byte *p) noexcept {
    upx_int16_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}
static forceinline upx_int64_t get_ne16_int64(const byte *p) noexcept {
    upx_uint16_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}
static forceinline upx_uint64_t get_ne16_uint64(const byte *p) noexcept {
    upx_uint16_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}
static forceinline upx_int64_t get_me32_int64(const byte *p) noexcept {
    upx_int32_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}
static forceinline upx_uint64_t get_me32_uint64(const byte *p) noexcept {
    upx_int32_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}
static forceinline upx_int64_t get_ne32_int64(const byte *p) noexcept {
    upx_uint32_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}
static forceinline upx_uint64_t get_ne32_uint64(const byte *p) noexcept {
    upx_uint32_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}
#if (__SIZEOF_INT128__ == 16)
static forceinline upx_int128_t get_me32_int128(const byte *p) noexcept {
    upx_int32_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}
static forceinline upx_uint128_t get_me32_uint128(const byte *p) noexcept {
    upx_int32_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}
static forceinline upx_int128_t get_ne32_int128(const byte *p) noexcept {
    upx_uint32_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}
static forceinline upx_uint128_t get_ne32_uint128(const byte *p) noexcept {
    upx_uint32_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}
static forceinline upx_int128_t get_me64_int128(const byte *p) noexcept {
    upx_int64_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}
static forceinline upx_uint128_t get_me64_uint128(const byte *p) noexcept {
    upx_int64_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}
static forceinline upx_int128_t get_ne64_int128(const byte *p) noexcept {
    upx_uint64_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}
static forceinline upx_uint128_t get_ne64_uint128(const byte *p) noexcept {
    upx_uint64_t v = 0;
    upx_memcpy_inline(&v, p, sizeof(v));
    return v;
}
#endif

static forceinline constexpr int xign_extend32(unsigned v, unsigned bits) noexcept {
    return ACC_ICAST(int, v << (32 - bits)) >> (32 - bits);
}
static forceinline constexpr upx_int64_t xign_extend64(upx_uint64_t v, unsigned bits) noexcept {
    return ACC_ICAST(upx_int64_t, v << (64 - bits)) >> (64 - bits);
}

static forceinline constexpr int yign_extend32(unsigned v, unsigned bits) noexcept {
    const unsigned sign_bit = 1u << (bits - 1);
    return ACC_ICAST(int, (v & (sign_bit - 1)) - (v & sign_bit));
}
static forceinline constexpr upx_int64_t yign_extend64(upx_uint64_t v, unsigned bits) noexcept {
    const upx_uint64_t sign_bit = upx_uint64_t(1) << (bits - 1);
    return ACC_ICAST(upx_int64_t, (v & (sign_bit - 1)) - (v & sign_bit));
}

static forceinline constexpr unsigned x_unsign_extend32(unsigned v, unsigned bits) noexcept {
    return ACC_ICAST(unsigned, v << (32 - bits)) >> (32 - bits);
}
static forceinline constexpr upx_uint64_t x_unsign_extend64(upx_uint64_t v,
                                                            unsigned bits) noexcept {
    return ACC_ICAST(upx_uint64_t, v << (64 - bits)) >> (64 - bits);
}

struct alignas(1) TestXE final {
    byte d[8]; // public data

    static noinline upx_uint16_t noinline_bswap16(upx_uint16_t v) noexcept { return bswap16(v); }
    static noinline upx_uint32_t noinline_bswap32(upx_uint32_t v) noexcept { return bswap32(v); }
    static noinline upx_uint64_t noinline_bswap64(upx_uint64_t v) noexcept { return bswap64(v); }

    static noinline upx_uint16_t noinline_get_be16(const byte *p) noexcept { return get_be16(p); }
    static noinline upx_uint32_t noinline_get_be24(const byte *p) noexcept { return get_be24(p); }
    static noinline upx_uint32_t noinline_get_be32(const byte *p) noexcept { return get_be32(p); }
    static noinline upx_uint64_t noinline_get_be64(const byte *p) noexcept { return get_be64(p); }
    static noinline upx_uint16_t noinline_get_le16(const byte *p) noexcept { return get_le16(p); }
    static noinline upx_uint32_t noinline_get_le24(const byte *p) noexcept { return get_le24(p); }
    static noinline upx_uint32_t noinline_get_le32(const byte *p) noexcept { return get_le32(p); }
    static noinline upx_uint64_t noinline_get_le64(const byte *p) noexcept { return get_le64(p); }

    static noinline void noinline_set_be16(byte * p, upx_uint16_t v) noexcept { set_be16(p, v); }
    static noinline void noinline_set_be24(byte * p, upx_uint32_t v) noexcept { set_be24(p, v); }
    static noinline void noinline_set_be32(byte * p, upx_uint32_t v) noexcept { set_be32(p, v); }
    static noinline void noinline_set_be64(byte * p, upx_uint64_t v) noexcept { set_be64(p, v); }
    static noinline void noinline_set_le16(byte * p, upx_uint16_t v) noexcept { set_le16(p, v); }
    static noinline void noinline_set_le24(byte * p, upx_uint32_t v) noexcept { set_le24(p, v); }
    static noinline void noinline_set_le32(byte * p, upx_uint32_t v) noexcept { set_le32(p, v); }
    static noinline void noinline_set_le64(byte * p, upx_uint64_t v) noexcept { set_le64(p, v); }

    static noinline unsigned noinline_bswap16_unsigned(unsigned v) noexcept { return bswap16(v); }
    static noinline unsigned noinline_get_be16_unsigned(const byte *p) noexcept {
        return get_be16(p);
    }
    static noinline unsigned noinline_get_le16_unsigned(const byte *p) noexcept {
        return get_le16(p);
    }
    static noinline void noinline_set_be16_unsigned(byte * p, unsigned v) noexcept {
        set_be16(p, v);
    }
    static noinline void noinline_set_le16_unsigned(byte * p, unsigned v) noexcept {
        set_le16(p, v);
    }

    static noinline upx_int64_t noinline_get_me8_int64(const byte *p) noexcept {
        return get_me8_int64(p);
    }
    static noinline upx_uint64_t noinline_get_me8_uint64(const byte *p) noexcept {
        return get_me8_uint64(p);
    }
    static noinline upx_int64_t noinline_get_ne8_int64(const byte *p) noexcept {
        return get_ne8_int64(p);
    }
    static noinline upx_uint64_t noinline_get_ne8_uint64(const byte *p) noexcept {
        return get_ne8_uint64(p);
    }
    static noinline upx_int64_t noinline_get_me16_int64(const byte *p) noexcept {
        return get_me16_int64(p);
    }
    static noinline upx_uint64_t noinline_get_me16_uint64(const byte *p) noexcept {
        return get_me16_uint64(p);
    }
    static noinline upx_int64_t noinline_get_ne16_int64(const byte *p) noexcept {
        return get_ne16_int64(p);
    }
    static noinline upx_uint64_t noinline_get_ne16_uint64(const byte *p) noexcept {
        return get_ne16_uint64(p);
    }
    static noinline upx_int64_t noinline_get_me32_int64(const byte *p) noexcept {
        return get_me32_int64(p);
    }
    static noinline upx_uint64_t noinline_get_me32_uint64(const byte *p) noexcept {
        return get_me32_uint64(p);
    }
    static noinline upx_int64_t noinline_get_ne32_int64(const byte *p) noexcept {
        return get_ne32_int64(p);
    }
    static noinline upx_uint64_t noinline_get_ne32_uint64(const byte *p) noexcept {
        return get_ne32_uint64(p);
    }
#if (__SIZEOF_INT128__ == 16)
    static noinline upx_int128_t noinline_get_me32_int128(const byte *p) noexcept {
        return get_me32_int128(p);
    }
    static noinline upx_uint128_t noinline_get_me32_uint128(const byte *p) noexcept {
        return get_me32_uint128(p);
    }
    static noinline upx_int128_t noinline_get_ne32_int128(const byte *p) noexcept {
        return get_ne32_int128(p);
    }
    static noinline upx_uint128_t noinline_get_ne32_uint128(const byte *p) noexcept {
        return get_ne32_uint128(p);
    }
    static noinline upx_int128_t noinline_get_me64_int128(const byte *p) noexcept {
        return get_me64_int128(p);
    }
    static noinline upx_uint128_t noinline_get_me64_uint128(const byte *p) noexcept {
        return get_me64_uint128(p);
    }
    static noinline upx_int128_t noinline_get_ne64_int128(const byte *p) noexcept {
        return get_ne64_int128(p);
    }
    static noinline upx_uint128_t noinline_get_ne64_uint128(const byte *p) noexcept {
        return get_ne64_uint128(p);
    }
#endif

    static noinline int noinline_get_be16_signed(const byte *p) noexcept {
        return get_be16_signed(p);
    }
    static noinline int noinline_get_be24_signed(const byte *p) noexcept {
        return get_be24_signed(p);
    }
    static noinline int noinline_get_be32_signed(const byte *p) noexcept {
        return get_be32_signed(p);
    }
    static noinline upx_int64_t noinline_get_be64_signed(const byte *p) noexcept {
        return get_be64_signed(p);
    }
    static noinline int noinline_get_le16_signed(const byte *p) noexcept {
        return get_le16_signed(p);
    }
    static noinline int noinline_get_le24_signed(const byte *p) noexcept {
        return get_le24_signed(p);
    }
    static noinline int noinline_get_le32_signed(const byte *p) noexcept {
        return get_le32_signed(p);
    }
    static noinline upx_int64_t noinline_get_le64_signed(const byte *p) noexcept {
        return get_le64_signed(p);
    }

    static noinline BE16 noinline_make_be16(unsigned v) noexcept { return BE16::make(v); }
    static noinline BE32 noinline_make_be32(unsigned v) noexcept { return BE32::make(v); }
    static noinline BE64 noinline_make_be64(upx_uint64_t v) noexcept { return BE64::make(v); }
    static noinline LE16 noinline_make_le16(unsigned v) noexcept { return LE16::make(v); }
    static noinline LE32 noinline_make_le32(unsigned v) noexcept { return LE32::make(v); }
    static noinline LE64 noinline_make_le64(upx_uint64_t v) noexcept { return LE64::make(v); }

    static noinline bool noinline_equal_be16(BE16 a, unsigned v) noexcept {
        const BE16 b = BE16::make(v);
        return a == b;
    }
    static noinline bool noinline_equal_be32(BE32 a, unsigned v) noexcept {
        const BE32 b = BE32::make(v);
        return a == b;
    }
    static noinline bool noinline_equal_be64(BE64 a, upx_uint64_t v) noexcept {
        const BE64 b = BE64::make(v);
        return a == b;
    }
    static noinline bool noinline_equal_le16(LE16 a, unsigned v) noexcept {
        const LE16 b = LE16::make(v);
        return a == b;
    }
    static noinline bool noinline_equal_le32(LE32 a, unsigned v) noexcept {
        const LE32 b = LE32::make(v);
        return a == b;
    }
    static noinline bool noinline_equal_le64(LE64 a, upx_uint64_t v) noexcept {
        const LE64 b = LE64::make(v);
        return a == b;
    }

    static noinline bool noinline_less_be16(BE16 a, unsigned v) noexcept {
        const BE16 b = BE16::make(v);
        return a < b;
    }
    static noinline bool noinline_less_be32(BE32 a, unsigned v) noexcept {
        const BE32 b = BE32::make(v);
        return a < b;
    }
    static noinline bool noinline_less_be64(BE64 a, upx_uint64_t v) noexcept {
        const BE64 b = BE64::make(v);
        return a < b;
    }
    static noinline bool noinline_less_le16(LE16 a, unsigned v) noexcept {
        const LE16 b = LE16::make(v);
        return a < b;
    }
    static noinline bool noinline_less_le32(LE32 a, unsigned v) noexcept {
        const LE32 b = LE32::make(v);
        return a < b;
    }
    static noinline bool noinline_less_le64(LE64 a, upx_uint64_t v) noexcept {
        const LE64 b = LE64::make(v);
        return a < b;
    }

    static noinline int noinline_sign_extend32(unsigned v, unsigned bits) noexcept {
        return sign_extend32(v, bits);
    }
    static noinline upx_int64_t noinline_sign_extend64(upx_uint64_t v, unsigned bits) noexcept {
        return sign_extend64(v, bits);
    }

    static noinline int noinline_sign_extend32_4(unsigned v) noexcept {
        return sign_extend32(v, 4);
    }
    static noinline int noinline_sign_extend32_8(unsigned v) noexcept {
        return sign_extend32(v, 8);
    }
    static noinline int noinline_sign_extend32_16(unsigned v) noexcept {
        return sign_extend32(v, 16);
    }
    static noinline int noinline_sign_extend32_24(unsigned v) noexcept {
        return sign_extend32(v, 24);
    }
    static noinline int noinline_sign_extend32_32(unsigned v) noexcept {
        return sign_extend32(v, 32);
    }
    static noinline upx_int64_t noinline_sign_extend64_4(upx_uint64_t v) noexcept {
        return sign_extend64(v, 4);
    }
    static noinline upx_int64_t noinline_sign_extend64_8(upx_uint64_t v) noexcept {
        return sign_extend64(v, 8);
    }
    static noinline upx_int64_t noinline_sign_extend64_16(upx_uint64_t v) noexcept {
        return sign_extend64(v, 16);
    }
    static noinline upx_int64_t noinline_sign_extend64_24(upx_uint64_t v) noexcept {
        return sign_extend64(v, 24);
    }
    static noinline upx_int64_t noinline_sign_extend64_32(upx_uint64_t v) noexcept {
        return sign_extend64(v, 32);
    }
    static noinline upx_int64_t noinline_sign_extend64_48(upx_uint64_t v) noexcept {
        return sign_extend64(v, 48);
    }
    static noinline upx_int64_t noinline_sign_extend64_64(upx_uint64_t v) noexcept {
        return sign_extend64(v, 64);
    }

    static noinline int noinline_xign_extend32(unsigned v, unsigned bits) noexcept {
        return xign_extend32(v, bits);
    }
    static noinline upx_int64_t noinline_xign_extend64(upx_uint64_t v, unsigned bits) noexcept {
        return xign_extend64(v, bits);
    }

    static noinline int noinline_xign_extend32_4(unsigned v) noexcept {
        return xign_extend32(v, 4);
    }
    static noinline int noinline_xign_extend32_8(unsigned v) noexcept {
        return xign_extend32(v, 8);
    }
    static noinline int noinline_xign_extend32_16(unsigned v) noexcept {
        return xign_extend32(v, 16);
    }
    static noinline int noinline_xign_extend32_24(unsigned v) noexcept {
        return xign_extend32(v, 24);
    }
    static noinline int noinline_xign_extend32_32(unsigned v) noexcept {
        return xign_extend32(v, 32);
    }
    static noinline upx_int64_t noinline_xign_extend64_4(upx_uint64_t v) noexcept {
        return xign_extend64(v, 4);
    }
    static noinline upx_int64_t noinline_xign_extend64_8(upx_uint64_t v) noexcept {
        return xign_extend64(v, 8);
    }
    static noinline upx_int64_t noinline_xign_extend64_16(upx_uint64_t v) noexcept {
        return xign_extend64(v, 16);
    }
    static noinline upx_int64_t noinline_xign_extend64_24(upx_uint64_t v) noexcept {
        return xign_extend64(v, 24);
    }
    static noinline upx_int64_t noinline_xign_extend64_32(upx_uint64_t v) noexcept {
        return xign_extend64(v, 32);
    }
    static noinline upx_int64_t noinline_xign_extend64_48(upx_uint64_t v) noexcept {
        return xign_extend64(v, 48);
    }
    static noinline upx_int64_t noinline_xign_extend64_64(upx_uint64_t v) noexcept {
        return xign_extend64(v, 64);
    }

    static noinline int noinline_yign_extend32(unsigned v, unsigned bits) noexcept {
        return yign_extend32(v, bits);
    }
    static noinline upx_int64_t noinline_yign_extend64(upx_uint64_t v, unsigned bits) noexcept {
        return yign_extend64(v, bits);
    }

    static noinline int noinline_yign_extend32_4(unsigned v) noexcept {
        return yign_extend32(v, 4);
    }
    static noinline int noinline_yign_extend32_8(unsigned v) noexcept {
        return yign_extend32(v, 8);
    }
    static noinline int noinline_yign_extend32_16(unsigned v) noexcept {
        return yign_extend32(v, 16);
    }
    static noinline int noinline_yign_extend32_24(unsigned v) noexcept {
        return yign_extend32(v, 24);
    }
    static noinline int noinline_yign_extend32_32(unsigned v) noexcept {
        return yign_extend32(v, 32);
    }
    static noinline upx_int64_t noinline_yign_extend64_4(upx_uint64_t v) noexcept {
        return yign_extend64(v, 4);
    }
    static noinline upx_int64_t noinline_yign_extend64_8(upx_uint64_t v) noexcept {
        return yign_extend64(v, 8);
    }
    static noinline upx_int64_t noinline_yign_extend64_16(upx_uint64_t v) noexcept {
        return yign_extend64(v, 16);
    }
    static noinline upx_int64_t noinline_yign_extend64_24(upx_uint64_t v) noexcept {
        return yign_extend64(v, 24);
    }
    static noinline upx_int64_t noinline_yign_extend64_32(upx_uint64_t v) noexcept {
        return yign_extend64(v, 32);
    }
    static noinline upx_int64_t noinline_yign_extend64_48(upx_uint64_t v) noexcept {
        return yign_extend64(v, 48);
    }
    static noinline upx_int64_t noinline_yign_extend64_64(upx_uint64_t v) noexcept {
        return yign_extend64(v, 64);
    }

    static noinline unsigned noinline_x_unsign_extend32(unsigned v, unsigned bits) noexcept {
        return x_unsign_extend32(v, bits);
    }
    static noinline upx_uint64_t noinline_x_unsign_extend64(upx_uint64_t v, unsigned bits)
        noexcept {
        return x_unsign_extend64(v, bits);
    }

    static noinline unsigned noinline_x_unsign_extend32_4(unsigned v) noexcept {
        return x_unsign_extend32(v, 4);
    }
    static noinline unsigned noinline_x_unsign_extend32_8(unsigned v) noexcept {
        return x_unsign_extend32(v, 8);
    }
    static noinline unsigned noinline_x_unsign_extend32_16(unsigned v) noexcept {
        return x_unsign_extend32(v, 16);
    }
    static noinline unsigned noinline_x_unsign_extend32_24(unsigned v) noexcept {
        return x_unsign_extend32(v, 24);
    }
    static noinline unsigned noinline_x_unsign_extend32_32(unsigned v) noexcept {
        return x_unsign_extend32(v, 32);
    }
    static noinline upx_uint64_t noinline_x_unsign_extend64_4(upx_uint64_t v) noexcept {
        return x_unsign_extend64(v, 4);
    }
    static noinline upx_uint64_t noinline_x_unsign_extend64_8(upx_uint64_t v) noexcept {
        return x_unsign_extend64(v, 8);
    }
    static noinline upx_uint64_t noinline_x_unsign_extend64_16(upx_uint64_t v) noexcept {
        return x_unsign_extend64(v, 16);
    }
    static noinline upx_uint64_t noinline_x_unsign_extend64_24(upx_uint64_t v) noexcept {
        return x_unsign_extend64(v, 24);
    }
    static noinline upx_uint64_t noinline_x_unsign_extend64_32(upx_uint64_t v) noexcept {
        return x_unsign_extend64(v, 32);
    }
    static noinline upx_uint64_t noinline_x_unsign_extend64_48(upx_uint64_t v) noexcept {
        return x_unsign_extend64(v, 48);
    }
    static noinline upx_uint64_t noinline_x_unsign_extend64_64(upx_uint64_t v) noexcept {
        return x_unsign_extend64(v, 64);
    }

    static noinline bool noinline_mem_size_valid_bytes(upx_uint64_t bytes) noexcept {
        return mem_size_valid_bytes(bytes);
    }
    static noinline upx_rsize_t noinline_mem_size(upx_uint64_t element_size, upx_uint64_t n)
        may_throw {
        return mem_size(element_size, n);
    }
    static noinline upx_rsize_t noinline_mem_size_get_n(upx_uint64_t element_size, upx_uint64_t n)
        may_throw {
        return mem_size_get_n(element_size, n);
    }

    static noinline void noinline_memcpy_0(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 0);
    }
    static noinline void noinline_memcpy_1(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 1);
    }
    static noinline void noinline_memcpy_2(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 2);
    }
    static noinline void noinline_memcpy_4(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 4);
    }
    static noinline void noinline_memcpy_8(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 8);
    }
    static noinline void noinline_memcpy_16(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 16);
    }
    static noinline void noinline_memcpy_32(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 32);
    }
    static noinline void noinline_memcpy_64(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 64);
    }
    static noinline void noinline_memcpy_128(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 128);
    }
    static noinline void noinline_memcpy_256(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 256);
    }
    static noinline void noinline_memcpy_512(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 512);
    }
    static noinline void noinline_memcpy_1024(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 1024);
    }
    static noinline void noinline_memcpy_65536(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 65536);
    }
    static noinline void noinline_memcpy_1048576(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 1048576);
    }

    static noinline void noinline_memcpy_3(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 3);
    }
    static noinline void noinline_memcpy_6(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 6);
    }
    static noinline void noinline_memcpy_12(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 12);
    }
    static noinline void noinline_memcpy_24(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 24);
    }
    static noinline void noinline_memcpy_48(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 48);
    }
    static noinline void noinline_memcpy_96(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 96);
    }
    static noinline void noinline_memcpy_192(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 192);
    }
    static noinline void noinline_memcpy_384(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 384);
    }
    static noinline void noinline_memcpy_768(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 768);
    }

    static noinline void noinline_memcpy_7(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 7);
    }
    static noinline void noinline_memcpy_13(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 13);
    }
    static noinline void noinline_memcpy_25(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 25);
    }
    static noinline void noinline_memcpy_49(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 49);
    }
    static noinline void noinline_memcpy_97(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 97);
    }
    static noinline void noinline_memcpy_193(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 193);
    }
    static noinline void noinline_memcpy_385(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 385);
    }
    static noinline void noinline_memcpy_769(void *d, const void *s) noexcept {
        upx_memcpy_inline(d, s, 769);
    }
};
} // namespace

TEST_CASE("upx::compile_time") {
    constexpr upx_uint16_t v16 = 0x0201;
    constexpr upx_uint32_t v24 = 0x030201;
    constexpr upx_uint32_t v32 = 0x04030201;
    constexpr upx_uint64_t v64 = 0x0807060504030201ull;
    {
        static_assert(upx::compile_time::bswap16(v16) == 0x0102);
        static_assert(upx::compile_time::bswap32(v32) == 0x01020304);
        static_assert(upx::compile_time::bswap64(v64) == 0x0102030405060708ull);
        assert_noexcept(TestCT::noinline_bswap16(v16) == 0x0102);
        assert_noexcept(TestCT::noinline_bswap32(v32) == 0x01020304);
        assert_noexcept(TestCT::noinline_bswap64(v64) == 0x0102030405060708ull);
    }
    {
        const constexpr byte d[8] = {1, 2, 3, 4, 5, 6, 7, 8};
        static_assert(upx::compile_time::get_be16(d) == 0x0102);
        static_assert(upx::compile_time::get_be24(d) == 0x010203);
        static_assert(upx::compile_time::get_be32(d) == 0x01020304);
        static_assert(upx::compile_time::get_be64(d) == 0x0102030405060708ull);
        static_assert(upx::compile_time::get_le16(d) == 0x0201);
        static_assert(upx::compile_time::get_le24(d) == 0x030201);
        static_assert(upx::compile_time::get_le32(d) == 0x04030201);
        static_assert(upx::compile_time::get_le64(d) == 0x0807060504030201ull);
    }
    {
        using upx::compile_time::mem_eq;
        upx_alignas_max byte aligned_buffer[16] = {};
        upx::compile_time::mem_set(aligned_buffer, 0xff, 16);
        assert_noexcept(mem_eq(aligned_buffer, aligned_buffer + 8, 8));
        byte *const buf = aligned_buffer + 7;

        constexpr auto be16 = TestCT::makeBE16(v16);
        static_assert(upx::compile_time::get_be16(be16.d) == v16);
        static_assert(mem_eq(be16.d, "\x02\x01", 2));
        upx::compile_time::mem_clear(buf, 8);
        TestCT::noinline_set_be16(buf, v16);
        assert_noexcept(TestCT::noinline_get_be16(buf) == v16);
        assert_noexcept(upx::compile_time::get_be16(buf) == v16);
        assert_noexcept(memcmp(buf, be16.d, 8) == 0);

        constexpr auto be24 = TestCT::makeBE24(v24);
        static_assert(upx::compile_time::get_be24(be24.d) == v24);
        static_assert(mem_eq(be24.d, "\x03\x02\x01", 3));
        upx::compile_time::mem_clear(buf, 8);
        TestCT::noinline_set_be24(buf, v24);
        assert_noexcept(TestCT::noinline_get_be24(buf) == v24);
        assert_noexcept(upx::compile_time::get_be24(buf) == v24);
        assert_noexcept(memcmp(buf, be24.d, 8) == 0);

        constexpr auto be32 = TestCT::makeBE32(v32);
        static_assert(upx::compile_time::get_be32(be32.d) == v32);
        static_assert(mem_eq(be32.d, "\x04\x03\x02\x01", 4));
        upx::compile_time::mem_clear(buf, 8);
        TestCT::noinline_set_be32(buf, v32);
        assert_noexcept(TestCT::noinline_get_be32(buf) == v32);
        assert_noexcept(upx::compile_time::get_be32(buf) == v32);
        assert_noexcept(memcmp(buf, be32.d, 8) == 0);

        constexpr auto be64 = TestCT::makeBE64(v64);
        static_assert(upx::compile_time::get_be64(be64.d) == v64);
        static_assert(mem_eq(be64.d, "\x08\x07\x06\x05\x04\x03\x02\x01", 8));
        upx::compile_time::mem_clear(buf, 8);
        TestCT::noinline_set_be64(buf, v64);
        assert_noexcept(TestCT::noinline_get_be64(buf) == v64);
        assert_noexcept(upx::compile_time::get_be64(buf) == v64);
        assert_noexcept(memcmp(buf, be64.d, 8) == 0);

        constexpr auto le16 = TestCT::makeLE16(v16);
        static_assert(upx::compile_time::get_le16(le16.d) == v16);
        static_assert(mem_eq(le16.d, "\x01\x02", 2));
        upx::compile_time::mem_clear(buf, 8);
        TestCT::noinline_set_le16(buf, v16);
        assert_noexcept(TestCT::noinline_get_le16(buf) == v16);
        assert_noexcept(upx::compile_time::get_le16(buf) == v16);
        assert_noexcept(memcmp(buf, le16.d, 8) == 0);

        constexpr auto le24 = TestCT::makeLE24(v24);
        static_assert(upx::compile_time::get_le24(le24.d) == v24);
        static_assert(mem_eq(le24.d, "\x01\x02\x03", 3));
        upx::compile_time::mem_clear(buf, 8);
        TestCT::noinline_set_le24(buf, v24);
        assert_noexcept(TestCT::noinline_get_le24(buf) == v24);
        assert_noexcept(upx::compile_time::get_le24(buf) == v24);
        assert_noexcept(memcmp(buf, le24.d, 8) == 0);

        constexpr auto le32 = TestCT::makeLE32(v32);
        static_assert(upx::compile_time::get_le32(le32.d) == v32);
        static_assert(mem_eq(le32.d, "\x01\x02\x03\x04", 4));
        upx::compile_time::mem_clear(buf, 8);
        TestCT::noinline_set_le32(buf, v32);
        assert_noexcept(TestCT::noinline_get_le32(buf) == v32);
        assert_noexcept(upx::compile_time::get_le32(buf) == v32);
        assert_noexcept(memcmp(buf, le32.d, 8) == 0);

        constexpr auto le64 = TestCT::makeLE64(v64);
        static_assert(upx::compile_time::get_le64(le64.d) == v64);
        static_assert(mem_eq(le64.d, "\x01\x02\x03\x04\x05\x06\x07\x08", 8));
        upx::compile_time::mem_clear(buf, 8);
        TestCT::noinline_set_le64(buf, v64);
        assert_noexcept(TestCT::noinline_get_le64(buf) == v64);
        assert_noexcept(upx::compile_time::get_le64(buf) == v64);
        assert_noexcept(memcmp(buf, le64.d, 8) == 0);

        static_assert(upx::compile_time::bswap16(upx::compile_time::get_be16(be16.d)) ==
                      upx::compile_time::get_be16(le16.d));
        static_assert(upx::compile_time::bswap32(upx::compile_time::get_be32(be32.d)) ==
                      upx::compile_time::get_be32(le32.d));
        static_assert(upx::compile_time::bswap64(upx::compile_time::get_be64(be64.d)) ==
                      upx::compile_time::get_be64(le64.d));
        assert_noexcept(TestCT::noinline_bswap16(TestCT::noinline_get_be16(be16.d)) ==
                        TestCT::noinline_get_be16(le16.d));
        assert_noexcept(TestCT::noinline_bswap32(TestCT::noinline_get_be32(be32.d)) ==
                        TestCT::noinline_get_be32(le32.d));
        assert_noexcept(TestCT::noinline_bswap64(TestCT::noinline_get_be64(be64.d)) ==
                        TestCT::noinline_get_be64(le64.d));
    }
}

TEST_CASE("upx::run_time 1a") {
    const upx_uint16_t v16 = upx_uint16_t(acc_vget_acc_int64l_t(0xf2f1, 0));
    const upx_uint32_t v24 = upx_uint32_t(acc_vget_acc_int64l_t(0xf3f2f1, 0));
    const upx_uint32_t v32 = upx_uint32_t(acc_vget_acc_int64l_t(0xf4f3f2f1, 0));
    const upx_uint64_t v64 = upx_uint64_t(acc_vget_acc_int64l_t(0xf8f7f6f5f4f3f2f1ull, 0));
    {
        assert_noexcept2(TestCT::noinline_bswap16(v16) == 0xf1f2);
        assert_noexcept2(TestCT::noinline_bswap32(v32) == 0xf1f2f3f4);
        assert_noexcept2(TestCT::noinline_bswap64(v64) == 0xf1f2f3f4f5f6f7f8ull);
    }
    {
        upx_alignas_max byte aligned_buffer[32];
        byte *const buf1 = aligned_buffer + 3;
        byte *const buf2 = aligned_buffer + 13;

        upx::compile_time::set_be16(buf1, v16);
        TestCT::noinline_set_be16(buf2, v16);
        assert_noexcept(TestCT::noinline_get_be16(buf1) == v16);
        assert_noexcept(TestCT::noinline_get_be16(buf2) == v16);
        assert_noexcept(memcmp(buf1, buf2, 2) == 0);
        upx::compile_time::set_be24(buf1, v24);
        TestCT::noinline_set_be24(buf2, v24);
        assert_noexcept(TestCT::noinline_get_be24(buf1) == v24);
        assert_noexcept(TestCT::noinline_get_be24(buf2) == v24);
        assert_noexcept(memcmp(buf1, buf2, 3) == 0);
        upx::compile_time::set_be32(buf1, v32);
        TestCT::noinline_set_be32(buf2, v32);
        assert_noexcept(TestCT::noinline_get_be32(buf1) == v32);
        assert_noexcept(TestCT::noinline_get_be32(buf2) == v32);
        assert_noexcept(memcmp(buf1, buf2, 4) == 0);
        upx::compile_time::set_be64(buf1, v64);
        TestCT::noinline_set_be64(buf2, v64);
        assert_noexcept(TestCT::noinline_get_be64(buf1) == v64);
        assert_noexcept(TestCT::noinline_get_be64(buf2) == v64);
        assert_noexcept(memcmp(buf1, buf2, 8) == 0);

        upx::compile_time::set_le16(buf1, v16);
        TestCT::noinline_set_le16(buf2, v16);
        assert_noexcept(TestCT::noinline_get_le16(buf1) == v16);
        assert_noexcept(TestCT::noinline_get_le16(buf2) == v16);
        assert_noexcept(memcmp(buf1, buf2, 2) == 0);
        upx::compile_time::set_le24(buf1, v24);
        TestCT::noinline_set_le24(buf2, v24);
        assert_noexcept(TestCT::noinline_get_le24(buf1) == v24);
        assert_noexcept(TestCT::noinline_get_le24(buf2) == v24);
        assert_noexcept(memcmp(buf1, buf2, 3) == 0);
        upx::compile_time::set_le32(buf1, v32);
        TestCT::noinline_set_le32(buf2, v32);
        assert_noexcept(TestCT::noinline_get_le32(buf1) == v32);
        assert_noexcept(TestCT::noinline_get_le32(buf2) == v32);
        assert_noexcept(memcmp(buf1, buf2, 4) == 0);
        upx::compile_time::set_le64(buf1, v64);
        TestCT::noinline_set_le64(buf2, v64);
        assert_noexcept(TestCT::noinline_get_le64(buf1) == v64);
        assert_noexcept(TestCT::noinline_get_le64(buf2) == v64);
        assert_noexcept(memcmp(buf1, buf2, 8) == 0);
    }
}

TEST_CASE("upx::run_time 1b") {
    const upx_uint16_t v16 = upx_uint16_t(acc_vget_acc_int64l_t(0xf2f1, 0));
    const upx_uint32_t v24 = upx_uint32_t(acc_vget_acc_int64l_t(0xf3f2f1, 0));
    const upx_uint32_t v32 = upx_uint32_t(acc_vget_acc_int64l_t(0xf4f3f2f1, 0));
    const upx_uint64_t v64 = upx_uint64_t(acc_vget_acc_int64l_t(0xf8f7f6f5f4f3f2f1ull, 0));
    {
        assert_noexcept2(TestXE::noinline_bswap16(v16) == 0xf1f2);
        assert_noexcept2(TestXE::noinline_bswap32(v32) == 0xf1f2f3f4);
        assert_noexcept2(TestXE::noinline_bswap64(v64) == 0xf1f2f3f4f5f6f7f8ull);
        assert_noexcept2(TestXE::noinline_bswap16_unsigned(v16) == 0xf1f2);
    }
    {
        upx_alignas_max byte aligned_buffer[32];
        byte *const buf1 = aligned_buffer + 3;
        byte *const buf2 = aligned_buffer + 13;

        set_be16(buf1, v16);
        TestXE::noinline_set_be16(buf2, v16);
        assert_noexcept(TestXE::noinline_get_be16(buf1) == v16);
        assert_noexcept(TestXE::noinline_get_be16_unsigned(buf2) == v16);
        assert_noexcept(memcmp(buf1, buf2, 2) == 0);
        set_be24(buf1, v24);
        TestXE::noinline_set_be24(buf2, v24);
        assert_noexcept(TestXE::noinline_get_be24(buf1) == v24);
        assert_noexcept(TestXE::noinline_get_be24(buf2) == v24);
        assert_noexcept(memcmp(buf1, buf2, 3) == 0);
        set_be32(buf1, v32);
        TestXE::noinline_set_be32(buf2, v32);
        assert_noexcept(TestXE::noinline_get_be32(buf1) == v32);
        assert_noexcept(TestXE::noinline_get_be32(buf2) == v32);
        assert_noexcept(memcmp(buf1, buf2, 4) == 0);
        set_be64(buf1, v64);
        TestXE::noinline_set_be64(buf2, v64);
        assert_noexcept(TestXE::noinline_get_be64(buf1) == v64);
        assert_noexcept(TestXE::noinline_get_be64(buf2) == v64);
        assert_noexcept(memcmp(buf1, buf2, 8) == 0);

        set_le16(buf1, v16);
        TestXE::noinline_set_le16(buf2, v16);
        assert_noexcept(TestXE::noinline_get_le16(buf1) == v16);
        assert_noexcept(TestXE::noinline_get_le16_unsigned(buf2) == v16);
        assert_noexcept(memcmp(buf1, buf2, 2) == 0);
        set_le24(buf1, v24);
        TestXE::noinline_set_le24(buf2, v24);
        assert_noexcept(TestXE::noinline_get_le24(buf1) == v24);
        assert_noexcept(TestXE::noinline_get_le24(buf2) == v24);
        assert_noexcept(memcmp(buf1, buf2, 3) == 0);
        set_le32(buf1, v32);
        TestXE::noinline_set_le32(buf2, v32);
        assert_noexcept(TestXE::noinline_get_le32(buf1) == v32);
        assert_noexcept(TestXE::noinline_get_le32(buf2) == v32);
        assert_noexcept(memcmp(buf1, buf2, 4) == 0);
        set_le64(buf1, v64);
        TestXE::noinline_set_le64(buf2, v64);
        assert_noexcept(TestXE::noinline_get_le64(buf1) == v64);
        assert_noexcept(TestXE::noinline_get_le64(buf2) == v64);
        assert_noexcept(memcmp(buf1, buf2, 8) == 0);

        TestXE::noinline_set_be16_unsigned(buf2, v16);
        TestXE::noinline_set_le16_unsigned(buf2, v16);

        assert_noexcept2(TestXE::noinline_get_me8_int64(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_me8_uint64(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_ne8_int64(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_ne8_uint64(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_me16_int64(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_me16_uint64(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_ne16_int64(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_ne16_uint64(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_me32_int64(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_me32_uint64(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_ne32_int64(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_ne32_uint64(buf1) != 0);
#if (__SIZEOF_INT128__ == 16)
        assert_noexcept2(TestXE::noinline_get_me32_int128(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_me32_uint128(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_ne32_int128(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_ne32_uint128(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_me64_int128(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_me64_uint128(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_ne64_int128(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_ne64_uint128(buf1) != 0);
#endif

        assert_noexcept2(TestXE::noinline_get_be16_signed(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_be24_signed(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_be32_signed(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_be64_signed(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_le16_signed(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_le24_signed(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_le32_signed(buf1) != 0);
        assert_noexcept2(TestXE::noinline_get_le64_signed(buf1) != 0);
    }
    {
        assert_noexcept2(TestXE::noinline_make_be16(v32) == 0xf2f1);
        assert_noexcept2(TestXE::noinline_make_be32(v32) == 0xf4f3f2f1);
        assert_noexcept2(TestXE::noinline_make_be64(v64) == 0xf8f7f6f5f4f3f2f1ull);
        assert_noexcept2(TestXE::noinline_make_le16(v32) == 0xf2f1);
        assert_noexcept2(TestXE::noinline_make_le32(v32) == 0xf4f3f2f1);
        assert_noexcept2(TestXE::noinline_make_le64(v64) == 0xf8f7f6f5f4f3f2f1ull);
        const int n = acc_vget_int(0, 0);
        assert_noexcept2(TestXE::noinline_make_be16(n) == 0);
        assert_noexcept2(TestXE::noinline_make_be32(n) == 0);
        assert_noexcept2(TestXE::noinline_make_be64(n) == 0);
        assert_noexcept2(TestXE::noinline_make_le16(n) == 0);
        assert_noexcept2(TestXE::noinline_make_le32(n) == 0);
        assert_noexcept2(TestXE::noinline_make_le64(n) == 0);
    }
    {
        const BE16 be16 = BE16::make(acc_vget_int(v16, 0));
        const BE32 be32 = BE32::make(acc_vget_int(v32, 0));
        const BE64 be64 = BE64::make(acc_vget_acc_int64l_t(v64, 0));
        const LE16 le16 = LE16::make(acc_vget_int(v16, 0));
        const LE32 le32 = LE32::make(acc_vget_int(v32, 0));
        const LE64 le64 = LE64::make(acc_vget_acc_int64l_t(v64, 0));
        assert_noexcept2(TestXE::noinline_equal_be16(be16, v32));
        assert_noexcept2(TestXE::noinline_equal_be32(be32, v32));
        assert_noexcept2(TestXE::noinline_equal_be64(be64, v64));
        assert_noexcept2(TestXE::noinline_equal_le16(le16, v32));
        assert_noexcept2(TestXE::noinline_equal_le32(le32, v32));
        assert_noexcept2(TestXE::noinline_equal_le64(le64, v64));
        assert_noexcept2(!TestXE::noinline_less_be16(be16, v32));
        assert_noexcept2(!TestXE::noinline_less_be32(be32, v32));
        assert_noexcept2(!TestXE::noinline_less_be64(be64, v64));
        assert_noexcept2(!TestXE::noinline_less_le16(le16, v32));
        assert_noexcept2(!TestXE::noinline_less_le32(le32, v32));
        assert_noexcept2(!TestXE::noinline_less_le64(le64, v64));
    }
    {
        for (int i = -8; i < 8; i++) {
            {
                const unsigned u = i;
                assert_noexcept(TestXE::noinline_sign_extend32(u, 1) == ((i & 1) ? -1 : 0));
                assert_noexcept(TestXE::noinline_sign_extend32(u, 4 + (i & 1)) == i);
                assert_noexcept(TestXE::noinline_sign_extend32_4(u) == i);
                assert_noexcept(TestXE::noinline_sign_extend32_8(u) == i);
                assert_noexcept(TestXE::noinline_sign_extend32_16(u) == i);
                assert_noexcept(TestXE::noinline_sign_extend32_24(u) == i);
                assert_noexcept(TestXE::noinline_sign_extend32_32(u) == i);
                assert_noexcept(TestXE::noinline_xign_extend32(u, 1) == ((i & 1) ? -1 : 0));
                assert_noexcept(TestXE::noinline_xign_extend32(u, 4 + (i & 1)) == i);
                assert_noexcept(TestXE::noinline_xign_extend32_4(u) == i);
                assert_noexcept(TestXE::noinline_xign_extend32_8(u) == i);
                assert_noexcept(TestXE::noinline_xign_extend32_16(u) == i);
                assert_noexcept(TestXE::noinline_xign_extend32_24(u) == i);
                assert_noexcept(TestXE::noinline_xign_extend32_32(u) == i);
                assert_noexcept(TestXE::noinline_yign_extend32(u, 1) == ((i & 1) ? -1 : 0));
                assert_noexcept(TestXE::noinline_yign_extend32(u, 4 + (i & 1)) == i);
                assert_noexcept(TestXE::noinline_yign_extend32_4(u) == i);
                assert_noexcept(TestXE::noinline_yign_extend32_8(u) == i);
                assert_noexcept(TestXE::noinline_yign_extend32_16(u) == i);
                assert_noexcept(TestXE::noinline_yign_extend32_24(u) == i);
                assert_noexcept(TestXE::noinline_yign_extend32_32(u) == i);
                if (i >= 0) {
                    assert_noexcept(TestXE::noinline_x_unsign_extend32(u, 4 + (i & 1)) == u);
                    assert_noexcept(TestXE::noinline_x_unsign_extend32_4(u) == u);
                    assert_noexcept(TestXE::noinline_x_unsign_extend32_8(u) == u);
                    assert_noexcept(TestXE::noinline_x_unsign_extend32_16(u) == u);
                    assert_noexcept(TestXE::noinline_x_unsign_extend32_24(u) == u);
                    assert_noexcept(TestXE::noinline_x_unsign_extend32_32(u) == u);
                }
            }
            {
                const upx_uint64_t u = i;
                assert_noexcept(TestXE::noinline_sign_extend64(u, 1) == ((i & 1) ? -1 : 0));
                assert_noexcept(TestXE::noinline_sign_extend64(u, 4 + (i & 1)) == i);
                assert_noexcept(TestXE::noinline_sign_extend64_4(u) == i);
                assert_noexcept(TestXE::noinline_sign_extend64_8(u) == i);
                assert_noexcept(TestXE::noinline_sign_extend64_16(u) == i);
                assert_noexcept(TestXE::noinline_sign_extend64_24(u) == i);
                assert_noexcept(TestXE::noinline_sign_extend64_32(u) == i);
                assert_noexcept(TestXE::noinline_sign_extend64_48(u) == i);
                assert_noexcept(TestXE::noinline_sign_extend64_64(u) == i);
                assert_noexcept(TestXE::noinline_xign_extend64(u, 1) == ((i & 1) ? -1 : 0));
                assert_noexcept(TestXE::noinline_xign_extend64(u, 4 + (i & 1)) == i);
                assert_noexcept(TestXE::noinline_xign_extend64_4(u) == i);
                assert_noexcept(TestXE::noinline_xign_extend64_8(u) == i);
                assert_noexcept(TestXE::noinline_xign_extend64_16(u) == i);
                assert_noexcept(TestXE::noinline_xign_extend64_24(u) == i);
                assert_noexcept(TestXE::noinline_xign_extend64_32(u) == i);
                assert_noexcept(TestXE::noinline_xign_extend64_48(u) == i);
                assert_noexcept(TestXE::noinline_xign_extend64_64(u) == i);
                assert_noexcept(TestXE::noinline_yign_extend64(u, 1) == ((i & 1) ? -1 : 0));
                assert_noexcept(TestXE::noinline_yign_extend64(u, 4 + (i & 1)) == i);
                assert_noexcept(TestXE::noinline_yign_extend64_4(u) == i);
                assert_noexcept(TestXE::noinline_yign_extend64_8(u) == i);
                assert_noexcept(TestXE::noinline_yign_extend64_16(u) == i);
                assert_noexcept(TestXE::noinline_yign_extend64_24(u) == i);
                assert_noexcept(TestXE::noinline_yign_extend64_32(u) == i);
                assert_noexcept(TestXE::noinline_yign_extend64_48(u) == i);
                assert_noexcept(TestXE::noinline_yign_extend64_64(u) == i);
                if (i >= 0) {
                    assert_noexcept(TestXE::noinline_x_unsign_extend64(u, 4 + (i & 1)) == u);
                    assert_noexcept(TestXE::noinline_x_unsign_extend64_4(u) == u);
                    assert_noexcept(TestXE::noinline_x_unsign_extend64_8(u) == u);
                    assert_noexcept(TestXE::noinline_x_unsign_extend64_16(u) == u);
                    assert_noexcept(TestXE::noinline_x_unsign_extend64_24(u) == u);
                    assert_noexcept(TestXE::noinline_x_unsign_extend64_32(u) == u);
                    assert_noexcept(TestXE::noinline_x_unsign_extend64_48(u) == u);
                    assert_noexcept(TestXE::noinline_x_unsign_extend64_64(u) == u);
                }
            }
        }
    }
    {
        const upx_uint64_t element_size = upx_uint64_t(acc_vget_acc_int64l_t(1, 0));
        const upx_uint64_t n = upx_uint64_t(acc_vget_acc_int64l_t(0, 0));
        assert_noexcept2(TestXE::noinline_mem_size_valid_bytes(n));
        assert_noexcept2(TestXE::noinline_mem_size(element_size, n) == 0);
        assert_noexcept2(TestXE::noinline_mem_size_get_n(element_size, n) == 0);
    }
    if (acc_vget_int(0, 0)) {
        ByteArray(d, 1048576);
        ByteArray(s, 1048576);
        s_membuf.clear();
        TestXE::noinline_memcpy_0(d, s);
        TestXE::noinline_memcpy_1(d, s);
        TestXE::noinline_memcpy_2(d, s);
        TestXE::noinline_memcpy_4(d, s);
        TestXE::noinline_memcpy_8(d, s);
        TestXE::noinline_memcpy_16(d, s);
        TestXE::noinline_memcpy_32(d, s);
        TestXE::noinline_memcpy_64(d, s);
        TestXE::noinline_memcpy_128(d, s);
        TestXE::noinline_memcpy_256(d, s);
        TestXE::noinline_memcpy_512(d, s);
        TestXE::noinline_memcpy_1024(d, s);
        TestXE::noinline_memcpy_65536(d, s);
        TestXE::noinline_memcpy_1048576(d, s);
        TestXE::noinline_memcpy_3(d, s);
        TestXE::noinline_memcpy_6(d, s);
        TestXE::noinline_memcpy_12(d, s);
        TestXE::noinline_memcpy_24(d, s);
        TestXE::noinline_memcpy_48(d, s);
        TestXE::noinline_memcpy_96(d, s);
        TestXE::noinline_memcpy_192(d, s);
        TestXE::noinline_memcpy_384(d, s);
        TestXE::noinline_memcpy_768(d, s);
        TestXE::noinline_memcpy_7(d, s);
        TestXE::noinline_memcpy_13(d, s);
        TestXE::noinline_memcpy_25(d, s);
        TestXE::noinline_memcpy_49(d, s);
        TestXE::noinline_memcpy_97(d, s);
        TestXE::noinline_memcpy_193(d, s);
        TestXE::noinline_memcpy_385(d, s);
        TestXE::noinline_memcpy_769(d, s);
    }
}

TEST_CASE("upx::run_time 2") {
    const upx_ptraddr_t p = upx_ptraddr_t(acc_vget_int(1, 0));
    const upx_ptraddr_t a = upx_ptraddr_t(acc_vget_int(8, 0));
    assert_noexcept2(TestCT::noinline_has_single_bit(p));
    assert_noexcept2(TestCT::noinline_align_down(p, a) == 0);
    assert_noexcept2(TestCT::noinline_align_down_gap(p, a) == 1);
    assert_noexcept2(TestCT::noinline_align_up(p, a) == 8);
    assert_noexcept2(TestCT::noinline_align_up_gap(p, a) == 7);
    assert_noexcept2(TestCT::noinline_align_down_16(p) == 0);
    assert_noexcept2(TestCT::noinline_align_down_gap_16(p) == 1);
    assert_noexcept2(TestCT::noinline_align_up_16(p) == 16);
    assert_noexcept2(TestCT::noinline_align_up_gap_16(p) == 15);
    alignas(16) static byte b[16] = {};
    assert_noexcept2(TestCT::noinline_ptr_align_down(b + 1, a) != nullptr);
    assert_noexcept2(TestCT::noinline_ptr_align_up(b + 1, a) != nullptr);
    assert_noexcept2(TestCT::noinline_ptr_align_down_16(b + 1) != nullptr);
    assert_noexcept2(TestCT::noinline_ptr_align_up_16(b + 1) != nullptr);
#if !defined(upx_fake_alignas_16)
#if 1 && (ACC_OS_DOS32) && defined(__DJGPP__)
    // @COMPILER_BUG @GCC_BUG
#else
    assert_noexcept2(ptr_is_aligned(b, 16));
    assert_noexcept2(TestCT::noinline_ptr_align_down(b + 1, a) == b);
    assert_noexcept2(TestCT::noinline_ptr_align_up(b + 1, a) == b + 8);
    assert_noexcept2(TestCT::noinline_ptr_align_down_16(b + 1) == b);
    assert_noexcept2(TestCT::noinline_ptr_align_up_16(b + 1) == b + 16);
#endif
#endif
}

/*************************************************************************
// codegen
**************************************************************************/

#if (ACC_CC_MSC)
#pragma warning(push)
#pragma warning(disable : 4310) // warning C4310: cast truncates constant value
#endif

namespace {
struct TestConstant final {
    template <class T>
    static noinline T noinline_zero(T) noexcept {
        return T(0);
    }
    template <class T>
    static noinline T noinline_one(T) noexcept {
        return T(1);
    }
    template <class T>
    static noinline T noinline_minus_one(T) noexcept {
        return T(T(0) - T(1));
    }
    template <class T>
    static noinline T noinline_0xff(T) noexcept {
        return T(0xff);
    }
    template <class T>
    static noinline T noinline_0xffff(T) noexcept {
        return T(0xffff);
    }
    template <class T>
    static noinline T noinline_0xffffffff(T) noexcept {
        return T(0xffffffff);
    }

    template <class T>
    static noinline T noinline_add_zero(T n) noexcept {
        return T(n + T(0));
    }
    template <class T>
    static noinline T noinline_add_one(T n) noexcept {
        return T(n + T(1));
    }
    template <class T>
    static noinline T noinline_add_minus_one(T n) noexcept {
        return T(n + T(T(0) - T(1)));
    }
    template <class T>
    static noinline T noinline_add_0xff(T n) noexcept {
        return T(n + T(0xff));
    }
    template <class T>
    static noinline T noinline_add_0xffff(T n) noexcept {
        return T(n + T(0xffff));
    }
    template <class T>
    static noinline T noinline_add_0xffffffff(T n) noexcept {
        return T(n + T(0xffffffff));
    }

    template <class T>
    static noinline T noinline_add_0x0102030405060708(T n) noexcept {
        return T(n + T(0x0102030405060708ull));
    }
    template <class T>
    static noinline T noinline_add_0xf9f7f5f1c9c7c5c1(T n) noexcept {
        return T(n + T(0xf9f7f5f1c9c7c5c1ull));
    }

    template <class T>
    static noinline T noinline_mul(T a, T b) noexcept {
        return T(a * b);
    }
    template <class T>
    static noinline T noinline_div(T a, T b) noexcept {
        return T(a / b);
    }
    template <class T>
    static noinline T noinline_mod(T a, T b) noexcept {
        return T(a % b);
    }
    template <class T>
    static noinline T noinline_divmod(T a, T b) noexcept {
        return T(T(a / b) + T(a % b));
    }

    template <class T>
    static noinline T noinline_add_mul(T n, T a, T b) noexcept {
        return T(n + T(a * b));
    }
    template <class T>
    static noinline T noinline_add_div(T n, T a, T b) noexcept {
        return T(n + T(a / b));
    }
    template <class T>
    static noinline T noinline_add_mod(T n, T a, T b) noexcept {
        return T(n + T(a % b));
    }
    template <class T>
    static noinline T noinline_add_divmod(T n, T a, T b) noexcept {
        return T(n + T(T(a / b) + T(a % b)));
    }

    template <class T>
    static noinline T noinline_xadd_0() noexcept {
        return T(0);
    }
    template <class T>
    static noinline T noinline_xadd_1(T n) noexcept {
        return T(n);
    }
    template <class T>
    static noinline T noinline_xadd_2(T n, T a) noexcept {
        return T(n + a);
    }
    template <class T>
    static noinline T noinline_xadd_3(T n, T a, T b) noexcept {
        return T(n + a + b);
    }
    template <class T>
    static noinline T noinline_xadd_4(T n, T a, T b, T c) noexcept {
        return T(n + a + b + c);
    }
    template <class T>
    static noinline T noinline_xadd_5(T n, T a, T b, T c, T d) noexcept {
        return T(n + a + b + c + d);
    }
    template <class T>
    static noinline T noinline_xadd_6(T n, T a, T b, T c, T d, T e) noexcept {
        return T(n + a + b + c + d + e);
    }
    template <class T>
    static noinline T noinline_xadd_7(T n, T a, T b, T c, T d, T e, T f) noexcept {
        return T(n + a + b + c + d + e + f);
    }
    template <class T>
    static noinline T noinline_xadd_8(T n, T a, T b, T c, T d, T e, T f, T g) noexcept {
        return T(n + a + b + c + d + e + f + g);
    }
    template <class T>
    static noinline T noinline_xadd_9(T n, T a, T b, T c, T d, T e, T f, T g, T h) noexcept {
        return T(n + a + b + c + d + e + f + g + h);
    }
    template <class T>
    static noinline T noinline_xadd_10(T n, T a, T b, T c, T d, T e, T f, T g, T h, T i) noexcept {
        return T(n + a + b + c + d + e + f + g + h + i);
    }
    template <class T>
    static noinline T noinline_xadd_11(T n, T a, T b, T c, T d, T e, T f, T g, T h, T i,
                                       T j) noexcept {
        return T(n + a + b + c + d + e + f + g + h + i + j);
    }
    template <class T>
    static noinline T noinline_xadd_12(T n, T a, T b, T c, T d, T e, T f, T g, T h, T i, T j,
                                       T k) noexcept {
        return T(n + a + b + c + d + e + f + g + h + i + j + k);
    }

    struct R1 final {
        size_t a[1];
    };
    struct R2 final {
        size_t a[2];
    };
    struct R3 final {
        size_t a[3];
    };
    struct R4 final {
        size_t a[4];
    };
    struct R5 final {
        size_t a[5];
    };
    struct R6 final {
        size_t a[6];
    };
    struct R7 final {
        size_t a[7];
    };
    struct R8 final {
        size_t a[8];
    };
    static noinline R1 noinline_return_1() noexcept {
        R1 r = {};
        return r;
    }
    static noinline R2 noinline_return_2() noexcept {
        R2 r = {};
        return r;
    }
    static noinline R3 noinline_return_3() noexcept {
        R3 r = {};
        return r;
    }
    static noinline R4 noinline_return_4() noexcept {
        R4 r = {};
        return r;
    }
    static noinline R5 noinline_return_5() noexcept {
        R5 r = {};
        return r;
    }
    static noinline R6 noinline_return_6() noexcept {
        R6 r = {};
        return r;
    }
    static noinline R7 noinline_return_7() noexcept {
        R7 r = {};
        return r;
    }
    static noinline R8 noinline_return_8() noexcept {
        R8 r = {};
        return r;
    }
};
template <class T>
struct TestXX final {
    template <class U>
    static const noinline T *noinline_add_ptr(const T *p, U n) noexcept {
        return p + n;
    }
    template <class U>
    static noinline T noinline_get_ptr(const T *p, U n) noexcept {
        return p[n];
    }
    template <class U>
    static noinline void noinline_set_ptr(T *p, U n, T v) noexcept {
        p[n] = v;
    }
    template <class U>
    static noinline T noinline_get_or_1(const T *p, U n) noexcept {
        return T(p[n] | 1);
    }
    template <class U>
    static noinline void noinline_or_1(T *p, U n) noexcept {
        p[n] = T(p[n] | 1);
    }

    template <class U>
    static noinline auto noinline_add_ptr_promoted(const T *p, U n) noexcept {
        typedef decltype(U(0) + U(0)) promoted_type;
        return noinline_add_ptr(p, promoted_type(n));
    }
    template <class U>
    static noinline auto noinline_get_ptr_promoted(const T *p, U n) noexcept {
        typedef decltype(U(0) + U(0)) promoted_type;
        return noinline_get_ptr(p, promoted_type(n));
    }
};
} // namespace

TEST_CASE("codegen constant") {
    const int n = acc_vget_int(0, 0);
    const int a = acc_vget_int(0, 0);
    const int b = acc_vget_int(1, 0);
    {
        assert_noexcept2((TestConstant::noinline_zero(upx_int8_t(n)) == 0));
        assert_noexcept2((TestConstant::noinline_zero(upx_uint8_t(n)) == 0));
        assert_noexcept2((TestConstant::noinline_zero(upx_int16_t(n)) == 0));
        assert_noexcept2((TestConstant::noinline_zero(upx_uint16_t(n)) == 0));
        assert_noexcept2((TestConstant::noinline_zero(upx_int32_t(n)) == 0));
        assert_noexcept2((TestConstant::noinline_zero(upx_uint32_t(n)) == 0));
        assert_noexcept2((TestConstant::noinline_zero(upx_int64_t(n)) == 0));
        assert_noexcept2((TestConstant::noinline_zero(upx_uint64_t(n)) == 0));

        assert_noexcept2((TestConstant::noinline_one(upx_int8_t(n)) == 1));
        assert_noexcept2((TestConstant::noinline_one(upx_uint8_t(n)) == 1));
        assert_noexcept2((TestConstant::noinline_one(upx_int16_t(n)) == 1));
        assert_noexcept2((TestConstant::noinline_one(upx_uint16_t(n)) == 1));
        assert_noexcept2((TestConstant::noinline_one(upx_int32_t(n)) == 1));
        assert_noexcept2((TestConstant::noinline_one(upx_uint32_t(n)) == 1));
        assert_noexcept2((TestConstant::noinline_one(upx_int64_t(n)) == 1));
        assert_noexcept2((TestConstant::noinline_one(upx_uint64_t(n)) == 1));

        assert_noexcept2((TestConstant::noinline_minus_one(upx_int8_t(n)) == -1));
        assert_noexcept2((TestConstant::noinline_minus_one(upx_uint8_t(n)) == 0xff));
        assert_noexcept2((TestConstant::noinline_minus_one(upx_int16_t(n)) == -1));
        assert_noexcept2((TestConstant::noinline_minus_one(upx_uint16_t(n)) == 0xffff));
        assert_noexcept2((TestConstant::noinline_minus_one(upx_int32_t(n)) == -1));
        assert_noexcept2((TestConstant::noinline_minus_one(upx_uint32_t(n)) == 0xffffffff));
        assert_noexcept2((TestConstant::noinline_minus_one(upx_int64_t(n)) == -1));
        assert_noexcept2(
            (TestConstant::noinline_minus_one(upx_uint64_t(n)) == 0xffffffffffffffffULL));

        assert_noexcept2((TestConstant::noinline_0xff(upx_int8_t(n)) == -1));
        assert_noexcept2((TestConstant::noinline_0xff(upx_uint8_t(n)) == 0xff));
        assert_noexcept2((TestConstant::noinline_0xff(upx_int16_t(n)) == 0xff));
        assert_noexcept2((TestConstant::noinline_0xff(upx_uint16_t(n)) == 0xff));
        assert_noexcept2((TestConstant::noinline_0xff(upx_int32_t(n)) == 0xff));
        assert_noexcept2((TestConstant::noinline_0xff(upx_uint32_t(n)) == 0xff));
        assert_noexcept2((TestConstant::noinline_0xff(upx_int64_t(n)) == 0xff));
        assert_noexcept2((TestConstant::noinline_0xff(upx_uint64_t(n)) == 0xff));

        assert_noexcept2((TestConstant::noinline_0xffff(upx_int8_t(n)) == -1));
        assert_noexcept2((TestConstant::noinline_0xffff(upx_uint8_t(n)) == 0xff));
        assert_noexcept2((TestConstant::noinline_0xffff(upx_int16_t(n)) == -1));
        assert_noexcept2((TestConstant::noinline_0xffff(upx_uint16_t(n)) == 0xffff));
        assert_noexcept2((TestConstant::noinline_0xffff(upx_int32_t(n)) == 0xffff));
        assert_noexcept2((TestConstant::noinline_0xffff(upx_uint32_t(n)) == 0xffff));
        assert_noexcept2((TestConstant::noinline_0xffff(upx_int64_t(n)) == 0xffff));
        assert_noexcept2((TestConstant::noinline_0xffff(upx_uint64_t(n)) == 0xffff));

        assert_noexcept2((TestConstant::noinline_0xffffffff(upx_int8_t(n)) == -1));
        assert_noexcept2((TestConstant::noinline_0xffffffff(upx_uint8_t(n)) == 0xff));
        assert_noexcept2((TestConstant::noinline_0xffffffff(upx_int16_t(n)) == -1));
        assert_noexcept2((TestConstant::noinline_0xffffffff(upx_uint16_t(n)) == 0xffff));
        assert_noexcept2((TestConstant::noinline_0xffffffff(upx_int32_t(n)) == -1));
        assert_noexcept2((TestConstant::noinline_0xffffffff(upx_uint32_t(n)) == 0xffffffff));
        assert_noexcept2((TestConstant::noinline_0xffffffff(upx_int64_t(n)) == 0xffffffff));
        assert_noexcept2((TestConstant::noinline_0xffffffff(upx_uint64_t(n)) == 0xffffffff));

        assert_noexcept2((TestConstant::noinline_add_zero(upx_int8_t(n)) == 0));
        assert_noexcept2((TestConstant::noinline_add_zero(upx_uint8_t(n)) == 0));
        assert_noexcept2((TestConstant::noinline_add_zero(upx_int16_t(n)) == 0));
        assert_noexcept2((TestConstant::noinline_add_zero(upx_uint16_t(n)) == 0));
        assert_noexcept2((TestConstant::noinline_add_zero(upx_int32_t(n)) == 0));
        assert_noexcept2((TestConstant::noinline_add_zero(upx_uint32_t(n)) == 0));
        assert_noexcept2((TestConstant::noinline_add_zero(upx_int64_t(n)) == 0));
        assert_noexcept2((TestConstant::noinline_add_zero(upx_uint64_t(n)) == 0));

        assert_noexcept2((TestConstant::noinline_add_one(upx_int8_t(n)) == 1));
        assert_noexcept2((TestConstant::noinline_add_one(upx_uint8_t(n)) == 1));
        assert_noexcept2((TestConstant::noinline_add_one(upx_int16_t(n)) == 1));
        assert_noexcept2((TestConstant::noinline_add_one(upx_uint16_t(n)) == 1));
        assert_noexcept2((TestConstant::noinline_add_one(upx_int32_t(n)) == 1));
        assert_noexcept2((TestConstant::noinline_add_one(upx_uint32_t(n)) == 1));
        assert_noexcept2((TestConstant::noinline_add_one(upx_int64_t(n)) == 1));
        assert_noexcept2((TestConstant::noinline_add_one(upx_uint64_t(n)) == 1));

        assert_noexcept2((TestConstant::noinline_add_minus_one(upx_int8_t(n)) == -1));
        assert_noexcept2((TestConstant::noinline_add_minus_one(upx_uint8_t(n)) == 0xff));
        assert_noexcept2((TestConstant::noinline_add_minus_one(upx_int16_t(n)) == -1));
        assert_noexcept2((TestConstant::noinline_add_minus_one(upx_uint16_t(n)) == 0xffff));
        assert_noexcept2((TestConstant::noinline_add_minus_one(upx_int32_t(n)) == -1));
        assert_noexcept2((TestConstant::noinline_add_minus_one(upx_uint32_t(n)) == 0xffffffff));
        assert_noexcept2((TestConstant::noinline_add_minus_one(upx_int64_t(n)) == -1));
        assert_noexcept2(
            (TestConstant::noinline_add_minus_one(upx_uint64_t(n)) == 0xffffffffffffffffULL));

        assert_noexcept2((TestConstant::noinline_add_0xff(upx_int8_t(n)) == -1));
        assert_noexcept2((TestConstant::noinline_add_0xff(upx_uint8_t(n)) == 0xff));
        assert_noexcept2((TestConstant::noinline_add_0xff(upx_int16_t(n)) == 0xff));
        assert_noexcept2((TestConstant::noinline_add_0xff(upx_uint16_t(n)) == 0xff));
        assert_noexcept2((TestConstant::noinline_add_0xff(upx_int32_t(n)) == 0xff));
        assert_noexcept2((TestConstant::noinline_add_0xff(upx_uint32_t(n)) == 0xff));
        assert_noexcept2((TestConstant::noinline_add_0xff(upx_int64_t(n)) == 0xff));
        assert_noexcept2((TestConstant::noinline_add_0xff(upx_uint64_t(n)) == 0xff));

        assert_noexcept2((TestConstant::noinline_add_0xffff(upx_int8_t(n)) == -1));
        assert_noexcept2((TestConstant::noinline_add_0xffff(upx_uint8_t(n)) == 0xff));
        assert_noexcept2((TestConstant::noinline_add_0xffff(upx_int16_t(n)) == -1));
        assert_noexcept2((TestConstant::noinline_add_0xffff(upx_uint16_t(n)) == 0xffff));
        assert_noexcept2((TestConstant::noinline_add_0xffff(upx_int32_t(n)) == 0xffff));
        assert_noexcept2((TestConstant::noinline_add_0xffff(upx_uint32_t(n)) == 0xffff));
        assert_noexcept2((TestConstant::noinline_add_0xffff(upx_int64_t(n)) == 0xffff));
        assert_noexcept2((TestConstant::noinline_add_0xffff(upx_uint64_t(n)) == 0xffff));

        assert_noexcept2((TestConstant::noinline_add_0xffffffff(upx_int8_t(n)) == -1));
        assert_noexcept2((TestConstant::noinline_add_0xffffffff(upx_uint8_t(n)) == 0xff));
        assert_noexcept2((TestConstant::noinline_add_0xffffffff(upx_int16_t(n)) == -1));
        assert_noexcept2((TestConstant::noinline_add_0xffffffff(upx_uint16_t(n)) == 0xffff));
        assert_noexcept2((TestConstant::noinline_add_0xffffffff(upx_int32_t(n)) == -1));
        assert_noexcept2((TestConstant::noinline_add_0xffffffff(upx_uint32_t(n)) == 0xffffffff));
        assert_noexcept2((TestConstant::noinline_add_0xffffffff(upx_int64_t(n)) == 0xffffffff));
        assert_noexcept2((TestConstant::noinline_add_0xffffffff(upx_uint64_t(n)) == 0xffffffff));

        assert_noexcept2((TestConstant::noinline_add_0x0102030405060708(upx_int8_t(n)) != 0));
        assert_noexcept2((TestConstant::noinline_add_0x0102030405060708(upx_uint8_t(n)) != 0));
        assert_noexcept2((TestConstant::noinline_add_0x0102030405060708(upx_int16_t(n)) != 0));
        assert_noexcept2((TestConstant::noinline_add_0x0102030405060708(upx_uint16_t(n)) != 0));
        assert_noexcept2((TestConstant::noinline_add_0x0102030405060708(upx_int32_t(n)) != 0));
        assert_noexcept2((TestConstant::noinline_add_0x0102030405060708(upx_uint32_t(n)) != 0));
        assert_noexcept2((TestConstant::noinline_add_0x0102030405060708(upx_int64_t(n)) != 0));
        assert_noexcept2((TestConstant::noinline_add_0x0102030405060708(upx_uint64_t(n)) != 0));

        assert_noexcept2((TestConstant::noinline_add_0xf9f7f5f1c9c7c5c1(upx_int8_t(n)) != 0));
        assert_noexcept2((TestConstant::noinline_add_0xf9f7f5f1c9c7c5c1(upx_uint8_t(n)) != 0));
        assert_noexcept2((TestConstant::noinline_add_0xf9f7f5f1c9c7c5c1(upx_int16_t(n)) != 0));
        assert_noexcept2((TestConstant::noinline_add_0xf9f7f5f1c9c7c5c1(upx_uint16_t(n)) != 0));
        assert_noexcept2((TestConstant::noinline_add_0xf9f7f5f1c9c7c5c1(upx_int32_t(n)) != 0));
        assert_noexcept2((TestConstant::noinline_add_0xf9f7f5f1c9c7c5c1(upx_uint32_t(n)) != 0));
        assert_noexcept2((TestConstant::noinline_add_0xf9f7f5f1c9c7c5c1(upx_int64_t(n)) != 0));
        assert_noexcept2((TestConstant::noinline_add_0xf9f7f5f1c9c7c5c1(upx_uint64_t(n)) != 0));

        assert_noexcept2((TestConstant::noinline_mul(upx_int8_t(a), upx_int8_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_mul(upx_uint8_t(a), upx_uint8_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_mul(upx_int16_t(a), upx_int16_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_mul(upx_uint16_t(a), upx_uint16_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_mul(upx_int32_t(a), upx_int32_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_mul(upx_uint32_t(a), upx_uint32_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_mul(upx_int64_t(a), upx_int64_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_mul(upx_uint64_t(a), upx_uint64_t(b)) == 0));

        assert_noexcept2((TestConstant::noinline_div(upx_int8_t(a), upx_int8_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_div(upx_uint8_t(a), upx_uint8_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_div(upx_int16_t(a), upx_int16_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_div(upx_uint16_t(a), upx_uint16_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_div(upx_int32_t(a), upx_int32_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_div(upx_uint32_t(a), upx_uint32_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_div(upx_int64_t(a), upx_int64_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_div(upx_uint64_t(a), upx_uint64_t(b)) == 0));

        assert_noexcept2((TestConstant::noinline_mod(upx_int8_t(a), upx_int8_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_mod(upx_uint8_t(a), upx_uint8_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_mod(upx_int16_t(a), upx_int16_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_mod(upx_uint16_t(a), upx_uint16_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_mod(upx_int32_t(a), upx_int32_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_mod(upx_uint32_t(a), upx_uint32_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_mod(upx_int64_t(a), upx_int64_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_mod(upx_uint64_t(a), upx_uint64_t(b)) == 0));

        assert_noexcept2((TestConstant::noinline_divmod(upx_int8_t(a), upx_int8_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_divmod(upx_uint8_t(a), upx_uint8_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_divmod(upx_int16_t(a), upx_int16_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_divmod(upx_uint16_t(a), upx_uint16_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_divmod(upx_int32_t(a), upx_int32_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_divmod(upx_uint32_t(a), upx_uint32_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_divmod(upx_int64_t(a), upx_int64_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_divmod(upx_uint64_t(a), upx_uint64_t(b)) == 0));

        assert_noexcept2(
            (TestConstant::noinline_add_mul(upx_int8_t(n), upx_int8_t(a), upx_int8_t(b)) == 0));
        assert_noexcept2(
            (TestConstant::noinline_add_mul(upx_uint8_t(n), upx_uint8_t(a), upx_uint8_t(b)) == 0));
        assert_noexcept2(
            (TestConstant::noinline_add_mul(upx_int16_t(n), upx_int16_t(a), upx_int16_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_add_mul(upx_uint16_t(n), upx_uint16_t(a),
                                                         upx_uint16_t(b)) == 0));
        assert_noexcept2(
            (TestConstant::noinline_add_mul(upx_int32_t(n), upx_int32_t(a), upx_int32_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_add_mul(upx_uint32_t(n), upx_uint32_t(a),
                                                         upx_uint32_t(b)) == 0));
        assert_noexcept2(
            (TestConstant::noinline_add_mul(upx_int64_t(n), upx_int64_t(a), upx_int64_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_add_mul(upx_uint64_t(n), upx_uint64_t(a),
                                                         upx_uint64_t(b)) == 0));

        assert_noexcept2(
            (TestConstant::noinline_add_div(upx_int8_t(n), upx_int8_t(a), upx_int8_t(b)) == 0));
        assert_noexcept2(
            (TestConstant::noinline_add_div(upx_uint8_t(n), upx_uint8_t(a), upx_uint8_t(b)) == 0));
        assert_noexcept2(
            (TestConstant::noinline_add_div(upx_int16_t(n), upx_int16_t(a), upx_int16_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_add_div(upx_uint16_t(n), upx_uint16_t(a),
                                                         upx_uint16_t(b)) == 0));
        assert_noexcept2(
            (TestConstant::noinline_add_div(upx_int32_t(n), upx_int32_t(a), upx_int32_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_add_div(upx_uint32_t(n), upx_uint32_t(a),
                                                         upx_uint32_t(b)) == 0));
        assert_noexcept2(
            (TestConstant::noinline_add_div(upx_int64_t(n), upx_int64_t(a), upx_int64_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_add_div(upx_uint64_t(n), upx_uint64_t(a),
                                                         upx_uint64_t(b)) == 0));

        assert_noexcept2(
            (TestConstant::noinline_add_mod(upx_int8_t(n), upx_int8_t(a), upx_int8_t(b)) == 0));
        assert_noexcept2(
            (TestConstant::noinline_add_mod(upx_uint8_t(n), upx_uint8_t(a), upx_uint8_t(b)) == 0));
        assert_noexcept2(
            (TestConstant::noinline_add_mod(upx_int16_t(n), upx_int16_t(a), upx_int16_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_add_mod(upx_uint16_t(n), upx_uint16_t(a),
                                                         upx_uint16_t(b)) == 0));
        assert_noexcept2(
            (TestConstant::noinline_add_mod(upx_int32_t(n), upx_int32_t(a), upx_int32_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_add_mod(upx_uint32_t(n), upx_uint32_t(a),
                                                         upx_uint32_t(b)) == 0));
        assert_noexcept2(
            (TestConstant::noinline_add_mod(upx_int64_t(n), upx_int64_t(a), upx_int64_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_add_mod(upx_uint64_t(n), upx_uint64_t(a),
                                                         upx_uint64_t(b)) == 0));

        assert_noexcept2(
            (TestConstant::noinline_add_divmod(upx_int8_t(n), upx_int8_t(a), upx_int8_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_add_divmod(upx_uint8_t(n), upx_uint8_t(a),
                                                            upx_uint8_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_add_divmod(upx_int16_t(n), upx_int16_t(a),
                                                            upx_int16_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_add_divmod(upx_uint16_t(n), upx_uint16_t(a),
                                                            upx_uint16_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_add_divmod(upx_int32_t(n), upx_int32_t(a),
                                                            upx_int32_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_add_divmod(upx_uint32_t(n), upx_uint32_t(a),
                                                            upx_uint32_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_add_divmod(upx_int64_t(n), upx_int64_t(a),
                                                            upx_int64_t(b)) == 0));
        assert_noexcept2((TestConstant::noinline_add_divmod(upx_uint64_t(n), upx_uint64_t(a),
                                                            upx_uint64_t(b)) == 0));

        typedef size_t T;
        assert_noexcept2((TestConstant::noinline_xadd_0<T>() == 0));
        assert_noexcept2((TestConstant::noinline_xadd_1<T>(n) == 0));
        assert_noexcept2((TestConstant::noinline_xadd_2<T>(n, n) == 0));
        assert_noexcept2((TestConstant::noinline_xadd_3<T>(n, n, n) == 0));
        assert_noexcept2((TestConstant::noinline_xadd_4<T>(n, n, n, n) == 0));
        assert_noexcept2((TestConstant::noinline_xadd_5<T>(n, n, n, n, n) == 0));
        assert_noexcept2((TestConstant::noinline_xadd_6<T>(n, n, n, n, n, n) == 0));
        assert_noexcept2((TestConstant::noinline_xadd_7<T>(n, n, n, n, n, n, n) == 0));
        assert_noexcept2((TestConstant::noinline_xadd_8<T>(n, n, n, n, n, n, n, n) == 0));
        assert_noexcept2((TestConstant::noinline_xadd_9<T>(n, n, n, n, n, n, n, n, n) == 0));
        assert_noexcept2((TestConstant::noinline_xadd_10<T>(n, n, n, n, n, n, n, n, n, n) == 0));
        assert_noexcept2((TestConstant::noinline_xadd_11<T>(n, n, n, n, n, n, n, n, n, n, n) == 0));
        assert_noexcept2(
            (TestConstant::noinline_xadd_12<T>(n, n, n, n, n, n, n, n, n, n, n, n) == 0));

        assert_noexcept2((TestConstant::noinline_return_1().a[0] == 0));
        assert_noexcept2((TestConstant::noinline_return_2().a[0] == 0));
        assert_noexcept2((TestConstant::noinline_return_3().a[0] == 0));
        assert_noexcept2((TestConstant::noinline_return_4().a[0] == 0));
        assert_noexcept2((TestConstant::noinline_return_5().a[0] == 0));
        assert_noexcept2((TestConstant::noinline_return_6().a[0] == 0));
        assert_noexcept2((TestConstant::noinline_return_7().a[0] == 0));
        assert_noexcept2((TestConstant::noinline_return_8().a[0] == 0));
    }
    (void) n;
    (void) a;
    (void) b;
}

TEST_CASE("codegen") {
    const int n = acc_vget_int(2, 0);
    {
        typedef byte T;
        T buf[4] = {0, 1, 2, 3};
        const T v = T(acc_vget_int(0, 0));

        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int64_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint64_t(n)) == (buf + n));

        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_int8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_uint8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_int16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_uint16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_int32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_uint32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_int64_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_uint64_t(n)) == (buf + n));

        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_int8_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_uint8_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_int16_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_uint16_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_int32_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_uint32_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_int64_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_uint64_t(n)) == 2);

        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_int8_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_uint8_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_int16_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_uint16_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_int32_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_uint32_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_int64_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_uint64_t(n), v));

        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_int8_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_uint8_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_int16_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_uint16_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_int32_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_uint32_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_int64_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_uint64_t(n)) == 1);

        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_int8_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_uint8_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_int16_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_uint16_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_int32_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_uint32_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_int64_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_uint64_t(n)));

        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_int8_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_uint8_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_int16_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_uint16_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_int32_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_uint32_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_int64_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_uint64_t(n)) == 1));

        (void) buf;
        (void) v;
    }
    {
        typedef upx_int32_t T;
        T buf[4] = {0, 1, 2, 3};
        const T v = T(acc_vget_int(0, 0));

        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int64_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint64_t(n)) == (buf + n));

        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_int8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_uint8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_int16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_uint16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_int32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_uint32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_int64_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_uint64_t(n)) == (buf + n));

        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_int8_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_uint8_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_int16_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_uint16_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_int32_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_uint32_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_int64_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_uint64_t(n)) == 2);

        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_int8_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_uint8_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_int16_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_uint16_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_int32_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_uint32_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_int64_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_uint64_t(n), v));

        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_int8_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_uint8_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_int16_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_uint16_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_int32_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_uint32_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_int64_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_uint64_t(n)) == 1);

        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_int8_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_uint8_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_int16_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_uint16_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_int32_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_uint32_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_int64_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_uint64_t(n)));

        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_int8_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_uint8_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_int16_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_uint16_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_int32_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_uint32_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_int64_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_uint64_t(n)) == 1));

        (void) buf;
        (void) v;
    }
    {
        typedef upx_uint64_t T;
        T buf[4] = {0, 1, 2, 3};
        const T v = T(acc_vget_int(0, 0));

        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int64_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint64_t(n)) == (buf + n));

        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_int8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_uint8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_int16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_uint16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_int32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_uint32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_int64_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr_promoted(buf, upx_uint64_t(n)) == (buf + n));

        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_int8_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_uint8_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_int16_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_uint16_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_int32_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_uint32_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_int64_t(n)) == 2);
        CHECK(TestXX<T>::noinline_get_ptr(buf, upx_uint64_t(n)) == 2);

        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_int8_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_uint8_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_int16_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_uint16_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_int32_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_uint32_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_int64_t(n), v));
        CHECK_NOTHROW(TestXX<T>::noinline_set_ptr(buf, upx_uint64_t(n), v));

        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_int8_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_uint8_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_int16_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_uint16_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_int32_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_uint32_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_int64_t(n)) == 1);
        CHECK(TestXX<T>::noinline_get_or_1(buf, upx_uint64_t(n)) == 1);

        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_int8_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_uint8_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_int16_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_uint16_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_int32_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_uint32_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_int64_t(n)));
        CHECK_NOTHROW(TestXX<T>::noinline_or_1(buf, upx_uint64_t(n)));

        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_int8_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_uint8_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_int16_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_uint16_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_int32_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_uint32_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_int64_t(n)) == 1));
        CHECK((TestXX<T>::noinline_get_ptr_promoted(buf, upx_uint64_t(n)) == 1));

        (void) buf;
        (void) v;
    }
#if (__SIZEOF_INT128__ == 16)
    {
        typedef upx_uint128_t T;
        T buf[4] = {0, 1, 2, 3};

        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int64_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint64_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int128_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint128_t(n)) == (buf + n));

        (void) buf;
    }
#endif
    {
        struct alignas(1) T16 final { byte a[16]; };
        typedef T16 T;
        T buf[2] = {};

        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int64_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint64_t(n)) == (buf + n));

        (void) buf;
    }
    {
        struct alignas(1) T32 final { byte a[32]; };
        typedef T32 T;
        T buf[2] = {};

        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int64_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint64_t(n)) == (buf + n));

        (void) buf;
    }
    {
        struct alignas(1) T64 final { byte a[64]; };
        typedef T64 T;
        T buf[2] = {};

        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int64_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint64_t(n)) == (buf + n));

        (void) buf;
    }
    {
        struct alignas(1) T473 final { byte a[473]; };
        typedef T473 T;
        T buf[2] = {};

        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int64_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint64_t(n)) == (buf + n));

        (void) buf;
    }
    {
        struct alignas(1) T1024 final { byte a[1024]; };
        typedef T1024 T;
        T buf[2] = {};

        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int64_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint64_t(n)) == (buf + n));

        (void) buf;
    }
    {
        struct alignas(1) T1025 final { byte a[1025]; };
        typedef T1025 T;
        T buf[2] = {};

        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int64_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint64_t(n)) == (buf + n));

        (void) buf;
    }
    {
        struct alignas(1) T1027 final { byte a[1027]; };
        typedef T1027 T;
        T buf[2] = {};

        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int64_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint64_t(n)) == (buf + n));

        (void) buf;
    }
    {
        struct alignas(1) T1031 final { byte a[1031]; };
        typedef T1031 T;
        T buf[2] = {};

        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int64_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint64_t(n)) == (buf + n));

        (void) buf;
    }
    {
        struct alignas(1) T1039 final { byte a[1039]; };
        typedef T1039 T;
        T buf[2] = {};

        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int64_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint64_t(n)) == (buf + n));

        (void) buf;
    }
    {
        struct alignas(1) T1055 final { byte a[1055]; };
        typedef T1055 T;
        T buf[2] = {};

        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint8_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint16_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint32_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_int64_t(n)) == (buf + n));
        CHECK(TestXX<T>::noinline_add_ptr(buf, upx_uint64_t(n)) == (buf + n));

        (void) buf;
    }
    (void) n;
}

#if (ACC_CC_MSC)
#pragma warning(pop)
#endif

/*************************************************************************
// bit codegen
**************************************************************************/

#if __cplusplus >= 202002L // C++20

#if (ACC_ABI_BIG_ENDIAN)
static_assert(std::endian::native == std::endian::big);
static_assert(std::endian::native != std::endian::little);
#elif (ACC_ABI_LITTLE_ENDIAN)
static_assert(std::endian::native == std::endian::little);
static_assert(std::endian::native != std::endian::big);
#else
#error "ACC_ABI_ENDIAN"
#endif

namespace {
struct TestBit final {
    template <class T>
    static noinline T noinline_bit_cast(T from) noexcept {
        return std::bit_cast<T>(from);
    }

#if __cpp_lib_byteswap >= 202110L
    template <class T>
    static noinline T noinline_byteswap(T n) noexcept {
        return std::byteswap(n);
    }
#endif

    template <class T>
    static noinline bool noinline_has_single_bit(T n) noexcept {
        return std::has_single_bit(n);
    }
    template <class T>
    static noinline T noinline_bit_ceil(T n) noexcept {
        return std::bit_ceil(n);
    }
    template <class T>
    static noinline T noinline_bit_floor(T n) noexcept {
        return std::bit_floor(n);
    }
    template <class T>
    static noinline int noinline_bit_width(T n) noexcept {
        return std::bit_width(n);
    }

    template <class T>
    static noinline T noinline_rotl(T n, int s) noexcept {
        return std::rotl(n, s);
    }
    template <class T>
    static noinline T noinline_rotr(T n, int s) noexcept {
        return std::rotr(n, s);
    }

    template <class T>
    static noinline int noinline_countl_zero(T n) noexcept {
        return std::countl_zero(n);
    }
    template <class T>
    static noinline int noinline_countl_one(T n) noexcept {
        return std::countl_one(n);
    }
    template <class T>
    static noinline int noinline_countr_zero(T n) noexcept {
        return std::countr_zero(n);
    }
    template <class T>
    static noinline int noinline_countr_one(T n) noexcept {
        return std::countr_one(n);
    }
    template <class T>
    static noinline int noinline_popcount(T n) noexcept {
        return std::popcount(n);
    }
};
} // namespace

TEST_CASE("bit codegen") {
    const int s = acc_vget_int(1, 0);
    int n = acc_vget_int(1, 0);
    {
        CHECK_NOTHROW(TestBit::noinline_bit_cast(upx_uint8_t(n)));
        CHECK_NOTHROW(TestBit::noinline_bit_cast(upx_uint16_t(n)));
        CHECK_NOTHROW(TestBit::noinline_bit_cast(upx_uint32_t(n)));
        CHECK_NOTHROW(TestBit::noinline_bit_cast(upx_uint64_t(n)));

#if __cpp_lib_byteswap >= 202110L
        CHECK_NOTHROW(TestBit::noinline_byteswap(upx_uint8_t(n)));
        CHECK_NOTHROW(TestBit::noinline_byteswap(upx_uint16_t(n)));
        CHECK_NOTHROW(TestBit::noinline_byteswap(upx_uint32_t(n)));
        CHECK_NOTHROW(TestBit::noinline_byteswap(upx_uint64_t(n)));
#endif

        CHECK_NOTHROW(TestBit::noinline_has_single_bit(upx_uint8_t(n)));
        CHECK_NOTHROW(TestBit::noinline_has_single_bit(upx_uint16_t(n)));
        CHECK_NOTHROW(TestBit::noinline_has_single_bit(upx_uint32_t(n)));
        CHECK_NOTHROW(TestBit::noinline_has_single_bit(upx_uint64_t(n)));

        CHECK_NOTHROW(TestBit::noinline_bit_ceil(upx_uint8_t(n)));
        CHECK_NOTHROW(TestBit::noinline_bit_ceil(upx_uint16_t(n)));
        CHECK_NOTHROW(TestBit::noinline_bit_ceil(upx_uint32_t(n)));
        CHECK_NOTHROW(TestBit::noinline_bit_ceil(upx_uint64_t(n)));

        CHECK_NOTHROW(TestBit::noinline_bit_floor(upx_uint8_t(n)));
        CHECK_NOTHROW(TestBit::noinline_bit_floor(upx_uint16_t(n)));
        CHECK_NOTHROW(TestBit::noinline_bit_floor(upx_uint32_t(n)));
        CHECK_NOTHROW(TestBit::noinline_bit_floor(upx_uint64_t(n)));

        CHECK_NOTHROW(TestBit::noinline_bit_width(upx_uint8_t(n)));
        CHECK_NOTHROW(TestBit::noinline_bit_width(upx_uint16_t(n)));
        CHECK_NOTHROW(TestBit::noinline_bit_width(upx_uint32_t(n)));
        CHECK_NOTHROW(TestBit::noinline_bit_width(upx_uint64_t(n)));

        CHECK_NOTHROW(TestBit::noinline_rotl(upx_uint8_t(n), s));
        CHECK_NOTHROW(TestBit::noinline_rotl(upx_uint16_t(n), s));
        CHECK_NOTHROW(TestBit::noinline_rotl(upx_uint32_t(n), s));
        CHECK_NOTHROW(TestBit::noinline_rotl(upx_uint64_t(n), s));

        CHECK_NOTHROW(TestBit::noinline_rotr(upx_uint8_t(n), s));
        CHECK_NOTHROW(TestBit::noinline_rotr(upx_uint16_t(n), s));
        CHECK_NOTHROW(TestBit::noinline_rotr(upx_uint32_t(n), s));
        CHECK_NOTHROW(TestBit::noinline_rotr(upx_uint64_t(n), s));

        CHECK_NOTHROW(TestBit::noinline_countl_zero(upx_uint8_t(n)));
        CHECK_NOTHROW(TestBit::noinline_countl_zero(upx_uint16_t(n)));
        CHECK_NOTHROW(TestBit::noinline_countl_zero(upx_uint32_t(n)));
        CHECK_NOTHROW(TestBit::noinline_countl_zero(upx_uint64_t(n)));

        CHECK_NOTHROW(TestBit::noinline_countl_one(upx_uint8_t(n)));
        CHECK_NOTHROW(TestBit::noinline_countl_one(upx_uint16_t(n)));
        CHECK_NOTHROW(TestBit::noinline_countl_one(upx_uint32_t(n)));
        CHECK_NOTHROW(TestBit::noinline_countl_one(upx_uint64_t(n)));

        CHECK_NOTHROW(TestBit::noinline_countr_zero(upx_uint8_t(n)));
        CHECK_NOTHROW(TestBit::noinline_countr_zero(upx_uint16_t(n)));
        CHECK_NOTHROW(TestBit::noinline_countr_zero(upx_uint32_t(n)));
        CHECK_NOTHROW(TestBit::noinline_countr_zero(upx_uint64_t(n)));

        CHECK_NOTHROW(TestBit::noinline_countr_one(upx_uint8_t(n)));
        CHECK_NOTHROW(TestBit::noinline_countr_one(upx_uint16_t(n)));
        CHECK_NOTHROW(TestBit::noinline_countr_one(upx_uint32_t(n)));
        CHECK_NOTHROW(TestBit::noinline_countr_one(upx_uint64_t(n)));

        CHECK_NOTHROW(TestBit::noinline_popcount(upx_uint8_t(n)));
        CHECK_NOTHROW(TestBit::noinline_popcount(upx_uint16_t(n)));
        CHECK_NOTHROW(TestBit::noinline_popcount(upx_uint32_t(n)));
        CHECK_NOTHROW(TestBit::noinline_popcount(upx_uint64_t(n)));
    }
    n = acc_vget_int(0, 0);
    {
        CHECK(TestBit::noinline_bit_cast(upx_uint8_t(n)) == 0);
        CHECK(TestBit::noinline_bit_cast(upx_uint16_t(n)) == 0);
        CHECK(TestBit::noinline_bit_cast(upx_uint32_t(n)) == 0);
        CHECK(TestBit::noinline_bit_cast(upx_uint64_t(n)) == 0);

#if __cpp_lib_byteswap >= 202110L
        CHECK(TestBit::noinline_byteswap(upx_uint8_t(n)) == 0);
        CHECK(TestBit::noinline_byteswap(upx_uint16_t(n)) == 0);
        CHECK(TestBit::noinline_byteswap(upx_uint32_t(n)) == 0);
        CHECK(TestBit::noinline_byteswap(upx_uint64_t(n)) == 0);
#endif

        CHECK(!TestBit::noinline_has_single_bit(upx_uint8_t(n)));
        CHECK(!TestBit::noinline_has_single_bit(upx_uint16_t(n)));
        CHECK(!TestBit::noinline_has_single_bit(upx_uint32_t(n)));
        CHECK(!TestBit::noinline_has_single_bit(upx_uint64_t(n)));

        CHECK(TestBit::noinline_bit_ceil(upx_uint8_t(n)) == 1);
        CHECK(TestBit::noinline_bit_ceil(upx_uint16_t(n)) == 1);
        CHECK(TestBit::noinline_bit_ceil(upx_uint32_t(n)) == 1);
        CHECK(TestBit::noinline_bit_ceil(upx_uint64_t(n)) == 1);

        CHECK(TestBit::noinline_bit_floor(upx_uint8_t(n)) == 0);
        CHECK(TestBit::noinline_bit_floor(upx_uint16_t(n)) == 0);
        CHECK(TestBit::noinline_bit_floor(upx_uint32_t(n)) == 0);
        CHECK(TestBit::noinline_bit_floor(upx_uint64_t(n)) == 0);

        CHECK(TestBit::noinline_bit_width(upx_uint8_t(n)) == 0);
        CHECK(TestBit::noinline_bit_width(upx_uint16_t(n)) == 0);
        CHECK(TestBit::noinline_bit_width(upx_uint32_t(n)) == 0);
        CHECK(TestBit::noinline_bit_width(upx_uint64_t(n)) == 0);

        CHECK(TestBit::noinline_rotl(upx_uint8_t(n), s) == 0);
        CHECK(TestBit::noinline_rotl(upx_uint16_t(n), s) == 0);
        CHECK(TestBit::noinline_rotl(upx_uint32_t(n), s) == 0);
        CHECK(TestBit::noinline_rotl(upx_uint64_t(n), s) == 0);

        CHECK(TestBit::noinline_rotl(upx_uint8_t(n), -s) == 0);
        CHECK(TestBit::noinline_rotl(upx_uint16_t(n), -s) == 0);
        CHECK(TestBit::noinline_rotl(upx_uint32_t(n), -s) == 0);
        CHECK(TestBit::noinline_rotl(upx_uint64_t(n), -s) == 0);

        CHECK(TestBit::noinline_rotr(upx_uint8_t(n), s) == 0);
        CHECK(TestBit::noinline_rotr(upx_uint16_t(n), s) == 0);
        CHECK(TestBit::noinline_rotr(upx_uint32_t(n), s) == 0);
        CHECK(TestBit::noinline_rotr(upx_uint64_t(n), s) == 0);

        CHECK(TestBit::noinline_rotr(upx_uint8_t(n), -s) == 0);
        CHECK(TestBit::noinline_rotr(upx_uint16_t(n), -s) == 0);
        CHECK(TestBit::noinline_rotr(upx_uint32_t(n), -s) == 0);
        CHECK(TestBit::noinline_rotr(upx_uint64_t(n), -s) == 0);

        CHECK(TestBit::noinline_countl_zero(upx_uint8_t(n)) == 8);
        CHECK(TestBit::noinline_countl_zero(upx_uint16_t(n)) == 16);
        CHECK(TestBit::noinline_countl_zero(upx_uint32_t(n)) == 32);
        CHECK(TestBit::noinline_countl_zero(upx_uint64_t(n)) == 64);

        CHECK(TestBit::noinline_countl_one(upx_uint8_t(n)) == 0);
        CHECK(TestBit::noinline_countl_one(upx_uint16_t(n)) == 0);
        CHECK(TestBit::noinline_countl_one(upx_uint32_t(n)) == 0);
        CHECK(TestBit::noinline_countl_one(upx_uint64_t(n)) == 0);

        CHECK(TestBit::noinline_countr_zero(upx_uint8_t(n)) == 8);
        CHECK(TestBit::noinline_countr_zero(upx_uint16_t(n)) == 16);
        CHECK(TestBit::noinline_countr_zero(upx_uint32_t(n)) == 32);
        CHECK(TestBit::noinline_countr_zero(upx_uint64_t(n)) == 64);

        CHECK(TestBit::noinline_countr_one(upx_uint8_t(n)) == 0);
        CHECK(TestBit::noinline_countr_one(upx_uint16_t(n)) == 0);
        CHECK(TestBit::noinline_countr_one(upx_uint32_t(n)) == 0);
        CHECK(TestBit::noinline_countr_one(upx_uint64_t(n)) == 0);

        CHECK(TestBit::noinline_popcount(upx_uint8_t(n)) == 0);
        CHECK(TestBit::noinline_popcount(upx_uint16_t(n)) == 0);
        CHECK(TestBit::noinline_popcount(upx_uint32_t(n)) == 0);
        CHECK(TestBit::noinline_popcount(upx_uint64_t(n)) == 0);
    }
    n = acc_vget_int(6, 0);
    {
        CHECK(!TestBit::noinline_has_single_bit(upx_uint8_t(n)));
        CHECK(!TestBit::noinline_has_single_bit(upx_uint16_t(n)));
        CHECK(!TestBit::noinline_has_single_bit(upx_uint32_t(n)));
        CHECK(!TestBit::noinline_has_single_bit(upx_uint64_t(n)));

        CHECK(TestBit::noinline_bit_ceil(upx_uint8_t(n)) == 8);
        CHECK(TestBit::noinline_bit_ceil(upx_uint16_t(n)) == 8);
        CHECK(TestBit::noinline_bit_ceil(upx_uint32_t(n)) == 8);
        CHECK(TestBit::noinline_bit_ceil(upx_uint64_t(n)) == 8);

        CHECK(TestBit::noinline_bit_floor(upx_uint8_t(n)) == 4);
        CHECK(TestBit::noinline_bit_floor(upx_uint16_t(n)) == 4);
        CHECK(TestBit::noinline_bit_floor(upx_uint32_t(n)) == 4);
        CHECK(TestBit::noinline_bit_floor(upx_uint64_t(n)) == 4);

        CHECK(TestBit::noinline_bit_width(upx_uint8_t(n)) == 3);
        CHECK(TestBit::noinline_bit_width(upx_uint16_t(n)) == 3);
        CHECK(TestBit::noinline_bit_width(upx_uint32_t(n)) == 3);
        CHECK(TestBit::noinline_bit_width(upx_uint64_t(n)) == 3);

        CHECK(TestBit::noinline_rotl(upx_uint8_t(n), s) == 12);
        CHECK(TestBit::noinline_rotl(upx_uint16_t(n), s) == 12);
        CHECK(TestBit::noinline_rotl(upx_uint32_t(n), s) == 12);
        CHECK(TestBit::noinline_rotl(upx_uint64_t(n), s) == 12);

        CHECK(TestBit::noinline_rotl(upx_uint8_t(n), -s) == 3);
        CHECK(TestBit::noinline_rotl(upx_uint16_t(n), -s) == 3);
        CHECK(TestBit::noinline_rotl(upx_uint32_t(n), -s) == 3);
        CHECK(TestBit::noinline_rotl(upx_uint64_t(n), -s) == 3);

        CHECK(TestBit::noinline_rotr(upx_uint8_t(n), s) == 3);
        CHECK(TestBit::noinline_rotr(upx_uint16_t(n), s) == 3);
        CHECK(TestBit::noinline_rotr(upx_uint32_t(n), s) == 3);
        CHECK(TestBit::noinline_rotr(upx_uint64_t(n), s) == 3);

        CHECK(TestBit::noinline_rotr(upx_uint8_t(n), -s) == 12);
        CHECK(TestBit::noinline_rotr(upx_uint16_t(n), -s) == 12);
        CHECK(TestBit::noinline_rotr(upx_uint32_t(n), -s) == 12);
        CHECK(TestBit::noinline_rotr(upx_uint64_t(n), -s) == 12);

        CHECK(TestBit::noinline_countl_zero(upx_uint8_t(n)) == 5);
        CHECK(TestBit::noinline_countl_zero(upx_uint16_t(n)) == 13);
        CHECK(TestBit::noinline_countl_zero(upx_uint32_t(n)) == 29);
        CHECK(TestBit::noinline_countl_zero(upx_uint64_t(n)) == 61);

        CHECK(TestBit::noinline_countl_one(upx_uint8_t(n)) == 0);
        CHECK(TestBit::noinline_countl_one(upx_uint16_t(n)) == 0);
        CHECK(TestBit::noinline_countl_one(upx_uint32_t(n)) == 0);
        CHECK(TestBit::noinline_countl_one(upx_uint64_t(n)) == 0);

        CHECK(TestBit::noinline_countr_zero(upx_uint8_t(n)) == 1);
        CHECK(TestBit::noinline_countr_zero(upx_uint16_t(n)) == 1);
        CHECK(TestBit::noinline_countr_zero(upx_uint32_t(n)) == 1);
        CHECK(TestBit::noinline_countr_zero(upx_uint64_t(n)) == 1);

        CHECK(TestBit::noinline_countr_one(upx_uint8_t(n)) == 0);
        CHECK(TestBit::noinline_countr_one(upx_uint16_t(n)) == 0);
        CHECK(TestBit::noinline_countr_one(upx_uint32_t(n)) == 0);
        CHECK(TestBit::noinline_countr_one(upx_uint64_t(n)) == 0);

        CHECK(TestBit::noinline_popcount(upx_uint8_t(n)) == 2);
        CHECK(TestBit::noinline_popcount(upx_uint16_t(n)) == 2);
        CHECK(TestBit::noinline_popcount(upx_uint32_t(n)) == 2);
        CHECK(TestBit::noinline_popcount(upx_uint64_t(n)) == 2);
    }
    (void) s;
    (void) n;
}

#endif // C++20

/*************************************************************************
// TriBool
**************************************************************************/

namespace {
template <class T>
struct TestTriBool final {
    static noinline void test(bool expect_true) {
        static_assert(std::is_class<T>::value);
        static_assert(std::is_nothrow_default_constructible<T>::value);
        static_assert(std::is_nothrow_destructible<T>::value);
        static_assert(std::is_standard_layout<T>::value);
        static_assert(std::is_trivially_copyable<T>::value);
        static_assert(sizeof(typename T::value_type) == sizeof(typename T::underlying_type));
        static_assert(alignof(typename T::value_type) == alignof(typename T::underlying_type));
#if defined(__m68k__) && defined(__atarist__) && defined(__GNUC__)
        // broken compiler or broken ABI
#elif defined(__i386__) && defined(__GNUC__) && (__GNUC__ == 7) && !defined(__clang__)
        static_assert(sizeof(T) == sizeof(typename T::underlying_type));
        // i386: "long long" enum align bug/ABI problem on older compilers
        static_assert(alignof(T) <= alignof(typename T::underlying_type));
#elif defined(__i386__) && defined(__clang__) && (__clang__ < 6)
        static_assert(sizeof(T) == sizeof(typename T::underlying_type));
        // i386: "long long" enum align bug/ABI problem on older compilers
        static_assert(alignof(T) <= alignof(typename T::underlying_type));
#else
        static_assert(sizeof(T) == sizeof(typename T::underlying_type));
        static_assert(alignof(T) == alignof(typename T::underlying_type));
#endif
        static_assert(!bool(T(false)));
        static_assert(bool(T(true)));
        static_assert(bool(T(T::Third)) == T::is_third_true);
        static_assert(T(false) == T::False);
        static_assert(T(true) == T::True);
        static_assert(T(T::False) == T::False);
        static_assert(T(T::True) == T::True);
        static_assert(T(T::Third) == T::Third);
        static_assert(T(T::Third) == T(9));
        static_assert(T(8) == T(9));
        static_assert(!(T(0) == T(9)));
        static_assert(!(T(1) == T(9)));
        static_assert(T(T::Third) == 9);
        static_assert(T(8) == 9);
        static_assert(!(T(0) == 9));
        static_assert(!(T(1) == 9));
        constexpr T array[] = {false, true, T::Third};
        static_assert(array[0].isStrictFalse());
        static_assert(array[1].isStrictTrue());
        static_assert(array[2].isThird());
        static_assert(sizeof(array) == 3 * sizeof(T));
        T a;
        assert(a.getValue() == T::False);
        assert(!a);
        assert(!bool(a));
        assert((!a ? true : false));
        assert(a.isStrictFalse());
        assert(!a.isStrictTrue());
        assert(a.isStrictBool());
        assert(!a.isThird());
        a = false;
        assert(a.getValue() == T::False);
        assert(!a);
        assert(!bool(a));
        assert((!a ? true : false));
        assert(a.isStrictFalse());
        assert(!a.isStrictTrue());
        assert(a.isStrictBool());
        assert(!a.isThird());
        a = true;
        assert(a.getValue() == T::True);
        assert(a);
        assert(bool(a));
        assert((a ? true : false));
        assert(!a.isStrictFalse());
        assert(a.isStrictTrue());
        assert(a.isStrictBool());
        assert(!a.isThird());
        a = T::Third;
        assert(a.getValue() == T::Third);
        assert(T::is_third_true == expect_true);
        if (expect_true) {
            assert(a);
            assert(bool(a));
            assert((a ? true : false));
        } else {
            assert(!a);
            assert(!bool(a));
            assert((!a ? true : false));
        }
        assert(!a.isStrictFalse());
        assert(!a.isStrictTrue());
        assert(!a.isStrictBool());
        assert(a.isThird());
        a = 99;
        assert(a.getValue() == T::Third);
        if (expect_true) {
            assert(a);
            assert(bool(a));
            assert((a ? true : false));
            assert((!a ? false : true));
        } else {
            assert(!a);
            assert(!bool(a));
            assert((a ? false : true));
            assert((!a ? true : false));
        }
        assert(!a.isStrictFalse());
        assert(!a.isStrictTrue());
        assert(!a.isStrictBool());
        assert(a.isThird());
        mem_clear(&a);
        assert(a.isStrictFalse());
    }
};
} // namespace

TEST_CASE("upx::TriBool") {
    using upx::TriBool, upx::tribool;
    static_assert(!tribool(false));
    static_assert(tribool(true));
    static_assert(!tribool(tribool::Third));
    TestTriBool<tribool>::test(false);
#if DEBUG || 1
    TestTriBool<TriBool<upx_int8_t> >::test(false);
    TestTriBool<TriBool<upx_uint8_t> >::test(false);
    TestTriBool<TriBool<upx_int16_t> >::test(false);
    TestTriBool<TriBool<upx_uint16_t> >::test(false);
    TestTriBool<TriBool<upx_int32_t> >::test(false);
    TestTriBool<TriBool<upx_uint32_t> >::test(false);
    TestTriBool<TriBool<upx_int64_t> >::test(false);
    TestTriBool<TriBool<upx_uint64_t> >::test(false);
    TestTriBool<TriBool<upx_int8_t, true> >::test(true);
    TestTriBool<TriBool<upx_uint8_t, true> >::test(true);
    TestTriBool<TriBool<upx_int16_t, true> >::test(true);
    TestTriBool<TriBool<upx_uint16_t, true> >::test(true);
    TestTriBool<TriBool<upx_int32_t, true> >::test(true);
    TestTriBool<TriBool<upx_uint32_t, true> >::test(true);
    TestTriBool<TriBool<upx_int64_t, true> >::test(true);
    TestTriBool<TriBool<upx_uint64_t, true> >::test(true);
#endif
}

/* vim:set ts=4 sw=4 et: */
