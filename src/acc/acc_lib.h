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


#ifndef __ACC_LIB_H_INCLUDED
#define __ACC_LIB_H_INCLUDED 1


#if !defined(__ACCLIB_FUNCNAME)
#  define __ACCLIB_FUNCNAME(f)  f
#endif
#if !defined(ACCLIB_EXTERN)
#  define ACCLIB_EXTERN(r,f)    extern r __ACCLIB_FUNCNAME(f)
#endif


#if !defined(__ACCLIB_CONST_CAST_RETURN)
#if (ACC_CC_GNUC)
#  define __ACCLIB_CONST_CAST_RETURN(type,var) \
        { union { type a; const type b; } u; u.b = (var); return u.a; }
#else
#  define __ACCLIB_CONST_CAST_RETURN(type,var) return (type) (var);
#endif
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
// wrap <ctype.h>
**************************************************************************/

#if 0
ACCLIB_EXTERN(int, acc_ascii_digit)   (int);
ACCLIB_EXTERN(int, acc_ascii_islower) (int);
ACCLIB_EXTERN(int, acc_ascii_isupper) (int);
ACCLIB_EXTERN(int, acc_ascii_tolower) (int);
ACCLIB_EXTERN(int, acc_ascii_toupper) (int);
ACCLIB_EXTERN(int, acc_ascii_utolower) (int);
ACCLIB_EXTERN(int, acc_ascii_utoupper) (int);
#endif

#define acc_ascii_isdigit(c)    (((unsigned)(c) - 48) < 10)
#define acc_ascii_islower(c)    (((unsigned)(c) - 97) < 26)
#define acc_ascii_isupper(c)    (((unsigned)(c) - 65) < 26)
#define acc_ascii_tolower(c)    ((int)(c) + (acc_ascii_isupper(c) << 5))
#define acc_ascii_toupper(c)    ((int)(c) - (acc_ascii_islower(c) << 5))

#define acc_ascii_utolower(c)   acc_ascii_tolower((unsigned char)(c))
#define acc_ascii_utoupper(c)   acc_ascii_toupper((unsigned char)(c))


/*************************************************************************
// huge pointer layer
**************************************************************************/

#ifndef acc_hsize_t
#if (ACC_HAVE_MM_HUGE_PTR)
#  define acc_hsize_t  unsigned long
#  define acc_hvoid_p  void __huge *
#  define acc_hchar_p  char __huge *
#  define acc_hchar_pp char __huge * __huge *
#  define acc_hbyte_p  unsigned char __huge *
#else
#  define acc_hsize_t  size_t
#  define acc_hvoid_p  void *
#  define acc_hchar_p  char *
#  define acc_hchar_pp char **
#  define acc_hbyte_p  unsigned char *
#endif
#endif
#ifndef ACC_FILE_P
#define ACC_FILE_P FILE *
#endif

/* halloc */
ACCLIB_EXTERN(acc_hvoid_p, acc_halloc) (acc_hsize_t);
ACCLIB_EXTERN(void, acc_hfree) (acc_hvoid_p);

/* dos_alloc */
#if (ACC_OS_DOS16 || ACC_OS_OS216)
ACCLIB_EXTERN(void __far*, acc_dos_alloc) (unsigned long);
ACCLIB_EXTERN(int, acc_dos_free) (void __far*);
#endif

/* string */
ACCLIB_EXTERN(int, acc_hmemcmp) (const acc_hvoid_p, const acc_hvoid_p, acc_hsize_t);
ACCLIB_EXTERN(acc_hvoid_p, acc_hmemcpy) (acc_hvoid_p, const acc_hvoid_p, acc_hsize_t);
ACCLIB_EXTERN(acc_hvoid_p, acc_hmemmove) (acc_hvoid_p, const acc_hvoid_p, acc_hsize_t);
ACCLIB_EXTERN(acc_hvoid_p, acc_hmemset) (acc_hvoid_p, int, acc_hsize_t);

/* string */
ACCLIB_EXTERN(acc_hsize_t, acc_hstrlen) (const acc_hchar_p);
ACCLIB_EXTERN(int, acc_hstrcmp) (const acc_hchar_p, const acc_hchar_p);
ACCLIB_EXTERN(int, acc_hstrncmp)(const acc_hchar_p, const acc_hchar_p, acc_hsize_t);
ACCLIB_EXTERN(int, acc_ascii_hstricmp) (const acc_hchar_p, const acc_hchar_p);
ACCLIB_EXTERN(int, acc_ascii_hstrnicmp)(const acc_hchar_p, const acc_hchar_p, acc_hsize_t);
ACCLIB_EXTERN(int, acc_ascii_hmemicmp) (const acc_hvoid_p, const acc_hvoid_p, acc_hsize_t);
ACCLIB_EXTERN(acc_hchar_p, acc_hstrstr) (const acc_hchar_p, const acc_hchar_p);
ACCLIB_EXTERN(acc_hchar_p, acc_ascii_hstristr) (const acc_hchar_p, const acc_hchar_p);
ACCLIB_EXTERN(acc_hvoid_p, acc_hmemmem) (const acc_hvoid_p, acc_hsize_t, const acc_hvoid_p, acc_hsize_t);
ACCLIB_EXTERN(acc_hvoid_p, acc_ascii_hmemimem) (const acc_hvoid_p, acc_hsize_t, const acc_hvoid_p, acc_hsize_t);
ACCLIB_EXTERN(acc_hchar_p, acc_hstrcpy) (acc_hchar_p, const acc_hchar_p);
ACCLIB_EXTERN(acc_hchar_p, acc_hstrcat) (acc_hchar_p, const acc_hchar_p);
ACCLIB_EXTERN(acc_hsize_t, acc_hstrlcpy) (acc_hchar_p, const acc_hchar_p, acc_hsize_t);
ACCLIB_EXTERN(acc_hsize_t, acc_hstrlcat) (acc_hchar_p, const acc_hchar_p, acc_hsize_t);
ACCLIB_EXTERN(int, acc_hstrscpy) (acc_hchar_p, const acc_hchar_p, acc_hsize_t);
ACCLIB_EXTERN(int, acc_hstrscat) (acc_hchar_p, const acc_hchar_p, acc_hsize_t);
ACCLIB_EXTERN(acc_hchar_p, acc_hstrccpy) (acc_hchar_p, const acc_hchar_p, int);
ACCLIB_EXTERN(acc_hvoid_p, acc_hmemccpy) (acc_hvoid_p, const acc_hvoid_p, int, acc_hsize_t);
ACCLIB_EXTERN(acc_hchar_p, acc_hstrchr)  (const acc_hchar_p, int);
ACCLIB_EXTERN(acc_hchar_p, acc_hstrrchr) (const acc_hchar_p, int);
ACCLIB_EXTERN(acc_hchar_p, acc_ascii_hstrichr) (const acc_hchar_p, int);
ACCLIB_EXTERN(acc_hchar_p, acc_ascii_hstrrichr) (const acc_hchar_p, int);
ACCLIB_EXTERN(acc_hvoid_p, acc_hmemchr)  (const acc_hvoid_p, int, acc_hsize_t);
ACCLIB_EXTERN(acc_hvoid_p, acc_hmemrchr) (const acc_hvoid_p, int, acc_hsize_t);
ACCLIB_EXTERN(acc_hvoid_p, acc_ascii_hmemichr) (const acc_hvoid_p, int, acc_hsize_t);
ACCLIB_EXTERN(acc_hvoid_p, acc_ascii_hmemrichr) (const acc_hvoid_p, int, acc_hsize_t);
ACCLIB_EXTERN(acc_hsize_t, acc_hstrspn)  (const acc_hchar_p, const acc_hchar_p);
ACCLIB_EXTERN(acc_hsize_t, acc_hstrrspn) (const acc_hchar_p, const acc_hchar_p);
ACCLIB_EXTERN(acc_hsize_t, acc_hstrcspn)  (const acc_hchar_p, const acc_hchar_p);
ACCLIB_EXTERN(acc_hsize_t, acc_hstrrcspn) (const acc_hchar_p, const acc_hchar_p);
ACCLIB_EXTERN(acc_hchar_p, acc_hstrpbrk)  (const acc_hchar_p, const acc_hchar_p);
ACCLIB_EXTERN(acc_hchar_p, acc_hstrrpbrk) (const acc_hchar_p, const acc_hchar_p);
ACCLIB_EXTERN(acc_hchar_p, acc_hstrsep)  (acc_hchar_pp, const acc_hchar_p);
ACCLIB_EXTERN(acc_hchar_p, acc_hstrrsep) (acc_hchar_pp, const acc_hchar_p);
ACCLIB_EXTERN(acc_hchar_p, acc_ascii_hstrlwr) (acc_hchar_p);
ACCLIB_EXTERN(acc_hchar_p, acc_ascii_hstrupr) (acc_hchar_p);
ACCLIB_EXTERN(acc_hvoid_p, acc_ascii_hmemlwr) (acc_hvoid_p, acc_hsize_t);
ACCLIB_EXTERN(acc_hvoid_p, acc_ascii_hmemupr) (acc_hvoid_p, acc_hsize_t);

/* stdio */
ACCLIB_EXTERN(acc_hsize_t, acc_hfread) (ACC_FILE_P, acc_hvoid_p, acc_hsize_t);
ACCLIB_EXTERN(acc_hsize_t, acc_hfwrite) (ACC_FILE_P, const acc_hvoid_p, acc_hsize_t);

/* io */
#if (ACC_HAVE_MM_HUGE_PTR)
ACCLIB_EXTERN(long, acc_hread) (int, acc_hvoid_p, long);
ACCLIB_EXTERN(long, acc_hwrite) (int, const acc_hvoid_p, long);
#endif
ACCLIB_EXTERN(long, acc_safe_hread) (int, acc_hvoid_p, long);
ACCLIB_EXTERN(long, acc_safe_hwrite) (int, const acc_hvoid_p, long);


/*************************************************************************
// wrap filename limits
**************************************************************************/

/* maximum length of full pathname (excl. '\0') */
#if !defined(ACC_FN_PATH_MAX)
#if (ACC_OS_DOS16 || ACC_OS_WIN16)
#  define ACC_FN_PATH_MAX   143
#elif (ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_WIN32 || ACC_OS_WIN64)
#  define ACC_FN_PATH_MAX   259
#elif (ACC_OS_TOS)
#  define ACC_FN_PATH_MAX   259
#endif
#endif
#if !defined(ACC_FN_PATH_MAX)
   /* arbitrary limit for acclib implementation */
#  define ACC_FN_PATH_MAX   1024
#endif

/* maximum length of a filename (a single path component) (excl. '\0') */
#if !defined(ACC_FN_NAME_MAX)
#if (ACC_OS_DOS16 || ACC_OS_WIN16)
#  define ACC_FN_NAME_MAX   12
#elif (ACC_OS_TOS && (ACC_CC_PUREC || ACC_CC_TURBOC))
#  define ACC_FN_NAME_MAX   12
#elif (ACC_OS_DOS32 && ACC_CC_GNUC) && defined(__DJGPP__)
#elif (ACC_OS_DOS32)
#  define ACC_FN_NAME_MAX   12
#endif
#endif
#if !defined(ACC_FN_NAME_MAX)
#  define ACC_FN_NAME_MAX   ACC_FN_PATH_MAX
#endif


#define ACC_FNMATCH_NOESCAPE        1
#define ACC_FNMATCH_PATHNAME        2
#define ACC_FNMATCH_PATHSTAR        4
#define ACC_FNMATCH_PERIOD          8
#define ACC_FNMATCH_ASCII_CASEFOLD  16
ACCLIB_EXTERN(int, acc_fnmatch) (const acc_hchar_p, const acc_hchar_p, int);


/*************************************************************************
// wrap <dirent.h>
**************************************************************************/

#undef __ACCLIB_USE_OPENDIR
#if (HAVE_DIRENT_H || ACC_CC_WATCOMC)
#  define __ACCLIB_USE_OPENDIR 1
#  if (ACC_OS_DOS32 && defined(__BORLANDC__))
#  elif (ACC_OS_DOS32 && ACC_CC_GNUC) && defined(__DJGPP__)
#  elif (ACC_OS_OS2 || ACC_OS_OS216)
#  elif (ACC_OS_TOS && ACC_CC_GNUC)
#  elif (ACC_OS_WIN32 && !defined(ACC_HAVE_WINDOWS_H))
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

#ifndef acc_dir_p
#define acc_dir_p acc_dir_t *
#endif

ACCLIB_EXTERN(int, acc_opendir)  (acc_dir_p, const char*);
ACCLIB_EXTERN(int, acc_readdir)  (acc_dir_p);
ACCLIB_EXTERN(int, acc_closedir) (acc_dir_p);


/*************************************************************************
// wrap misc
**************************************************************************/

#if (ACC_CC_GNUC) && (defined(__CYGWIN__) || defined(__MINGW32__))
#  define acc_alloca(x)     __builtin_alloca((x))
#elif (ACC_CC_BORLANDC) && defined(__linux__)
  /* FIXME: alloca does not work */
#elif (HAVE_ALLOCA)
#  define acc_alloca(x)     alloca((x))
#endif

#if (ACC_OS_DOS32 && ACC_CC_GNUC) && defined(__DJGPP__)
#  define acc_stackavail()  stackavail()
#elif (ACC_ARCH_IA16 && ACC_CC_BORLANDC && (__BORLANDC__ >= 0x0410))
#  define acc_stackavail()  stackavail()
#elif (ACC_ARCH_IA16 && ACC_CC_BORLANDC && (__BORLANDC__ >= 0x0400))
#  if (ACC_OS_WIN16) && (ACC_MM_TINY || ACC_MM_SMALL || ACC_MM_MEDIUM)
#  else
#    define acc_stackavail()  stackavail()
#  endif
#elif ((ACC_ARCH_IA16 || ACC_ARCH_IA32) && (ACC_CC_DMC || ACC_CC_SYMANTECC))
#  define acc_stackavail()  stackavail()
#elif ((ACC_ARCH_IA16) && ACC_CC_MSC && (_MSC_VER >= 700))
#  define acc_stackavail()  _stackavail()
#elif ((ACC_ARCH_IA16) && ACC_CC_MSC)
#  define acc_stackavail()  stackavail()
#elif ((ACC_ARCH_IA16 || ACC_ARCH_IA32) && ACC_CC_TURBOC && (__TURBOC__ >= 0x0450))
#  define acc_stackavail()  stackavail()
#elif (ACC_ARCH_IA16 && ACC_CC_TURBOC && (__TURBOC__ >= 0x0400))
   ACC_EXTERN_C size_t __cdecl stackavail(void);
#  define acc_stackavail()  stackavail()
#elif ((ACC_ARCH_IA16 || ACC_ARCH_IA32) && (ACC_CC_WATCOMC))
#  define acc_stackavail()  stackavail()
#elif (ACC_ARCH_IA16 && ACC_CC_ZORTECHC)
#  define acc_stackavail()  _chkstack()
#endif

ACCLIB_EXTERN(acclib_handle_t, acc_get_osfhandle) (int);
ACCLIB_EXTERN(int, acc_isatty) (int);
ACCLIB_EXTERN(int, acc_mkdir) (const char*, unsigned);
ACCLIB_EXTERN(int, acc_response) (int*, char***);
ACCLIB_EXTERN(int, acc_set_binmode) (int, int);

ACCLIB_EXTERN(acc_int32l_t, acc_muldiv32) (acc_int32l_t, acc_int32l_t, acc_int32l_t);
ACCLIB_EXTERN(acc_uint32l_t, acc_umuldiv32) (acc_uint32l_t, acc_uint32l_t, acc_uint32l_t);

ACCLIB_EXTERN(void, acc_wildargv) (int*, char***);


/*************************************************************************
// uclock (real, i.e. "wall" clock)
**************************************************************************/

#if defined(acc_int32e_t)
ACCLIB_EXTERN(int, acc_tsc_read) (acc_uint32e_t*);
ACCLIB_EXTERN(int, acc_tsc_read_add) (acc_uint32e_t*);
#endif


typedef struct { /* all private */
    acclib_handle_t h;
    int mode;
#if (ACC_HAVE_WINDOWS_H) && defined(acc_int64l_t)
    double qpf;
#endif
} acc_uclock_handle_t;

typedef struct { /* all private */
    union {
        acc_uint32l_t t32;
#if !(ACC_OS_DOS16 || ACC_OS_WIN16)
        double td;
#  if defined(acc_int64l_t)
        acc_int64l_t t64;
#  endif
#endif
    } ticks;
#if (ACC_HAVE_WINDOWS_H) && defined(acc_int64l_t)
    acc_int64l_t qpc;
#endif
} acc_uclock_t;

#ifndef acc_uclock_handle_p
#define acc_uclock_handle_p acc_uclock_handle_t *
#endif
#ifndef acc_uclock_p
#define acc_uclock_p acc_uclock_t *
#endif

ACCLIB_EXTERN(int, acc_uclock_open)  (acc_uclock_handle_p);
ACCLIB_EXTERN(int, acc_uclock_close) (acc_uclock_handle_p);
ACCLIB_EXTERN(void, acc_uclock_read) (acc_uclock_handle_p, acc_uclock_p);
ACCLIB_EXTERN(double, acc_uclock_get_elapsed) (acc_uclock_handle_p, const acc_uclock_p, const acc_uclock_p);


/*************************************************************************
// performance counters (virtual clock)
**************************************************************************/

#if defined(acc_int64l_t)

typedef struct { /* all private */
    void* h;
    unsigned cpu_type, cpu_features, cpu_khz, cpu_nrctrs;
    const char* cpu_name;
} acc_perfctr_handle_t;

typedef struct {
    acc_uint64l_t tsc;
#if (ACC_OS_POSIX_LINUX)
    acc_uint64l_t pmc[18];
#else
    acc_uint64l_t pmc[1];
#endif
} acc_perfctr_clock_t;

#ifndef acc_perfctr_handle_p
#define acc_perfctr_handle_p acc_perfctr_handle_t *
#endif
#ifndef acc_perfctr_clock_p
#define acc_perfctr_clock_p acc_perfctr_clock_t *
#endif

ACCLIB_EXTERN(int, acc_perfctr_open)  (acc_perfctr_handle_p);
ACCLIB_EXTERN(int, acc_perfctr_close) (acc_perfctr_handle_p);
ACCLIB_EXTERN(void, acc_perfctr_read) (acc_perfctr_handle_p, acc_perfctr_clock_p);
ACCLIB_EXTERN(double, acc_perfctr_get_elapsed) (acc_perfctr_handle_p, const acc_perfctr_clock_p, const acc_perfctr_clock_p);
ACCLIB_EXTERN(double, acc_perfctr_get_elapsed_tsc) (acc_perfctr_handle_p, acc_uint64l_t);

#endif


/*************************************************************************
// Big Endian / Little Endian
**************************************************************************/

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
#if (ACC_ARCH_AMD64 || ACC_ARCH_IA32)
#  define ACC_GET_LE16(p)       (* (const unsigned short *) (p))
#  define ACC_GET_LE32(p)       (* (const acc_uint32e_t *) (p))
#  define ACC_SET_LE16(p,v)     (* (unsigned short *) (p) = (unsigned short) (v))
#  define ACC_SET_LE32(p,v)     (* (acc_uint32e_t *) (p) = (acc_uint32e_t) (v))
#endif
#if (ACC_ARCH_AMD64)
#  define ACC_GET_LE64(p)       (* (const acc_uint64l_t *) (p))
#  define ACC_SET_LE64(p,v)     (* (acc_uint64l_t *) (p) = (acc_uint64l_t) (v))
#endif


/*************************************************************************
// getopt
**************************************************************************/

typedef struct {
    const char* name;
    int has_arg;
    int* flag;
    int val;
} acc_getopt_longopt_t;
#ifndef acc_getopt_longopt_p
#define acc_getopt_longopt_p acc_getopt_longopt_t *
#endif

typedef struct {
    int go_argc;
    char** go_argv;
    const char* go_shortopts;
    const acc_getopt_longopt_p longopts;
#if (ACC_BROKEN_CDECL_ALT_SYNTAX)
    int __acc_cdecl_va (*go_error)(const char *, ...);
#else
    int (__acc_cdecl_va *go_error)(const char *, ...);
#endif
} acc_getopt_t;
#ifndef acc_getopt_p
#define acc_getopt_p acc_getopt_t *
#endif

ACCLIB_EXTERN(void, acc_getopt_init) (acc_getopt_p);
ACCLIB_EXTERN(int, acc_getopt) (acc_getopt_p);
ACCLIB_EXTERN(void, acc_getopt_close)(acc_getopt_p);


/*************************************************************************
// rand
**************************************************************************/

typedef struct { /* all private */
    acc_uint32l_t seed;
} acc_rand31_t;
#ifndef acc_rand31_p
#define acc_rand31_p acc_rand31_t *
#endif
ACCLIB_EXTERN(void, acc_srand31) (acc_rand31_p, acc_uint32l_t);
ACCLIB_EXTERN(acc_uint32l_t, acc_rand31) (acc_rand31_p);

#if defined(acc_uint64l_t)
typedef struct { /* all private */
    acc_uint64l_t seed;
} acc_rand48_t;
#ifndef acc_rand48_p
#define acc_rand48_p acc_rand48_t *
#endif
ACCLIB_EXTERN(void, acc_srand48) (acc_rand48_p, acc_uint32l_t);
ACCLIB_EXTERN(acc_uint32l_t, acc_rand48) (acc_rand48_p);
ACCLIB_EXTERN(acc_uint32l_t, acc_rand48_r32) (acc_rand48_p);
#endif

#if defined(acc_uint64l_t)
typedef struct { /* all private */
    acc_uint64l_t seed;
} acc_rand64_t;
#ifndef acc_rand64_p
#define acc_rand64_p acc_rand64_t *
#endif
ACCLIB_EXTERN(void, acc_srand64) (acc_rand64_p, acc_uint64l_t);
ACCLIB_EXTERN(acc_uint32l_t, acc_rand64) (acc_rand64_p);
ACCLIB_EXTERN(acc_uint32l_t, acc_rand64_r32) (acc_rand64_p);
#endif

typedef struct { /* all private */
    unsigned n;
    acc_uint32l_t s[624];
} acc_randmt_t;
#ifndef acc_randmt_p
#define acc_randmt_p acc_randmt_t *
#endif
ACCLIB_EXTERN(void, acc_srandmt) (acc_randmt_p, acc_uint32l_t);
ACCLIB_EXTERN(acc_uint32l_t, acc_randmt) (acc_randmt_p);
ACCLIB_EXTERN(acc_uint32l_t, acc_randmt_r32) (acc_randmt_p);


#endif /* already included */


/*
vi:ts=4:et
*/
