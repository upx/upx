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


/*
 * Operating System - exactly one of:
 *
 *   ACC_OS_POSIX           [default]
 *   ACC_OS_BEOS
 *   ACC_OS_CYGWIN          hybrid WIN32 and POSIX
 *   ACC_OS_DOS16           16-bit DOS (segmented memory model)
 *   ACC_OS_DOS32
 *   ACC_OS_EMX             hybrid OS/2, DOS32 and POSIX
 *   ACC_OS_MAC9            Macintosh Classic
 *   ACC_OS_MACOSX          Mac OS/X
 *   ACC_OS_PALMOS
 *   ACC_OS_OS2             OS/2
 *   ACC_OS_OS216           16-bit OS/2 1.x (segmented memory model)
 *   ACC_OS_QNX
 *   ACC_OS_TOS             Atari TOS / MiNT
 *   ACC_OS_VMS
 *   ACC_OS_WIN16           16-bit Windows 3.x (segmented memory model)
 *   ACC_OS_WIN32
 *   ACC_OS_WIN64
 */


#if defined(__CYGWIN32__) && !defined(__CYGWIN__)
#  define __CYGWIN__ __CYGWIN32__
#endif

#if defined(__CYGWIN__) && defined(__GNUC__)
#  define ACC_OS_CYGWIN         1
#  define ACC_INFO_OS           "cygwin"
#elif defined(__EMX__) && defined(__GNUC__)
#  define ACC_OS_EMX            1
#  define ACC_INFO_OS           "emx"
#elif defined(__WIN64__) || defined(_WIN64) || defined(WIN64)
#  define ACC_OS_WIN64          1
#  define ACC_INFO_OS           "win64"
#elif defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
#  define ACC_OS_WIN32          1
#  define ACC_INFO_OS           "win32"
#elif defined(__NT__) || defined(__NT_DLL__) || defined(__WINDOWS_386__)
#  define ACC_OS_WIN32          1
#  define ACC_INFO_OS           "win32"
#elif defined(__MWERKS__) && defined(__INTEL__)
#  define ACC_OS_WIN32          1
#  define ACC_INFO_OS           "win32"
#elif defined(__WINDOWS__) || defined(_WINDOWS) || defined(_Windows)
#  if (UINT_MAX == 0xffffffffL)
#    define ACC_OS_WIN32        1
#    define ACC_INFO_OS         "win32"
#  elif (UINT_MAX == 0xffffL)
#    define ACC_OS_WIN16        1
#    define ACC_INFO_OS         "win16"
#  else
#    error "check your limits.h header"
#  endif
#elif defined(__DOS__) || defined(__MSDOS__) || defined(MSDOS) || (defined(__PACIFIC__) && defined(DOS))
#  if (UINT_MAX == 0xffffffffL)
#    define ACC_OS_DOS32        1
#    define ACC_INFO_OS         "dos32"
#  elif (UINT_MAX == 0xffffL)
#    define ACC_OS_DOS16        1
#    define ACC_INFO_OS         "dos16"
#  else
#    error "check your limits.h header"
#  endif
#elif defined(__OS2__) || defined(__OS2V2__)
#  if (UINT_MAX == 0xffffffffL)
#    define ACC_OS_OS2          1
#    define ACC_INFO_OS         "os2"
#  elif (UINT_MAX == 0xffffL)
#    define ACC_OS_OS216       1
#    define ACC_INFO_OS         "os216"
#  else
#    error "check your limits.h header"
#  endif
#elif defined(__palmos__)
#  if (UINT_MAX == 0xffffL)
#    define ACC_OS_PALMOS       1
#    define ACC_INFO_OS         "palmos"
#  else
#    error "check your limits.h header"
#  endif
#elif defined(__TOS__) || defined(__atarist__)
#  define ACC_OS_TOS            1
#  define ACC_INFO_OS           "tos"
#elif defined(__QNX__)
#  define ACC_OS_QNX            1
#  define ACC_INFO_OS           "qnx"
#elif defined(__MACOSX__)
#  define ACC_OS_MACOSX         1
#  define ACC_INFO_OS           "macosx"
#elif defined(macintosh)
#  define ACC_OS_MACOS9         1
#  define ACC_INFO_OS           "macos9"
#elif defined(__VMS)
#  define ACC_OS_VMS            1
#  define ACC_INFO_OS           "vms"
#else
#  define ACC_OS_POSIX          1
#  define ACC_INFO_OS           "posix"
#endif


#if (ACC_OS_POSIX)
#  if defined(_AIX) || defined(__AIX__) || defined(__aix__)
#    define ACC_OS_POSIX_AIX        1
#    define ACC_INFO_OS_POSIX       "aix"
#  elif defined(__FreeBSD__)
#    define ACC_OS_POSIX_FREEBSD    1
#    define ACC_INFO_OS_POSIX       "freebsd"
#  elif defined(__hpux__) || defined(__hpux)
#    define ACC_OS_POSIX_HPUX       1
#    define ACC_INFO_OS_POSIX       "hpux"
#  elif defined(__IRIX__) || defined(__irix__)
#    define ACC_OS_POSIX_IRIX       1
#    define ACC_INFO_OS_POSIX       "irix"
#  elif defined(__linux__) || defined(__linux)
#    define ACC_OS_POSIX_LINUX      1
#    define ACC_INFO_OS_POSIX       "linux"
#  elif defined(__NetBSD__)
#    define ACC_OS_POSIX_NETBSD     1
#    define ACC_INFO_OS_POSIX       "netbsd"
#  elif defined(__OpenBSD__)
#    define ACC_OS_POSIX_OPENBSD    1
#    define ACC_INFO_OS_POSIX       "openbsd"
#  elif defined(__solaris__) || defined(__sun)
#    if defined(__SVR4) || defined(__svr4__)
#      define ACC_OS_POSIX_SOLARIS  1
#      define ACC_INFO_OS_POSIX     "solaris"
#    else
#      define ACC_OS_POSIX_SUNOS    1
#      define ACC_INFO_OS_POSIX     "sunos"
#    endif
#  elif defined(__ultrix__) || defined(__ultrix)
#    define ACC_OS_POSIX_ULTRIX     1
#    define ACC_INFO_OS_POSIX       "ultrix"
#  else
#    define ACC_OS_POSIX_UNKNOWN    1
#    define ACC_INFO_OS_POSIX       "unknown"
#  endif
#endif


#if (ACC_OS_DOS16 || ACC_OS_OS216 || ACC_OS_WIN16)
#  if (UINT_MAX != 0xffffL)
#    error
#  endif
#endif
#if (ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_WIN32 || ACC_OS_WIN64)
#  if (UINT_MAX != 0xffffffffL)
#    error
#  endif
#endif


/*
vi:ts=4:et
*/
