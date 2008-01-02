/* util.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
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
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
 */


#include "conf.h"
#include "util.h"

#if (ACC_CC_MSC && (_MSC_VER >= 1000 && _MSC_VER < 1200))
   /* avoid -W4 warnings in <conio.h> */
#  pragma warning(disable: 4032)
   /* avoid -W4 warnings in <windows.h> */
#  pragma warning(disable: 4201 4214 4514)
#endif
#define ACC_WANT_ACC_INCI_H 1
#define ACC_WANT_ACCLIB_HSREAD 1
#define ACC_WANT_ACCLIB_MISC 1
#define ACC_WANT_ACCLIB_UA 1
#define ACC_WANT_ACCLIB_WILDARGV 1
#include "miniacc.h"


/*************************************************************************
// bele.h
**************************************************************************/

namespace N_BELE_CTP {
const BEPolicy be_policy;
const LEPolicy le_policy;
}

namespace N_BELE_RTP {
const BEPolicy be_policy;
const LEPolicy le_policy;
}


/*************************************************************************
// qsort() util
**************************************************************************/

int __acc_cdecl_qsort be16_compare(const void *e1, const void *e2)
{
    const unsigned d1 = get_be16(e1);
    const unsigned d2 = get_be16(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort be24_compare(const void *e1, const void *e2)
{
    const unsigned d1 = get_be24(e1);
    const unsigned d2 = get_be24(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort be32_compare(const void *e1, const void *e2)
{
    const unsigned d1 = get_be32(e1);
    const unsigned d2 = get_be32(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort be64_compare(const void *e1, const void *e2)
{
    const acc_uint64l_t d1 = get_be64(e1);
    const acc_uint64l_t d2 = get_be64(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort le16_compare(const void *e1, const void *e2)
{
    const unsigned d1 = get_le16(e1);
    const unsigned d2 = get_le16(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort le24_compare(const void *e1, const void *e2)
{
    const unsigned d1 = get_le24(e1);
    const unsigned d2 = get_le24(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort le32_compare(const void *e1, const void *e2)
{
    const unsigned d1 = get_le32(e1);
    const unsigned d2 = get_le32(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort le64_compare(const void *e1, const void *e2)
{
    const acc_uint64l_t d1 = get_le64(e1);
    const acc_uint64l_t d2 = get_le64(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}


int __acc_cdecl_qsort be16_compare_signed(const void *e1, const void *e2)
{
    const int d1 = get_be16_signed(e1);
    const int d2 = get_be16_signed(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort be24_compare_signed(const void *e1, const void *e2)
{
    const int d1 = get_be24_signed(e1);
    const int d2 = get_be24_signed(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort be32_compare_signed(const void *e1, const void *e2)
{
    const int d1 = get_be32_signed(e1);
    const int d2 = get_be32_signed(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort be64_compare_signed(const void *e1, const void *e2)
{
    const acc_int64l_t d1 = get_be64_signed(e1);
    const acc_int64l_t d2 = get_be64_signed(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort le16_compare_signed(const void *e1, const void *e2)
{
    const int d1 = get_le16_signed(e1);
    const int d2 = get_le16_signed(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort le24_compare_signed(const void *e1, const void *e2)
{
    const int d1 = get_le24_signed(e1);
    const int d2 = get_le24_signed(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort le32_compare_signed(const void *e1, const void *e2)
{
    const int d1 = get_le32_signed(e1);
    const int d2 = get_le32_signed(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int __acc_cdecl_qsort le64_compare_signed(const void *e1, const void *e2)
{
    const acc_int64l_t d1 = get_le64_signed(e1);
    const acc_int64l_t d2 = get_le64_signed(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}


/*************************************************************************
// find util
**************************************************************************/

int find(const void *b, int blen, const void *what, int wlen)
{
    if (b == NULL || blen <= 0 || what == NULL || wlen <= 0)
        return -1;

    int i;
    const unsigned char *base = (const unsigned char *) b;
    unsigned char firstc = * (const unsigned char *) what;

    blen -= wlen;
    for (i = 0; i <= blen; i++, base++)
        if (*base == firstc && memcmp(base, what, wlen) == 0)
            return i;

    return -1;
}


int find_be16(const void *b, int blen, unsigned what)
{
    unsigned char w[2];
    set_be16(w, what);
    return find(b, blen, w, 2);
}


int find_be32(const void *b, int blen, unsigned what)
{
    unsigned char w[4];
    set_be32(w, what);
    return find(b, blen, w, 4);
}


int find_be64(const void *b, int blen, acc_uint64l_t what)
{
    unsigned char w[8];
    set_be64(w, what);
    return find(b, blen, w, 8);
}


int find_le16(const void *b, int blen, unsigned what)
{
    unsigned char w[2];
    set_le16(w, what);
    return find(b, blen, w, 2);
}


int find_le32(const void *b, int blen, unsigned what)
{
    unsigned char w[4];
    set_le32(w, what);
    return find(b, blen, w, 4);
}


int find_le64(const void *b, int blen, acc_uint64l_t what)
{
    unsigned char w[8];
    set_le64(w, what);
    return find(b, blen, w, 8);
}


int mem_replace(void *bb, int blen, const void *what, int wlen, const void *r)
{
    unsigned char *b = (unsigned char *) bb;
    int boff = 0;
    int n = 0;

    while (blen - boff >= wlen)
    {
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
// filename util
**************************************************************************/

#if (ACC_OS_CYGWIN || ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_EMX || ACC_OS_OS2 || ACC_OS_OS16 || ACC_OS_TOS || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)

static const char dir_sep[] = "/\\";
#define fn_is_drive(s)      (s[0] && s[1] == ':')
#define fn_is_sep(c)        (strchr(dir_sep,c) != NULL)
#define fn_skip_drive(s)    (fn_is_drive(s) ? (s) + 2 : (s))
#define fn_tolower(c)       (tolower(((unsigned char)(c))))

#else

static const char dir_sep[] = "/";
#define fn_is_drive(s)      (0)
#define fn_is_sep(c)        ((c) == '/')
#define fn_skip_drive(s)    (s)
#define fn_tolower(c)       (c)

#endif


char *fn_basename(const char *name)
{
    const char *n, *nn;

    name = fn_skip_drive(name);
    for (nn = n = name; *nn; nn++)
        if (fn_is_sep(*nn))
            n = nn + 1;
    return const_cast<char *>(n);
}


bool fn_has_ext(const char *name, const char *ext, bool ignore_case)
{
    const char *n, *e;

    name = fn_basename(name);
    for (n = e = name; *n; n++)
        if (*n == '.')
            e = n;
    if (ignore_case)
        return (strcasecmp(ext,e+1) == 0);
    else
        return (fn_strcmp(ext,e+1) == 0);
}


void fn_addslash(char *name, bool slash)
{
    char *p;

    name = fn_skip_drive(name);
    p = name + strlen(name);
    while (p > name && fn_is_sep(p[-1]))
        *p-- = 0;
    if (p > name)
    {
        if (slash)
            *p++ = dir_sep[0];
        *p = 0;
    }
}


char *fn_strlwr(char *n)
{
    char *p;
    for (p = n; *p; p++)
        *p = (char) fn_tolower(*p);
    return n;
}


int fn_strcmp(const char *n1, const char *n2)
{
    for (;;)
    {
        if (*n1 != *n2)
        {
            int c = fn_tolower(*n1) - fn_tolower(*n2);
            if (c)
                return c;
        }
        if (*n1 == 0)
            return 0;
        n1++; n2++;
    }
}


bool fn_is_same_file(const char *n1, const char *n2)
{
    /* very simple... */
    if (fn_strcmp(n1, n2) == 0)
        return 1;
    return 0;
}


/*************************************************************************
// time util
**************************************************************************/

#if 0 // not used

#if defined(HAVE_LOCALTIME)
void tm2str(char *s, size_t size, const struct tm *tmp)
{
    upx_snprintf(s, size, "%04d-%02d-%02d %02d:%02d:%02d",
                 (int) tmp->tm_year + 1900, (int) tmp->tm_mon + 1,
                 (int) tmp->tm_mday,
                 (int) tmp->tm_hour, (int) tmp->tm_min, (int) tmp->tm_sec);
}
#endif


void time2str(char *s, size_t size, const time_t *t)
{
    assert(size >= 18);
#if defined(HAVE_LOCALTIME)
    tm2str(s, size, localtime(t));
#elif defined(HAVE_CTIME)
    const char *p = ctime(t);
    memset(s, ' ', 16);
    memcpy(s + 2, p + 4, 6);
    memcpy(s + 11, p + 11, 5);
    s[16] = 0;
#else
    s[0] = 0;
#endif
}

#endif


/*************************************************************************
// misc.
**************************************************************************/

bool set_method_name(char *buf, size_t size, int method, int level)
{
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
    else
    {
        alg = "???";
        r = false;
    }
    if (level > 0)
        upx_snprintf(buf, size, "%s/%d", alg, level);
    else
        upx_snprintf(buf, size, "%s", alg);
    return r;
}


void center_string(char *buf, size_t size, const char *s)
{
    size_t l1 = size - 1;
    size_t l2 = strlen(s);
    assert(size > 0);
    assert(l2 < size);
    memset(buf, ' ', l1);
    memcpy(buf+(l1-l2)/2, s, l2);
    buf[l1] = 0;
}


bool file_exists(const char *name)
{
    int fd, r;
    struct stat st;

    /* return true if we can open it */
    fd = open(name, O_RDONLY, 0);
    if (fd >= 0)
    {
        (void) close(fd);
        return true;
    }

    /* return true if we can stat it */
    //memset(&st, 0, sizeof(st));
    r = stat(name, &st);
    if (r != -1)
        return true;

    /* return true if we can lstat it */
#if defined(HAVE_LSTAT)
    //memset(&st, 0, sizeof(st));
    r = lstat(name, &st);
    if (r != -1)
        return true;
#endif

    return false;
}


bool maketempname(char *ofilename, size_t size,
                  const char *ifilename, const char *ext, bool force)
{
    char *ofext = NULL, *ofname;
    int ofile;

    if (size <= 0)
        return false;

    strcpy(ofilename, ifilename);
    for (ofname = fn_basename(ofilename); *ofname; ofname++)
    {
        if (*ofname == '.')
            ofext = ofname;
    }
    if (ofext == NULL)
        ofext = ofilename + strlen(ofilename);
    strcpy(ofext, ext);

    for (ofile = 0; ofile < 1000; ofile++)
    {
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


bool makebakname(char *ofilename, size_t size,
                 const char *ifilename, bool force)
{
    char *ofext = NULL, *ofname;
    int ofile;

    if (size <= 0)
        return false;

    strcpy(ofilename, ifilename);
    for (ofname = fn_basename(ofilename); *ofname; ofname++)
    {
        if (*ofname == '.')
            ofext = ofname;
    }
    if (ofext == NULL)
    {
        ofext = ofilename + strlen(ofilename);
        strcpy(ofext, ".~");
    }
    else if (strlen(ofext) < 1 + 3)
        strcat(ofilename, "~");
    else
        ofext[strlen(ofext)-1] = '~';

    for (ofile = 0; ofile < 1000; ofile++)
    {
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

unsigned get_ratio(unsigned u_len, unsigned c_len)
{
    const unsigned n = 1000000;
    if (u_len <= 0)
        return c_len <= 0 ? 0 : n;
#if defined(acc_uint64l_t)
    return (unsigned) ((c_len * (acc_uint64l_t)n) / u_len);
#else
# if 0
    return (unsigned) acc_umuldiv32(c_len, n, u_len);
# else
    return (unsigned) ((c_len * (double)n) / u_len);
# endif
#endif
}


/*************************************************************************
// Don't link these functions from libc ==> save xxx bytes
**************************************************************************/

extern "C" {

// FIXME - quick hack for arm-wince-gcc-3.4 (Debian pocketpc-*.deb packages)
#if 1 && (ACC_ARCH_ARM) && defined(__pe__) && !defined(__CEGCC__) && !defined(_WIN32)
int dup(int fd) { UNUSED(fd); return -1; }
#endif

#if defined(__DJGPP__)
int _is_executable(const char *, int, const char *)
{
    return 0;
}

// FIXME: something wants to link in ctime.o
time_t XXX_mktime(struct tm *)
{
    return 0;
}

time_t time(time_t *t)
{
    if (t) *t = 0;
    return 0;
}
#endif /* __DJGPP__ */


} // extern "C"


/*
vi:ts=4:et
*/

