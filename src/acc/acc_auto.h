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


/*
 * Possible configuration values:
 *
 *   ACC_CONFIG_AUTO_NO_HEADERS
 *   ACC_CONFIG_AUTO_NO_FUNCTIONS
 *   ACC_CONFIG_AUTO_NO_SIZES
 */


/*************************************************************************
// Checks for <stdint.h>
**************************************************************************/

#if !defined(ACC_CONFIG_AUTO_NO_HEADERS)

#if defined(__GLIBC__) && defined(__GLIBC_MINOR__)
#  if (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 1))
#    define HAVE_STDINT_H 1
#  endif
#elif defined(__dietlibc__)
#  undef HAVE_STDINT_H /* incomplete */
#elif (ACC_CC_BORLANDC) && (__BORLANDC__ >= 0x560)
#  undef HAVE_STDINT_H /* broken & incomplete */
#elif (ACC_CC_DMC) && (__DMC__ >= 0x825)
#  define HAVE_STDINT_H 1
#endif

#if HAVE_STDINT_H
#  include <stdint.h>
#endif

#endif /* !defined(ACC_CONFIG_AUTO_NO_HEADERS) */


/*************************************************************************
// Checks for header files
**************************************************************************/

#if !defined(ACC_CONFIG_AUTO_NO_HEADERS)

#define STDC_HEADERS 1

#define HAVE_ASSERT_H 1
#define HAVE_CTYPE_H 1
#define HAVE_DIRENT_H 1
#define HAVE_ERRNO_H 1
#define HAVE_FCNTL_H 1
#define HAVE_LIMITS_H 1
#define HAVE_MALLOC_H 1
#define HAVE_MEMORY_H 1
#define HAVE_SETJMP_H 1
#define HAVE_SIGNAL_H 1
#define HAVE_STDARG_H 1
#define HAVE_STDDEF_H 1
#define HAVE_STDIO_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_TIME_H 1
#define HAVE_UNISTD_H 1
#define HAVE_UTIME_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TYPES_H 1

#undef HAVE_ALLOCA_H
#undef HAVE_CONIO_H
#undef HAVE_DIRECT_H
#undef HAVE_DOS_H
#undef HAVE_IO_H
#undef HAVE_SHARE_H
#undef HAVE_STDINT_H
#undef HAVE_STRINGS_H
#undef HAVE_SYS_UTIME_H


#if (ACC_OS_POSIX)
#  define HAVE_STRINGS_H 1
#  if (ACC_OS_POSIX_FREEBSD || ACC_OS_POSIX_MACOSX || ACC_OS_POSIX_OPENBSD)
#    undef HAVE_MALLOC_H /* deprecated */
#  elif (ACC_OS_POSIX_HPUX || ACC_OS_POSIX_INTERIX)
#    define HAVE_ALLOCA_H 1
#  endif
#  if (ACC_OS_POSIX_MACOSX && ACC_CC_MWERKS) && defined(__MSL__)
     /* FIXME ??? */
#    undef HAVE_SYS_TIME_H
#    undef HAVE_SYS_TYPES_H
#  endif
#elif (ACC_OS_CYGWIN)
#  define HAVE_IO_H 1
#elif (ACC_OS_EMX)
#  define HAVE_ALLOCA_H 1
#  define HAVE_IO_H 1
#elif (ACC_OS_TOS && ACC_CC_GNUC)
#  if !defined(__MINT__)
#    undef HAVE_MALLOC_H
#  endif
#elif (ACC_OS_TOS && (ACC_CC_PUREC || ACC_CC_TURBOC))
#  undef HAVE_DIRENT_H
#  undef HAVE_FCNTL_H
#  undef HAVE_MALLOC_H
#  undef HAVE_MEMORY_H
#  undef HAVE_UNISTD_H
#  undef HAVE_UTIME_H
#  undef HAVE_SYS_STAT_H
#  undef HAVE_SYS_TIME_H
#  undef HAVE_SYS_TYPES_H
#endif


/* DOS, OS/2 & Windows */
#if (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)

#define HAVE_CONIO_H 1
#define HAVE_DIRECT_H 1
#define HAVE_DOS_H 1
#define HAVE_IO_H 1
#define HAVE_SHARE_H 1

#if (ACC_CC_AZTECC)
#  undef HAVE_CONIO_H
#  undef HAVE_DIRECT_H
#  undef HAVE_DIRENT_H
#  undef HAVE_MALLOC_H
#  undef HAVE_SHARE_H
#  undef HAVE_UNISTD_H
#  undef HAVE_UTIME_H
#  undef HAVE_SYS_STAT_H
#  undef HAVE_SYS_TIME_H
#  undef HAVE_SYS_TYPES_H
#elif (ACC_CC_BORLANDC)
#  undef HAVE_UNISTD_H
#  undef HAVE_SYS_TIME_H
#  if (ACC_OS_WIN32 || ACC_OS_WIN64)
#    undef HAVE_DIRENT_H /* pulls in <windows.h>; use <dir.h> instead */
#  endif
#  if (__BORLANDC__ < 0x0400)
#    undef HAVE_DIRENT_H
#    undef HAVE_UTIME_H
#  endif
#elif (ACC_CC_DMC)
#  undef HAVE_DIRENT_H /* not working */
#  undef HAVE_UNISTD_H /* not working */
#  define HAVE_SYS_DIRENT_H 1
#elif (ACC_OS_DOS32 && ACC_CC_GNUC) && defined(__DJGPP__)
#elif (ACC_OS_DOS32 && ACC_CC_HIGHC)
#  define HAVE_ALLOCA_H 1
#  undef HAVE_DIRENT_H
#  undef HAVE_UNISTD_H
#elif (ACC_CC_IBMC && ACC_OS_OS2)
#  undef HAVE_DOS_H
#  undef HAVE_DIRENT_H
#  undef HAVE_UNISTD_H
#  undef HAVE_UTIME_H
#  undef HAVE_SYS_TIME_H
#  define HAVE_SYS_UTIME_H 1
#elif (ACC_CC_INTELC || ACC_CC_MSC)
#  undef HAVE_DIRENT_H
#  undef HAVE_UNISTD_H
#  undef HAVE_UTIME_H
#  undef HAVE_SYS_TIME_H
#  define HAVE_SYS_UTIME_H 1
#elif (ACC_CC_LCC)
#  undef HAVE_DIRENT_H
#  undef HAVE_DOS_H
#  undef HAVE_SYS_TIME_H
#elif (ACC_OS_WIN32 && ACC_CC_GNUC) && defined(__MINGW32__)
#  undef HAVE_UTIME_H
#  define HAVE_SYS_UTIME_H 1
#elif (ACC_OS_WIN32 && ACC_CC_MWERKS) && defined(__MSL__)
#  define HAVE_ALLOCA_H 1
#  undef HAVE_DOS_H
#  undef HAVE_SHARE_H
#  undef HAVE_SYS_TIME_H
#elif (ACC_CC_NDPC)
#  undef HAVE_DIRENT_H
#  undef HAVE_DOS_H
#  undef HAVE_UNISTD_H
#  undef HAVE_UTIME_H
#  undef HAVE_SYS_TIME_H
#elif (ACC_CC_PACIFICC)
#  undef HAVE_DIRECT_H
#  undef HAVE_DIRENT_H
#  undef HAVE_FCNTL_H
#  undef HAVE_IO_H
#  undef HAVE_MALLOC_H
#  undef HAVE_MEMORY_H
#  undef HAVE_SHARE_H
#  undef HAVE_UNISTD_H
#  undef HAVE_UTIME_H
#  undef HAVE_SYS_STAT_H
#  undef HAVE_SYS_TIME_H
#  undef HAVE_SYS_TYPES_H
#elif (ACC_OS_WIN32 && ACC_CC_PELLESC)
#  undef HAVE_DIRENT_H
#  undef HAVE_DOS_H
#  undef HAVE_MALLOC_H
#  undef HAVE_SHARE_H
#  undef HAVE_UNISTD_H
#  undef HAVE_UTIME_H
#  undef HAVE_SYS_TIME_H
#  if (__POCC__ < 280)
#  else
#    define HAVE_SYS_UTIME_H 1
#  endif
#elif (ACC_OS_WIN32 && ACC_CC_PGI) && defined(__MINGW32__)
#  undef HAVE_UTIME_H
#  define HAVE_SYS_UTIME_H 1
#elif (ACC_OS_WIN32 && ACC_CC_GNUC) && defined(__PW32__)
#elif (ACC_CC_SYMANTECC)
#  undef HAVE_DIRENT_H /* opendir() not implemented in libc */
#  undef HAVE_UNISTD_H /* not working */
#  if (__SC__ < 0x700)
#    undef HAVE_UTIME_H
#    undef HAVE_SYS_TIME_H
#  endif
#elif (ACC_CC_TOPSPEEDC)
#  undef HAVE_DIRENT_H
#  undef HAVE_UNISTD_H
#  undef HAVE_UTIME_H
#  undef HAVE_SYS_STAT_H
#  undef HAVE_SYS_TIME_H
#  undef HAVE_SYS_TYPES_H
#elif (ACC_CC_TURBOC)
#  undef HAVE_UNISTD_H
#  undef HAVE_SYS_TIME_H
#  undef HAVE_SYS_TYPES_H /* useless */
#  if (ACC_OS_WIN32 || ACC_OS_WIN64)
#    undef HAVE_DIRENT_H /* pulls in <windows.h>; use <dir.h> instead */
#  endif
#  if (__TURBOC__ < 0x0200)
#    undef HAVE_SIGNAL_H /* not working */
#  endif
#  if (__TURBOC__ < 0x0400)
#    undef HAVE_DIRECT_H
#    undef HAVE_DIRENT_H
#    undef HAVE_MALLOC_H
#    undef HAVE_MEMORY_H
#    undef HAVE_UTIME_H
#  endif
#elif (ACC_CC_WATCOMC)
#  undef HAVE_DIRENT_H
#  undef HAVE_UTIME_H
#  undef HAVE_SYS_TIME_H
#  define HAVE_SYS_UTIME_H 1
#  if (__WATCOMC__ < 950)
#    undef HAVE_UNISTD_H
#  endif
#elif (ACC_CC_ZORTECHC)
#  undef HAVE_DIRENT_H
#  undef HAVE_MEMORY_H
#  undef HAVE_UNISTD_H
#  undef HAVE_UTIME_H
#  undef HAVE_SYS_TIME_H
#endif

#endif /* DOS, OS/2 & Windows */


#if (HAVE_SYS_TIME_H && HAVE_TIME_H)
#  define TIME_WITH_SYS_TIME 1
#endif

#endif /* !defined(ACC_CONFIG_AUTO_NO_HEADERS) */


/*************************************************************************
// Checks for library functions
**************************************************************************/

#if !defined(ACC_CONFIG_AUTO_NO_FUNCTIONS)

#define HAVE_ACCESS 1
#define HAVE_ALLOCA 1
#define HAVE_ATEXIT 1
#define HAVE_ATOI 1
#define HAVE_ATOL 1
#define HAVE_CHMOD 1
#define HAVE_CHOWN 1
#define HAVE_CTIME 1
#define HAVE_DIFFTIME 1
#define HAVE_FILENO 1
#define HAVE_FSTAT 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_GMTIME 1
#define HAVE_LOCALTIME 1
#define HAVE_LONGJMP 1
#define HAVE_LSTAT 1
#define HAVE_MEMCMP 1
#define HAVE_MEMCPY 1
#define HAVE_MEMMOVE 1
#define HAVE_MEMSET 1
#define HAVE_MKTIME 1
#define HAVE_QSORT 1
#define HAVE_RAISE 1
#define HAVE_SETJMP 1
#define HAVE_SIGNAL 1
#define HAVE_SNPRINTF 1
#define HAVE_STAT 1
#define HAVE_STRCHR 1
#define HAVE_STRDUP 1
#define HAVE_STRERROR 1
#define HAVE_STRFTIME 1
#define HAVE_STRRCHR 1
#define HAVE_STRSTR 1
#define HAVE_TIME 1
#define HAVE_UMASK 1
#define HAVE_UTIME 1
#define HAVE_VSNPRINTF 1

#if (ACC_OS_BEOS || ACC_OS_CYGWIN || ACC_OS_POSIX || ACC_OS_QNX)
#  define HAVE_STRCASECMP 1
#  define HAVE_STRNCASECMP 1
#elif (ACC_OS_WIN32 && ACC_CC_GNUC) && defined(__PW32__)
#  define HAVE_STRCASECMP 1
#  define HAVE_STRNCASECMP 1
#else
#  define HAVE_STRICMP 1
#  define HAVE_STRNICMP 1
#endif


#if (ACC_OS_POSIX)
#  if (ACC_CC_TINYC)
#    undef HAVE_ALLOCA
#  elif defined(__dietlibc__)
#  endif
#  if (ACC_OS_POSIX_MACOSX && ACC_CC_MWERKS) && defined(__MSL__)
     /* FIXME ??? */
#    undef HAVE_CHOWN
#    undef HAVE_LSTAT
#  endif
#elif (ACC_OS_CYGWIN)
#  if (ACC_CC_GNUC < 0x025a00ul)
#    undef HAVE_GETTIMEOFDAY
#    undef HAVE_LSTAT
#  endif
#  if (ACC_CC_GNUC < 0x025f00ul)
#    undef HAVE_SNPRINTF
#    undef HAVE_VSNPRINTF
#  endif
#elif (ACC_OS_EMX)
#  undef HAVE_CHOWN
#  undef HAVE_LSTAT
#elif (ACC_OS_TOS && ACC_CC_GNUC)
#  if !defined(__MINT__)
#    undef HAVE_SNPRINTF
#    undef HAVE_VSNPRINTF
#  endif
#elif (ACC_OS_TOS && (ACC_CC_PUREC || ACC_CC_TURBOC))
#  undef HAVE_ALLOCA
#  undef HAVE_ACCESS
#  undef HAVE_CHMOD
#  undef HAVE_CHOWN
#  undef HAVE_FSTAT
#  undef HAVE_GETTIMEOFDAY
#  undef HAVE_LSTAT
#  undef HAVE_SNPRINTF
#  undef HAVE_UMASK
#  undef HAVE_UTIME
#  undef HAVE_VSNPRINTF
#endif


/* DOS, OS/2 & Windows */
#if (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)

#undef HAVE_CHOWN
#undef HAVE_GETTIMEOFDAY
#undef HAVE_LSTAT
#undef HAVE_UMASK

#if (ACC_CC_AZTECC)
#  undef HAVE_ALLOCA
#  undef HAVE_DIFFTIME /* difftime() is in the math library */
#  undef HAVE_FSTAT
#  undef HAVE_STRDUP /* missing in 5.2a */
#  undef HAVE_SNPRINTF
#  undef HAVE_UTIME /* struct utimbuf is missing */
#  undef HAVE_VSNPRINTF
#elif (ACC_CC_BORLANDC)
#  if (__BORLANDC__ < 0x0400)
#    undef HAVE_ALLOCA
#    undef HAVE_UTIME
#  endif
#  if ((__BORLANDC__ < 0x0410) && ACC_OS_WIN16)
#    undef HAVE_ALLOCA
#  endif
#  if (__BORLANDC__ < 0x0550)
#    undef HAVE_SNPRINTF
#    undef HAVE_VSNPRINTF
#  endif
#elif (ACC_CC_DMC)
#  if (ACC_OS_WIN16)
#    undef HAVE_ALLOCA
#  endif
#  define snprintf _snprintf
#  define vsnprintf _vsnprintf
#elif (ACC_OS_DOS32 && ACC_CC_GNUC) && defined(__DJGPP__)
#  undef HAVE_SNPRINTF
#  undef HAVE_VSNPRINTF
#elif (ACC_OS_DOS32 && ACC_CC_HIGHC)
#  undef HAVE_SNPRINTF
#  undef HAVE_VSNPRINTF
#elif (ACC_CC_IBMC)
#  undef HAVE_SNPRINTF
#  undef HAVE_VSNPRINTF
#elif (ACC_CC_INTELC)
#  define snprintf _snprintf
#  define vsnprintf _vsnprintf
#elif (ACC_CC_LCC)
#  define utime _utime
#elif (ACC_CC_MSC)
#  if (_MSC_VER < 600)
#    undef HAVE_STRFTIME
#  endif
#  if (_MSC_VER < 700)
#    undef HAVE_SNPRINTF
#    undef HAVE_VSNPRINTF
#  else
#    define snprintf _snprintf
#    define vsnprintf _vsnprintf
#  endif
#  if ((_MSC_VER < 800) && ACC_OS_WIN16)
#    undef HAVE_ALLOCA
#  endif
#  if (ACC_ARCH_IA16) && defined(__cplusplus)
#    undef HAVE_LONGJMP
#    undef HAVE_SETJMP
#  endif
#elif (ACC_OS_WIN32 && ACC_CC_GNUC) && defined(__MINGW32__)
#  if (ACC_CC_GNUC < 0x025f00ul)
#    undef HAVE_SNPRINTF
#    undef HAVE_VSNPRINTF
#  else
#    define snprintf _snprintf
#    define vsnprintf _vsnprintf
#  endif
#elif (ACC_OS_WIN32 && ACC_CC_MWERKS) && defined(__MSL__)
#  if (__MSL__ < 0x8000ul)
#    undef HAVE_CHMOD /* <unix.h> which in turn pulls in <windows.h> */
#  endif
#elif (ACC_CC_NDPC)
#  undef HAVE_ALLOCA
#  undef HAVE_SNPRINTF
#  undef HAVE_STRNICMP
#  undef HAVE_UTIME
#  undef HAVE_VSNPRINTF
#  if defined(__cplusplus)
#    undef HAVE_STAT
#  endif
#elif (ACC_CC_PACIFICC)
#  undef HAVE_ACCESS
#  undef HAVE_ALLOCA
#  undef HAVE_CHMOD
#  undef HAVE_DIFFTIME
#  undef HAVE_FSTAT
#  undef HAVE_MKTIME
#  undef HAVE_RAISE
#  undef HAVE_SNPRINTF
#  undef HAVE_STRFTIME
#  undef HAVE_UTIME
#  undef HAVE_VSNPRINTF
#elif (ACC_OS_WIN32 && ACC_CC_PELLESC)
#  if (__POCC__ < 280)
#    define alloca _alloca
#    undef HAVE_UTIME
#  endif
#elif (ACC_OS_WIN32 && ACC_CC_PGI) && defined(__MINGW32__)
#  define snprintf _snprintf
#  define vsnprintf _vsnprintf
#elif (ACC_OS_WIN32 && ACC_CC_GNUC) && defined(__PW32__)
#  undef HAVE_SNPRINTF
#  undef HAVE_VSNPRINTF
#elif (ACC_CC_SYMANTECC)
#  if (ACC_OS_WIN16 && (ACC_MM_MEDIUM || ACC_MM_LARGE || ACC_MM_HUGE))
#    undef HAVE_ALLOCA
#  endif
#  if (__SC__ < 0x600)
#    undef HAVE_SNPRINTF
#    undef HAVE_VSNPRINTF
#  else
#    define snprintf _snprintf
#    define vsnprintf _vsnprintf
#  endif
#  if (__SC__ < 0x700)
#    undef HAVE_DIFFTIME /* difftime() is broken */
#    undef HAVE_UTIME /* struct utimbuf is missing */
#  endif
#elif (ACC_CC_TOPSPEEDC)
#  undef HAVE_SNPRINTF
#  undef HAVE_VSNPRINTF
#elif (ACC_CC_TURBOC)
#  undef HAVE_ALLOCA
#  undef HAVE_SNPRINTF
#  undef HAVE_VSNPRINTF
#  if (__TURBOC__ < 0x0200)
#    undef HAVE_RAISE
#    undef HAVE_SIGNAL
#  endif
#  if (__TURBOC__ < 0x0295)
#    undef HAVE_MKTIME
#    undef HAVE_STRFTIME
#  endif
#  if (__TURBOC__ < 0x0400)
#    undef HAVE_UTIME
#  endif
#elif (ACC_CC_WATCOMC)
#  if (__WATCOMC__ < 1100)
#    undef HAVE_SNPRINTF
#    undef HAVE_VSNPRINTF
#  elif (__WATCOMC__ < 1200)
#    define snprintf _snprintf
#    define vsnprintf _vsnprintf
#  endif
#elif (ACC_CC_ZORTECHC)
#  if (ACC_OS_WIN16 && (ACC_MM_MEDIUM || ACC_MM_LARGE || ACC_MM_HUGE))
#    undef HAVE_ALLOCA
#  endif
#  undef HAVE_DIFFTIME /* difftime() is broken */
#  undef HAVE_SNPRINTF
#  undef HAVE_UTIME /* struct utimbuf is missing */
#  undef HAVE_VSNPRINTF
#endif

#endif /* DOS, OS/2 & Windows */


#endif /* !defined(ACC_CONFIG_AUTO_NO_FUNCTIONS) */


/*************************************************************************
// Checks for sizes
**************************************************************************/

#if !defined(ACC_CONFIG_AUTO_NO_SIZES)

#define SIZEOF_SHORT            (__ACC_SHORT_BIT / 8)
#define SIZEOF_INT              (__ACC_INT_BIT / 8)
#define SIZEOF_LONG             (__ACC_LONG_BIT / 8)

#if (ACC_OS_WIN64) /* LLP64 programming model */
#  define SIZEOF_PTRDIFF_T      8
#  define SIZEOF_SIZE_T         8
#  define SIZEOF_VOID_P         8
#elif (ACC_OS_DOS16 || ACC_OS_OS216 || ACC_OS_WIN16)
#  define SIZEOF_SIZE_T         2
#  if (ACC_MM_TINY || ACC_MM_SMALL || ACC_MM_MEDIUM)
#    define SIZEOF_VOID_P       2
#  elif (ACC_MM_COMPACT || ACC_MM_LARGE || ACC_MM_HUGE)
#    define SIZEOF_VOID_P       4
#  else
#    error "ACC_MM"
#  endif
#  if (ACC_MM_TINY || ACC_MM_SMALL || ACC_MM_MEDIUM)
#    define SIZEOF_PTRDIFF_T    2
#  elif (ACC_MM_COMPACT || ACC_MM_LARGE)
#    if (ACC_CC_BORLANDC || ACC_CC_TURBOC)
#      define SIZEOF_PTRDIFF_T  4
#    else
#      define SIZEOF_PTRDIFF_T  2
#    endif
#  elif (ACC_MM_HUGE)
#    define SIZEOF_PTRDIFF_T    4
#  else
#    error "ACC_MM"
#  endif
#elif (ACC_ARCH_AVR || ACC_ARCH_C166 || ACC_ARCH_MCS51 || ACC_ARCH_MCS251)
#  define SIZEOF_PTRDIFF_T      2
#  define SIZEOF_SIZE_T         2
#  define SIZEOF_VOID_P         2
#else
#  define SIZEOF_PTRDIFF_T      SIZEOF_LONG
#  define SIZEOF_SIZE_T         SIZEOF_LONG
#  define SIZEOF_VOID_P         SIZEOF_LONG
#endif

#if !defined(SIZEOF_CHAR_P) && (SIZEOF_VOID_P > 0)
#  define SIZEOF_CHAR_P         SIZEOF_VOID_P
#endif


#if ((SIZEOF_LONG) > 0 && (SIZEOF_LONG) < 8)
#if (ACC_ARCH_IA16 && ACC_CC_DMC)
#elif (ACC_CC_GNUC)
#  define SIZEOF_LONG_LONG          8
#  define SIZEOF_UNSIGNED_LONG_LONG 8
#elif ((ACC_OS_WIN32 || ACC_OS_WIN64) && ACC_CC_MSC && (_MSC_VER >= 1400))
#  define SIZEOF_LONG_LONG          8
#  define SIZEOF_UNSIGNED_LONG_LONG 8
#elif (ACC_OS_WIN64)
#  define SIZEOF___INT64            8
#  define SIZEOF_UNSIGNED___INT64   8
#elif (ACC_ARCH_IA32 && (ACC_CC_DMC))
#  define SIZEOF_LONG_LONG          8
#  define SIZEOF_UNSIGNED_LONG_LONG 8
#elif (ACC_ARCH_IA32 && (ACC_CC_SYMANTECC && (__SC__ >= 0x700)))
#  define SIZEOF_LONG_LONG          8
#  define SIZEOF_UNSIGNED_LONG_LONG 8
#elif (ACC_ARCH_IA32 && (ACC_CC_INTELC && defined(__linux__)))
#  define SIZEOF_LONG_LONG          8
#  define SIZEOF_UNSIGNED_LONG_LONG 8
#elif (ACC_ARCH_IA32 && (ACC_CC_MWERKS || ACC_CC_PELLESC || ACC_CC_PGI))
#  define SIZEOF_LONG_LONG          8
#  define SIZEOF_UNSIGNED_LONG_LONG 8
#elif (ACC_ARCH_IA32 && (ACC_CC_INTELC || ACC_CC_MSC))
#  define SIZEOF___INT64            8
#  define SIZEOF_UNSIGNED___INT64   8
#elif (ACC_ARCH_IA32 && (ACC_CC_BORLANDC && (__BORLANDC__ >= 0x0520)))
   /* INFO: unsigned __int64 is somewhat broken in 0x0520; fixed in 0x0530 */
#  define SIZEOF___INT64            8
#  define SIZEOF_UNSIGNED___INT64   8
#elif (ACC_ARCH_IA32 && (ACC_CC_WATCOMC && (__WATCOMC__ >= 1100)))
#  define SIZEOF___INT64            8
#  define SIZEOF_UNSIGNED___INT64   8
#elif (ACC_CC_WATCOMC && defined(_INTEGRAL_MAX_BITS) && (_INTEGRAL_MAX_BITS == 64))
#  define SIZEOF___INT64            8
#  define SIZEOF_UNSIGNED___INT64   8
#elif 1 && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#  define SIZEOF_LONG_LONG          8
#  define SIZEOF_UNSIGNED_LONG_LONG 8
#endif
#endif

#if defined(__cplusplus) && defined(ACC_CC_GNUC)
#  if (ACC_CC_GNUC < 0x020800ul)
#    undef SIZEOF_LONG_LONG
#    undef SIZEOF_UNSIGNED_LONG_LONG
#  endif
#endif

#endif /* !defined(ACC_CONFIG_AUTO_NO_SIZES) */


/*************************************************************************
// misc
**************************************************************************/

#if defined(HAVE_SIGNAL) && !defined(RETSIGTYPE)
#  define RETSIGTYPE void
#endif


/*
vi:ts=4:et
*/
