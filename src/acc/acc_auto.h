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


/*************************************************************************
// Checks for <stdint.h>
**************************************************************************/

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


/*************************************************************************
// Checks for header files
**************************************************************************/

#define STDC_HEADERS 1

#define HAVE_ASSERT_H 1
#define HAVE_CTYPE_H 1
#define HAVE_DIRENT_H 1
#define HAVE_ERRNO_H 1
#define HAVE_FCNTL_H 1
#define HAVE_LIMITS_H 1
#define HAVE_MALLOC_H 1
#define HAVE_MEMORY_H 1
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
#  if (ACC_OS_POSIX_FREEBSD || ACC_OS_POSIX_OPENBSD)
#    undef HAVE_MALLOC_H /* deprecated */
#  elif (ACC_OS_POSIX_HPUX)
#    define HAVE_ALLOCA_H 1
#  endif
#elif (ACC_OS_CYGWIN)
#  define HAVE_ALLOCA_H 1
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
#elif (ACC_CC_DMC)
#  undef HAVE_DIRENT_H /* not working */
#  undef HAVE_UNISTD_H /* not working */
#  define HAVE_SYS_DIRENT_H 1
#elif defined(__DJGPP__)
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
#elif defined(__MINGW32__)
#  undef HAVE_UTIME_H
#  define HAVE_SYS_UTIME_H 1
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
#elif (ACC_CC_SYMANTECC)
#  undef HAVE_DIRENT_H /* opendir() not implemented in libc */
#  undef HAVE_UNISTD_H /* not working */
#  if (__SC__ < 0x700)
#    undef HAVE_UTIME_H
#    undef HAVE_SYS_TIME_H
#  endif
#elif (ACC_CC_TURBOC)
#  undef HAVE_UNISTD_H
#  undef HAVE_SYS_TIME_H
#  if (__TURBOC__ < 0x0200)
#    undef HAVE_SYS_TYPES_H
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
#  if (__WATCOMC__ < 900)
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


/*************************************************************************
// Checks for library functions
**************************************************************************/

#define HAVE_ACCESS 1
#define HAVE_ALLOCA 1
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
#define HAVE_LSTAT 1
#define HAVE_MEMCMP 1
#define HAVE_MEMCPY 1
#define HAVE_MEMMOVE 1
#define HAVE_MEMSET 1
#define HAVE_SNPRINTF 1
#define HAVE_STRCHR 1
#define HAVE_STRDUP 1
#define HAVE_STRERROR 1
#define HAVE_STRFTIME 1
#define HAVE_STRRCHR 1
#define HAVE_UMASK 1
#define HAVE_UTIME 1
#define HAVE_VSNPRINTF 1

#if (ACC_OS_POSIX || ACC_OS_CYGWIN)
#  define HAVE_STRCASECMP 1
#  define HAVE_STRNCASECMP 1
#else
#  define HAVE_STRICMP 1
#  define HAVE_STRNICMP 1
#endif


#if (ACC_OS_POSIX)
#elif (ACC_OS_CYGWIN)
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
#  undef HAVE_DIFFTIME /* difftime() is in the math library */
#  undef HAVE_FSTAT
#  undef HAVE_STRDUP /* missing in 5.2a */
#  undef HAVE_SNPRINTF
#  undef HAVE_VSNPRINTF
#elif (ACC_CC_BORLANDC)
#  if (ACC_OS_DOS16 || ACC_OS_WIN16)
#    undef HAVE_DIFFTIME /* difftime() is in the math library */
#  endif
#  if (__BORLANDC__ < 0x0550)
#    undef HAVE_SNPRINTF
#    undef HAVE_VSNPRINTF
#  endif
#elif (ACC_CC_DMC)
#  define snprintf _snprintf
#  define vsnprintf _vsnprintf
#elif defined(__DJGPP__)
#  undef HAVE_SNPRINTF
#  undef HAVE_VSNPRINTF
#elif (ACC_CC_INTELC)
#  define snprintf _snprintf
#  define vsnprintf _vsnprintf
#elif (ACC_CC_LCC)
#  define utime _utime
#elif (ACC_CC_MSC)
#  if (_MSC_VER >= 700)
#    define snprintf _snprintf
#    define vsnprintf _vsnprintf
#  else
#    undef HAVE_SNPRINTF
#    undef HAVE_VSNPRINTF
#  endif
#elif defined(__MINGW32__)
#elif (ACC_CC_PACIFICC)
#  undef HAVE_ACCESS
#  undef HAVE_ALLOCA
#  undef HAVE_CHMOD
#  undef HAVE_DIFFTIME
#  undef HAVE_FSTAT
#  undef HAVE_SNPRINTF
#  undef HAVE_STRFTIME
#  undef HAVE_UTIME
#  undef HAVE_VSNPRINTF
#elif (ACC_CC_SYMANTECC)
#  if (ACC_OS_WIN16 && (ACC_MM_MEDIUM || ACC_MM_LARGE || ACC_MM_HUGE))
#    undef HAVE_ALLOCA
#  endif
#  if (__SC__ < 0x700)
#    undef HAVE_DIFFTIME /* difftime() is broken */
#  endif
#  if (__SC__ >= 0x610)
#    define snprintf _snprintf
#    define vsnprintf _vsnprintf
#  else
#    undef HAVE_SNPRINTF
#    undef HAVE_VSNPRINTF
#  endif
#elif (ACC_CC_TURBOC)
#  undef HAVE_ALLOCA
#  undef HAVE_SNPRINTF
#  undef HAVE_VSNPRINTF
#  if (__TURBOC__ < 0x0295)
#    undef HAVE_STRFTIME
#  endif
#  if (__TURBOC__ < 0x0400)
#    undef HAVE_UTIME
#  endif
#elif (ACC_CC_WATCOMC)
#  if (__WATCOMC__ >= 1100)
#    define snprintf _snprintf
#    define vsnprintf _vsnprintf
#  else
#    undef HAVE_SNPRINTF
#    undef HAVE_VSNPRINTF
#  endif
#elif (ACC_CC_ZORTECHC)
#  if (ACC_OS_WIN16 && (ACC_MM_MEDIUM || ACC_MM_LARGE || ACC_MM_HUGE))
#    undef HAVE_ALLOCA
#  endif
#  undef HAVE_DIFFTIME /* difftime() is broken */
#  undef HAVE_SNPRINTF
#  undef HAVE_VSNPRINTF
#endif

#endif /* DOS, OS/2 & Windows */


/*************************************************************************
// Checks for typedefs and structures
**************************************************************************/

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
#  if (ACC_MM_HUGE)
#    define SIZEOF_PTRDIFF_T    4
#  elif (ACC_MM_COMPACT || ACC_MM_LARGE)
#    if (ACC_CC_BORLANDC || ACC_CC_TURBOC)
#      define SIZEOF_PTRDIFF_T  4
#    else
#      define SIZEOF_PTRDIFF_T  2
#    endif
#  else
#    define SIZEOF_PTRDIFF_T    2
#  endif
#else
#  define SIZEOF_PTRDIFF_T      SIZEOF_LONG
#  define SIZEOF_SIZE_T         SIZEOF_LONG
#  define SIZEOF_VOID_P         SIZEOF_LONG
#endif

#if !defined(SIZEOF_CHAR_P) && (SIZEOF_VOID_P > 0)
#  define SIZEOF_CHAR_P         SIZEOF_VOID_P
#endif


/* FIXME: add more sizes */

#if (ACC_ARCH_IA32 && (ACC_CC_DMC || ACC_CC_GNUC))
#  define SIZEOF_LONG_LONG          8
#  define SIZEOF_UNSIGNED_LONG_LONG 8
#elif (ACC_ARCH_IA32 && (ACC_CC_INTELC || ACC_CC_MSC))
#  define SIZEOF___INT64            8
#  define SIZEOF_UNSIGNED___INT64   8
#elif (ACC_ARCH_IA32 && (ACC_CC_BORLANDC && __BORLANDC__ >= 0x0550))
   /* info: unsigned __int64 is broken in 0x0520 */
#  define SIZEOF___INT64            8
#  define SIZEOF_UNSIGNED___INT64   8
#elif (ACC_ARCH_IA32 && (ACC_CC_WATCOMC && __WATCOMC__ >= 1100))
#  define SIZEOF___INT64            8
#  define SIZEOF_UNSIGNED___INT64   8
#elif (ACC_ARCH_M68K && (ACC_CC_GNUC))
#  define SIZEOF_LONG_LONG          8
#  define SIZEOF_UNSIGNED_LONG_LONG 8
#endif


/*
vi:ts=4:et
*/
