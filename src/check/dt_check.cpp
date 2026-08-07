/* dt_check.cpp -- doctest check

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

// doctest checks, and various tests to catch toolchain/qemu/sanitizer/valgrind/wine/etc
// problems; grown historically; modern compilers will optimize away much of this code

#include "../util/system_headers.h"
#include <cmath> // std::isinf std::isnan
#include "../conf.h"

/*************************************************************************
// upx_doctest_check()
//
// honors environment variables:
//   UPX_DEBUG_DOCTEST_DISABLE
//   UPX_DEBUG_DOCTEST_VERBOSE
//
// HINT: set "UPX_DEBUG_DOCTEST_DISABLE=1" for improved debugging experience
**************************************************************************/

int upx_doctest_check(int argc, char **argv) {
#if defined(DOCTEST_CONFIG_DISABLE)
    UNUSED(argc);
    UNUSED(argv);
    return 0;
#else
    if (is_envvar_true("UPX_DEBUG_DOCTEST_DISABLE", "UPX_DEBUG_DISABLE_DOCTEST"))
        return 0;
    bool minimal = true;   // don't show summary
    bool duration = false; // don't show timings
    bool success = false;  // don't show all succeeding tests
#if DEBUG
    // default for debug builds: do show the [doctest] summary
    minimal = false;
#endif
    const char *e = upx_getenv("UPX_DEBUG_DOCTEST_VERBOSE");
    if (e && e[0]) {
        if (strcmp(e, "0") == 0) {
            minimal = true;
        } else if (strcmp(e, "1") == 0) {
            minimal = false;
        } else if (strcmp(e, "2") == 0) {
            minimal = false;
            duration = true;
        } else if (strcmp(e, "3") == 0) {
            minimal = false;
            duration = true;
            success = true;
        }
    }
    doctest::Context context;
    if (minimal)
        context.setOption("dt-minimal", true);
    if (duration)
        context.setOption("dt-duration", true);
    if (success)
        context.setOption("dt-success", true);
    // this requires that main_get_options() understands/ignores doctest "--dt-XXX" options
    if (argc > 0 && argv != nullptr)
        context.applyCommandLine(argc, argv);
    int r = context.run();
    if (r != 0)
        return 1;
    if (context.shouldExit())
        return 2;
    return 0;
#endif // DOCTEST_CONFIG_DISABLE
}

int upx_doctest_check() { return upx_doctest_check(0, nullptr); }

/*************************************************************************
// check fundamental types
**************************************************************************/

static_assert(std::is_integral<ptrdiff_t>::value, "");
static_assert(std::is_integral<size_t>::value, "");
static_assert(std::is_integral<intptr_t>::value, "");
static_assert(std::is_integral<uintptr_t>::value, "");

static_assert(std::is_signed<ptrdiff_t>::value, "");
static_assert(!std::is_signed<size_t>::value, "");
static_assert(std::is_signed<intptr_t>::value, "");
static_assert(!std::is_signed<uintptr_t>::value, "");

static_assert(!std::is_unsigned<ptrdiff_t>::value, "");
static_assert(std::is_unsigned<size_t>::value, "");
static_assert(!std::is_unsigned<intptr_t>::value, "");
static_assert(std::is_unsigned<uintptr_t>::value, "");

#if defined(__SIZEOF_SHORT__)
static_assert(sizeof(short) == __SIZEOF_SHORT__, "");
#endif
#if defined(__SIZEOF_INT__)
static_assert(sizeof(int) == __SIZEOF_INT__, "");
#endif
#if defined(__SIZEOF_LONG__)
static_assert(sizeof(long) == __SIZEOF_LONG__, "");
#endif
#if defined(__SIZEOF_LONG_LONG__)
static_assert(sizeof(long long) == __SIZEOF_LONG_LONG__, "");
#endif
#if defined(__SIZEOF_INT128__)
static_assert(16 == __SIZEOF_INT128__, "");
static_assert(sizeof(__int128) == __SIZEOF_INT128__, "");
static_assert(sizeof(unsigned __int128) == __SIZEOF_INT128__, "");
static_assert(sizeof(upx_int128_t) == __SIZEOF_INT128__, "");
static_assert(sizeof(upx_uint128_t) == __SIZEOF_INT128__, "");
#endif
#if defined(__SIZEOF_PTRDIFF_T__)
static_assert(sizeof(ptrdiff_t) == __SIZEOF_PTRDIFF_T__, "");
#endif
#if defined(__SIZEOF_SIZE_T__)
static_assert(sizeof(size_t) == __SIZEOF_SIZE_T__, "");
#endif
#if defined(__SIZEOF_POINTER__)
static_assert(sizeof(void *) == __SIZEOF_POINTER__, "");
static_assert(sizeof(intptr_t) == __SIZEOF_POINTER__, "");
static_assert(sizeof(uintptr_t) == __SIZEOF_POINTER__, "");
#endif
#if defined(__SIZEOF_PTRADDR_T__)
static_assert(sizeof(__PTRADDR_TYPE__) == __SIZEOF_PTRADDR_T__, "");
static_assert(sizeof(upx_ptraddr_t) == __SIZEOF_PTRADDR_T__, "");
#endif

#if defined(__SCHAR_WIDTH__)
static_assert(8 * sizeof(signed char) == __SCHAR_WIDTH__, "");
#endif
#if defined(__SHRT_WIDTH__)
static_assert(8 * sizeof(short) == __SHRT_WIDTH__, "");
#endif
#if defined(__INT_WIDTH__)
static_assert(8 * sizeof(int) == __INT_WIDTH__, "");
#endif
#if defined(__LONG_WIDTH__)
static_assert(8 * sizeof(long) == __LONG_WIDTH__, "");
#endif
#if defined(__LLONG_WIDTH__)
static_assert(8 * sizeof(long long) == __LLONG_WIDTH__, "");
#endif
#if defined(__INTMAX_WIDTH__)
static_assert(8 * sizeof(intmax_t) == __INTMAX_WIDTH__, "");
static_assert(8 * sizeof(uintmax_t) == __INTMAX_WIDTH__, "");
#endif
#if defined(__PTRDIFF_WIDTH__)
static_assert(8 * sizeof(ptrdiff_t) == __PTRDIFF_WIDTH__, "");
#endif
#if defined(__SIZE_WIDTH__)
static_assert(8 * sizeof(size_t) == __SIZE_WIDTH__, "");
#endif
#if defined(__INTPTR_WIDTH__)
static_assert(8 * sizeof(intptr_t) == __INTPTR_WIDTH__, "");
static_assert(8 * sizeof(uintptr_t) == __INTPTR_WIDTH__, "");
#endif
#if defined(__UINTPTR_WIDTH__)
static_assert(8 * sizeof(intptr_t) == __UINTPTR_WIDTH__, "");
static_assert(8 * sizeof(uintptr_t) == __UINTPTR_WIDTH__, "");
#endif
#if defined(__PTRADDR_WIDTH__)
static_assert(8 * sizeof(__PTRADDR_TYPE__) == __PTRADDR_WIDTH__, "");
static_assert(8 * sizeof(upx_ptraddr_t) == __PTRADDR_WIDTH__, "");
#endif

// true types from compiler
typedef decltype((const char *) nullptr - (const char *) nullptr) true_ptrdiff_t;
typedef decltype(sizeof(0)) true_size_t;

// expected types from pre-defined macros
#if defined(__PTRDIFF_TYPE__)
typedef __PTRDIFF_TYPE__ expected_ptrdiff_t;
#endif
#if defined(__SIZE_TYPE__)
typedef __SIZE_TYPE__ expected_size_t;
#endif
#if defined(__INTPTR_TYPE__)
typedef __INTPTR_TYPE__ expected_intptr_t;
#endif
#if defined(__UINTPTR_TYPE__)
typedef __UINTPTR_TYPE__ expected_uintptr_t;
#endif
#if defined(__PTRADDR_TYPE__)
typedef __PTRADDR_TYPE__ expected_ptraddr_t;
#endif

#define ASSERT_COMPATIBLE_TYPE(A, B)                                                               \
    static_assert(std::is_integral<A>::value, "");                                                 \
    static_assert(std::is_integral<B>::value, "");                                                 \
    static_assert(std::is_signed<A>::value == std::is_signed<B>::value, "");                       \
    static_assert(std::is_unsigned<A>::value == std::is_unsigned<B>::value, "");                   \
    static_assert(std::is_signed<A>::value == !std::is_unsigned<A>::value, "");                    \
    static_assert(std::is_signed<B>::value == !std::is_unsigned<B>::value, "");                    \
    static_assert(sizeof(A) == sizeof(B), "");                                                     \
    static_assert(alignof(A) == alignof(B), "")

#define ASSERT_SAME_TYPE(A, B)                                                                     \
    ASSERT_COMPATIBLE_TYPE(A, B);                                                                  \
    static_assert(std::is_same<A, B>::value, "")

// C vs C++ headers
ASSERT_SAME_TYPE(ptrdiff_t, std::ptrdiff_t);
ASSERT_SAME_TYPE(size_t, std::size_t);
ASSERT_SAME_TYPE(intptr_t, std::intptr_t);
ASSERT_SAME_TYPE(uintptr_t, std::uintptr_t);

// true types
ASSERT_SAME_TYPE(ptrdiff_t, true_ptrdiff_t);
ASSERT_SAME_TYPE(size_t, true_size_t);
#if __cplusplus >= 201103L
typedef decltype(nullptr) true_nullptr_t;
static_assert(std::is_same<std::nullptr_t, true_nullptr_t>::value, "");
#endif

// expected types
#if defined(__PTRDIFF_TYPE__)
static_assert(std::is_signed<expected_ptrdiff_t>::value, "");
ASSERT_SAME_TYPE(ptrdiff_t, expected_ptrdiff_t);
#endif
#if defined(__SIZE_TYPE__)
static_assert(std::is_unsigned<expected_size_t>::value, "");
ASSERT_SAME_TYPE(size_t, expected_size_t);
#endif
#if defined(__INTPTR_TYPE__)
static_assert(std::is_signed<expected_intptr_t>::value, "");
ASSERT_COMPATIBLE_TYPE(intptr_t, expected_intptr_t); // some toolchains are buggy
#endif
#if defined(__UINTPTR_TYPE__)
static_assert(std::is_unsigned<expected_uintptr_t>::value, "");
ASSERT_COMPATIBLE_TYPE(uintptr_t, expected_uintptr_t); // some toolchains are buggy
#endif
#if defined(__PTRADDR_TYPE__)
static_assert(std::is_unsigned<expected_ptraddr_t>::value, "");
ASSERT_SAME_TYPE(upx_ptraddr_t, expected_ptraddr_t);
#endif

// UPX types
ASSERT_SAME_TYPE(signed char, upx_int8_t);
ASSERT_SAME_TYPE(unsigned char, upx_uint8_t);
ASSERT_SAME_TYPE(short, upx_int16_t);
ASSERT_SAME_TYPE(unsigned short, upx_uint16_t);
ASSERT_SAME_TYPE(int, upx_int32_t);
ASSERT_SAME_TYPE(unsigned, upx_uint32_t);
#if (__SIZEOF_LONG_LONG__ + 0 < 16)
ASSERT_SAME_TYPE(long long, upx_int64_t);
ASSERT_SAME_TYPE(unsigned long long, upx_uint64_t);
#endif
static_assert(sizeof(upx_int8_t) == 1, "");
static_assert(sizeof(upx_uint8_t) == 1, "");
static_assert(sizeof(upx_int16_t) == 2, "");
static_assert(sizeof(upx_uint16_t) == 2, "");
static_assert(sizeof(upx_int32_t) == 4, "");
static_assert(sizeof(upx_uint32_t) == 4, "");
static_assert(sizeof(upx_int64_t) == 8, "");
static_assert(sizeof(upx_uint64_t) == 8, "");
#if (__SIZEOF_INT128__ == 16)
static_assert(sizeof(upx_int128_t) == 16, "");
static_assert(sizeof(upx_uint128_t) == 16, "");
#endif
static_assert(alignof(upx_int8_t) == 1, "");
static_assert(alignof(upx_uint8_t) == 1, "");
static_assert(alignof(upx_int16_t) <= 2, "");
static_assert(alignof(upx_uint16_t) <= 2, "");
static_assert(alignof(upx_int32_t) <= 4, "");
static_assert(alignof(upx_uint32_t) <= 4, "");
static_assert(alignof(upx_int64_t) <= 8, "");
static_assert(alignof(upx_uint64_t) <= 8, "");
#if (__SIZEOF_INT128__ == 16)
static_assert(alignof(upx_int128_t) <= 16, "");
static_assert(alignof(upx_uint128_t) <= 16, "");
#endif

static_assert(sizeof(void *) == sizeof(char *), "");
static_assert(sizeof(void *) == sizeof(byte *), "");
static_assert(sizeof(void *) == sizeof(short *), "");
static_assert(sizeof(void *) == sizeof(int *), "");
static_assert(sizeof(void *) == sizeof(long *), "");
static_assert(sizeof(void *) == sizeof(long long *), "");
static_assert(sizeof(void *) == sizeof(float *), "");
static_assert(sizeof(void *) == sizeof(double *), "");
static_assert(sizeof(void *) == sizeof(void (*)(void)), "");

/*************************************************************************
// compile-time checks
**************************************************************************/

static_assert(no_bswap16(0x04030201) == 0x0201);
static_assert(no_bswap32(0x04030201) == 0x04030201);
static_assert(no_bswap64(0x0807060504030201ull) == 0x0807060504030201ull);
#if !(ACC_CC_MSC) || defined(upx_is_constant_evaluated)
static_assert(bswap16(0x04030201) == 0x0102);
static_assert(bswap32(0x04030201) == 0x01020304);
static_assert(bswap64(0x0807060504030201ull) == 0x0102030405060708ull);
static_assert(bswap16(bswap16(0xf4f3f2f1)) == no_bswap16(0xf4f3f2f1));
static_assert(bswap32(bswap32(0xf4f3f2f1)) == no_bswap32(0xf4f3f2f1));
static_assert(bswap64(bswap64(0xf8f7f6f5f4f3f2f1ull)) == no_bswap64(0xf8f7f6f5f4f3f2f1ull));
#endif

static_assert(sign_extend32(0u + 0, 8) == 0);
static_assert(sign_extend32(0u + 1, 8) == 1);
static_assert(sign_extend32(0u + 127, 8) == 127);
static_assert(sign_extend32(0u + 128, 8) == -128);
static_assert(sign_extend32(0u - 1, 8) == -1);
static_assert(sign_extend32(0u + 256, 8) == 0);
static_assert(sign_extend32(0u + 257, 8) == 1);
static_assert(sign_extend32(0u + 383, 8) == 127);
static_assert(sign_extend32(0u + 384, 8) == -128);
static_assert(sign_extend32(0u + 511, 8) == -1);
static_assert(sign_extend64(upx_uint64_t(0) + 0, 1) == 0);
static_assert(sign_extend64(upx_uint64_t(0) + 1, 1) == -1);

static_assert(CHAR_BIT == 8);
#if 0 // does not work with MSVC
#if '\0' - 1 < 0
static_assert(CHAR_MAX == 127);
#else
static_assert(CHAR_MAX == 255);
#endif
#if L'\0' - 1 < 0
static_assert((wchar_t) -1 < 0);
#else
static_assert((wchar_t) -1 > 0);
#endif
#endif

/*************************************************************************
// upx_compiler_sanity_check()
// assert a sane architecture and compiler
**************************************************************************/

namespace {

template <class T>
struct CheckIntegral final {
    // UPX extras
    static_assert(upx_is_integral<T>::value);
    static_assert(upx_is_integral_v<T>);
    struct TestT final {
        T a;
        T x[2];
    };
    template <class U>
    struct TestU final {
        U a = {};
        const U b = {};
        static constexpr U c = {};
        U x[2] = {};
        const U y[2] = {};
        static constexpr U z[2] = {};
    };
    template <class U>
    static noinline void checkU() noexcept {
        // UPX extras
        static_assert(upx_is_integral<U>::value);
        static_assert(upx_is_integral_v<U>);
#if __cplusplus <= 201703L
        static_assert(std::is_pod<U>::value); // std::is_pod is deprecated in C++20
#endif
        static_assert(std::is_standard_layout<U>::value);
#if __cplusplus <= 202302L
        static_assert(std::is_trivial<U>::value); // std::is_trivial is deprecated in C++26
#endif
        // more checks, these are probably implied by std::is_trivial
        static_assert(std::is_nothrow_default_constructible<U>::value);
        static_assert(std::is_nothrow_destructible<U>::value);
        static_assert(std::is_trivially_copyable<U>::value);
        static_assert(std::is_trivially_default_constructible<U>::value);
        {
            U a = {};
            const U b = {};
            constexpr U c = {};
            U x[2] = {};
            const U y[2] = {};
            constexpr U z[2] = {};
            assert_noexcept(a == 0);
            assert_noexcept(b == 0);
            assert_noexcept(c == 0);
            assert_noexcept(x[0] == 0 && x[1] == 0);
            assert_noexcept(y[0] == 0 && y[1] == 0);
            assert_noexcept(z[0] == 0 && z[1] == 0);
#if defined(upx_is_constant_evaluated)
            static_assert(c == 0);
            static_assert(z[0] == 0 && z[1] == 0);
#endif
        }
        {
            TestU<U> t;
            assert_noexcept(t.a == 0);
            assert_noexcept(t.b == 0);
            assert_noexcept(t.c == 0);
            assert_noexcept(t.x[0] == 0 && t.x[1] == 0);
            assert_noexcept(t.y[0] == 0 && t.y[1] == 0);
            assert_noexcept(t.z[0] == 0 && t.z[1] == 0);
        }
    }
    static noinline void check_core() noexcept {
        checkU<T>();
        checkU<typename std::add_const<T>::type>();
        {
            TestT t = {};
            assert_noexcept(t.a == 0);
            assert_noexcept(t.x[0] == 0 && t.x[1] == 0);
        }
        {
            const TestT t = {};
            assert_noexcept(t.a == 0);
            assert_noexcept(t.x[0] == 0 && t.x[1] == 0);
        }
        {
            constexpr TestT t = {};
            assert_noexcept(t.a == 0);
            assert_noexcept(t.x[0] == 0 && t.x[1] == 0);
        }
        {
            TestT t;
            mem_clear(&t);
            assert_noexcept(t.a == 0);
            assert_noexcept(t.x[0] == 0 && t.x[1] == 0);
        }
        {
            T zero, one, three, four;
            zero = 0;
            one = 1;
            three = 3;
            four = 4;
            assert_noexcept(zero == 0);
            assert_noexcept(one == 1);
            assert_noexcept(three == 3);
            assert_noexcept(four == 4);
            // min / max
            assert_noexcept(upx::min(one, four) == 1);
            assert_noexcept(upx::min(one, four) == one);
            assert_noexcept(upx::max(one, four) == 4);
            assert_noexcept(upx::max(one, four) == four);
        }
    }
    static noinline void check() noexcept {
        check_core();
        {
            T zero, one, three, four;
            zero = 0;
            one = 1;
            three = 3;
            four = 4;
            // align - needs binary expressions which do not work
            //   on CHERI uintptr_t because of pointer provenance
            assert_noexcept(upx::align_down(zero, four) == 0);
            assert_noexcept(upx::align_down(zero, four) == zero);
            assert_noexcept(upx::align_down(one, four) == 0);
            assert_noexcept(upx::align_down(one, four) == zero);
            assert_noexcept(upx::align_down(three, four) == 0);
            assert_noexcept(upx::align_down(three, four) == zero);
            assert_noexcept(upx::align_down(four, four) == 4);
            assert_noexcept(upx::align_down(four, four) == four);
            assert_noexcept(upx::align_up(zero, four) == 0);
            assert_noexcept(upx::align_up(zero, four) == zero);
            assert_noexcept(upx::align_up(one, four) == 4);
            assert_noexcept(upx::align_up(one, four) == four);
            assert_noexcept(upx::align_up(three, four) == 4);
            assert_noexcept(upx::align_up(three, four) == four);
            assert_noexcept(upx::align_up(four, four) == 4);
            assert_noexcept(upx::align_up(four, four) == four);
            assert_noexcept(upx::align_up_gap(zero, four) == 0);
            assert_noexcept(upx::align_up_gap(zero, four) == zero);
            assert_noexcept(upx::align_up_gap(one, four) == 3);
            assert_noexcept(upx::align_up_gap(one, four) == three);
            assert_noexcept(upx::align_up_gap(three, four) == 1);
            assert_noexcept(upx::align_up_gap(three, four) == one);
            assert_noexcept(upx::align_up_gap(four, four) == 0);
            assert_noexcept(upx::align_up_gap(four, four) == zero);
        }
    }
};

template <class T>
struct CheckAlignment final {
    static noinline void check() noexcept {
        COMPILE_TIME_ASSERT_ALIGNED1(T)
        struct alignas(1) Test1 final {
            char a;
            T b;
        };
        struct alignas(1) Test2 final {
            char a;
            T b[3];
        };
        COMPILE_TIME_ASSERT_ALIGNED1(Test1)
        COMPILE_TIME_ASSERT_ALIGNED1(Test2)
        Test1 t1[7];
        Test2 t2[7];
        static_assert(sizeof(Test1) == 1 + sizeof(T));
        static_assert(sizeof(t1) == 7 + 7 * sizeof(T));
        static_assert(sizeof(Test2) == 1 + 3 * sizeof(T));
        static_assert(sizeof(t2) == 7 + 21 * sizeof(T));
        UNUSED(t1);
        UNUSED(t2);
    }
};

template <class T>
struct TestBELE final {
    static_assert(upx::is_same_any_v<T, BE16, BE32, BE64, LE16, LE32, LE64>);
    static_assert(
        upx::is_same_any_v<typename T::integral_conversion_type, upx_uint32_t, upx_uint64_t>);
    static noinline bool test() noexcept {
        CheckIntegral<T>::check();
        CheckAlignment<T>::check();
        // arithmetic checks
        {
            T all_bits = {}; // == zero
            assert_noexcept(all_bits == 0);
            assert_noexcept(!upx::has_single_bit(all_bits));
            all_bits += 1;
            assert_noexcept(upx::has_single_bit(all_bits));
            all_bits -= 2;
            assert_noexcept(!upx::has_single_bit(all_bits));
            T v1;
            v1 = 1;
            v1 *= 4;
            v1 /= 2;
            v1 -= 1;
            T v2;
            v2 = 1;
            assert_noexcept((v1 == v2));
            assert_noexcept(!(v1 != v2));
            assert_noexcept((v1 <= v2));
            assert_noexcept((v1 >= v2));
            assert_noexcept(!(v1 < v2));
            assert_noexcept(!(v1 > v2));
            v2 ^= all_bits;
            assert_noexcept(!(v1 == v2));
            assert_noexcept((v1 != v2));
            assert_noexcept((v1 <= v2));
            assert_noexcept(!(v1 >= v2));
            assert_noexcept((v1 < v2));
            assert_noexcept(!(v1 > v2));
            v2 += 2;
            assert_noexcept(v1 == 1);
            assert_noexcept(v2 == 0);
            v1 <<= 1;
            v1 |= v2;
            v1 >>= 1;
            v2 &= v1;
            v2 /= v1;
            v2 *= v1;
            v1 += v2;
            v1 -= v2;
            assert_noexcept(v1 == 1);
            assert_noexcept(v2 == 0);
            if ((v1 ^ v2) != 1)
                return false;
        }
        // min/max
        {
            constexpr T a = {}; // == zero
            typedef typename T::integral_conversion_type U;
            constexpr U b = 1;
            assert_noexcept(upx::min(a, a) == 0);
            assert_noexcept(upx::min(a, a) == a);
            assert_noexcept(upx::min(a, b) == 0);
            assert_noexcept(upx::min(a, b) == a);
            assert_noexcept(upx::min(b, a) == 0);
            assert_noexcept(upx::min(b, a) == a);
            assert_noexcept(upx::min(b, b) == 1);
            assert_noexcept(upx::min(b, b) == b);
            assert_noexcept(upx::max(a, a) == 0);
            assert_noexcept(upx::max(a, a) == a);
            assert_noexcept(upx::max(a, b) == 1);
            assert_noexcept(upx::max(a, b) == b);
            assert_noexcept(upx::max(b, a) == 1);
            assert_noexcept(upx::max(b, a) == b);
            assert_noexcept(upx::max(b, b) == 1);
            assert_noexcept(upx::max(b, b) == b);
            T minus_one_t = {}, minus_two_t = {};
            minus_one_t -= 1;
            minus_two_t -= 2;
            const U minus_one_u = minus_one_t;
            const U minus_two_u = minus_two_t;
            assert_noexcept(upx::min(minus_one_t, minus_one_t) == minus_one_t);
            assert_noexcept(upx::min(minus_one_t, minus_one_t) == minus_one_u);
            assert_noexcept(upx::min(minus_one_u, minus_one_t) == minus_one_t);
            assert_noexcept(upx::min(minus_one_u, minus_one_t) == minus_one_u);
            assert_noexcept(upx::min(minus_one_t, minus_one_u) == minus_one_t);
            assert_noexcept(upx::min(minus_one_t, minus_one_u) == minus_one_u);
            assert_noexcept(upx::min(minus_two_t, minus_one_t) == minus_two_t);
            assert_noexcept(upx::min(minus_two_t, minus_one_t) == minus_two_u);
            assert_noexcept(upx::min(minus_two_u, minus_one_t) == minus_two_t);
            assert_noexcept(upx::min(minus_two_u, minus_one_t) == minus_two_u);
            assert_noexcept(upx::min(minus_two_t, minus_one_u) == minus_two_t);
            assert_noexcept(upx::min(minus_two_t, minus_one_u) == minus_two_u);
            assert_noexcept(upx::min(minus_one_t, minus_two_t) == minus_two_t);
            assert_noexcept(upx::min(minus_one_t, minus_two_t) == minus_two_u);
            assert_noexcept(upx::min(minus_one_u, minus_two_t) == minus_two_t);
            assert_noexcept(upx::min(minus_one_u, minus_two_t) == minus_two_u);
            assert_noexcept(upx::min(minus_one_t, minus_two_u) == minus_two_t);
            assert_noexcept(upx::min(minus_one_t, minus_two_u) == minus_two_u);
            assert_noexcept(upx::max(minus_one_t, minus_one_t) == minus_one_t);
            assert_noexcept(upx::max(minus_one_t, minus_one_t) == minus_one_u);
            assert_noexcept(upx::max(minus_one_u, minus_one_t) == minus_one_t);
            assert_noexcept(upx::max(minus_one_u, minus_one_t) == minus_one_u);
            assert_noexcept(upx::max(minus_one_t, minus_one_u) == minus_one_t);
            assert_noexcept(upx::max(minus_one_t, minus_one_u) == minus_one_u);
            assert_noexcept(upx::max(minus_two_t, minus_one_t) == minus_one_t);
            assert_noexcept(upx::max(minus_two_t, minus_one_t) == minus_one_u);
            assert_noexcept(upx::max(minus_two_u, minus_one_t) == minus_one_t);
            assert_noexcept(upx::max(minus_two_u, minus_one_t) == minus_one_u);
            assert_noexcept(upx::max(minus_two_t, minus_one_u) == minus_one_t);
            assert_noexcept(upx::max(minus_two_t, minus_one_u) == minus_one_u);
            assert_noexcept(upx::max(minus_one_t, minus_two_t) == minus_one_t);
            assert_noexcept(upx::max(minus_one_t, minus_two_t) == minus_one_u);
            assert_noexcept(upx::max(minus_one_u, minus_two_t) == minus_one_t);
            assert_noexcept(upx::max(minus_one_u, minus_two_t) == minus_one_u);
            assert_noexcept(upx::max(minus_one_t, minus_two_u) == minus_one_t);
            assert_noexcept(upx::max(minus_one_t, minus_two_u) == minus_one_u);
        }
        // constexpr
        {
            constexpr T zero = {};
            constexpr T zero_copy = T::make(zero);
            assert_noexcept(zero_copy == 0);
            assert_noexcept(!upx::has_single_bit(zero));
#if defined(upx_is_constant_evaluated)
            static_assert(zero_copy == 0);
            static_assert(zero_copy == zero);
            static_assert(!upx::has_single_bit(zero));
            static_assert(!upx::has_single_bit(zero_copy));
#endif
        }
#if defined(upx_is_constant_evaluated)
        {
            typedef typename T::integral_conversion_type U;
            constexpr T one = T::make(1);
            static_assert(one == 1);
            static_assert(upx::has_single_bit(one));
            constexpr T four = T::make(one + 3);
            static_assert(four == 4);
            static_assert(upx::has_single_bit(four));
            constexpr U all_bits_u = (U) T::make(U(0) - U(1));
            constexpr T all_bits = T::make(all_bits_u);
            static_assert(all_bits == all_bits_u);
            static_assert(all_bits == T::make(one - 2));
            static_assert(!upx::has_single_bit(all_bits));
            static_assert(one == one);
            static_assert(!(one == four));
            static_assert(!(one == all_bits));
            static_assert(one < four);
            static_assert(one < all_bits);
            static_assert(upx::min(one, four) == 1);
            static_assert(upx::min(one, four) == one);
            static_assert(upx::min(U(1), four) == 1);
            static_assert(upx::min(one, U(4)) == 1);
            static_assert(upx::max(one, four) == 4);
            static_assert(upx::max(one, four) == four);
            static_assert(upx::max(U(1), four) == 4);
            static_assert(upx::max(one, U(4)) == 4);
            static_assert(upx::align_down(one, four) == 0);
            static_assert(upx::align_up(one, four) == 4);
            static_assert(upx::align_up(one, four) == four);
            static_assert(upx::align_up_gap(one, four) == 3);
            static_assert(upx::align_up_gap(one, four) == T::make(four - 1));
            static_assert(upx::align_up_gap(one, four) == T::make(four - one));
            static_assert(upx::align_up_gap(one, four) == T::make(four + one - one - one));
            static_assert(upx::align_up_gap(one, four) == T::make(four + one - 2 * one));
            static_assert(upx::align_down_gap(T::make(4), four) == 0);
            static_assert(upx::align_down_gap(T::make(5), four) == 1);
            static_assert(upx::align_down_gap(T::make(6), four) == 2);
            static_assert(upx::align_down_gap(T::make(7), four) == 3);
            static_assert(upx::align_down_gap(T::make(8), four) == 0);
            constexpr T one_copy = T::make(one);
            static_assert(one_copy == one);
            static_assert(one_copy == 1);
        }
#endif
        return true;
    }
};

template <class T, bool T_is_signed>
struct CheckSignedness final {
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_signed_v<T> == T_is_signed);
    static_assert(std::is_unsigned_v<T> == !T_is_signed);
    template <class U, bool U_is_signed>
    static inline void checkU() noexcept {
        static_assert(std::is_integral_v<U>);
        static_assert(std::is_signed_v<U> == U_is_signed);
        static_assert(std::is_unsigned_v<U> == !U_is_signed);
        static_assert(sizeof(U) == sizeof(T));
        static_assert(alignof(U) == alignof(T));
        constexpr U all_bits = U(U(0) - U(1));
        static_assert(all_bits == U(~U(0)));
        static_assert(U_is_signed ? (all_bits < 0) : (all_bits > 0));
    }
    static noinline void check() noexcept {
        checkU<T, T_is_signed>();
        using signed_type = std::make_signed_t<T>;
        checkU<signed_type, true>();
        using unsigned_type = std::make_unsigned_t<T>;
        checkU<unsigned_type, false>();
    }
};

template <class A, class B>
struct CheckTypePair final {
    static_assert(std::is_integral_v<A>);
    static_assert(std::is_integral_v<B>);
    static_assert(std::is_signed_v<A>);
    static_assert(!std::is_signed_v<B>);
    static_assert(!std::is_unsigned_v<A>);
    static_assert(std::is_unsigned_v<B>);
    static_assert(std::is_same_v<A, std::make_signed_t<A> >);
    static_assert(std::is_same_v<A, std::make_signed_t<B> >);
    static_assert(std::is_same_v<B, std::make_unsigned_t<A> >);
    static_assert(std::is_same_v<B, std::make_unsigned_t<B> >);
    static_assert(sizeof(A) == sizeof(B));
    static_assert(alignof(A) == alignof(B));
    static inline void check() noexcept {}
};

template <class A, class B>
struct TestNoAliasingStruct final { // check working -fno-strict-aliasing
    static noinline bool test(A *a, B *b) noexcept {
        *a = 0;
        *b = 0;
        *b -= 3;
        return *a != 0;
    }
};
template <class A, class B>
static forceinline bool testNoAliasing(A *a, B *b) noexcept {
    return TestNoAliasingStruct<A, B>::test(a, b);
}

template <class T>
struct TestIntegerWrap final { // check working -fno-strict-overflow
    static inline bool inc_gt(const T x) noexcept { return x + 1 > x; }
    static inline bool dec_lt(const T x) noexcept { return x - 1 < x; }
    static inline bool neg_eq(const T x) noexcept { return T(T(0) - x) == x; }
};

//
// basic exception handling checks to early catch toolchain/qemu/wine/etc problems
//

struct TestDestructor /*not_final*/ {
    explicit TestDestructor(int *pp, int vv) noexcept : p(pp), v(vv) {}
    virtual noinline ~TestDestructor() noexcept { *p = (*p << 2) + v; }
    int *p;
    int v;
};

static noinline void throwSomeValue(int x) may_throw {
    if (x < 0)
        throw int(x);
    else
        throw size_t(x);
}

static noinline void check_exceptions_2(void (*func)(int), int *x) may_throw {
    TestDestructor d(x, *x);
    func(*x);
}

static noinline void check_basic_cxx_exception_handling(void (*func)(int)) noexcept {
    bool cxx_exception_handling_works = false;
    int x = 1;
    try {
        TestDestructor d(&x, 3);
        check_exceptions_2(func, &x);
    } catch (const size_t &e) {
        if (e == 1 && x == 23)
            cxx_exception_handling_works = true;
    } catch (...) {
    }
    assert_noexcept(cxx_exception_handling_works);
}

//
// basic floating point checks to early catch toolchain/qemu/wine/etc problems
//

static noinline float i64_f32_add_div(upx_int64_t a, upx_int64_t b) noexcept {
    return (a + b) / 1000000.0f;
}
static noinline float u64_f32_add_div(upx_uint64_t a, upx_uint64_t b) noexcept {
    return (a + b) / 1000000.0f;
}
static noinline float i64_f32_sub_div(upx_int64_t a, upx_int64_t b) noexcept {
    return (a - b) / 1000000.0f;
}
static noinline float u64_f32_sub_div(upx_uint64_t a, upx_uint64_t b) noexcept {
    return (a - b) / 1000000.0f;
}

static noinline double i64_f64_add_div(upx_int64_t a, upx_int64_t b) noexcept {
    return (a + b) / 1000000.0;
}
static noinline double u64_f64_add_div(upx_uint64_t a, upx_uint64_t b) noexcept {
    return (a + b) / 1000000.0;
}
static noinline double i64_f64_sub_div(upx_int64_t a, upx_int64_t b) noexcept {
    return (a - b) / 1000000.0;
}
static noinline double u64_f64_sub_div(upx_uint64_t a, upx_uint64_t b) noexcept {
    return (a - b) / 1000000.0;
}

template <class Int, class Float>
struct TestFloat final {
    static constexpr Int X = 1000000;
    static noinline Float div(Int a, Float f) noexcept { return a / f; }
    static noinline Float add_div(Int a, Int b, Float f) noexcept { return Float(a + b) / f; }
    static noinline Float sub_div(Int a, Int b, Float f) noexcept { return Float(a - b) / f; }
    static noinline Float add_div_x(Int a, Int b) noexcept { return Float(a + b) / Float(X); }
    static noinline Float sub_div_x(Int a, Int b) noexcept { return Float(a - b) / Float(X); }
    static noinline void test() noexcept {
        assert_noexcept(div(2 * X, Float(X)) == Float(2));
        assert_noexcept(add_div(X, X, Float(X)) == Float(2));
        assert_noexcept(add_div_x(X, X) == Float(2));
        assert_noexcept(sub_div(3 * X, X, Float(X)) == Float(2));
        assert_noexcept(sub_div_x(3 * X, X) == Float(2));
        // extra debugging; floating point edge cases cause portability problems in practice
        static const char envvar[] = "UPX_DEBUG_TEST_FLOAT_DIVISION_BY_ZERO";
        if (is_envvar_true(envvar)) {
#if defined(__FAST_MATH__)
            // warning: comparison with NaN always evaluates to false in fast floating point modes
            fprintf(stderr, "upx: WARNING: ignoring %s: __FAST_MATH__\n", envvar);
#else
            assert_noexcept(std::isnan(div(0, Float(0))));
            assert_noexcept(std::isinf(div(1, Float(0))));
            assert_noexcept(std::isinf(div(Int(-1), Float(0))));
#endif
        }
    }
};

static noinline void check_basic_floating_point() noexcept {
    assert_noexcept(i64_f32_add_div(1000000, 1000000) == 2.0f);
    assert_noexcept(u64_f32_add_div(1000000, 1000000) == 2.0f);
    assert_noexcept(i64_f32_sub_div(3000000, 1000000) == 2.0f);
    assert_noexcept(u64_f32_sub_div(3000000, 1000000) == 2.0f);
    assert_noexcept(i64_f64_add_div(1000000, 1000000) == 2.0);
    assert_noexcept(u64_f64_add_div(1000000, 1000000) == 2.0);
    assert_noexcept(i64_f64_sub_div(3000000, 1000000) == 2.0);
    assert_noexcept(u64_f64_sub_div(3000000, 1000000) == 2.0);
    TestFloat<upx_int32_t, float>::test();
    TestFloat<upx_uint32_t, float>::test();
    TestFloat<upx_int64_t, float>::test();
    TestFloat<upx_uint64_t, float>::test();
    TestFloat<upx_int32_t, double>::test();
    TestFloat<upx_uint32_t, double>::test();
    TestFloat<upx_int64_t, double>::test();
    TestFloat<upx_uint64_t, double>::test();
}

} // namespace

#define ACC_WANT_ACC_CHK_CH 1
#undef ACCCHK_ASSERT
#include "../util/miniacc.h"

void upx_compiler_sanity_check() noexcept {
    check_basic_floating_point();

    if (is_envvar_true("UPX_DEBUG_DOCTEST_DISABLE", "UPX_DEBUG_DISABLE_DOCTEST")) {
        // If UPX_DEBUG_DOCTEST_DISABLE is set then we don't want to throw any
        // exceptions in order to improve debugging experience.
    } else {
        // check working C++ exception handling to early catch toolchain/qemu/wine/etc problems
        check_basic_cxx_exception_handling(throwSomeValue);
    }

    // check_basic_decltype()
    {
        auto a = +0;
        constexpr auto b = -0;
        const auto &c = -1;
        static_assert((std::is_same<int, decltype(a)>::value));
        static_assert((std::is_same<const int, decltype(b)>::value));
        static_assert((std::is_same<const int &, decltype(c)>::value));
        UNUSED(a);
        UNUSED(b);
        UNUSED(c);
    }

#define ACC_WANT_ACC_CHK_CH 1
#undef ACCCHK_ASSERT
#define ACCCHK_ASSERT(expr) ACC_COMPILE_TIME_ASSERT(expr)
#include "../util/miniacc.h"
#undef ACCCHK_ASSERT

    static_assert(sizeof(char) == 1);
    static_assert(sizeof(short) == 2);
    static_assert(sizeof(int) == 4);
    static_assert(sizeof(long) >= 4);
    static_assert(sizeof(long long) >= 8);
    static_assert(sizeof(void *) >= 4);
    static_assert(sizeof(upx_off_t) >= 8);
    static_assert(sizeof(upx_off_t) >= sizeof(long long));

// __int64
#if defined(_MSC_VER)
    {
        ASSERT_SAME_TYPE(long long, __int64);
        ASSERT_SAME_TYPE(unsigned long long, unsigned __int64);
        typedef __int64 my_int64;
        typedef unsigned __int64 my_uint64;
        ASSERT_SAME_TYPE(long long, my_int64);
        ASSERT_SAME_TYPE(unsigned long long, my_uint64);
    }
#endif

    static_assert(sizeof(BE16) == 2);
    static_assert(sizeof(BE32) == 4);
    static_assert(sizeof(BE64) == 8);
    static_assert(sizeof(LE16) == 2);
    static_assert(sizeof(LE32) == 4);
    static_assert(sizeof(LE64) == 8);

    COMPILE_TIME_ASSERT_ALIGNED1(BE16)
    COMPILE_TIME_ASSERT_ALIGNED1(BE32)
    COMPILE_TIME_ASSERT_ALIGNED1(BE64)
    COMPILE_TIME_ASSERT_ALIGNED1(LE16)
    COMPILE_TIME_ASSERT_ALIGNED1(LE32)
    COMPILE_TIME_ASSERT_ALIGNED1(LE64)

    // check that these types are not some multi-word macro
#define CHECK_TYPE(T) (void) (T())
    CHECK_TYPE(int8_t);
    CHECK_TYPE(uint8_t);
    CHECK_TYPE(int16_t);
    CHECK_TYPE(uint16_t);
    CHECK_TYPE(int32_t);
    CHECK_TYPE(uint32_t);
    CHECK_TYPE(int64_t);
    CHECK_TYPE(uint64_t);
    CHECK_TYPE(intmax_t);
    CHECK_TYPE(uintmax_t);
    CHECK_TYPE(ptrdiff_t);
    CHECK_TYPE(size_t);
    CHECK_TYPE(intptr_t);
    CHECK_TYPE(uintptr_t);
#if 0
    CHECK_TYPE(acc_int8_t);
    CHECK_TYPE(acc_uint8_t);
    CHECK_TYPE(acc_int16_t);
    CHECK_TYPE(acc_uint16_t);
    CHECK_TYPE(acc_int32_t);
    CHECK_TYPE(acc_uint32_t);
    CHECK_TYPE(acc_int64_t);
    CHECK_TYPE(acc_uint64_t);
    CHECK_TYPE(acc_intptr_t);
    CHECK_TYPE(acc_uintptr_t);
#endif
    CHECK_TYPE(upx_int8_t);
    CHECK_TYPE(upx_uint8_t);
    CHECK_TYPE(upx_int16_t);
    CHECK_TYPE(upx_uint16_t);
    CHECK_TYPE(upx_int32_t);
    CHECK_TYPE(upx_uint32_t);
    CHECK_TYPE(upx_int64_t);
    CHECK_TYPE(upx_uint64_t);
#if (__SIZEOF_INT128__ == 16)
    CHECK_TYPE(upx_int128_t);
    CHECK_TYPE(upx_uint128_t);
#endif
    CHECK_TYPE(upx_off_t);
    CHECK_TYPE(upx_uptrdiff_t);
    CHECK_TYPE(upx_ssize_t);
    CHECK_TYPE(upx_ptraddr_t);
    CHECK_TYPE(upx_uintptr_t);
#undef CHECK_TYPE

    CheckIntegral<char>::check();
    CheckIntegral<signed char>::check();
    CheckIntegral<unsigned char>::check();
    CheckIntegral<short>::check();
    CheckIntegral<unsigned short>::check();
    CheckIntegral<int>::check();
    CheckIntegral<unsigned>::check();
    CheckIntegral<long>::check();
    CheckIntegral<unsigned long>::check();
    CheckIntegral<long long>::check();
    CheckIntegral<unsigned long long>::check();
    CheckIntegral<intmax_t>::check();
    CheckIntegral<uintmax_t>::check();
    CheckIntegral<upx_int8_t>::check();
    CheckIntegral<upx_uint8_t>::check();
    CheckIntegral<upx_int16_t>::check();
    CheckIntegral<upx_uint16_t>::check();
    CheckIntegral<upx_int32_t>::check();
    CheckIntegral<upx_uint32_t>::check();
    CheckIntegral<upx_int64_t>::check();
    CheckIntegral<upx_uint64_t>::check();
#if (__SIZEOF_INT128__ == 16)
#if defined(_CPP_VER) || defined(_WIN32) // int128 is not fully supported by MSVC libstdc++ yet
#else
    CheckIntegral<upx_int128_t>::check();
    CheckIntegral<upx_uint128_t>::check();
#endif
#endif
    CheckIntegral<upx_off_t>::check();
    CheckIntegral<ptrdiff_t>::check();
    CheckIntegral<upx_uptrdiff_t>::check();
    CheckIntegral<upx_ssize_t>::check();
    CheckIntegral<size_t>::check();
    CheckIntegral<upx_ptraddr_t>::check();
#if defined(__CHERI__) && defined(__CHERI_PURE_CAPABILITY__)
    static_assert(sizeof(upx_ptraddr_t) == 8);
    static_assert(alignof(upx_ptraddr_t) == 8);
    static_assert(sizeof(void *) == 16);
    static_assert(alignof(void *) == 16);
    static_assert(sizeof(uintptr_t) == 16);
    static_assert(alignof(uintptr_t) == 16);
    // warning: binary expression on capability types 'unsigned __intcap' and 'unsigned __intcap'
    CheckIntegral<intptr_t>::check_core();
    CheckIntegral<uintptr_t>::check_core();
    CheckIntegral<upx_uintptr_t>::check_core();
#else
    CheckIntegral<intptr_t>::check();
    CheckIntegral<uintptr_t>::check();
    CheckIntegral<upx_uintptr_t>::check();
#endif

    CheckSignedness<char, false>::check(); // -funsigned-char
    CheckSignedness<signed char, true>::check();
    CheckSignedness<unsigned char, false>::check();
    CheckSignedness<short, true>::check();
    CheckSignedness<unsigned short, false>::check();
    CheckSignedness<int, true>::check();
    CheckSignedness<unsigned, false>::check();
    CheckSignedness<long, true>::check();
    CheckSignedness<unsigned long, false>::check();
    CheckSignedness<long long, true>::check();
    CheckSignedness<unsigned long long, false>::check();
    CheckSignedness<intmax_t, true>::check();
    CheckSignedness<uintmax_t, false>::check();
    CheckSignedness<upx_int8_t, true>::check();
    CheckSignedness<upx_uint8_t, false>::check();
    CheckSignedness<upx_int16_t, true>::check();
    CheckSignedness<upx_uint16_t, false>::check();
    CheckSignedness<upx_int32_t, true>::check();
    CheckSignedness<upx_uint32_t, false>::check();
    CheckSignedness<upx_int64_t, true>::check();
    CheckSignedness<upx_uint64_t, false>::check();
#if (__SIZEOF_INT128__ == 16)
#if defined(_CPP_VER) || defined(_WIN32) // int128 is not fully supported by MSVC libstdc++ yet
#else
    CheckSignedness<upx_int128_t, true>::check();
    CheckSignedness<upx_uint128_t, false>::check();
#endif
#endif
    CheckSignedness<upx_off_t, true>::check();
    CheckSignedness<ptrdiff_t, true>::check();
    CheckSignedness<upx_uptrdiff_t, false>::check();
    CheckSignedness<upx_ssize_t, true>::check();
    CheckSignedness<size_t, false>::check();
    CheckSignedness<upx_ptraddr_t, false>::check();
    CheckSignedness<intptr_t, true>::check();
    CheckSignedness<uintptr_t, false>::check();
    CheckSignedness<upx_uintptr_t, false>::check();

#define CHECK_TYPE_PAIR(A, B)                                                                      \
    CheckTypePair<A, B>::check();                                                                  \
    static_assert(alignof(A) == alignof(B))
    CHECK_TYPE_PAIR(signed char, unsigned char);
    CHECK_TYPE_PAIR(short, unsigned short);
    CHECK_TYPE_PAIR(int, unsigned);
    CHECK_TYPE_PAIR(long, unsigned long);
    CHECK_TYPE_PAIR(long long, unsigned long long);
    CHECK_TYPE_PAIR(intmax_t, uintmax_t);
    CHECK_TYPE_PAIR(upx_int8_t, upx_uint8_t);
    CHECK_TYPE_PAIR(upx_int16_t, upx_uint16_t);
    CHECK_TYPE_PAIR(upx_int32_t, upx_uint32_t);
    CHECK_TYPE_PAIR(upx_int64_t, upx_uint64_t);
#if (__SIZEOF_INT128__ == 16)
#if defined(_CPP_VER) || defined(_WIN32) // int128 is not fully supported by MSVC libstdc++ yet
#else
    CHECK_TYPE_PAIR(upx_int128_t, upx_uint128_t);
#endif
#endif
    CHECK_TYPE_PAIR(ptrdiff_t, upx_uptrdiff_t);
    CHECK_TYPE_PAIR(upx_ssize_t, size_t);
    CHECK_TYPE_PAIR(upx_sptraddr_t, upx_ptraddr_t);
    CHECK_TYPE_PAIR(intptr_t, uintptr_t);
    CHECK_TYPE_PAIR(acc_intptr_t, acc_uintptr_t);
#undef CHECK_TYPE_PAIR

    static_assert(sizeof(upx_charptr_unit_type) == 1);
    COMPILE_TIME_ASSERT_ALIGNED1(upx_charptr_unit_type)
    static_assert(sizeof(*((charptr) nullptr)) == 1);

    // check UPX_VERSION_xxx
    {
        using upx::compile_time::mem_eq;
        using upx::compile_time::string_len;
        static_assert(string_len(UPX_VERSION_STRING4) == 4);
        static_assert(string_len(UPX_VERSION_YEAR) == 4);
        static_assert(string_len(UPX_VERSION_DATE_ISO) == 10);
        static_assert(string_len(UPX_VERSION_DATE) == 12 || string_len(UPX_VERSION_DATE) == 13);
        static_assert(mem_eq(UPX_VERSION_STRING4, UPX_VERSION_STRING, 3));
        static_assert(mem_eq(UPX_VERSION_YEAR, UPX_VERSION_DATE_ISO, 4));
        static_assert(mem_eq(UPX_VERSION_YEAR, &UPX_VERSION_DATE[sizeof(UPX_VERSION_DATE) - 5], 4));
        char buf[64];
        constexpr long long v = UPX_VERSION_HEX;
        upx_safe_snprintf_noexcept(buf, sizeof(buf), "%lld.%lld.%lld", (v >> 16), (v >> 8) & 255,
                                   v & 255);
        assert_noexcept(strcmp(buf, UPX_VERSION_STRING) == 0);
        upx_safe_snprintf_noexcept(buf, sizeof(buf), "%lld.%lld%lld", (v >> 16), (v >> 8) & 255,
                                   v & 255);
        assert_noexcept(strcmp(buf, UPX_VERSION_STRING4) == 0);
    }

    if (gitrev[0]) {
        size_t revlen = strlen(gitrev);
        if (strncmp(gitrev, "ERROR", 5) == 0) {
            assert_noexcept(revlen == 5 || revlen == 6);
        } else {
            assert_noexcept(revlen == 12 || revlen == 13);
        }
        if (revlen == 6 || revlen == 13) {
            assert_noexcept(gitrev[revlen - 1] == '+');
        }
    }
    assert_noexcept(UPX_RSIZE_MAX_MEM == 805306368);

#if DEBUG || 1
    assert_noexcept(TestBELE<LE16>::test());
    assert_noexcept(TestBELE<LE32>::test());
    assert_noexcept(TestBELE<LE64>::test());
    assert_noexcept(TestBELE<BE16>::test());
    assert_noexcept(TestBELE<BE32>::test());
    assert_noexcept(TestBELE<BE64>::test());
    {
        alignas(16) static const constexpr byte dd[32] = {
            0, 0, 0, 0,    0,    0,    0,    0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8, 0,
            0, 0, 0, 0x7f, 0x7e, 0x7d, 0x7c, 0x7b, 0x7a, 0x79, 0x78, 0,    0,    0,    0,    0};
#if !defined(upx_fake_alignas_16)
        assert_noexcept2(ptr_is_aligned<16>(dd));
        assert_noexcept2(ptr_is_aligned(dd, 16));
        assert_noexcept2(ptr_align_down(dd + 1, 16) == (dd));
        assert_noexcept2(ptr_align_up(dd + 1, 16) == (dd + 16));
#endif
        const constexpr byte *d = dd + 7;
        static_assert(upx::compile_time::get_be16(d) == 0xfffe);
        static_assert(upx::compile_time::get_be24(d) == 0xfffefd);
        static_assert(upx::compile_time::get_be32(d) == 0xfffefdfc);
        static_assert(upx::compile_time::get_be64(d) == 0xfffefdfcfbfaf9f8ULL);
        static_assert(upx::compile_time::get_le16(d) == 0xfeff);
        static_assert(upx::compile_time::get_le24(d) == 0xfdfeff);
        static_assert(upx::compile_time::get_le32(d) == 0xfcfdfeff);
        static_assert(upx::compile_time::get_le64(d) == 0xf8f9fafbfcfdfeffULL);
        assert_noexcept(upx_adler32(d, 4) == 0x09f003f7);
        assert_noexcept(upx_adler32(d, 4, 0) == 0x09ec03f6);
        assert_noexcept(upx_adler32(d, 4, 1) == 0x09f003f7);
        const N_BELE_RTP::AbstractPolicy *bele;
        bele = &N_BELE_RTP::be_policy;
        assert_noexcept(get_be16(d) == 0xfffe);
        assert_noexcept(bele->get16(d) == 0xfffe);
        assert_noexcept(get_be16_signed(d) == -2);
        assert_noexcept(bele->get16_signed(d) == -2);
        assert_noexcept(get_be24(d) == 0xfffefd);
        assert_noexcept(bele->get24(d) == 0xfffefd);
        assert_noexcept(get_be24_signed(d) == -259);
        assert_noexcept(bele->get24_signed(d) == -259);
        assert_noexcept(get_be32(d) == 0xfffefdfc);
        assert_noexcept(bele->get32(d) == 0xfffefdfc);
        assert_noexcept(get_be32_signed(d) == -66052);
        assert_noexcept(bele->get32_signed(d) == -66052);
        assert_noexcept(get_be64(d) == 0xfffefdfcfbfaf9f8ULL);
        assert_noexcept(bele->get64(d) == 0xfffefdfcfbfaf9f8ULL);
        assert_noexcept(get_be64_signed(d) == -283686952306184LL);
        assert_noexcept(bele->get64_signed(d) == -283686952306184LL);
        bele = &N_BELE_RTP::le_policy;
        assert_noexcept(get_le16(d) == 0xfeff);
        assert_noexcept(bele->get16(d) == 0xfeff);
        assert_noexcept(get_le16_signed(d) == -257);
        assert_noexcept(bele->get16_signed(d) == -257);
        assert_noexcept(get_le24(d) == 0xfdfeff);
        assert_noexcept(bele->get24(d) == 0xfdfeff);
        assert_noexcept(get_le24_signed(d) == -131329);
        assert_noexcept(bele->get24_signed(d) == -131329);
        assert_noexcept(get_le32(d) == 0xfcfdfeff);
        assert_noexcept(bele->get32(d) == 0xfcfdfeff);
        assert_noexcept(get_le32_signed(d) == -50462977);
        assert_noexcept(bele->get32_signed(d) == -50462977);
        assert_noexcept(get_le64(d) == 0xf8f9fafbfcfdfeffULL);
        assert_noexcept(bele->get64(d) == 0xf8f9fafbfcfdfeffULL);
        assert_noexcept(get_le64_signed(d) == -506097522914230529LL);
        assert_noexcept(bele->get64_signed(d) == -506097522914230529LL);
        static_assert(get_be24(d) == 0xfffefd);
        static_assert(get_le24(d) == 0xfdfeff);
#if defined(upx_is_constant_evaluated)
        static_assert(get_be24_signed(d) == -259);
        static_assert(get_le24_signed(d) == -131329);
        static_assert(get_be16(d) == 0xfffe);
        static_assert(get_be16_signed(d) == -2);
        static_assert(get_be32(d) == 0xfffefdfc);
        static_assert(get_be32_signed(d) == -66052);
        static_assert(get_be64(d) == 0xfffefdfcfbfaf9f8ULL);
        static_assert(get_be64_signed(d) == -283686952306184LL);
        static_assert(get_le16(d) == 0xfeff);
        static_assert(get_le16_signed(d) == -257);
        static_assert(get_le32(d) == 0xfcfdfeff);
        static_assert(get_le32_signed(d) == -50462977);
        static_assert(get_le64(d) == 0xf8f9fafbfcfdfeffULL);
        static_assert(get_le64_signed(d) == -506097522914230529LL);
#endif
        assert_noexcept(find_be16(d, 2, 0xfffe) == 0);
        assert_noexcept(find_le16(d, 2, 0xfeff) == 0);
        assert_noexcept(find_be32(d, 4, 0xfffefdfc) == 0);
        assert_noexcept(find_le32(d, 4, 0xfcfdfeff) == 0);
        const constexpr byte *e = d + 12;
        assert_noexcept(get_be16_signed(e) == 32638);
        assert_noexcept(get_be24_signed(e) == 8355453);
        assert_noexcept(get_be32_signed(e) == 2138996092);
        assert_noexcept(get_be64_signed(e) == 9186918263483431288LL);
#if defined(upx_is_constant_evaluated)
        static_assert(get_be16_signed(e) == 32638);
        static_assert(get_be24_signed(e) == 8355453);
        static_assert(get_be32_signed(e) == 2138996092);
        static_assert(get_be64_signed(e) == 9186918263483431288LL);
#endif
    }
#if defined(upx_is_constant_evaluated)
    {
        constexpr upx_uint16_t v16 = 0x0201;
        constexpr upx_uint32_t v32 = 0x04030201;
        constexpr upx_uint64_t v64 = 0x0807060504030201ull;
        constexpr BE16 be16 = BE16::make(v16);
        constexpr BE32 be32 = BE32::make(v32);
        constexpr BE64 be64 = BE64::make(v64);
        constexpr LE16 le16 = LE16::make(v16);
        constexpr LE32 le32 = LE32::make(v32);
        constexpr LE64 le64 = LE64::make(v64);
        using upx::compile_time::mem_eq;
        static_assert(mem_eq(be16.d, "\x02\x01", 2));
        static_assert(mem_eq(be32.d, "\x04\x03\x02\x01", 4));
        static_assert(mem_eq(be64.d, "\x08\x07\x06\x05\x04\x03\x02\x01", 8));
        static_assert(mem_eq(le16.d, "\x01\x02", 2));
        static_assert(mem_eq(le32.d, "\x01\x02\x03\x04", 4));
        static_assert(mem_eq(le64.d, "\x01\x02\x03\x04\x05\x06\x07\x08", 8));
        constexpr NE16 ne16 = NE16::make(v16);
        constexpr NE32 ne32 = NE32::make(v32);
        constexpr NE64 ne64 = NE64::make(v64);
        assert_noexcept(memcmp(&v16, ne16.d, 2) == 0);
        assert_noexcept(memcmp(&v32, ne32.d, 4) == 0);
        assert_noexcept(memcmp(&v64, ne64.d, 8) == 0);
    }
#endif
#if DEBUG >= 1
    {
        for (int i = -256; i < 256; i++) {
            {
                const unsigned u = i;
                assert_noexcept(sign_extend32(u, 1) == ((i & 1) ? -1 : 0));
                assert_noexcept(sign_extend32(u, 2) == ((i & 2) ? -2 + (i & 1) : (i & 1)));
                assert_noexcept(sign_extend32(u, 3) == ((i & 4) ? -4 + (i & 3) : (i & 3)));
                assert_noexcept(sign_extend32(u, 4) == ((i & 8) ? -8 + (i & 7) : (i & 7)));
                assert_noexcept(sign_extend32(u, 5) == ((i & 16) ? -16 + (i & 15) : (i & 15)));
                assert_noexcept(sign_extend32(u, 6) == ((i & 32) ? -32 + (i & 31) : (i & 31)));
                assert_noexcept(sign_extend32(u, 7) == ((i & 64) ? -64 + (i & 63) : (i & 63)));
                assert_noexcept(sign_extend32(u, 8) == ((i & 128) ? -128 + (i & 127) : (i & 127)));
                assert_noexcept(sign_extend32(u, 9) == i);
                assert_noexcept(sign_extend32(u, 32) == i);
                assert_noexcept(sign_extend32(0u - u, 32) == -i);
            }
            {
                const upx_uint64_t u = i;
                assert_noexcept(sign_extend64(u, 1) == ((i & 1) ? -1 : 0));
                assert_noexcept(sign_extend64(u, 2) == ((i & 2) ? -2 + (i & 1) : (i & 1)));
                assert_noexcept(sign_extend64(u, 3) == ((i & 4) ? -4 + (i & 3) : (i & 3)));
                assert_noexcept(sign_extend64(u, 4) == ((i & 8) ? -8 + (i & 7) : (i & 7)));
                assert_noexcept(sign_extend64(u, 5) == ((i & 16) ? -16 + (i & 15) : (i & 15)));
                assert_noexcept(sign_extend64(u, 6) == ((i & 32) ? -32 + (i & 31) : (i & 31)));
                assert_noexcept(sign_extend64(u, 7) == ((i & 64) ? -64 + (i & 63) : (i & 63)));
                assert_noexcept(sign_extend64(u, 8) == ((i & 128) ? -128 + (i & 127) : (i & 127)));
                assert_noexcept(sign_extend64(u, 9) == i);
                assert_noexcept(sign_extend64(u, 32) == i);
                assert_noexcept(sign_extend64(upx_uint64_t(0) - u, 32) == -i);
                assert_noexcept(sign_extend64(u, 64) == i);
                assert_noexcept(sign_extend64(upx_uint64_t(0) - u, 64) == -i);
            }
        }
    }
#endif
    {
        unsigned dd;
        void *const d = &dd;
        dd = ne32_to_le32(0xf7f6f5f4);
        assert_noexcept(get_le26(d) == 0x03f6f5f4);
        set_le26(d, 0);
        assert_noexcept(get_le26(d) == 0);
        assert_noexcept(dd == ne32_to_le32(0xf4000000));
        set_le26(d, 0xff020304);
        assert_noexcept(get_le26(d) == 0x03020304);
        assert_noexcept(dd == ne32_to_le32(0xf7020304));
    }
    {
        upx_uint16_t a = 0;
        upx_uint32_t b = 0;
        upx_uint64_t c = 0;
        set_ne16(&a, 0x04030201); // ignore upper bits
        set_ne32(&b, 0x04030201);
        set_ne64(&c, 0x0807060504030201ull);
        assert_noexcept(a == 0x0201);
        assert_noexcept(b == 0x04030201);
        assert_noexcept(c == 0x0807060504030201ull);
        assert_noexcept(get_ne16(&a) == 0x0201);
        assert_noexcept(get_ne32(&b) == 0x04030201);
        assert_noexcept(get_ne64(&c) == 0x0807060504030201ull);
    }
#endif // DEBUG
    union {
        short v_short;
        int v_int;
        long v_long;
        long long v_llong;
        BE16 b16;
        BE32 b32;
        BE64 b64;
        LE16 l16;
        LE32 l32;
        LE64 l64;
    } u;
    assert_noexcept(testNoAliasing(&u.v_short, &u.b32));
    assert_noexcept(testNoAliasing(&u.v_short, &u.l32));
    assert_noexcept(testNoAliasing(&u.v_int, &u.b64));
    assert_noexcept(testNoAliasing(&u.v_int, &u.l64));
    // check working -fno-strict-aliasing
    assert_noexcept(testNoAliasing(&u.v_short, &u.v_int));
    assert_noexcept(testNoAliasing(&u.v_int, &u.v_long));
    assert_noexcept(testNoAliasing(&u.v_int, &u.v_llong));
    assert_noexcept(testNoAliasing(&u.v_long, &u.v_llong));

#if 1 && (ACC_CC_MSC) && (defined(_M_ARM64) || defined(_M_ARM64EC))
    // @COMPILER_BUG @MSVC_BUG
#else
    assert_noexcept(TestIntegerWrap<unsigned>::inc_gt(0));
    assert_noexcept(!TestIntegerWrap<unsigned>::inc_gt(UINT_MAX));
    assert_noexcept(TestIntegerWrap<unsigned>::dec_lt(1));
    assert_noexcept(!TestIntegerWrap<unsigned>::dec_lt(0));
    assert_noexcept(TestIntegerWrap<unsigned>::neg_eq(0));
    assert_noexcept(!TestIntegerWrap<unsigned>::neg_eq(1));
    assert_noexcept(!TestIntegerWrap<unsigned>::neg_eq(UINT_MAX));
    // check working -fno-strict-overflow
    assert_noexcept(TestIntegerWrap<int>::inc_gt(0));
    assert_noexcept(!TestIntegerWrap<int>::inc_gt(INT_MAX));
    assert_noexcept(TestIntegerWrap<int>::dec_lt(0));
    assert_noexcept(!TestIntegerWrap<int>::dec_lt(INT_MIN));
    assert_noexcept(TestIntegerWrap<int>::neg_eq(0));
    assert_noexcept(!TestIntegerWrap<int>::neg_eq(1));
    assert_noexcept(!TestIntegerWrap<int>::neg_eq(INT_MAX));
    assert_noexcept(TestIntegerWrap<int>::neg_eq(INT_MIN)); // special case
#endif
}

/*************************************************************************
// some doctest test cases
**************************************************************************/

TEST_CASE("upx_is_constant_evaluated") {
#if defined(upx_is_constant_evaluated)
    CHECK(true);
#endif
}

TEST_CASE("assert_noexcept") {
    // just to make sure that our own assert() macros do not trigger any compiler warnings
    byte dummy = 0;
    byte *ptr1 = &dummy;
    const byte *const ptr2 = &dummy;
    void *ptr3 = nullptr;
    assert(true);
    assert(1);
    assert(ptr1);
    assert(ptr2);
    assert(!ptr3);
    assert_noexcept(true);
    assert_noexcept(1);
    assert_noexcept(ptr1);
    assert_noexcept(ptr2);
    assert_noexcept(!ptr3);
}

TEST_CASE("acc_vget") {
    CHECK_EQ(acc_vget_int(0, 0), 0);
    CHECK_EQ(acc_vget_long(1, -1), 1);
    CHECK_EQ(acc_vget_acc_int64l_t(2, 1), 2);
    CHECK_EQ(acc_vget_acc_hvoid_p(nullptr, 0), nullptr);
    if (acc_vget_int(1, 0) > 0)
        return;
    assert_noexcept(false);
}

TEST_CASE("ptr_invalidate_and_poison") {
    int *ip = nullptr; // initialized
    ptr_invalidate_and_poison(ip);
    assert(ip != nullptr);
    (void) ip;
    double *dp; // not initialized
    ptr_invalidate_and_poison(dp);
    assert(dp != nullptr);
    (void) dp;
}

TEST_CASE("upx_getenv") {
    CHECK_EQ(upx_getenv(nullptr), nullptr);
    CHECK_EQ(upx_getenv(""), nullptr);
}

TEST_CASE("working -fno-strict-aliasing") {
    bool ok;
    long v = 0;
    short *ps = ACC_STATIC_CAST(short *, acc_vget_acc_hvoid_p(&v, 0));
    int *pi = ACC_STATIC_CAST(int *, acc_vget_acc_hvoid_p(&v, 0));
    long *pl = ACC_STATIC_CAST(long *, acc_vget_acc_hvoid_p(&v, 0));
    *ps = 0;
    *pl = -1;
    ok = *ps == -1;
    CHECK(ok);
    *pi = 0;
    *pl = -1;
    ok = *pi == -1;
    CHECK(ok);
    *pl = 0;
    *ps = -1;
    ok = *pl != 0;
    CHECK(ok);
    *pl = 0;
    *pi = -1;
    ok = *pl != 0;
    CHECK(ok);
    UNUSED(ok);
}

TEST_CASE("working -fno-strict-overflow") {
    CHECK_EQ(acc_vget_int(INT_MAX, 0) + 1, INT_MIN);
    CHECK_EQ(acc_vget_int(INT_MIN, 0) - 1, INT_MAX);
    CHECK_EQ(acc_vget_long(LONG_MAX, 0) + 1, LONG_MIN);
    CHECK_EQ(acc_vget_long(LONG_MIN, 0) - 1, LONG_MAX);
    bool ok;
    int i;
    i = INT_MAX;
    i += 1;
    ok = i == INT_MIN;
    CHECK(ok);
    i = INT_MIN;
    i -= 1;
    ok = i == INT_MAX;
    CHECK(ok);
    UNUSED(ok);
}

TEST_CASE("libc snprintf") {
    // runtime check that Windows/MinGW <stdio.h> works as expected
    char buf[64];
    long long ll = acc_vget_int(-1, 0);
    unsigned long long llu = (upx_uint64_t) (upx_int64_t) ll;
    snprintf(buf, sizeof(buf), "%d.%ld.%lld.%u.%lu.%llu", -3, -2L, ll, 3U, 2LU, llu);
    CHECK_EQ(strcmp(buf, "-3.-2.-1.3.2.18446744073709551615"), 0);
    intmax_t im = ll;
    uintmax_t um = llu;
    snprintf(buf, sizeof(buf), "%d.%d.%d.%d.%d.%d.%d.%d.%d.%jd", -4, 0, 0, 0, 0, 0, 0, 0, 4, im);
    CHECK_EQ(strcmp(buf, "-4.0.0.0.0.0.0.0.4.-1"), 0);
    snprintf(buf, sizeof(buf), "%d.%d.%d.%d.%d.%d.%d.%d.%d.%ju", -5, 0, 0, 0, 0, 0, 0, 0, 5, um);
    CHECK_EQ(strcmp(buf, "-5.0.0.0.0.0.0.0.5.18446744073709551615"), 0);
    snprintf(buf, sizeof(buf), "%d.%d.%d.%d.%d.%d.%d.%d.%d.%jx", -6, 0, 0, 0, 0, 0, 0, 0, 6, um);
    CHECK_EQ(strcmp(buf, "-6.0.0.0.0.0.0.0.6.ffffffffffffffff"), 0);
    snprintf(buf, sizeof(buf), "%d.%d.%d.%d.%d.%d.%d.%d.%d.%#jx", -7, 0, 0, 0, 0, 0, 0, 0, 7, um);
    CHECK_EQ(strcmp(buf, "-7.0.0.0.0.0.0.0.7.0xffffffffffffffff"), 0);
    snprintf(buf, sizeof(buf), "%#X %#lx %#llx", 26u, 27ul, 28ull);
    CHECK_EQ(strcmp(buf, "0X1A 0x1b 0x1c"), 0);
    snprintf(buf, sizeof(buf), "%#06x %#06lX %#06llx", 26u, 27ul, 28ull);
    CHECK_EQ(strcmp(buf, "0x001a 0X001B 0x001c"), 0);
    snprintf(buf, sizeof(buf), "%#6x %#6lx %#6llX", 26u, 27ul, 28ull);
    CHECK_EQ(strcmp(buf, "  0x1a   0x1b   0X1C"), 0);
    snprintf(buf, sizeof(buf), "%#-6X %#-6lx %#-6llx", 26u, 27ul, 28ull);
    CHECK_EQ(strcmp(buf, "0X1A   0x1b   0x1c  "), 0);
}

TEST_CASE("libc qsort") {
    // runtime check that libc qsort() never compares identical objects
    // UPDATE: while only poor implementations of qsort() would actually do this
    //   it is probably allowed by the standard, so skip this test by default
    if (!is_envvar_true("UPX_DEBUG_TEST_LIBC_QSORT"))
        return;

    struct Elem final {
        upx_uint16_t id;
        upx_uint16_t value;
        static int __acc_cdecl_qsort compare(const void *aa, const void *bb) noexcept {
            const Elem *a = (const Elem *) aa;
            const Elem *b = (const Elem *) bb;
            assert_noexcept(a->id != b->id); // check not IDENTICAL
            return a->value == b->value ? 0 : (a->value < b->value ? -1 : 1);
        }
        static noinline bool check_sort(upx_sort_func_t sort, Elem *e, size_t n, bool is_stable) {
            upx_uint32_t x = 5381 + (upx_rand() & 255);
            for (size_t i = 0; i < n; i++) {
                e[i].id = (upx_uint16_t) i;
                x = x * 33 + 1 + (i & 255);
                e[i].value = (upx_uint16_t) ((x >> 4) & 15);
            }
            sort(e, n, sizeof(Elem), Elem::compare);
            // verify
            for (size_t i = 1; i < n; i++) {
                if very_unlikely (e[i - 1].value > e[i].value)
                    return false;
                if (is_stable)
                    if very_unlikely (e[i - 1].value == e[i].value && e[i - 1].id >= e[i].id)
                        return false;
            }
            return true;
        }
    };

    constexpr size_t N = 256;
    Elem e[N];
    for (size_t n = 0; n <= N; n = 2 * n + 1) {
        // system sort functions
        CHECK(Elem::check_sort(::qsort, e, n, false)); // libc qsort()
#if UPX_CONFIG_USE_STABLE_SORT
        upx_sort_func_t wrap_stable_sort = [](void *aa, size_t nn, size_t, upx_compare_func_t cc) {
            upx_std_stable_sort<sizeof(Elem)>(aa, nn, cc);
        };
        CHECK(Elem::check_sort(wrap_stable_sort, e, n, true)); // std::stable_sort()
#endif
        // simple UPX sort functions
        CHECK(Elem::check_sort(upx_gnomesort, e, n, true));
        CHECK(Elem::check_sort(upx_shellsort_memswap, e, n, false));
        CHECK(Elem::check_sort(upx_shellsort_memcpy, e, n, false));
    }
}

/* vim:set ts=4 sw=4 et: */
