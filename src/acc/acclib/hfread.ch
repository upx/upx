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


#define __ACCLIB_HFREAD_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif


/***********************************************************************
// huge pointer layer - stdio.h
************************************************************************/

ACCLIB_PUBLIC(acc_hsize_t, acc_hfread) (ACC_FILE_P fp, acc_hvoid_p buf, acc_hsize_t size)
{
#if (ACC_HAVE_MM_HUGE_PTR)
#if (ACC_MM_TINY || ACC_MM_SMALL || ACC_MM_MEDIUM)
#define __ACCLIB_REQUIRE_HMEMCPY_CH 1
    unsigned char tmp[512];
    acc_hsize_t l = 0;

    while (l < size)
    {
        size_t n = size - l > sizeof(tmp) ? sizeof(tmp) : (size_t) (size - l);
        n = fread(tmp, 1, n, fp);
        if (n == 0)
            break;
        __ACCLIB_FUNCNAME(acc_hmemcpy)((acc_hbyte_p)buf + l, tmp, (acc_hsize_t)n);
        l += n;
    }
    return l;
#elif (ACC_MM_COMPACT || ACC_MM_LARGE || ACC_MM_HUGE)
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
#  error "unknown memory model"
#endif
#else
    return fread(buf, 1, size, fp);
#endif
}


ACCLIB_PUBLIC(acc_hsize_t, acc_hfwrite) (ACC_FILE_P fp, const acc_hvoid_p buf, acc_hsize_t size)
{
#if (ACC_HAVE_MM_HUGE_PTR)
#if (ACC_MM_TINY || ACC_MM_SMALL || ACC_MM_MEDIUM)
#define __ACCLIB_REQUIRE_HMEMCPY_CH 1
    unsigned char tmp[512];
    acc_hsize_t l = 0;

    while (l < size)
    {
        size_t n = size - l > sizeof(tmp) ? sizeof(tmp) : (size_t) (size - l);
        __ACCLIB_FUNCNAME(acc_hmemcpy)(tmp, (const acc_hbyte_p)buf + l, (acc_hsize_t)n);
        n = fwrite(tmp, 1, n, fp);
        if (n == 0)
            break;
        l += n;
    }
    return l;
#elif (ACC_MM_COMPACT || ACC_MM_LARGE || ACC_MM_HUGE)
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
#  error "unknown memory model"
#endif
#else
    return fwrite(buf, 1, size, fp);
#endif
}


/*
vi:ts=4:et
*/
