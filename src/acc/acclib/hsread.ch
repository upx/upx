/* ACC -- Automatic Compiler Configuration

   Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   This software is a copyrighted work licensed under the terms of
   the GNU General Public License. Please consult the file "ACC_LICENSE"
   for details.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/
 */


#define __ACCLIB_HSREAD_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif


/***********************************************************************
// huge pointer layer - safe read/write
// handles partial pipe writes and interrupted system calls
************************************************************************/

ACCLIB_PUBLIC(long, acc_safe_hread) (int fd, acc_hvoid_p buf, long size)
{
    acc_hbyte_p b = (acc_hbyte_p) buf;
    long l = 0;
    int saved_errno;

    saved_errno = errno;
    while (l < size)
    {
        long n = size - l;
#if (ACC_HAVE_MM_HUGE_PTR)
#  define __ACCLIB_REQUIRE_HREAD_CH 1
        errno = 0; n = acc_hread(fd, b, n);
#elif (ACC_OS_DOS32) && defined(__DJGPP__)
        errno = 0; n = _read(fd, b, n);
#else
        errno = 0; n = read(fd, b, n);
#endif
        if (n == 0)
            break;
        if (n < 0) {
#if defined(EINTR)
            if (errno == (EINTR)) continue;
#endif
            if (errno == 0) errno = 1;
            return l;
        }
        b += n; l += n;
    }
    errno = saved_errno;
    return l;
}


ACCLIB_PUBLIC(long, acc_safe_hwrite) (int fd, const acc_hvoid_p buf, long size)
{
    const acc_hbyte_p b = (const acc_hbyte_p) buf;
    long l = 0;
    int saved_errno;

    saved_errno = errno;
    while (l < size)
    {
        long n = size - l;
#if (ACC_HAVE_MM_HUGE_PTR)
#  define __ACCLIB_REQUIRE_HREAD_CH 1
        errno = 0; n = acc_hwrite(fd, b, n);
#elif (ACC_OS_DOS32) && defined(__DJGPP__)
        errno = 0; n = _write(fd, b, n);
#else
        errno = 0; n = write(fd, b, n);
#endif
        if (n == 0)
            break;
        if (n < 0) {
#if defined(EINTR)
            if (errno == (EINTR)) continue;
#endif
            if (errno == 0) errno = 1;
            return l;
        }
        b += n; l += n;
    }
    errno = saved_errno;
    return l;
}


/*
vi:ts=4:et
*/
