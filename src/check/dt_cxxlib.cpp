/* dt_cxxlib.cpp -- doctest check

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

// lots of tests (and probably quite a number of redundant tests)
// modern compilers will optimize away much of this code

#include "../conf.h"

/*************************************************************************
// compile-time checks
**************************************************************************/

// need extra parenthesis because the C preprocessor does not understand C++ templates
ACC_COMPILE_TIME_ASSERT_HEADER((upx::is_same_all_v<int>) )
ACC_COMPILE_TIME_ASSERT_HEADER((upx::is_same_all_v<int, int>) )
ACC_COMPILE_TIME_ASSERT_HEADER((upx::is_same_all_v<int, int, int>) )
ACC_COMPILE_TIME_ASSERT_HEADER((!upx::is_same_all_v<int, char>) )
ACC_COMPILE_TIME_ASSERT_HEADER((!upx::is_same_all_v<int, char, int>) )
ACC_COMPILE_TIME_ASSERT_HEADER((!upx::is_same_all_v<int, int, char>) )

ACC_COMPILE_TIME_ASSERT_HEADER((!upx::is_same_any_v<int>) )
ACC_COMPILE_TIME_ASSERT_HEADER((upx::is_same_any_v<int, int>) )
ACC_COMPILE_TIME_ASSERT_HEADER((upx::is_same_any_v<int, char, int>) )
ACC_COMPILE_TIME_ASSERT_HEADER((upx::is_same_any_v<int, int, char>) )
ACC_COMPILE_TIME_ASSERT_HEADER((!upx::is_same_any_v<int, char>) )
ACC_COMPILE_TIME_ASSERT_HEADER((!upx::is_same_any_v<int, char, char>) )
ACC_COMPILE_TIME_ASSERT_HEADER((!upx::is_same_any_v<int, char, long>) )

ACC_COMPILE_TIME_ASSERT_HEADER(usizeof(int) == sizeof(int))
ACC_COMPILE_TIME_ASSERT_HEADER(usizeof('a') == sizeof(char))
ACC_COMPILE_TIME_ASSERT_HEADER(usizeof("") == 1)
ACC_COMPILE_TIME_ASSERT_HEADER(usizeof("a") == 2)
ACC_COMPILE_TIME_ASSERT_HEADER(usizeof(0) == sizeof(int))
ACC_COMPILE_TIME_ASSERT_HEADER(usizeof(0L) == sizeof(long))
ACC_COMPILE_TIME_ASSERT_HEADER(usizeof(0LL) == sizeof(long long))
ACC_COMPILE_TIME_ASSERT_HEADER(usizeof(nullptr) == sizeof(void *))
ACC_COMPILE_TIME_ASSERT_HEADER(usizeof(sizeof(0)) == sizeof(size_t))
ACC_COMPILE_TIME_ASSERT_HEADER(usizeof(usizeof(0)) == sizeof(unsigned))

namespace compile_time = upx::compile_time;
ACC_COMPILE_TIME_ASSERT_HEADER(compile_time::string_len("") == 0)
ACC_COMPILE_TIME_ASSERT_HEADER(compile_time::string_len("a") == 1)
ACC_COMPILE_TIME_ASSERT_HEADER(compile_time::string_len("ab") == 2)
ACC_COMPILE_TIME_ASSERT_HEADER(compile_time::string_len("abc") == 3)

ACC_COMPILE_TIME_ASSERT_HEADER(compile_time::string_eq("", ""))
ACC_COMPILE_TIME_ASSERT_HEADER(!compile_time::string_eq("a", ""))
ACC_COMPILE_TIME_ASSERT_HEADER(!compile_time::string_eq("", "a"))
ACC_COMPILE_TIME_ASSERT_HEADER(compile_time::string_eq("abc", "abc"))
ACC_COMPILE_TIME_ASSERT_HEADER(!compile_time::string_eq("ab", "abc"))
ACC_COMPILE_TIME_ASSERT_HEADER(!compile_time::string_eq("abc", "ab"))

ACC_COMPILE_TIME_ASSERT_HEADER(!compile_time::string_lt("", ""))
ACC_COMPILE_TIME_ASSERT_HEADER(!compile_time::string_lt("a", ""))
ACC_COMPILE_TIME_ASSERT_HEADER(compile_time::string_lt("", "a"))
ACC_COMPILE_TIME_ASSERT_HEADER(!compile_time::string_lt("abc", "abc"))
ACC_COMPILE_TIME_ASSERT_HEADER(compile_time::string_lt("ab", "abc"))
ACC_COMPILE_TIME_ASSERT_HEADER(!compile_time::string_lt("abc", "ab"))
ACC_COMPILE_TIME_ASSERT_HEADER(!compile_time::string_lt("abc", "aba"))
ACC_COMPILE_TIME_ASSERT_HEADER(compile_time::string_lt("abc", "abz"))

ACC_COMPILE_TIME_ASSERT_HEADER(compile_time::string_ne("abc", "abz"))
ACC_COMPILE_TIME_ASSERT_HEADER(!compile_time::string_gt("abc", "abz"))
ACC_COMPILE_TIME_ASSERT_HEADER(!compile_time::string_ge("abc", "abz"))
ACC_COMPILE_TIME_ASSERT_HEADER(compile_time::string_le("abc", "abz"))

/*************************************************************************
// util
**************************************************************************/

TEST_CASE("ptr_reinterpret_cast") {
    // check that we don't trigger any -Wcast-align warnings
    using upx::ptr_reinterpret_cast;
    void *vp = nullptr;
    byte *bp = nullptr;
    int *ip = nullptr;
    double *dp = nullptr;

    assert((vp == ptr_reinterpret_cast<void *>(vp)));
    assert((vp == ptr_reinterpret_cast<void *>(bp)));
    assert((vp == ptr_reinterpret_cast<void *>(ip)));
    assert((vp == ptr_reinterpret_cast<void *>(dp)));

    assert((bp == ptr_reinterpret_cast<byte *>(vp)));
    assert((bp == ptr_reinterpret_cast<byte *>(bp)));
    assert((bp == ptr_reinterpret_cast<byte *>(ip)));
    assert((bp == ptr_reinterpret_cast<byte *>(dp)));

    assert((ip == ptr_reinterpret_cast<int *>(vp)));
    assert((ip == ptr_reinterpret_cast<int *>(bp)));
    assert((ip == ptr_reinterpret_cast<int *>(ip)));
    assert((ip == ptr_reinterpret_cast<int *>(dp)));

    assert((dp == ptr_reinterpret_cast<double *>(vp)));
    assert((dp == ptr_reinterpret_cast<double *>(bp)));
    assert((dp == ptr_reinterpret_cast<double *>(ip)));
    assert((dp == ptr_reinterpret_cast<double *>(dp)));

    const byte *bc = nullptr;
    const int *ic = nullptr;
    assert((bc == ptr_reinterpret_cast<byte *>(bp)));
    assert((bc == ptr_reinterpret_cast<const byte *>(bc)));
    assert((bc == ptr_reinterpret_cast<byte *>(ip)));
    assert((bc == ptr_reinterpret_cast<const byte *>(ic)));
    assert((ic == ptr_reinterpret_cast<int *>(bp)));
    assert((ic == ptr_reinterpret_cast<const int *>(bc)));
    assert((ic == ptr_reinterpret_cast<int *>(ip)));
    assert((ic == ptr_reinterpret_cast<const int *>(ic)));
}

TEST_CASE("noncopyable") {
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
// TriBool checks
**************************************************************************/

namespace {
template <class T>
struct TestTriBool {
    static void test(bool expect_true) {
        static_assert(std::is_class<T>::value);
        static_assert(std::is_nothrow_default_constructible<T>::value);
        static_assert(std::is_nothrow_destructible<T>::value);
        static_assert(std::is_standard_layout<T>::value);
        static_assert(std::is_trivially_copyable<T>::value);
        static_assert(sizeof(typename T::value_type) == sizeof(typename T::underlying_type));
        static_assert(alignof(typename T::value_type) == alignof(typename T::underlying_type));
#if (ACC_ARCH_M68K && ACC_OS_TOS && ACC_CC_GNUC) && defined(__MINT__)
        // broken compiler or broken ABI
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
        } else {
            assert(!a);
            assert(!bool(a));
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

TEST_CASE("TriBool") {
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
