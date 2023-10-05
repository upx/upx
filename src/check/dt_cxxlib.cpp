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
// TriBool
**************************************************************************/

namespace {
template <class T>
struct TestTriBool {
    static void test(bool expect_true, int x) noexcept {
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
        CHECK(!a);
        CHECK(a.isStrictFalse());
        CHECK(!a.isStrictTrue());
        CHECK(a.isStrictBool());
        CHECK(!a.isThird());
        a = false;
        CHECK(!a);
        CHECK(a.isStrictFalse());
        CHECK(!a.isStrictTrue());
        CHECK(a.isStrictBool());
        CHECK(!a.isThird());
        a = true;
        CHECK(a);
        CHECK(!a.isStrictFalse());
        CHECK(a.isStrictTrue());
        CHECK(a.isStrictBool());
        CHECK(!a.isThird());
        a = T::Third;
        if (expect_true)
            CHECK(a);
        else
            CHECK(!a);
        CHECK(!a.isStrictFalse());
        CHECK(!a.isStrictTrue());
        CHECK(!a.isStrictBool());
        CHECK(a.isThird());
        a = x;
        if (expect_true)
            CHECK(a);
        else
            CHECK(!a);
        CHECK(!a.isStrictFalse());
        CHECK(!a.isStrictTrue());
        CHECK(!a.isStrictBool());
        CHECK(a.isThird());
        mem_clear(&a);
        CHECK(a.isStrictFalse());
    }
};
} // namespace

TEST_CASE("TriBool") {
    using upx::TriBool, upx::tribool;
    //
    static_assert(!tribool(false));
    static_assert(tribool(true));
    static_assert(!tribool(tribool::Third));
    TestTriBool<tribool>::test(false, -1);
    //
    TestTriBool<TriBool<upx_int8_t> >::test(false, -1);
    TestTriBool<TriBool<upx_int64_t> >::test(false, -1);
    //
    TestTriBool<TriBool<unsigned, 2> >::test(true, 2);
    TestTriBool<TriBool<upx_int8_t, 2> >::test(true, 2);
    TestTriBool<TriBool<upx_uint8_t, 2> >::test(true, 2);
    TestTriBool<TriBool<upx_int64_t, 2> >::test(true, 2);
    TestTriBool<TriBool<upx_uint64_t, 2> >::test(true, 2);
}

/* vim:set ts=4 sw=4 et: */
