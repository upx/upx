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
// huge pointer layer
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


/*************************************************************************
// wrap <dirent.h>
**************************************************************************/

#if (HAVE_DIRENT_H)

ACC_LIBFUNC(int, acc_opendir) (ACC_DIR* dirp, const char* name)
{
    dirp->dirp = opendir(name);
    return dirp->dirp ? 0 : -1;
}

ACC_LIBFUNC(int, acc_closedir) (ACC_DIR* dirp)
{
    int r = -1;
    if (!dirp->dirp)
        return r;
    r = closedir((DIR*) dirp->dirp);
    dirp->dirp = 0;
    return r;
}

ACC_LIBFUNC(int, acc_readdir) (ACC_DIR* dirp, struct acc_dirent* d)
{
    const struct dirent* dp;
    if (!dirp->dirp)
        return -1;
    dp = readdir((DIR*) dirp->dirp);
    if (!dp || !dp->d_name[0])
        return -1;
    if (strlen(dp->d_name) >= sizeof(d->d_name))
        return -1;
    strcpy(d->d_name, dp->d_name);
    return 0;
}

#else

ACC_LIBFUNC(int, acc_opendir) (ACC_DIR* dirp, const char* name)
{
    dirp->dirp = 0;
    ACC_UNUSED(name);
    return -1;
}

ACC_LIBFUNC(int, acc_closedir) (ACC_DIR* dirp)
{
    if (!dirp->dirp)
        return -1;
    dirp->dirp = 0;
    return -1;
}

ACC_LIBFUNC(int, acc_readdir) (ACC_DIR* dirp, struct acc_dirent* d)
{
    if (!dirp->dirp)
        return -1;
    d->d_name[0] = 0;
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
    if ((old_flags & 1) != (__djgpp_hwint_flags & 1))
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
    if (fd < 0)
        return 0;
#if (ACC_H_WINDOWS_H)
    /* work around naive library implementations that think that
     * any character device like `nul' is a tty */
    {
        long h = acc_get_osfhandle(fd);
        if (h != -1)
        {
            DWORD d = 0;
            int r = GetConsoleMode((HANDLE)h, &d);
            /* fprintf(stderr, "GetConsoleMode %d 0x%lx\n", r, (long) d); */
            if (!r)
                return 0;   /* GetConsoleMode failed -> not a tty */
        }
    }
#endif
    return (isatty(fd)) ? 1 : 0;
}


ACC_LIBFUNC(int, acc_mkdir) (const char* name, unsigned mode)
{
#if (ACC_OS_POSIX || ACC_OS_CYGWIN || ACC_OS_EMX)
    return mkdir(name, mode);
#elif defined(__DJGPP__) || defined(__MINT__)
    return mkdir(name, mode);
#elif (ACC_OS_TOS && (ACC_CC_PUREC || ACC_CC_TURBOC))
    ACC_UNUSED(mode);
    return Dcreate(name);
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
