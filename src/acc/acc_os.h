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
 * Operating System - exactly one of:
 *
 *   ACC_OS_POSIX           [default]
 *   ACC_OS_AMIGAOS
 *   ACC_OS_BEOS
 *   ACC_OS_CYGWIN          hybrid WIN32 and POSIX
 *   ACC_OS_DOS16           16-bit DOS (segmented memory model)
 *   ACC_OS_DOS32
 *   ACC_OS_EMX             hybrid OS/2, DOS32, WIN32 (with RSX) and POSIX
 *   ACC_OS_MACCLASSIC      Macintosh Classic
 *   ACC_OS_PALMOS
 *   ACC_OS_OS2             OS/2
 *   ACC_OS_OS216           16-bit OS/2 1.x (segmented memory model)
 *   ACC_OS_QNX
 *   ACC_OS_RISCOS
 *   ACC_OS_TOS             Atari TOS / MiNT
 *   ACC_OS_VMS
 *   ACC_OS_WIN16           16-bit Windows 3.x (segmented memory model)
 *   ACC_OS_WIN32
 *   ACC_OS_WIN64           64-bit Windows (LLP64 programming model)
 */


#if defined(__CYGWIN__) && defined(__GNUC__)
#  define ACC_OS_CYGWIN         1
#  define ACC_INFO_OS           "cygwin"
#elif defined(__EMX__) && defined(__GNUC__)
#  define ACC_OS_EMX            1
#  define ACC_INFO_OS           "emx"
#elif defined(__BEOS__)
#  define ACC_OS_BEOS           1
#  define ACC_INFO_OS           "beos"
#elif defined(__QNX__)
#  define ACC_OS_QNX            1
#  define ACC_INFO_OS           "qnx"
#elif defined(__BORLANDC__) && defined(__DPMI32__) && (__BORLANDC__ >= 0x0460)
#  define ACC_OS_DOS32          1
#  define ACC_INFO_OS           "dos32"
#elif defined(__BORLANDC__) && defined(__DPMI16__)
#  define ACC_OS_DOS16          1
#  define ACC_INFO_OS           "dos16"
#elif defined(__ZTC__) && defined(DOS386)
#  define ACC_OS_DOS32          1
#  define ACC_INFO_OS           "dos32"
#elif defined(__OS2__) || defined(__OS2V2__)
#  if (UINT_MAX == ACC_0xffffL)
#    define ACC_OS_OS216        1
#    define ACC_INFO_OS         "os216"
#  elif (UINT_MAX == ACC_0xffffffffL)
#    define ACC_OS_OS2          1
#    define ACC_INFO_OS         "os2"
#  else
#    error "check your limits.h header"
#  endif
#elif defined(__WIN64__) || defined(_WIN64) || defined(WIN64)
#  define ACC_OS_WIN64          1
#  define ACC_INFO_OS           "win64"
#elif defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__WINDOWS_386__)
#  define ACC_OS_WIN32          1
#  define ACC_INFO_OS           "win32"
#elif defined(__MWERKS__) && defined(__INTEL__)
#  define ACC_OS_WIN32          1
#  define ACC_INFO_OS           "win32"
#elif defined(__WINDOWS__) || defined(_WINDOWS) || defined(_Windows)
#  if (UINT_MAX == ACC_0xffffL)
#    define ACC_OS_WIN16        1
#    define ACC_INFO_OS         "win16"
#  elif (UINT_MAX == ACC_0xffffffffL)
#    define ACC_OS_WIN32        1
#    define ACC_INFO_OS         "win32"
#  else
#    error "check your limits.h header"
#  endif
#elif defined(__DOS__) || defined(__MSDOS__) || defined(_MSDOS) || defined(MSDOS) || (defined(__PACIFIC__) && defined(DOS))
#  if (UINT_MAX == ACC_0xffffL)
#    define ACC_OS_DOS16        1
#    define ACC_INFO_OS         "dos16"
#  elif (UINT_MAX == ACC_0xffffffffL)
#    define ACC_OS_DOS32        1
#    define ACC_INFO_OS         "dos32"
#  else
#    error "check your limits.h header"
#  endif
#elif defined(__WATCOMC__)
#  if defined(__NT__) && (UINT_MAX == ACC_0xffffL)
     /* wcl: NT host defaults to DOS target */
#    define ACC_OS_DOS16        1
#    define ACC_INFO_OS         "dos16"
#  elif defined(__NT__) && (__WATCOMC__ < 1100)
     /* wcl386: Watcom C 11 defines _WIN32 */
#    define ACC_OS_WIN32        1
#    define ACC_INFO_OS         "win32"
#  else
#    error "please specify a target using the -bt compiler option"
#  endif
#elif defined(__palmos__)
#  if (UINT_MAX == ACC_0xffffL)
#    define ACC_OS_PALMOS       1
#    define ACC_INFO_OS         "palmos"
#  else
#    error "check your limits.h header"
#  endif
#elif defined(__TOS__) || defined(__atarist__)
#  define ACC_OS_TOS            1
#  define ACC_INFO_OS           "tos"
#elif defined(macintosh) && !defined(__ppc__)
#  define ACC_OS_MACCLASSIC     1
#  define ACC_INFO_OS           "macclassic"
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
#  elif defined(__INTERIX)
#    define ACC_OS_POSIX_INTERIX    1
#    define ACC_INFO_OS_POSIX       "interix"
#  elif defined(__IRIX__) || defined(__irix__)
#    define ACC_OS_POSIX_IRIX       1
#    define ACC_INFO_OS_POSIX       "irix"
#  elif defined(__linux__) || defined(__linux)
#    define ACC_OS_POSIX_LINUX      1
#    define ACC_INFO_OS_POSIX       "linux"
#  elif defined(__APPLE__) || defined(__MACOS__)
#    define ACC_OS_POSIX_MACOSX     1
#    define ACC_INFO_OS_POSIX       "macosx"
#  elif defined(__NetBSD__)
#    define ACC_OS_POSIX_NETBSD     1
#    define ACC_INFO_OS_POSIX       "netbsd"
#  elif defined(__OpenBSD__)
#    define ACC_OS_POSIX_OPENBSD    1
#    define ACC_INFO_OS_POSIX       "openbsd"
#  elif defined(__osf__)
#    define ACC_OS_POSIX_OSF        1
#    define ACC_INFO_OS_POSIX       "osf"
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
#  if (UINT_MAX != ACC_0xffffL)
#    error "this should not happen"
#  endif
#  if (ULONG_MAX != ACC_0xffffffffL)
#    error "this should not happen"
#  endif
#endif
#if (ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_WIN32 || ACC_OS_WIN64)
#  if (UINT_MAX != ACC_0xffffffffL)
#    error "this should not happen"
#  endif
#  if (ULONG_MAX != ACC_0xffffffffL)
#    error "this should not happen"
#  endif
#endif


/*
vi:ts=4:et
*/
