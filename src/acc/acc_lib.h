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

/* stdlib */

/* stdio */



/*************************************************************************
// wrap <dirent.h>
**************************************************************************/

struct ACC_DIR { acc_hvoid_p dirp; };     /* opaque type */
typedef struct ACC_DIR ACC_DIR;
struct acc_dirent
{
    char d_name[255+1];
};

ACC_LIBFUNC(int, acc_opendir)  (ACC_DIR* dirp, const char* name);
ACC_LIBFUNC(int, acc_readdir)  (ACC_DIR* dirp, struct acc_dirent* d);
ACC_LIBFUNC(int, acc_closedir) (ACC_DIR* dirp);


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
