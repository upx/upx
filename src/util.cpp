/* util.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2000 Laszlo Molnar
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

   Markus F.X.J. Oberhumer                   Laszlo Molnar
   markus.oberhumer@jk.uni-linz.ac.at        ml1050@cdata.tvnet.hu
 */


#include "conf.h"
#include "util.h"


/*************************************************************************
//
**************************************************************************/

int be16_compare(const void *e1, const void *e2)
{
    const unsigned d1 = get_be16(e1);
    const unsigned d2 = get_be16(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int be32_compare(const void *e1, const void *e2)
{
    const unsigned d1 = get_be32(e1);
    const unsigned d2 = get_be32(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int le16_compare(const void *e1, const void *e2)
{
    const unsigned d1 = get_le16(e1);
    const unsigned d2 = get_le16(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}

int le32_compare(const void *e1, const void *e2)
{
    const unsigned d1 = get_le32(e1);
    const unsigned d2 = get_le32(e2);
    return (d1 < d2) ? -1 : ((d1 > d2) ? 1 : 0);
}


/*************************************************************************
//
**************************************************************************/

upx_bytep find(const void *b, int blen, const void *what, int wlen)
{
    int i;
    const upx_bytep base = (const upx_bytep) b;
    unsigned char firstc = * (const upx_bytep) what;

    for (i = 0; i <= blen - wlen; i++, base++)
        if (*base == firstc && memcmp(base,what,wlen) == 0)
            return const_cast<upx_bytep>(base);

    return NULL;
}


upx_bytep find_be16(const void *b, int blen, unsigned what)
{
    unsigned char w[2];
    set_be16(w,what);
    return find(b,blen,w,2);
}


upx_bytep find_be32(const void *b, int blen, unsigned what)
{
    unsigned char w[4];
    set_be32(w,what);
    return find(b,blen,w,4);
}


upx_bytep find_le16(const void *b, int blen, unsigned what)
{
    unsigned char w[2];
    set_le16(w,what);
    return find(b,blen,w,2);
}


upx_bytep find_le32(const void *b, int blen, unsigned what)
{
    unsigned char w[4];
    set_le32(w,what);
    return find(b,blen,w,4);
}


/*************************************************************************
// string util
**************************************************************************/

int upx_snprintf(char *str, long n, const char *format, ...)
{
    int r = -1;
    if (n > 0)
    {
        va_list args;
        va_start(args,format);
        r = upx_vsnprintf(str,n,format,args);
        va_end(args);
    }
    assert(r >= 0 && r < n);        // UPX assertion
    return r;
}


int upx_vsnprintf(char *str, long n, const char *format, va_list ap)
{
    int r = -1;
    if (n > 0)
    {
#if defined(HAVE_VSNPRINTF)
        r = vsnprintf(str,(size_t)n,format,ap);
#else
        r = vsprintf(str,format,ap);
#endif
        // UPX extension: make sure the string is '\0' terminated in any case
        str[n-1] = 0;
    }
    assert(r >= 0 && r < n);        // UPX assertion
    return r;
}


/*************************************************************************
// ctype util
**************************************************************************/

#if 0
bool upx_isdigit(int c)
{
    return c >= '0' && c <= '9';
}

bool upx_islower(int c)
{
    return c >= 'a' && c <= 'z';
}

bool upx_isspace(int c)
{
    // according to "C" and "POSIX" locales
    return strchr(" \f\n\r\t\v", c) != NULL;
}

int upx_tolower(int c)
{
    if (c >= 'A' && c <= 'Z')
        c += 'a' - 'A';
    return c;
}
#endif


/*************************************************************************
// filename util
**************************************************************************/

static const char dir_sep[] = DIR_SEP;

#define fn_is_sep(c)        (strchr(dir_sep,c) != NULL)

#if defined(DOSISH)
#define fn_is_drive(n)      (n[0] && n[1] == ':')
#define fn_skip_drive(n)    (fn_is_drive(n) ? (n) + 2 : (n))
#else
#define fn_is_drive(n)      (0)
#define fn_skip_drive(n)    (n)
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


bool fn_has_ext(const char *name, const char *ext)
{
    const char *n, *e;

    name = fn_basename(name);
    for (n = e = name; *n; n++)
        if (*n == '.')
            e = n;
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
    if (fn_strcmp(n1,n2) == 0)
        return 1;
    return 0;
}


/*************************************************************************
// time util
**************************************************************************/

#if 0 // not used

#if defined(HAVE_LOCALTIME)
void tm2str(char *s, const struct tm *tmp)
{
    sprintf(s,"%04d-%02d-%02d %02d:%02d:%02d",
            (int) tmp->tm_year + 1900, (int) tmp->tm_mon + 1,
            (int) tmp->tm_mday,
            (int) tmp->tm_hour, (int) tmp->tm_min, (int) tmp->tm_sec);
}
#endif


void time2str(char *s, const time_t *t)
{
#if defined(HAVE_LOCALTIME)
    tm2str(s,localtime(t));
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

char *center_string(const char *name, size_t s)
{
    static char buf[256+1];
    size_t l = strlen(name);
    assert(l <= s && l < sizeof(buf));
    memset(buf,' ',s);
    memcpy(buf+(s-l)/2,name,l);
    buf[s] = 0;
    return buf;
}


bool file_exists(const char *name)
{
    int fd, r;
    struct stat st;

    /* return true if we can open it */
    fd = open(name,O_RDONLY);
    if (fd >= 0)
    {
        (void) close(fd);
        return true;
    }

    /* return true if we can stat it */
    memset(&st, 0, sizeof(st));
#if defined(HAVE_LSTAT)
    r = lstat(name,&st);
#else
    r = stat(name,&st);
#endif
    if (r != -1)
        return true;

    return false;
}


bool maketempname(char *ofilename, const char *ifilename,
                  const char *ext, bool force)
{
    char *ofext = NULL, *ofname;
    int ofile;

    strcpy(ofilename, ifilename);
    for (ofname = fn_basename(ofilename); *ofname; ofname++)
    {
        if (*ofname == '.')
            ofext = ofname;
    }
    if (ofext == NULL)
        ofext = ofilename + strlen(ofilename);
    strcpy(ofext, ext);
    if (!force)
        return true;
    if (file_exists(ofilename))
        for (ofile = 0; ofile < 999; ofile++)
        {
            sprintf(ofext, ".%03d", ofile);
            if (!file_exists(ofilename))
                return true;
        }
    return false;
}


bool makebakname(char *ofilename, const char *ifilename, bool force)
{
    char *ofext = NULL, *ofname;
    int ofile;

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
    if (!force)
        return true;
    if (file_exists(ofilename))
        for (ofile = 0; ofile < 999; ofile++)
        {
            sprintf(ofext, ".%03d", ofile);
            if (!file_exists(ofilename))
                return true;
        }
    return false;
}


/* test if fd is connected to a file or a pipe */
bool isafile(int fd)
{
    if (isatty(fd))
        return 0;
#if defined(HAVE_FSTAT)
    {
        struct stat st;
        if (fstat(fd, &st) != 0)
            return 0;
        /* fprintf(stderr,"fstat(%d): %o\n", fd, st.st_mode); */
        if (S_ISDIR(st.st_mode))
            return 0;
    }
#endif
    return 1;
}


/*************************************************************************
//
**************************************************************************/

unsigned get_ratio (unsigned long packedsize, unsigned long size,
                    unsigned long scale)
{
    unsigned long n1, n2;

    n1 = 100 * scale; n2 = 1;
    if (size <= 0)
        return (unsigned) n1;
    while (n1 > 1 && packedsize > (ULONG_MAX / n1))
    {
        n1 /= 10;
        n2 *= 10;
    }
    return (unsigned) ((packedsize * n1) / (size / n2)) + 5;
}


/*************************************************************************
// memory debugging
**************************************************************************/

#if defined(WITH_GC)
extern "C" {

#undef malloc
#undef realloc
#undef free
#ifndef __malloc_ptr_t
#  define __malloc_ptr_t    __ptr_t
#endif

__malloc_ptr_t malloc(size_t size)
{
    return GC_MALLOC(size);
}

__malloc_ptr_t realloc(__malloc_ptr_t ptr, size_t size)
{
    return GC_REALLOC(ptr, size);
}

void free(__malloc_ptr_t ptr)
{
    GC_FREE(ptr);
}

}; // extern "C"
#endif


/*************************************************************************
// Don't link these functions from libc ==> save xxx bytes
**************************************************************************/

extern "C" {

#if defined(__DJGPP__)
int _is_executable(const char *, int , const char *)
{
    return 0;
}

//FIXME: something wants to link in ctime.o
time_t mktime(struct tm *)
{
    return 0;
}

time_t time(time_t *)
{
    return 0;
}
#endif /* __DJGPP__ */


// These space savings are only useful when building a statically
// linked version, so this is disabled for now.
#if 0 && defined(__linux__) && defined(__GLIBC__)

#if 1
// We don't need floating point support in printf().
// See glibc/stdio-common/*
int __printf_fp(void)
{
    assert(0);
    return -1;
}
int __printf_fphex(void)
{
    assert(0);
    return -1;
}
#endif


#if 1
// We don't need multibyte character support.
// See <wchar.h> and glibc/xxx/vfprintf.c
int mblen(const char *, size_t)
{
    assert(0);
    return -1;
}
int mbtowc(...)
{
    assert(0);
    return -1;
}
int mbrtowc(...)
{
    assert(0);
    return -1;
}
// glibc internals
size_t __ctype_get_mb_cur_max(void)
{
    assert(0);
    return 1;
}
int __mbrlen(...)
{
    assert(0);
    return -1;
}
int __mbrtowc(...)
{
    assert(0);
    return -1;
}
int __wcrtomb(...)
{
    assert(0);
    return -1;
}
int __wcsrtombs(...)
{
    assert(0);
    return -1;
}
// see strtol.c
int __btowc(...)
{
    assert(0);
    return -1;
}
#endif


#if 1
// We don't need *scanf.
// See iovsscanf.c
// (libc6 2.1.1-10: this function pulls in ~80kB code !)
int _IO_vsscanf(void)
{
    assert(0);
    return -1;
}
#endif


#if 0
// FIXME - also should get rid of this intl stuff
// see glibc/intl/*
static const char _nl_default_default_domain[] = "messages";
static const char *_nl_current_default_domain = _nl_default_default_domain;
static const char _nl_default_dirname[] = "";
char *bindtextdomain(const char *, const char *)
{
    return NULL;
}
char *textdomain(const char *)
{
    return (char *) _nl_current_default_domain;
}
#endif

#endif /* __linux__ && __GLIBC__ */


}; // extern "C"


/*
vi:ts=4:et
*/

