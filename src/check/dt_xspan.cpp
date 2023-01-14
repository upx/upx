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
// basic
**************************************************************************/

TEST_CASE("basic xspan usage") {
    char buf[4] = {0, 1, 2, 3};

    SUBCASE("SPAN_x") {
        SPAN_0(char) a0 = nullptr;

        SPAN_0(char) b0 = buf;
        SPAN_P(char) bp = buf;

        SPAN_0(char) c0 = SPAN_0_MAKE(char, buf);
        SPAN_P(char) cp = SPAN_P_MAKE(char, buf);
        SPAN_S(char) cs = SPAN_S_MAKE(char, buf, sizeof(buf));

        SPAN_0(const char) const x0 = SPAN_0_MAKE(const char, buf);
        SPAN_P(const char) const xp = SPAN_P_MAKE(const char, buf);
        SPAN_S(const char) const xs = SPAN_S_MAKE(const char, buf, sizeof(buf));
        SPAN_P(const char) const yp = xs;
        SPAN_0(const char) const z0p = yp;
        SPAN_0(const char) const z0s = xs;

        CHECK((a0 == nullptr));
        CHECK(c0 == b0);
        CHECK(cp == bp);
        CHECK(cs == bp);
        CHECK(x0 == z0p);
        CHECK(xp == z0s);

        CHECK(raw_bytes(c0, 4) == buf);
        CHECK(raw_index_bytes(c0, 1, 3) == buf + 1);
        CHECK(raw_bytes(cp, 4) == buf);
        CHECK(raw_index_bytes(cp, 1, 3) == buf + 1);
        CHECK(raw_bytes(cs, 4) == buf);
        CHECK(raw_index_bytes(cs, 1, 3) == buf + 1);
        CHECK_THROWS(raw_bytes(cs, 5));
        CHECK_THROWS(raw_index_bytes(cs, 1, 4));
    }

    SUBCASE("SPAN_x_VAR") {
        SPAN_0_VAR(char, a0, nullptr);

        SPAN_0_VAR(char, b0, buf);
        SPAN_P_VAR(char, bp, buf);

        SPAN_0_VAR(char, c0, buf, sizeof(buf));
        SPAN_P_VAR(char, cp, buf, sizeof(buf));
        SPAN_S_VAR(char, cs, buf, sizeof(buf));

        SPAN_0_VAR(char, d0, buf + 1, sizeof(buf), buf);
        SPAN_P_VAR(char, dp, buf + 1, sizeof(buf), buf);
        SPAN_S_VAR(char, ds, buf + 1, sizeof(buf), buf);

        SPAN_0_VAR(const char, const x0, buf, sizeof(buf));
        SPAN_P_VAR(const char, const xp, buf, sizeof(buf));
        SPAN_S_VAR(const char, const xs, buf, sizeof(buf));
        SPAN_P_VAR(const char, const yp, xs);
        SPAN_0_VAR(const char, const z0p, yp);
        SPAN_0_VAR(const char, const z0s, xs);

        CHECK((a0 == nullptr));
        CHECK(c0 == b0);
        CHECK(cp == bp);
        CHECK(cs == bp);
        CHECK(d0 == dp);
        CHECK(d0 == ds);
        CHECK(x0 == z0p);
        CHECK(xp == z0s);

        CHECK(raw_bytes(c0, 4) == buf);
        CHECK(raw_index_bytes(c0, 1, 3) == buf + 1);
        CHECK(raw_bytes(cp, 4) == buf);
        CHECK(raw_index_bytes(cp, 1, 3) == buf + 1);
        CHECK(raw_bytes(cs, 4) == buf);
        CHECK(raw_index_bytes(cs, 1, 3) == buf + 1);
        CHECK_THROWS(raw_bytes(cs, 5));
        CHECK_THROWS(raw_index_bytes(cs, 1, 4));
    }

    SUBCASE("xspan in class") {
        struct MyType {
            SPAN_0(char) s0;
            SPAN_P(char) sp;
            SPAN_S(char) ss;
#if __cplusplus >= 201103L
            SPAN_0(char) x0 = nullptr;
#endif
#if WITH_SPAN >= 2
            // much nicer syntax when using fully checked xspan:
            MyType(char *b, size_t n, bool) : s0(b, n), sp(b, n), ss(b, n) {}
#endif
            MyType(char *b, size_t n)
                : s0(SPAN_0_MAKE(char, b, n)), sp(SPAN_P_MAKE(char, b, n)),
                  ss(SPAN_S_MAKE(char, b, n)) {
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

#if (WITH_SPAN >= 2) && DEBUG

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
#if SPAN_CONFIG_ENABLE_SPAN_CONVERSION
    typedef PtrOrSpanOrNull<char> Span0;
    Span0 s0_no_base(nullptr);
    Span0 s0_with_base(nullptr, 4, base_buf);
    CHECK_THROWS(sp_no_base = s0_no_base);     // nullptr assignment
    CHECK_THROWS(sp_no_base = s0_with_base);   // nullptr assignment
    CHECK_THROWS(sp_with_base = s0_no_base);   // nullptr assignment
    CHECK_THROWS(sp_with_base = s0_with_base); // nullptr assignment
#endif
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
#if SPAN_CONFIG_ENABLE_SPAN_CONVERSION
    {
        typedef PtrOrSpanOrNull<char> Span0;
        // v0 nullptr, b0 base, b1 base + 1
        const Span0 v0_v0(nullptr);
        const Span0 v0_b0(nullptr, 4, base_buf);
        const Span0 v0_b1(nullptr, 3, base_buf + 1);
        const Span0 b0_v0(base_buf);
        const Span0 b0_b0(base_buf, 4, base_buf);
        CHECK_THROWS(SPAN_0_MAKE(char, base_buf, 3, base_buf + 1)); // b0_b1
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
        CHECK_THROWS(SPAN_S_MAKE(char, v0_v0));
        CHECK_THROWS(SPAN_S_MAKE(char, v0_b0));
        CHECK_THROWS(SPAN_S_MAKE(char, v0_b1));
        CHECK_THROWS(SPAN_S_MAKE(char, b0_v0));
        CHECK_NOTHROW(SPAN_S_MAKE(char, b0_b0));
        CHECK_THROWS(SPAN_S_MAKE(char, b1_v0));
        CHECK_NOTHROW(SPAN_S_MAKE(char, b1_b0));
        CHECK_NOTHROW(SPAN_S_MAKE(char, b1_b1));
        //
        CHECK((SPAN_S_MAKE(char, b0_b0).raw_base() == base_buf));
        CHECK((SPAN_S_MAKE(char, b1_b0).raw_base() == base_buf));
        CHECK((SPAN_S_MAKE(char, b1_b1).raw_base() == base_buf + 1));
    }
    {
        typedef PtrOrSpan<char> SpanP;
        // v0 nullptr, b0 base, b1 base + 1
        const SpanP b0_v0(base_buf);
        const SpanP b0_b0(base_buf, 4, base_buf);
        CHECK_THROWS(SPAN_P_MAKE(char, base_buf, 3, base_buf + 1)); // b0_b1
        const SpanP b1_v0(base_buf + 1);
        const SpanP b1_b0(base_buf + 1, 4, base_buf);
        const SpanP b1_b1(base_buf + 1, 3, base_buf + 1);
        CHECK_NOTHROW(ss_with_base = b0_v0);
        CHECK_NOTHROW(ss_with_base = b0_b0);
        CHECK_NOTHROW(ss_with_base = b1_v0);
        CHECK_NOTHROW(ss_with_base = b1_b0);
        CHECK_THROWS(ss_with_base = b1_b1); // different base
        CHECK_THROWS(SPAN_S_MAKE(char, b0_v0));
        CHECK_NOTHROW(SPAN_S_MAKE(char, b0_b0));
        CHECK_THROWS(SPAN_S_MAKE(char, b1_v0));
        CHECK_NOTHROW(SPAN_S_MAKE(char, b1_b0));
        CHECK_NOTHROW(SPAN_S_MAKE(char, b1_b1));
        //
        CHECK((SPAN_S_MAKE(char, b0_b0).raw_base() == base_buf));
        CHECK((SPAN_S_MAKE(char, b1_b0).raw_base() == base_buf));
        CHECK((SPAN_S_MAKE(char, b1_b1).raw_base() == base_buf + 1));
    }
#endif
}

/*************************************************************************
//
**************************************************************************/

TEST_CASE("Span void ptr") {
    static char a[4] = {0, 1, 2, 3};
    SPAN_0(void) a0(a, 4);
    SPAN_P(void) ap(a, 4);
    SPAN_S(void) as(a, 4);
    SPAN_0(const void) c0(a, 4);
    SPAN_P(const void) cp(a, 4);
    SPAN_S(const void) cs(a, 4);
    static const char b[4] = {0, 1, 2, 3};
    SPAN_0(const void) b0(b, 4);
    SPAN_P(const void) bp(b, 4);
    SPAN_S(const void) bs(b, 4);
}

TEST_CASE("Span deref/array/arrow") {
    static char real_a[2 + 4 + 2] = {126, 127, 0, 1, 2, 3, 124, 125};
    static char *a = real_a + 2;
    SPAN_0(char) a0(a, 4);
    SPAN_P(char) ap(a, 4);
    SPAN_S(char) as(a, 4);
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
    SPAN_S(char) as(buf, 4);
    CHECK(as.subspan(1, 1)[0] == 1);
    CHECK((as + 1).subspan(1, 1)[0] == 2);
    CHECK((as + 2).subspan(0, -2)[0] == 0);
    CHECK_THROWS(as.subspan(1, 0)[0]);
    CHECK_THROWS(as.subspan(1, 1)[-1]);
}

TEST_CASE("Span constness") {
    static char buf[4] = {0, 1, 2, 3};

    SPAN_0(char) b0(buf, 4);
    SPAN_P(char) bp(buf, 4);
    SPAN_S(char) bs(buf, 4);

    SPAN_0(char) s0(b0);
    SPAN_P(char) sp(bp);
    SPAN_S(char) ss(bs);

    SPAN_0(const char) b0c(buf, 4);
    SPAN_P(const char) bpc(buf, 4);
    SPAN_S(const char) bsc(buf, 4);

    SPAN_0(const char) s0c(b0c);
    SPAN_P(const char) spc(bpc);
    SPAN_S(const char) ssc(bsc);

    SPAN_0(const char) x0c(b0);
    SPAN_P(const char) xpc(bp);
    SPAN_S(const char) xsc(bs);

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
int my_memcmp_v1(SPAN_P(const void) a, SPAN_0(const void) b, size_t n) {
    if (b == nullptr)
        return -2;
    SPAN_0(const void) x(a);
    return memcmp(x, b, n);
}
int my_memcmp_v2(SPAN_P(const char) a, SPAN_0(const char) b, size_t n) {
    if (a == b)
        return 0;
    if (b == nullptr)
        return -2;
    a += 1;
    b -= 1;
    SPAN_0(const char) x(a);
    SPAN_0(const char) y = b;
    return memcmp(x, y, n);
}
} // namespace
#endif

TEST_CASE("PtrOrSpan") {
    static const char buf[4] = {0, 1, 2, 3};
    CHECK(my_memcmp_v1(buf, nullptr, 4) == -2);
    CHECK(my_memcmp_v2(buf + 4, buf + 4, 999) == 0);
    CHECK(my_memcmp_v2(buf, buf + 2, 3) == 0);
}

/*************************************************************************
//
**************************************************************************/

TEST_CASE("PtrOrSpan char") {
    char real_buf[2 + 8 + 2] = {126, 127, 0, 1, 2, 3, 4, 5, 6, 7, 124, 125};
    char *buf = real_buf + 2;
    SPAN_P(char) a(buf, SpanSizeInBytes(8));
    SPAN_P(char) b = a.subspan(0, 7);
    SPAN_P(char) c = (b + 1).subspan(0, 6);
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
    int buf[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    SPAN_P(int) a(buf, SpanCount(8));
    CHECK(a.raw_size_in_bytes() == 8 * sizeof(int));
    SPAN_P(int) b = a.subspan(0, 7);
    CHECK(b.raw_size_in_bytes() == 7 * sizeof(int));
    SPAN_P(int) c = (b + 1).subspan(0, 6);
    CHECK(c.raw_size_in_bytes() == 6 * sizeof(int));
    a += 1;
    CHECK(*a == 1);
    CHECK(*a++ == 1);
    CHECK(*++a == 3);
    CHECK(--*a == 2);
    CHECK(*a-- == 2);
    CHECK(*b == 0);
    CHECK(*c == 1);
    a = buf + 7;
#ifdef UPX_VERSION_HEX
    CHECK(get_le32(a) == ne32_to_le32(7));
#endif
    a++;
#ifdef UPX_VERSION_HEX
    CHECK_THROWS(get_le32(a));
#endif
    CHECK_THROWS(raw_bytes(a, 1));
}

/*************************************************************************
// codegen
**************************************************************************/

namespace {
template <class T>
__acc_static_noinline int foo(T p) {
    unsigned r = 0;
    r += *p++;
    r += *++p;
    p += 3;
    r += *p;
    return r;
}

template <class T>
SPAN_0(T)
make_span_0(T *ptr, size_t count) {
    return PtrOrSpanOrNull<T>(ptr, count);
}
template <class T>
SPAN_P(T)
make_span_p(T *ptr, size_t count) {
    return PtrOrSpan<T>(ptr, count);
}
template <class T>
SPAN_S(T)
make_span_s(T *ptr, size_t count) {
    return Span<T>(ptr, count);
}
} // namespace

TEST_CASE("Span codegen") {
    char buf[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    CHECK(foo(buf) == 0 + 2 + 5);
    CHECK(foo(make_span_0(buf, 8)) == 0 + 2 + 5);
    CHECK(foo(make_span_p(buf, 8)) == 0 + 2 + 5);
    CHECK(foo(make_span_s(buf, 8)) == 0 + 2 + 5);
}

#endif // WITH_SPAN >= 2

/* vim:set ts=4 sw=4 et: */
