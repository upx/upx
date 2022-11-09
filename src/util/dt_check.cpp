/* dt_check.cpp -- doctest check

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2022 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2022 Laszlo Molnar
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

#include "../conf.h"

/*************************************************************************
// upx_doctest_check()
**************************************************************************/

int upx_doctest_check(int argc, char **argv) {
#if defined(DOCTEST_CONFIG_DISABLE)
    UNUSED(argc);
    UNUSED(argv);
    return 0;
#else
    const char *e = getenv("UPX_DEBUG_DOCTEST_DISABLE");
    if (e && e[0] && strcmp(e, "0") != 0)
        return 0;
    bool minimal = true;   // only show failing tests
    bool duration = false; // show timings
    bool success = false;  // show all tests
#if DEBUG
    minimal = false;
#endif
    e = getenv("UPX_DEBUG_DOCTEST_VERBOSE");
    if (e && e[0]) {
        minimal = false;
        if (strcmp(e, "0") == 0) {
            minimal = true;
        } else if (strcmp(e, "2") == 0) {
            duration = true;
        } else if (strcmp(e, "3") == 0) {
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

ACC_COMPILE_TIME_ASSERT_HEADER((std::is_same<short, upx_int16_t>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((std::is_same<unsigned short, upx_uint16_t>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((std::is_same<int, upx_int32_t>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((std::is_same<unsigned, upx_uint32_t>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((std::is_same<long long, upx_int64_t>::value))
ACC_COMPILE_TIME_ASSERT_HEADER((std::is_same<unsigned long long, upx_uint64_t>::value))

ACC_COMPILE_TIME_ASSERT_HEADER(no_bswap16(0x04030201) == 0x0201)
ACC_COMPILE_TIME_ASSERT_HEADER(no_bswap32(0x04030201) == 0x04030201)
ACC_COMPILE_TIME_ASSERT_HEADER(no_bswap64(0x0807060504030201ull) == 0x0807060504030201ull)
#if !(ACC_CC_MSC)
ACC_COMPILE_TIME_ASSERT_HEADER(bswap16(0x04030201) == 0x0102)
ACC_COMPILE_TIME_ASSERT_HEADER(bswap32(0x04030201) == 0x01020304)
ACC_COMPILE_TIME_ASSERT_HEADER(bswap64(0x0807060504030201ull) == 0x0102030405060708ull)
#endif

ACC_COMPILE_TIME_ASSERT_HEADER(compile_time::string_len("") == 0)
ACC_COMPILE_TIME_ASSERT_HEADER(compile_time::string_len("a") == 1)

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
// upx_compiler_sanity_check()
// assert a sane architecture and compiler
**************************************************************************/

namespace {

template <class T>
struct TestBELE {
    __acc_static_noinline bool test(void) {
        COMPILE_TIME_ASSERT_ALIGNED1(T)
        struct alignas(1) test1_t {
            char a;
            T b;
        };
        struct alignas(1) test2_t {
            char a;
            T b[3];
        };
        test1_t t1[7];
        UNUSED(t1);
        test2_t t2[7];
        UNUSED(t2);
        COMPILE_TIME_ASSERT(sizeof(test1_t) == 1 + sizeof(T))
        COMPILE_TIME_ASSERT_ALIGNED1(test1_t)
        COMPILE_TIME_ASSERT(sizeof(t1) == 7 + 7 * sizeof(T))
        COMPILE_TIME_ASSERT(sizeof(test2_t) == 1 + 3 * sizeof(T))
        COMPILE_TIME_ASSERT_ALIGNED1(test2_t)
        COMPILE_TIME_ASSERT(sizeof(t2) == 7 + 21 * sizeof(T))
#if 1
        T allbits;
        allbits = 0;
        allbits += 1;
        allbits -= 2;
        T v1;
        v1 = 1;
        v1 *= 2;
        v1 -= 1;
        T v2;
        v2 = 1;
        assert((v1 == v2));
        assert(!(v1 != v2));
        assert((v1 <= v2));
        assert((v1 >= v2));
        assert(!(v1 < v2));
        assert(!(v1 > v2));
        v2 ^= allbits;
        assert(!(v1 == v2));
        assert((v1 != v2));
        assert((v1 <= v2));
        assert(!(v1 >= v2));
        assert((v1 < v2));
        assert(!(v1 > v2));
        v2 += 2;
        assert(v1 == 1);
        assert(v2 == 0);
        v1 <<= 1;
        v1 |= v2;
        v1 >>= 1;
        v2 &= v1;
        v2 /= v1;
        v2 *= v1;
        assert(v1 == 1);
        assert(v2 == 0);
        if ((v1 ^ v2) != 1)
            return false;
#endif
        return true;
    }
};

template <class A, class B>
struct TestNoAliasingStruct {
    __acc_static_noinline bool test(A *a, B *b) {
        *a = 0;
        *b = 0;
        *b -= 3;
        return *a != 0;
    }
};
template <class A, class B>
__acc_static_forceinline bool testNoAliasing(A *a, B *b) {
    return TestNoAliasingStruct<A, B>::test(a, b);
}
template <class T>
struct TestIntegerWrap {
    static inline bool inc(T x) { return x + 1 > x; }
    static inline bool dec(T x) { return x - 1 < x; }
};

} // namespace

#define ACC_WANT_ACC_CHK_CH 1
#undef ACCCHK_ASSERT
#include "../miniacc.h"

void upx_compiler_sanity_check(void) {
#define ACC_WANT_ACC_CHK_CH 1
#undef ACCCHK_ASSERT
#define ACCCHK_ASSERT(expr) ACC_COMPILE_TIME_ASSERT(expr)
#include "../miniacc.h"
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

    COMPILE_TIME_ASSERT(sizeof(UPX_VERSION_STRING4) == 4 + 1)
    assert(strlen(UPX_VERSION_STRING4) == 4);
    COMPILE_TIME_ASSERT(sizeof(UPX_VERSION_YEAR) == 4 + 1)
    assert(strlen(UPX_VERSION_YEAR) == 4);
    assert(memcmp(UPX_VERSION_DATE_ISO, UPX_VERSION_YEAR, 4) == 0);
    assert(memcmp(&UPX_VERSION_DATE[sizeof(UPX_VERSION_DATE) - 1 - 4], UPX_VERSION_YEAR, 4) == 0);
    if (gitrev[0]) {
        size_t revlen = strlen(gitrev);
        if (strncmp(gitrev, "ERROR", 5) == 0) {
            assert(revlen == 5 || revlen == 6);
        } else {
            assert(revlen == 12 || revlen == 13);
        }
        if (revlen == 6 || revlen == 13) {
            assert(gitrev[revlen - 1] == '+');
        }
    }
    assert(UPX_RSIZE_MAX_MEM == 805306368);

#if 1
    assert(TestBELE<LE16>::test());
    assert(TestBELE<LE32>::test());
    assert(TestBELE<LE64>::test());
    assert(TestBELE<BE16>::test());
    assert(TestBELE<BE32>::test());
    assert(TestBELE<BE64>::test());
    {
        alignas(16) static const unsigned char dd[32] = {
            0, 0, 0, 0,    0,    0,    0,    0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8, 0,
            0, 0, 0, 0x7f, 0x7e, 0x7d, 0x7c, 0x7b, 0x7a, 0x79, 0x78, 0,    0,    0,    0,    0};
        const unsigned char *d;
        const N_BELE_RTP::AbstractPolicy *bele;
        d = dd + 7;
        assert(upx_adler32(d, 4) == 0x09f003f7);
        assert(upx_adler32(d, 4, 0) == 0x09ec03f6);
        assert(upx_adler32(d, 4, 1) == 0x09f003f7);
        bele = &N_BELE_RTP::be_policy;
        assert(get_be16(d) == 0xfffe);
        assert(bele->get16(d) == 0xfffe);
        assert(get_be16_signed(d) == -2);
        assert(get_be24(d) == 0xfffefd);
        assert(bele->get24(d) == 0xfffefd);
        assert(get_be24_signed(d) == -259);
        assert(get_be32(d) == 0xfffefdfc);
        assert(bele->get32(d) == 0xfffefdfc);
        assert(get_be32_signed(d) == -66052);
        bele = &N_BELE_RTP::le_policy;
        assert(get_le16(d) == 0xfeff);
        assert(bele->get16(d) == 0xfeff);
        assert(get_le16_signed(d) == -257);
        assert(get_le24(d) == 0xfdfeff);
        assert(bele->get24(d) == 0xfdfeff);
        assert(get_le24_signed(d) == -131329);
        assert(get_le32(d) == 0xfcfdfeff);
        assert(bele->get32(d) == 0xfcfdfeff);
        assert(get_le32_signed(d) == -50462977);
        assert(get_le64_signed(d) == -506097522914230529LL);
        assert(find_be16(d, 2, 0xfffe) == 0);
        assert(find_le16(d, 2, 0xfeff) == 0);
        assert(find_be32(d, 4, 0xfffefdfc) == 0);
        assert(find_le32(d, 4, 0xfcfdfeff) == 0);
        d += 12;
        assert(get_be16_signed(d) == 32638);
        assert(get_be24_signed(d) == 8355453);
        assert(get_be32_signed(d) == 2138996092);
        assert(get_be64_signed(d) == 9186918263483431288LL);
    }
    {
        unsigned dd;
        void *const d = &dd;
        dd = ne32_to_le32(0xf7f6f5f4);
        assert(get_le26(d) == 0x03f6f5f4);
        set_le26(d, 0);
        assert(get_le26(d) == 0);
        assert(dd == ne32_to_le32(0xf4000000));
        set_le26(d, 0xff020304);
        assert(get_le26(d) == 0x03020304);
        assert(dd == ne32_to_le32(0xf7020304));
    }
#endif
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
    assert(testNoAliasing(&u.v_short, &u.b32));
    assert(testNoAliasing(&u.v_short, &u.l32));
    assert(testNoAliasing(&u.v_int, &u.b64));
    assert(testNoAliasing(&u.v_int, &u.l64));
    // check working -fno-strict-aliasing
    assert(testNoAliasing(&u.v_short, &u.v_int));
    assert(testNoAliasing(&u.v_int, &u.v_long));
    assert(testNoAliasing(&u.v_int, &u.v_llong));
    assert(testNoAliasing(&u.v_long, &u.v_llong));

    assert(TestIntegerWrap<unsigned>::inc(0));
    assert(!TestIntegerWrap<unsigned>::inc(UINT_MAX));
    assert(TestIntegerWrap<unsigned>::dec(1));
    assert(!TestIntegerWrap<unsigned>::dec(0));
    // check working -fno-strict-overflow
    assert(TestIntegerWrap<int>::inc(0));
    assert(!TestIntegerWrap<int>::inc(INT_MAX));
    assert(TestIntegerWrap<int>::dec(0));
    assert(!TestIntegerWrap<int>::dec(INT_MIN));
}

/*************************************************************************
// some doctest test cases
**************************************************************************/

TEST_CASE("acc_vget") {
    CHECK_EQ(acc_vget_int(0, 0), 0);
    CHECK_EQ(acc_vget_long(1, -1), 1);
    CHECK_EQ(acc_vget_acc_int64l_t(2, 1), 2);
    CHECK_EQ(acc_vget_acc_hvoid_p(nullptr, 0), nullptr);
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
}

TEST_CASE("libc snprintf") {
    // runtime check that Win32/MinGW <stdio.h> works as expected
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
}

/* vim:set ts=4 sw=4 et: */
