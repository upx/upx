/* ACC -- Automatic Compiler Configuration

   Copyright (C) 1996-2003 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   This software is a copyrighted work licensed under the terms of
   the GNU General Public License. Please consult the file "ACC_LICENSE"
   for details.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/
 */



#if !defined(ACC_VERSION)
#  if defined ACC_CONFIG_INCLUDE
#    include ACC_CONFIG_INCLUDE("acc.h")
#  else
#    include "acc.h"
#  endif
#endif
#ifndef __ACC_INCE_H_INCLUDED
#  include ACC_CONFIG_INCLUDE("acc_ince.h")
#endif
#ifndef __ACC_INCI_H_INCLUDED
#  include ACC_CONFIG_INCLUDE("acc_inci.h")
#endif
#ifndef __ACC_LIB_H_INCLUDED
#  include ACC_CONFIG_INCLUDE("acc_lib.h")
#endif


/***********************************************************************
// huge pointer layer - alloc
************************************************************************/

ACC_LIBFUNC(acc_hvoid_p, acc_halloc) (acc_alloc_p ap, acc_hsize_t items, size_t size)
{
    acc_hvoid_p p = 0;
#if (SIZEOF_SIZE_T > SIZEOF_LONG)
    size_t s = (size_t) items * size;
#else
    unsigned long s = (unsigned long) items * size;
#endif

    if (!ap)
        return p;
    ap->ptr = p;
#if 0
    ap->items = items;
    ap->size = size;
    ap->flags = 0;
#endif
    if (items <= 0 || size <= 0 || s < items || s < size)
        return p;

#if defined(__palmos__)
    p = MemPtrNew(s);
#elif !defined(ACC_HAVE_MM_HUGE_PTR)
    if (s < (size_t) -1)
        p = malloc((size_t) s);
#else
#if (ACC_CC_MSC && _MSC_VER >= 700)
    if (size < (size_t) -1)
        p = _halloc(items, (size_t) size);
#elif (ACC_CC_MSC || ACC_CC_WATCOMC)
    if (size < (size_t) -1)
        p = halloc(items, (size_t) size);
#elif (ACC_CC_DMC || ACC_CC_SYMANTECC || ACC_CC_ZORTECHC)
    if (size < (size_t) -1)
        p = _halloc(items, (size_t) size);
#elif (ACC_CC_BORLANDC || ACC_CC_TURBOC)
    p = farmalloc(s);
#elif defined(ACC_CC_AZTECC)
    p = lmalloc(s);
#else
    if (s < (size_t) -1)
        p = malloc((size_t) s);
#endif
#endif

    ap->ptr = p;
    return p;
}


ACC_LIBFUNC(int, acc_hfree) (acc_alloc_p ap)
{
    acc_hvoid_p p;
    int r = 0;

    if (!ap || !ap->ptr)
        return r;
    p = ap->ptr;
    ap->ptr = 0;

#if defined(__palmos__)
    r = MemPtrFree(p);
#elif !defined(ACC_HAVE_MM_HUGE_PTR)
    free(p);
#else
#if (ACC_CC_MSC && _MSC_VER >= 700)
    _hfree(p);
#elif (ACC_CC_MSC || ACC_CC_WATCOMC)
    hfree(p);
#elif (ACC_CC_DMC || ACC_CC_SYMANTECC || ACC_CC_ZORTECHC)
    _hfree(p);
#elif (ACC_CC_BORLANDC || ACC_CC_TURBOC)
    farfree((void __far *) p);
#elif defined(ACC_CC_AZTECC)
    lfree(p);
#else
    free(p);
#endif
#endif

    return r;
}


/***********************************************************************
// huge pointer layer - string.h
************************************************************************/

ACC_LIBFUNC(int, acc_hmemcmp) (const acc_hvoid_p s1, const acc_hvoid_p s2, acc_hsize_t len)
{
#if (ACC_HAVE_MM_HUGE_PTR) || !defined(HAVE_MEMCMP)
    const acc_hbyte_p p1 = (const acc_hbyte_p) s1;
    const acc_hbyte_p p2 = (const acc_hbyte_p) s2;

    if (len > 0) do
    {
        int d = *p1 - *p2;
        if (d != 0)
            return d;
        p1++; p2++;
    } while (--len > 0);
    return 0;
#else
    return memcmp(s1, s2, len);
#endif
}


ACC_LIBFUNC(acc_hvoid_p, acc_hmemcpy) (acc_hvoid_p dest, const acc_hvoid_p src, acc_hsize_t len)
{
#if (ACC_HAVE_MM_HUGE_PTR) || !defined(HAVE_MEMCPY)
    acc_hbyte_p p1 = (acc_hbyte_p) dest;
    const acc_hbyte_p p2 = (const acc_hbyte_p) src;

    if (len <= 0 || p1 == p2)
        return dest;
    do
        *p1++ = *p2++;
    while (--len > 0);
    return dest;
#else
    return memcpy(dest, src, len);
#endif
}


ACC_LIBFUNC(acc_hvoid_p, acc_hmemmove) (acc_hvoid_p dest, const acc_hvoid_p src, acc_hsize_t len)
{
#if (ACC_HAVE_MM_HUGE_PTR) || !defined(HAVE_MEMMOVE)
    acc_hbyte_p p1 = (acc_hbyte_p) dest;
    const acc_hbyte_p p2 = (const acc_hbyte_p) src;

    if (len <= 0 || p1 == p2)
        return dest;

    if (p1 < p2)
    {
        do
            *p1++ = *p2++;
        while (--len > 0);
    }
    else
    {
        p1 += len;
        p2 += len;
        do
            *--p1 = *--p2;
        while (--len > 0);
    }
    return dest;
#else
    return memmove(dest, src, len);
#endif
}


ACC_LIBFUNC(acc_hvoid_p, acc_hmemset) (acc_hvoid_p s, int c, acc_hsize_t len)
{
#if (ACC_HAVE_MM_HUGE_PTR) || !defined(HAVE_MEMSET)
    acc_hbyte_p p = (acc_hbyte_p) s;

    if (len > 0) do
        *p++ = (unsigned char) c;
    while (--len > 0);
    return s;
#else
    return memset(s, c, len);
#endif
}


/***********************************************************************
// huge pointer layer - stdio.h
************************************************************************/

ACC_LIBFUNC(acc_hsize_t, acc_hfread) (FILE *fp, acc_hvoid_p buf, acc_hsize_t size)
{
#if (ACC_HAVE_MM_HUGE_PTR)
#if (ACC_MM_COMPACT || ACC_MM_LARGE || ACC_MM_HUGE)
    acc_hbyte_p b = (acc_hbyte_p) buf;
    acc_hsize_t l = 0;

    while (l < size)
    {
        size_t n;
        n = FP_OFF(b); n = (n <= 1) ? 0x8000u : (0u - n);
        if ((acc_hsize_t) n > size - l)
            n = (size_t) (size - l);
        n = fread((void __far*)b, 1, n, fp);
        if (n == 0)
            break;
        b += n; l += n;
    }
    return l;
#else
    unsigned char tmp[512];
    acc_hsize_t l = 0;

    while (l < size)
    {
        size_t n = size - l > sizeof(tmp) ? sizeof(tmp) : (size_t) (size - l);
        n = fread(tmp, 1, n, fp);
        if (n == 0)
            break;
        acc_hmemcpy((acc_hbyte_p)buf + l, tmp, n);
        l += n;
    }
    return l;
#endif
#else
    return fread(buf, 1, size, fp);
#endif
}


ACC_LIBFUNC(acc_hsize_t, acc_hfwrite) (FILE *fp, const acc_hvoid_p buf, acc_hsize_t size)
{
#if (ACC_HAVE_MM_HUGE_PTR)
#if (ACC_MM_COMPACT || ACC_MM_LARGE || ACC_MM_HUGE)
    const acc_hbyte_p b = (const acc_hbyte_p) buf;
    acc_hsize_t l = 0;

    while (l < size)
    {
        size_t n;
        n = FP_OFF(b); n = (n <= 1) ? 0x8000u : (0u - n);
        if ((acc_hsize_t) n > size - l)
            n = (size_t) (size - l);
        n = fwrite((void __far*)b, 1, n, fp);
        if (n == 0)
            break;
        b += n; l += n;
    }
    return l;
#else
    unsigned char tmp[512];
    acc_hsize_t l = 0;

    while (l < size)
    {
        size_t n = size - l > sizeof(tmp) ? sizeof(tmp) : (size_t) (size - l);
        acc_hmemcpy(tmp, (const acc_hbyte_p)buf + l, n);
        n = fwrite(tmp, 1, n, fp);
        if (n == 0)
            break;
        l += n;
    }
    return l;
#endif
#else
    return fwrite(buf, 1, size, fp);
#endif
}


/***********************************************************************
// huge pointer layer - stdio.h
************************************************************************/

#if (ACC_HAVE_MM_HUGE_PTR)

ACC_LIBFUNC(long, acc_hread) (int fd, acc_hvoid_p buf, long size)
{
#if (ACC_MM_COMPACT || ACC_MM_LARGE || ACC_MM_HUGE)
    acc_hbyte_p b = (acc_hbyte_p) buf;
    long l = 0;

    while (l < size)
    {
        unsigned n;
        n = FP_OFF(b); n = (n <= 1) ? 0x8000u : (0u - n);
        if ((long) n > size - l)
            n = (unsigned) (size - l);
        n = read(fd, (void __far*)b, n);
        if (n == 0)
            break;
        if (n == (unsigned)-1)
            return -1;
        b += n; l += n;
    }
    return l;
#else
    unsigned char tmp[512];
    long l = 0;

    while (l < size)
    {
        int n = size - l > (long)sizeof(tmp) ? (int) sizeof(tmp) : (int) (size - l);
        n = read(fd, tmp, n);
        if (n == 0)
            break;
        if (n < 0)
            return -1;
        acc_hmemcpy((acc_hbyte_p)buf + l, tmp, n);
        l += n;
    }
    return l;
#endif
}


ACC_LIBFUNC(long, acc_hwrite) (int fd, const acc_hvoid_p buf, long size)
{
#if (ACC_MM_COMPACT || ACC_MM_LARGE || ACC_MM_HUGE)
    const acc_hbyte_p b = (const acc_hbyte_p) buf;
    long l = 0;

    while (l < size)
    {
        unsigned n;
        n = FP_OFF(b); n = (n <= 1) ? 0x8000u : (0u - n);
        if ((long) n > size - l)
            n = (unsigned) (size - l);
        n = write(fd, (void __far*)b, n);
        if (n == 0)
            break;
        if (n == (unsigned)-1)
            return -1;
        b += n; l += n;
    }
    return l;
#else
    unsigned char tmp[512];
    long l = 0;

    while (l < size)
    {
        int n = size - l > (long)sizeof(tmp) ? (int) sizeof(tmp) : (int) (size - l);
        acc_hmemcpy(tmp, (const acc_hbyte_p)buf + l, n);
        n = write(fd, tmp, n);
        if (n == 0)
            break;
        if (n < 0)
            return -1;
        l += n;
    }
    return l;
#endif
}

#endif


/*************************************************************************
// wrap <dirent.h>
**************************************************************************/

#if (ACC_OS_WIN32 || ACC_OS_WIN64)

ACC_LIBFUNC(int, acc_findfirst) (struct acc_find_t* f, const char* path)
{
    WIN32_FIND_DATAA d;
    HANDLE h;
#if 1
    /* transform to backslashes, and add a '\*' to the directory name */
    char* p = f->f_name;
    p[0] = 0;
    if (!path[0] || strlen(path) >= sizeof(f->f_name) - 2)
        return -1;
    strcpy(p, path);
    for ( ; *p; p++)
        if (*p == '/')
            *p = '\\';
    if (p[-1] != '\\')
        *p++ = '\\';
    *p++ = '*';
    *p = '\0';
    h = FindFirstFileA(f->f_name, &d);
#else
    h = FindFirstFileA(path, &d);
#endif
    f->f_name[0] = 0;
    if ((f->u.h = (long) h) == -1)
        return -1;
    if (!d.cFileName[0] || strlen(d.cFileName) >= sizeof(f->f_name))
        return -1;
    strcpy(f->f_name, d.cFileName);
    return 0;
}

ACC_LIBFUNC(int, acc_findnext) (struct acc_find_t* f)
{
    WIN32_FIND_DATAA d;
    f->f_name[0] = 0;
    if (f->u.h == -1 || FindNextFileA((HANDLE) f->u.h, &d) != 0)
        return -1;
    if (!d.cFileName[0] || strlen(d.cFileName) >= sizeof(f->f_name))
        return -1;
    strcpy(f->f_name, d.cFileName);
    return 0;
}

ACC_LIBFUNC(int, acc_findclose) (struct acc_find_t* f)
{
    int r = -1;
    if (f->u.h != -1)
        r = FindClose((HANDLE) f->u.h);
    f->u.h = -1;
    return r;
}


#elif (HAVE_DIRENT_H)

ACC_LIBFUNC(int, acc_findfirst) (struct acc_find_t* f, const char* path)
{
    f->u.dirp = opendir(path);
    return acc_findnext(f);
}

ACC_LIBFUNC(int, acc_findnext) (struct acc_find_t* f)
{
    const struct dirent* dp;
    f->f_name[0] = 0;
    if (!f->u.dirp)
        return -1;
    dp = readdir((DIR*) f->u.dirp);
    if (!dp)
        return -1;
    if (!dp->d_name[0] || strlen(dp->d_name) >= sizeof(f->f_name))
        return -1;
    strcpy(f->f_name, dp->d_name);
    return 0;
}

ACC_LIBFUNC(int, acc_findclose) (struct acc_find_t* f)
{
    int r = -1;
    if (f->u.dirp)
        r = closedir((DIR*) f->u.dirp);
    f->u.dirp = 0;
    return r;
}


#else

ACC_LIBFUNC(int, acc_findfirst) (struct acc_find_t* f, const char* path)
{
    ACC_UNUSED(path);
    f->f_name[0] = 0;
    return -1;
}

ACC_LIBFUNC(int, acc_findnext) (struct acc_find_t* f)
{
    f->f_name[0] = 0;
    return -1;
}

ACC_LIBFUNC(int, acc_findclose) (struct acc_find_t* f)
{
    f->u.dirp = 0;
    return -1;
}

#endif


/*************************************************************************
// wrap <fnmatch.h>
**************************************************************************/


/*************************************************************************
// wrap <getopt.h>
**************************************************************************/


/*************************************************************************
// wrap misc
**************************************************************************/

ACC_LIBFUNC(long, acc_get_osfhandle) (int fd)
{
    if (fd < 0)
        return -1;
#if (ACC_OS_CYGWIN)
    return get_osfhandle(fd);
#elif (ACC_OS_EMX && defined(__RSXNT__))
    /* FIXME */
    return -1;
#elif (ACC_OS_WIN32 || ACC_OS_WIN64)
    return _get_osfhandle(fd);
#else
    return fd;
#endif
}


ACC_LIBFUNC(int, acc_set_binmode) (int fd, int binary)
{
#if (ACC_OS_TOS && defined(__MINT__))
    int old_binary;
    FILE* f;
    if (fd == STDIN_FILENO) f = stdin;
    else if (fd == STDOUT_FILENO) f = stdout;
    else if (fd == STDERR_FILENO) f = stderr;
    else return -1;
    old_binary = f->__mode.__binary;
    __set_binmode(f, binary ? 1 : 0);
    return old_binary ? 1 : 0;
#elif (ACC_OS_TOS)
    ACC_UNUSED(fd); ACC_UNUSED(binary);
    return -1;
#elif (ACC_OS_DOS16 && (ACC_CC_AZTECC || ACC_CC_PACIFICC))
    ACC_UNUSED(fd); ACC_UNUSED(binary);
    return -1;
#elif defined(__DJGPP__)
    int r;
    unsigned old_flags = __djgpp_hwint_flags;
    ACC_COMPILE_TIME_ASSERT(O_BINARY > 0)
    ACC_COMPILE_TIME_ASSERT(O_TEXT > 0)
    if (fd < 0)
        return -1;
    r = setmode(fd, binary ? O_BINARY : O_TEXT);
    if ((old_flags & 1u) != (__djgpp_hwint_flags & 1u))
        __djgpp_set_ctrl_c(!(old_flags & 1));
    if (r == -1)
        return -1;
    return (r & O_TEXT) ? 0 : 1;
#elif (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64 || ACC_OS_CYGWIN || ACC_OS_EMX)
    int r;
#if !defined(ACC_CC_ZORTECHC)
    ACC_COMPILE_TIME_ASSERT(O_BINARY > 0)
#endif
    ACC_COMPILE_TIME_ASSERT(O_TEXT > 0)
    if (fd < 0)
        return -1;
    r = setmode(fd, binary ? O_BINARY : O_TEXT);
    if (r == -1)
        return -1;
    return (r & O_TEXT) ? 0 : 1;
#else
    if (fd < 0)
        return -1;
    ACC_UNUSED(binary);
    return 1;
#endif
}


ACC_LIBFUNC(int, acc_isatty) (int fd)
{
#if (ACC_H_WINDOWS_H)
    /* work around naive library implementations that think that
     * any character device like `nul' is a tty */
    long h = acc_get_osfhandle(fd);
    if (h != -1)
    {
        DWORD d = 0;
        int r = GetConsoleMode((HANDLE)h, &d);
        if (!r)
            return 0;   /* GetConsoleMode failed -> not a tty */
    }
#endif
    if (fd < 0)
        return 0;
    return (isatty(fd)) ? 1 : 0;
}


ACC_LIBFUNC(int, acc_mkdir) (const char* name, unsigned mode)
{
#if (ACC_OS_POSIX || ACC_OS_CYGWIN || ACC_OS_EMX)
    return mkdir(name, mode);
#elif (ACC_OS_TOS && (ACC_CC_PUREC || ACC_CC_TURBOC))
    ACC_UNUSED(mode);
    return Dcreate(name);
#elif defined(__DJGPP__) || (ACC_OS_TOS)
    return mkdir(name, mode);
#else
    ACC_UNUSED(mode);
    return mkdir(name);
#endif
}


#if 0
ACC_LIBFUNC(int, acc_response) (int* argc, char*** argv)
{
}
#endif


/*
vi:ts=4:et
*/
