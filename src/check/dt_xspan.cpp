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

// lots of tests (and probably quite a number of redundant tests)

#include "../conf.h"

/*************************************************************************
// raw_bytes
**************************************************************************/

TEST_CASE("raw_bytes ptr") {
    upx_uint16_t *ptr = nullptr;
    CHECK_NOTHROW(raw_bytes(ptr, 0));
    CHECK_THROWS(raw_bytes(ptr, 1));
    CHECK_THROWS(raw_index_bytes(ptr, 0, 0));
    CHECK_THROWS(raw_index_bytes(ptr, 1, 0));
    CHECK_THROWS(raw_index_bytes(ptr, 0, 1));
    upx_uint16_t buf[4];
    ptr = buf;
    CHECK(ptr_udiff_bytes(raw_index_bytes(ptr, 1, 1), ptr) == 2u);
    CHECK(ptr_udiff_bytes(raw_index_bytes(ptr, 4, 0), ptr) == 8u);
    UNUSED(ptr);
}

TEST_CASE("raw_bytes bounded array") {
    upx_uint16_t buf[4];
    CHECK_NOTHROW(raw_bytes(buf, 8));
    CHECK_THROWS(raw_bytes(buf, 9));
    CHECK_NOTHROW(raw_index_bytes(buf, 4, 0));
    CHECK_THROWS(raw_index_bytes(buf, 4, 1));
    CHECK_NOTHROW(raw_index_bytes(buf, 3, 2));
    CHECK_THROWS(raw_index_bytes(buf, 3, 3));
    CHECK(ptr_udiff_bytes(raw_index_bytes(buf, 1, 1), buf) == 2u);
    CHECK(ptr_udiff_bytes(raw_index_bytes(buf, 4, 0), buf) == 8u);
    UNUSED(buf);
}

/*************************************************************************
// basic xspan
**************************************************************************/

TEST_CASE("basic xspan usage") {
    alignas(4) char buf[4] = {0, 1, 2, 3};

    SUBCASE("XSPAN_x") {
        XSPAN_0(char) a0 = nullptr;

        XSPAN_0(char) b0 = buf;
        XSPAN_P(char) bp = buf;

        XSPAN_0(char) c0 = XSPAN_0_MAKE(char, buf);
        XSPAN_P(char) cp = XSPAN_P_MAKE(char, buf);
        XSPAN_S(char) cs = XSPAN_S_MAKE(char, buf, sizeof(buf));

        XSPAN_0(const char) const x0 = XSPAN_0_MAKE(const char, buf);
        XSPAN_P(const char) const xp = XSPAN_P_MAKE(const char, buf);
        XSPAN_S(const char) const xs = XSPAN_S_MAKE(const char, buf, sizeof(buf));
        XSPAN_P(const char) const yp = xs;
        XSPAN_0(const char) const z0p = yp;
        XSPAN_0(const char) const z0s = xs;

        CHECK((a0 == nullptr));
        CHECK(c0 == b0);
        CHECK(cp == bp);
        CHECK(cs == bp);
        CHECK(x0 == z0p);
        CHECK(xp == z0s);

        CHECK_NOTHROW(raw_bytes(a0, 0));
        CHECK_THROWS(raw_bytes(a0, 1));
        CHECK_THROWS(raw_index_bytes(a0, 0, 0));

        CHECK(raw_bytes(b0, 0) == buf);
        CHECK(raw_bytes(bp, 0) == buf);
        // info: these will fail if we ever add an overload for bounded-arrays
#if WITH_XSPAN >= 2
        CHECK(b0.raw_size_in_bytes() == 0u);
        CHECK(bp.raw_size_in_bytes() == 0u);
#endif
        CHECK(raw_bytes(b0, 999999) == buf);
        CHECK(raw_bytes(bp, 999999) == buf);

        CHECK(raw_bytes(c0, 4) == buf);
        CHECK(raw_index_bytes(c0, 1, 3) == buf + 1);
        CHECK(raw_bytes(cp, 4) == buf);
        CHECK(raw_index_bytes(cp, 1, 3) == buf + 1);
        CHECK(raw_bytes(cs, 4) == buf);
        CHECK(raw_index_bytes(cs, 1, 3) == buf + 1);
#if WITH_XSPAN >= 2
        CHECK_THROWS(raw_bytes(cs, 5));
        CHECK_THROWS(raw_index_bytes(cs, 1, 4));
#endif

        XSPAN_0(upx_uint16_t) c0_2 = XSPAN_TYPE_CAST(upx_uint16_t, c0 + 2);
        XSPAN_P(upx_uint16_t) cp_2 = XSPAN_TYPE_CAST(upx_uint16_t, cp + 2);
        XSPAN_S(upx_uint16_t) cs_2 = XSPAN_TYPE_CAST(upx_uint16_t, cs + 2);
        CHECK(ptr_udiff_bytes(c0_2, c0) == 2u);
        CHECK(ptr_udiff_bytes(cp_2, c0) == 2u);
        CHECK(ptr_udiff_bytes(cs_2, c0) == 2u);
        CHECK(ptr_udiff_bytes(c0_2, cp) == 2u);
        CHECK(ptr_udiff_bytes(cp_2, cp) == 2u);
        CHECK(ptr_udiff_bytes(cs_2, cp) == 2u);
        CHECK(ptr_udiff_bytes(c0_2, cs) == 2u);
        CHECK(ptr_udiff_bytes(cp_2, cs) == 2u);
        CHECK(ptr_udiff_bytes(cs_2, cs) == 2u);
        XSPAN_0(upx_uint16_t) c0_2b = XSPAN_TYPE_CAST(upx_uint16_t, c0) + 1;
        XSPAN_P(upx_uint16_t) cp_2b = XSPAN_TYPE_CAST(upx_uint16_t, cp) + 1;
        XSPAN_S(upx_uint16_t) cs_2b = XSPAN_TYPE_CAST(upx_uint16_t, cs) + 1;
        CHECK(c0_2 == c0_2b);
        CHECK(cp_2 == cp_2b);
        CHECK(cs_2 == cs_2b);

        CHECK(sizeof(*c0) == 1u);
        CHECK(sizeof(*c0_2) == 2u);
    }

    SUBCASE("XSPAN_x_VAR") {
        XSPAN_0_VAR(char, b0, buf);
        XSPAN_P_VAR(char, bp, buf);

        XSPAN_0_VAR(char, c0, buf, sizeof(buf));
        XSPAN_P_VAR(char, cp, buf, sizeof(buf));
        XSPAN_S_VAR(char, cs, buf, sizeof(buf));

        XSPAN_0_VAR(char, d0, buf + 1, sizeof(buf), buf);
        XSPAN_P_VAR(char, dp, buf + 1, sizeof(buf), buf);
        XSPAN_S_VAR(char, ds, buf + 1, sizeof(buf), buf);

        XSPAN_0_VAR(const char, const x0, buf, sizeof(buf));
        XSPAN_P_VAR(const char, const xp, buf, sizeof(buf));
        XSPAN_S_VAR(const char, const xs, buf, sizeof(buf));
        XSPAN_P_VAR(const char, const yp, xs);
        XSPAN_0_VAR(const char, const z0p, yp);
        XSPAN_0_VAR(const char, const z0s, xs);

        CHECK(c0 == b0);
        CHECK(cp == bp);
        CHECK(cs == bp);
        CHECK(d0 == dp);
        CHECK(d0 == ds);
        CHECK(x0 == z0p);
        CHECK(xp == z0s);

#if WITH_XSPAN >= 1 || __cplusplus >= 201103L
        XSPAN_0_VAR(char, a0, nullptr);
        CHECK((a0 == nullptr));
        CHECK_NOTHROW(raw_bytes(a0, 0));
        CHECK_THROWS(raw_bytes(a0, 1));
        CHECK_THROWS(raw_index_bytes(a0, 0, 0));
#endif

        CHECK(raw_bytes(b0, 0) == buf);
        CHECK(raw_bytes(bp, 0) == buf);
        // info: these will fail if we ever add an overload for bounded-arrays
#if WITH_XSPAN >= 2
        CHECK(b0.raw_size_in_bytes() == 0u);
        CHECK(bp.raw_size_in_bytes() == 0u);
#endif
        CHECK(raw_bytes(b0, 999999) == buf);
        CHECK(raw_bytes(bp, 999999) == buf);

        CHECK(raw_bytes(c0, 4) == buf);
        CHECK(raw_index_bytes(c0, 1, 3) == buf + 1);
        CHECK(raw_bytes(cp, 4) == buf);
        CHECK(raw_index_bytes(cp, 1, 3) == buf + 1);
        CHECK(raw_bytes(cs, 4) == buf);
        CHECK(raw_index_bytes(cs, 1, 3) == buf + 1);
#if WITH_XSPAN >= 2
        CHECK_THROWS(raw_bytes(cs, 5));
        CHECK_THROWS(raw_index_bytes(cs, 1, 4));
#endif

        XSPAN_0_VAR(upx_uint16_t, c0_2, XSPAN_TYPE_CAST(upx_uint16_t, c0 + 2));
        XSPAN_P_VAR(upx_uint16_t, cp_2, XSPAN_TYPE_CAST(upx_uint16_t, cp + 2));
        XSPAN_S_VAR(upx_uint16_t, cs_2, XSPAN_TYPE_CAST(upx_uint16_t, cs + 2));
        CHECK(ptr_udiff_bytes(c0_2, c0) == 2u);
        CHECK(ptr_udiff_bytes(cp_2, c0) == 2u);
        CHECK(ptr_udiff_bytes(cs_2, c0) == 2u);
        CHECK(ptr_udiff_bytes(c0_2, cp) == 2u);
        CHECK(ptr_udiff_bytes(cp_2, cp) == 2u);
        CHECK(ptr_udiff_bytes(cs_2, cp) == 2u);
        CHECK(ptr_udiff_bytes(c0_2, cs) == 2u);
        CHECK(ptr_udiff_bytes(cp_2, cs) == 2u);
        CHECK(ptr_udiff_bytes(cs_2, cs) == 2u);
        XSPAN_0_VAR(upx_uint16_t, c0_2b, XSPAN_TYPE_CAST(upx_uint16_t, c0) + 1);
        XSPAN_P_VAR(upx_uint16_t, cp_2b, XSPAN_TYPE_CAST(upx_uint16_t, cp) + 1);
        XSPAN_S_VAR(upx_uint16_t, cs_2b, XSPAN_TYPE_CAST(upx_uint16_t, cs) + 1);
        CHECK(c0_2 == c0_2b);
        CHECK(cp_2 == cp_2b);
        CHECK(cs_2 == cs_2b);

        CHECK(sizeof(*c0) == 1u);
        CHECK(sizeof(*c0_2) == 2u);
    }

    SUBCASE("xspan in class") {
        struct MyType {
            XSPAN_0(char) s0;
            XSPAN_P(char) sp;
            XSPAN_S(char) ss;
#if __cplusplus >= 201103L
            XSPAN_0(char) x0 = nullptr;
#endif
#if WITH_XSPAN >= 2
            // much nicer syntax when using fully checked xspan:
            MyType(char *b, size_t n, bool) : s0(b, n), sp(b, n), ss(b, n) {}
#endif
            MyType(char *b, size_t n)
                : s0(XSPAN_0_MAKE(char, b, n)), sp(XSPAN_P_MAKE(char, b, n)),
                  ss(XSPAN_S_MAKE(char, b, n)) {
                UNUSED(n);
            }
        };
        MyType x(buf, sizeof(buf));
        MyType y = MyType(buf, sizeof(buf));
        CHECK(x.s0 == y.sp);
    }
}

/*************************************************************************
//
**************************************************************************/

#if (WITH_XSPAN >= 2) && DEBUG

TEST_CASE("PtrOrSpanOrNull") {
    char real_buf[2 + 6 + 2] = {126, 127, 0, 1, 2, 3, 4, 5, 124, 125};
    char *base_buf = real_buf + 2;
    char *const my_null = nullptr;
    typedef PtrOrSpanOrNull<char> Span0;

    // basic nullptr
    CHECK_NOTHROW(Span0(base_buf, 4, base_buf) = my_null);
    CHECK_NOTHROW(Span0(base_buf, 4, base_buf).assign(my_null));
    // basic range checking
    CHECK_NOTHROW(Span0(base_buf, 4, base_buf));
    CHECK_NOTHROW(Span0(base_buf, 0, base_buf));
    CHECK_NOTHROW(Span0(base_buf, 0, base_buf) - 0);
    CHECK_THROWS(Span0(base_buf, 0, base_buf) + 1);
    CHECK_THROWS(Span0(base_buf, 0, base_buf) - 1);
    CHECK_NOTHROW(Span0(base_buf, 4, base_buf) + 4);
    CHECK_THROWS(Span0(base_buf, 4, base_buf) + 5);
    CHECK_THROWS(Span0(base_buf - 1, 4, base_buf));
    CHECK_THROWS(Span0(base_buf + 1, 0, base_buf));
    // basic same base
    CHECK_NOTHROW(Span0(base_buf, 4, base_buf) = Span0(base_buf + 1, 3, base_buf));
    CHECK_NOTHROW(Span0(base_buf, 4, base_buf) = Span0(base_buf + 1, 1, base_buf));
    CHECK_NOTHROW(Span0(base_buf, 4, base_buf) = Span0(base_buf + 1, 5, base_buf));
    CHECK_THROWS(Span0(base_buf, 4, base_buf) = Span0(base_buf + 1, 3, base_buf + 1));

    Span0 a1(nullptr);
    assert(a1 == nullptr);
    assert(a1.raw_ptr() == nullptr);
    assert(a1.raw_base() == nullptr);
    assert(a1.raw_size_in_bytes() == 0u);
    CHECK_THROWS(*a1);
    CHECK_THROWS(a1[0]);

    Span0 a2 = nullptr;
    assert(a2 == nullptr);
    assert(a2.raw_ptr() == nullptr);
    assert(a2.raw_base() == nullptr);
    assert(a2.raw_size_in_bytes() == 0u);
    CHECK_THROWS(*a2);
    CHECK_THROWS(a2[0]);

    Span0 base0(nullptr, 4, base_buf);
    assert(base0.raw_ptr() == nullptr);
    assert(base0.raw_base() == base_buf);
    assert(base0.raw_size_in_bytes() == 4u);
    CHECK_THROWS(*base0);    // nullptr
    CHECK_THROWS(base0[0]);  // nullptr
    CHECK_THROWS(base0 + 1); // nullptr

    Span0 base4(base_buf, 4);
    assert(base4.raw_ptr() == base_buf);
    assert(base4.raw_base() == base_buf);
    assert(base4.raw_size_in_bytes() == 4u);

    a1 = base_buf;
    a1 = base0;
    assert(a1 == nullptr);
    assert(a1.raw_ptr() == nullptr);
    assert(a1.raw_base() == base_buf);
    assert(a1.raw_size_in_bytes() == 4u);
    a1 = base4;
    assert(a1 == base_buf);
    assert(a1.raw_ptr() == base_buf);
    assert(a1.raw_base() == base_buf);
    assert(a1.raw_size_in_bytes() == 4u);

    a1 = base_buf;
    assert(a1 != nullptr);
    a1 = base_buf + 1;
    CHECK(*a1++ == 1);
    CHECK(*++a1 == 3);
    CHECK(*a1 == 3);
    a1 = base_buf + 4; // at the end of buffer
    CHECK_THROWS(*a1);
    CHECK_THROWS(a1 = base_buf + 5); // range error
    assert(a1 == base_buf + 4);
    CHECK(a1[-4] == 0);
    CHECK_THROWS(a1[-5]); // range error
    a1 = base_buf;
    CHECK(*a1 == 0);

    Span0 new_base4(base_buf + 2, 4);
    CHECK_THROWS(a1 = new_base4); // not same base
    a2 = new_base4;
    CHECK_THROWS(a2 = base4); // not same base

    Span0 s0_no_base(nullptr);
    Span0 s0_with_base(nullptr, 4, base_buf);
    s0_no_base = nullptr;
    s0_with_base = nullptr;
    s0_with_base = s0_no_base;
    assert(s0_no_base.raw_base() == nullptr);
    assert(s0_with_base.raw_base() == base_buf);
    s0_no_base = s0_with_base;
    assert(s0_no_base.raw_base() == base_buf);
    assert(s0_no_base.raw_ptr() == nullptr);
    assert(s0_with_base.raw_ptr() == nullptr);
    s0_no_base = my_null;
    s0_with_base = my_null;
}

/*************************************************************************
//
**************************************************************************/

TEST_CASE("PtrOrSpan") {
    char real_buf[2 + 6 + 2] = {126, 127, 0, 1, 2, 3, 4, 5, 124, 125};
    char *base_buf = real_buf + 2;
    char *const my_null = nullptr;
    typedef PtrOrSpan<char> SpanP;

    // basic nullptr
    CHECK_THROWS(SpanP(base_buf, 4, base_buf) = my_null);
    CHECK_THROWS(SpanP(base_buf, 4, base_buf).assign(my_null));
    // basic range checking
    CHECK_NOTHROW(SpanP(base_buf, 4, base_buf));
    CHECK_NOTHROW(SpanP(base_buf, 0, base_buf));
    CHECK_NOTHROW(SpanP(base_buf, 0, base_buf) - 0);
    CHECK_THROWS(SpanP(base_buf, 0, base_buf) + 1);
    CHECK_THROWS(SpanP(base_buf, 0, base_buf) - 1);
    CHECK_NOTHROW(SpanP(base_buf, 4, base_buf) + 4);
    CHECK_THROWS(SpanP(base_buf, 4, base_buf) + 5);
    CHECK_THROWS(SpanP(base_buf - 1, 4, base_buf));
    CHECK_THROWS(SpanP(base_buf + 1, 0, base_buf));
    // basic same base
    CHECK_NOTHROW(SpanP(base_buf, 4, base_buf) = SpanP(base_buf + 1, 3, base_buf));
    CHECK_NOTHROW(SpanP(base_buf, 4, base_buf) = SpanP(base_buf + 1, 1, base_buf));
    CHECK_NOTHROW(SpanP(base_buf, 4, base_buf) = SpanP(base_buf + 1, 5, base_buf));
    CHECK_THROWS(SpanP(base_buf, 4, base_buf) = SpanP(base_buf + 1, 3, base_buf + 1));

    SpanP x1(base_buf, 0);
    assert(x1 != nullptr);
    assert(x1.raw_ptr() == base_buf);
    assert(x1.raw_base() == base_buf);
    assert(x1.raw_size_in_bytes() == 0u);
    CHECK_THROWS(*x1);
    CHECK_THROWS(x1[0]);

    SpanP a2 = base_buf;
    assert(a2 != nullptr);
    assert(a2.raw_ptr() == base_buf);
    assert(a2.raw_base() == nullptr);
    assert(a2.raw_size_in_bytes() == 0u);
    CHECK(*a2 == 0);
    CHECK(a2[1] == 1);

    SpanP base0(base_buf, 4, base_buf);
    assert(base0.raw_ptr() == base_buf);
    assert(base0.raw_base() == base_buf);
    assert(base0.raw_size_in_bytes() == 4u);

    SpanP base4(base_buf, 4);
    assert(base4.raw_ptr() == base_buf);
    assert(base4.raw_base() == base_buf);
    assert(base4.raw_size_in_bytes() == 4u);

    SpanP a1(base_buf, 4);
    a1 = base_buf;
    a1 = base0;
    assert(a1 == base0);
    assert(a1 != nullptr);
    assert(a1.raw_ptr() == base0.raw_ptr());
    assert(a1.raw_base() == base_buf);
    assert(a1.raw_size_in_bytes() == 4u);
    a1 = base4;
    assert(a1 == base_buf);
    assert(a1.raw_ptr() == base_buf);
    assert(a1.raw_base() == base_buf);
    assert(a1.raw_size_in_bytes() == 4u);

    a1 = base_buf;
    a1 = base_buf + 1;
    CHECK(*a1++ == 1);
    CHECK(*++a1 == 3);
    CHECK(*a1 == 3);
    a1 = base_buf + 4; // at the end of buffer
    CHECK_THROWS(*a1);
    CHECK_THROWS(a1 = base_buf + 5); // range error
    assert(a1 == base_buf + 4);
    CHECK(a1[-4] == 0);
    CHECK_THROWS(a1[-5]); // range error
    a1 = base_buf;
    CHECK(*a1 == 0);

    SpanP new_base4(base_buf + 2, 4);
    CHECK_THROWS(a1 = new_base4); // not same base
    a2 = new_base4;
    CHECK_THROWS(a2 = base4); // not same base

    SpanP sp_no_base(base_buf);
    SpanP sp_with_base(base_buf, 4, base_buf);
    assert(sp_no_base.raw_base() == nullptr);
    assert(sp_with_base.raw_base() == base_buf);
    CHECK_THROWS(sp_no_base = my_null);   // nullptr assignment
    CHECK_THROWS(sp_with_base = my_null); // nullptr assignment
#if XSPAN_CONFIG_ENABLE_SPAN_CONVERSION
    typedef PtrOrSpanOrNull<char> Span0;
    Span0 s0_no_base(nullptr);
    Span0 s0_with_base(nullptr, 4, base_buf);
    CHECK_THROWS(sp_no_base = s0_no_base);     // nullptr assignment
    CHECK_THROWS(sp_no_base = s0_with_base);   // nullptr assignment
    CHECK_THROWS(sp_with_base = s0_no_base);   // nullptr assignment
    CHECK_THROWS(sp_with_base = s0_with_base); // nullptr assignment
#endif
    UNUSED(my_null);
}

/*************************************************************************
//
**************************************************************************/

TEST_CASE("Span") {
    char real_buf[2 + 6 + 2] = {126, 127, 0, 1, 2, 3, 4, 5, 124, 125};
    char *base_buf = real_buf + 2;
    char *const my_null = nullptr;
    typedef Span<char> SpanS;

    // basic nullptr
    CHECK_THROWS(SpanS(base_buf, 4, base_buf) = my_null);
    CHECK_THROWS(SpanS(base_buf, 4, base_buf).assign(my_null));
    // basic range checking
    CHECK_NOTHROW(SpanS(base_buf, 4, base_buf));
    CHECK_NOTHROW(SpanS(base_buf, 0, base_buf));
    CHECK_NOTHROW(SpanS(base_buf, 0, base_buf) - 0);
    CHECK_THROWS(SpanS(base_buf, 0, base_buf) + 1);
    CHECK_THROWS(SpanS(base_buf, 0, base_buf) - 1);
    CHECK_NOTHROW(SpanS(base_buf, 4, base_buf) + 4);
    CHECK_THROWS(SpanS(base_buf, 4, base_buf) + 5);
    CHECK_THROWS(SpanS(base_buf - 1, 4, base_buf));
    CHECK_THROWS(SpanS(base_buf + 1, 0, base_buf));
    // basic same base
    CHECK_NOTHROW(SpanS(base_buf, 4, base_buf) = SpanS(base_buf + 1, 3, base_buf));
    CHECK_NOTHROW(SpanS(base_buf, 4, base_buf) = SpanS(base_buf + 1, 1, base_buf));
    CHECK_NOTHROW(SpanS(base_buf, 4, base_buf) = SpanS(base_buf + 1, 5, base_buf));
    CHECK_THROWS(SpanS(base_buf, 4, base_buf) = SpanS(base_buf + 1, 3, base_buf + 1));

    SpanS x1(base_buf, 0);
    assert(x1 != nullptr);
    assert(x1.raw_ptr() == base_buf);
    assert(x1.raw_base() == base_buf);
    assert(x1.raw_size_in_bytes() == 0u);
    CHECK_THROWS(*x1);
    CHECK_THROWS(x1[0]);

    SpanS a2(base_buf, 4);
    assert(a2 != nullptr);
    assert(a2.raw_ptr() == base_buf);
    assert(a2.raw_base() == base_buf);
    assert(a2.raw_size_in_bytes() == 4u);
    CHECK(*a2 == 0);
    CHECK(a2[1] == 1);

    SpanS base0(base_buf, 4, base_buf);
    assert(base0.raw_ptr() == base_buf);
    assert(base0.raw_base() == base_buf);
    assert(base0.raw_size_in_bytes() == 4u);

    SpanS base4(base_buf, 4);
    assert(base4.raw_ptr() == base_buf);
    assert(base4.raw_base() == base_buf);
    assert(base4.raw_size_in_bytes() == 4u);

    SpanS a1(base_buf, 4);
    a1 = base_buf;
    a1 = base0;
    assert(a1 == base0);
    assert(a1 != nullptr);
    assert(a1.raw_ptr() == base0.raw_ptr());
    assert(a1.raw_base() == base_buf);
    assert(a1.raw_size_in_bytes() == 4u);
    a1 = base4;
    assert(a1 == base_buf);
    assert(a1.raw_ptr() == base_buf);
    assert(a1.raw_base() == base_buf);
    assert(a1.raw_size_in_bytes() == 4u);

    a1 = base_buf;
    a1 = base_buf + 1;
    CHECK(*a1++ == 1);
    CHECK(*++a1 == 3);
    CHECK(*a1 == 3);
    a1 = base_buf + 4; // at the end of buffer
    CHECK_THROWS(*a1);
    CHECK_THROWS(a1 = base_buf + 5); // range error
    assert(a1 == base_buf + 4);
    CHECK(a1[-4] == 0);
    CHECK_THROWS(a1[-5]); // range error
    a1 = base_buf;
    CHECK(*a1 == 0);

    SpanS new_base4(base_buf + 2, 4);
    CHECK_THROWS(a1 = new_base4); // not same base
    CHECK_THROWS(a2 = new_base4); // not same base

    SpanS ss_with_base(base_buf, 4, base_buf);
    assert(ss_with_base.raw_base() == base_buf);
    CHECK_THROWS(ss_with_base = my_null); // nullptr assignment
#if XSPAN_CONFIG_ENABLE_SPAN_CONVERSION
    {
        typedef PtrOrSpanOrNull<char> Span0;
        // v0 nullptr, b0 base, b1 base + 1
        const Span0 v0_v0(nullptr);
        const Span0 v0_b0(nullptr, 4, base_buf);
        const Span0 v0_b1(nullptr, 3, base_buf + 1);
        const Span0 b0_v0(base_buf);
        const Span0 b0_b0(base_buf, 4, base_buf);
        CHECK_THROWS(XSPAN_0_MAKE(char, base_buf, 3, base_buf + 1)); // b0_b1
        const Span0 b1_v0(base_buf + 1);
        const Span0 b1_b0(base_buf + 1, 4, base_buf);
        const Span0 b1_b1(base_buf + 1, 3, base_buf + 1);
        CHECK_THROWS(ss_with_base = v0_v0); // nullptr assignment
        CHECK_THROWS(ss_with_base = v0_b0); // nullptr assignment
        CHECK_THROWS(ss_with_base = v0_b1); // nullptr assignment
        CHECK_NOTHROW(ss_with_base = b0_v0);
        CHECK_NOTHROW(ss_with_base = b0_b0);
        CHECK_NOTHROW(ss_with_base = b1_v0);
        CHECK_NOTHROW(ss_with_base = b1_b0);
        CHECK_THROWS(ss_with_base = b1_b1); // different base
        CHECK_THROWS(XSPAN_S_MAKE(char, v0_v0));
        CHECK_THROWS(XSPAN_S_MAKE(char, v0_b0));
        CHECK_THROWS(XSPAN_S_MAKE(char, v0_b1));
        CHECK_THROWS(XSPAN_S_MAKE(char, b0_v0));
        CHECK_NOTHROW(XSPAN_S_MAKE(char, b0_b0));
        CHECK_THROWS(XSPAN_S_MAKE(char, b1_v0));
        CHECK_NOTHROW(XSPAN_S_MAKE(char, b1_b0));
        CHECK_NOTHROW(XSPAN_S_MAKE(char, b1_b1));
        //
        CHECK((XSPAN_S_MAKE(char, b0_b0).raw_base() == base_buf));
        CHECK((XSPAN_S_MAKE(char, b1_b0).raw_base() == base_buf));
        CHECK((XSPAN_S_MAKE(char, b1_b1).raw_base() == base_buf + 1));
    }
    {
        typedef PtrOrSpan<char> SpanP;
        // v0 nullptr, b0 base, b1 base + 1
        const SpanP b0_v0(base_buf);
        const SpanP b0_b0(base_buf, 4, base_buf);
        CHECK_THROWS(XSPAN_P_MAKE(char, base_buf, 3, base_buf + 1)); // b0_b1
        const SpanP b1_v0(base_buf + 1);
        const SpanP b1_b0(base_buf + 1, 4, base_buf);
        const SpanP b1_b1(base_buf + 1, 3, base_buf + 1);
        CHECK_NOTHROW(ss_with_base = b0_v0);
        CHECK_NOTHROW(ss_with_base = b0_b0);
        CHECK_NOTHROW(ss_with_base = b1_v0);
        CHECK_NOTHROW(ss_with_base = b1_b0);
        CHECK_THROWS(ss_with_base = b1_b1); // different base
        CHECK_THROWS(XSPAN_S_MAKE(char, b0_v0));
        CHECK_NOTHROW(XSPAN_S_MAKE(char, b0_b0));
        CHECK_THROWS(XSPAN_S_MAKE(char, b1_v0));
        CHECK_NOTHROW(XSPAN_S_MAKE(char, b1_b0));
        CHECK_NOTHROW(XSPAN_S_MAKE(char, b1_b1));
        //
        CHECK((XSPAN_S_MAKE(char, b0_b0).raw_base() == base_buf));
        CHECK((XSPAN_S_MAKE(char, b1_b0).raw_base() == base_buf));
        CHECK((XSPAN_S_MAKE(char, b1_b1).raw_base() == base_buf + 1));
    }
#endif
    UNUSED(my_null);
}

/*************************************************************************
//
**************************************************************************/

TEST_CASE("Span void ptr") {
    static char a[4] = {0, 1, 2, 3};
    XSPAN_0(void) a0(a, 4);
    XSPAN_P(void) ap(a, 4);
    XSPAN_S(void) as(a, 4);
    XSPAN_0(const void) c0(a, 4);
    XSPAN_P(const void) cp(a, 4);
    XSPAN_S(const void) cs(a, 4);
    static const char b[4] = {0, 1, 2, 3};
    XSPAN_0(const void) b0(b, 4);
    XSPAN_P(const void) bp(b, 4);
    XSPAN_S(const void) bs(b, 4);
}

TEST_CASE("Span deref/array/arrow") {
    static char real_a[2 + 4 + 2] = {126, 127, 0, 1, 2, 3, 124, 125};
    static char *a = real_a + 2;
    XSPAN_0(char) a0(a, 4);
    XSPAN_P(char) ap(a, 4);
    XSPAN_S(char) as(a, 4);
    CHECK_THROWS(a0[4]);
    CHECK_THROWS(a0[-1]);
    CHECK_THROWS(a0[-2]);
    a0 += 2;
    CHECK(*a0 == 2);
    CHECK(a0[-1] == 1);
    CHECK(a0[0] == 2);
    CHECK(a0[1] == 3);
    ap += 2;
    CHECK(*ap == 2);
    CHECK(ap[-1] == 1);
    CHECK(ap[0] == 2);
    CHECK(ap[1] == 3);
    as += 2;
    CHECK(*as == 2);
    CHECK(as[-1] == 1);
    CHECK(as[0] == 2);
    CHECK(as[1] == 3);
}

TEST_CASE("Span subspan") {
    static char buf[4] = {0, 1, 2, 3};
    XSPAN_S(char) as(buf, 4);
    CHECK(as.subspan(1, 1)[0] == 1);
    CHECK((as + 1).subspan(1, 1)[0] == 2);
    CHECK((as + 2).subspan(0, -2)[0] == 0);
    CHECK_THROWS(as.subspan(1, 0)[0]);
    CHECK_THROWS(as.subspan(1, 1)[-1]);
}

TEST_CASE("Span constness") {
    static char buf[4] = {0, 1, 2, 3};

    // NOLINTBEGIN(performance-unnecessary-copy-initialization)

    XSPAN_0(char) b0(buf, 4);
    XSPAN_P(char) bp(buf, 4);
    XSPAN_S(char) bs(buf, 4);

    XSPAN_0(char) s0(b0);
    XSPAN_P(char) sp(bp);
    XSPAN_S(char) ss(bs);

    XSPAN_0(const char) b0c(buf, 4);
    XSPAN_P(const char) bpc(buf, 4);
    XSPAN_S(const char) bsc(buf, 4);

    XSPAN_0(const char) s0c(b0c);
    XSPAN_P(const char) spc(bpc);
    XSPAN_S(const char) ssc(bsc);

    XSPAN_0(const char) x0c(b0);
    XSPAN_P(const char) xpc(bp);
    XSPAN_S(const char) xsc(bs);

    // NOLINTEND(performance-unnecessary-copy-initialization)

    CHECK(ptr_diff_bytes(b0, buf) == 0);
    CHECK(ptr_diff_bytes(bp, buf) == 0);
    CHECK(ptr_diff_bytes(bs, buf) == 0);
    CHECK(ptr_diff_bytes(s0, buf) == 0);
    CHECK(ptr_diff_bytes(sp, buf) == 0);
    CHECK(ptr_diff_bytes(bs, buf) == 0);
    //
    CHECK(ptr_diff_bytes(s0, bp) == 0);
    CHECK(ptr_diff_bytes(s0, sp) == 0);
    CHECK(ptr_diff_bytes(s0, ss) == 0);
    //
    CHECK(ptr_diff_bytes(s0c, b0c) == 0);
    CHECK(ptr_diff_bytes(spc, bpc) == 0);
    CHECK(ptr_diff_bytes(ssc, bsc) == 0);
}

/*************************************************************************
//
**************************************************************************/

#if !defined(DOCTEST_CONFIG_DISABLE)
namespace {
int my_memcmp_v1(XSPAN_P(const void) a, XSPAN_0(const void) b, size_t n) {
    if (b == nullptr)
        return -2;
    XSPAN_0(const void) x(a);
    return memcmp(x, b, n);
}
int my_memcmp_v2(XSPAN_P(const char) a, XSPAN_0(const char) b, size_t n) {
    if (a == b)
        return 0;
    if (b == nullptr)
        return -2;
    a += 1;
    b -= 1;
    XSPAN_0(const char) x(a);
    XSPAN_0(const char) y = b;
    return memcmp(x, y, n);
}
} // namespace
#endif

TEST_CASE("PtrOrSpan") {
    static const char buf[4] = {0, 1, 2, 3};
    CHECK(my_memcmp_v1(buf, nullptr, 4) == -2);
    CHECK(my_memcmp_v2(buf + 4, buf + 4, 999) == 0);
    CHECK(my_memcmp_v2(buf, buf + 2, 3) == 0);
    UNUSED(buf);
}

/*************************************************************************
//
**************************************************************************/

TEST_CASE("PtrOrSpan char") {
    char real_buf[2 + 8 + 2] = {126, 127, 0, 1, 2, 3, 4, 5, 6, 7, 124, 125};
    char *buf = real_buf + 2;
    XSPAN_P(char) a(buf, XSpanSizeInBytes(8));
    XSPAN_P(char) b = a.subspan(0, 7);
    XSPAN_P(char) c = (b + 1).subspan(0, 6);
    a += 1;
    CHECK(*a == 1);
    *a++ += 1;
    *b++ = 1;
    CHECK(a == buf + 2);
    CHECK(b == buf + 1);
    CHECK(c == buf + 1);
    CHECK(*b == 2);
    CHECK(*c == 2);
    CHECK(a.raw_size_in_bytes() == 8u);
    CHECK(b.raw_size_in_bytes() == 7u);
    CHECK(c.raw_size_in_bytes() == 6u);
    CHECK(a.raw_base() == buf);
    CHECK(b.raw_base() == buf);
    CHECK(c.raw_base() == buf + 1);
#ifdef UPX_VERSION_HEX
    CHECK(get_le32(a) != 0);
#endif
    ++c;
    c++;
#ifdef UPX_VERSION_HEX
    CHECK(get_le32(c) != 0);
#endif
    ++c;
#ifdef UPX_VERSION_HEX
    CHECK_THROWS(get_le32(c));
#endif
    ++b;
    b++;
    b += 4;
    CHECK(b.raw_ptr() == buf + 7);
    CHECK_THROWS(*b);
    CHECK(a.raw_size_in_bytes() == 8u);
    a = b;
    CHECK(a.raw_size_in_bytes() == 8u);
    CHECK(a.raw_ptr() == buf + 7);
    a++;
    CHECK_THROWS(*a);
    CHECK_THROWS(raw_bytes(a, 1));
    a = b;
    CHECK_THROWS(a = c);
    *a = 0;
    a = buf;
#ifdef UPX_VERSION_HEX
    CHECK(upx_safe_strlen(a) == 7u);
#endif
}

TEST_CASE("PtrOrSpan int") {
    int buf[8] = {0, 11, 22, 33, 44, 55, 66, 77};
    XSPAN_P(const int) a(buf, XSpanCount(8));
    CHECK(a.raw_size_in_bytes() == 8 * sizeof(int));
    XSPAN_P(const int) b = a.subspan(0, 7);
    CHECK(b.raw_size_in_bytes() == 7 * sizeof(int));
    XSPAN_P(const int) c = (b + 1).subspan(0, 6);
    CHECK(c.raw_size_in_bytes() == 6 * sizeof(int));
    a += 1;
    CHECK(a == buf + 1);
    CHECK(*a == 11);
    CHECK(*a++ == 11);
    CHECK(a == buf + 2);
    CHECK(*a == 22);
    CHECK(*++a == 33);
    CHECK(a == buf + 3);
    CHECK(*a == 33);
    CHECK(*--a == 22);
    CHECK(a == buf + 2);
    CHECK(*a == 22);
    CHECK(*a-- == 22);
    CHECK(a == buf + 1);
    CHECK(*a == 11);
    CHECK(*b == 0);
    CHECK(*c == 11);
    a -= 1;
    a += 7;
#ifdef UPX_VERSION_HEX
    CHECK(get_le32(a) == ne32_to_le32(77));
#endif
    a++;
#ifdef UPX_VERSION_HEX
    CHECK_THROWS(get_le32(a));
#endif
    CHECK_THROWS(raw_bytes(a, 1));
    CHECK_THROWS(a++);
    CHECK_THROWS(++a);
    CHECK_THROWS(a += 1);
    CHECK(a == buf + 8);
    a = buf;
    CHECK_THROWS(a--);
    CHECK_THROWS(--a);
    CHECK_THROWS(a -= 1);
    CHECK_THROWS(a += 9);
    CHECK(a == buf);
    a += 8;
    CHECK(a == buf + 8);
}

/*************************************************************************
// codegen
**************************************************************************/

namespace {
template <class T>
static noinline int foo(T p) {
    unsigned r = 0;
    r += *p++;
    r += *++p;
    p += 3;
    r += *p;
    r += *--p;
    r += *p--;
    r += *p;
    return r;
}

template <class T>
XSPAN_0(T)
make_span_0(T *ptr, size_t count) {
    return PtrOrSpanOrNull<T>(ptr, count);
}
template <class T>
XSPAN_P(T)
make_span_p(T *ptr, size_t count) {
    return PtrOrSpan<T>(ptr, count);
}
template <class T>
XSPAN_S(T)
make_span_s(T *ptr, size_t count) {
    return Span<T>(ptr, count);
}
} // namespace

TEST_CASE("Span codegen") {
    upx_uint8_t buf[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    CHECK(foo(buf) == 0 + 2 + 5 + 4 + 4 + 3);
    CHECK(foo(make_span_0(buf, 8)) == 0 + 2 + 5 + 4 + 4 + 3);
    CHECK(foo(make_span_p(buf, 8)) == 0 + 2 + 5 + 4 + 4 + 3);
    CHECK(foo(make_span_s(buf, 8)) == 0 + 2 + 5 + 4 + 4 + 3);
    CHECK(foo(XSPAN_0_MAKE(upx_uint8_t, buf, 8)) == 0 + 2 + 5 + 4 + 4 + 3);
    CHECK(foo(XSPAN_P_MAKE(upx_uint8_t, buf, 8)) == 0 + 2 + 5 + 4 + 4 + 3);
    CHECK(foo(XSPAN_S_MAKE(upx_uint8_t, buf, 8)) == 0 + 2 + 5 + 4 + 4 + 3);
    UNUSED(buf);
}

#endif // WITH_XSPAN >= 2

/*************************************************************************
// misc
**************************************************************************/

namespace {
template <class T>
struct PointerTraits {
    typedef typename std::add_lvalue_reference<T>::type reference;
    typedef
        typename std::add_lvalue_reference<typename std::add_const<T>::type>::type const_reference;
    typedef typename std::add_pointer<T>::type pointer;
    typedef typename std::add_pointer<typename std::add_const<T>::type>::type const_pointer;
};
} // namespace

#if __cplusplus >= 201103L

TEST_CASE("decltype integral constants") {
    static_assert((std::is_same<decltype(0), int>::value), "");
    static_assert((std::is_same<decltype(0u), unsigned>::value), "");
    static_assert((std::is_same<decltype(0l), long>::value), "");
    static_assert((std::is_same<decltype(0ul), unsigned long>::value), "");
    static_assert((std::is_same<decltype(0ll), long long>::value), "");
    static_assert((std::is_same<decltype(0ull), unsigned long long>::value), "");
    static_assert((std::is_same<decltype((char) 0), char>::value), "");
    static_assert((std::is_same<decltype((short) 0), short>::value), "");
    static_assert((std::is_same<decltype((long) 0), long>::value), "");
    static_assert((std::is_same<decltype((long long) 0), long long>::value), "");
    static_assert((std::is_same<decltype(char(0)), char>::value), "");
    static_assert((std::is_same<decltype(short(0)), short>::value), "");
    static_assert((std::is_same<decltype(long(0)), long>::value), "");
}

TEST_CASE("decltype pointer") {
    int dummy = 0;
    int *p = &dummy;
    const int *c = &dummy;
    static_assert((std::is_same<decltype(p - p), std::ptrdiff_t>::value), "");
    static_assert((std::is_same<decltype(c - c), std::ptrdiff_t>::value), "");
    static_assert((std::is_same<decltype(p - c), std::ptrdiff_t>::value), "");
    static_assert((std::is_same<decltype(c - p), std::ptrdiff_t>::value), "");
    typedef PointerTraits<int> TInt;
    typedef PointerTraits<const int> TConstInt;
    static_assert((std::is_same<int *, TInt::pointer>::value), "");
    static_assert((std::is_same<const int *, TInt::const_pointer>::value), "");
    static_assert((std::is_same<const int *, TConstInt::pointer>::value), "");
    static_assert((std::is_same<const int *, TConstInt::const_pointer>::value), "");
    //
    static_assert((std::is_same<decltype(p), TInt::pointer>::value), "");
    static_assert((std::is_same<decltype(c), TInt::const_pointer>::value), "");
    static_assert((std::is_same<decltype(c), TConstInt::pointer>::value), "");
    static_assert((std::is_same<decltype(p + 1), TInt::pointer>::value), "");
    static_assert((std::is_same<decltype(c + 1), TInt::const_pointer>::value), "");
    static_assert((std::is_same<decltype(c + 1), TConstInt::pointer>::value), "");
    static_assert((std::is_same<decltype(c + 1), TConstInt::const_pointer>::value), "");
    static_assert((std::is_same<decltype(c + 1), const int *>::value), "");
    // dereference
    static_assert((std::is_same<decltype(*p), TInt::reference>::value), "");
    static_assert((std::is_same<decltype(*c), TInt::const_reference>::value), "");
#if 0
    // this works, but avoid clang warnings:
    //   "Expression with side effects has no effect in an unevaluated context"
    static_assert((std::is_same<decltype(*p++), TInt::reference>::value), "");
    static_assert((std::is_same<decltype(*++p), TInt::reference>::value), "");
    static_assert((std::is_same<decltype(*c++), TInt::const_reference>::value), "");
    static_assert((std::is_same<decltype(*c++), TConstInt::reference>::value), "");
    static_assert((std::is_same<decltype(*c++), TConstInt::const_reference>::value), "");
    static_assert((std::is_same<decltype(*++c), TInt::const_reference>::value), "");
    static_assert((std::is_same<decltype(*++c), TConstInt::reference>::value), "");
    static_assert((std::is_same<decltype(*++c), TConstInt::const_reference>::value), "");
#endif
    // array access
    static_assert((std::is_same<decltype(p[0]), TInt::reference>::value), "");
    static_assert((std::is_same<decltype(c[0]), TInt::const_reference>::value), "");
    static_assert((std::is_same<decltype(c[0]), TConstInt::reference>::value), "");
    static_assert((std::is_same<decltype(c[0]), TConstInt::const_reference>::value), "");
    UNUSED(p);
    UNUSED(c);
}

#endif // __cplusplus >= 201103L

/* vim:set ts=4 sw=4 et: */
