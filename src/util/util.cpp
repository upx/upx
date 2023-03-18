/* util.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2023 Laszlo Molnar
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

#include "../headers.h"
#include <algorithm>
#include "../conf.h"

#define ACC_WANT_ACC_INCI_H 1
#include "../miniacc.h"
#define ACC_WANT_ACCLIB_GETOPT 1
#define ACC_WANT_ACCLIB_HSREAD 1
#define ACC_WANT_ACCLIB_MISC 1
#define ACC_WANT_ACCLIB_VGET 1
#define ACC_WANT_ACCLIB_WILDARGV 1
#undef HAVE_MKDIR
#include "../miniacc.h"

/*************************************************************************
// assert sane memory buffer sizes to protect against integer overflows
// and malicious header fields
// see C 11 standard, Annex K
**************************************************************************/

ACC_COMPILE_TIME_ASSERT_HEADER(UPX_RSIZE_MAX_MEM == UPX_RSIZE_MAX)
ACC_COMPILE_TIME_ASSERT_HEADER(UPX_RSIZE_MAX_STR <= UPX_RSIZE_MAX / 256)
ACC_COMPILE_TIME_ASSERT_HEADER(2ull * UPX_RSIZE_MAX * 9 / 8 + 256 * 1024 * 1024 < INT_MAX)
ACC_COMPILE_TIME_ASSERT_HEADER(2ull * UPX_RSIZE_MAX * 10 / 8 + 128 * 1024 * 1024 <= INT_MAX + 1u)
ACC_COMPILE_TIME_ASSERT_HEADER(5ull * UPX_RSIZE_MAX < UINT_MAX)
ACC_COMPILE_TIME_ASSERT_HEADER(UPX_RSIZE_MAX >= 8192 * 65536)
ACC_COMPILE_TIME_ASSERT_HEADER(UPX_RSIZE_MAX_STR >= 1024)

upx_rsize_t mem_size(upx_uint64_t element_size, upx_uint64_t n, upx_uint64_t extra1,
                     upx_uint64_t extra2) {
    assert(element_size > 0);
    if very_unlikely (element_size > UPX_RSIZE_MAX)
        throwCantPack("mem_size 1; take care");
    if very_unlikely (n > UPX_RSIZE_MAX)
        throwCantPack("mem_size 2; take care");
    if very_unlikely (extra1 > UPX_RSIZE_MAX)
        throwCantPack("mem_size 3; take care");
    if very_unlikely (extra2 > UPX_RSIZE_MAX)
        throwCantPack("mem_size 4; take care");
    upx_uint64_t bytes = element_size * n + extra1 + extra2; // cannot overflow
    if very_unlikely (bytes > UPX_RSIZE_MAX)
        throwCantPack("mem_size 5; take care");
    return ACC_ICONV(upx_rsize_t, bytes);
}

bool mem_size_valid(upx_uint64_t element_size, upx_uint64_t n, upx_uint64_t extra1,
                    upx_uint64_t extra2) noexcept {
    assert(element_size > 0);
    if very_unlikely (element_size > UPX_RSIZE_MAX)
        return false;
    if very_unlikely (n > UPX_RSIZE_MAX)
        return false;
    if very_unlikely (extra1 > UPX_RSIZE_MAX)
        return false;
    if very_unlikely (extra2 > UPX_RSIZE_MAX)
        return false;
    upx_uint64_t bytes = element_size * n + extra1 + extra2; // cannot overflow
    if very_unlikely (bytes > UPX_RSIZE_MAX)
        return false;
    return true;
}

TEST_CASE("mem_size") {
    CHECK(mem_size_valid(1, 0));
    CHECK(mem_size_valid(1, 0x30000000));
    CHECK(!mem_size_valid(1, 0x30000000 + 1));
    CHECK(!mem_size_valid(1, 0x30000000, 1));
    CHECK(!mem_size_valid(1, 0x30000000, 0, 1));
    CHECK(!mem_size_valid(1, 0x30000000, 0x30000000, 0x30000000));
    CHECK_NOTHROW(mem_size(1, 0));
    CHECK_NOTHROW(mem_size(1, 0x30000000));
    CHECK_THROWS(mem_size(1, 0x30000000 + 1));
    CHECK_THROWS(mem_size(1, 0x30000000, 1));
    CHECK_THROWS(mem_size(1, 0x30000000, 0, 1));
    CHECK_THROWS(mem_size(1, 0x30000000, 0x30000000, 0x30000000));
}

/*************************************************************************
// ptr util
**************************************************************************/

int ptr_diff_bytes(const void *a, const void *b) {
    if very_unlikely (a == nullptr) {
        throwCantPack("ptr_diff_bytes null 1; take care");
    }
    if very_unlikely (b == nullptr) {
        throwCantPack("ptr_diff_bytes null 2; take care");
    }
    ptrdiff_t d = (const charptr) a - (const charptr) b;
    if (a >= b) {
        if very_unlikely (!mem_size_valid_bytes(d))
            throwCantPack("ptr_diff_bytes-1; take care");
    } else {
        if very_unlikely (!mem_size_valid_bytes(-d))
            throwCantPack("ptr_diff_bytes-2; take care");
    }
    return ACC_ICONV(int, d);
}

unsigned ptr_udiff_bytes(const void *a, const void *b) {
    int d = ptr_diff_bytes(a, b);
    if very_unlikely (d < 0)
        throwCantPack("ptr_udiff_bytes; take care");
    return ACC_ICONV(unsigned, d);
}

TEST_CASE("ptr_diff") {
    byte buf[4] = {0, 1, 2, 3};
    CHECK_THROWS(ptr_diff_bytes(nullptr, buf));
    CHECK_THROWS(ptr_diff_bytes(buf, nullptr));
    CHECK(ptr_diff(buf, buf) == 0);
    CHECK(ptr_diff(buf + 1, buf) == 1);
    CHECK(ptr_diff(buf, buf + 1) == -1);
    CHECK(ptr_udiff(buf, buf) == 0);
    CHECK(ptr_udiff(buf + 1, buf) == 1);
    CHECK_THROWS(ptr_udiff(buf, buf + 1));
}

// check that 2 buffers do not overlap; will throw on error
void uintptr_check_no_overlap(upx_uintptr_t a, size_t a_size, upx_uintptr_t b, size_t b_size) {
    if very_unlikely (a == 0 || b == 0)
        throwCantPack("ptr_check_no_overlap-nullptr");
    upx_uintptr_t a_end = a + mem_size(1, a_size);
    upx_uintptr_t b_end = b + mem_size(1, b_size);
    if very_unlikely (a_end < a || b_end < b) // wrap-around
        throwCantPack("ptr_check_no_overlap-overflow");
    // same as (!(a >= b_end || b >= a_end))
    if (a < b_end && b < a_end)
        throwCantPack("ptr_check_no_overlap-ab");
}

// check that 3 buffers do not overlap; will throw on error
void uintptr_check_no_overlap(upx_uintptr_t a, size_t a_size, upx_uintptr_t b, size_t b_size,
                              upx_uintptr_t c, size_t c_size) {
    if very_unlikely (a == 0 || b == 0 || c == 0)
        throwCantPack("ptr_check_no_overlap-nullptr");
    upx_uintptr_t a_end = a + mem_size(1, a_size);
    upx_uintptr_t b_end = b + mem_size(1, b_size);
    upx_uintptr_t c_end = c + mem_size(1, c_size);
    if very_unlikely (a_end < a || b_end < b || c_end < c) // wrap-around
        throwCantPack("ptr_check_no_overlap-overflow");
    if (a < b_end && b < a_end)
        throwCantPack("ptr_check_no_overlap-ab");
    if (a < c_end && c < a_end)
        throwCantPack("ptr_check_no_overlap-ac");
    if (b < c_end && c < b_end)
        throwCantPack("ptr_check_no_overlap-bc");
}

#if DEBUG
TEST_CASE("ptr_check_no_overlap 2") {
    byte p[4] = {};

    auto check_nothrow = [&p](int a, int as, int b, int bs) {
        CHECK_NOTHROW(ptr_check_no_overlap(p + a, as, p + b, bs)); // ab
        CHECK_NOTHROW(ptr_check_no_overlap(p + b, bs, p + a, as)); // ba
    };
    auto check_throws_ = [&p](int a, int as, int b, int bs) {
        CHECK_THROWS(ptr_check_no_overlap(p + a, as, p + b, bs)); // ab
        CHECK_THROWS(ptr_check_no_overlap(p + b, bs, p + a, as)); // ba
    };

    check_throws_(0, 1, 0, 1);
    check_nothrow(0, 1, 1, 1);
    check_throws_(0, 2, 1, 1);
    check_nothrow(0, 2, 2, 1);
    // empty buffers at edge
    check_nothrow(0, 0, 0, 0);
    check_nothrow(0, 0, 0, 1);
    check_nothrow(0, 0, 1, 0);
    // empty buffer
    check_nothrow(0, 4, 0, 0);
    check_throws_(0, 4, 1, 0);
    check_throws_(0, 4, 2, 0);
    check_throws_(0, 4, 3, 0);
    check_nothrow(0, 4, 4, 0);
}

TEST_CASE("ptr_check_no_overlap 3") {
    byte p[4] = {};

    auto check_nothrow = [&p](int a, int as, int b, int bs, int c, int cs) {
        CHECK_NOTHROW(ptr_check_no_overlap(p + a, as, p + b, bs, p + c, cs)); // abc
        CHECK_NOTHROW(ptr_check_no_overlap(p + a, as, p + c, cs, p + b, bs)); // acb
        CHECK_NOTHROW(ptr_check_no_overlap(p + b, bs, p + a, as, p + c, cs)); // bac
        CHECK_NOTHROW(ptr_check_no_overlap(p + b, bs, p + c, cs, p + a, as)); // bca
        CHECK_NOTHROW(ptr_check_no_overlap(p + c, cs, p + a, as, p + b, bs)); // cab
        CHECK_NOTHROW(ptr_check_no_overlap(p + c, cs, p + b, bs, p + a, as)); // cba
    };
    auto check_throws_ = [&p](int a, int as, int b, int bs, int c, int cs) {
        CHECK_THROWS(ptr_check_no_overlap(p + a, as, p + b, bs, p + c, cs)); // abc
        CHECK_THROWS(ptr_check_no_overlap(p + a, as, p + c, cs, p + b, bs)); // acb
        CHECK_THROWS(ptr_check_no_overlap(p + b, bs, p + a, as, p + c, cs)); // bac
        CHECK_THROWS(ptr_check_no_overlap(p + b, bs, p + c, cs, p + a, as)); // bca
        CHECK_THROWS(ptr_check_no_overlap(p + c, cs, p + a, as, p + b, bs)); // cab
        CHECK_THROWS(ptr_check_no_overlap(p + c, cs, p + b, bs, p + a, as)); // cba
    };

    check_throws_(0, 1, 0, 1, 1, 1);
    check_nothrow(0, 1, 1, 1, 2, 1);
    check_throws_(0, 2, 1, 1, 2, 1);
    check_nothrow(0, 2, 2, 1, 3, 1);
    // empty buffers at edge
    check_nothrow(0, 0, 0, 0, 0, 0);
    check_nothrow(0, 0, 0, 0, 0, 1);
    check_nothrow(0, 0, 0, 1, 1, 1);
    check_nothrow(0, 0, 1, 0, 1, 1);
    // empty buffer
    check_nothrow(0, 4, 0, 0, 0, 0);
    check_throws_(0, 4, 1, 0, 0, 0);
    check_throws_(0, 4, 2, 0, 0, 0);
    check_throws_(0, 4, 3, 0, 0, 0);
    check_nothrow(0, 4, 4, 0, 0, 0);
    // empty buffer
    check_throws_(0, 4, 0, 0, 1, 0);
    check_throws_(0, 4, 1, 0, 1, 0);
    check_throws_(0, 4, 2, 0, 1, 0);
    check_throws_(0, 4, 3, 0, 1, 0);
    check_throws_(0, 4, 4, 0, 1, 0);
}
#endif // DEBUG

/*************************************************************************
// bele.h
**************************************************************************/

namespace N_BELE_CTP {
const BEPolicy be_policy;
const LEPolicy le_policy;
} // namespace N_BELE_CTP

namespace N_BELE_RTP {
const BEPolicy be_policy;
const LEPolicy le_policy;
} // namespace N_BELE_RTP

/*************************************************************************
// stdlib
**************************************************************************/

void *upx_calloc(size_t n, size_t element_size) {
    size_t bytes = mem_size(element_size, n); // assert size
    void *p = malloc(bytes);
    if (p != nullptr)
        memset(p, 0, bytes);
    return p;
}

// simple unoptimized memswap()
void upx_memswap(void *a, void *b, size_t n) {
    if (a != b && n != 0) {
        char *x = (char *) a;
        char *y = (char *) b;
        do {
            char tmp = *x;
            *x++ = *y;
            *y++ = tmp;
        } while (--n != 0);
    }
}

// extremely simple (and beautiful) stable sort: Gnomesort
// WARNING: O(n^2) and thus very inefficient for large n
void upx_stable_sort(void *array, size_t n, size_t element_size,
                     int (*compare)(const void *, const void *)) {
    for (size_t i = 1; i < n; i++) {
        char *a = (char *) array + element_size * i; // a = &array[i]
        if (i != 0 && compare(a - element_size, a) > 0) {
            upx_memswap(a - element_size, a, element_size); // swap elements a[-1] <=> a[0]
            i -= 2;
        }
    }
}

#if DEBUG
TEST_CASE("upx_stable_sort") {
    {
        unsigned a[] = {0, 1};
        upx_stable_sort(a, 2, sizeof(*a), ne32_compare);
        CHECK((a[0] == 0 && a[1] == 1));
    }
    {
        unsigned a[] = {1, 0};
        upx_stable_sort(a, 2, sizeof(*a), ne32_compare);
        CHECK((a[0] == 0 && a[1] == 1));
    }
    {
        unsigned a[] = {2, 1, 0};
        upx_stable_sort(a, 3, sizeof(*a), ne32_compare);
        CHECK((a[0] == 0 && a[1] == 1 && a[2] == 2));
    }
#if __cplusplus >= 202002L // use C++20 std::next_permutation() to test all permutations
    {
        upx_uint16_t perm[5] = {0, 1, 2, 3, 4}; // 120 permutations
        do {
            upx_uint16_t a[5] = {};
            memcpy(a, perm, sizeof(perm));
            upx_stable_sort(a, 5, sizeof(*a), ne16_compare);
            CHECK((a[0] == 0 && a[1] == 1 && a[2] == 2 && a[3] == 3 && a[4] == 4));
        } while (std::next_permutation(perm, perm + 5));
    }
#endif
}
#endif // DEBUG

/*************************************************************************
// qsort() util
**************************************************************************/

int __acc_cdecl_qsort be16_compare(const void *e1, const void *e2) {
    const unsigned d1 = get_be16(e1);
    const unsigned d2 = get_be16(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort be24_compare(const void *e1, const void *e2) {
    const unsigned d1 = get_be24(e1);
    const unsigned d2 = get_be24(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort be32_compare(const void *e1, const void *e2) {
    const unsigned d1 = get_be32(e1);
    const unsigned d2 = get_be32(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort be64_compare(const void *e1, const void *e2) {
    const upx_uint64_t d1 = get_be64(e1);
    const upx_uint64_t d2 = get_be64(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort le16_compare(const void *e1, const void *e2) {
    const unsigned d1 = get_le16(e1);
    const unsigned d2 = get_le16(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort le24_compare(const void *e1, const void *e2) {
    const unsigned d1 = get_le24(e1);
    const unsigned d2 = get_le24(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort le32_compare(const void *e1, const void *e2) {
    const unsigned d1 = get_le32(e1);
    const unsigned d2 = get_le32(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort le64_compare(const void *e1, const void *e2) {
    const upx_uint64_t d1 = get_le64(e1);
    const upx_uint64_t d2 = get_le64(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort be16_compare_signed(const void *e1, const void *e2) {
    const int d1 = get_be16_signed(e1);
    const int d2 = get_be16_signed(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort be24_compare_signed(const void *e1, const void *e2) {
    const int d1 = get_be24_signed(e1);
    const int d2 = get_be24_signed(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort be32_compare_signed(const void *e1, const void *e2) {
    const int d1 = get_be32_signed(e1);
    const int d2 = get_be32_signed(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort be64_compare_signed(const void *e1, const void *e2) {
    const upx_int64_t d1 = get_be64_signed(e1);
    const upx_int64_t d2 = get_be64_signed(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort le16_compare_signed(const void *e1, const void *e2) {
    const int d1 = get_le16_signed(e1);
    const int d2 = get_le16_signed(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort le24_compare_signed(const void *e1, const void *e2) {
    const int d1 = get_le24_signed(e1);
    const int d2 = get_le24_signed(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort le32_compare_signed(const void *e1, const void *e2) {
    const int d1 = get_le32_signed(e1);
    const int d2 = get_le32_signed(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort le64_compare_signed(const void *e1, const void *e2) {
    const upx_int64_t d1 = get_le64_signed(e1);
    const upx_int64_t d2 = get_le64_signed(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

/*************************************************************************
// find and mem_replace util
**************************************************************************/

int find(const void *buf, int blen, const void *what, int wlen) {
    // nullptr is explicitly allowed here
    if (buf == nullptr || blen < wlen || what == nullptr || wlen <= 0)
        return -1;

    const byte *b = (const byte *) buf;
    byte first_byte = *(const byte *) what;

    blen -= wlen;
    for (int i = 0; i <= blen; i++, b++)
        if (*b == first_byte && memcmp(b, what, wlen) == 0)
            return i;

    return -1;
}

int find_be16(const void *b, int blen, unsigned what) {
    byte w[2];
    set_be16(w, what);
    return find(b, blen, w, 2);
}

int find_be32(const void *b, int blen, unsigned what) {
    byte w[4];
    set_be32(w, what);
    return find(b, blen, w, 4);
}

int find_be64(const void *b, int blen, upx_uint64_t what) {
    byte w[8];
    set_be64(w, what);
    return find(b, blen, w, 8);
}

int find_le16(const void *b, int blen, unsigned what) {
    byte w[2];
    set_le16(w, what);
    return find(b, blen, w, 2);
}

int find_le32(const void *b, int blen, unsigned what) {
    byte w[4];
    set_le32(w, what);
    return find(b, blen, w, 4);
}

int find_le64(const void *b, int blen, upx_uint64_t what) {
    byte w[8];
    set_le64(w, what);
    return find(b, blen, w, 8);
}

TEST_CASE("find") {
    CHECK(find(nullptr, -1, nullptr, -1) == -1);
    static const byte b[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    CHECK(find(b, 16, b, 0) == -1);
    for (int i = 0; i < 16; i++) {
        CHECK(find(b, 16, b + i, 1) == i);
    }
    for (int i = 1; i <= 16; i++) {
        CHECK(find(b, 16, b, i) == 0);
    }
    CHECK(find(b, 16, b, 17) == -1);
    CHECK(find_be16(b, 16, 0x0203) == 2);
    CHECK(find_le16(b, 16, 0x0302) == 2);
    CHECK(find_be32(b, 16, 0x04050607) == 4);
    CHECK(find_le32(b, 16, 0x07060504) == 4);
    CHECK(find_be64(b, 16, 0x08090a0b0c0d0e0fULL) == 8);
    CHECK(find_le64(b, 16, 0x0f0e0d0c0b0a0908ULL) == 8);
    CHECK(find_be64(b, 15, 0x08090a0b0c0d0e0fULL) == -1);
    CHECK(find_le64(b, 15, 0x0f0e0d0c0b0a0908ULL) == -1);
}

int mem_replace(void *buf, int blen, const void *what, int wlen, const void *replacement) {
    byte *b = (byte *) buf;
    int boff = 0;
    int n = 0;

    while (blen - boff >= wlen) {
        int off = find(b + boff, blen - boff, what, wlen);
        if (off < 0)
            break;
        boff += off;
        memcpy(b + boff, replacement, wlen);
        boff += wlen;
        n++;
    }
    return n;
}

TEST_CASE("mem_replace") {
    char b[16 + 1] = "aaaaaaaaaaaaaaaa";
    CHECK(mem_replace(b, 16, "a", 0, "x") == 0);
    CHECK(mem_replace(b, 16, "a", 1, "b") == 16);
    CHECK(mem_replace(b, 8, "bb", 2, "cd") == 4);
    CHECK(mem_replace(b + 8, 8, "bbb", 3, "efg") == 2);
    CHECK(mem_replace(b, 16, "b", 1, "h") == 2);
    CHECK(strcmp(b, "cdcdcdcdefgefghh") == 0);
}

/*************************************************************************
// fn - FileName util
**************************************************************************/

#if (ACC_OS_CYGWIN || ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_EMX || ACC_OS_OS2 || ACC_OS_OS16 ||   \
     ACC_OS_TOS || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)

static const char dir_sep[] = "/\\";
#define fn_is_drive(s) (s[0] && s[1] == ':')
#define fn_is_sep(c) (strchr(dir_sep, c) != nullptr)
#define fn_skip_drive(s) (fn_is_drive(s) ? (s) + 2 : (s))
#define fn_tolower(c) (tolower(((uchar) (c))))

#else

// static const char dir_sep[] = "/";
#define fn_is_drive(s) (0)
#define fn_is_sep(c) ((c) == '/')
#define fn_skip_drive(s) (s)
#define fn_tolower(c) (c)

#endif

char *fn_basename(const char *name) {
    const char *n, *nn;

    name = fn_skip_drive(name);
    for (nn = n = name; *nn; nn++)
        if (fn_is_sep(*nn))
            n = nn + 1;
    return ACC_UNCONST_CAST(char *, n);
}

bool fn_has_ext(const char *name, const char *ext, bool ignore_case) {
    const char *n, *e;

    name = fn_basename(name);
    for (n = e = name; *n; n++)
        if (*n == '.')
            e = n;
    if (ignore_case)
        return (strcasecmp(ext, e + 1) == 0);
    else
        return (fn_strcmp(ext, e + 1) == 0);
}

char *fn_strlwr(char *n) {
    char *p;
    for (p = n; *p; p++)
        *p = (char) fn_tolower(*p);
    return n;
}

int fn_strcmp(const char *n1, const char *n2) {
    for (;;) {
        if (*n1 != *n2) {
            int c = fn_tolower(*n1) - fn_tolower(*n2);
            if (c)
                return c;
        }
        if (*n1 == 0)
            return 0;
        n1++;
        n2++;
    }
}

/*************************************************************************
// misc.
**************************************************************************/

bool set_method_name(char *buf, size_t size, int method, int level) {
    bool r = true;
    const char *alg;
    if (M_IS_NRV2B(method))
        alg = "NRV2B";
    else if (M_IS_NRV2D(method))
        alg = "NRV2D";
    else if (M_IS_NRV2E(method))
        alg = "NRV2E";
    else if (M_IS_LZMA(method))
        alg = "LZMA";
    else {
        alg = "???";
        r = false;
    }
    if (level > 0)
        upx_safe_snprintf(buf, size, "%s/%d", alg, level);
    else
        upx_safe_snprintf(buf, size, "%s", alg);
    return r;
}

void center_string(char *buf, size_t size, const char *s) {
    size_t l1 = size - 1;
    size_t l2 = strlen(s);
    assert(size > 0);
    assert(l2 < size);
    memset(buf, ' ', l1);
    memcpy(buf + (l1 - l2) / 2, s, l2);
    buf[l1] = 0;
}

bool file_exists(const char *name) {
    int fd, r;
    struct stat st;

    /* return true if we can open it */
    fd = open(name, O_RDONLY | O_BINARY, 0);
    if (fd >= 0) {
        (void) close(fd);
        return true;
    }

    /* return true if we can stat it */
    // memset(&st, 0, sizeof(st));
    r = stat(name, &st);
    if (r != -1)
        return true;

/* return true if we can lstat it */
#if (HAVE_LSTAT)
    // memset(&st, 0, sizeof(st));
    r = lstat(name, &st);
    if (r != -1)
        return true;
#endif

    return false;
}

bool maketempname(char *ofilename, size_t size, const char *ifilename, const char *ext,
                  bool force) {
    char *ofext = nullptr, *ofname;
    int ofile;

    if (size <= 0)
        return false;

    strcpy(ofilename, ifilename);
    for (ofname = fn_basename(ofilename); *ofname; ofname++) {
        if (*ofname == '.')
            ofext = ofname;
    }
    if (ofext == nullptr)
        ofext = ofilename + strlen(ofilename);
    strcpy(ofext, ext);

    for (ofile = 0; ofile < 1000; ofile++) {
        assert(strlen(ofilename) < size);
        if (!file_exists(ofilename))
            return true;
        if (!force)
            break;
        upx_safe_snprintf(ofext, 5, ".%03d", ofile);
    }

    ofilename[0] = 0;
    return false;
}

bool makebakname(char *ofilename, size_t size, const char *ifilename, bool force) {
    char *ofext = nullptr, *ofname;
    int ofile;

    if (size <= 0)
        return false;

    strcpy(ofilename, ifilename);
    for (ofname = fn_basename(ofilename); *ofname; ofname++) {
        if (*ofname == '.')
            ofext = ofname;
    }
    if (ofext == nullptr) {
        ofext = ofilename + strlen(ofilename);
        strcpy(ofext, ".~");
    } else if (strlen(ofext) < 1 + 3)
        strcat(ofilename, "~");
    else
        ofext[strlen(ofext) - 1] = '~';

    for (ofile = 0; ofile < 1000; ofile++) {
        assert(strlen(ofilename) < size);
        if (!file_exists(ofilename))
            return true;
        if (!force)
            break;
        upx_safe_snprintf(ofext, 5, ".%03d", ofile);
    }

    ofilename[0] = 0;
    return false;
}

/*************************************************************************
// return compression ratio, where 100% == 1000*1000 == 1e6
**************************************************************************/

unsigned get_ratio(upx_uint64_t u_len, upx_uint64_t c_len) {
    const unsigned n = 1000 * 1000;
    if (u_len == 0)
        return c_len == 0 ? 0 : n;
    upx_uint64_t x = c_len * n;
    assert(x / n == c_len);
    x /= u_len;
    x += 50;         // rounding
    if (x >= 10 * n) // >= "1000%"
        x = 10 * n - 1;
    return ACC_ICONV(unsigned, x);
}

TEST_CASE("get_ratio") {
    CHECK(get_ratio(0, 0) == 0);
    CHECK(get_ratio(0, 1) == 1000000);
    CHECK(get_ratio(1, 0) == 50);
    CHECK(get_ratio(1, 1) == 1000050);
    CHECK(get_ratio(1, 9) == 9000050);
    CHECK(get_ratio(1, 10) == 9999999);
    CHECK(get_ratio(1, 11) == 9999999);
    CHECK(get_ratio(100000, 100000) == 1000050);
    CHECK(get_ratio(100000, 200000) == 2000050);
    CHECK(get_ratio(UPX_RSIZE_MAX, UPX_RSIZE_MAX) == 1000050);
    CHECK(get_ratio(2 * UPX_RSIZE_MAX, 2 * UPX_RSIZE_MAX) == 1000050);
    CHECK(get_ratio(2 * UPX_RSIZE_MAX, 1024ull * UPX_RSIZE_MAX) == 9999999);
}

/* vim:set ts=4 sw=4 et: */
