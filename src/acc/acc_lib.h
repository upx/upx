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


#if !defined(__ACCLIB_FUNCNAME)
#  define __ACCLIB_FUNCNAME(f)  f
#endif
#if !defined(ACCLIB_EXTERN)
#  define ACCLIB_EXTERN(r,f)    extern r __ACCLIB_FUNCNAME(f)
#endif


#if (ACC_OS_WIN64)
#  define acclib_handle_t       acc_int64l_t
#  define acclib_uhandle_t      acc_uint64l_t
#elif (ACC_ARCH_IA32 && ACC_CC_MSC && (_MSC_VER >= 1300))
   typedef __w64 long           acclib_handle_t;
   typedef __w64 unsigned long  acclib_uhandle_t;
#  define acclib_handle_t       acclib_handle_t
#  define acclib_uhandle_t      acclib_uhandle_t
#else
#  define acclib_handle_t       long
#  define acclib_uhandle_t      unsigned long
#endif


/*************************************************************************
// huge pointer layer
**************************************************************************/

#if (ACC_HAVE_MM_HUGE_PTR)
#  define acc_hsize_t  unsigned long
#  define acc_hvoid_p  void __huge *
#  define acc_hbyte_p  unsigned char __huge *
#else
#  define acc_hsize_t  size_t
#  define acc_hvoid_p  void *
#  define acc_hbyte_p  unsigned char *
#endif

/* halloc */
ACCLIB_EXTERN(acc_hvoid_p, acc_halloc) (acc_hsize_t size);
ACCLIB_EXTERN(void, acc_hfree) (acc_hvoid_p p);

#if (ACC_OS_DOS16 || ACC_OS_OS216)
ACCLIB_EXTERN(void __far*, acc_dos_alloc) (unsigned long size);
ACCLIB_EXTERN(int, acc_dos_free) (void __far* p);
#endif

/* string */
ACCLIB_EXTERN(int, acc_hmemcmp) (const acc_hvoid_p s1, const acc_hvoid_p s2, acc_hsize_t len);
ACCLIB_EXTERN(acc_hvoid_p, acc_hmemcpy) (acc_hvoid_p dest, const acc_hvoid_p src, acc_hsize_t len);
ACCLIB_EXTERN(acc_hvoid_p, acc_hmemmove) (acc_hvoid_p dest, const acc_hvoid_p src, acc_hsize_t len);
ACCLIB_EXTERN(acc_hvoid_p, acc_hmemset) (acc_hvoid_p s, int c, acc_hsize_t len);

/* stdio */
ACCLIB_EXTERN(acc_hsize_t, acc_hfread) (FILE* fp, acc_hvoid_p buf, acc_hsize_t size);
ACCLIB_EXTERN(acc_hsize_t, acc_hfwrite) (FILE* fp, const acc_hvoid_p buf, acc_hsize_t size);

#if (ACC_HAVE_MM_HUGE_PTR)
ACCLIB_EXTERN(long, acc_hread) (int fd, acc_hvoid_p buf, long size);
ACCLIB_EXTERN(long, acc_hwrite) (int fd, const acc_hvoid_p buf, long size);
#endif


/*************************************************************************
// wrap filename limits
**************************************************************************/

/* maximum length of full pathname (excl. '\0') */
#if (ACC_OS_DOS16 || ACC_OS_WIN16)
#  define ACC_FN_PATH_MAX   143
#elif (ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_WIN32 || ACC_OS_WIN64)
#  define ACC_FN_PATH_MAX   259
#elif (ACC_OS_TOS)
#  define ACC_FN_PATH_MAX   259
#else
   /* arbitrary limit for acclib implementation */
#  define ACC_FN_PATH_MAX   1024
#endif

/* maximum length of a filename (a single path component) (excl. '\0') */
#if (ACC_OS_DOS16 || ACC_OS_WIN16)
#  define ACC_FN_NAME_MAX   12
#elif (ACC_OS_TOS && (ACC_CC_PUREC || ACC_CC_TURBOC))
#  define ACC_FN_NAME_MAX   12
#elif (ACC_OS_DOS32 && defined(__DJGPP__))
#elif (ACC_OS_DOS32)
#  define ACC_FN_NAME_MAX   12
#endif

#if !defined(ACC_FN_NAME_MAX)
#  define ACC_FN_NAME_MAX   ACC_FN_PATH_MAX
#endif


/*************************************************************************
// wrap <dirent.h>
**************************************************************************/

#undef __ACCLIB_USE_OPENDIR
#if (HAVE_DIRENT_H || ACC_CC_WATCOMC)
#  define __ACCLIB_USE_OPENDIR 1
#  if (ACC_OS_DOS32 && defined(__BORLANDC__))
#  elif (ACC_OS_DOS32 && defined(__DJGPP__))
#  elif (ACC_OS_OS2 || ACC_OS_OS216)
#  elif (ACC_OS_TOS && ACC_CC_GNUC)
#  elif (ACC_OS_WIN32 && !defined(ACC_H_WINDOWS_H))
#  elif (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_TOS || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)
#    undef __ACCLIB_USE_OPENDIR
#  endif
#endif


typedef struct
{
#if defined(__ACCLIB_USE_OPENDIR)
    void *u_dirp; /* private */
# if (ACC_CC_WATCOMC)
    unsigned short f_time;
    unsigned short f_date;
    unsigned long f_size;
# endif
    char f_name[ACC_FN_NAME_MAX+1];
#elif (ACC_OS_WIN32 || ACC_OS_WIN64)
    acclib_handle_t u_handle; /* private */
    unsigned f_attr;
    unsigned f_size_low;
    unsigned f_size_high;
    char f_name[ACC_FN_NAME_MAX+1];
#elif (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_TOS || ACC_OS_WIN16)
    char u_dta[21]; /* private */
    unsigned char f_attr;
    unsigned short f_time;
    unsigned short f_date;
    unsigned short f_size_low;
    unsigned short f_size_high;
    char f_name[ACC_FN_NAME_MAX+1];
    char u_dirp; /* private */
#else
    void *u_dirp; /* private */
    char f_name[ACC_FN_NAME_MAX+1];
#endif
} acc_dir_t;

ACCLIB_EXTERN(int, acc_opendir)  (acc_dir_t* d, const char* path);
ACCLIB_EXTERN(int, acc_readdir)  (acc_dir_t* d);
ACCLIB_EXTERN(int, acc_closedir) (acc_dir_t* d);


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

#undef acc_alloca
#if defined(__CYGWIN__) || defined(__MINGW32__)
#  define acc_alloca(x)     __builtin_alloca((x))
#elif defined(__BORLANDC__) && defined(__linux__)
  /* FIXME: alloca does not work */
#elif (HAVE_ALLOCA)
#  define acc_alloca(x)     alloca((x))
#endif

ACCLIB_EXTERN(acclib_handle_t, acc_get_osfhandle) (int fd);
ACCLIB_EXTERN(int,  acc_isatty) (int fd);
ACCLIB_EXTERN(int,  acc_mkdir) (const char* name, unsigned mode);
ACCLIB_EXTERN(int,  acc_response) (int* argc, char*** argv);
ACCLIB_EXTERN(int,  acc_set_binmode) (int fd, int binary);

#if defined(acc_uint64l_t)
#  define acc_uclock_t      acc_uint64l_t
#else
#  define acc_uclock_t      acc_uint32l_t
#endif
ACCLIB_EXTERN(acc_uclock_t, acc_uclock) (void);

ACCLIB_EXTERN(unsigned, acc_get_be16) (const acc_hvoid_p);
ACCLIB_EXTERN(acc_uint32l_t, acc_get_be24) (const acc_hvoid_p);
ACCLIB_EXTERN(acc_uint32l_t, acc_get_be32) (const acc_hvoid_p);
ACCLIB_EXTERN(void, acc_set_be16) (acc_hvoid_p, unsigned);
ACCLIB_EXTERN(void, acc_set_be24) (acc_hvoid_p, acc_uint32l_t);
ACCLIB_EXTERN(void, acc_set_be32) (acc_hvoid_p, acc_uint32l_t);
ACCLIB_EXTERN(unsigned, acc_get_le16) (const acc_hvoid_p);
ACCLIB_EXTERN(acc_uint32l_t, acc_get_le24) (const acc_hvoid_p);
ACCLIB_EXTERN(acc_uint32l_t, acc_get_le32) (const acc_hvoid_p);
ACCLIB_EXTERN(void, acc_set_le16) (acc_hvoid_p, unsigned);
ACCLIB_EXTERN(void, acc_set_le24) (acc_hvoid_p, acc_uint32l_t);
ACCLIB_EXTERN(void, acc_set_le32) (acc_hvoid_p, acc_uint32l_t);
#if defined(acc_uint64l_t)
ACCLIB_EXTERN(acc_uint64l_t, acc_get_be64) (const acc_hvoid_p);
ACCLIB_EXTERN(void, acc_set_be64) (acc_hvoid_p, acc_uint64l_t);
ACCLIB_EXTERN(acc_uint64l_t, acc_get_le64) (const acc_hvoid_p);
ACCLIB_EXTERN(void, acc_set_le64) (acc_hvoid_p, acc_uint64l_t);
#endif
/* inline versions */
#if (ACC_ARCH_IA32)
#  define ACC_GET_LE16(p)       (* (const unsigned short *) (p))
#  define ACC_GET_LE32(p)       (* (const unsigned int *) (p))
#  define ACC_SET_LE16(p,v)     (* (unsigned short *) (p) = (unsigned short) (v))
#  define ACC_SET_LE32(p,v)     (* (unsigned int *) (p) = (unsigned int) (v))
#endif

ACCLIB_EXTERN(acc_int32l_t, acc_muldiv32) (acc_int32l_t, acc_int32l_t, acc_int32l_t);
ACCLIB_EXTERN(acc_uint32l_t, acc_umuldiv32) (acc_uint32l_t, acc_uint32l_t, acc_uint32l_t);

typedef struct {
    acc_uint32l_t seed;
} acc_rand31_t;
ACCLIB_EXTERN(void, acc_srand31) (acc_rand31_t* r, acc_uint32l_t seed);
ACCLIB_EXTERN(acc_uint32l_t, acc_rand31) (acc_rand31_t* r);

#if defined(acc_uint64l_t)
typedef struct {
    acc_uint64l_t seed;
} acc_rand48_t;
ACCLIB_EXTERN(void, acc_srand48) (acc_rand48_t* r, acc_uint32l_t seed);
ACCLIB_EXTERN(acc_uint32l_t, acc_rand48) (acc_rand48_t* r);
#endif /* defined(acc_uint64l_t) */

#if defined(acc_uint64l_t)
typedef struct {
    acc_uint64l_t seed;
} acc_rand64_t;
ACCLIB_EXTERN(void, acc_srand64) (acc_rand64_t* r, acc_uint64l_t seed);
ACCLIB_EXTERN(acc_uint32l_t, acc_rand64) (acc_rand64_t* r);
#endif /* defined(acc_uint64l_t) */


#endif /* already included */


/*
vi:ts=4:et
*/
