/* util.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2020 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2020 Laszlo Molnar
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

#include "conf.h"
#include "util.h"

#if (ACC_CC_MSC && (_MSC_VER >= 1000 && _MSC_VER < 1200))
/* avoid -W4 warnings in <conio.h> */
#pragma warning(disable : 4032)
/* avoid -W4 warnings in <windows.h> */
#pragma warning(disable : 4201 4214 4514)
#endif
#define ACC_WANT_ACC_INCI_H 1
#include "miniacc.h"
#define ACC_WANT_ACCLIB_GETOPT 1
#define ACC_WANT_ACCLIB_HSREAD 1
#define ACC_WANT_ACCLIB_MISC 1
#define ACC_WANT_ACCLIB_UA 1
#define ACC_WANT_ACCLIB_WILDARGV 1
#undef HAVE_MKDIR
#include "miniacc.h"

/*************************************************************************
// assert sane memory buffer sizes to protect against integer overflows
// and malicious header fields
**************************************************************************/

ACC_COMPILE_TIME_ASSERT_HEADER(UPX_RSIZE_MAX_MEM == UPX_RSIZE_MAX)
ACC_COMPILE_TIME_ASSERT_HEADER(UPX_RSIZE_MAX_STR <= UPX_RSIZE_MAX / 256)
ACC_COMPILE_TIME_ASSERT_HEADER(2ull * UPX_RSIZE_MAX * 9 / 8 + 16 * 1024 * 1024 < INT_MAX)

upx_rsize_t mem_size(upx_uint64_t element_size, upx_uint64_t n, upx_uint64_t extra1,
                     upx_uint64_t extra2) {
    assert(element_size > 0);
    if (element_size > UPX_RSIZE_MAX)
        throwCantPack("mem_size 1; take care");
    if (n > UPX_RSIZE_MAX)
        throwCantPack("mem_size 2; take care");
    if (extra1 > UPX_RSIZE_MAX)
        throwCantPack("mem_size 3; take care");
    if (extra2 > UPX_RSIZE_MAX)
        throwCantPack("mem_size 4; take care");
    upx_uint64_t bytes = element_size * n + extra1 + extra2; // cannot overflow
    if (bytes > UPX_RSIZE_MAX)
        throwCantPack("mem_size 5; take care");
    return ACC_ICONV(upx_rsize_t, bytes);
}

upx_rsize_t mem_size_get_n(upx_uint64_t element_size, upx_uint64_t n) {
    mem_size_assert(element_size, n);
    return ACC_ICONV(upx_rsize_t, n); // return n
}

bool mem_size_valid(upx_uint64_t element_size, upx_uint64_t n, upx_uint64_t extra1,
                    upx_uint64_t extra2) {
    assert(element_size > 0);
    if (element_size > UPX_RSIZE_MAX)
        return false;
    if (n > UPX_RSIZE_MAX)
        return false;
    if (extra1 > UPX_RSIZE_MAX)
        return false;
    if (extra2 > UPX_RSIZE_MAX)
        return false;
    upx_uint64_t bytes = element_size * n + extra1 + extra2; // cannot overflow
    if (bytes > UPX_RSIZE_MAX)
        return false;
    return true;
}

bool mem_size_valid_bytes(upx_uint64_t bytes) {
    if (bytes > UPX_RSIZE_MAX)
        return false;
    return true;
}

int ptr_diff(const void *p1, const void *p2) {
    assert(p1 != NULL);
    assert(p2 != NULL);
    ptrdiff_t d = (const char *) p1 - (const char *) p2;
    if (p1 >= p2)
        assert(mem_size_valid_bytes(d));
    else
        assert(mem_size_valid_bytes(-d));
    return ACC_ICONV(int, d);
}

unsigned ptr_udiff(const void *p1, const void *p2) {
    int d = ptr_diff(p1, p2);
    assert(d >= 0);
    return ACC_ICONV(unsigned, d);
}

void mem_clear(void *p, size_t n) {
    mem_size_assert(1, n);
    memset(p, 0, n);
}

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
// find util
**************************************************************************/

int find(const void *b, int blen, const void *what, int wlen) {
    if (b == NULL || blen <= 0 || what == NULL || wlen <= 0)
        return -1;

    int i;
    const unsigned char *base = (const unsigned char *) b;
    unsigned char firstc = *(const unsigned char *) what;

    blen -= wlen;
    for (i = 0; i <= blen; i++, base++)
        if (*base == firstc && memcmp(base, what, wlen) == 0)
            return i;

    return -1;
}

int find_be16(const void *b, int blen, unsigned what) {
    unsigned char w[2];
    set_be16(w, what);
    return find(b, blen, w, 2);
}

int find_be32(const void *b, int blen, unsigned what) {
    unsigned char w[4];
    set_be32(w, what);
    return find(b, blen, w, 4);
}

int find_be64(const void *b, int blen, upx_uint64_t what) {
    unsigned char w[8];
    set_be64(w, what);
    return find(b, blen, w, 8);
}

int find_le16(const void *b, int blen, unsigned what) {
    unsigned char w[2];
    set_le16(w, what);
    return find(b, blen, w, 2);
}

int find_le32(const void *b, int blen, unsigned what) {
    unsigned char w[4];
    set_le32(w, what);
    return find(b, blen, w, 4);
}

int find_le64(const void *b, int blen, upx_uint64_t what) {
    unsigned char w[8];
    set_le64(w, what);
    return find(b, blen, w, 8);
}

int mem_replace(void *bb, int blen, const void *what, int wlen, const void *r) {
    unsigned char *b = (unsigned char *) bb;
    int boff = 0;
    int n = 0;

    while (blen - boff >= wlen) {
        int off = find(b + boff, blen - boff, what, wlen);
        if (off < 0)
            break;
        boff += off;
        memcpy(b + boff, r, wlen);
        boff += wlen;
        n++;
    }
    return n;
}

/*************************************************************************
// fn - FileName util
**************************************************************************/

#if (ACC_OS_CYGWIN || ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_EMX || ACC_OS_OS2 || ACC_OS_OS16 ||   \
     ACC_OS_TOS || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)

static const char dir_sep[] = "/\\";
#define fn_is_drive(s) (s[0] && s[1] == ':')
#define fn_is_sep(c) (strchr(dir_sep, c) != NULL)
#define fn_skip_drive(s) (fn_is_drive(s) ? (s) + 2 : (s))
#define fn_tolower(c) (tolower(((unsigned char) (c))))

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
        upx_snprintf(buf, size, "%s/%d", alg, level);
    else
        upx_snprintf(buf, size, "%s", alg);
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
    char *ofext = NULL, *ofname;
    int ofile;

    if (size <= 0)
        return false;

    strcpy(ofilename, ifilename);
    for (ofname = fn_basename(ofilename); *ofname; ofname++) {
        if (*ofname == '.')
            ofext = ofname;
    }
    if (ofext == NULL)
        ofext = ofilename + strlen(ofilename);
    strcpy(ofext, ext);

    for (ofile = 0; ofile < 1000; ofile++) {
        assert(strlen(ofilename) < size);
        if (!file_exists(ofilename))
            return true;
        if (!force)
            break;
        upx_snprintf(ofext, 5, ".%03d", ofile);
    }

    ofilename[0] = 0;
    return false;
}

bool makebakname(char *ofilename, size_t size, const char *ifilename, bool force) {
    char *ofext = NULL, *ofname;
    int ofile;

    if (size <= 0)
        return false;

    strcpy(ofilename, ifilename);
    for (ofname = fn_basename(ofilename); *ofname; ofname++) {
        if (*ofname == '.')
            ofext = ofname;
    }
    if (ofext == NULL) {
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
        upx_snprintf(ofext, 5, ".%03d", ofile);
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

/*************************************************************************
// Don't link these functions from libc ==> save xxx bytes
**************************************************************************/

extern "C" {

// FIXME - quick hack for arm-wince-gcc-3.4 (Debian pocketpc-*.deb packages)
#if 1 && (ACC_ARCH_ARM) && defined(__pe__) && !defined(__CEGCC__) && !defined(_WIN32)
int dup(int fd) {
    UNUSED(fd);
    return -1;
}
#endif

#if (ACC_OS_DOS32) && defined(__DJGPP__)
// int _is_executable(const char *, int, const char *) { return 0; }

// FIXME: something wants to link in ctime.o
// time_t mktime(struct tm *) { return 0; }

// time_t time(time_t *t) { if (t) *t = 0; return 0; }
#endif

} // extern "C"

/* vim:set ts=4 sw=4 et: */
