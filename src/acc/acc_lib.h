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


#ifndef __ACC_LIB_H_INCLUDED
#define __ACC_LIB_H_INCLUDED


#if !defined(ACC_LIBFUNC)
#  define ACC_LIBFUNC(a,b)  a b
#endif


/*************************************************************************
// huge pointer layer
**************************************************************************/

#if (ACC_HAVE_MM_HUGE_PTR)
#  define __acc_huge __huge
#  define acc_hsize_t unsigned long
#else
#  define __acc_huge
#  define acc_hsize_t size_t
#endif
#define acc_hvoid_p void __acc_huge *
#define acc_hbyte_p unsigned char __acc_huge *

typedef struct
{
    acc_hvoid_p ptr;
}
acc_alloc_t;
#define acc_alloc_p acc_alloc_t __acc_huge *

/* malloc */
ACC_LIBFUNC(acc_hvoid_p, acc_halloc) (acc_alloc_p ap, acc_hsize_t items, size_t size);
ACC_LIBFUNC(int, acc_hfree) (acc_alloc_p ap);

/* string */
ACC_LIBFUNC(int, acc_hmemcmp) (const acc_hvoid_p s1, const acc_hvoid_p s2, acc_hsize_t len);
ACC_LIBFUNC(acc_hvoid_p, acc_hmemcpy) (acc_hvoid_p dest, const acc_hvoid_p src, acc_hsize_t len);
ACC_LIBFUNC(acc_hvoid_p, acc_hmemmove) (acc_hvoid_p dest, const acc_hvoid_p src, acc_hsize_t len);
ACC_LIBFUNC(acc_hvoid_p, acc_hmemset) (acc_hvoid_p s, int c, acc_hsize_t len);

/* stdio */
ACC_LIBFUNC(acc_hsize_t, acc_hfread) (FILE *fp, acc_hvoid_p buf, acc_hsize_t size);
ACC_LIBFUNC(acc_hsize_t, acc_hfwrite) (FILE *fp, const acc_hvoid_p buf, acc_hsize_t size);
#if (ACC_HAVE_MM_HUGE_PTR)
ACC_LIBFUNC(long, acc_hread) (int fd, acc_hvoid_p buf, long size);
ACC_LIBFUNC(long, acc_hwrite) (int fd, const acc_hvoid_p buf, long size);
#endif


/*************************************************************************
// wrap <dirent.h>
**************************************************************************/

struct acc_find_t
{
    union { acc_hvoid_p dirp; long h; } u;  /* private */
#if (ACC_OS_DOS16 || ACC_OS_WIN16)
    char f_name[8+1+3+1];
#elif (ACC_DOS32 || ACC_OS_WIN32 || ACC_OS_WIN64)
    char f_name[259+1];
#else
    char f_name[1024+1];
#endif
};

ACC_LIBFUNC(int, acc_findfirst) (struct acc_find_t* f, const char* path);
ACC_LIBFUNC(int, acc_findnext)  (struct acc_find_t* f);
ACC_LIBFUNC(int, acc_findclose) (struct acc_find_t* f);


/*************************************************************************
// wrap <fnmatch.h>
**************************************************************************/


/*************************************************************************
// wrap <getopt.h>
**************************************************************************/


/*************************************************************************
// wrap <string.h>
**************************************************************************/


/*************************************************************************
// wrap misc
**************************************************************************/

#if defined(__CYGWIN__) || defined(__MINGW32__)
#  define acc_alloca(x)     __builtin_alloca((x))
#elif defined(__BORLANDC__) && defined(__linux__)
  /* FIXME: this does not work */
#elif (HAVE_ALLOCA)
#  define acc_alloca(x)     alloca((x))
#endif

ACC_LIBFUNC(long, acc_get_osfhandle) (int fd);
ACC_LIBFUNC(int,  acc_isatty) (int fd);
ACC_LIBFUNC(int,  acc_mkdir) (const char* name, unsigned mode);
ACC_LIBFUNC(int,  acc_response) (int* argc, char*** argv);
ACC_LIBFUNC(int,  acc_set_binmode) (int fd, int binary);

#endif /* already included */


/*
vi:ts=4:et
*/
