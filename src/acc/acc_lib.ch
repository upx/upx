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
// halloc
************************************************************************/

ACC_LIBFUNC(acc_hvoid_p, acc_halloc) (acc_hsize_t size)
{
    acc_hvoid_p p = 0;

    if (size <= 0)
        return p;

#if 0 && defined(__palmos__)
    p = MemPtrNew(size);
#elif !defined(ACC_HAVE_MM_HUGE_PTR)
    if (size < (size_t) -1)
        p = malloc((size_t) size);
#else
#if (ACC_CC_MSC && _MSC_VER >= 700)
    p = _halloc(size, 1);
#elif (ACC_CC_MSC || ACC_CC_WATCOMC)
    p = halloc(size, 1);
#elif (ACC_CC_DMC || ACC_CC_SYMANTECC || ACC_CC_ZORTECHC)
    p = farmalloc(size);
#elif (ACC_CC_BORLANDC || ACC_CC_TURBOC)
    p = farmalloc(size);
#elif defined(ACC_CC_AZTECC)
    p = lmalloc(size);
#else
    if (size < (size_t) -1)
        p = malloc((size_t) size);
#endif
#endif

    return p;
}


ACC_LIBFUNC(int, acc_hfree) (acc_hvoid_p p)
{
    int r = 0;

    if (!p)
        return r;

#if 0 && defined(__palmos__)
    r = MemPtrFree(p);
#elif !defined(ACC_HAVE_MM_HUGE_PTR)
    free(p);
#else
#if (ACC_CC_MSC && _MSC_VER >= 700)
    _hfree(p);
#elif (ACC_CC_MSC || ACC_CC_WATCOMC)
    hfree(p);
#elif (ACC_CC_DMC || ACC_CC_SYMANTECC || ACC_CC_ZORTECHC)
    farfree((void __far*) p);
#elif (ACC_CC_BORLANDC || ACC_CC_TURBOC)
    farfree((void __far*) p);
#elif defined(ACC_CC_AZTECC)
    lfree(p);
#else
    free(p);
#endif
#endif

    return r;
}


/***********************************************************************
// dos_alloc
************************************************************************/

#if (ACC_OS_DOS16)
#if !defined(ACC_CC_AZTECC)

ACC_LIBFUNC(void __far*, acc_dos_alloc) (unsigned long size)
{
    void __far* p = 0;
    union REGS ri, ro;

    if ((long)size <= 0)
        return p;
    size = (size + 15) >> 4;
    if (size > 0xffffu)
        return p;
    ri.x.ax = 0x4800;
    ri.x.bx = (unsigned short) size;
    ro.x.cflag = 1;
    int86(0x21, &ri, &ro);
    if ((ro.x.cflag & 1) == 0)  /* if carry flag not set */
        p = (void __far*) ACC_MK_FP(ro.x.ax, 0);
    return p;
}


ACC_LIBFUNC(int, acc_dos_free) (void __far* p)
{
    union REGS ri, ro;
    struct SREGS rs;

    if (!p)
        return 0;
    if (ACC_FP_OFF(p) != 0)
        return -1;
    segread(&rs);
    ri.x.ax = 0x4900;
    rs.es = ACC_FP_SEG(p);
    ro.x.cflag = 1;
    int86x(0x21, &ri, &ro, &rs);
    if (ro.x.cflag & 1)         /* if carry flag set */
        return -1;
    return 0;
}

#endif
#endif


#if (ACC_OS_OS216)

ACC_LIBFUNC(void __far*, acc_dos_alloc) (unsigned long size)
{
    void __far* p = 0;
    unsigned short sel = 0;
    unsigned long pmask = 0xffffu >> ACC_MM_AHSHIFT; /* 8191 */

    if ((long)size <= 0)
        return p;
    size = (size + pmask) &~ pmask;     /* align up to paragraph size */
    if (DosAllocHuge((unsigned short)(size >> 16), (unsigned short)size, &sel, 0, 0) == 0)
        p = (void __far*) ACC_MK_FP(sel, 0);
    return p;
}

ACC_LIBFUNC(int, acc_dos_free) (void __far* p)
{
    if (!p)
        return 0;
    if (ACC_FP_OFF(p) != 0)
        return -1;
    if (DosFreeSeg(ACC_FP_SEG(p)) != 0)
        return -1;
    return 0;
}

#endif


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

ACC_LIBFUNC(acc_hsize_t, acc_hfread) (FILE* fp, acc_hvoid_p buf, acc_hsize_t size)
{
#if (ACC_HAVE_MM_HUGE_PTR)
#if (ACC_MM_COMPACT || ACC_MM_LARGE || ACC_MM_HUGE)
    acc_hbyte_p b = (acc_hbyte_p) buf;
    acc_hsize_t l = 0;

    while (l < size)
    {
        size_t n;
        n = ACC_FP_OFF(b); n = (n <= 1) ? 0x8000u : (0u - n);
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
        acc_hmemcpy((acc_hbyte_p)buf + l, tmp, (acc_hsize_t)n);
        l += n;
    }
    return l;
#endif
#else
    return fread(buf, 1, size, fp);
#endif
}


ACC_LIBFUNC(acc_hsize_t, acc_hfwrite) (FILE* fp, const acc_hvoid_p buf, acc_hsize_t size)
{
#if (ACC_HAVE_MM_HUGE_PTR)
#if (ACC_MM_COMPACT || ACC_MM_LARGE || ACC_MM_HUGE)
    const acc_hbyte_p b = (const acc_hbyte_p) buf;
    acc_hsize_t l = 0;

    while (l < size)
    {
        size_t n;
        n = ACC_FP_OFF(b); n = (n <= 1) ? 0x8000u : (0u - n);
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
        acc_hmemcpy(tmp, (const acc_hbyte_p)buf + l, (acc_hsize_t)n);
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
// huge pointer layer - read/write
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
        n = ACC_FP_OFF(b); n = (n <= 1) ? 0x8000u : (0u - n);
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
        acc_hmemcpy((acc_hbyte_p)buf + l, tmp, (acc_hsize_t)n);
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
        n = ACC_FP_OFF(b); n = (n <= 1) ? 0x8000u : (0u - n);
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
        acc_hmemcpy(tmp, (const acc_hbyte_p)buf + l, (acc_hsize_t)n);
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

#if !defined(ACCLIB_USE_OPENDIR)
#if (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_TOS || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)

static int acc_opendir_init(struct acc_dir_t* f, const char* path, char* buf, size_t bufsize)
{
    size_t l;
    char* p;
    f->f_name[0] = 0;
    buf[0] = 0;
    l = strlen(path);
    if (l == 0 || l >= bufsize - 4)
        return -1;
    strcpy(buf, path);
    p = buf + l;
    if (p[-1] == ':' || p[-1] == '\\' || p[-1] == '/')
        strcpy(p, "*.*");
    else
        strcpy(p, "\\*.*");
    return 0;
}

#endif
#endif


#if (ACCLIB_USE_OPENDIR)

ACC_LIBFUNC(int, acc_opendir) (struct acc_dir_t* f, const char* path)
{
    f->u_dirp = opendir(path);
    if (!f->u_dirp)
        return -2;
    return acc_readdir(f);
}

ACC_LIBFUNC(int, acc_readdir) (struct acc_dir_t* f)
{
    const struct dirent* dp;
    f->f_name[0] = 0;
    if (!f->u_dirp)
        return -1;
    dp = readdir((DIR*) f->u_dirp);
    if (!dp)
        return -1;
    if (!dp->d_name[0] || strlen(dp->d_name) >= sizeof(f->f_name))
        return -1;
    strcpy(f->f_name, dp->d_name);
#if (ACC_CC_WATCOMC)
    ACC_COMPILE_TIME_ASSERT(sizeof(f->f_name) >= sizeof(dp->d_name))
    f->f_time = dp->d_time;
    f->f_date = dp->d_date;
    f->f_size = dp->d_size;
#endif
    return 0;
}

ACC_LIBFUNC(int, acc_closedir) (struct acc_dir_t* f)
{
    int r = -1;
    if (f->u_dirp)
        r = closedir((DIR*) f->u_dirp);
    f->u_dirp = 0;
    return r;
}


#elif (ACC_OS_WIN32 || ACC_OS_WIN64)

ACC_LIBFUNC(int, acc_opendir) (struct acc_dir_t* f, const char* path)
{
    WIN32_FIND_DATAA d;
    HANDLE h;
    if (acc_opendir_init(f, path, f->f_name, sizeof(f->f_name)) != 0)
        return -1;
    h = FindFirstFileA(f->f_name, &d);
    f->f_name[0] = 0;
    if ((f->u_handle = (long) h) == -1)
        return -1;
    if (!d.cFileName[0] || strlen(d.cFileName) >= sizeof(f->f_name))
        return -1;
    strcpy(f->f_name, d.cFileName);
    f->f_attr = d.dwFileAttributes;
    f->f_size_high = d.nFileSizeHigh;
    f->f_size_low = d.nFileSizeLow;
    return 0;
}

ACC_LIBFUNC(int, acc_readdir) (struct acc_dir_t* f)
{
    WIN32_FIND_DATAA d;
    f->f_name[0] = 0;
    if (f->u_handle == -1 || FindNextFileA((HANDLE) f->u_handle, &d) == 0)
        return -1;
    if (!d.cFileName[0] || strlen(d.cFileName) >= sizeof(f->f_name))
        return -1;
    strcpy(f->f_name, d.cFileName);
    f->f_attr = d.dwFileAttributes;
    f->f_size_high = d.nFileSizeHigh;
    f->f_size_low = d.nFileSizeLow;
    return 0;
}

ACC_LIBFUNC(int, acc_closedir) (struct acc_dir_t* f)
{
    int r = -1;
    if (f->u_handle != -1)
        r = FindClose((HANDLE) f->u_handle);
    f->u_handle = -1;
    return r;
}


#elif (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_WIN16)

ACC_LIBFUNC(int, acc_opendir) (struct acc_dir_t* f, const char* path)
{
    char tmp[ACC_FN_PATH_MAX+1];
    int r;
    f->u_dirp = 0;
    if (acc_opendir_init(f, path, tmp, sizeof(tmp)) != 0)
        return -1;
#if (ACC_CC_AZTECC || ACC_CC_PACIFICC)
    r = -1;
#elif (ACC_CC_BORLANDC || ACC_CC_TURBOC)
    r = findfirst(tmp, (struct ffblk*) f->u_dta, FA_HIDDEN|FA_SYSTEM|FA_RDONLY|FA_DIREC);
#else
    r = _dos_findfirst(tmp, _A_HIDDEN|_A_SYSTEM|_A_RDONLY|_A_SUBDIR, (struct find_t*) f->u_dta);
#endif
    if (r != 0) f->f_name[0] = 0;
    if (!f->f_name[0]) return -1;
    f->u_dirp = 1;
    return 0;
}

ACC_LIBFUNC(int, acc_readdir) (struct acc_dir_t* f)
{
    int r;
    f->f_name[0] = 0;
    if (!f->u_dirp)
        return -1;
#if (ACC_CC_AZTECC || ACC_CC_PACIFICC)
    r = -1;
#elif (ACC_CC_BORLANDC || ACC_CC_TURBOC)
    r = findnext((struct ffblk*) f->u_dta);
#else
    r = _dos_findnext((struct find_t*) f->u_dta);
#endif
    if (r != 0) f->f_name[0] = 0;
    if (!f->f_name[0]) return -1;
    return 0;
}

ACC_LIBFUNC(int, acc_closedir) (struct acc_dir_t* f)
{
    ACC_COMPILE_TIME_ASSERT(sizeof(*f) == 44);
    f->f_name[0] = 0;
    f->u_dirp = 0;
    return 0;
}


#elif (ACC_OS_TOS)

ACC_LIBFUNC(int, acc_opendir) (struct acc_dir_t* f, const char* path)
{
    char tmp[ACC_FN_PATH_MAX+1];
    int r;
    DTA* olddta;
    f->u_dirp = 0;
    if (acc_opendir_init(f, path, tmp, sizeof(tmp)) != 0)
        return -1;
    olddta = Fgetdta();
    Fsetdta((DTA*) f->u_dta);
    r = Fsfirst(tmp, FA_HIDDEN|FA_SYSTEM|FA_READONLY|FA_SUBDIR);
    Fsetdta(olddta);
    if (r != 0) f->f_name[0] = 0;
    if (!f->f_name[0]) return -1;
    f->u_dirp = 1;
    return 0;
}

ACC_LIBFUNC(int, acc_readdir) (struct acc_dir_t* f)
{
    int r;
    DTA* olddta;
    f->f_name[0] = 0;
    if (!f->u_dirp)
        return -1;
    olddta = Fgetdta();
    Fsetdta((DTA*) f->u_dta);
    r = Fsnext();
    Fsetdta(olddta);
    if (r != 0) f->f_name[0] = 0;
    if (!f->f_name[0]) return -1;
    return 0;
}

ACC_LIBFUNC(int, acc_closedir) (struct acc_dir_t* f)
{
    ACC_COMPILE_TIME_ASSERT(sizeof(*f) == 44);
    f->f_name[0] = 0;
    f->u_dirp = 0;
    return 0;
}


#else

ACC_LIBFUNC(int, acc_opendir) (struct acc_dir_t* f, const char* path)
{
    ACC_UNUSED(path);
    f->f_name[0] = 0;
    return -3;
}

ACC_LIBFUNC(int, acc_readdir) (struct acc_dir_t* f)
{
    f->f_name[0] = 0;
    return -1;
}

ACC_LIBFUNC(int, acc_closedir) (struct acc_dir_t* f)
{
    f->u_dirp = 0;
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
# if (ACC_CC_WATCOMC && __WATCOMC__ < 1100)
    /* FIXME */
    return -1;
# else
    return _get_osfhandle(fd);
# endif
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
    /* work around library implementations that think that
     * any character device like `nul' is a tty */
    if (fd < 0)
        return 0;
#if (ACC_OS_DOS16 && !defined(ACC_CC_AZTECC))
    {
        union REGS ri, ro;
        ri.x.ax = 0x4400;
        ri.x.bx = fd;
        ro.x.ax = 0;
        ro.x.cflag = 1;
        int86(0x21, &ri, &ro);
        if ((ro.x.cflag & 1) == 0)  /* if carry flag not set */
            if ((ro.x.ax & 0x83) != 0x83)
                return 0;
    }
#elif (ACC_H_WINDOWS_H)
    {
        long h = acc_get_osfhandle(fd);
        if (h != -1)
        {
            DWORD d = 0;
            int r = GetConsoleMode((HANDLE)h, &d);
            if (!r)
                return 0;   /* GetConsoleMode failed -> not a tty */
        }
    }
#endif
    return (isatty(fd)) ? 1 : 0;
}


ACC_LIBFUNC(int, acc_mkdir) (const char* name, unsigned mode)
{
#if (ACC_OS_TOS && (ACC_CC_PUREC || ACC_CC_TURBOC))
    ACC_UNUSED(mode);
    return Dcreate(name);
#elif defined(__DJGPP__)
    return mkdir(name, mode);
#elif (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)
    ACC_UNUSED(mode);
    return mkdir(name);
#else
    return mkdir(name, mode);
#endif
}


#if 0
ACC_LIBFUNC(int, acc_response) (int* argc, char*** argv)
{
}
#endif


/*************************************************************************
// some linear congruential pseudo random number generators (PRNG)
**************************************************************************/

ACC_LIBFUNC(void, acc_srand31) (acc_rand31_t* r, acc_uint32l_t seed)
{
    r->seed = seed & ACC_UINT32L_C(0xffffffff);
}

ACC_LIBFUNC(acc_uint32l_t, acc_rand31) (acc_rand31_t* r)
{
    r->seed = (r->seed * ACC_UINT32L_C(1103515245)) + 12345;
    r->seed &= ACC_UINT32L_C(0x7fffffff);
    return r->seed;
}


#if defined(acc_uint64l_t)

ACC_LIBFUNC(void, acc_srand48) (acc_rand48_t* r, acc_uint32l_t seed)
{
    r->seed = seed & ACC_UINT32L_C(0xffffffff);
    r->seed <<= 16; r->seed |= 0x330e;
}

ACC_LIBFUNC(acc_uint32l_t, acc_rand48) (acc_rand48_t* r)
{
    r->seed = (r->seed * ACC_UINT64L_C(25214903917)) + 11;
    r->seed &= ACC_UINT64L_C(0xffffffffffff);
    return (acc_uint32l_t) (r->seed >> 17);
}

#endif /* defined(acc_uint64l_t) */


#if defined(acc_uint64l_t)

ACC_LIBFUNC(void, acc_srand64) (acc_rand64_t* r, acc_uint64l_t seed)
{
    r->seed = seed & ACC_UINT64L_C(0xffffffffffffffff);
}

ACC_LIBFUNC(acc_uint32l_t, acc_rand64) (acc_rand64_t* r)
{
    r->seed = (r->seed * ACC_UINT64L_C(6364136223846793005)) + 1;
    r->seed &= ACC_UINT64L_C(0xffffffffffffffff);
    return (acc_uint32l_t) (r->seed >> 33);
}

#endif /* defined(acc_uint64l_t) */


/*
vi:ts=4:et
*/
