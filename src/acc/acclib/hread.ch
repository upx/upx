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


#define __ACCLIB_HREAD_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif


/***********************************************************************
// huge pointer layer - read/write
************************************************************************/

#if (ACC_HAVE_MM_HUGE_PTR)

ACCLIB_PUBLIC(long, acc_hread) (int fd, acc_hvoid_p buf, long size)
{
#if (ACC_MM_TINY || ACC_MM_SMALL || ACC_MM_MEDIUM)
#define __ACCLIB_REQUIRE_HMEMCPY_CH 1
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
        __ACCLIB_FUNCNAME(acc_hmemcpy)((acc_hbyte_p)buf + l, tmp, (acc_hsize_t)n);
        l += n;
    }
    return l;
#elif (ACC_MM_COMPACT || ACC_MM_LARGE || ACC_MM_HUGE)
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
#  error "unknown memory model"
#endif
}


ACCLIB_PUBLIC(long, acc_hwrite) (int fd, const acc_hvoid_p buf, long size)
{
#if (ACC_MM_TINY || ACC_MM_SMALL || ACC_MM_MEDIUM)
#define __ACCLIB_REQUIRE_HMEMCPY_CH 1
    unsigned char tmp[512];
    long l = 0;

    while (l < size)
    {
        int n = size - l > (long)sizeof(tmp) ? (int) sizeof(tmp) : (int) (size - l);
        __ACCLIB_FUNCNAME(acc_hmemcpy)(tmp, (const acc_hbyte_p)buf + l, (acc_hsize_t)n);
        n = write(fd, tmp, n);
        if (n == 0)
            break;
        if (n < 0)
            return -1;
        l += n;
    }
    return l;
#elif (ACC_MM_COMPACT || ACC_MM_LARGE || ACC_MM_HUGE)
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
#  error "unknown memory model"
#endif
}

#endif /* #if (ACC_HAVE_MM_HUGE_PTR) */


/*
vi:ts=4:et
*/
