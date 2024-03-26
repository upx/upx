/* dt_check.cpp -- doctest check

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2024 Markus Franz Xaver Johannes Oberhumer
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
    const char *e = getenv("UPX_DEBUG_DOCTEST_VERBOSE");
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
    // this requires that main_get_options() understands "--dt-XXX" options
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
// compile-time checks
**************************************************************************/

// need extra parenthesis because the C preprocessor does not understand C++ templates
ACC_COMPILE_TIME_ASSERT_HEADER((std::is_same<short, upx_int16_t>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((std::is_same<unsigned short, upx_uint16_t>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((std::is_same<int, upx_int32_t>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((std::is_same<unsigned, upx_uint32_t>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((std::is_same<long long, upx_int64_t>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((std::is_same<unsigned long long, upx_uint64_t>::value))

ACC_COMPILE_TIME_ASSERT_HEADER(no_bswap16(0x04030201) == 0x0201)
ACC_COMPILE_TIME_ASSERT_HEADER(no_bswap32(0x04030201) == 0x04030201)
ACC_COMPILE_TIME_ASSERT_HEADER(no_bswap64(0x0807060504030201ull) == 0x0807060504030201ull)
#if !(ACC_CC_MSC) // unfortunately *not* constexpr with current MSVC
ACC_COMPILE_TIME_ASSERT_HEADER(bswap16(0x04030201) == 0x0102)
ACC_COMPILE_TIME_ASSERT_HEADER(bswap32(0x04030201) == 0x01020304)
ACC_COMPILE_TIME_ASSERT_HEADER(bswap64(0x0807060504030201ull) == 0x0102030405060708ull)
ACC_COMPILE_TIME_ASSERT_HEADER(bswap16(bswap16(0xf4f3f2f1)) == no_bswap16(0xf4f3f2f1))
ACC_COMPILE_TIME_ASSERT_HEADER(bswap32(bswap32(0xf4f3f2f1)) == no_bswap32(0xf4f3f2f1))
ACC_COMPILE_TIME_ASSERT_HEADER(bswap64(bswap64(0xf8f7f6f5f4f3f2f1ull)) ==
                               no_bswap64(0xf8f7f6f5f4f3f2f1ull))
#endif

ACC_COMPILE_TIME_ASSERT_HEADER(sign_extend(0u + 0, 8) == 0)
ACC_COMPILE_TIME_ASSERT_HEADER(sign_extend(0u + 1, 8) == 1)
ACC_COMPILE_TIME_ASSERT_HEADER(sign_extend(0u + 127, 8) == 127)
ACC_COMPILE_TIME_ASSERT_HEADER(sign_extend(0u + 128, 8) == -128)
ACC_COMPILE_TIME_ASSERT_HEADER(sign_extend(0u - 1, 8) == -1)
ACC_COMPILE_TIME_ASSERT_HEADER(sign_extend(0u + 256, 8) == 0)
ACC_COMPILE_TIME_ASSERT_HEADER(sign_extend(0u + 257, 8) == 1)
ACC_COMPILE_TIME_ASSERT_HEADER(sign_extend(0u + 383, 8) == 127)
ACC_COMPILE_TIME_ASSERT_HEADER(sign_extend(0u + 384, 8) == -128)
ACC_COMPILE_TIME_ASSERT_HEADER(sign_extend(0u + 511, 8) == -1)
ACC_COMPILE_TIME_ASSERT_HEADER(sign_extend(0ull + 0, 1) == 0)
ACC_COMPILE_TIME_ASSERT_HEADER(sign_extend(0ull + 1, 1) == -1)

ACC_COMPILE_TIME_ASSERT_HEADER(CHAR_BIT == 8)
#if 0 // does not work with MSVC
#if '\0' - 1 < 0
ACC_COMPILE_TIME_ASSERT_HEADER(CHAR_MAX == 127)
#else
ACC_COMPILE_TIME_ASSERT_HEADER(CHAR_MAX == 255)
#endif
#if L'\0' - 1 < 0
ACC_COMPILE_TIME_ASSERT_HEADER((wchar_t) -1 < 0)
#else
ACC_COMPILE_TIME_ASSERT_HEADER((wchar_t) -1 > 0)
#endif
#endif

/*************************************************************************
// upx_compiler_sanity_check()
// assert a sane architecture and compiler
// (modern compilers will optimize away most of this code)
**************************************************************************/

namespace {

template <class T>
struct CheckIntegral {
    struct TestT {
        T a;
        T x[2];
    };
    template <class U>
    struct TestU {
        U a = {};
        const U b = {};
        static constexpr U c = {};
        U x[2] = {};
        const U y[2] = {};
        static constexpr U z[2] = {};
    };
    template <class U>
    static void checkU(void) noexcept {
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
#if __cplusplus <= 201703L
        COMPILE_TIME_ASSERT(std::is_pod<U>::value) // std::is_pod is deprecated in C++20
#endif
        COMPILE_TIME_ASSERT(std::is_standard_layout<U>::value)
        COMPILE_TIME_ASSERT(std::is_trivial<U>::value)
        // more checks, these are probably implied by std::is_trivial
        COMPILE_TIME_ASSERT(std::is_nothrow_default_constructible<U>::value)
        COMPILE_TIME_ASSERT(std::is_nothrow_destructible<U>::value)
        COMPILE_TIME_ASSERT(std::is_trivially_copyable<U>::value)
        COMPILE_TIME_ASSERT(std::is_trivially_default_constructible<U>::value)
        // UPX extras
        COMPILE_TIME_ASSERT(upx_is_integral<U>::value)
        COMPILE_TIME_ASSERT(upx_is_integral_v<U>)
    }
    static void check(void) noexcept {
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
        checkU<T>();
        checkU<typename std::add_const<T>::type>();
    }
};
template <class T>
struct CheckAlignment {
    static void check(void) noexcept {
        COMPILE_TIME_ASSERT_ALIGNED1(T)
        struct alignas(1) Test1 {
            char a;
            T b;
        };
        struct alignas(1) Test2 {
            char a;
            T b[3];
        };
        COMPILE_TIME_ASSERT_ALIGNED1(Test1)
        COMPILE_TIME_ASSERT_ALIGNED1(Test2)
        Test1 t1[7];
        Test2 t2[7];
        COMPILE_TIME_ASSERT(sizeof(Test1) == 1 + sizeof(T))
        COMPILE_TIME_ASSERT(sizeof(t1) == 7 + 7 * sizeof(T))
        COMPILE_TIME_ASSERT(sizeof(Test2) == 1 + 3 * sizeof(T))
        COMPILE_TIME_ASSERT(sizeof(t2) == 7 + 21 * sizeof(T))
        UNUSED(t1);
        UNUSED(t2);
    }
};
template <class T>
struct TestBELE {
    static noinline bool test(void) noexcept {
        CheckIntegral<T>::check();
        CheckAlignment<T>::check();
        // arithmetic checks
        T allbits = {};
        assert_noexcept(allbits == 0);
        allbits += 1;
        allbits -= 2;
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
        v2 ^= allbits;
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
        return true;
    }
};

template <class A, class B>
struct TestNoAliasingStruct {
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
struct TestIntegerWrap {
    static inline bool inc_gt(const T x) noexcept { return x + 1 > x; }
    static inline bool dec_lt(const T x) noexcept { return x - 1 < x; }
    static inline bool neg_eq(const T x) noexcept { return T(0) - x == x; }
};

//
// basic exception handling checks to early catch toolchain/qemu/wine/etc problems
//

static noinline void throwSomeValue(int x) may_throw {
    if (x < 0)
        throw int(x);
    else
        throw size_t(x);
}

static noinline void check_basic_cxx_exception_handling(void (*func)(int)) noexcept {
    bool cxx_exception_handling_works = false;
    try {
        func(42);
    } catch (const size_t &e) {
        if (e == 42)
            cxx_exception_handling_works = true;
    } catch (...) {
    }
    assert_noexcept(cxx_exception_handling_works);
}

//
// basic floating point checks to early catch toolchain/qemu/wine/etc problems
//

static noinline float i64_f32_add_div(upx_int64_t a, upx_int64_t b) { return (a + b) / 1000000.0f; }
static noinline float u64_f32_add_div(upx_uint64_t a, upx_uint64_t b) {
    return (a + b) / 1000000.0f;
}
static noinline float i64_f32_sub_div(upx_int64_t a, upx_int64_t b) { return (a - b) / 1000000.0f; }
static noinline float u64_f32_sub_div(upx_uint64_t a, upx_uint64_t b) {
    return (a - b) / 1000000.0f;
}

static noinline double i64_f64_add_div(upx_int64_t a, upx_int64_t b) { return (a + b) / 1000000.0; }
static noinline double u64_f64_add_div(upx_uint64_t a, upx_uint64_t b) {
    return (a + b) / 1000000.0;
}
static noinline double i64_f64_sub_div(upx_int64_t a, upx_int64_t b) { return (a - b) / 1000000.0; }
static noinline double u64_f64_sub_div(upx_uint64_t a, upx_uint64_t b) {
    return (a - b) / 1000000.0;
}

template <class Int, class Float>
struct TestFloat {
    static constexpr Int X = 1000000;
    static noinline Float div(Int a, Float f) { return a / f; }
    static noinline Float add_div(Int a, Int b, Float f) { return Float(a + b) / f; }
    static noinline Float sub_div(Int a, Int b, Float f) { return Float(a - b) / f; }
    static noinline Float add_div_x(Int a, Int b) { return Float(a + b) / Float(X); }
    static noinline Float sub_div_x(Int a, Int b) { return Float(a - b) / Float(X); }
    static noinline void check() noexcept {
        assert_noexcept(add_div(X, X, Float(X)) == Float(2));
        assert_noexcept(add_div_x(X, X) == Float(2));
        assert_noexcept(sub_div(3 * X, X, Float(X)) == Float(2));
        assert_noexcept(sub_div_x(3 * X, X) == Float(2));
        // extra debugging; floating point edge cases cause portability problems in practice
        if (is_envvar_true("UPX_DEBUG_TEST_FLOAT_DIVISION_BY_ZERO")) {
#if defined(__clang__) && defined(__FAST_MATH__) && defined(__INTEL_LLVM_COMPILER)
            // warning: comparison with NaN always evaluates to false in fast floating point modes
            fprintf(stderr, "upx: WARNING: ignoring UPX_DEBUG_TEST_FLOAT_DIVISION_BY_ZERO\n");
#elif defined(__clang__) && (__clang_major__ < 9) && defined(__SANITIZE_UNDEFINED_BEHAVIOR__)
            // @COMPILER_BUG @CLANG_BUG
            fprintf(stderr, "upx: WARNING: ignoring UPX_DEBUG_TEST_FLOAT_DIVISION_BY_ZERO\n");
#else
            assert_noexcept(std::isnan(div(0, Float(0))));
            assert_noexcept(std::isinf(div(1, Float(0))));
#endif
        }
    }
};

static noinline void check_basic_floating_point(void) noexcept {
    assert_noexcept(i64_f32_add_div(1000000, 1000000) == 2.0f);
    assert_noexcept(u64_f32_add_div(1000000, 1000000) == 2.0f);
    assert_noexcept(i64_f32_sub_div(3000000, 1000000) == 2.0f);
    assert_noexcept(u64_f32_sub_div(3000000, 1000000) == 2.0f);
    assert_noexcept(i64_f64_add_div(1000000, 1000000) == 2.0);
    assert_noexcept(u64_f64_add_div(1000000, 1000000) == 2.0);
    assert_noexcept(i64_f64_sub_div(3000000, 1000000) == 2.0);
    assert_noexcept(u64_f64_sub_div(3000000, 1000000) == 2.0);
    TestFloat<upx_int32_t, float>::check();
    TestFloat<upx_uint32_t, float>::check();
    TestFloat<upx_int64_t, float>::check();
    TestFloat<upx_uint64_t, float>::check();
    TestFloat<upx_int32_t, double>::check();
    TestFloat<upx_uint32_t, double>::check();
    TestFloat<upx_int64_t, double>::check();
    TestFloat<upx_uint64_t, double>::check();
}

} // namespace

#define ACC_WANT_ACC_CHK_CH 1
#undef ACCCHK_ASSERT
#include "../util/miniacc.h"

void upx_compiler_sanity_check(void) noexcept {
    if (is_envvar_true("UPX_DEBUG_DOCTEST_DISABLE", "UPX_DEBUG_DISABLE_DOCTEST")) {
        // If UPX_DEBUG_DOCTEST_DISABLE is set then we don't want to throw any
        // exceptions in order to improve debugging experience.
    } else {
        // check working C++ exception handling to early catch toolchain/qemu/wine/etc problems
        check_basic_cxx_exception_handling(throwSomeValue);
    }

    check_basic_floating_point();

#define ACC_WANT_ACC_CHK_CH 1
#undef ACCCHK_ASSERT
#define ACCCHK_ASSERT(expr) ACC_COMPILE_TIME_ASSERT(expr)
#include "../util/miniacc.h"
#undef ACCCHK_ASSERT

    COMPILE_TIME_ASSERT(sizeof(char) == 1)
    COMPILE_TIME_ASSERT(sizeof(short) == 2)
    COMPILE_TIME_ASSERT(sizeof(int) == 4)
    COMPILE_TIME_ASSERT(sizeof(long) >= 4)
    COMPILE_TIME_ASSERT(sizeof(void *) >= 4)

    COMPILE_TIME_ASSERT(sizeof(BE16) == 2)
    COMPILE_TIME_ASSERT(sizeof(BE32) == 4)
    COMPILE_TIME_ASSERT(sizeof(BE64) == 8)
    COMPILE_TIME_ASSERT(sizeof(LE16) == 2)
    COMPILE_TIME_ASSERT(sizeof(LE32) == 4)
    COMPILE_TIME_ASSERT(sizeof(LE64) == 8)

    COMPILE_TIME_ASSERT_ALIGNED1(BE16)
    COMPILE_TIME_ASSERT_ALIGNED1(BE32)
    COMPILE_TIME_ASSERT_ALIGNED1(BE64)
    COMPILE_TIME_ASSERT_ALIGNED1(LE16)
    COMPILE_TIME_ASSERT_ALIGNED1(LE32)
    COMPILE_TIME_ASSERT_ALIGNED1(LE64)

    CheckIntegral<char>::check();
    CheckIntegral<signed char>::check();
    CheckIntegral<unsigned char>::check();
    CheckIntegral<short>::check();
    CheckIntegral<int>::check();
    CheckIntegral<long>::check();
    CheckIntegral<long long>::check();
    CheckIntegral<ptrdiff_t>::check();
    CheckIntegral<size_t>::check();
    CheckIntegral<upx_off_t>::check();
    CheckIntegral<upx_uintptr_t>::check();

    COMPILE_TIME_ASSERT(sizeof(upx_charptr_unit_type) == 1)
    COMPILE_TIME_ASSERT_ALIGNED1(upx_charptr_unit_type)
    COMPILE_TIME_ASSERT(sizeof(*((charptr) nullptr)) == 1)

    COMPILE_TIME_ASSERT(sizeof(UPX_VERSION_STRING4) == 4 + 1)
    assert_noexcept(strlen(UPX_VERSION_STRING4) == 4);
    COMPILE_TIME_ASSERT(sizeof(UPX_VERSION_YEAR) == 4 + 1)
    assert_noexcept(strlen(UPX_VERSION_YEAR) == 4);
    assert_noexcept(memcmp(UPX_VERSION_DATE_ISO, UPX_VERSION_YEAR, 4) == 0);
    assert_noexcept(
        memcmp(&UPX_VERSION_DATE[sizeof(UPX_VERSION_DATE) - 1 - 4], UPX_VERSION_YEAR, 4) == 0);
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
        alignas(16) static const byte dd[32] = {
            0, 0, 0, 0,    0,    0,    0,    0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8, 0,
            0, 0, 0, 0x7f, 0x7e, 0x7d, 0x7c, 0x7b, 0x7a, 0x79, 0x78, 0,    0,    0,    0,    0};
        const byte *d;
        const N_BELE_RTP::AbstractPolicy *bele;
        d = dd + 7;
        assert_noexcept(upx_adler32(d, 4) == 0x09f003f7);
        assert_noexcept(upx_adler32(d, 4, 0) == 0x09ec03f6);
        assert_noexcept(upx_adler32(d, 4, 1) == 0x09f003f7);
        bele = &N_BELE_RTP::be_policy;
        assert_noexcept(get_be16(d) == 0xfffe);
        assert_noexcept(bele->get16(d) == 0xfffe);
        assert_noexcept(get_be16_signed(d) == -2);
        assert_noexcept(get_be24(d) == 0xfffefd);
        assert_noexcept(bele->get24(d) == 0xfffefd);
        assert_noexcept(get_be24_signed(d) == -259);
        assert_noexcept(get_be32(d) == 0xfffefdfc);
        assert_noexcept(bele->get32(d) == 0xfffefdfc);
        assert_noexcept(get_be32_signed(d) == -66052);
        bele = &N_BELE_RTP::le_policy;
        assert_noexcept(get_le16(d) == 0xfeff);
        assert_noexcept(bele->get16(d) == 0xfeff);
        assert_noexcept(get_le16_signed(d) == -257);
        assert_noexcept(get_le24(d) == 0xfdfeff);
        assert_noexcept(bele->get24(d) == 0xfdfeff);
        assert_noexcept(get_le24_signed(d) == -131329);
        assert_noexcept(get_le32(d) == 0xfcfdfeff);
        assert_noexcept(bele->get32(d) == 0xfcfdfeff);
        assert_noexcept(get_le32_signed(d) == -50462977);
        assert_noexcept(get_le64_signed(d) == -506097522914230529LL);
        assert_noexcept(find_be16(d, 2, 0xfffe) == 0);
        assert_noexcept(find_le16(d, 2, 0xfeff) == 0);
        assert_noexcept(find_be32(d, 4, 0xfffefdfc) == 0);
        assert_noexcept(find_le32(d, 4, 0xfcfdfeff) == 0);
        d += 12;
        assert_noexcept(get_be16_signed(d) == 32638);
        assert_noexcept(get_be24_signed(d) == 8355453);
        assert_noexcept(get_be32_signed(d) == 2138996092);
        assert_noexcept(get_be64_signed(d) == 9186918263483431288LL);
    }
#if DEBUG >= 1
    {
        for (int i = 0; i < 256; i++) {
            {
                const unsigned u = i;
                assert_noexcept(sign_extend(u, 1) == ((i & 1) ? -1 : 0));
                assert_noexcept(sign_extend(u, 2) == ((i & 2) ? -2 + (i & 1) : (i & 1)));
                assert_noexcept(sign_extend(u, 3) == ((i & 4) ? -4 + (i & 3) : (i & 3)));
                assert_noexcept(sign_extend(u, 4) == ((i & 8) ? -8 + (i & 7) : (i & 7)));
                assert_noexcept(sign_extend(u, 5) == ((i & 16) ? -16 + (i & 15) : (i & 15)));
                assert_noexcept(sign_extend(u, 6) == ((i & 32) ? -32 + (i & 31) : (i & 31)));
                assert_noexcept(sign_extend(u, 7) == ((i & 64) ? -64 + (i & 63) : (i & 63)));
                assert_noexcept(sign_extend(u, 8) == ((i & 128) ? -128 + (i & 127) : (i & 127)));
                assert_noexcept(sign_extend(u, 32) == i);
                assert_noexcept(sign_extend(0u - u, 32) == -i);
            }
            {
                const upx_uint64_t u = i;
                assert_noexcept(sign_extend(u, 1) == ((i & 1) ? -1 : 0));
                assert_noexcept(sign_extend(u, 2) == ((i & 2) ? -2 + (i & 1) : (i & 1)));
                assert_noexcept(sign_extend(u, 3) == ((i & 4) ? -4 + (i & 3) : (i & 3)));
                assert_noexcept(sign_extend(u, 4) == ((i & 8) ? -8 + (i & 7) : (i & 7)));
                assert_noexcept(sign_extend(u, 5) == ((i & 16) ? -16 + (i & 15) : (i & 15)));
                assert_noexcept(sign_extend(u, 6) == ((i & 32) ? -32 + (i & 31) : (i & 31)));
                assert_noexcept(sign_extend(u, 7) == ((i & 64) ? -64 + (i & 63) : (i & 63)));
                assert_noexcept(sign_extend(u, 8) == ((i & 128) ? -128 + (i & 127) : (i & 127)));
                assert_noexcept(sign_extend(u, 64) == i);
                assert_noexcept(sign_extend(0ull - u, 64) == -i);
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
}

/*************************************************************************
// some doctest test cases
**************************************************************************/

TEST_CASE("assert_noexcept") {
    // just to make sure that our own assert() macros don't generate any warnings
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
}

TEST_CASE("ptr_invalidate_and_poison") {
    int *ip = nullptr;
    ptr_invalidate_and_poison(ip);
    assert(ip != nullptr);
    (void) ip;
    double *dp; // not initialized
    ptr_invalidate_and_poison(dp);
    assert(dp != nullptr);
    (void) dp;
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
    unsigned long long llu = (unsigned long long) ll;
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

#if 0
TEST_CASE("libc qsort") {
    // runtime check that libc qsort() never compares identical objects
    // UPDATE: while only poor implementations of qsort() would actually do
    //   this, it is probably allowed by the standard; so skip this test case
    struct Elem {
        upx_uint16_t id;
        upx_uint16_t value;
        static int __acc_cdecl_qsort compare(const void *aa, const void *bb) noexcept {
            const Elem *a = (const Elem *) aa;
            const Elem *b = (const Elem *) bb;
            assert_noexcept(a->id != b->id); // check not IDENTICAL
            return a->value < b->value ? -1 : (a->value == b->value ? 0 : 1);
        }
        static bool check_sort(upx_sort_func_t sort, Elem *e, size_t n) {
            upx_uint32_t x = 5381 + n + ((upx_uintptr_t) e & 0xff);
            for (size_t i = 0; i < n; i++) {
                e[i].id = (upx_uint16_t) i;
                x = x * 33 + 1 + (i & 255);
                e[i].value = (upx_uint16_t) x;
            }
            sort(e, n, sizeof(Elem), Elem::compare);
            for (size_t i = 1; i < n; i++)
                if very_unlikely (e[i - 1].value > e[i].value)
                    return false;
            return true;
        }
    };
    constexpr size_t N = 4096;
    Elem e[N];
    for (size_t n = 0; n <= N; n = 2 * n + 1) {
        // CHECK(Elem::check_sort(qsort, e, n)); // libc qsort()
        CHECK(Elem::check_sort(upx_gnomesort, e, n));
        CHECK(Elem::check_sort(upx_shellsort_memswap, e, n));
        CHECK(Elem::check_sort(upx_shellsort_memcpy, e, n));
#if UPX_CONFIG_USE_STABLE_SORT
        upx_sort_func_t wrap_stable_sort = [](void *aa, size_t nn, size_t, upx_compare_func_t cc) {
            upx_std_stable_sort<sizeof(Elem)>(aa, nn, cc);
        };
        CHECK(Elem::check_sort(wrap_stable_sort, e, n));
#endif
    }
}
#endif

/* vim:set ts=4 sw=4 et: */
