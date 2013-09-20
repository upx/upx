/* ACC --- Automatic Compiler Configuration

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2013 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   UPX and the UCL library are free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/
 */

#ifndef __ACC_H_INCLUDED
#define __ACC_H_INCLUDED 1
#define ACC_VERSION     20130916L
#if defined(__CYGWIN32__) && !defined(__CYGWIN__)
#  define __CYGWIN__ __CYGWIN32__
#endif
#if defined(__IBMCPP__) && !defined(__IBMC__)
#  define __IBMC__ __IBMCPP__
#endif
#if defined(__ICL) && defined(_WIN32) && !defined(__INTEL_COMPILER)
#  define __INTEL_COMPILER __ICL
#endif
#if 1 && defined(__INTERIX) && defined(__GNUC__) && !defined(_ALL_SOURCE)
#  define _ALL_SOURCE 1
#endif
#if defined(__mips__) && defined(__R5900__)
#  if !defined(__LONG_MAX__)
#    define __LONG_MAX__ 9223372036854775807L
#  endif
#endif
#if !defined(ACC_CFG_NO_DISABLE_WUNDEF)
#if defined(__ARMCC_VERSION)
#  pragma diag_suppress 193
#elif defined(__clang__) && defined(__clang_minor__)
#  pragma clang diagnostic ignored "-Wundef"
#elif defined(__INTEL_COMPILER)
#  pragma warning(disable: 193)
#elif defined(__KEIL__) && defined(__C166__)
#  pragma warning disable = 322
#elif defined(__GNUC__) && defined(__GNUC_MINOR__)
#  if ((__GNUC__-0) >= 5 || ((__GNUC__-0) == 4 && (__GNUC_MINOR__-0) >= 2))
#    pragma GCC diagnostic ignored "-Wundef"
#  endif
#elif defined(_MSC_VER) && !defined(__clang__) && !defined(__INTEL_COMPILER) && !defined(__MWERKS__)
#  if ((_MSC_VER-0) >= 1300)
#    pragma warning(disable: 4668)
#  endif
#endif
#endif
#if 0 && defined(__POCC__) && defined(_WIN32)
#  if (__POCC__ >= 400)
#    pragma warn(disable: 2216)
#  endif
#endif
#if 0 && defined(__WATCOMC__)
#  if (__WATCOMC__ >= 1050) && (__WATCOMC__ < 1060)
#    pragma warning 203 9
#  endif
#endif
#if defined(__BORLANDC__) && defined(__MSDOS__) && !defined(__FLAT__)
#  pragma option -h
#endif
#if !(ACC_CFG_NO_DISABLE_WCRTNONSTDC)
#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE 1
#endif
#ifndef _CRT_NONSTDC_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS 1
#endif
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif
#endif
#if (ACC_CFG_NO_CONFIG_HEADER)
#elif defined(ACC_CFG_CONFIG_HEADER)
#  include ACC_CFG_CONFIG_HEADER
#else
#endif
#if (ACC_CFG_NO_LIMITS_H)
#elif (ACC_LIBC_NAKED) || (ACC_BROKEN_LIMITS_H)
#ifndef __ACC_FALLBACK_LIMITS_H_INCLUDED
#define __ACC_FALLBACK_LIMITS_H_INCLUDED 1
#undef CHAR_BIT
#define CHAR_BIT        8
#ifndef MB_LEN_MAX
#define MB_LEN_MAX      1
#endif
#ifndef __SCHAR_MAX__
#define __SCHAR_MAX__   127
#endif
#ifndef __SHRT_MAX__
#define __SHRT_MAX__    32767
#endif
#ifndef __INT_MAX__
#define __INT_MAX__     2147483647
#endif
#ifndef __LONG_MAX__
#if defined(_ILP32) || defined(__ILP32__)
#define __LONG_MAX__    2147483647L
#elif defined(__alpha__) || defined(_LP64) || defined(__MIPS_PSX2__)
#define __LONG_MAX__    9223372036854775807L
#else
#define __LONG_MAX__    2147483647L
#endif
#endif
#undef SCHAR_MIN
#undef SCHAR_MAX
#undef UCHAR_MAX
#define SCHAR_MIN       (-1 - SCHAR_MAX)
#define SCHAR_MAX       (__SCHAR_MAX__)
#define UCHAR_MAX       (SCHAR_MAX * 2 + 1)
#undef SHRT_MIN
#undef SHRT_MAX
#undef USHRT_MAX
#define SHRT_MIN        (-1 - SHRT_MAX)
#define SHRT_MAX        (__SHRT_MAX__)
#if ((__INT_MAX__) == (__SHRT_MAX__))
#define USHRT_MAX       (SHRT_MAX * 2U + 1U)
#else
#define USHRT_MAX       (SHRT_MAX * 2 + 1)
#endif
#undef INT_MIN
#undef INT_MAX
#undef UINT_MAX
#define INT_MIN         (-1 - INT_MAX)
#define INT_MAX         (__INT_MAX__)
#define UINT_MAX        (INT_MAX * 2U + 1U)
#undef LONG_MIN
#undef LONG_MAX
#undef ULONG_MAX
#define LONG_MIN        (-1L - LONG_MAX)
#define LONG_MAX        ((__LONG_MAX__) + 0L)
#define ULONG_MAX       (LONG_MAX * 2UL + 1UL)
#undef CHAR_MIN
#undef CHAR_MAX
#if defined(__CHAR_UNSIGNED__) || defined(_CHAR_UNSIGNED)
#define CHAR_MIN        0
#define CHAR_MAX        UCHAR_MAX
#else
#define CHAR_MIN        SCHAR_MIN
#define CHAR_MAX        SCHAR_MAX
#endif
#endif
#else
#  include <limits.h>
#endif
#if 0
#define ACC_0xffffL             0xfffful
#define ACC_0xffffffffL         0xfffffffful
#else
#define ACC_0xffffL             65535ul
#define ACC_0xffffffffL         4294967295ul
#endif
#if (ACC_0xffffL == ACC_0xffffffffL)
#  error "your preprocessor is broken 1"
#endif
#if (16ul * 16384ul != 262144ul)
#  error "your preprocessor is broken 2"
#endif
#if 0
#if (32767 >= 4294967295ul)
#  error "your preprocessor is broken 3"
#endif
#if (65535u >= 4294967295ul)
#  error "your preprocessor is broken 4"
#endif
#endif
#if defined(__COUNTER__)
#  ifndef ACC_CFG_USE_COUNTER
#  define ACC_CFG_USE_COUNTER 1
#  endif
#else
#  undef ACC_CFG_USE_COUNTER
#endif
#if (UINT_MAX == ACC_0xffffL)
#if defined(__ZTC__) && defined(__I86__) && !defined(__OS2__)
#  if !defined(MSDOS)
#    define MSDOS 1
#  endif
#  if !defined(_MSDOS)
#    define _MSDOS 1
#  endif
#elif 0 && defined(__VERSION) && defined(MB_LEN_MAX)
#  if (__VERSION == 520) && (MB_LEN_MAX == 1)
#    if !defined(__AZTEC_C__)
#      define __AZTEC_C__ __VERSION
#    endif
#    if !defined(__DOS__)
#      define __DOS__ 1
#    endif
#  endif
#endif
#endif
#if defined(_MSC_VER) && defined(M_I86HM) && (UINT_MAX == ACC_0xffffL)
#  define ptrdiff_t long
#  define _PTRDIFF_T_DEFINED 1
#endif
#if (UINT_MAX == ACC_0xffffL)
#  undef __ACC_RENAME_A
#  undef __ACC_RENAME_B
#  if defined(__AZTEC_C__) && defined(__DOS__)
#    define __ACC_RENAME_A 1
#  elif defined(_MSC_VER) && defined(MSDOS)
#    if (_MSC_VER < 600)
#      define __ACC_RENAME_A 1
#    elif (_MSC_VER < 700)
#      define __ACC_RENAME_B 1
#    endif
#  elif defined(__TSC__) && defined(__OS2__)
#    define __ACC_RENAME_A 1
#  elif defined(__MSDOS__) && defined(__TURBOC__) && (__TURBOC__ < 0x0410)
#    define __ACC_RENAME_A 1
#  elif defined(__PACIFIC__) && defined(DOS)
#    if !defined(__far)
#      define __far far
#    endif
#    if !defined(__near)
#      define __near near
#    endif
#  endif
#  if defined(__ACC_RENAME_A)
#    if !defined(__cdecl)
#      define __cdecl cdecl
#    endif
#    if !defined(__far)
#      define __far far
#    endif
#    if !defined(__huge)
#      define __huge huge
#    endif
#    if !defined(__near)
#      define __near near
#    endif
#    if !defined(__pascal)
#      define __pascal pascal
#    endif
#    if !defined(__huge)
#      define __huge huge
#    endif
#  elif defined(__ACC_RENAME_B)
#    if !defined(__cdecl)
#      define __cdecl _cdecl
#    endif
#    if !defined(__far)
#      define __far _far
#    endif
#    if !defined(__huge)
#      define __huge _huge
#    endif
#    if !defined(__near)
#      define __near _near
#    endif
#    if !defined(__pascal)
#      define __pascal _pascal
#    endif
#  elif (defined(__PUREC__) || defined(__TURBOC__)) && defined(__TOS__)
#    if !defined(__cdecl)
#      define __cdecl cdecl
#    endif
#    if !defined(__pascal)
#      define __pascal pascal
#    endif
#  endif
#  undef __ACC_RENAME_A
#  undef __ACC_RENAME_B
#endif
#if (UINT_MAX == ACC_0xffffL)
#if defined(__AZTEC_C__) && defined(__DOS__)
#  define ACC_BROKEN_CDECL_ALT_SYNTAX 1
#elif defined(_MSC_VER) && defined(MSDOS)
#  if (_MSC_VER < 600)
#    define ACC_BROKEN_INTEGRAL_CONSTANTS 1
#  endif
#  if (_MSC_VER < 700)
#    define ACC_BROKEN_INTEGRAL_PROMOTION 1
#    define ACC_BROKEN_SIZEOF 1
#  endif
#elif defined(__PACIFIC__) && defined(DOS)
#  define ACC_BROKEN_INTEGRAL_CONSTANTS 1
#elif defined(__TURBOC__) && defined(__MSDOS__)
#  if (__TURBOC__ < 0x0150)
#    define ACC_BROKEN_CDECL_ALT_SYNTAX 1
#    define ACC_BROKEN_INTEGRAL_CONSTANTS 1
#    define ACC_BROKEN_INTEGRAL_PROMOTION 1
#  endif
#  if (__TURBOC__ < 0x0200)
#    define ACC_BROKEN_SIZEOF 1
#  endif
#  if (__TURBOC__ < 0x0400) && defined(__cplusplus)
#    define ACC_BROKEN_CDECL_ALT_SYNTAX 1
#  endif
#elif (defined(__PUREC__) || defined(__TURBOC__)) && defined(__TOS__)
#  define ACC_BROKEN_CDECL_ALT_SYNTAX 1
#  define ACC_BROKEN_SIZEOF 1
#endif
#endif
#if defined(__WATCOMC__) && (__WATCOMC__ < 900)
#  define ACC_BROKEN_INTEGRAL_CONSTANTS 1
#endif
#if defined(_CRAY) && defined(_CRAY1)
#  define ACC_BROKEN_SIGNED_RIGHT_SHIFT 1
#endif
#define ACC_PP_STRINGIZE(x)             #x
#define ACC_PP_MACRO_EXPAND(x)          ACC_PP_STRINGIZE(x)
#define ACC_PP_CONCAT2(a,b)             a ## b
#define ACC_PP_CONCAT3(a,b,c)           a ## b ## c
#define ACC_PP_CONCAT4(a,b,c,d)         a ## b ## c ## d
#define ACC_PP_CONCAT5(a,b,c,d,e)       a ## b ## c ## d ## e
#define ACC_PP_CONCAT6(a,b,c,d,e,f)     a ## b ## c ## d ## e ## f
#define ACC_PP_CONCAT7(a,b,c,d,e,f,g)   a ## b ## c ## d ## e ## f ## g
#define ACC_PP_ECONCAT2(a,b)            ACC_PP_CONCAT2(a,b)
#define ACC_PP_ECONCAT3(a,b,c)          ACC_PP_CONCAT3(a,b,c)
#define ACC_PP_ECONCAT4(a,b,c,d)        ACC_PP_CONCAT4(a,b,c,d)
#define ACC_PP_ECONCAT5(a,b,c,d,e)      ACC_PP_CONCAT5(a,b,c,d,e)
#define ACC_PP_ECONCAT6(a,b,c,d,e,f)    ACC_PP_CONCAT6(a,b,c,d,e,f)
#define ACC_PP_ECONCAT7(a,b,c,d,e,f,g)  ACC_PP_CONCAT7(a,b,c,d,e,f,g)
#if 1
#define ACC_CPP_STRINGIZE(x)            #x
#define ACC_CPP_MACRO_EXPAND(x)         ACC_CPP_STRINGIZE(x)
#define ACC_CPP_CONCAT2(a,b)            a ## b
#define ACC_CPP_CONCAT3(a,b,c)          a ## b ## c
#define ACC_CPP_CONCAT4(a,b,c,d)        a ## b ## c ## d
#define ACC_CPP_CONCAT5(a,b,c,d,e)      a ## b ## c ## d ## e
#define ACC_CPP_CONCAT6(a,b,c,d,e,f)    a ## b ## c ## d ## e ## f
#define ACC_CPP_CONCAT7(a,b,c,d,e,f,g)  a ## b ## c ## d ## e ## f ## g
#define ACC_CPP_ECONCAT2(a,b)           ACC_CPP_CONCAT2(a,b)
#define ACC_CPP_ECONCAT3(a,b,c)         ACC_CPP_CONCAT3(a,b,c)
#define ACC_CPP_ECONCAT4(a,b,c,d)       ACC_CPP_CONCAT4(a,b,c,d)
#define ACC_CPP_ECONCAT5(a,b,c,d,e)     ACC_CPP_CONCAT5(a,b,c,d,e)
#define ACC_CPP_ECONCAT6(a,b,c,d,e,f)   ACC_CPP_CONCAT6(a,b,c,d,e,f)
#define ACC_CPP_ECONCAT7(a,b,c,d,e,f,g) ACC_CPP_CONCAT7(a,b,c,d,e,f,g)
#endif
#define __ACC_MASK_GEN(o,b)     (((((o) << ((b)-!!(b))) - (o)) << 1) + (o)*!!(b))
#if 1 && defined(__cplusplus)
#  if !defined(__STDC_CONSTANT_MACROS)
#    define __STDC_CONSTANT_MACROS 1
#  endif
#  if !defined(__STDC_LIMIT_MACROS)
#    define __STDC_LIMIT_MACROS 1
#  endif
#endif
#if defined(__cplusplus)
#  define ACC_EXTERN_C          extern "C"
#  define ACC_EXTERN_C_BEGIN    extern "C" {
#  define ACC_EXTERN_C_END      }
#else
#  define ACC_EXTERN_C          extern
#  define ACC_EXTERN_C_BEGIN    /*empty*/
#  define ACC_EXTERN_C_END      /*empty*/
#endif
#if !defined(__ACC_OS_OVERRIDE)
#if (ACC_OS_FREESTANDING)
#  define ACC_INFO_OS           "freestanding"
#elif (ACC_OS_EMBEDDED)
#  define ACC_INFO_OS           "embedded"
#elif 1 && defined(__IAR_SYSTEMS_ICC__)
#  define ACC_OS_EMBEDDED       1
#  define ACC_INFO_OS           "embedded"
#elif defined(__CYGWIN__) && defined(__GNUC__)
#  define ACC_OS_CYGWIN         1
#  define ACC_INFO_OS           "cygwin"
#elif defined(__EMX__) && defined(__GNUC__)
#  define ACC_OS_EMX            1
#  define ACC_INFO_OS           "emx"
#elif defined(__BEOS__)
#  define ACC_OS_BEOS           1
#  define ACC_INFO_OS           "beos"
#elif defined(__Lynx__)
#  define ACC_OS_LYNXOS         1
#  define ACC_INFO_OS           "lynxos"
#elif defined(__OS400__)
#  define ACC_OS_OS400          1
#  define ACC_INFO_OS           "os400"
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
#    define ACC_OS_DOS16        1
#    define ACC_INFO_OS         "dos16"
#  elif defined(__NT__) && (__WATCOMC__ < 1100)
#    define ACC_OS_WIN32        1
#    define ACC_INFO_OS         "win32"
#  elif defined(__linux__) || defined(__LINUX__)
#    define ACC_OS_POSIX        1
#    define ACC_INFO_OS         "posix"
#  else
#    error "please specify a target using the -bt compiler option"
#  endif
#elif defined(__palmos__)
#  define ACC_OS_PALMOS         1
#  define ACC_INFO_OS           "palmos"
#elif defined(__TOS__) || defined(__atarist__)
#  define ACC_OS_TOS            1
#  define ACC_INFO_OS           "tos"
#elif defined(macintosh) && !defined(__ppc__)
#  define ACC_OS_MACCLASSIC     1
#  define ACC_INFO_OS           "macclassic"
#elif defined(__VMS)
#  define ACC_OS_VMS            1
#  define ACC_INFO_OS           "vms"
#elif (defined(__mips__) && defined(__R5900__)) || defined(__MIPS_PSX2__)
#  define ACC_OS_CONSOLE        1
#  define ACC_OS_CONSOLE_PS2    1
#  define ACC_INFO_OS           "console"
#  define ACC_INFO_OS_CONSOLE   "ps2"
#elif defined(__mips__) && defined(__psp__)
#  define ACC_OS_CONSOLE        1
#  define ACC_OS_CONSOLE_PSP    1
#  define ACC_INFO_OS           "console"
#  define ACC_INFO_OS_CONSOLE   "psp"
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
#  elif defined(__linux__) || defined(__linux) || defined(__LINUX__)
#    define ACC_OS_POSIX_LINUX      1
#    define ACC_INFO_OS_POSIX       "linux"
#  elif defined(__APPLE__) && defined(__MACH__)
#    if ((__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__-0) >= 20000)
#      define ACC_OS_POSIX_DARWIN     1040
#      define ACC_INFO_OS_POSIX       "darwin_iphone"
#    elif ((__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__-0) >= 1040)
#      define ACC_OS_POSIX_DARWIN     __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#      define ACC_INFO_OS_POSIX       "darwin"
#    else
#      define ACC_OS_POSIX_DARWIN     1
#      define ACC_INFO_OS_POSIX       "darwin"
#    endif
#    define ACC_OS_POSIX_MACOSX     ACC_OS_POSIX_DARWIN
#  elif defined(__minix__) || defined(__minix)
#    define ACC_OS_POSIX_MINIX      1
#    define ACC_INFO_OS_POSIX       "minix"
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
#  elif defined(_UNICOS)
#    define ACC_OS_POSIX_UNICOS     1
#    define ACC_INFO_OS_POSIX       "unicos"
#  else
#    define ACC_OS_POSIX_UNKNOWN    1
#    define ACC_INFO_OS_POSIX       "unknown"
#  endif
#endif
#endif
#if (ACC_OS_DOS16 || ACC_OS_OS216 || ACC_OS_WIN16)
#  if (UINT_MAX != ACC_0xffffL)
#    error "unexpected configuration - check your compiler defines"
#  endif
#  if (ULONG_MAX != ACC_0xffffffffL)
#    error "unexpected configuration - check your compiler defines"
#  endif
#endif
#if (ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_WIN32 || ACC_OS_WIN64)
#  if (UINT_MAX != ACC_0xffffffffL)
#    error "unexpected configuration - check your compiler defines"
#  endif
#  if (ULONG_MAX != ACC_0xffffffffL)
#    error "unexpected configuration - check your compiler defines"
#  endif
#endif
#if defined(CIL) && defined(_GNUCC) && defined(__GNUC__)
#  define ACC_CC_CILLY          1
#  define ACC_INFO_CC           "Cilly"
#  if defined(__CILLY__)
#    define ACC_INFO_CCVER      ACC_PP_MACRO_EXPAND(__CILLY__)
#  else
#    define ACC_INFO_CCVER      "unknown"
#  endif
#elif 0 && defined(SDCC) && defined(__VERSION__) && !defined(__GNUC__)
#  define ACC_CC_SDCC           1
#  define ACC_INFO_CC           "sdcc"
#  define ACC_INFO_CCVER        ACC_PP_MACRO_EXPAND(SDCC)
#elif defined(__PATHSCALE__) && defined(__PATHCC_PATCHLEVEL__)
#  define ACC_CC_PATHSCALE      (__PATHCC__ * 0x10000L + (__PATHCC_MINOR__-0) * 0x100 + (__PATHCC_PATCHLEVEL__-0))
#  define ACC_INFO_CC           "Pathscale C"
#  define ACC_INFO_CCVER        __PATHSCALE__
#elif defined(__INTEL_COMPILER) && ((__INTEL_COMPILER-0) > 0)
#  define ACC_CC_INTELC         __INTEL_COMPILER
#  define ACC_INFO_CC           "Intel C"
#  define ACC_INFO_CCVER        ACC_PP_MACRO_EXPAND(__INTEL_COMPILER)
#  if defined(_MSC_VER) && ((_MSC_VER-0) > 0)
#    define ACC_CC_INTELC_MSC   _MSC_VER
#  elif defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__VERSION__)
#    define ACC_CC_INTELC_GNUC   (__GNUC__ * 0x10000L + (__GNUC_MINOR__-0) * 0x100 + (__GNUC_PATCHLEVEL__-0))
#  endif
#elif defined(__POCC__) && defined(_WIN32)
#  define ACC_CC_PELLESC        1
#  define ACC_INFO_CC           "Pelles C"
#  define ACC_INFO_CCVER        ACC_PP_MACRO_EXPAND(__POCC__)
#elif defined(__ARMCC_VERSION) && defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__VERSION__)
#  if defined(__GNUC_PATCHLEVEL__)
#    define ACC_CC_ARMCC_GNUC   (__GNUC__ * 0x10000L + (__GNUC_MINOR__-0) * 0x100 + (__GNUC_PATCHLEVEL__-0))
#  else
#    define ACC_CC_ARMCC_GNUC   (__GNUC__ * 0x10000L + (__GNUC_MINOR__-0) * 0x100)
#  endif
#  define ACC_CC_ARMCC          __ARMCC_VERSION
#  define ACC_INFO_CC           "ARM C Compiler"
#  define ACC_INFO_CCVER        __VERSION__
#elif defined(__clang__) && defined(__llvm__) && defined(__VERSION__)
#  if defined(__clang_major__) && defined(__clang_minor__) && defined(__clang_patchlevel__)
#    define ACC_CC_CLANG        (__clang_major__ * 0x10000L + (__clang_minor__-0) * 0x100 + (__clang_patchlevel__-0))
#  else
#    define ACC_CC_CLANG        0x010000L
#  endif
#  if defined(_MSC_VER) && ((_MSC_VER-0) > 0)
#    define ACC_CC_CLANG_MSC    _MSC_VER
#  elif defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__VERSION__)
#    define ACC_CC_CLANG_GNUC   (__GNUC__ * 0x10000L + (__GNUC_MINOR__-0) * 0x100 + (__GNUC_PATCHLEVEL__-0))
#  endif
#  define ACC_INFO_CC           "clang"
#  define ACC_INFO_CCVER        __VERSION__
#elif defined(__llvm__) && defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__VERSION__)
#  if defined(__GNUC_PATCHLEVEL__)
#    define ACC_CC_LLVM_GNUC    (__GNUC__ * 0x10000L + (__GNUC_MINOR__-0) * 0x100 + (__GNUC_PATCHLEVEL__-0))
#  else
#    define ACC_CC_LLVM_GNUC    (__GNUC__ * 0x10000L + (__GNUC_MINOR__-0) * 0x100)
#  endif
#  define ACC_CC_LLVM           ACC_CC_LLVM_GNUC
#  define ACC_INFO_CC           "llvm-gcc"
#  define ACC_INFO_CCVER        __VERSION__
#elif defined(__GNUC__) && defined(__VERSION__)
#  if defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
#    define ACC_CC_GNUC         (__GNUC__ * 0x10000L + (__GNUC_MINOR__-0) * 0x100 + (__GNUC_PATCHLEVEL__-0))
#  elif defined(__GNUC_MINOR__)
#    define ACC_CC_GNUC         (__GNUC__ * 0x10000L + (__GNUC_MINOR__-0) * 0x100)
#  else
#    define ACC_CC_GNUC         (__GNUC__ * 0x10000L)
#  endif
#  define ACC_INFO_CC           "gcc"
#  define ACC_INFO_CCVER        __VERSION__
#elif defined(__ACK__) && defined(_ACK)
#  define ACC_CC_ACK            1
#  define ACC_INFO_CC           "Amsterdam Compiler Kit C"
#  define ACC_INFO_CCVER        "unknown"
#elif defined(__ARMCC_VERSION) && !defined(__GNUC__)
#  define ACC_CC_ARMCC          __ARMCC_VERSION
#  define ACC_CC_ARMCC_ARMCC    __ARMCC_VERSION
#  define ACC_INFO_CC           "ARM C Compiler"
#  define ACC_INFO_CCVER        ACC_PP_MACRO_EXPAND(__ARMCC_VERSION)
#elif defined(__AZTEC_C__)
#  define ACC_CC_AZTECC         1
#  define ACC_INFO_CC           "Aztec C"
#  define ACC_INFO_CCVER        ACC_PP_MACRO_EXPAND(__AZTEC_C__)
#elif defined(__CODEGEARC__)
#  define ACC_CC_CODEGEARC      1
#  define ACC_INFO_CC           "CodeGear C"
#  define ACC_INFO_CCVER        ACC_PP_MACRO_EXPAND(__CODEGEARC__)
#elif defined(__BORLANDC__)
#  define ACC_CC_BORLANDC       1
#  define ACC_INFO_CC           "Borland C"
#  define ACC_INFO_CCVER        ACC_PP_MACRO_EXPAND(__BORLANDC__)
#elif defined(_CRAYC) && defined(_RELEASE)
#  define ACC_CC_CRAYC          1
#  define ACC_INFO_CC           "Cray C"
#  define ACC_INFO_CCVER        ACC_PP_MACRO_EXPAND(_RELEASE)
#elif defined(__DMC__) && defined(__SC__)
#  define ACC_CC_DMC            1
#  define ACC_INFO_CC           "Digital Mars C"
#  define ACC_INFO_CCVER        ACC_PP_MACRO_EXPAND(__DMC__)
#elif defined(__DECC)
#  define ACC_CC_DECC           1
#  define ACC_INFO_CC           "DEC C"
#  define ACC_INFO_CCVER        ACC_PP_MACRO_EXPAND(__DECC)
#elif defined(__HIGHC__)
#  define ACC_CC_HIGHC          1
#  define ACC_INFO_CC           "MetaWare High C"
#  define ACC_INFO_CCVER        "unknown"
#elif defined(__IAR_SYSTEMS_ICC__)
#  define ACC_CC_IARC           1
#  define ACC_INFO_CC           "IAR C"
#  if defined(__VER__)
#    define ACC_INFO_CCVER      ACC_PP_MACRO_EXPAND(__VER__)
#  else
#    define ACC_INFO_CCVER      "unknown"
#  endif
#elif defined(__IBMC__)
#  define ACC_CC_IBMC           1
#  define ACC_INFO_CC           "IBM C"
#  define ACC_INFO_CCVER        ACC_PP_MACRO_EXPAND(__IBMC__)
#elif defined(__KEIL__) && defined(__C166__)
#  define ACC_CC_KEILC          1
#  define ACC_INFO_CC           "Keil C"
#  define ACC_INFO_CCVER        ACC_PP_MACRO_EXPAND(__C166__)
#elif defined(__LCC__) && defined(_WIN32) && defined(__LCCOPTIMLEVEL)
#  define ACC_CC_LCCWIN32       1
#  define ACC_INFO_CC           "lcc-win32"
#  define ACC_INFO_CCVER        "unknown"
#elif defined(__LCC__)
#  define ACC_CC_LCC            1
#  define ACC_INFO_CC           "lcc"
#  if defined(__LCC_VERSION__)
#    define ACC_INFO_CCVER      ACC_PP_MACRO_EXPAND(__LCC_VERSION__)
#  else
#    define ACC_INFO_CCVER      "unknown"
#  endif
#elif defined(_MSC_VER) && ((_MSC_VER-0) > 0)
#  define ACC_CC_MSC            _MSC_VER
#  define ACC_INFO_CC           "Microsoft C"
#  if defined(_MSC_FULL_VER)
#    define ACC_INFO_CCVER      ACC_PP_MACRO_EXPAND(_MSC_VER) "." ACC_PP_MACRO_EXPAND(_MSC_FULL_VER)
#  else
#    define ACC_INFO_CCVER      ACC_PP_MACRO_EXPAND(_MSC_VER)
#  endif
#elif defined(__MWERKS__)
#  define ACC_CC_MWERKS         1
#  define ACC_INFO_CC           "Metrowerks C"
#  define ACC_INFO_CCVER        ACC_PP_MACRO_EXPAND(__MWERKS__)
#elif (defined(__NDPC__) || defined(__NDPX__)) && defined(__i386)
#  define ACC_CC_NDPC           1
#  define ACC_INFO_CC           "Microway NDP C"
#  define ACC_INFO_CCVER        "unknown"
#elif defined(__PACIFIC__)
#  define ACC_CC_PACIFICC       1
#  define ACC_INFO_CC           "Pacific C"
#  define ACC_INFO_CCVER        ACC_PP_MACRO_EXPAND(__PACIFIC__)
#elif defined(__PGI) && (defined(__linux__) || defined(__WIN32__))
#  define ACC_CC_PGI            1
#  define ACC_INFO_CC           "Portland Group PGI C"
#  define ACC_INFO_CCVER        "unknown"
#elif defined(__PUREC__) && defined(__TOS__)
#  define ACC_CC_PUREC          1
#  define ACC_INFO_CC           "Pure C"
#  define ACC_INFO_CCVER        ACC_PP_MACRO_EXPAND(__PUREC__)
#elif defined(__SC__) && defined(__ZTC__)
#  define ACC_CC_SYMANTECC      1
#  define ACC_INFO_CC           "Symantec C"
#  define ACC_INFO_CCVER        ACC_PP_MACRO_EXPAND(__SC__)
#elif defined(__SUNPRO_C)
#  define ACC_INFO_CC           "SunPro C"
#  if ((__SUNPRO_C-0) > 0)
#    define ACC_CC_SUNPROC      __SUNPRO_C
#    define ACC_INFO_CCVER      ACC_PP_MACRO_EXPAND(__SUNPRO_C)
#  else
#    define ACC_CC_SUNPROC      1
#    define ACC_INFO_CCVER      "unknown"
#  endif
#elif defined(__SUNPRO_CC)
#  define ACC_INFO_CC           "SunPro C"
#  if ((__SUNPRO_CC-0) > 0)
#    define ACC_CC_SUNPROC      __SUNPRO_CC
#    define ACC_INFO_CCVER      ACC_PP_MACRO_EXPAND(__SUNPRO_CC)
#  else
#    define ACC_CC_SUNPROC      1
#    define ACC_INFO_CCVER      "unknown"
#  endif
#elif defined(__TINYC__)
#  define ACC_CC_TINYC          1
#  define ACC_INFO_CC           "Tiny C"
#  define ACC_INFO_CCVER        ACC_PP_MACRO_EXPAND(__TINYC__)
#elif defined(__TSC__)
#  define ACC_CC_TOPSPEEDC      1
#  define ACC_INFO_CC           "TopSpeed C"
#  define ACC_INFO_CCVER        ACC_PP_MACRO_EXPAND(__TSC__)
#elif defined(__WATCOMC__)
#  define ACC_CC_WATCOMC        1
#  define ACC_INFO_CC           "Watcom C"
#  define ACC_INFO_CCVER        ACC_PP_MACRO_EXPAND(__WATCOMC__)
#elif defined(__TURBOC__)
#  define ACC_CC_TURBOC         1
#  define ACC_INFO_CC           "Turbo C"
#  define ACC_INFO_CCVER        ACC_PP_MACRO_EXPAND(__TURBOC__)
#elif defined(__ZTC__)
#  define ACC_CC_ZORTECHC       1
#  define ACC_INFO_CC           "Zortech C"
#  if (__ZTC__ == 0x310)
#    define ACC_INFO_CCVER      "0x310"
#  else
#    define ACC_INFO_CCVER      ACC_PP_MACRO_EXPAND(__ZTC__)
#  endif
#else
#  define ACC_CC_UNKNOWN        1
#  define ACC_INFO_CC           "unknown"
#  define ACC_INFO_CCVER        "unknown"
#endif
#if (ACC_CC_GNUC) && defined(__OPEN64__)
#  if defined(__OPENCC__) && defined(__OPENCC_MINOR__) && defined(__OPENCC_PATCHLEVEL__)
#    define ACC_CC_OPEN64       (__OPENCC__ * 0x10000L + (__OPENCC_MINOR__-0) * 0x100 + (__OPENCC_PATCHLEVEL__-0))
#    define ACC_CC_OPEN64_GNUC  ACC_CC_GNUC
#  endif
#endif
#if (ACC_CC_GNUC) && defined(__PCC__)
#  if defined(__PCC__) && defined(__PCC_MINOR__) && defined(__PCC_MINORMINOR__)
#    define ACC_CC_PCC          (__PCC__ * 0x10000L + (__PCC_MINOR__-0) * 0x100 + (__PCC_MINORMINOR__-0))
#    define ACC_CC_PCC_GNUC     ACC_CC_GNUC
#  endif
#endif
#if 0 && (ACC_CC_MSC && (_MSC_VER >= 1200)) && !defined(_MSC_FULL_VER)
#  error "ACC_CC_MSC: _MSC_FULL_VER is not defined"
#endif
#if !defined(__ACC_ARCH_OVERRIDE) && !(ACC_ARCH_GENERIC) && defined(_CRAY)
#  if (UINT_MAX > ACC_0xffffffffL) && defined(_CRAY)
#    if defined(_CRAYMPP) || defined(_CRAYT3D) || defined(_CRAYT3E)
#      define ACC_ARCH_CRAY_MPP     1
#    elif defined(_CRAY1)
#      define ACC_ARCH_CRAY_PVP     1
#    endif
#  endif
#endif
#if !defined(__ACC_ARCH_OVERRIDE)
#if (ACC_ARCH_GENERIC)
#  define ACC_INFO_ARCH             "generic"
#elif (ACC_OS_DOS16 || ACC_OS_OS216 || ACC_OS_WIN16)
#  define ACC_ARCH_I086             1
#  define ACC_INFO_ARCH             "i086"
#elif defined(__aarch64__)
#  define ACC_ARCH_ARM64            1
#  define ACC_INFO_ARCH             "arm64"
#elif defined(__alpha__) || defined(__alpha) || defined(_M_ALPHA)
#  define ACC_ARCH_ALPHA            1
#  define ACC_INFO_ARCH             "alpha"
#elif (ACC_ARCH_CRAY_MPP) && (defined(_CRAYT3D) || defined(_CRAYT3E))
#  define ACC_ARCH_ALPHA            1
#  define ACC_INFO_ARCH             "alpha"
#elif defined(__amd64__) || defined(__x86_64__) || defined(_M_AMD64)
#  define ACC_ARCH_AMD64            1
#  define ACC_INFO_ARCH             "amd64"
#elif defined(__thumb__) || (defined(_M_ARM) && defined(_M_THUMB))
#  define ACC_ARCH_ARM              1
#  define ACC_ARCH_ARM_THUMB        1
#  define ACC_INFO_ARCH             "arm_thumb"
#elif defined(__IAR_SYSTEMS_ICC__) && defined(__ICCARM__)
#  define ACC_ARCH_ARM              1
#  if defined(__CPU_MODE__) && ((__CPU_MODE__-0) == 1)
#    define ACC_ARCH_ARM_THUMB      1
#    define ACC_INFO_ARCH           "arm_thumb"
#  elif defined(__CPU_MODE__) && ((__CPU_MODE__-0) == 2)
#    define ACC_INFO_ARCH           "arm"
#  else
#    define ACC_INFO_ARCH           "arm"
#  endif
#elif defined(__arm__) || defined(_M_ARM)
#  define ACC_ARCH_ARM              1
#  define ACC_INFO_ARCH             "arm"
#elif (UINT_MAX <= ACC_0xffffL) && defined(__AVR__)
#  define ACC_ARCH_AVR              1
#  define ACC_INFO_ARCH             "avr"
#elif defined(__avr32__) || defined(__AVR32__)
#  define ACC_ARCH_AVR32            1
#  define ACC_INFO_ARCH             "avr32"
#elif defined(__bfin__)
#  define ACC_ARCH_BLACKFIN         1
#  define ACC_INFO_ARCH             "blackfin"
#elif (UINT_MAX == ACC_0xffffL) && defined(__C166__)
#  define ACC_ARCH_C166             1
#  define ACC_INFO_ARCH             "c166"
#elif defined(__cris__)
#  define ACC_ARCH_CRIS             1
#  define ACC_INFO_ARCH             "cris"
#elif defined(__IAR_SYSTEMS_ICC__) && defined(__ICCEZ80__)
#  define ACC_ARCH_EZ80             1
#  define ACC_INFO_ARCH             "ez80"
#elif defined(__H8300__) || defined(__H8300H__) || defined(__H8300S__) || defined(__H8300SX__)
#  define ACC_ARCH_H8300            1
#  define ACC_INFO_ARCH             "h8300"
#elif defined(__hppa__) || defined(__hppa)
#  define ACC_ARCH_HPPA             1
#  define ACC_INFO_ARCH             "hppa"
#elif defined(__386__) || defined(__i386__) || defined(__i386) || defined(_M_IX86) || defined(_M_I386)
#  define ACC_ARCH_I386             1
#  define ACC_ARCH_IA32             1
#  define ACC_INFO_ARCH             "i386"
#elif (ACC_CC_ZORTECHC && defined(__I86__))
#  define ACC_ARCH_I386             1
#  define ACC_ARCH_IA32             1
#  define ACC_INFO_ARCH             "i386"
#elif (ACC_OS_DOS32 && ACC_CC_HIGHC) && defined(_I386)
#  define ACC_ARCH_I386             1
#  define ACC_ARCH_IA32             1
#  define ACC_INFO_ARCH             "i386"
#elif defined(__ia64__) || defined(__ia64) || defined(_M_IA64)
#  define ACC_ARCH_IA64             1
#  define ACC_INFO_ARCH             "ia64"
#elif (UINT_MAX == ACC_0xffffL) && defined(__m32c__)
#  define ACC_ARCH_M16C             1
#  define ACC_INFO_ARCH             "m16c"
#elif defined(__IAR_SYSTEMS_ICC__) && defined(__ICCM16C__)
#  define ACC_ARCH_M16C             1
#  define ACC_INFO_ARCH             "m16c"
#elif defined(__m32r__)
#  define ACC_ARCH_M32R             1
#  define ACC_INFO_ARCH             "m32r"
#elif (ACC_OS_TOS) || defined(__m68k__) || defined(__m68000__) || defined(__mc68000__) || defined(__mc68020__) || defined(_M_M68K)
#  define ACC_ARCH_M68K             1
#  define ACC_INFO_ARCH             "m68k"
#elif (UINT_MAX == ACC_0xffffL) && defined(__C251__)
#  define ACC_ARCH_MCS251           1
#  define ACC_INFO_ARCH             "mcs251"
#elif (UINT_MAX == ACC_0xffffL) && defined(__C51__)
#  define ACC_ARCH_MCS51            1
#  define ACC_INFO_ARCH             "mcs51"
#elif defined(__IAR_SYSTEMS_ICC__) && defined(__ICC8051__)
#  define ACC_ARCH_MCS51            1
#  define ACC_INFO_ARCH             "mcs51"
#elif defined(__mips__) || defined(__mips) || defined(_MIPS_ARCH) || defined(_M_MRX000)
#  define ACC_ARCH_MIPS             1
#  define ACC_INFO_ARCH             "mips"
#elif (UINT_MAX == ACC_0xffffL) && defined(__MSP430__)
#  define ACC_ARCH_MSP430           1
#  define ACC_INFO_ARCH             "msp430"
#elif defined(__IAR_SYSTEMS_ICC__) && defined(__ICC430__)
#  define ACC_ARCH_MSP430           1
#  define ACC_INFO_ARCH             "msp430"
#elif defined(__powerpc__) || defined(__powerpc) || defined(__ppc__) || defined(__PPC__) || defined(_M_PPC) || defined(_ARCH_PPC) || defined(_ARCH_PWR)
#  define ACC_ARCH_POWERPC          1
#  define ACC_INFO_ARCH             "powerpc"
#elif defined(__s390__) || defined(__s390) || defined(__s390x__) || defined(__s390x)
#  define ACC_ARCH_S390             1
#  define ACC_INFO_ARCH             "s390"
#elif defined(__sh__) || defined(_M_SH)
#  define ACC_ARCH_SH               1
#  define ACC_INFO_ARCH             "sh"
#elif defined(__sparc__) || defined(__sparc) || defined(__sparcv8)
#  define ACC_ARCH_SPARC            1
#  define ACC_INFO_ARCH             "sparc"
#elif defined(__SPU__)
#  define ACC_ARCH_SPU              1
#  define ACC_INFO_ARCH             "spu"
#elif (UINT_MAX == ACC_0xffffL) && defined(__z80)
#  define ACC_ARCH_Z80              1
#  define ACC_INFO_ARCH             "z80"
#elif (ACC_ARCH_CRAY_PVP)
#  if defined(_CRAYSV1)
#    define ACC_ARCH_CRAY_SV1       1
#    define ACC_INFO_ARCH           "cray_sv1"
#  elif (_ADDR64)
#    define ACC_ARCH_CRAY_T90       1
#    define ACC_INFO_ARCH           "cray_t90"
#  elif (_ADDR32)
#    define ACC_ARCH_CRAY_YMP       1
#    define ACC_INFO_ARCH           "cray_ymp"
#  else
#    define ACC_ARCH_CRAY_XMP       1
#    define ACC_INFO_ARCH           "cray_xmp"
#  endif
#else
#  define ACC_ARCH_UNKNOWN          1
#  define ACC_INFO_ARCH             "unknown"
#endif
#endif
#if 1 && (ACC_ARCH_UNKNOWN) && (ACC_OS_DOS32 || ACC_OS_OS2)
#  error "FIXME - missing define for CPU architecture"
#endif
#if 1 && (ACC_ARCH_UNKNOWN) && (ACC_OS_WIN32)
#  error "FIXME - missing ACC_OS_WIN32 define for CPU architecture"
#endif
#if 1 && (ACC_ARCH_UNKNOWN) && (ACC_OS_WIN64)
#  error "FIXME - missing ACC_OS_WIN64 define for CPU architecture"
#endif
#if (ACC_OS_OS216 || ACC_OS_WIN16)
#  define ACC_ARCH_I086PM           1
#elif 1 && (ACC_OS_DOS16 && defined(BLX286))
#  define ACC_ARCH_I086PM           1
#elif 1 && (ACC_OS_DOS16 && defined(DOSX286))
#  define ACC_ARCH_I086PM           1
#elif 1 && (ACC_OS_DOS16 && ACC_CC_BORLANDC && defined(__DPMI16__))
#  define ACC_ARCH_I086PM           1
#endif
#if (ACC_ARCH_AMD64 && !ACC_ARCH_X64)
#  define ACC_ARCH_X64              1
#elif (!ACC_ARCH_AMD64 && ACC_ARCH_X64) && defined(__ACC_ARCH_OVERRIDE)
#  define ACC_ARCH_AMD64            1
#endif
#if (ACC_ARCH_ARM64 && !ACC_ARCH_AARCH64)
#  define ACC_ARCH_AARCH64          1
#elif (!ACC_ARCH_ARM64 && ACC_ARCH_AARCH64) && defined(__ACC_ARCH_OVERRIDE)
#  define ACC_ARCH_ARM64            1
#endif
#if (ACC_ARCH_I386 && !ACC_ARCH_X86)
#  define ACC_ARCH_X86              1
#elif (!ACC_ARCH_I386 && ACC_ARCH_X86) && defined(__ACC_ARCH_OVERRIDE)
#  define ACC_ARCH_I386            1
#endif
#if (ACC_ARCH_AMD64 && !ACC_ARCH_X64) || (!ACC_ARCH_AMD64 && ACC_ARCH_X64)
#  error "unexpected configuration - check your compiler defines"
#endif
#if (ACC_ARCH_ARM64 && !ACC_ARCH_AARCH64) || (!ACC_ARCH_ARM64 && ACC_ARCH_AARCH64)
#  error "unexpected configuration - check your compiler defines"
#endif
#if (ACC_ARCH_I386 && !ACC_ARCH_X86) || (!ACC_ARCH_I386 && ACC_ARCH_X86)
#  error "unexpected configuration - check your compiler defines"
#endif
#if (ACC_ARCH_ARM_THUMB && !ACC_ARCH_ARM)
#  error "unexpected configuration - check your compiler defines"
#endif
#if (ACC_ARCH_ARM_THUMB1 && !ACC_ARCH_ARM_THUMB)
#  error "unexpected configuration - check your compiler defines"
#endif
#if (ACC_ARCH_ARM_THUMB2 && !ACC_ARCH_ARM_THUMB)
#  error "unexpected configuration - check your compiler defines"
#endif
#if (ACC_ARCH_ARM_THUMB1 && ACC_ARCH_ARM_THUMB2)
#  error "unexpected configuration - check your compiler defines"
#endif
#if (ACC_ARCH_I086PM && !ACC_ARCH_I086)
#  error "unexpected configuration - check your compiler defines"
#endif
#if (ACC_ARCH_I086)
#  if (UINT_MAX != ACC_0xffffL)
#    error "unexpected configuration - check your compiler defines"
#  endif
#  if (ULONG_MAX != ACC_0xffffffffL)
#    error "unexpected configuration - check your compiler defines"
#  endif
#endif
#if (ACC_ARCH_I386)
#  if (UINT_MAX != ACC_0xffffL) && defined(__i386_int16__)
#    error "unexpected configuration - check your compiler defines"
#  endif
#  if (UINT_MAX != ACC_0xffffffffL) && !defined(__i386_int16__)
#    error "unexpected configuration - check your compiler defines"
#  endif
#  if (ULONG_MAX != ACC_0xffffffffL)
#    error "unexpected configuration - check your compiler defines"
#  endif
#endif
#if !defined(__ACC_MM_OVERRIDE)
#if (ACC_ARCH_I086)
#if (UINT_MAX != ACC_0xffffL)
#  error "unexpected configuration - check your compiler defines"
#endif
#if defined(__TINY__) || defined(M_I86TM) || defined(_M_I86TM)
#  define ACC_MM_TINY           1
#elif defined(__HUGE__) || defined(_HUGE_) || defined(M_I86HM) || defined(_M_I86HM)
#  define ACC_MM_HUGE           1
#elif defined(__SMALL__) || defined(M_I86SM) || defined(_M_I86SM) || defined(SMALL_MODEL)
#  define ACC_MM_SMALL          1
#elif defined(__MEDIUM__) || defined(M_I86MM) || defined(_M_I86MM)
#  define ACC_MM_MEDIUM         1
#elif defined(__COMPACT__) || defined(M_I86CM) || defined(_M_I86CM)
#  define ACC_MM_COMPACT        1
#elif defined(__LARGE__) || defined(M_I86LM) || defined(_M_I86LM) || defined(LARGE_MODEL)
#  define ACC_MM_LARGE          1
#elif (ACC_CC_AZTECC)
#  if defined(_LARGE_CODE) && defined(_LARGE_DATA)
#    define ACC_MM_LARGE        1
#  elif defined(_LARGE_CODE)
#    define ACC_MM_MEDIUM       1
#  elif defined(_LARGE_DATA)
#    define ACC_MM_COMPACT      1
#  else
#    define ACC_MM_SMALL        1
#  endif
#elif (ACC_CC_ZORTECHC && defined(__VCM__))
#  define ACC_MM_LARGE          1
#else
#  error "unknown ACC_ARCH_I086 memory model"
#endif
#if (ACC_OS_DOS16 || ACC_OS_OS216 || ACC_OS_WIN16)
#define ACC_HAVE_MM_HUGE_PTR        1
#define ACC_HAVE_MM_HUGE_ARRAY      1
#if (ACC_MM_TINY)
#  undef ACC_HAVE_MM_HUGE_ARRAY
#endif
#if (ACC_CC_AZTECC || ACC_CC_PACIFICC || ACC_CC_ZORTECHC)
#  undef ACC_HAVE_MM_HUGE_PTR
#  undef ACC_HAVE_MM_HUGE_ARRAY
#elif (ACC_CC_DMC || ACC_CC_SYMANTECC)
#  undef ACC_HAVE_MM_HUGE_ARRAY
#elif (ACC_CC_MSC && defined(_QC))
#  undef ACC_HAVE_MM_HUGE_ARRAY
#  if (_MSC_VER < 600)
#    undef ACC_HAVE_MM_HUGE_PTR
#  endif
#elif (ACC_CC_TURBOC && (__TURBOC__ < 0x0295))
#  undef ACC_HAVE_MM_HUGE_ARRAY
#endif
#if (ACC_ARCH_I086PM) && !(ACC_HAVE_MM_HUGE_PTR)
#  if (ACC_OS_DOS16)
#    error "unexpected configuration - check your compiler defines"
#  elif (ACC_CC_ZORTECHC)
#  else
#    error "unexpected configuration - check your compiler defines"
#  endif
#endif
#ifdef __cplusplus
extern "C" {
#endif
#if (ACC_CC_BORLANDC && (__BORLANDC__ >= 0x0200))
   extern void __near __cdecl _AHSHIFT(void);
#  define ACC_MM_AHSHIFT      ((unsigned) _AHSHIFT)
#elif (ACC_CC_DMC || ACC_CC_SYMANTECC || ACC_CC_ZORTECHC)
   extern void __near __cdecl _AHSHIFT(void);
#  define ACC_MM_AHSHIFT      ((unsigned) _AHSHIFT)
#elif (ACC_CC_MSC || ACC_CC_TOPSPEEDC)
   extern void __near __cdecl _AHSHIFT(void);
#  define ACC_MM_AHSHIFT      ((unsigned) _AHSHIFT)
#elif (ACC_CC_TURBOC && (__TURBOC__ >= 0x0295))
   extern void __near __cdecl _AHSHIFT(void);
#  define ACC_MM_AHSHIFT      ((unsigned) _AHSHIFT)
#elif ((ACC_CC_AZTECC || ACC_CC_PACIFICC || ACC_CC_TURBOC) && ACC_OS_DOS16)
#  define ACC_MM_AHSHIFT      12
#elif (ACC_CC_WATCOMC)
   extern unsigned char _HShift;
#  define ACC_MM_AHSHIFT      ((unsigned) _HShift)
#else
#  error "FIXME - implement ACC_MM_AHSHIFT"
#endif
#ifdef __cplusplus
}
#endif
#endif
#elif (ACC_ARCH_C166)
#if !defined(__MODEL__)
#  error "FIXME - ACC_ARCH_C166 __MODEL__"
#elif ((__MODEL__) == 0)
#  define ACC_MM_SMALL          1
#elif ((__MODEL__) == 1)
#  define ACC_MM_SMALL          1
#elif ((__MODEL__) == 2)
#  define ACC_MM_LARGE          1
#elif ((__MODEL__) == 3)
#  define ACC_MM_TINY           1
#elif ((__MODEL__) == 4)
#  define ACC_MM_XTINY          1
#elif ((__MODEL__) == 5)
#  define ACC_MM_XSMALL         1
#else
#  error "FIXME - ACC_ARCH_C166 __MODEL__"
#endif
#elif (ACC_ARCH_MCS251)
#if !defined(__MODEL__)
#  error "FIXME - ACC_ARCH_MCS251 __MODEL__"
#elif ((__MODEL__) == 0)
#  define ACC_MM_SMALL          1
#elif ((__MODEL__) == 2)
#  define ACC_MM_LARGE          1
#elif ((__MODEL__) == 3)
#  define ACC_MM_TINY           1
#elif ((__MODEL__) == 4)
#  define ACC_MM_XTINY          1
#elif ((__MODEL__) == 5)
#  define ACC_MM_XSMALL         1
#else
#  error "FIXME - ACC_ARCH_MCS251 __MODEL__"
#endif
#elif (ACC_ARCH_MCS51)
#if !defined(__MODEL__)
#  error "FIXME - ACC_ARCH_MCS51 __MODEL__"
#elif ((__MODEL__) == 1)
#  define ACC_MM_SMALL          1
#elif ((__MODEL__) == 2)
#  define ACC_MM_LARGE          1
#elif ((__MODEL__) == 3)
#  define ACC_MM_TINY           1
#elif ((__MODEL__) == 4)
#  define ACC_MM_XTINY          1
#elif ((__MODEL__) == 5)
#  define ACC_MM_XSMALL         1
#else
#  error "FIXME - ACC_ARCH_MCS51 __MODEL__"
#endif
#elif (ACC_ARCH_CRAY_PVP)
#  define ACC_MM_PVP            1
#else
#  define ACC_MM_FLAT           1
#endif
#if (ACC_MM_COMPACT)
#  define ACC_INFO_MM           "compact"
#elif (ACC_MM_FLAT)
#  define ACC_INFO_MM           "flat"
#elif (ACC_MM_HUGE)
#  define ACC_INFO_MM           "huge"
#elif (ACC_MM_LARGE)
#  define ACC_INFO_MM           "large"
#elif (ACC_MM_MEDIUM)
#  define ACC_INFO_MM           "medium"
#elif (ACC_MM_PVP)
#  define ACC_INFO_MM           "pvp"
#elif (ACC_MM_SMALL)
#  define ACC_INFO_MM           "small"
#elif (ACC_MM_TINY)
#  define ACC_INFO_MM           "tiny"
#else
#  error "unknown memory model"
#endif
#endif
#if !defined(__acc_gnuc_extension__)
#if (ACC_CC_GNUC >= 0x020800ul)
#  define __acc_gnuc_extension__    __extension__
#elif (ACC_CC_ARMCC_GNUC || ACC_CC_CLANG || ACC_CC_LLVM || ACC_CC_PATHSCALE)
#  define __acc_gnuc_extension__    __extension__
#else
#  define __acc_gnuc_extension__    /*empty*/
#endif
#endif
#if !defined(__acc_ua_volatile)
#  define __acc_ua_volatile     volatile
#endif
#if !defined(__acc_alignof)
#if (ACC_CC_ARMCC || ACC_CC_CILLY || ACC_CC_CLANG || ACC_CC_GNUC || ACC_CC_LLVM || ACC_CC_PATHSCALE || ACC_CC_PGI)
#  define __acc_alignof(e)      __alignof__(e)
#elif (ACC_CC_INTELC && (__INTEL_COMPILER >= 700))
#  define __acc_alignof(e)      __alignof__(e)
#elif (ACC_CC_MSC && (_MSC_VER >= 1300))
#  define __acc_alignof(e)      __alignof(e)
#elif (ACC_CC_SUNPROC && (ACC_CC_SUNPROC >= 0x5100))
#  define __acc_alignof(e)      __alignof__(e)
#endif
#endif
#if defined(__acc_alignof) && !defined(__acc_HAVE_alignof)
#  define __acc_HAVE_alignof 1
#endif
#if !defined(__acc_struct_align16) && (__acc_HAVE_alignof)
#if (ACC_CC_GNUC && (ACC_CC_GNUC < 0x030000ul))
#elif (ACC_CC_CLANG && (ACC_CC_CLANG < 0x020800ul)) && defined(__cplusplus)
#elif (ACC_CC_CILLY || ACC_CC_PCC)
#elif (ACC_CC_INTELC_MSC) || (ACC_CC_MSC && (_MSC_VER >= 1300))
#  define __acc_struct_align16(s)       struct __declspec(align(16)) s {
#  define __acc_struct_align16_end()    };
#  define __acc_struct_align32(s)       struct __declspec(align(32)) s {
#  define __acc_struct_align32_end()    };
#  define __acc_struct_align64(s)       struct __declspec(align(64)) s {
#  define __acc_struct_align64_end()    };
#elif (ACC_CC_ARMCC || ACC_CC_CLANG || ACC_CC_GNUC || ACC_CC_INTELC_GNUC || ACC_CC_LLVM || ACC_CC_PATHSCALE)
#  define __acc_struct_align16(s)       struct s {
#  define __acc_struct_align16_end()    } __attribute__((__aligned__(16)));
#  define __acc_struct_align32(s)       struct s {
#  define __acc_struct_align32_end()    } __attribute__((__aligned__(32)));
#  define __acc_struct_align64(s)       struct s {
#  define __acc_struct_align64_end()    } __attribute__((__aligned__(64)));
#endif
#endif
#if !defined(__acc_constructor)
#if (ACC_CC_GNUC >= 0x030400ul)
#  define __acc_constructor     __attribute__((__constructor__,__used__))
#elif (ACC_CC_GNUC >= 0x020700ul)
#  define __acc_constructor     __attribute__((__constructor__))
#elif (ACC_CC_ARMCC_GNUC || ACC_CC_CLANG || ACC_CC_LLVM || ACC_CC_PATHSCALE)
#  define __acc_constructor     __attribute__((__constructor__))
#endif
#endif
#if defined(__acc_constructor)
#  define __acc_HAVE_constructor 1
#endif
#if !defined(__acc_destructor)
#if (ACC_CC_GNUC >= 0x030400ul)
#  define __acc_destructor      __attribute__((__destructor__,__used__))
#elif (ACC_CC_GNUC >= 0x020700ul)
#  define __acc_destructor      __attribute__((__destructor__))
#elif (ACC_CC_ARMCC_GNUC || ACC_CC_CLANG || ACC_CC_LLVM || ACC_CC_PATHSCALE)
#  define __acc_destructor      __attribute__((__destructor__))
#endif
#endif
#if defined(__acc_destructor)
#  define __acc_HAVE_destructor 1
#endif
#if (__acc_HAVE_destructor) && !(__acc_HAVE_constructor)
#  error "unexpected configuration - check your compiler defines"
#endif
#if !defined(__acc_inline)
#if (ACC_CC_TURBOC && (__TURBOC__ <= 0x0295))
#elif defined(__cplusplus)
#  define __acc_inline          inline
#elif (ACC_CC_BORLANDC && (__BORLANDC__ >= 0x0550))
#  define __acc_inline          __inline
#elif (ACC_CC_ARMCC_GNUC || ACC_CC_CILLY || ACC_CC_CLANG || ACC_CC_GNUC || ACC_CC_LLVM || ACC_CC_PATHSCALE || ACC_CC_PGI)
#  define __acc_inline          __inline__
#elif (ACC_CC_DMC)
#  define __acc_inline          __inline
#elif (ACC_CC_INTELC)
#  define __acc_inline          __inline
#elif (ACC_CC_MWERKS && (__MWERKS__ >= 0x2405))
#  define __acc_inline          __inline
#elif (ACC_CC_MSC && (_MSC_VER >= 900))
#  define __acc_inline          __inline
#elif (ACC_CC_SUNPROC && (ACC_CC_SUNPROC >= 0x5100))
#  define __acc_inline          __inline__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#  define __acc_inline          inline
#endif
#endif
#if defined(__acc_inline)
#  define __acc_HAVE_inline 1
#else
#  define __acc_inline          /*empty*/
#endif
#if !defined(__acc_forceinline)
#if (ACC_CC_GNUC >= 0x030200ul)
#  define __acc_forceinline     __inline__ __attribute__((__always_inline__))
#elif (ACC_CC_INTELC_MSC && (__INTEL_COMPILER >= 450))
#  define __acc_forceinline     __forceinline
#elif (ACC_CC_INTELC_GNUC && (__INTEL_COMPILER >= 800))
#  define __acc_forceinline     __inline__ __attribute__((__always_inline__))
#elif (ACC_CC_ARMCC_GNUC || ACC_CC_CLANG || ACC_CC_LLVM || ACC_CC_PATHSCALE)
#  define __acc_forceinline     __inline__ __attribute__((__always_inline__))
#elif (ACC_CC_MSC && (_MSC_VER >= 1200))
#  define __acc_forceinline     __forceinline
#elif (ACC_CC_SUNPROC && (ACC_CC_SUNPROC >= 0x5100))
#  define __acc_forceinline     __inline__ __attribute__((__always_inline__))
#endif
#endif
#if defined(__acc_forceinline)
#  define __acc_HAVE_forceinline 1
#else
#  define __acc_forceinline     __acc_inline
#endif
#if !defined(__acc_noinline)
#if 1 && (ACC_ARCH_I386) && (ACC_CC_GNUC >= 0x040000ul) && (ACC_CC_GNUC < 0x040003ul)
#  define __acc_noinline        __attribute__((__noinline__,__used__))
#elif (ACC_CC_GNUC >= 0x030200ul)
#  define __acc_noinline        __attribute__((__noinline__))
#elif (ACC_CC_INTELC_MSC && (__INTEL_COMPILER >= 600))
#  define __acc_noinline        __declspec(noinline)
#elif (ACC_CC_INTELC_GNUC && (__INTEL_COMPILER >= 800))
#  define __acc_noinline        __attribute__((__noinline__))
#elif (ACC_CC_ARMCC_GNUC || ACC_CC_CLANG || ACC_CC_LLVM || ACC_CC_PATHSCALE)
#  define __acc_noinline        __attribute__((__noinline__))
#elif (ACC_CC_MSC && (_MSC_VER >= 1300))
#  define __acc_noinline        __declspec(noinline)
#elif (ACC_CC_MWERKS && (__MWERKS__ >= 0x3200) && (ACC_OS_WIN32 || ACC_OS_WIN64))
#  if defined(__cplusplus)
#  else
#    define __acc_noinline      __declspec(noinline)
#  endif
#elif (ACC_CC_SUNPROC && (ACC_CC_SUNPROC >= 0x5100))
#  define __acc_noinline        __attribute__((__noinline__))
#endif
#endif
#if defined(__acc_noinline)
#  define __acc_HAVE_noinline 1
#else
#  define __acc_noinline        /*empty*/
#endif
#if (__acc_HAVE_forceinline || __acc_HAVE_noinline) && !(__acc_HAVE_inline)
#  error "unexpected configuration - check your compiler defines"
#endif
#if !defined(__acc_c99_extern_inline)
#if defined(__GNUC_GNU_INLINE__)
#  define __acc_c99_extern_inline   __acc_inline
#elif defined(__GNUC_STDC_INLINE__)
#  define __acc_c99_extern_inline   extern __acc_inline
#elif ((__STDC_VERSION__ + 0) >= 199901L)
#  define __acc_c99_extern_inline   extern __acc_inline
#endif
#if !defined(__acc_c99_extern_inline) && (__acc_HAVE_inline)
#  define __acc_c99_extern_inline   __acc_inline
#endif
#endif
#if defined(__acc_c99_extern_inline)
#  define __acc_HAVE_c99_extern_inline 1
#else
#  define __acc_c99_extern_inline   /*empty*/
#endif
#if !defined(__acc_may_alias)
#if (ACC_CC_GNUC >= 0x030400ul)
#  define __acc_may_alias       __attribute__((__may_alias__))
#elif (ACC_CC_CLANG >= 0x020900ul)
#  define __acc_may_alias       __attribute__((__may_alias__))
#elif (ACC_CC_INTELC_GNUC && (__INTEL_COMPILER >= 1210))
#  define __acc_may_alias       __attribute__((__may_alias__))
#endif
#endif
#if defined(__acc_may_alias)
#  define __acc_HAVE_may_alias 1
#else
#  define __acc_may_alias       /*empty*/
#endif
#if !defined(__acc_noreturn)
#if (ACC_CC_GNUC >= 0x020700ul)
#  define __acc_noreturn        __attribute__((__noreturn__))
#elif (ACC_CC_INTELC_MSC && (__INTEL_COMPILER >= 450))
#  define __acc_noreturn        __declspec(noreturn)
#elif (ACC_CC_INTELC_GNUC && (__INTEL_COMPILER >= 600))
#  define __acc_noreturn        __attribute__((__noreturn__))
#elif (ACC_CC_ARMCC_GNUC || ACC_CC_CLANG || ACC_CC_LLVM || ACC_CC_PATHSCALE)
#  define __acc_noreturn        __attribute__((__noreturn__))
#elif (ACC_CC_MSC && (_MSC_VER >= 1200))
#  define __acc_noreturn        __declspec(noreturn)
#endif
#endif
#if defined(__acc_noreturn)
#  define __acc_HAVE_noreturn 1
#else
#  define __acc_noreturn        /*empty*/
#endif
#if !defined(__acc_nothrow)
#if (ACC_CC_GNUC >= 0x030300ul)
#  define __acc_nothrow         __attribute__((__nothrow__))
#elif (ACC_CC_INTELC_MSC && (__INTEL_COMPILER >= 450)) && defined(__cplusplus)
#  define __acc_nothrow         __declspec(nothrow)
#elif (ACC_CC_INTELC_GNUC && (__INTEL_COMPILER >= 900))
#  define __acc_nothrow         __attribute__((__nothrow__))
#elif (ACC_CC_ARMCC_GNUC || ACC_CC_CLANG || ACC_CC_LLVM || ACC_CC_PATHSCALE)
#  define __acc_nothrow         __attribute__((__nothrow__))
#elif (ACC_CC_MSC && (_MSC_VER >= 1200)) && defined(__cplusplus)
#  define __acc_nothrow         __declspec(nothrow)
#endif
#endif
#if defined(__acc_nothrow)
#  define __acc_HAVE_nothrow 1
#else
#  define __acc_nothrow         /*empty*/
#endif
#if !defined(__acc_restrict)
#if (ACC_CC_GNUC >= 0x030400ul)
#  define __acc_restrict        __restrict__
#elif (ACC_CC_INTELC_GNUC && (__INTEL_COMPILER >= 600))
#  define __acc_restrict        __restrict__
#elif (ACC_CC_ARMCC_GNUC || ACC_CC_CLANG || ACC_CC_LLVM)
#  define __acc_restrict        __restrict__
#elif (ACC_CC_MSC && (_MSC_VER >= 1400))
#  define __acc_restrict        __restrict
#endif
#endif
#if defined(__acc_restrict)
#  define __acc_HAVE_restrict 1
#else
#  define __acc_restrict        /*empty*/
#endif
#if !defined(__acc_likely) && !defined(__acc_unlikely)
#if (ACC_CC_GNUC >= 0x030200ul)
#  define __acc_likely(e)       (__builtin_expect(!!(e),1))
#  define __acc_unlikely(e)     (__builtin_expect(!!(e),0))
#elif (ACC_CC_INTELC && (__INTEL_COMPILER >= 800))
#  define __acc_likely(e)       (__builtin_expect(!!(e),1))
#  define __acc_unlikely(e)     (__builtin_expect(!!(e),0))
#elif (ACC_CC_ARMCC_GNUC || ACC_CC_CLANG || ACC_CC_LLVM || ACC_CC_PATHSCALE)
#  define __acc_likely(e)       (__builtin_expect(!!(e),1))
#  define __acc_unlikely(e)     (__builtin_expect(!!(e),0))
#endif
#endif
#if defined(__acc_likely)
#  define __acc_HAVE_likely 1
#else
#  define __acc_likely(e)       (e)
#endif
#if defined(__acc_unlikely)
#  define __acc_HAVE_unlikely 1
#else
#  define __acc_unlikely(e)     (e)
#endif
#if !defined(ACC_UNUSED)
#  if (ACC_CC_BORLANDC && (__BORLANDC__ >= 0x0600))
#    define ACC_UNUSED(var)         ((void) &var)
#  elif (ACC_CC_BORLANDC || ACC_CC_HIGHC || ACC_CC_NDPC || ACC_CC_PELLESC || ACC_CC_TURBOC)
#    define ACC_UNUSED(var)         if (&var) ; else
#  elif (ACC_CC_CLANG && (ACC_CC_CLANG >= 0x030200ul))
#    define ACC_UNUSED(var)         ((void) &var)
#  elif (ACC_CC_ARMCC_GNUC || ACC_CC_CLANG || ACC_CC_GNUC || ACC_CC_LLVM || ACC_CC_PATHSCALE)
#    define ACC_UNUSED(var)         ((void) var)
#  elif (ACC_CC_MSC && (_MSC_VER < 900))
#    define ACC_UNUSED(var)         if (&var) ; else
#  elif (ACC_CC_KEILC)
#    define ACC_UNUSED(var)         {ACC_EXTERN_C int acc_unused__[1-2*!(sizeof(var)>0)];}
#  elif (ACC_CC_PACIFICC)
#    define ACC_UNUSED(var)         ((void) sizeof(var))
#  elif (ACC_CC_WATCOMC) && defined(__cplusplus)
#    define ACC_UNUSED(var)         ((void) var)
#  else
#    define ACC_UNUSED(var)         ((void) &var)
#  endif
#endif
#if !defined(ACC_UNUSED_FUNC)
#  if (ACC_CC_BORLANDC && (__BORLANDC__ >= 0x0600))
#    define ACC_UNUSED_FUNC(func)   ((void) func)
#  elif (ACC_CC_BORLANDC || ACC_CC_NDPC || ACC_CC_TURBOC)
#    define ACC_UNUSED_FUNC(func)   if (func) ; else
#  elif (ACC_CC_ARMCC_GNUC || ACC_CC_CLANG || ACC_CC_LLVM)
#    define ACC_UNUSED_FUNC(func)   ((void) &func)
#  elif (ACC_CC_MSC && (_MSC_VER < 900))
#    define ACC_UNUSED_FUNC(func)   if (func) ; else
#  elif (ACC_CC_MSC)
#    define ACC_UNUSED_FUNC(func)   ((void) &func)
#  elif (ACC_CC_KEILC || ACC_CC_PELLESC)
#    define ACC_UNUSED_FUNC(func)   {ACC_EXTERN_C int acc_unused_func__[1-2*!(sizeof((int)func)>0)];}
#  else
#    define ACC_UNUSED_FUNC(func)   ((void) func)
#  endif
#endif
#if !defined(ACC_UNUSED_LABEL)
#  if (ACC_CC_WATCOMC) && defined(__cplusplus)
#    define ACC_UNUSED_LABEL(l)     switch(0) case 1:goto l
#  elif (ACC_CC_ARMCC || ACC_CC_CLANG || ACC_CC_INTELC || ACC_CC_WATCOMC)
#    define ACC_UNUSED_LABEL(l)     if (0) goto l
#  else
#    define ACC_UNUSED_LABEL(l)     switch(0) case 1:goto l
#  endif
#endif
#if !defined(ACC_DEFINE_UNINITIALIZED_VAR)
#  if 0
#    define ACC_DEFINE_UNINITIALIZED_VAR(type,var,init)  type var
#  elif 0 && (ACC_CC_GNUC)
#    define ACC_DEFINE_UNINITIALIZED_VAR(type,var,init)  type var = var
#  else
#    define ACC_DEFINE_UNINITIALIZED_VAR(type,var,init)  type var = init
#  endif
#endif
#if !defined(ACC_UNCONST_CAST)
#  if 0 && defined(__cplusplus)
#    define ACC_UNCONST_CAST(t,e)   (const_cast<t> (e))
#  elif (ACC_CC_ARMCC_GNUC || ACC_CC_CLANG || ACC_CC_GNUC || ACC_CC_LLVM || ACC_CC_PATHSCALE)
#    define ACC_UNCONST_CAST(t,e)           ((t) ((void *) ((char *) ((acc_uintptr_t) ((const void *) (e))))))
#  else
#    define ACC_UNCONST_CAST(t,e)           ((t) ((void *) ((char *) ((const void *) (e)))))
#  endif
#endif
#if !defined(ACC_UNCONST_VOLATILE_CAST)
#  if (ACC_CC_ARMCC_GNUC || ACC_CC_CLANG || ACC_CC_GNUC || ACC_CC_LLVM || ACC_CC_PATHSCALE)
#    define ACC_UNCONST_VOLATILE_CAST(t,e)  ((t) ((volatile void *) ((volatile char *) ((acc_uintptr_t) ((volatile const void *) (e))))))
#  else
#    define ACC_UNCONST_VOLATILE_CAST(t,e)  ((t) ((volatile void *) ((volatile char *) ((volatile const void *) (e)))))
#  endif
#endif
#if !defined(ACC_UNVOLATILE_CAST)
#  if (ACC_CC_ARMCC_GNUC || ACC_CC_CLANG || ACC_CC_GNUC || ACC_CC_LLVM || ACC_CC_PATHSCALE)
#    define ACC_UNVOLATILE_CAST(t,e)        ((t) ((void *) ((char *) ((acc_uintptr_t) ((volatile void *) (e))))))
#  else
#    define ACC_UNVOLATILE_CAST(t,e)        ((t) ((void *) ((char *) ((volatile void *) (e)))))
#  endif
#endif
#if !defined(ACC_UNVOLATILE_CONST_CAST)
#  if (ACC_CC_ARMCC_GNUC || ACC_CC_CLANG || ACC_CC_GNUC || ACC_CC_LLVM || ACC_CC_PATHSCALE)
#    define ACC_UNVOLATILE_CONST_CAST(t,e)  ((t) ((const void *) ((const char *) ((acc_uintptr_t) ((volatile const void *) (e))))))
#  else
#    define ACC_UNVOLATILE_CONST_CAST(t,e)  ((t) ((const void *) ((const char *) ((volatile const void *) (e)))))
#  endif
#endif
#if !defined(ACC_BLOCK_BEGIN)
#  if (ACC_CC_MSC)
#    define ACC_BLOCK_BEGIN         do {
#    define ACC_BLOCK_END           } while (0,0);
#  else
#    define ACC_BLOCK_BEGIN         do {
#    define ACC_BLOCK_END           } while (0);
#  endif
#endif
#ifndef __ACC_CTA_NAME
#if (ACC_CFG_USE_COUNTER)
#  define __ACC_CTA_NAME(a)         ACC_PP_ECONCAT2(a,__COUNTER__)
#else
#  define __ACC_CTA_NAME(a)         ACC_PP_ECONCAT2(a,__LINE__)
#endif
#endif
#if !defined(ACC_COMPILE_TIME_ASSERT_HEADER)
#  if (ACC_CC_AZTECC || ACC_CC_ZORTECHC)
#    define ACC_COMPILE_TIME_ASSERT_HEADER(e)  ACC_EXTERN_C_BEGIN extern int __ACC_CTA_NAME(acc_cta__)[1-!(e)]; ACC_EXTERN_C_END
#  elif (ACC_CC_DMC || ACC_CC_SYMANTECC)
#    define ACC_COMPILE_TIME_ASSERT_HEADER(e)  ACC_EXTERN_C_BEGIN extern int __ACC_CTA_NAME(acc_cta__)[1u-2*!(e)]; ACC_EXTERN_C_END
#  elif (ACC_CC_TURBOC && (__TURBOC__ == 0x0295))
#    define ACC_COMPILE_TIME_ASSERT_HEADER(e)  ACC_EXTERN_C_BEGIN extern int __ACC_CTA_NAME(acc_cta__)[1-!(e)]; ACC_EXTERN_C_END
#  elif (ACC_CC_CLANG && (ACC_CC_CLANG < 0x020900ul)) && defined(__cplusplus)
#    define ACC_COMPILE_TIME_ASSERT_HEADER(e)  ACC_EXTERN_C_BEGIN int __ACC_CTA_NAME(acc_cta_f__)(int [1-2*!(e)]); ACC_EXTERN_C_END
#  else
#    define ACC_COMPILE_TIME_ASSERT_HEADER(e)  ACC_EXTERN_C_BEGIN extern int __ACC_CTA_NAME(acc_cta__)[1-2*!(e)]; ACC_EXTERN_C_END
#  endif
#endif
#if !defined(ACC_COMPILE_TIME_ASSERT)
#  if (ACC_CC_AZTECC)
#    define ACC_COMPILE_TIME_ASSERT(e)  {typedef int __ACC_CTA_NAME(acc_cta_t__)[1-!(e)];}
#  elif (ACC_CC_DMC || ACC_CC_PACIFICC || ACC_CC_SYMANTECC || ACC_CC_ZORTECHC)
#    define ACC_COMPILE_TIME_ASSERT(e)  switch(0) case 1:case !(e):break;
#  elif (ACC_CC_GNUC >= 0x040700ul) && defined(__cplusplus)
#    define ACC_COMPILE_TIME_ASSERT(e)  {enum {__ACC_CTA_NAME(acc_cta_e__)=1/!!(e)} __attribute__((__unused__));}
#  elif (ACC_CC_GNUC >= 0x040700ul)
#    define ACC_COMPILE_TIME_ASSERT(e)  {typedef int __ACC_CTA_NAME(acc_cta_t__)[1-2*!(e)] __attribute__((__unused__));}
#  elif (ACC_CC_MSC && (_MSC_VER < 900))
#    define ACC_COMPILE_TIME_ASSERT(e)  switch(0) case 1:case !(e):break;
#  elif (ACC_CC_TURBOC && (__TURBOC__ == 0x0295))
#    define ACC_COMPILE_TIME_ASSERT(e)  switch(0) case 1:case !(e):break;
#  else
#    define ACC_COMPILE_TIME_ASSERT(e)  {typedef int __ACC_CTA_NAME(acc_cta_t__)[1-2*!(e)];}
#  endif
#endif
ACC_COMPILE_TIME_ASSERT_HEADER(1 == 1)
#if defined(__cplusplus)
extern "C" { ACC_COMPILE_TIME_ASSERT_HEADER(2 == 2) }
#endif
ACC_COMPILE_TIME_ASSERT_HEADER(3 == 3)
#if (ACC_ARCH_I086 || ACC_ARCH_I386) && (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)
#  if (ACC_CC_GNUC || ACC_CC_HIGHC || ACC_CC_NDPC || ACC_CC_PACIFICC)
#  elif (ACC_CC_DMC || ACC_CC_SYMANTECC || ACC_CC_ZORTECHC)
#    define __acc_cdecl                 __cdecl
#    define __acc_cdecl_atexit          /*empty*/
#    define __acc_cdecl_main            __cdecl
#    if (ACC_OS_OS2 && (ACC_CC_DMC || ACC_CC_SYMANTECC))
#      define __acc_cdecl_qsort         __pascal
#    elif (ACC_OS_OS2 && (ACC_CC_ZORTECHC))
#      define __acc_cdecl_qsort         _stdcall
#    else
#      define __acc_cdecl_qsort         __cdecl
#    endif
#  elif (ACC_CC_WATCOMC)
#    define __acc_cdecl                 __cdecl
#  else
#    define __acc_cdecl                 __cdecl
#    define __acc_cdecl_atexit          __cdecl
#    define __acc_cdecl_main            __cdecl
#    define __acc_cdecl_qsort           __cdecl
#  endif
#  if (ACC_CC_GNUC || ACC_CC_HIGHC || ACC_CC_NDPC || ACC_CC_PACIFICC || ACC_CC_WATCOMC)
#  elif (ACC_OS_OS2 && (ACC_CC_DMC || ACC_CC_SYMANTECC))
#    define __acc_cdecl_sighandler      __pascal
#  elif (ACC_OS_OS2 && (ACC_CC_ZORTECHC))
#    define __acc_cdecl_sighandler      _stdcall
#  elif (ACC_CC_MSC && (_MSC_VER >= 1400)) && defined(_M_CEE_PURE)
#    define __acc_cdecl_sighandler      __clrcall
#  elif (ACC_CC_MSC && (_MSC_VER >= 600 && _MSC_VER < 700))
#    if defined(_DLL)
#      define __acc_cdecl_sighandler    _far _cdecl _loadds
#    elif defined(_MT)
#      define __acc_cdecl_sighandler    _far _cdecl
#    else
#      define __acc_cdecl_sighandler    _cdecl
#    endif
#  else
#    define __acc_cdecl_sighandler      __cdecl
#  endif
#elif (ACC_ARCH_I386) && (ACC_CC_WATCOMC)
#  define __acc_cdecl                   __cdecl
#elif (ACC_ARCH_M68K && ACC_OS_TOS && (ACC_CC_PUREC || ACC_CC_TURBOC))
#  define __acc_cdecl                   cdecl
#endif
#if !defined(__acc_cdecl)
#  define __acc_cdecl                   /*empty*/
#endif
#if !defined(__acc_cdecl_atexit)
#  define __acc_cdecl_atexit            /*empty*/
#endif
#if !defined(__acc_cdecl_main)
#  define __acc_cdecl_main              /*empty*/
#endif
#if !defined(__acc_cdecl_qsort)
#  define __acc_cdecl_qsort             /*empty*/
#endif
#if !defined(__acc_cdecl_sighandler)
#  define __acc_cdecl_sighandler        /*empty*/
#endif
#if !defined(__acc_cdecl_va)
#  define __acc_cdecl_va                __acc_cdecl
#endif
#if !(ACC_CFG_NO_WINDOWS_H)
#if !defined(ACC_HAVE_WINDOWS_H)
#if (ACC_OS_CYGWIN || (ACC_OS_EMX && defined(__RSXNT__)) || ACC_OS_WIN32 || ACC_OS_WIN64)
#  if (ACC_CC_WATCOMC && (__WATCOMC__ < 1000))
#  elif (ACC_OS_WIN32 && ACC_CC_GNUC) && defined(__PW32__)
#  elif ((ACC_OS_CYGWIN || defined(__MINGW32__)) && (ACC_CC_GNUC && (ACC_CC_GNUC < 0x025f00ul)))
#  else
#    define ACC_HAVE_WINDOWS_H 1
#  endif
#endif
#endif
#endif
#ifndef ACC_SIZEOF_SHORT
#if defined(SIZEOF_SHORT)
#  define ACC_SIZEOF_SHORT          (SIZEOF_SHORT)
#elif defined(__SIZEOF_SHORT__)
#  define ACC_SIZEOF_SHORT          (__SIZEOF_SHORT__)
#endif
#endif
#ifndef ACC_SIZEOF_INT
#if defined(SIZEOF_INT)
#  define ACC_SIZEOF_INT            (SIZEOF_INT)
#elif defined(__SIZEOF_INT__)
#  define ACC_SIZEOF_INT            (__SIZEOF_INT__)
#endif
#endif
#ifndef ACC_SIZEOF_LONG
#if defined(SIZEOF_LONG)
#  define ACC_SIZEOF_LONG           (SIZEOF_LONG)
#elif defined(__SIZEOF_LONG__)
#  define ACC_SIZEOF_LONG           (__SIZEOF_LONG__)
#endif
#endif
#ifndef ACC_SIZEOF_LONG_LONG
#if defined(SIZEOF_LONG_LONG)
#  define ACC_SIZEOF_LONG_LONG      (SIZEOF_LONG_LONG)
#elif defined(__SIZEOF_LONG_LONG__)
#  define ACC_SIZEOF_LONG_LONG      (__SIZEOF_LONG_LONG__)
#endif
#endif
#ifndef ACC_SIZEOF___INT16
#if defined(SIZEOF___INT16)
#  define ACC_SIZEOF___INT16        (SIZEOF___INT16)
#endif
#endif
#ifndef ACC_SIZEOF___INT32
#if defined(SIZEOF___INT32)
#  define ACC_SIZEOF___INT32        (SIZEOF___INT32)
#endif
#endif
#ifndef ACC_SIZEOF___INT64
#if defined(SIZEOF___INT64)
#  define ACC_SIZEOF___INT64        (SIZEOF___INT64)
#endif
#endif
#ifndef ACC_SIZEOF_VOID_P
#if defined(SIZEOF_VOID_P)
#  define ACC_SIZEOF_VOID_P         (SIZEOF_VOID_P)
#elif defined(__SIZEOF_POINTER__)
#  define ACC_SIZEOF_VOID_P         (__SIZEOF_POINTER__)
#endif
#endif
#ifndef ACC_SIZEOF_SIZE_T
#if defined(SIZEOF_SIZE_T)
#  define ACC_SIZEOF_SIZE_T         (SIZEOF_SIZE_T)
#elif defined(__SIZEOF_SIZE_T__)
#  define ACC_SIZEOF_SIZE_T         (__SIZEOF_SIZE_T__)
#endif
#endif
#ifndef ACC_SIZEOF_PTRDIFF_T
#if defined(SIZEOF_PTRDIFF_T)
#  define ACC_SIZEOF_PTRDIFF_T      (SIZEOF_PTRDIFF_T)
#elif defined(__SIZEOF_PTRDIFF_T__)
#  define ACC_SIZEOF_PTRDIFF_T      (__SIZEOF_PTRDIFF_T__)
#endif
#endif
#define __ACC_LSR(x,b)    (((x)+0ul) >> (b))
#if !defined(ACC_SIZEOF_SHORT)
#  if (ACC_ARCH_CRAY_PVP)
#    define ACC_SIZEOF_SHORT        8
#  elif (USHRT_MAX == ACC_0xffffL)
#    define ACC_SIZEOF_SHORT        2
#  elif (__ACC_LSR(USHRT_MAX,7) == 1)
#    define ACC_SIZEOF_SHORT        1
#  elif (__ACC_LSR(USHRT_MAX,15) == 1)
#    define ACC_SIZEOF_SHORT        2
#  elif (__ACC_LSR(USHRT_MAX,31) == 1)
#    define ACC_SIZEOF_SHORT        4
#  elif (__ACC_LSR(USHRT_MAX,63) == 1)
#    define ACC_SIZEOF_SHORT        8
#  elif (__ACC_LSR(USHRT_MAX,127) == 1)
#    define ACC_SIZEOF_SHORT        16
#  else
#    error "ACC_SIZEOF_SHORT"
#  endif
#endif
ACC_COMPILE_TIME_ASSERT_HEADER(ACC_SIZEOF_SHORT == sizeof(short))
#if !defined(ACC_SIZEOF_INT)
#  if (ACC_ARCH_CRAY_PVP)
#    define ACC_SIZEOF_INT          8
#  elif (UINT_MAX == ACC_0xffffL)
#    define ACC_SIZEOF_INT          2
#  elif (UINT_MAX == ACC_0xffffffffL)
#    define ACC_SIZEOF_INT          4
#  elif (__ACC_LSR(UINT_MAX,7) == 1)
#    define ACC_SIZEOF_INT          1
#  elif (__ACC_LSR(UINT_MAX,15) == 1)
#    define ACC_SIZEOF_INT          2
#  elif (__ACC_LSR(UINT_MAX,31) == 1)
#    define ACC_SIZEOF_INT          4
#  elif (__ACC_LSR(UINT_MAX,63) == 1)
#    define ACC_SIZEOF_INT          8
#  elif (__ACC_LSR(UINT_MAX,127) == 1)
#    define ACC_SIZEOF_INT          16
#  else
#    error "ACC_SIZEOF_INT"
#  endif
#endif
ACC_COMPILE_TIME_ASSERT_HEADER(ACC_SIZEOF_INT == sizeof(int))
#if !defined(ACC_SIZEOF_LONG)
#  if (ULONG_MAX == ACC_0xffffffffL)
#    define ACC_SIZEOF_LONG         4
#  elif (__ACC_LSR(ULONG_MAX,7) == 1)
#    define ACC_SIZEOF_LONG         1
#  elif (__ACC_LSR(ULONG_MAX,15) == 1)
#    define ACC_SIZEOF_LONG         2
#  elif (__ACC_LSR(ULONG_MAX,31) == 1)
#    define ACC_SIZEOF_LONG         4
#  elif (__ACC_LSR(ULONG_MAX,39) == 1)
#    define ACC_SIZEOF_LONG         5
#  elif (__ACC_LSR(ULONG_MAX,63) == 1)
#    define ACC_SIZEOF_LONG         8
#  elif (__ACC_LSR(ULONG_MAX,127) == 1)
#    define ACC_SIZEOF_LONG         16
#  else
#    error "ACC_SIZEOF_LONG"
#  endif
#endif
ACC_COMPILE_TIME_ASSERT_HEADER(ACC_SIZEOF_LONG == sizeof(long))
#if !defined(ACC_SIZEOF_LONG_LONG) && !defined(ACC_SIZEOF___INT64)
#if (ACC_SIZEOF_LONG > 0 && ACC_SIZEOF_LONG < 8)
#  if defined(__LONG_MAX__) && defined(__LONG_LONG_MAX__)
#    if (ACC_CC_GNUC >= 0x030300ul)
#      if ((__LONG_MAX__-0) == (__LONG_LONG_MAX__-0))
#        define ACC_SIZEOF_LONG_LONG      ACC_SIZEOF_LONG
#      elif (__ACC_LSR(__LONG_LONG_MAX__,30) == 1)
#        define ACC_SIZEOF_LONG_LONG      4
#      endif
#    endif
#  endif
#endif
#endif
#if !defined(ACC_SIZEOF_LONG_LONG) && !defined(ACC_SIZEOF___INT64)
#if (ACC_SIZEOF_LONG > 0 && ACC_SIZEOF_LONG < 8)
#if (ACC_ARCH_I086 && ACC_CC_DMC)
#elif (ACC_CC_CILLY) && defined(__GNUC__)
#  define ACC_SIZEOF_LONG_LONG      8
#elif (ACC_CC_ARMCC_GNUC || ACC_CC_CLANG || ACC_CC_GNUC || ACC_CC_LLVM || ACC_CC_PATHSCALE)
#  define ACC_SIZEOF_LONG_LONG      8
#elif ((ACC_OS_WIN32 || ACC_OS_WIN64 || defined(_WIN32)) && ACC_CC_MSC && (_MSC_VER >= 1400))
#  define ACC_SIZEOF_LONG_LONG      8
#elif (ACC_OS_WIN64 || defined(_WIN64))
#  define ACC_SIZEOF___INT64        8
#elif (ACC_ARCH_I386 && (ACC_CC_DMC))
#  define ACC_SIZEOF_LONG_LONG      8
#elif (ACC_ARCH_I386 && (ACC_CC_SYMANTECC && (__SC__ >= 0x700)))
#  define ACC_SIZEOF_LONG_LONG      8
#elif (ACC_ARCH_I386 && (ACC_CC_INTELC && defined(__linux__)))
#  define ACC_SIZEOF_LONG_LONG      8
#elif (ACC_ARCH_I386 && (ACC_CC_MWERKS || ACC_CC_PELLESC || ACC_CC_PGI || ACC_CC_SUNPROC))
#  define ACC_SIZEOF_LONG_LONG      8
#elif (ACC_ARCH_I386 && (ACC_CC_INTELC || ACC_CC_MSC))
#  define ACC_SIZEOF___INT64        8
#elif ((ACC_OS_WIN32 || defined(_WIN32)) && (ACC_CC_MSC))
#  define ACC_SIZEOF___INT64        8
#elif (ACC_ARCH_I386 && (ACC_CC_BORLANDC && (__BORLANDC__ >= 0x0520)))
#  define ACC_SIZEOF___INT64        8
#elif (ACC_ARCH_I386 && (ACC_CC_WATCOMC && (__WATCOMC__ >= 1100)))
#  define ACC_SIZEOF___INT64        8
#elif (ACC_CC_WATCOMC && defined(_INTEGRAL_MAX_BITS) && ((_INTEGRAL_MAX_BITS-0) == 64))
#  define ACC_SIZEOF___INT64        8
#elif (ACC_OS_OS400 || defined(__OS400__)) && defined(__LLP64_IFC__)
#  define ACC_SIZEOF_LONG_LONG      8
#elif (defined(__vms) || defined(__VMS)) && ((__INITIAL_POINTER_SIZE-0) == 64)
#  define ACC_SIZEOF_LONG_LONG      8
#elif (ACC_CC_SDCC) && (ACC_SIZEOF_INT == 2)
#elif 1 && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#  define ACC_SIZEOF_LONG_LONG      8
#endif
#endif
#endif
#if defined(__cplusplus) && (ACC_CC_GNUC)
#  if (ACC_CC_GNUC < 0x020800ul)
#    undef ACC_SIZEOF_LONG_LONG
#  endif
#endif
#if (ACC_CFG_NO_LONG_LONG) || defined(__NO_LONG_LONG)
#  undef ACC_SIZEOF_LONG_LONG
#endif
#if !defined(ACC_WORDSIZE)
#if (ACC_ARCH_ALPHA)
#  define ACC_WORDSIZE              8
#elif (ACC_ARCH_AMD64)
#  define ACC_WORDSIZE              8
#elif (ACC_ARCH_AVR)
#  define ACC_WORDSIZE              1
#elif (ACC_ARCH_H8300)
#  if defined(__NORMAL_MODE__)
#    define ACC_WORDSIZE            4
#  elif defined(__H8300H__) || defined(__H8300S__) || defined(__H8300SX__)
#    define ACC_WORDSIZE            4
#  else
#    define ACC_WORDSIZE            2
#  endif
#elif (ACC_ARCH_I086)
#  define ACC_WORDSIZE              2
#elif (ACC_ARCH_IA64)
#  define ACC_WORDSIZE              8
#elif (ACC_ARCH_M16C)
#  define ACC_WORDSIZE              2
#elif (ACC_ARCH_SPU)
#  define ACC_WORDSIZE              4
#elif (ACC_ARCH_Z80)
#  define ACC_WORDSIZE              1
#elif (ACC_SIZEOF_LONG == 8) && ((defined(__mips__) && defined(__R5900__)) || defined(__MIPS_PSX2__))
#  define ACC_WORDSIZE              8
#elif (ACC_OS_OS400 || defined(__OS400__))
#  define ACC_WORDSIZE              8
#elif (defined(__vms) || defined(__VMS)) && (__INITIAL_POINTER_SIZE+0 == 64)
#  define ACC_WORDSIZE              8
#endif
#endif
#if !defined(ACC_SIZEOF_VOID_P)
#if defined(__ILP32__) || defined(__ILP32) || defined(_ILP32)
#  define ACC_SIZEOF_VOID_P         4
#elif defined(__ILP64__) || defined(__ILP64) || defined(_ILP64)
#  define ACC_SIZEOF_VOID_P         8
#elif defined(__LLP64__) || defined(__LLP64) || defined(_LLP64) || defined(_WIN64)
#  define ACC_SIZEOF_VOID_P         8
#elif defined(__LP64__) || defined(__LP64) || defined(_LP64)
#  define ACC_SIZEOF_VOID_P         8
#elif (ACC_ARCH_AVR)
#  define ACC_SIZEOF_VOID_P         2
#elif (ACC_ARCH_C166 || ACC_ARCH_MCS51 || ACC_ARCH_MCS251 || ACC_ARCH_MSP430)
#  define ACC_SIZEOF_VOID_P         2
#elif (ACC_ARCH_H8300)
#  if defined(__NORMAL_MODE__)
#    define ACC_SIZEOF_VOID_P       2
#  elif defined(__H8300H__) || defined(__H8300S__) || defined(__H8300SX__)
#    define ACC_SIZEOF_VOID_P       4
#  else
#    define ACC_SIZEOF_VOID_P       2
#  endif
#  if (ACC_CC_GNUC && (ACC_CC_GNUC < 0x040000ul)) && (ACC_SIZEOF_INT == 4)
#    define ACC_SIZEOF_SIZE_T       ACC_SIZEOF_INT
#    define ACC_SIZEOF_PTRDIFF_T    ACC_SIZEOF_INT
#  endif
#elif (ACC_ARCH_I086)
#  if (ACC_MM_TINY || ACC_MM_SMALL || ACC_MM_MEDIUM)
#    define ACC_SIZEOF_VOID_P       2
#  elif (ACC_MM_COMPACT || ACC_MM_LARGE || ACC_MM_HUGE)
#    define ACC_SIZEOF_VOID_P       4
#  else
#    error "invalid ACC_ARCH_I086 memory model"
#  endif
#elif (ACC_ARCH_M16C)
#  if defined(__m32c_cpu__) || defined(__m32cm_cpu__)
#    define ACC_SIZEOF_VOID_P       4
#  else
#    define ACC_SIZEOF_VOID_P       2
#  endif
#elif (ACC_ARCH_SPU)
#  define ACC_SIZEOF_VOID_P         4
#elif (ACC_ARCH_Z80)
#  define ACC_SIZEOF_VOID_P         2
#elif (ACC_SIZEOF_LONG == 8) && ((defined(__mips__) && defined(__R5900__)) || defined(__MIPS_PSX2__))
#  define ACC_SIZEOF_VOID_P         4
#elif (ACC_OS_OS400 || defined(__OS400__))
#  if defined(__LLP64_IFC__)
#    define ACC_SIZEOF_VOID_P       8
#    define ACC_SIZEOF_SIZE_T       ACC_SIZEOF_LONG
#    define ACC_SIZEOF_PTRDIFF_T    ACC_SIZEOF_LONG
#  else
#    define ACC_SIZEOF_VOID_P       16
#    define ACC_SIZEOF_SIZE_T       ACC_SIZEOF_LONG
#    define ACC_SIZEOF_PTRDIFF_T    ACC_SIZEOF_LONG
#  endif
#elif (defined(__vms) || defined(__VMS)) && (__INITIAL_POINTER_SIZE+0 == 64)
#  define ACC_SIZEOF_VOID_P         8
#  define ACC_SIZEOF_SIZE_T         ACC_SIZEOF_LONG
#  define ACC_SIZEOF_PTRDIFF_T      ACC_SIZEOF_LONG
#endif
#endif
#if !defined(ACC_SIZEOF_VOID_P)
#  define ACC_SIZEOF_VOID_P         ACC_SIZEOF_LONG
#endif
ACC_COMPILE_TIME_ASSERT_HEADER(ACC_SIZEOF_VOID_P == sizeof(void *))
#if !defined(ACC_SIZEOF_SIZE_T)
#if (ACC_ARCH_I086 || ACC_ARCH_M16C)
#  define ACC_SIZEOF_SIZE_T         2
#endif
#endif
#if !defined(ACC_SIZEOF_SIZE_T)
#  define ACC_SIZEOF_SIZE_T         ACC_SIZEOF_VOID_P
#endif
#if defined(offsetof)
ACC_COMPILE_TIME_ASSERT_HEADER(ACC_SIZEOF_SIZE_T == sizeof(size_t))
#endif
#if !defined(ACC_SIZEOF_PTRDIFF_T)
#if (ACC_ARCH_I086)
#  if (ACC_MM_TINY || ACC_MM_SMALL || ACC_MM_MEDIUM || ACC_MM_HUGE)
#    define ACC_SIZEOF_PTRDIFF_T    ACC_SIZEOF_VOID_P
#  elif (ACC_MM_COMPACT || ACC_MM_LARGE)
#    if (ACC_CC_BORLANDC || ACC_CC_TURBOC)
#      define ACC_SIZEOF_PTRDIFF_T  4
#    else
#      define ACC_SIZEOF_PTRDIFF_T  2
#    endif
#  else
#    error "invalid ACC_ARCH_I086 memory model"
#  endif
#endif
#endif
#if !defined(ACC_SIZEOF_PTRDIFF_T)
#  define ACC_SIZEOF_PTRDIFF_T      ACC_SIZEOF_SIZE_T
#endif
#if defined(offsetof)
ACC_COMPILE_TIME_ASSERT_HEADER(ACC_SIZEOF_PTRDIFF_T == sizeof(ptrdiff_t))
#endif
#if !defined(ACC_WORDSIZE)
#  define ACC_WORDSIZE              ACC_SIZEOF_VOID_P
#endif
#if (ACC_ABI_NEUTRAL_ENDIAN)
#  undef ACC_ABI_BIG_ENDIAN
#  undef ACC_ABI_LITTLE_ENDIAN
#elif !(ACC_ABI_BIG_ENDIAN) && !(ACC_ABI_LITTLE_ENDIAN)
#if (ACC_ARCH_ALPHA) && (ACC_ARCH_CRAY_MPP)
#  define ACC_ABI_BIG_ENDIAN        1
#elif (ACC_ARCH_IA64) && (ACC_OS_POSIX_LINUX || ACC_OS_WIN64)
#  define ACC_ABI_LITTLE_ENDIAN     1
#elif (ACC_ARCH_ALPHA || ACC_ARCH_AMD64 || ACC_ARCH_BLACKFIN || ACC_ARCH_CRIS || ACC_ARCH_I086 || ACC_ARCH_I386 || ACC_ARCH_MSP430)
#  define ACC_ABI_LITTLE_ENDIAN     1
#elif (ACC_ARCH_AVR32 || ACC_ARCH_M68K || ACC_ARCH_S390 || ACC_ARCH_SPU)
#  define ACC_ABI_BIG_ENDIAN        1
#elif 1 && defined(__IAR_SYSTEMS_ICC__) && defined(__LITTLE_ENDIAN__)
#  if (__LITTLE_ENDIAN__ == 1)
#    define ACC_ABI_LITTLE_ENDIAN   1
#  else
#    define ACC_ABI_BIG_ENDIAN      1
#  endif
#elif 1 && defined(__BIG_ENDIAN__) && !defined(__LITTLE_ENDIAN__)
#  define ACC_ABI_BIG_ENDIAN        1
#elif 1 && defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
#  define ACC_ABI_LITTLE_ENDIAN     1
#elif 1 && (ACC_ARCH_ARM) && defined(__ARMEB__) && !defined(__ARMEL__)
#  define ACC_ABI_BIG_ENDIAN        1
#elif 1 && (ACC_ARCH_ARM) && defined(__ARMEL__) && !defined(__ARMEB__)
#  define ACC_ABI_LITTLE_ENDIAN     1
#elif 1 && (ACC_ARCH_ARM && ACC_CC_ARMCC_ARMCC)
#  if defined(__BIG_ENDIAN) && defined(__LITTLE_ENDIAN)
#    error "unexpected configuration - check your compiler defines"
#  elif defined(__BIG_ENDIAN)
#    define ACC_ABI_BIG_ENDIAN      1
#  else
#    define ACC_ABI_LITTLE_ENDIAN   1
#  endif
#  define ACC_ABI_LITTLE_ENDIAN     1
#elif 1 && (ACC_ARCH_ARM64) && defined(__AARCH64EB__) && !defined(__AARCH64EL__)
#  define ACC_ABI_BIG_ENDIAN        1
#elif 1 && (ACC_ARCH_ARM64) && defined(__AARCH64EL__) && !defined(__AARCH64EB__)
#  define ACC_ABI_LITTLE_ENDIAN     1
#elif 1 && (ACC_ARCH_MIPS) && defined(__MIPSEB__) && !defined(__MIPSEL__)
#  define ACC_ABI_BIG_ENDIAN        1
#elif 1 && (ACC_ARCH_MIPS) && defined(__MIPSEL__) && !defined(__MIPSEB__)
#  define ACC_ABI_LITTLE_ENDIAN     1
#endif
#endif
#if (ACC_ABI_BIG_ENDIAN) && (ACC_ABI_LITTLE_ENDIAN)
#  error "unexpected configuration - check your compiler defines"
#endif
#if (ACC_ABI_BIG_ENDIAN)
#  define ACC_INFO_ABI_ENDIAN       "be"
#elif (ACC_ABI_LITTLE_ENDIAN)
#  define ACC_INFO_ABI_ENDIAN       "le"
#elif (ACC_ABI_NEUTRAL_ENDIAN)
#  define ACC_INFO_ABI_ENDIAN       "neutral"
#endif
#if (ACC_SIZEOF_INT == 1 && ACC_SIZEOF_LONG == 2 && ACC_SIZEOF_VOID_P == 2)
#  define ACC_ABI_I8LP16         1
#  define ACC_INFO_ABI_PM       "i8lp16"
#elif (ACC_SIZEOF_INT == 2 && ACC_SIZEOF_LONG == 2 && ACC_SIZEOF_VOID_P == 2)
#  define ACC_ABI_ILP16         1
#  define ACC_INFO_ABI_PM       "ilp16"
#elif (ACC_SIZEOF_INT == 2 && ACC_SIZEOF_LONG == 4 && ACC_SIZEOF_VOID_P == 4)
#  define ACC_ABI_LP32          1
#  define ACC_INFO_ABI_PM       "lp32"
#elif (ACC_SIZEOF_INT == 4 && ACC_SIZEOF_LONG == 4 && ACC_SIZEOF_VOID_P == 4)
#  define ACC_ABI_ILP32         1
#  define ACC_INFO_ABI_PM       "ilp32"
#elif (ACC_SIZEOF_INT == 4 && ACC_SIZEOF_LONG == 4 && ACC_SIZEOF_VOID_P == 8 && ACC_SIZEOF_SIZE_T == 8)
#  define ACC_ABI_LLP64         1
#  define ACC_INFO_ABI_PM       "llp64"
#elif (ACC_SIZEOF_INT == 4 && ACC_SIZEOF_LONG == 8 && ACC_SIZEOF_VOID_P == 8)
#  define ACC_ABI_LP64          1
#  define ACC_INFO_ABI_PM       "lp64"
#elif (ACC_SIZEOF_INT == 8 && ACC_SIZEOF_LONG == 8 && ACC_SIZEOF_VOID_P == 8)
#  define ACC_ABI_ILP64         1
#  define ACC_INFO_ABI_PM       "ilp64"
#elif (ACC_SIZEOF_INT == 4 && ACC_SIZEOF_LONG == 8 && ACC_SIZEOF_VOID_P == 4)
#  define ACC_ABI_IP32L64       1
#  define ACC_INFO_ABI_PM       "ip32l64"
#endif
#if !defined(__ACC_LIBC_OVERRIDE)
#if (ACC_LIBC_NAKED)
#  define ACC_INFO_LIBC         "naked"
#elif (ACC_LIBC_FREESTANDING)
#  define ACC_INFO_LIBC         "freestanding"
#elif (ACC_LIBC_MOSTLY_FREESTANDING)
#  define ACC_INFO_LIBC         "mfreestanding"
#elif (ACC_LIBC_ISOC90)
#  define ACC_INFO_LIBC         "isoc90"
#elif (ACC_LIBC_ISOC99)
#  define ACC_INFO_LIBC         "isoc99"
#elif (ACC_CC_ARMCC_ARMCC) && defined(__ARMCLIB_VERSION)
#  define ACC_LIBC_ISOC90       1
#  define ACC_INFO_LIBC         "isoc90"
#elif defined(__dietlibc__)
#  define ACC_LIBC_DIETLIBC     1
#  define ACC_INFO_LIBC         "dietlibc"
#elif defined(_NEWLIB_VERSION)
#  define ACC_LIBC_NEWLIB       1
#  define ACC_INFO_LIBC         "newlib"
#elif defined(__UCLIBC__) && defined(__UCLIBC_MAJOR__) && defined(__UCLIBC_MINOR__)
#  if defined(__UCLIBC_SUBLEVEL__)
#    define ACC_LIBC_UCLIBC     (__UCLIBC_MAJOR__ * 0x10000L + (__UCLIBC_MINOR__-0) * 0x100 + (__UCLIBC_SUBLEVEL__-0))
#  else
#    define ACC_LIBC_UCLIBC     0x00090bL
#  endif
#  define ACC_INFO_LIBC         "uc" "libc"
#elif defined(__GLIBC__) && defined(__GLIBC_MINOR__)
#  define ACC_LIBC_GLIBC        (__GLIBC__ * 0x10000L + (__GLIBC_MINOR__-0) * 0x100)
#  define ACC_INFO_LIBC         "glibc"
#elif (ACC_CC_MWERKS) && defined(__MSL__)
#  define ACC_LIBC_MSL          __MSL__
#  define ACC_INFO_LIBC         "msl"
#elif 1 && defined(__IAR_SYSTEMS_ICC__)
#  define ACC_LIBC_ISOC90       1
#  define ACC_INFO_LIBC         "isoc90"
#else
#  define ACC_LIBC_DEFAULT      1
#  define ACC_INFO_LIBC         "default"
#endif
#endif
#if (ACC_ARCH_ALPHA)
#  define ACC_OPT_AVOID_UINT_INDEX  1
#elif (ACC_ARCH_AMD64)
#  define ACC_OPT_AVOID_INT_INDEX   1
#  define ACC_OPT_AVOID_UINT_INDEX  1
#  define ACC_OPT_UNALIGNED16       1
#  define ACC_OPT_UNALIGNED32       1
#  define ACC_OPT_UNALIGNED64       1
#elif (ACC_ARCH_ARM)
#  if defined(__ARM_FEATURE_UNALIGNED)
#    define ACC_OPT_UNALIGNED16     1
#    define ACC_OPT_UNALIGNED32     1
#  elif defined(__TARGET_ARCH_ARM) && ((__TARGET_ARCH_ARM+0) >= 7)
#    define ACC_OPT_UNALIGNED16     1
#    define ACC_OPT_UNALIGNED32     1
#  elif defined(__TARGET_ARCH_ARM) && ((__TARGET_ARCH_ARM+0) >= 6) && !defined(__TARGET_PROFILE_M)
#    define ACC_OPT_UNALIGNED16     1
#    define ACC_OPT_UNALIGNED32     1
#  endif
#elif (ACC_ARCH_ARM64)
#  define ACC_OPT_UNALIGNED16       1
#  define ACC_OPT_UNALIGNED32       1
#  define ACC_OPT_UNALIGNED64       1
#elif (ACC_ARCH_CRIS)
#  define ACC_OPT_UNALIGNED16       1
#  define ACC_OPT_UNALIGNED32       1
#elif (ACC_ARCH_I386)
#  define ACC_OPT_UNALIGNED16       1
#  define ACC_OPT_UNALIGNED32       1
#elif (ACC_ARCH_IA64)
#  define ACC_OPT_AVOID_INT_INDEX   1
#  define ACC_OPT_AVOID_UINT_INDEX  1
#  define ACC_OPT_PREFER_POSTINC    1
#elif (ACC_ARCH_M68K)
#  define ACC_OPT_PREFER_POSTINC    1
#  define ACC_OPT_PREFER_PREDEC     1
#  if defined(__mc68020__) && !defined(__mcoldfire__)
#    define ACC_OPT_UNALIGNED16     1
#    define ACC_OPT_UNALIGNED32     1
#  endif
#elif (ACC_ARCH_MIPS)
#  define ACC_OPT_AVOID_UINT_INDEX  1
#elif (ACC_ARCH_POWERPC)
#  define ACC_OPT_PREFER_PREINC     1
#  define ACC_OPT_PREFER_PREDEC     1
#  if (ACC_ABI_BIG_ENDIAN)
#    define ACC_OPT_UNALIGNED16     1
#    define ACC_OPT_UNALIGNED32     1
#    if (ACC_WORDSIZE == 8)
#      define ACC_OPT_UNALIGNED64   1
#    endif
#  endif
#elif (ACC_ARCH_S390)
#  define ACC_OPT_UNALIGNED16       1
#  define ACC_OPT_UNALIGNED32       1
#  if (ACC_WORDSIZE == 8)
#    define ACC_OPT_UNALIGNED64     1
#  endif
#elif (ACC_ARCH_SH)
#  define ACC_OPT_PREFER_POSTINC    1
#  define ACC_OPT_PREFER_PREDEC     1
#endif
#ifndef ACC_CFG_NO_INLINE_ASM
#if (ACC_ABI_NEUTRAL_ENDIAN) || (ACC_ARCH_GENERIC)
#  define ACC_CFG_NO_INLINE_ASM 1
#elif (ACC_CC_LLVM)
#  define ACC_CFG_NO_INLINE_ASM 1
#endif
#endif
#if (ACC_CFG_NO_INLINE_ASM)
#  undef ACC_ASM_SYNTAX_MSC
#  undef ACC_ASM_SYNTAX_GNUC
#  undef __ACC_ASM_CLOBBER
#endif
#ifndef ACC_CFG_NO_UNALIGNED
#if (ACC_ABI_NEUTRAL_ENDIAN) || (ACC_ARCH_GENERIC)
#  define ACC_CFG_NO_UNALIGNED 1
#endif
#endif
#if (ACC_CFG_NO_UNALIGNED)
#  undef ACC_OPT_UNALIGNED16
#  undef ACC_OPT_UNALIGNED32
#  undef ACC_OPT_UNALIGNED64
#endif
#if (ACC_ARCH_I386 && (ACC_OS_DOS32 || ACC_OS_WIN32) && (ACC_CC_DMC || ACC_CC_INTELC || ACC_CC_MSC || ACC_CC_PELLESC))
#  define ACC_ASM_SYNTAX_MSC 1
#elif (ACC_OS_WIN64 && (ACC_CC_DMC || ACC_CC_INTELC || ACC_CC_MSC || ACC_CC_PELLESC))
#elif (ACC_ARCH_I386 && ACC_CC_GNUC && (ACC_CC_GNUC == 0x011f00ul))
#elif (ACC_ARCH_I386 && (ACC_CC_CLANG || ACC_CC_GNUC || ACC_CC_INTELC || ACC_CC_PATHSCALE))
#  define ACC_ASM_SYNTAX_GNUC 1
#elif (ACC_ARCH_AMD64 && (ACC_CC_CLANG || ACC_CC_GNUC || ACC_CC_INTELC || ACC_CC_PATHSCALE))
#  define ACC_ASM_SYNTAX_GNUC 1
#elif (ACC_CC_GNUC)
#  define ACC_ASM_SYNTAX_GNUC 1
#endif
#if (ACC_ASM_SYNTAX_GNUC)
#if (ACC_ARCH_I386 && ACC_CC_GNUC && (ACC_CC_GNUC < 0x020000ul))
#  define __ACC_ASM_CLOBBER         "ax"
#elif (ACC_CC_INTELC)
#  define __ACC_ASM_CLOBBER         "memory"
#else
#  define __ACC_ASM_CLOBBER         "cc", "memory"
#endif
#endif
#if defined(__ACC_INFOSTR_MM)
#elif (ACC_MM_FLAT) && (defined(__ACC_INFOSTR_PM) || defined(ACC_INFO_ABI_PM))
#  define __ACC_INFOSTR_MM          ""
#elif defined(ACC_INFO_MM)
#  define __ACC_INFOSTR_MM          "." ACC_INFO_MM
#else
#  define __ACC_INFOSTR_MM          ""
#endif
#if defined(__ACC_INFOSTR_PM)
#elif defined(ACC_INFO_ABI_PM)
#  define __ACC_INFOSTR_PM          "." ACC_INFO_ABI_PM
#else
#  define __ACC_INFOSTR_PM          ""
#endif
#if defined(__ACC_INFOSTR_ENDIAN)
#elif defined(ACC_INFO_ABI_ENDIAN)
#  define __ACC_INFOSTR_ENDIAN      "." ACC_INFO_ABI_ENDIAN
#else
#  define __ACC_INFOSTR_ENDIAN      ""
#endif
#if defined(__ACC_INFOSTR_OSNAME)
#elif defined(ACC_INFO_OS_CONSOLE)
#  define __ACC_INFOSTR_OSNAME      ACC_INFO_OS "." ACC_INFO_OS_CONSOLE
#elif defined(ACC_INFO_OS_POSIX)
#  define __ACC_INFOSTR_OSNAME      ACC_INFO_OS "." ACC_INFO_OS_POSIX
#else
#  define __ACC_INFOSTR_OSNAME      ACC_INFO_OS
#endif
#if defined(__ACC_INFOSTR_LIBC)
#elif defined(ACC_INFO_LIBC)
#  define __ACC_INFOSTR_LIBC        "." ACC_INFO_LIBC
#else
#  define __ACC_INFOSTR_LIBC        ""
#endif
#if defined(__ACC_INFOSTR_CCVER)
#elif defined(ACC_INFO_CCVER)
#  define __ACC_INFOSTR_CCVER       " " ACC_INFO_CCVER
#else
#  define __ACC_INFOSTR_CCVER       ""
#endif
#define ACC_INFO_STRING \
    ACC_INFO_ARCH __ACC_INFOSTR_MM __ACC_INFOSTR_PM __ACC_INFOSTR_ENDIAN \
    " " __ACC_INFOSTR_OSNAME __ACC_INFOSTR_LIBC " " ACC_INFO_CC __ACC_INFOSTR_CCVER
#if !(ACC_CFG_SKIP_ACC_TYPES)
#if (!(ACC_SIZEOF_SHORT+0 > 0 && ACC_SIZEOF_INT+0 > 0 && ACC_SIZEOF_LONG+0 > 0))
#  error "missing defines for sizes"
#endif
#if (!(ACC_SIZEOF_PTRDIFF_T+0 > 0 && ACC_SIZEOF_SIZE_T+0 > 0 && ACC_SIZEOF_VOID_P+0 > 0))
#  error "missing defines for sizes"
#endif
#if !defined(acc_llong_t)
#if (ACC_SIZEOF_LONG_LONG+0 > 0)
__acc_gnuc_extension__ typedef long long acc_llong_t__;
__acc_gnuc_extension__ typedef unsigned long long acc_ullong_t__;
#  define acc_llong_t               acc_llong_t__
#  define acc_ullong_t              acc_ullong_t__
#endif
#endif
#if !defined(acc_int16e_t)
#if (ACC_SIZEOF_LONG == 2)
#  define acc_int16e_t              long
#  define acc_uint16e_t             unsigned long
#elif (ACC_SIZEOF_INT == 2)
#  define acc_int16e_t              int
#  define acc_uint16e_t             unsigned int
#elif (ACC_SIZEOF_SHORT == 2)
#  define acc_int16e_t              short int
#  define acc_uint16e_t             unsigned short int
#elif 1 && !(ACC_CFG_TYPE_NO_MODE_HI) && (ACC_CC_CLANG || (ACC_CC_GNUC >= 0x025f00ul) || ACC_CC_LLVM)
   typedef int acc_int16e_hi_t__ __attribute__((__mode__(__HI__)));
   typedef unsigned int acc_uint16e_hi_t__ __attribute__((__mode__(__HI__)));
#  define acc_int16e_t              acc_int16e_hi_t__
#  define acc_uint16e_t             acc_uint16e_hi_t__
#elif (ACC_SIZEOF___INT16 == 2)
#  define acc_int16e_t              __int16
#  define acc_uint16e_t             unsigned __int16
#else
#endif
#endif
#if defined(acc_int16e_t)
#  define ACC_SIZEOF_ACC_INT16E_T   2
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int16e_t) == 2)
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int16e_t) == ACC_SIZEOF_ACC_INT16E_T)
#endif
#if !defined(acc_int32e_t)
#if (ACC_SIZEOF_LONG == 4)
#  define acc_int32e_t              long int
#  define acc_uint32e_t             unsigned long int
#elif (ACC_SIZEOF_INT == 4)
#  define acc_int32e_t              int
#  define acc_uint32e_t             unsigned int
#elif (ACC_SIZEOF_SHORT == 4)
#  define acc_int32e_t              short int
#  define acc_uint32e_t             unsigned short int
#elif (ACC_SIZEOF_LONG_LONG == 4)
#  define acc_int32e_t              acc_llong_t
#  define acc_uint32e_t             acc_ullong_t
#elif 1 && !(ACC_CFG_TYPE_NO_MODE_SI) && (ACC_CC_CLANG || (ACC_CC_GNUC >= 0x025f00ul) || ACC_CC_LLVM) && (__INT_MAX__+0 > 2147483647L)
   typedef int acc_int32e_si_t__ __attribute__((__mode__(__SI__)));
   typedef unsigned int acc_uint32e_si_t__ __attribute__((__mode__(__SI__)));
#  define acc_int32e_t              acc_int32e_si_t__
#  define acc_uint32e_t             acc_uint32e_si_t__
#elif 1 && !(ACC_CFG_TYPE_NO_MODE_SI) && (ACC_CC_GNUC >= 0x025f00ul) && defined(__AVR__) && (__LONG_MAX__+0 == 32767L)
   typedef int acc_int32e_si_t__ __attribute__((__mode__(__SI__)));
   typedef unsigned int acc_uint32e_si_t__ __attribute__((__mode__(__SI__)));
#  define acc_int32e_t              acc_int32e_si_t__
#  define acc_uint32e_t             acc_uint32e_si_t__
#  define ACC_INT32_C(c)            c##LL
#  define ACC_UINT32_C(c)           c##ULL
#elif (ACC_SIZEOF___INT32 == 4)
#  define acc_int32e_t              __int32
#  define acc_uint32e_t             unsigned __int32
#else
#endif
#endif
#if defined(acc_int32e_t)
#  define ACC_SIZEOF_ACC_INT32E_T   4
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int32e_t) == 4)
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int32e_t) == ACC_SIZEOF_ACC_INT32E_T)
#endif
#if !defined(acc_int64e_t)
#if (ACC_SIZEOF___INT64 == 8)
#  if (ACC_CC_BORLANDC) && !(ACC_CFG_TYPE_PREFER___INT64)
#    define ACC_CFG_TYPE_PREFER___INT64 1
#  endif
#endif
#if (ACC_SIZEOF_INT == 8) && (ACC_SIZEOF_INT < ACC_SIZEOF_LONG)
#  define acc_int64e_t              int
#  define acc_uint64e_t             unsigned int
#  define ACC_SIZEOF_ACC_INT64E_T   ACC_SIZEOF_INT
#elif (ACC_SIZEOF_LONG == 8)
#  define acc_int64e_t              long int
#  define acc_uint64e_t             unsigned long int
#  define ACC_SIZEOF_ACC_INT64E_T   ACC_SIZEOF_LONG
#elif (ACC_SIZEOF_LONG_LONG == 8) && !(ACC_CFG_TYPE_PREFER___INT64)
#  define acc_int64e_t              acc_llong_t
#  define acc_uint64e_t             acc_ullong_t
#  if (ACC_CC_BORLANDC)
#    define ACC_INT64_C(c)          ((c) + 0ll)
#    define ACC_UINT64_C(c)         ((c) + 0ull)
#  else
#    define ACC_INT64_C(c)          c##LL
#    define ACC_UINT64_C(c)         c##ULL
#  endif
#  define ACC_SIZEOF_ACC_INT64E_T   ACC_SIZEOF_LONG_LONG
#elif (ACC_SIZEOF___INT64 == 8)
#  define acc_int64e_t              __int64
#  define acc_uint64e_t             unsigned __int64
#  if (ACC_CC_BORLANDC)
#    define ACC_INT64_C(c)          ((c) + 0i64)
#    define ACC_UINT64_C(c)         ((c) + 0ui64)
#  else
#    define ACC_INT64_C(c)          c##i64
#    define ACC_UINT64_C(c)         c##ui64
#  endif
#  define ACC_SIZEOF_ACC_INT64E_T   ACC_SIZEOF___INT64
#else
#endif
#endif
#if defined(acc_int64e_t)
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int64e_t) == 8)
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int64e_t) == ACC_SIZEOF_ACC_INT64E_T)
#endif
#if !defined(acc_int32l_t)
#if defined(acc_int32e_t)
#  define acc_int32l_t              acc_int32e_t
#  define acc_uint32l_t             acc_uint32e_t
#  define ACC_SIZEOF_ACC_INT32L_T   ACC_SIZEOF_ACC_INT32E_T
#elif (ACC_SIZEOF_INT >= 4) && (ACC_SIZEOF_INT < ACC_SIZEOF_LONG)
#  define acc_int32l_t              int
#  define acc_uint32l_t             unsigned int
#  define ACC_SIZEOF_ACC_INT32L_T   ACC_SIZEOF_INT
#elif (ACC_SIZEOF_LONG >= 4)
#  define acc_int32l_t              long int
#  define acc_uint32l_t             unsigned long int
#  define ACC_SIZEOF_ACC_INT32L_T   ACC_SIZEOF_LONG
#else
#  error "acc_int32l_t"
#endif
#endif
#if 1
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int32l_t) >= 4)
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int32l_t) == ACC_SIZEOF_ACC_INT32L_T)
#endif
#if !defined(acc_int64l_t)
#if defined(acc_int64e_t)
#  define acc_int64l_t              acc_int64e_t
#  define acc_uint64l_t             acc_uint64e_t
#  define ACC_SIZEOF_ACC_INT64L_T   ACC_SIZEOF_ACC_INT64E_T
#else
#endif
#endif
#if defined(acc_int64l_t)
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int64l_t) >= 8)
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int64l_t) == ACC_SIZEOF_ACC_INT64L_T)
#endif
#if !defined(acc_int32f_t)
#if (ACC_SIZEOF_SIZE_T >= 8)
#  define acc_int32f_t              acc_int64l_t
#  define acc_uint32f_t             acc_uint64l_t
#  define ACC_SIZEOF_ACC_INT32F_T   ACC_SIZEOF_ACC_INT64L_T
#else
#  define acc_int32f_t              acc_int32l_t
#  define acc_uint32f_t             acc_uint32l_t
#  define ACC_SIZEOF_ACC_INT32F_T   ACC_SIZEOF_ACC_INT32L_T
#endif
#endif
#if 1
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int32f_t) >= 4)
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int32f_t) == ACC_SIZEOF_ACC_INT32F_T)
#endif
#if !defined(acc_int64f_t)
#if defined(acc_int64l_t)
#  define acc_int64f_t              acc_int64l_t
#  define acc_uint64f_t             acc_uint64l_t
#  define ACC_SIZEOF_ACC_INT64F_T   ACC_SIZEOF_ACC_INT64L_T
#else
#endif
#endif
#if defined(acc_int64f_t)
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int64f_t) >= 8)
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int64f_t) == ACC_SIZEOF_ACC_INT64F_T)
#endif
#if !defined(acc_intptr_t)
#if 1 && (ACC_OS_OS400 && (ACC_SIZEOF_VOID_P == 16))
#  define __ACC_INTPTR_T_IS_POINTER 1
   typedef char*                    acc_intptr_t;
   typedef char*                    acc_uintptr_t;
#  define acc_intptr_t              acc_intptr_t
#  define acc_uintptr_t             acc_uintptr_t
#  define ACC_SIZEOF_ACC_INTPTR_T   ACC_SIZEOF_VOID_P
#elif (ACC_CC_MSC && (_MSC_VER >= 1300) && (ACC_SIZEOF_VOID_P == 4) && (ACC_SIZEOF_INT == 4))
   typedef __w64 int                acc_intptr_t;
   typedef __w64 unsigned int       acc_uintptr_t;
#  define acc_intptr_t              acc_intptr_t
#  define acc_uintptr_t             acc_uintptr_t
#  define ACC_SIZEOF_ACC_INTPTR_T   ACC_SIZEOF_INT
#elif (ACC_SIZEOF_SHORT == ACC_SIZEOF_VOID_P) && (ACC_SIZEOF_INT > ACC_SIZEOF_VOID_P)
#  define acc_intptr_t              short
#  define acc_uintptr_t             unsigned short
#  define ACC_SIZEOF_ACC_INTPTR_T   ACC_SIZEOF_SHORT
#elif (ACC_SIZEOF_INT >= ACC_SIZEOF_VOID_P) && (ACC_SIZEOF_INT < ACC_SIZEOF_LONG)
#  define acc_intptr_t              int
#  define acc_uintptr_t             unsigned int
#  define ACC_SIZEOF_ACC_INTPTR_T   ACC_SIZEOF_INT
#elif (ACC_SIZEOF_LONG >= ACC_SIZEOF_VOID_P)
#  define acc_intptr_t              long
#  define acc_uintptr_t             unsigned long
#  define ACC_SIZEOF_ACC_INTPTR_T   ACC_SIZEOF_LONG
#elif (ACC_SIZEOF_ACC_INT64L_T >= ACC_SIZEOF_VOID_P)
#  define acc_intptr_t              acc_int64l_t
#  define acc_uintptr_t             acc_uint64l_t
#  define ACC_SIZEOF_ACC_INTPTR_T   ACC_SIZEOF_ACC_INT64L_T
#else
#  error "acc_intptr_t"
#endif
#endif
#if 1
    ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_intptr_t) >= sizeof(void *))
    ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_intptr_t) == sizeof(acc_uintptr_t))
#endif
#if !defined(acc_word_t)
#if defined(ACC_WORDSIZE) && (ACC_WORDSIZE+0 > 0)
#if (ACC_WORDSIZE == ACC_SIZEOF_ACC_INTPTR_T) && !defined(__ACC_INTPTR_T_IS_POINTER)
#  define acc_word_t                acc_uintptr_t
#  define acc_sword_t               acc_intptr_t
#  define ACC_SIZEOF_ACC_WORD_T ACC_SIZEOF_ACC_INTPTR_T
#elif (ACC_WORDSIZE == ACC_SIZEOF_LONG)
#  define acc_word_t                unsigned long
#  define acc_sword_t               long
#  define ACC_SIZEOF_ACC_WORD_T ACC_SIZEOF_LONG
#elif (ACC_WORDSIZE == ACC_SIZEOF_INT)
#  define acc_word_t                unsigned int
#  define acc_sword_t               int
#  define ACC_SIZEOF_ACC_WORD_T ACC_SIZEOF_INT
#elif (ACC_WORDSIZE == ACC_SIZEOF_SHORT)
#  define acc_word_t                unsigned short
#  define acc_sword_t               short
#  define ACC_SIZEOF_ACC_WORD_T ACC_SIZEOF_SHORT
#elif (ACC_WORDSIZE == 1)
#  define acc_word_t                unsigned char
#  define acc_sword_t               signed char
#  define ACC_SIZEOF_ACC_WORD_T 1
#elif (ACC_WORDSIZE == ACC_SIZEOF_ACC_INT64L_T)
#  define acc_word_t                acc_uint64l_t
#  define acc_sword_t               acc_int64l_t
#  define ACC_SIZEOF_ACC_WORD_T ACC_SIZEOF_ACC_INT64L_T
#elif (ACC_ARCH_SPU) && (ACC_CC_GNUC)
#if 0
   typedef unsigned acc_word_t  __attribute__((__mode__(__V16QI__)));
   typedef int      acc_sword_t __attribute__((__mode__(__V16QI__)));
#  define acc_word_t                acc_word_t
#  define acc_sword_t               acc_sword_t
#  define ACC_SIZEOF_ACC_WORD_T     16
#endif
#else
#  error "acc_word_t"
#endif
#endif
#endif
#if 1
#define acc_int8_t                  signed char
#define acc_uint8_t                 unsigned char
#define ACC_SIZEOF_ACC_INT8_T       1
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int8_t) == 1)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int8_t) == sizeof(acc_uint8_t))
#endif
#if defined(acc_int16e_t)
#define acc_int16_t                 acc_int16e_t
#define acc_uint16_t                acc_uint16e_t
#define ACC_SIZEOF_ACC_INT16_T      ACC_SIZEOF_ACC_INT16E_T
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int16_t) == 2)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int16_t) == sizeof(acc_uint16_t))
#endif
#if defined(acc_int32e_t)
#define acc_int32_t                 acc_int32e_t
#define acc_uint32_t                acc_uint32e_t
#define ACC_SIZEOF_ACC_INT32_T      ACC_SIZEOF_ACC_INT32E_T
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int32_t) == 4)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int32_t) == sizeof(acc_uint32_t))
#endif
#if defined(acc_int64e_t)
#define acc_int64_t                 acc_int64e_t
#define acc_uint64_t                acc_uint64e_t
#define ACC_SIZEOF_ACC_INT64_T      ACC_SIZEOF_ACC_INT64E_T
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int64_t) == 8)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int64_t) == sizeof(acc_uint64_t))
#endif
#if 1
#define acc_int_least32_t           acc_int32l_t
#define acc_uint_least32_t          acc_uint32l_t
#define ACC_SIZEOF_ACC_INT_LEAST32_T ACC_SIZEOF_ACC_INT32L_T
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int_least32_t) >= 4)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int_least32_t) == sizeof(acc_uint_least32_t))
#endif
#if defined(acc_int64l_t)
#define acc_int_least64_t           acc_int64l_t
#define acc_uint_least64_t          acc_uint64l_t
#define ACC_SIZEOF_ACC_INT_LEAST64_T ACC_SIZEOF_ACC_INT64L_T
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int_least64_t) >= 8)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int_least64_t) == sizeof(acc_uint_least64_t))
#endif
#if 1
#define acc_int_fast32_t           acc_int32f_t
#define acc_uint_fast32_t          acc_uint32f_t
#define ACC_SIZEOF_ACC_INT_FAST32_T ACC_SIZEOF_ACC_INT32F_T
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int_fast32_t) >= 4)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int_fast32_t) == sizeof(acc_uint_fast32_t))
#endif
#if defined(acc_int64f_t)
#define acc_int_fast64_t           acc_int64f_t
#define acc_uint_fast64_t          acc_uint64f_t
#define ACC_SIZEOF_ACC_INT_FAST64_T ACC_SIZEOF_ACC_INT64F_T
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int_fast64_t) >= 8)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int_fast64_t) == sizeof(acc_uint_fast64_t))
#endif
#if !defined(ACC_INT16_C)
#  if (ACC_BROKEN_INTEGRAL_CONSTANTS) && (ACC_SIZEOF_INT >= 2)
#    define ACC_INT16_C(c)          ((c) + 0)
#    define ACC_UINT16_C(c)         ((c) + 0U)
#  elif (ACC_BROKEN_INTEGRAL_CONSTANTS) && (ACC_SIZEOF_LONG >= 2)
#    define ACC_INT16_C(c)          ((c) + 0L)
#    define ACC_UINT16_C(c)         ((c) + 0UL)
#  elif (ACC_SIZEOF_INT >= 2)
#    define ACC_INT16_C(c)          c
#    define ACC_UINT16_C(c)         c##U
#  elif (ACC_SIZEOF_LONG >= 2)
#    define ACC_INT16_C(c)          c##L
#    define ACC_UINT16_C(c)         c##UL
#  else
#    error "ACC_INT16_C"
#  endif
#endif
#if !defined(ACC_INT32_C)
#  if (ACC_BROKEN_INTEGRAL_CONSTANTS) && (ACC_SIZEOF_INT >= 4)
#    define ACC_INT32_C(c)          ((c) + 0)
#    define ACC_UINT32_C(c)         ((c) + 0U)
#  elif (ACC_BROKEN_INTEGRAL_CONSTANTS) && (ACC_SIZEOF_LONG >= 4)
#    define ACC_INT32_C(c)          ((c) + 0L)
#    define ACC_UINT32_C(c)         ((c) + 0UL)
#  elif (ACC_SIZEOF_INT >= 4)
#    define ACC_INT32_C(c)          c
#    define ACC_UINT32_C(c)         c##U
#  elif (ACC_SIZEOF_LONG >= 4)
#    define ACC_INT32_C(c)          c##L
#    define ACC_UINT32_C(c)         c##UL
#  elif (ACC_SIZEOF_LONG_LONG >= 4)
#    define ACC_INT32_C(c)          c##LL
#    define ACC_UINT32_C(c)         c##ULL
#  else
#    error "ACC_INT32_C"
#  endif
#endif
#if !defined(ACC_INT64_C) && defined(acc_int64l_t)
#  if (ACC_BROKEN_INTEGRAL_CONSTANTS) && (ACC_SIZEOF_INT >= 8)
#    define ACC_INT64_C(c)          ((c) + 0)
#    define ACC_UINT64_C(c)         ((c) + 0U)
#  elif (ACC_BROKEN_INTEGRAL_CONSTANTS) && (ACC_SIZEOF_LONG >= 8)
#    define ACC_INT64_C(c)          ((c) + 0L)
#    define ACC_UINT64_C(c)         ((c) + 0UL)
#  elif (ACC_SIZEOF_INT >= 8)
#    define ACC_INT64_C(c)          c
#    define ACC_UINT64_C(c)         c##U
#  elif (ACC_SIZEOF_LONG >= 8)
#    define ACC_INT64_C(c)          c##L
#    define ACC_UINT64_C(c)         c##UL
#  else
#    error "ACC_INT64_C"
#  endif
#endif
#endif
#if (ACC_CFG_NO_CONFIG_HEADER)
#elif defined(ACC_CFG_CONFIG_HEADER)
#else
#if !(ACC_CFG_AUTO_NO_HEADERS)
#if (ACC_LIBC_NAKED)
#elif (ACC_LIBC_FREESTANDING)
#  define HAVE_LIMITS_H 1
#  define HAVE_STDARG_H 1
#  define HAVE_STDDEF_H 1
#elif (ACC_LIBC_MOSTLY_FREESTANDING)
#  define HAVE_LIMITS_H 1
#  define HAVE_SETJMP_H 1
#  define HAVE_STDARG_H 1
#  define HAVE_STDDEF_H 1
#  define HAVE_STDIO_H 1
#  define HAVE_STRING_H 1
#else
#define STDC_HEADERS 1
#define HAVE_ASSERT_H 1
#define HAVE_CTYPE_H 1
#define HAVE_DIRENT_H 1
#define HAVE_ERRNO_H 1
#define HAVE_FCNTL_H 1
#define HAVE_FLOAT_H 1
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
#if (ACC_OS_POSIX)
#  if (ACC_OS_POSIX_AIX)
#    define HAVE_SYS_RESOURCE_H 1
#  elif (ACC_OS_POSIX_FREEBSD || ACC_OS_POSIX_MACOSX || ACC_OS_POSIX_NETBSD || ACC_OS_POSIX_OPENBSD)
#    define HAVE_STRINGS_H 1
#    undef HAVE_MALLOC_H
#  elif (ACC_OS_POSIX_HPUX || ACC_OS_POSIX_INTERIX)
#    define HAVE_ALLOCA_H 1
#  elif (ACC_OS_POSIX_MACOSX && ACC_LIBC_MSL)
#    undef HAVE_SYS_TIME_H
#    undef HAVE_SYS_TYPES_H
#  elif (ACC_OS_POSIX_SOLARIS || ACC_OS_POSIX_SUNOS)
#    define HAVE_ALLOCA_H 1
#  endif
#  if (ACC_LIBC_DIETLIBC || ACC_LIBC_GLIBC || ACC_LIBC_UCLIBC)
#    define HAVE_STRINGS_H 1
#    define HAVE_SYS_MMAN_H 1
#    define HAVE_SYS_RESOURCE_H 1
#    define HAVE_SYS_WAIT_H 1
#  endif
#  if (ACC_LIBC_NEWLIB)
#    undef HAVE_STRINGS_H
#  endif
#elif (ACC_OS_CYGWIN)
#  define HAVE_IO_H 1
#elif (ACC_OS_EMX)
#  define HAVE_ALLOCA_H 1
#  define HAVE_IO_H 1
#elif (ACC_ARCH_M68K && ACC_OS_TOS && ACC_CC_GNUC)
#  if !defined(__MINT__)
#    undef HAVE_MALLOC_H
#  endif
#elif (ACC_ARCH_M68K && ACC_OS_TOS && (ACC_CC_PUREC || ACC_CC_TURBOC))
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
#    undef HAVE_DIRENT_H
#  endif
#  if (__BORLANDC__ < 0x0400)
#    undef HAVE_DIRENT_H
#    undef HAVE_UTIME_H
#  endif
#elif (ACC_CC_DMC)
#  undef HAVE_DIRENT_H
#  undef HAVE_UNISTD_H
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
#elif (ACC_CC_LCCWIN32)
#  undef HAVE_DIRENT_H
#  undef HAVE_DOS_H
#  undef HAVE_UNISTD_H
#  undef HAVE_SYS_TIME_H
#elif (ACC_OS_WIN32 && ACC_CC_GNUC) && defined(__MINGW32__)
#  undef HAVE_UTIME_H
#  define HAVE_SYS_UTIME_H 1
#elif (ACC_OS_WIN32 && ACC_LIBC_MSL)
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
#  undef HAVE_DIRENT_H
#  undef HAVE_UNISTD_H
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
#  undef HAVE_SYS_TYPES_H
#  if (ACC_OS_WIN32 || ACC_OS_WIN64)
#    undef HAVE_DIRENT_H
#  endif
#  if (__TURBOC__ < 0x0200)
#    undef HAVE_SIGNAL_H
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
#endif
#if (ACC_OS_CONSOLE)
#  undef HAVE_DIRENT_H
#endif
#if (ACC_OS_EMBEDDED)
#  undef HAVE_DIRENT_H
#endif
#if (ACC_LIBC_ISOC90 || ACC_LIBC_ISOC99)
#  undef HAVE_DIRENT_H
#  undef HAVE_FCNTL_H
#  undef HAVE_MALLOC_H
#  undef HAVE_UNISTD_H
#  undef HAVE_UTIME_H
#  undef HAVE_SYS_STAT_H
#  undef HAVE_SYS_TIME_H
#  undef HAVE_SYS_TYPES_H
#endif
#if (ACC_LIBC_GLIBC >= 0x020100ul)
#  define HAVE_STDINT_H 1
#elif (ACC_LIBC_DIETLIBC)
#  undef HAVE_STDINT_H
#elif (ACC_LIBC_UCLIBC)
#  define HAVE_STDINT_H 1
#elif (ACC_CC_BORLANDC) && (__BORLANDC__ >= 0x560)
#  undef HAVE_STDINT_H
#elif (ACC_CC_DMC) && (__DMC__ >= 0x825)
#  define HAVE_STDINT_H 1
#endif
#if (HAVE_SYS_TIME_H && HAVE_TIME_H)
#  define TIME_WITH_SYS_TIME 1
#endif
#endif
#endif
#if !(ACC_CFG_AUTO_NO_FUNCTIONS)
#if (ACC_LIBC_NAKED)
#elif (ACC_LIBC_FREESTANDING)
#elif (ACC_LIBC_MOSTLY_FREESTANDING)
#  define HAVE_LONGJMP 1
#  define HAVE_MEMCMP 1
#  define HAVE_MEMCPY 1
#  define HAVE_MEMMOVE 1
#  define HAVE_MEMSET 1
#  define HAVE_SETJMP 1
#else
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
#define HAVE_GETENV 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_GMTIME 1
#define HAVE_ISATTY 1
#define HAVE_LOCALTIME 1
#define HAVE_LONGJMP 1
#define HAVE_LSTAT 1
#define HAVE_MEMCMP 1
#define HAVE_MEMCPY 1
#define HAVE_MEMMOVE 1
#define HAVE_MEMSET 1
#define HAVE_MKDIR 1
#define HAVE_MKTIME 1
#define HAVE_QSORT 1
#define HAVE_RAISE 1
#define HAVE_RMDIR 1
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
#if (ACC_OS_BEOS || ACC_OS_CYGWIN || ACC_OS_POSIX || ACC_OS_QNX || ACC_OS_VMS)
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
#  if (ACC_OS_POSIX_AIX)
#    define HAVE_GETRUSAGE 1
#  elif (ACC_OS_POSIX_MACOSX && ACC_LIBC_MSL)
#    undef HAVE_CHOWN
#    undef HAVE_LSTAT
#  elif (ACC_OS_POSIX_UNICOS)
#    undef HAVE_ALLOCA
#    undef HAVE_SNPRINTF
#    undef HAVE_VSNPRINTF
#  endif
#  if (ACC_CC_TINYC)
#    undef HAVE_ALLOCA
#  endif
#  if (ACC_LIBC_DIETLIBC || ACC_LIBC_GLIBC || ACC_LIBC_UCLIBC)
#    define HAVE_GETRUSAGE 1
#    define HAVE_GETPAGESIZE 1
#    define HAVE_MMAP 1
#    define HAVE_MPROTECT 1
#    define HAVE_MUNMAP 1
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
#elif (ACC_ARCH_M68K && ACC_OS_TOS && ACC_CC_GNUC)
#  if !defined(__MINT__)
#    undef HAVE_SNPRINTF
#    undef HAVE_VSNPRINTF
#  endif
#elif (ACC_ARCH_M68K && ACC_OS_TOS && (ACC_CC_PUREC || ACC_CC_TURBOC))
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
#if (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)
#undef HAVE_CHOWN
#undef HAVE_GETTIMEOFDAY
#undef HAVE_LSTAT
#undef HAVE_UMASK
#if (ACC_CC_AZTECC)
#  undef HAVE_ALLOCA
#  undef HAVE_DIFFTIME
#  undef HAVE_FSTAT
#  undef HAVE_STRDUP
#  undef HAVE_SNPRINTF
#  undef HAVE_UTIME
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
#  ifndef snprintf
#  define snprintf _snprintf
#  endif
#  ifndef vsnprintf
#  define vsnprintf _vsnprintf
#  endif
#elif (ACC_CC_LCCWIN32)
#  define utime _utime
#elif (ACC_CC_MSC)
#  if (_MSC_VER < 600)
#    undef HAVE_STRFTIME
#  endif
#  if (_MSC_VER < 700)
#    undef HAVE_SNPRINTF
#    undef HAVE_VSNPRINTF
#  elif (_MSC_VER < 1500)
#    ifndef snprintf
#    define snprintf _snprintf
#    endif
#    ifndef vsnprintf
#    define vsnprintf _vsnprintf
#    endif
#  else
#    ifndef snprintf
#    define snprintf _snprintf
#    endif
#  endif
#  if ((_MSC_VER < 800) && ACC_OS_WIN16)
#    undef HAVE_ALLOCA
#  endif
#  if (ACC_ARCH_I086) && defined(__cplusplus)
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
#elif (ACC_OS_WIN32 && ACC_LIBC_MSL)
#  if (__MSL__ < 0x8000ul)
#    undef HAVE_CHMOD
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
#    undef HAVE_DIFFTIME
#    undef HAVE_UTIME
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
#  undef HAVE_DIFFTIME
#  undef HAVE_SNPRINTF
#  undef HAVE_UTIME
#  undef HAVE_VSNPRINTF
#endif
#endif
#if (ACC_OS_CONSOLE)
#  undef HAVE_ACCESS
#  undef HAVE_CHMOD
#  undef HAVE_CHOWN
#  undef HAVE_GETTIMEOFDAY
#  undef HAVE_LSTAT
#  undef HAVE_TIME
#  undef HAVE_UMASK
#  undef HAVE_UTIME
#endif
#if (ACC_LIBC_ISOC90 || ACC_LIBC_ISOC99)
#  undef HAVE_ACCESS
#  undef HAVE_CHMOD
#  undef HAVE_CHOWN
#  undef HAVE_FILENO
#  undef HAVE_FSTAT
#  undef HAVE_GETTIMEOFDAY
#  undef HAVE_LSTAT
#  undef HAVE_STAT
#  undef HAVE_UMASK
#  undef HAVE_UTIME
# if 1
#  undef HAVE_ALLOCA
#  undef HAVE_ISATTY
#  undef HAVE_MKDIR
#  undef HAVE_RMDIR
#  undef HAVE_STRDUP
#  undef HAVE_STRICMP
#  undef HAVE_STRNICMP
# endif
#endif
#endif
#endif
#if !(ACC_CFG_AUTO_NO_SIZES)
#if !defined(SIZEOF_SHORT) && defined(ACC_SIZEOF_SHORT)
#  define SIZEOF_SHORT          ACC_SIZEOF_SHORT
#endif
#if !defined(SIZEOF_INT) && defined(ACC_SIZEOF_INT)
#  define SIZEOF_INT            ACC_SIZEOF_INT
#endif
#if !defined(SIZEOF_LONG) && defined(ACC_SIZEOF_LONG)
#  define SIZEOF_LONG           ACC_SIZEOF_LONG
#endif
#if !defined(SIZEOF_LONG_LONG) && defined(ACC_SIZEOF_LONG_LONG)
#  define SIZEOF_LONG_LONG      ACC_SIZEOF_LONG_LONG
#endif
#if !defined(SIZEOF___INT32) && defined(ACC_SIZEOF___INT32)
#  define SIZEOF___INT32        ACC_SIZEOF___INT32
#endif
#if !defined(SIZEOF___INT64) && defined(ACC_SIZEOF___INT64)
#  define SIZEOF___INT64        ACC_SIZEOF___INT64
#endif
#if !defined(SIZEOF_VOID_P) && defined(ACC_SIZEOF_VOID_P)
#  define SIZEOF_VOID_P         ACC_SIZEOF_VOID_P
#endif
#if !defined(SIZEOF_SIZE_T) && defined(ACC_SIZEOF_SIZE_T)
#  define SIZEOF_SIZE_T         ACC_SIZEOF_SIZE_T
#endif
#if !defined(SIZEOF_PTRDIFF_T) && defined(ACC_SIZEOF_PTRDIFF_T)
#  define SIZEOF_PTRDIFF_T      ACC_SIZEOF_PTRDIFF_T
#endif
#endif
#if (HAVE_SIGNAL) && !defined(RETSIGTYPE)
#  define RETSIGTYPE void
#endif
#endif
#if !(ACC_CFG_SKIP_ACC_TYPES)
#if 1 && !defined(acc_signo_t) && defined(__linux__) && defined(__dietlibc__) && (ACC_SIZEOF_INT != 4)
#  define acc_signo_t               acc_int32e_t
#endif
#if !defined(acc_signo_t)
#  define acc_signo_t               int
#endif
#if defined(__cplusplus)
extern "C" {
#endif
#if (ACC_BROKEN_CDECL_ALT_SYNTAX)
typedef void __acc_cdecl_sighandler (*acc_sighandler_t)(acc_signo_t);
#elif defined(RETSIGTYPE)
typedef RETSIGTYPE (__acc_cdecl_sighandler *acc_sighandler_t)(acc_signo_t);
#else
typedef void (__acc_cdecl_sighandler *acc_sighandler_t)(acc_signo_t);
#endif
#if defined(__cplusplus)
}
#endif
#  if !(ACC_CFG_SKIP_ACC_UA)
#if (ACC_CC_GNUC && (ACC_CC_GNUC < 0x020700ul))
#elif (ACC_CC_GNUC && (ACC_CC_GNUC < 0x020800ul)) && defined(__cplusplus)
#elif (ACC_CC_INTELC) && defined(_WIN32)
#elif (ACC_CC_INTELC && (__INTEL_COMPILER < 700))
#elif (ACC_CC_LLVM)
#elif (ACC_CC_GNUC || ACC_CC_INTELC || ACC_CC_PATHSCALE)
#if !defined(__acc_ua16_t) && (ACC_OPT_UNALIGNED16) && defined(acc_int16e_t)
   typedef struct { __acc_ua_volatile acc_uint16e_t v __attribute__((__packed__)); } __acc_ua16_t;
#  define __acc_ua16_t __acc_ua16_t
#endif
#if !defined(__acc_ua32_t) && (ACC_OPT_UNALIGNED32) && defined(acc_int32e_t)
   typedef struct { __acc_ua_volatile acc_uint32e_t v __attribute__((__packed__)); } __acc_ua32_t;
#  define __acc_ua32_t __acc_ua32_t
#endif
#if !defined(__acc_ua64_t) && (ACC_OPT_UNALIGNED64) && defined(acc_int64l_t)
   typedef struct { __acc_ua_volatile acc_uint64l_t v __attribute__((__packed__)); } __acc_ua64_t;
#  define __acc_ua64_t __acc_ua64_t
#endif
#endif
#if (ACC_OPT_UNALIGNED16) && defined(acc_int16e_t)
#define ACC_UA_GET16(p)         (* (__acc_ua_volatile const acc_uint16e_t*) (__acc_ua_volatile const void*) (p))
#define ACC_UA_SET16(p,v)       ((* (__acc_ua_volatile acc_uint16e_t*) (__acc_ua_volatile void*) (p)) = (acc_uint16e_t) (v))
#if (ACC_ABI_BIG_ENDIAN)
#  define ACC_UA_GET_BE16(p)    ACC_UA_GET16(p)
#  define ACC_UA_SET_BE16(p,v)  ACC_UA_SET16(p,v)
#elif (ACC_ABI_LITTLE_ENDIAN)
#  define ACC_UA_GET_LE16(p)    ACC_UA_GET16(p)
#  define ACC_UA_SET_LE16(p,v)  ACC_UA_SET16(p,v)
#endif
#if (__acc_HAVE_forceinline)
#if (ACC_ARCH_POWERPC && ACC_ABI_BIG_ENDIAN) && (ACC_ASM_SYNTAX_GNUC) && (ACC_CC_GNUC)
#if !defined(ACC_UA_GET_LE16)
static __acc_forceinline
#if (ACC_CC_CLANG || (ACC_CC_GNUC >= 0x020800ul) || ACC_CC_INTELC_GNUC || ACC_CC_LLVM)
__attribute__((__unused__))
#endif
unsigned long __ACC_UA_GET_LE16(__acc_ua_volatile const void* pp) {
    __acc_ua_volatile const acc_uint16e_t* p = (__acc_ua_volatile const acc_uint16e_t*) pp;
    unsigned long v;
    __asm__ __volatile__("lhbrx %0,0,%1" : "=r" (v) : "r" (p), "m" (*p));
    return v;
}
#define ACC_UA_GET_LE16(p)      __ACC_UA_GET_LE16(p)
#endif
#if !defined(ACC_UA_SET_LE16)
static __acc_forceinline
#if (ACC_CC_CLANG || (ACC_CC_GNUC >= 0x020800ul) || ACC_CC_INTELC_GNUC || ACC_CC_LLVM)
__attribute__((__unused__))
#endif
void __ACC_UA_SET_LE16(__acc_ua_volatile void* pp, unsigned long v) {
    __acc_ua_volatile acc_uint16e_t* p = (__acc_ua_volatile acc_uint16e_t*) pp;
    __asm__ __volatile__("sthbrx %2,0,%1" : "=m" (*p) : "r" (p), "r" (v));
}
#define ACC_UA_SET_LE16(p,v)    __ACC_UA_SET_LE16(p,v)
#endif
#endif
#endif
#if !defined(ACC_UA_COPY16)
#  define ACC_UA_COPY16(d,s)    ACC_UA_SET16(d, ACC_UA_GET16(s))
#endif
#endif
#if (ACC_OPT_UNALIGNED32) && defined(acc_int32e_t)
#define ACC_UA_GET32(p)         (* (__acc_ua_volatile const acc_uint32e_t*) (__acc_ua_volatile const void*) (p))
#define ACC_UA_SET32(p,v)       ((* (__acc_ua_volatile acc_uint32e_t*) (__acc_ua_volatile void*) (p)) = (acc_uint32e_t) (v))
#if (ACC_ABI_BIG_ENDIAN)
#  define ACC_UA_GET_BE32(p)    ACC_UA_GET32(p)
#  define ACC_UA_SET_BE32(p,v)  ACC_UA_SET32(p,v)
#elif (ACC_ABI_LITTLE_ENDIAN)
#  define ACC_UA_GET_LE32(p)    ACC_UA_GET32(p)
#  define ACC_UA_SET_LE32(p,v)  ACC_UA_SET32(p,v)
#endif
#if (__acc_HAVE_forceinline)
#if (ACC_ARCH_POWERPC && ACC_ABI_BIG_ENDIAN) && (ACC_ASM_SYNTAX_GNUC) && (ACC_CC_GNUC)
#if !defined(ACC_UA_GET_LE32)
static __acc_forceinline
#if (ACC_CC_CLANG || (ACC_CC_GNUC >= 0x020800ul) || ACC_CC_INTELC_GNUC || ACC_CC_LLVM)
__attribute__((__unused__))
#endif
unsigned long __ACC_UA_GET_LE32(__acc_ua_volatile const void* pp) {
    __acc_ua_volatile const acc_uint32e_t* p = (__acc_ua_volatile const acc_uint32e_t*) pp;
    unsigned long v;
    __asm__ __volatile__("lwbrx %0,0,%1" : "=r" (v) : "r" (p), "m" (*p));
    return v;
}
#define ACC_UA_GET_LE32(p)      __ACC_UA_GET_LE32(p)
#endif
#if !defined(ACC_UA_SET_LE32)
static __acc_forceinline
#if (ACC_CC_CLANG || (ACC_CC_GNUC >= 0x020800ul) || ACC_CC_INTELC_GNUC || ACC_CC_LLVM)
__attribute__((__unused__))
#endif
void __ACC_UA_SET_LE32(__acc_ua_volatile void* pp, unsigned long v) {
    __acc_ua_volatile acc_uint32e_t* p = (__acc_ua_volatile acc_uint32e_t*) pp;
    __asm__ __volatile__("stwbrx %2,0,%1" : "=m" (*p) : "r" (p), "r" (v));
}
#define ACC_UA_SET_LE32(p,v)    __ACC_UA_SET_LE32(p,v)
#endif
#endif
#endif
#if !defined(ACC_UA_COPY32)
#  define ACC_UA_COPY32(d,s)    ACC_UA_SET32(d, ACC_UA_GET32(s))
#endif
#endif
#if (ACC_OPT_UNALIGNED64) && defined(acc_int64l_t)
#define ACC_UA_GET64(p)         (* (__acc_ua_volatile const acc_uint64l_t*) (__acc_ua_volatile const void*) (p))
#define ACC_UA_SET64(p,v)       ((* (__acc_ua_volatile acc_uint64l_t*) (__acc_ua_volatile void*) (p)) = (acc_uint64l_t) (v))
#if (ACC_ABI_BIG_ENDIAN)
#  define ACC_UA_GET_BE64(p)    ACC_UA_GET64(p)
#  define ACC_UA_SET_BE64(p,v)  ACC_UA_SET64(p,v)
#elif (ACC_ABI_LITTLE_ENDIAN)
#  define ACC_UA_GET_LE64(p)    ACC_UA_GET64(p)
#  define ACC_UA_SET_LE64(p,v)  ACC_UA_SET64(p,v)
#endif
#if !defined(ACC_UA_COPY64)
#  define ACC_UA_COPY64(d,s)    ACC_UA_SET64(d, ACC_UA_GET64(s))
#endif
#endif
#  endif
#endif
#endif
#if defined(ACC_WANT_ACC_TYPE_H)
#  undef ACC_WANT_ACC_TYPE_H
#  if (ACC_CFG_SKIP_ACC_TYPES)
#    error "ACC_WANT_ACC_TYPE_H with ACC_CFG_SKIP_ACC_TYPES"
#  endif
#if (!(ACC_SIZEOF_SHORT+0 > 0 && ACC_SIZEOF_INT+0 > 0 && ACC_SIZEOF_LONG+0 > 0))
#  error "missing defines for sizes"
#endif
#if (!(ACC_SIZEOF_PTRDIFF_T+0 > 0 && ACC_SIZEOF_SIZE_T+0 > 0 && ACC_SIZEOF_VOID_P+0 > 0))
#  error "missing defines for sizes"
#endif
#if !defined(acc_llong_t)
#if (ACC_SIZEOF_LONG_LONG+0 > 0)
__acc_gnuc_extension__ typedef long long acc_llong_t__;
__acc_gnuc_extension__ typedef unsigned long long acc_ullong_t__;
#  define acc_llong_t               acc_llong_t__
#  define acc_ullong_t              acc_ullong_t__
#endif
#endif
#if !defined(acc_int16e_t)
#if (ACC_SIZEOF_LONG == 2)
#  define acc_int16e_t              long
#  define acc_uint16e_t             unsigned long
#elif (ACC_SIZEOF_INT == 2)
#  define acc_int16e_t              int
#  define acc_uint16e_t             unsigned int
#elif (ACC_SIZEOF_SHORT == 2)
#  define acc_int16e_t              short int
#  define acc_uint16e_t             unsigned short int
#elif 1 && !(ACC_CFG_TYPE_NO_MODE_HI) && (ACC_CC_CLANG || (ACC_CC_GNUC >= 0x025f00ul) || ACC_CC_LLVM)
   typedef int acc_int16e_hi_t__ __attribute__((__mode__(__HI__)));
   typedef unsigned int acc_uint16e_hi_t__ __attribute__((__mode__(__HI__)));
#  define acc_int16e_t              acc_int16e_hi_t__
#  define acc_uint16e_t             acc_uint16e_hi_t__
#elif (ACC_SIZEOF___INT16 == 2)
#  define acc_int16e_t              __int16
#  define acc_uint16e_t             unsigned __int16
#else
#endif
#endif
#if defined(acc_int16e_t)
#  define ACC_SIZEOF_ACC_INT16E_T   2
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int16e_t) == 2)
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int16e_t) == ACC_SIZEOF_ACC_INT16E_T)
#endif
#if !defined(acc_int32e_t)
#if (ACC_SIZEOF_LONG == 4)
#  define acc_int32e_t              long int
#  define acc_uint32e_t             unsigned long int
#elif (ACC_SIZEOF_INT == 4)
#  define acc_int32e_t              int
#  define acc_uint32e_t             unsigned int
#elif (ACC_SIZEOF_SHORT == 4)
#  define acc_int32e_t              short int
#  define acc_uint32e_t             unsigned short int
#elif (ACC_SIZEOF_LONG_LONG == 4)
#  define acc_int32e_t              acc_llong_t
#  define acc_uint32e_t             acc_ullong_t
#elif 1 && !(ACC_CFG_TYPE_NO_MODE_SI) && (ACC_CC_CLANG || (ACC_CC_GNUC >= 0x025f00ul) || ACC_CC_LLVM) && (__INT_MAX__+0 > 2147483647L)
   typedef int acc_int32e_si_t__ __attribute__((__mode__(__SI__)));
   typedef unsigned int acc_uint32e_si_t__ __attribute__((__mode__(__SI__)));
#  define acc_int32e_t              acc_int32e_si_t__
#  define acc_uint32e_t             acc_uint32e_si_t__
#elif 1 && !(ACC_CFG_TYPE_NO_MODE_SI) && (ACC_CC_GNUC >= 0x025f00ul) && defined(__AVR__) && (__LONG_MAX__+0 == 32767L)
   typedef int acc_int32e_si_t__ __attribute__((__mode__(__SI__)));
   typedef unsigned int acc_uint32e_si_t__ __attribute__((__mode__(__SI__)));
#  define acc_int32e_t              acc_int32e_si_t__
#  define acc_uint32e_t             acc_uint32e_si_t__
#  define ACC_INT32_C(c)            c##LL
#  define ACC_UINT32_C(c)           c##ULL
#elif (ACC_SIZEOF___INT32 == 4)
#  define acc_int32e_t              __int32
#  define acc_uint32e_t             unsigned __int32
#else
#endif
#endif
#if defined(acc_int32e_t)
#  define ACC_SIZEOF_ACC_INT32E_T   4
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int32e_t) == 4)
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int32e_t) == ACC_SIZEOF_ACC_INT32E_T)
#endif
#if !defined(acc_int64e_t)
#if (ACC_SIZEOF___INT64 == 8)
#  if (ACC_CC_BORLANDC) && !(ACC_CFG_TYPE_PREFER___INT64)
#    define ACC_CFG_TYPE_PREFER___INT64 1
#  endif
#endif
#if (ACC_SIZEOF_INT == 8) && (ACC_SIZEOF_INT < ACC_SIZEOF_LONG)
#  define acc_int64e_t              int
#  define acc_uint64e_t             unsigned int
#  define ACC_SIZEOF_ACC_INT64E_T   ACC_SIZEOF_INT
#elif (ACC_SIZEOF_LONG == 8)
#  define acc_int64e_t              long int
#  define acc_uint64e_t             unsigned long int
#  define ACC_SIZEOF_ACC_INT64E_T   ACC_SIZEOF_LONG
#elif (ACC_SIZEOF_LONG_LONG == 8) && !(ACC_CFG_TYPE_PREFER___INT64)
#  define acc_int64e_t              acc_llong_t
#  define acc_uint64e_t             acc_ullong_t
#  if (ACC_CC_BORLANDC)
#    define ACC_INT64_C(c)          ((c) + 0ll)
#    define ACC_UINT64_C(c)         ((c) + 0ull)
#  else
#    define ACC_INT64_C(c)          c##LL
#    define ACC_UINT64_C(c)         c##ULL
#  endif
#  define ACC_SIZEOF_ACC_INT64E_T   ACC_SIZEOF_LONG_LONG
#elif (ACC_SIZEOF___INT64 == 8)
#  define acc_int64e_t              __int64
#  define acc_uint64e_t             unsigned __int64
#  if (ACC_CC_BORLANDC)
#    define ACC_INT64_C(c)          ((c) + 0i64)
#    define ACC_UINT64_C(c)         ((c) + 0ui64)
#  else
#    define ACC_INT64_C(c)          c##i64
#    define ACC_UINT64_C(c)         c##ui64
#  endif
#  define ACC_SIZEOF_ACC_INT64E_T   ACC_SIZEOF___INT64
#else
#endif
#endif
#if defined(acc_int64e_t)
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int64e_t) == 8)
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int64e_t) == ACC_SIZEOF_ACC_INT64E_T)
#endif
#if !defined(acc_int32l_t)
#if defined(acc_int32e_t)
#  define acc_int32l_t              acc_int32e_t
#  define acc_uint32l_t             acc_uint32e_t
#  define ACC_SIZEOF_ACC_INT32L_T   ACC_SIZEOF_ACC_INT32E_T
#elif (ACC_SIZEOF_INT >= 4) && (ACC_SIZEOF_INT < ACC_SIZEOF_LONG)
#  define acc_int32l_t              int
#  define acc_uint32l_t             unsigned int
#  define ACC_SIZEOF_ACC_INT32L_T   ACC_SIZEOF_INT
#elif (ACC_SIZEOF_LONG >= 4)
#  define acc_int32l_t              long int
#  define acc_uint32l_t             unsigned long int
#  define ACC_SIZEOF_ACC_INT32L_T   ACC_SIZEOF_LONG
#else
#  error "acc_int32l_t"
#endif
#endif
#if 1
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int32l_t) >= 4)
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int32l_t) == ACC_SIZEOF_ACC_INT32L_T)
#endif
#if !defined(acc_int64l_t)
#if defined(acc_int64e_t)
#  define acc_int64l_t              acc_int64e_t
#  define acc_uint64l_t             acc_uint64e_t
#  define ACC_SIZEOF_ACC_INT64L_T   ACC_SIZEOF_ACC_INT64E_T
#else
#endif
#endif
#if defined(acc_int64l_t)
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int64l_t) >= 8)
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int64l_t) == ACC_SIZEOF_ACC_INT64L_T)
#endif
#if !defined(acc_int32f_t)
#if (ACC_SIZEOF_SIZE_T >= 8)
#  define acc_int32f_t              acc_int64l_t
#  define acc_uint32f_t             acc_uint64l_t
#  define ACC_SIZEOF_ACC_INT32F_T   ACC_SIZEOF_ACC_INT64L_T
#else
#  define acc_int32f_t              acc_int32l_t
#  define acc_uint32f_t             acc_uint32l_t
#  define ACC_SIZEOF_ACC_INT32F_T   ACC_SIZEOF_ACC_INT32L_T
#endif
#endif
#if 1
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int32f_t) >= 4)
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int32f_t) == ACC_SIZEOF_ACC_INT32F_T)
#endif
#if !defined(acc_int64f_t)
#if defined(acc_int64l_t)
#  define acc_int64f_t              acc_int64l_t
#  define acc_uint64f_t             acc_uint64l_t
#  define ACC_SIZEOF_ACC_INT64F_T   ACC_SIZEOF_ACC_INT64L_T
#else
#endif
#endif
#if defined(acc_int64f_t)
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int64f_t) >= 8)
   ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int64f_t) == ACC_SIZEOF_ACC_INT64F_T)
#endif
#if !defined(acc_intptr_t)
#if 1 && (ACC_OS_OS400 && (ACC_SIZEOF_VOID_P == 16))
#  define __ACC_INTPTR_T_IS_POINTER 1
   typedef char*                    acc_intptr_t;
   typedef char*                    acc_uintptr_t;
#  define acc_intptr_t              acc_intptr_t
#  define acc_uintptr_t             acc_uintptr_t
#  define ACC_SIZEOF_ACC_INTPTR_T   ACC_SIZEOF_VOID_P
#elif (ACC_CC_MSC && (_MSC_VER >= 1300) && (ACC_SIZEOF_VOID_P == 4) && (ACC_SIZEOF_INT == 4))
   typedef __w64 int                acc_intptr_t;
   typedef __w64 unsigned int       acc_uintptr_t;
#  define acc_intptr_t              acc_intptr_t
#  define acc_uintptr_t             acc_uintptr_t
#  define ACC_SIZEOF_ACC_INTPTR_T   ACC_SIZEOF_INT
#elif (ACC_SIZEOF_SHORT == ACC_SIZEOF_VOID_P) && (ACC_SIZEOF_INT > ACC_SIZEOF_VOID_P)
#  define acc_intptr_t              short
#  define acc_uintptr_t             unsigned short
#  define ACC_SIZEOF_ACC_INTPTR_T   ACC_SIZEOF_SHORT
#elif (ACC_SIZEOF_INT >= ACC_SIZEOF_VOID_P) && (ACC_SIZEOF_INT < ACC_SIZEOF_LONG)
#  define acc_intptr_t              int
#  define acc_uintptr_t             unsigned int
#  define ACC_SIZEOF_ACC_INTPTR_T   ACC_SIZEOF_INT
#elif (ACC_SIZEOF_LONG >= ACC_SIZEOF_VOID_P)
#  define acc_intptr_t              long
#  define acc_uintptr_t             unsigned long
#  define ACC_SIZEOF_ACC_INTPTR_T   ACC_SIZEOF_LONG
#elif (ACC_SIZEOF_ACC_INT64L_T >= ACC_SIZEOF_VOID_P)
#  define acc_intptr_t              acc_int64l_t
#  define acc_uintptr_t             acc_uint64l_t
#  define ACC_SIZEOF_ACC_INTPTR_T   ACC_SIZEOF_ACC_INT64L_T
#else
#  error "acc_intptr_t"
#endif
#endif
#if 1
    ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_intptr_t) >= sizeof(void *))
    ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_intptr_t) == sizeof(acc_uintptr_t))
#endif
#if !defined(acc_word_t)
#if defined(ACC_WORDSIZE) && (ACC_WORDSIZE+0 > 0)
#if (ACC_WORDSIZE == ACC_SIZEOF_ACC_INTPTR_T) && !defined(__ACC_INTPTR_T_IS_POINTER)
#  define acc_word_t                acc_uintptr_t
#  define acc_sword_t               acc_intptr_t
#  define ACC_SIZEOF_ACC_WORD_T ACC_SIZEOF_ACC_INTPTR_T
#elif (ACC_WORDSIZE == ACC_SIZEOF_LONG)
#  define acc_word_t                unsigned long
#  define acc_sword_t               long
#  define ACC_SIZEOF_ACC_WORD_T ACC_SIZEOF_LONG
#elif (ACC_WORDSIZE == ACC_SIZEOF_INT)
#  define acc_word_t                unsigned int
#  define acc_sword_t               int
#  define ACC_SIZEOF_ACC_WORD_T ACC_SIZEOF_INT
#elif (ACC_WORDSIZE == ACC_SIZEOF_SHORT)
#  define acc_word_t                unsigned short
#  define acc_sword_t               short
#  define ACC_SIZEOF_ACC_WORD_T ACC_SIZEOF_SHORT
#elif (ACC_WORDSIZE == 1)
#  define acc_word_t                unsigned char
#  define acc_sword_t               signed char
#  define ACC_SIZEOF_ACC_WORD_T 1
#elif (ACC_WORDSIZE == ACC_SIZEOF_ACC_INT64L_T)
#  define acc_word_t                acc_uint64l_t
#  define acc_sword_t               acc_int64l_t
#  define ACC_SIZEOF_ACC_WORD_T ACC_SIZEOF_ACC_INT64L_T
#elif (ACC_ARCH_SPU) && (ACC_CC_GNUC)
#if 0
   typedef unsigned acc_word_t  __attribute__((__mode__(__V16QI__)));
   typedef int      acc_sword_t __attribute__((__mode__(__V16QI__)));
#  define acc_word_t                acc_word_t
#  define acc_sword_t               acc_sword_t
#  define ACC_SIZEOF_ACC_WORD_T     16
#endif
#else
#  error "acc_word_t"
#endif
#endif
#endif
#if 1
#define acc_int8_t                  signed char
#define acc_uint8_t                 unsigned char
#define ACC_SIZEOF_ACC_INT8_T       1
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int8_t) == 1)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int8_t) == sizeof(acc_uint8_t))
#endif
#if defined(acc_int16e_t)
#define acc_int16_t                 acc_int16e_t
#define acc_uint16_t                acc_uint16e_t
#define ACC_SIZEOF_ACC_INT16_T      ACC_SIZEOF_ACC_INT16E_T
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int16_t) == 2)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int16_t) == sizeof(acc_uint16_t))
#endif
#if defined(acc_int32e_t)
#define acc_int32_t                 acc_int32e_t
#define acc_uint32_t                acc_uint32e_t
#define ACC_SIZEOF_ACC_INT32_T      ACC_SIZEOF_ACC_INT32E_T
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int32_t) == 4)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int32_t) == sizeof(acc_uint32_t))
#endif
#if defined(acc_int64e_t)
#define acc_int64_t                 acc_int64e_t
#define acc_uint64_t                acc_uint64e_t
#define ACC_SIZEOF_ACC_INT64_T      ACC_SIZEOF_ACC_INT64E_T
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int64_t) == 8)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int64_t) == sizeof(acc_uint64_t))
#endif
#if 1
#define acc_int_least32_t           acc_int32l_t
#define acc_uint_least32_t          acc_uint32l_t
#define ACC_SIZEOF_ACC_INT_LEAST32_T ACC_SIZEOF_ACC_INT32L_T
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int_least32_t) >= 4)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int_least32_t) == sizeof(acc_uint_least32_t))
#endif
#if defined(acc_int64l_t)
#define acc_int_least64_t           acc_int64l_t
#define acc_uint_least64_t          acc_uint64l_t
#define ACC_SIZEOF_ACC_INT_LEAST64_T ACC_SIZEOF_ACC_INT64L_T
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int_least64_t) >= 8)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int_least64_t) == sizeof(acc_uint_least64_t))
#endif
#if 1
#define acc_int_fast32_t           acc_int32f_t
#define acc_uint_fast32_t          acc_uint32f_t
#define ACC_SIZEOF_ACC_INT_FAST32_T ACC_SIZEOF_ACC_INT32F_T
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int_fast32_t) >= 4)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int_fast32_t) == sizeof(acc_uint_fast32_t))
#endif
#if defined(acc_int64f_t)
#define acc_int_fast64_t           acc_int64f_t
#define acc_uint_fast64_t          acc_uint64f_t
#define ACC_SIZEOF_ACC_INT_FAST64_T ACC_SIZEOF_ACC_INT64F_T
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int_fast64_t) >= 8)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(acc_int_fast64_t) == sizeof(acc_uint_fast64_t))
#endif
#if !defined(ACC_INT16_C)
#  if (ACC_BROKEN_INTEGRAL_CONSTANTS) && (ACC_SIZEOF_INT >= 2)
#    define ACC_INT16_C(c)          ((c) + 0)
#    define ACC_UINT16_C(c)         ((c) + 0U)
#  elif (ACC_BROKEN_INTEGRAL_CONSTANTS) && (ACC_SIZEOF_LONG >= 2)
#    define ACC_INT16_C(c)          ((c) + 0L)
#    define ACC_UINT16_C(c)         ((c) + 0UL)
#  elif (ACC_SIZEOF_INT >= 2)
#    define ACC_INT16_C(c)          c
#    define ACC_UINT16_C(c)         c##U
#  elif (ACC_SIZEOF_LONG >= 2)
#    define ACC_INT16_C(c)          c##L
#    define ACC_UINT16_C(c)         c##UL
#  else
#    error "ACC_INT16_C"
#  endif
#endif
#if !defined(ACC_INT32_C)
#  if (ACC_BROKEN_INTEGRAL_CONSTANTS) && (ACC_SIZEOF_INT >= 4)
#    define ACC_INT32_C(c)          ((c) + 0)
#    define ACC_UINT32_C(c)         ((c) + 0U)
#  elif (ACC_BROKEN_INTEGRAL_CONSTANTS) && (ACC_SIZEOF_LONG >= 4)
#    define ACC_INT32_C(c)          ((c) + 0L)
#    define ACC_UINT32_C(c)         ((c) + 0UL)
#  elif (ACC_SIZEOF_INT >= 4)
#    define ACC_INT32_C(c)          c
#    define ACC_UINT32_C(c)         c##U
#  elif (ACC_SIZEOF_LONG >= 4)
#    define ACC_INT32_C(c)          c##L
#    define ACC_UINT32_C(c)         c##UL
#  elif (ACC_SIZEOF_LONG_LONG >= 4)
#    define ACC_INT32_C(c)          c##LL
#    define ACC_UINT32_C(c)         c##ULL
#  else
#    error "ACC_INT32_C"
#  endif
#endif
#if !defined(ACC_INT64_C) && defined(acc_int64l_t)
#  if (ACC_BROKEN_INTEGRAL_CONSTANTS) && (ACC_SIZEOF_INT >= 8)
#    define ACC_INT64_C(c)          ((c) + 0)
#    define ACC_UINT64_C(c)         ((c) + 0U)
#  elif (ACC_BROKEN_INTEGRAL_CONSTANTS) && (ACC_SIZEOF_LONG >= 8)
#    define ACC_INT64_C(c)          ((c) + 0L)
#    define ACC_UINT64_C(c)         ((c) + 0UL)
#  elif (ACC_SIZEOF_INT >= 8)
#    define ACC_INT64_C(c)          c
#    define ACC_UINT64_C(c)         c##U
#  elif (ACC_SIZEOF_LONG >= 8)
#    define ACC_INT64_C(c)          c##L
#    define ACC_UINT64_C(c)         c##UL
#  else
#    error "ACC_INT64_C"
#  endif
#endif
#if 1 && !defined(acc_signo_t) && defined(__linux__) && defined(__dietlibc__) && (ACC_SIZEOF_INT != 4)
#  define acc_signo_t               acc_int32e_t
#endif
#if !defined(acc_signo_t)
#  define acc_signo_t               int
#endif
#if defined(__cplusplus)
extern "C" {
#endif
#if (ACC_BROKEN_CDECL_ALT_SYNTAX)
typedef void __acc_cdecl_sighandler (*acc_sighandler_t)(acc_signo_t);
#elif defined(RETSIGTYPE)
typedef RETSIGTYPE (__acc_cdecl_sighandler *acc_sighandler_t)(acc_signo_t);
#else
typedef void (__acc_cdecl_sighandler *acc_sighandler_t)(acc_signo_t);
#endif
#if defined(__cplusplus)
}
#endif
#  if !(ACC_CFG_SKIP_ACC_UA)
#if (ACC_CC_GNUC && (ACC_CC_GNUC < 0x020700ul))
#elif (ACC_CC_GNUC && (ACC_CC_GNUC < 0x020800ul)) && defined(__cplusplus)
#elif (ACC_CC_INTELC) && defined(_WIN32)
#elif (ACC_CC_INTELC && (__INTEL_COMPILER < 700))
#elif (ACC_CC_LLVM)
#elif (ACC_CC_GNUC || ACC_CC_INTELC || ACC_CC_PATHSCALE)
#if !defined(__acc_ua16_t) && (ACC_OPT_UNALIGNED16) && defined(acc_int16e_t)
   typedef struct { __acc_ua_volatile acc_uint16e_t v __attribute__((__packed__)); } __acc_ua16_t;
#  define __acc_ua16_t __acc_ua16_t
#endif
#if !defined(__acc_ua32_t) && (ACC_OPT_UNALIGNED32) && defined(acc_int32e_t)
   typedef struct { __acc_ua_volatile acc_uint32e_t v __attribute__((__packed__)); } __acc_ua32_t;
#  define __acc_ua32_t __acc_ua32_t
#endif
#if !defined(__acc_ua64_t) && (ACC_OPT_UNALIGNED64) && defined(acc_int64l_t)
   typedef struct { __acc_ua_volatile acc_uint64l_t v __attribute__((__packed__)); } __acc_ua64_t;
#  define __acc_ua64_t __acc_ua64_t
#endif
#endif
#if (ACC_OPT_UNALIGNED16) && defined(acc_int16e_t)
#define ACC_UA_GET16(p)         (* (__acc_ua_volatile const acc_uint16e_t*) (__acc_ua_volatile const void*) (p))
#define ACC_UA_SET16(p,v)       ((* (__acc_ua_volatile acc_uint16e_t*) (__acc_ua_volatile void*) (p)) = (acc_uint16e_t) (v))
#if (ACC_ABI_BIG_ENDIAN)
#  define ACC_UA_GET_BE16(p)    ACC_UA_GET16(p)
#  define ACC_UA_SET_BE16(p,v)  ACC_UA_SET16(p,v)
#elif (ACC_ABI_LITTLE_ENDIAN)
#  define ACC_UA_GET_LE16(p)    ACC_UA_GET16(p)
#  define ACC_UA_SET_LE16(p,v)  ACC_UA_SET16(p,v)
#endif
#if (__acc_HAVE_forceinline)
#if (ACC_ARCH_POWERPC && ACC_ABI_BIG_ENDIAN) && (ACC_ASM_SYNTAX_GNUC) && (ACC_CC_GNUC)
#if !defined(ACC_UA_GET_LE16)
static __acc_forceinline
#if (ACC_CC_CLANG || (ACC_CC_GNUC >= 0x020800ul) || ACC_CC_INTELC_GNUC || ACC_CC_LLVM)
__attribute__((__unused__))
#endif
unsigned long __ACC_UA_GET_LE16(__acc_ua_volatile const void* pp) {
    __acc_ua_volatile const acc_uint16e_t* p = (__acc_ua_volatile const acc_uint16e_t*) pp;
    unsigned long v;
    __asm__ __volatile__("lhbrx %0,0,%1" : "=r" (v) : "r" (p), "m" (*p));
    return v;
}
#define ACC_UA_GET_LE16(p)      __ACC_UA_GET_LE16(p)
#endif
#if !defined(ACC_UA_SET_LE16)
static __acc_forceinline
#if (ACC_CC_CLANG || (ACC_CC_GNUC >= 0x020800ul) || ACC_CC_INTELC_GNUC || ACC_CC_LLVM)
__attribute__((__unused__))
#endif
void __ACC_UA_SET_LE16(__acc_ua_volatile void* pp, unsigned long v) {
    __acc_ua_volatile acc_uint16e_t* p = (__acc_ua_volatile acc_uint16e_t*) pp;
    __asm__ __volatile__("sthbrx %2,0,%1" : "=m" (*p) : "r" (p), "r" (v));
}
#define ACC_UA_SET_LE16(p,v)    __ACC_UA_SET_LE16(p,v)
#endif
#endif
#endif
#if !defined(ACC_UA_COPY16)
#  define ACC_UA_COPY16(d,s)    ACC_UA_SET16(d, ACC_UA_GET16(s))
#endif
#endif
#if (ACC_OPT_UNALIGNED32) && defined(acc_int32e_t)
#define ACC_UA_GET32(p)         (* (__acc_ua_volatile const acc_uint32e_t*) (__acc_ua_volatile const void*) (p))
#define ACC_UA_SET32(p,v)       ((* (__acc_ua_volatile acc_uint32e_t*) (__acc_ua_volatile void*) (p)) = (acc_uint32e_t) (v))
#if (ACC_ABI_BIG_ENDIAN)
#  define ACC_UA_GET_BE32(p)    ACC_UA_GET32(p)
#  define ACC_UA_SET_BE32(p,v)  ACC_UA_SET32(p,v)
#elif (ACC_ABI_LITTLE_ENDIAN)
#  define ACC_UA_GET_LE32(p)    ACC_UA_GET32(p)
#  define ACC_UA_SET_LE32(p,v)  ACC_UA_SET32(p,v)
#endif
#if (__acc_HAVE_forceinline)
#if (ACC_ARCH_POWERPC && ACC_ABI_BIG_ENDIAN) && (ACC_ASM_SYNTAX_GNUC) && (ACC_CC_GNUC)
#if !defined(ACC_UA_GET_LE32)
static __acc_forceinline
#if (ACC_CC_CLANG || (ACC_CC_GNUC >= 0x020800ul) || ACC_CC_INTELC_GNUC || ACC_CC_LLVM)
__attribute__((__unused__))
#endif
unsigned long __ACC_UA_GET_LE32(__acc_ua_volatile const void* pp) {
    __acc_ua_volatile const acc_uint32e_t* p = (__acc_ua_volatile const acc_uint32e_t*) pp;
    unsigned long v;
    __asm__ __volatile__("lwbrx %0,0,%1" : "=r" (v) : "r" (p), "m" (*p));
    return v;
}
#define ACC_UA_GET_LE32(p)      __ACC_UA_GET_LE32(p)
#endif
#if !defined(ACC_UA_SET_LE32)
static __acc_forceinline
#if (ACC_CC_CLANG || (ACC_CC_GNUC >= 0x020800ul) || ACC_CC_INTELC_GNUC || ACC_CC_LLVM)
__attribute__((__unused__))
#endif
void __ACC_UA_SET_LE32(__acc_ua_volatile void* pp, unsigned long v) {
    __acc_ua_volatile acc_uint32e_t* p = (__acc_ua_volatile acc_uint32e_t*) pp;
    __asm__ __volatile__("stwbrx %2,0,%1" : "=m" (*p) : "r" (p), "r" (v));
}
#define ACC_UA_SET_LE32(p,v)    __ACC_UA_SET_LE32(p,v)
#endif
#endif
#endif
#if !defined(ACC_UA_COPY32)
#  define ACC_UA_COPY32(d,s)    ACC_UA_SET32(d, ACC_UA_GET32(s))
#endif
#endif
#if (ACC_OPT_UNALIGNED64) && defined(acc_int64l_t)
#define ACC_UA_GET64(p)         (* (__acc_ua_volatile const acc_uint64l_t*) (__acc_ua_volatile const void*) (p))
#define ACC_UA_SET64(p,v)       ((* (__acc_ua_volatile acc_uint64l_t*) (__acc_ua_volatile void*) (p)) = (acc_uint64l_t) (v))
#if (ACC_ABI_BIG_ENDIAN)
#  define ACC_UA_GET_BE64(p)    ACC_UA_GET64(p)
#  define ACC_UA_SET_BE64(p,v)  ACC_UA_SET64(p,v)
#elif (ACC_ABI_LITTLE_ENDIAN)
#  define ACC_UA_GET_LE64(p)    ACC_UA_GET64(p)
#  define ACC_UA_SET_LE64(p,v)  ACC_UA_SET64(p,v)
#endif
#if !defined(ACC_UA_COPY64)
#  define ACC_UA_COPY64(d,s)    ACC_UA_SET64(d, ACC_UA_GET64(s))
#endif
#endif
#  endif
#endif
#if defined(ACC_WANT_ACC_INCD_H)
#  undef ACC_WANT_ACC_INCD_H
#ifndef __ACC_INCD_H_INCLUDED
#define __ACC_INCD_H_INCLUDED 1
#if (ACC_LIBC_NAKED)
#ifndef __ACC_FALLBACK_STDDEF_H_INCLUDED
#define __ACC_FALLBACK_STDDEF_H_INCLUDED 1
#if defined(__PTRDIFF_TYPE__)
typedef __PTRDIFF_TYPE__ acc_fallback_ptrdiff_t;
#elif defined(__MIPS_PSX2__)
typedef int acc_fallback_ptrdiff_t;
#else
typedef long acc_fallback_ptrdiff_t;
#endif
#if defined(__SIZE_TYPE__)
typedef __SIZE_TYPE__ acc_fallback_size_t;
#elif defined(__MIPS_PSX2__)
typedef unsigned int acc_fallback_size_t;
#else
typedef unsigned long acc_fallback_size_t;
#endif
#if !defined(ptrdiff_t)
typedef acc_fallback_ptrdiff_t ptrdiff_t;
#ifndef _PTRDIFF_T_DEFINED
#define _PTRDIFF_T_DEFINED 1
#endif
#endif
#if !defined(size_t)
typedef acc_fallback_size_t size_t;
#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED 1
#endif
#endif
#if !defined(__cplusplus) && !defined(wchar_t)
typedef unsigned short wchar_t;
#ifndef _WCHAR_T_DEFINED
#define _WCHAR_T_DEFINED 1
#endif
#endif
#ifndef NULL
#if defined(__cplusplus) && defined(__GNUC__) && (__GNUC__ >= 4)
#define NULL    __null
#elif defined(__cplusplus)
#define NULL    0
#else
#define NULL    ((void*)0)
#endif
#endif
#ifndef offsetof
#define offsetof(s,m)   ((size_t)((ptrdiff_t)&(((s*)0)->m)))
#endif
#endif
#elif (ACC_LIBC_FREESTANDING)
# if defined(HAVE_STDDEF_H) && (HAVE_STDDEF_H+0)
#  include <stddef.h>
# endif
# if defined(HAVE_STDINT_H) && (HAVE_STDINT_H+0)
#  include <stdint.h>
# endif
#elif (ACC_LIBC_MOSTLY_FREESTANDING)
# if defined(HAVE_STDIO_H) && (HAVE_STDIO_H+0)
#  include <stdio.h>
# endif
# if defined(HAVE_STDDEF_H) && (HAVE_STDDEF_H+0)
#  include <stddef.h>
# endif
# if defined(HAVE_STDINT_H) && (HAVE_STDINT_H+0)
#  include <stdint.h>
# endif
#else
#include <stdio.h>
#if defined(HAVE_TIME_H) && (HAVE_TIME_H+0) && defined(__MSL__) && defined(__cplusplus)
# include <time.h>
#endif
#if defined(HAVE_SYS_TYPES_H) && (HAVE_SYS_TYPES_H+0)
# include <sys/types.h>
#endif
#if defined(HAVE_SYS_STAT_H) && (HAVE_SYS_STAT_H+0)
# include <sys/stat.h>
#endif
#if defined(STDC_HEADERS) && (STDC_HEADERS+0)
# include <stdlib.h>
#elif defined(HAVE_STDLIB_H) && (HAVE_STDLIB_H+0)
# include <stdlib.h>
#endif
#include <stddef.h>
#if defined(HAVE_STRING_H) && (HAVE_STRING_H+0)
# if defined(STDC_HEADERS) && (STDC_HEADERS+0)
# elif defined(HAVE_MEMORY_H) && (HAVE_MEMORY_H+0)
#  include <memory.h>
# endif
# include <string.h>
#endif
#if defined(HAVE_STRINGS_H) && (HAVE_STRINGS_H+0)
# include <strings.h>
#endif
#if defined(HAVE_INTTYPES_H) && (HAVE_INTTYPES_H+0)
# include <inttypes.h>
#endif
#if defined(HAVE_STDINT_H) && (HAVE_STDINT_H+0)
# include <stdint.h>
#endif
#if defined(HAVE_UNISTD_H) && (HAVE_UNISTD_H+0)
# include <unistd.h>
#endif
#endif
#endif
#endif
#if defined(ACC_WANT_ACC_INCE_H)
#  undef ACC_WANT_ACC_INCE_H
#ifndef __ACC_INCE_H_INCLUDED
#define __ACC_INCE_H_INCLUDED 1
#if (ACC_LIBC_NAKED)
#elif (ACC_LIBC_FREESTANDING)
#elif (ACC_LIBC_MOSTLY_FREESTANDING)
#  if (HAVE_SETJMP_H)
#    include <setjmp.h>
#  endif
#else
#if (HAVE_STDARG_H)
#  include <stdarg.h>
#endif
#if (HAVE_CTYPE_H)
#  include <ctype.h>
#endif
#if (HAVE_ERRNO_H)
#  include <errno.h>
#endif
#if (HAVE_MALLOC_H)
#  include <malloc.h>
#endif
#if (HAVE_ALLOCA_H)
#  include <alloca.h>
#endif
#if (HAVE_FCNTL_H)
#  include <fcntl.h>
#endif
#if (HAVE_DIRENT_H)
#  include <dirent.h>
#endif
#if (HAVE_SETJMP_H)
#  include <setjmp.h>
#endif
#if (HAVE_SIGNAL_H)
#  include <signal.h>
#endif
#if (TIME_WITH_SYS_TIME)
#  include <sys/time.h>
#  include <time.h>
#elif (HAVE_TIME_H)
#  include <time.h>
#endif
#if (HAVE_UTIME_H)
#  include <utime.h>
#elif (HAVE_SYS_UTIME_H)
#  include <sys/utime.h>
#endif
#if (HAVE_IO_H)
#  include <io.h>
#endif
#if (HAVE_DOS_H)
#  include <dos.h>
#endif
#if (HAVE_DIRECT_H)
#  include <direct.h>
#endif
#if (HAVE_SHARE_H)
#  include <share.h>
#endif
#if (ACC_CC_NDPC)
#  include <os.h>
#endif
#if defined(__TOS__) && (defined(__PUREC__) || defined(__TURBOC__))
#  include <ext.h>
#endif
#endif
#endif
#endif
#if defined(ACC_WANT_ACC_INCI_H)
#  undef ACC_WANT_ACC_INCI_H
#ifndef __ACC_INCI_H_INCLUDED
#define __ACC_INCI_H_INCLUDED 1
#if (ACC_LIBC_NAKED)
#elif (ACC_LIBC_FREESTANDING)
#elif (ACC_LIBC_MOSTLY_FREESTANDING)
#else
#if (ACC_OS_TOS && (ACC_CC_PUREC || ACC_CC_TURBOC))
#  include <tos.h>
#elif (ACC_HAVE_WINDOWS_H)
#  if 1 && !defined(WIN32_LEAN_AND_MEAN)
#    define WIN32_LEAN_AND_MEAN 1
#  endif
#  if 1 && !defined(_WIN32_WINNT)
#    define _WIN32_WINNT 0x0400
#  endif
#  include <windows.h>
#  if (ACC_CC_BORLANDC || ACC_CC_TURBOC)
#    include <dir.h>
#  endif
#elif (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_WIN16)
#  if (ACC_CC_AZTECC)
#    include <model.h>
#    include <stat.h>
#  elif (ACC_CC_BORLANDC || ACC_CC_TURBOC)
#    include <alloc.h>
#    include <dir.h>
#  elif (ACC_OS_DOS32 && ACC_CC_GNUC) && defined(__DJGPP__)
#    include <sys/exceptn.h>
#  elif (ACC_CC_PACIFICC)
#    include <unixio.h>
#    include <stat.h>
#    include <sys.h>
#  elif (ACC_CC_WATCOMC)
#    include <i86.h>
#  endif
#elif (ACC_OS_OS216)
#  if (ACC_CC_WATCOMC)
#    include <i86.h>
#  endif
#endif
#if (HAVE_SYS_MMAN_H)
#  include <sys/mman.h>
#endif
#if (HAVE_SYS_RESOURCE_H)
#  include <sys/resource.h>
#endif
#if (ACC_OS_DOS16 || ACC_OS_OS216 || ACC_OS_WIN16)
#  if defined(FP_OFF)
#    define ACC_PTR_FP_OFF(x)   FP_OFF(x)
#  elif defined(_FP_OFF)
#    define ACC_PTR_FP_OFF(x)   _FP_OFF(x)
#  else
#    define ACC_PTR_FP_OFF(x)   (((const unsigned __far*)&(x))[0])
#  endif
#  if defined(FP_SEG)
#    define ACC_PTR_FP_SEG(x)   FP_SEG(x)
#  elif defined(_FP_SEG)
#    define ACC_PTR_FP_SEG(x)   _FP_SEG(x)
#  else
#    define ACC_PTR_FP_SEG(x)   (((const unsigned __far*)&(x))[1])
#  endif
#  if defined(MK_FP)
#    define ACC_PTR_MK_FP(s,o)  MK_FP(s,o)
#  elif defined(_MK_FP)
#    define ACC_PTR_MK_FP(s,o)  _MK_FP(s,o)
#  else
#    define ACC_PTR_MK_FP(s,o)  ((void __far*)(((unsigned long)(s)<<16)+(unsigned)(o)))
#  endif
#  if 0
#    undef ACC_PTR_FP_OFF
#    undef ACC_PTR_FP_SEG
#    undef ACC_PTR_MK_FP
#    define ACC_PTR_FP_OFF(x)   (((const unsigned __far*)&(x))[0])
#    define ACC_PTR_FP_SEG(x)   (((const unsigned __far*)&(x))[1])
#    define ACC_PTR_MK_FP(s,o)  ((void __far*)(((unsigned long)(s)<<16)+(unsigned)(o)))
#  endif
#endif
#endif
#endif
#endif
#if defined(ACC_WANT_ACC_LIB_H)
#  undef ACC_WANT_ACC_LIB_H
#ifndef __ACC_LIB_H_INCLUDED
#define __ACC_LIB_H_INCLUDED 1
#if !defined(__ACCLIB_FUNCNAME)
#  define __ACCLIB_FUNCNAME(f)  f
#endif
#if !defined(ACCLIB_EXTERN)
#  define ACCLIB_EXTERN(r,f)                extern r __ACCLIB_FUNCNAME(f)
#endif
#if !defined(ACCLIB_EXTERN_NOINLINE)
#  if defined(__acc_noinline)
#    define ACCLIB_EXTERN_NOINLINE(r,f)     extern __acc_noinline r __ACCLIB_FUNCNAME(f)
#  else
#    define ACCLIB_EXTERN_NOINLINE(r,f)     extern r __ACCLIB_FUNCNAME(f)
#  endif
#endif
#if !defined(__ACCLIB_CONST_CAST_RETURN)
#if 1 && (ACC_CC_CLANG || ACC_CC_GNUC || ACC_CC_LLVM || ACC_CC_PATHSCALE)
#  define __ACCLIB_CONST_CAST_RETURN(type,var) return (type) (acc_uintptr_t) (var);
#elif (ACC_CC_CLANG || ACC_CC_GNUC || ACC_CC_LLVM || ACC_CC_PATHSCALE)
#  define __ACCLIB_CONST_CAST_RETURN(type,var) \
        { union { type a; const type b; } u; u.b = (var); return u.a; }
#else
#  define __ACCLIB_CONST_CAST_RETURN(type,var) return (type) (var);
#endif
#endif
#if (ACC_OS_WIN64)
#  define acclib_handle_t       acc_int64l_t
#  define acclib_uhandle_t      acc_uint64l_t
#elif (ACC_ARCH_I386 && ACC_CC_MSC && (_MSC_VER >= 1300))
   typedef __w64 long           acclib_handle_t;
   typedef __w64 unsigned long  acclib_uhandle_t;
#  define acclib_handle_t       acclib_handle_t
#  define acclib_uhandle_t      acclib_uhandle_t
#else
#  define acclib_handle_t       long
#  define acclib_uhandle_t      unsigned long
#endif
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
#ifndef acc_hsize_t
#if (ACC_HAVE_MM_HUGE_PTR)
#  define acc_hsize_t   unsigned long
#  define acc_hvoid_p   void __huge *
#  define acc_hchar_p   char __huge *
#  define acc_hchar_pp  char __huge * __huge *
#  define acc_hbyte_p   unsigned char __huge *
#else
#  define acc_hsize_t   size_t
#  define acc_hvoid_p   void *
#  define acc_hchar_p   char *
#  define acc_hchar_pp  char **
#  define acc_hbyte_p   unsigned char *
#endif
#endif
ACCLIB_EXTERN(acc_hvoid_p, acc_halloc) (acc_hsize_t);
ACCLIB_EXTERN(void, acc_hfree) (acc_hvoid_p);
#if (ACC_OS_DOS16 || ACC_OS_OS216)
ACCLIB_EXTERN(void __far*, acc_dos_alloc) (unsigned long);
ACCLIB_EXTERN(int, acc_dos_free) (void __far*);
#endif
ACCLIB_EXTERN(int, acc_hmemcmp) (const acc_hvoid_p, const acc_hvoid_p, acc_hsize_t);
ACCLIB_EXTERN(acc_hvoid_p, acc_hmemcpy) (acc_hvoid_p, const acc_hvoid_p, acc_hsize_t);
ACCLIB_EXTERN(acc_hvoid_p, acc_hmemmove) (acc_hvoid_p, const acc_hvoid_p, acc_hsize_t);
ACCLIB_EXTERN(acc_hvoid_p, acc_hmemset) (acc_hvoid_p, int, acc_hsize_t);
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
ACCLIB_EXTERN(acc_hsize_t, acc_hfread) (void *, acc_hvoid_p, acc_hsize_t);
ACCLIB_EXTERN(acc_hsize_t, acc_hfwrite) (void *, const acc_hvoid_p, acc_hsize_t);
#if (ACC_HAVE_MM_HUGE_PTR)
ACCLIB_EXTERN(long, acc_hread) (int, acc_hvoid_p, long);
ACCLIB_EXTERN(long, acc_hwrite) (int, const acc_hvoid_p, long);
#endif
ACCLIB_EXTERN(long, acc_safe_hread) (int, acc_hvoid_p, long);
ACCLIB_EXTERN(long, acc_safe_hwrite) (int, const acc_hvoid_p, long);
ACCLIB_EXTERN(unsigned, acc_ua_get_be16) (const acc_hvoid_p);
ACCLIB_EXTERN(acc_uint32l_t, acc_ua_get_be24) (const acc_hvoid_p);
ACCLIB_EXTERN(acc_uint32l_t, acc_ua_get_be32) (const acc_hvoid_p);
ACCLIB_EXTERN(void, acc_ua_set_be16) (acc_hvoid_p, unsigned);
ACCLIB_EXTERN(void, acc_ua_set_be24) (acc_hvoid_p, acc_uint32l_t);
ACCLIB_EXTERN(void, acc_ua_set_be32) (acc_hvoid_p, acc_uint32l_t);
ACCLIB_EXTERN(unsigned, acc_ua_get_le16) (const acc_hvoid_p);
ACCLIB_EXTERN(acc_uint32l_t, acc_ua_get_le24) (const acc_hvoid_p);
ACCLIB_EXTERN(acc_uint32l_t, acc_ua_get_le32) (const acc_hvoid_p);
ACCLIB_EXTERN(void, acc_ua_set_le16) (acc_hvoid_p, unsigned);
ACCLIB_EXTERN(void, acc_ua_set_le24) (acc_hvoid_p, acc_uint32l_t);
ACCLIB_EXTERN(void, acc_ua_set_le32) (acc_hvoid_p, acc_uint32l_t);
#if defined(acc_int64l_t)
ACCLIB_EXTERN(acc_uint64l_t, acc_ua_get_be64) (const acc_hvoid_p);
ACCLIB_EXTERN(void, acc_ua_set_be64) (acc_hvoid_p, acc_uint64l_t);
ACCLIB_EXTERN(acc_uint64l_t, acc_ua_get_le64) (const acc_hvoid_p);
ACCLIB_EXTERN(void, acc_ua_set_le64) (acc_hvoid_p, acc_uint64l_t);
#endif
ACCLIB_EXTERN_NOINLINE(short, acc_vget_short) (short, int);
ACCLIB_EXTERN_NOINLINE(int, acc_vget_int) (int, int);
ACCLIB_EXTERN_NOINLINE(long, acc_vget_long) (long, int);
#if defined(acc_int64l_t)
ACCLIB_EXTERN_NOINLINE(acc_int64l_t, acc_vget_acc_int64l_t) (acc_int64l_t, int);
#endif
ACCLIB_EXTERN_NOINLINE(acc_hsize_t, acc_vget_acc_hsize_t) (acc_hsize_t, int);
#if !(ACC_CFG_NO_FLOAT)
ACCLIB_EXTERN_NOINLINE(float, acc_vget_float) (float, int);
#endif
#if !(ACC_CFG_NO_DOUBLE)
ACCLIB_EXTERN_NOINLINE(double, acc_vget_double) (double, int);
#endif
ACCLIB_EXTERN_NOINLINE(acc_hvoid_p, acc_vget_acc_hvoid_p) (acc_hvoid_p, int);
ACCLIB_EXTERN_NOINLINE(const acc_hvoid_p, acc_vget_acc_hvoid_cp) (const acc_hvoid_p, int);
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
#  define ACC_FN_PATH_MAX   1023
#endif
#if !defined(ACC_FN_NAME_MAX)
#if (ACC_OS_DOS16 || ACC_OS_WIN16)
#  define ACC_FN_NAME_MAX   12
#elif (ACC_ARCH_M68K && ACC_OS_TOS && (ACC_CC_PUREC || ACC_CC_TURBOC))
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
#undef __ACCLIB_USE_OPENDIR
#if (HAVE_DIRENT_H || ACC_CC_WATCOMC)
#  define __ACCLIB_USE_OPENDIR 1
#  if (ACC_OS_DOS32 && defined(__BORLANDC__))
#  elif (ACC_OS_DOS32 && ACC_CC_GNUC) && defined(__DJGPP__)
#  elif (ACC_OS_OS2 || ACC_OS_OS216)
#  elif (ACC_ARCH_M68K && ACC_OS_TOS && ACC_CC_GNUC)
#  elif (ACC_OS_WIN32 && !(ACC_HAVE_WINDOWS_H))
#  elif (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_TOS || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)
#    undef __ACCLIB_USE_OPENDIR
#  endif
#endif
typedef struct
{
#if defined(__ACCLIB_USE_OPENDIR)
    void* u_dirp;
# if (ACC_CC_WATCOMC)
    unsigned short f_time;
    unsigned short f_date;
    unsigned long f_size;
# endif
    char f_name[ACC_FN_NAME_MAX+1];
#elif (ACC_OS_WIN32 || ACC_OS_WIN64)
    acclib_handle_t u_handle;
    unsigned f_attr;
    unsigned f_size_low;
    unsigned f_size_high;
    char f_name[ACC_FN_NAME_MAX+1];
#elif (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_TOS || ACC_OS_WIN16)
    char u_dta[21];
    unsigned char f_attr;
    unsigned short f_time;
    unsigned short f_date;
    unsigned short f_size_low;
    unsigned short f_size_high;
    char f_name[ACC_FN_NAME_MAX+1];
    char u_dirp;
#else
    void* u_dirp;
    char f_name[ACC_FN_NAME_MAX+1];
#endif
} acc_dir_t;
#ifndef acc_dir_p
#define acc_dir_p acc_dir_t *
#endif
ACCLIB_EXTERN(int, acc_opendir)  (acc_dir_p, const char*);
ACCLIB_EXTERN(int, acc_readdir)  (acc_dir_p);
ACCLIB_EXTERN(int, acc_closedir) (acc_dir_p);
#if (ACC_CC_GNUC) && (defined(__CYGWIN__) || defined(__MINGW32__))
#  define acc_alloca(x)     __builtin_alloca((x))
#elif (ACC_CC_GNUC) && (ACC_OS_CONSOLE_PS2)
#  define acc_alloca(x)     __builtin_alloca((x))
#elif (ACC_CC_BORLANDC || ACC_CC_LCC) && defined(__linux__)
#elif (HAVE_ALLOCA)
#  define acc_alloca(x)     ((void *) (alloca((x))))
#endif
#if (ACC_OS_DOS32 && ACC_CC_GNUC) && defined(__DJGPP__)
#  define acc_stackavail()  stackavail()
#elif (ACC_ARCH_I086 && ACC_CC_BORLANDC && (__BORLANDC__ >= 0x0410))
#  define acc_stackavail()  stackavail()
#elif (ACC_ARCH_I086 && ACC_CC_BORLANDC && (__BORLANDC__ >= 0x0400))
#  if (ACC_OS_WIN16) && (ACC_MM_TINY || ACC_MM_SMALL || ACC_MM_MEDIUM)
#  else
#    define acc_stackavail()  stackavail()
#  endif
#elif ((ACC_ARCH_I086 || ACC_ARCH_I386) && (ACC_CC_DMC || ACC_CC_SYMANTECC))
#  define acc_stackavail()  stackavail()
#elif ((ACC_ARCH_I086) && ACC_CC_MSC && (_MSC_VER >= 700))
#  define acc_stackavail()  _stackavail()
#elif ((ACC_ARCH_I086) && ACC_CC_MSC)
#  define acc_stackavail()  stackavail()
#elif ((ACC_ARCH_I086 || ACC_ARCH_I386) && ACC_CC_TURBOC && (__TURBOC__ >= 0x0450))
#  define acc_stackavail()  stackavail()
#elif (ACC_ARCH_I086 && ACC_CC_TURBOC && (__TURBOC__ >= 0x0400))
   ACC_EXTERN_C size_t __cdecl stackavail(void);
#  define acc_stackavail()  stackavail()
#elif ((ACC_ARCH_I086 || ACC_ARCH_I386) && (ACC_CC_WATCOMC))
#  define acc_stackavail()  stackavail()
#elif (ACC_ARCH_I086 && ACC_CC_ZORTECHC)
#  define acc_stackavail()  _chkstack()
#endif
ACCLIB_EXTERN(acclib_handle_t, acc_get_osfhandle) (int);
ACCLIB_EXTERN(const char *, acc_getenv) (const char *);
ACCLIB_EXTERN(int, acc_isatty) (int);
ACCLIB_EXTERN(int, acc_mkdir) (const char*, unsigned);
ACCLIB_EXTERN(int, acc_rmdir) (const char*);
ACCLIB_EXTERN(int, acc_response) (int*, char***);
ACCLIB_EXTERN(int, acc_set_binmode) (int, int);
#if defined(acc_int32e_t)
ACCLIB_EXTERN(acc_int32e_t, acc_muldiv32s) (acc_int32e_t, acc_int32e_t, acc_int32e_t);
ACCLIB_EXTERN(acc_uint32e_t, acc_muldiv32u) (acc_uint32e_t, acc_uint32e_t, acc_uint32e_t);
#endif
ACCLIB_EXTERN(void, acc_wildargv) (int*, char***);
ACCLIB_EXTERN_NOINLINE(void, acc_debug_break) (void);
ACCLIB_EXTERN_NOINLINE(void, acc_debug_nop) (void);
ACCLIB_EXTERN_NOINLINE(int, acc_debug_align_check_query) (void);
ACCLIB_EXTERN_NOINLINE(int, acc_debug_align_check_enable) (int);
ACCLIB_EXTERN_NOINLINE(unsigned, acc_debug_running_on_qemu) (void);
ACCLIB_EXTERN_NOINLINE(unsigned, acc_debug_running_on_valgrind) (void);
#if defined(acc_int32e_t)
ACCLIB_EXTERN(int, acc_tsc_read) (acc_uint32e_t*);
#endif
struct acc_pclock_handle_t;
struct acc_pclock_t;
typedef struct acc_pclock_handle_t acc_pclock_handle_t;
typedef struct acc_pclock_t acc_pclock_t;
#ifndef acc_pclock_handle_p
#define acc_pclock_handle_p acc_pclock_handle_t *
#endif
#ifndef acc_pclock_p
#define acc_pclock_p acc_pclock_t *
#endif
#define ACC_PCLOCK_REALTIME             0
#define ACC_PCLOCK_MONOTONIC            1
#define ACC_PCLOCK_PROCESS_CPUTIME_ID   2
#define ACC_PCLOCK_THREAD_CPUTIME_ID    3
struct acc_pclock_handle_t {
    acclib_handle_t h;
    int mode;
    int read_error;
    const char* name;
    int (*gettime) (acc_pclock_handle_p, acc_pclock_p);
#if defined(acc_int64l_t)
    acc_uint64l_t ticks_base;
#endif
};
struct acc_pclock_t {
#if defined(acc_int64l_t)
    acc_int64l_t tv_sec;
#else
    acc_int32l_t tv_sec_high;
    acc_uint32l_t tv_sec_low;
#endif
    acc_uint32l_t tv_nsec;
};
ACCLIB_EXTERN(int, acc_pclock_open)  (acc_pclock_handle_p, int);
ACCLIB_EXTERN(int, acc_pclock_open_default) (acc_pclock_handle_p);
ACCLIB_EXTERN(int, acc_pclock_close) (acc_pclock_handle_p);
ACCLIB_EXTERN(void, acc_pclock_read) (acc_pclock_handle_p, acc_pclock_p);
#if !(ACC_CFG_NO_DOUBLE)
ACCLIB_EXTERN(double, acc_pclock_get_elapsed) (acc_pclock_handle_p, const acc_pclock_p, const acc_pclock_p);
#endif
ACCLIB_EXTERN(int, acc_pclock_flush_cpu_cache) (acc_pclock_handle_p, unsigned);
struct acc_getopt_t;
typedef struct acc_getopt_t acc_getopt_t;
#ifndef acc_getopt_p
#define acc_getopt_p acc_getopt_t *
#endif
struct acc_getopt_longopt_t;
typedef struct acc_getopt_longopt_t acc_getopt_longopt_t;
#ifndef acc_getopt_longopt_p
#define acc_getopt_longopt_p acc_getopt_longopt_t *
#endif
struct acc_getopt_longopt_t {
    const char* name;
    int has_arg;
    int* flag;
    int val;
};
struct acc_getopt_t {
    void *user;
    const char *progname;
    int bad_option;
    char *optarg;
    void (*opterr)(acc_getopt_p, const char*, void *);
    int optind;
    int optopt;
    int errcount;
    int argc; char** argv;
    int eof; int shortpos;
    int pending_rotate_first, pending_rotate_middle;
};
enum { ACC_GETOPT_NO_ARG, ACC_GETOPT_REQUIRED_ARG, ACC_GETOPT_OPTIONAL_ARG, ACC_GETOPT_EXACT_ARG = 0x10 };
enum { ACC_GETOPT_PERMUTE, ACC_GETOPT_RETURN_IN_ORDER, ACC_GETOPT_REQUIRE_ORDER };
ACCLIB_EXTERN(void, acc_getopt_init) (acc_getopt_p g,
                                      int start_argc, int argc, char** argv);
ACCLIB_EXTERN(int, acc_getopt) (acc_getopt_p g,
                                const char* shortopts,
                                const acc_getopt_longopt_p longopts,
                                int* longind);
typedef struct {
    acc_uint32l_t seed;
} acc_rand31_t;
#ifndef acc_rand31_p
#define acc_rand31_p acc_rand31_t *
#endif
ACCLIB_EXTERN(void, acc_srand31) (acc_rand31_p, acc_uint32l_t);
ACCLIB_EXTERN(acc_uint32l_t, acc_rand31) (acc_rand31_p);
#if defined(acc_int64l_t)
typedef struct {
    acc_uint64l_t seed;
} acc_rand48_t;
#ifndef acc_rand48_p
#define acc_rand48_p acc_rand48_t *
#endif
ACCLIB_EXTERN(void, acc_srand48) (acc_rand48_p, acc_uint32l_t);
ACCLIB_EXTERN(acc_uint32l_t, acc_rand48) (acc_rand48_p);
ACCLIB_EXTERN(acc_uint32l_t, acc_rand48_r32) (acc_rand48_p);
#endif
#if defined(acc_int64l_t)
typedef struct {
    acc_uint64l_t seed;
} acc_rand64_t;
#ifndef acc_rand64_p
#define acc_rand64_p acc_rand64_t *
#endif
ACCLIB_EXTERN(void, acc_srand64) (acc_rand64_p, acc_uint64l_t);
ACCLIB_EXTERN(acc_uint32l_t, acc_rand64) (acc_rand64_p);
ACCLIB_EXTERN(acc_uint32l_t, acc_rand64_r32) (acc_rand64_p);
#endif
typedef struct {
    unsigned n;
    acc_uint32l_t s[624];
} acc_randmt_t;
#ifndef acc_randmt_p
#define acc_randmt_p acc_randmt_t *
#endif
ACCLIB_EXTERN(void, acc_srandmt) (acc_randmt_p, acc_uint32l_t);
ACCLIB_EXTERN(acc_uint32l_t, acc_randmt) (acc_randmt_p);
ACCLIB_EXTERN(acc_uint32l_t, acc_randmt_r32) (acc_randmt_p);
#if defined(acc_int64l_t)
typedef struct {
    unsigned n;
    acc_uint64l_t s[312];
} acc_randmt64_t;
#ifndef acc_randmt64_p
#define acc_randmt64_p acc_randmt64_t *
#endif
ACCLIB_EXTERN(void, acc_srandmt64) (acc_randmt64_p, acc_uint64l_t);
ACCLIB_EXTERN(acc_uint64l_t, acc_randmt64_r64) (acc_randmt64_p);
#endif
#define ACC_SPAWN_P_WAIT    0
#define ACC_SPAWN_P_NOWAIT  1
ACCLIB_EXTERN(int, acc_spawnv)  (int mode, const char* fn, const char* const * argv);
ACCLIB_EXTERN(int, acc_spawnvp) (int mode, const char* fn, const char* const * argv);
ACCLIB_EXTERN(int, acc_spawnve) (int mode, const char* fn, const char* const * argv, const char * const envp);
#endif
#endif
#if defined(ACC_WANT_ACC_CXX_H)
#  undef ACC_WANT_ACC_CXX_H
#ifndef __ACC_CXX_H_INCLUDED
#define __ACC_CXX_H_INCLUDED 1
#if defined(__cplusplus)
#if defined(ACC_CXX_NOTHROW)
#elif (ACC_CC_GNUC && (ACC_CC_GNUC < 0x020800ul))
#elif (ACC_CC_BORLANDC && (__BORLANDC__ < 0x0450))
#elif (ACC_CC_HIGHC)
#elif (ACC_CC_MSC && (_MSC_VER < 1100))
#elif (ACC_CC_NDPC)
#elif (ACC_CC_TURBOC)
#elif (ACC_CC_WATCOMC && !defined(_CPPUNWIND))
#elif (ACC_CC_ZORTECHC)
#else
#  define ACC_CXX_NOTHROW           throw()
#endif
#if !defined(ACC_CXX_NOTHROW)
#  define ACC_CXX_NOTHROW           /*empty*/
#endif
#if defined(__ACC_CXX_DO_NEW)
#elif (ACC_CC_NDPC || ACC_CC_PGI)
#  define __ACC_CXX_DO_NEW          { return 0; }
#elif ((ACC_CC_BORLANDC || ACC_CC_TURBOC) && ACC_ARCH_I086)
#  define __ACC_CXX_DO_NEW          { return 0; }
#else
#  define __ACC_CXX_DO_NEW          ;
#endif
#if defined(__ACC_CXX_DO_DELETE)
#elif (ACC_CC_BORLANDC || ACC_CC_TURBOC)
#  define __ACC_CXX_DO_DELETE       { }
#else
#  define __ACC_CXX_DO_DELETE       ACC_CXX_NOTHROW { }
#endif
#if (ACC_CC_BORLANDC && (__BORLANDC__ < 0x0450))
#elif (ACC_CC_MSC && ACC_MM_HUGE)
#  define ACC_CXX_DISABLE_NEW_DELETE private:
#elif (ACC_CC_MSC && (_MSC_VER < 1100))
#elif (ACC_CC_NDPC)
#elif (ACC_CC_SYMANTECC || ACC_CC_ZORTECHC)
#elif (ACC_CC_TURBOC)
#elif (ACC_CC_WATCOMC && (__WATCOMC__ < 1100))
#else
#  define __ACC_CXX_HAVE_ARRAY_NEW 1
#endif
#if (__ACC_CXX_HAVE_ARRAY_NEW)
#  define __ACC_CXX_HAVE_PLACEMENT_NEW 1
#endif
#if (__ACC_CXX_HAVE_PLACEMENT_NEW)
#  if (ACC_CC_GNUC >= 0x030000ul)
#    define __ACC_CXX_HAVE_PLACEMENT_DELETE 1
#  elif (ACC_CC_INTELC)
#    define __ACC_CXX_HAVE_PLACEMENT_DELETE 1
#  elif (ACC_CC_MSC && (_MSC_VER >= 1200))
#    define __ACC_CXX_HAVE_PLACEMENT_DELETE 1
#  elif (ACC_CC_CLANG || ACC_CC_LLVM || ACC_CC_PATHSCALE)
#    define __ACC_CXX_HAVE_PLACEMENT_DELETE 1
#  elif (ACC_CC_PGI)
#    define __ACC_CXX_HAVE_PLACEMENT_DELETE 1
#  endif
#endif
#if defined(ACC_CXX_DISABLE_NEW_DELETE)
#elif defined(new) || defined(delete)
#  define ACC_CXX_DISABLE_NEW_DELETE private:
#elif (ACC_CC_GNUC && (ACC_CC_GNUC < 0x025b00ul))
#  define ACC_CXX_DISABLE_NEW_DELETE private:
#elif  (ACC_CC_HIGHC)
#  define ACC_CXX_DISABLE_NEW_DELETE private:
#elif !(__ACC_CXX_HAVE_ARRAY_NEW)
#  define ACC_CXX_DISABLE_NEW_DELETE \
        protected: static void operator delete(void*) __ACC_CXX_DO_DELETE \
        protected: static void* operator new(size_t) __ACC_CXX_DO_NEW \
        private:
#else
#  define ACC_CXX_DISABLE_NEW_DELETE \
        protected: static void operator delete(void*) __ACC_CXX_DO_DELETE \
                   static void operator delete[](void*) __ACC_CXX_DO_DELETE \
        private:   static void* operator new(size_t)  __ACC_CXX_DO_NEW \
                   static void* operator new[](size_t) __ACC_CXX_DO_NEW
#endif
#if defined(ACC_CXX_TRIGGER_FUNCTION)
#else
#  define ACC_CXX_TRIGGER_FUNCTION \
        protected: virtual const void* acc_cxx_trigger_function() const; \
        private:
#endif
#if defined(ACC_CXX_TRIGGER_FUNCTION_IMPL)
#else
#  define ACC_CXX_TRIGGER_FUNCTION_IMPL(klass) \
        const void* klass::acc_cxx_trigger_function() const { return 0; }
#endif
#endif
#endif
#endif
#if defined(ACC_WANT_ACC_CHK_CH)
#  undef ACC_WANT_ACC_CHK_CH
#if !defined(ACCCHK_ASSERT)
#  define ACCCHK_ASSERT(expr)   ACC_COMPILE_TIME_ASSERT_HEADER(expr)
#endif
#if !defined(ACCCHK_ASSERT_SIGN_T)
#  define ACCCHK_ASSERT_SIGN_T(type,relop) \
        ACCCHK_ASSERT( (type) (-1)       relop  (type) 0 ) \
        ACCCHK_ASSERT( (type) (~(type)0) relop  (type) 0 ) \
        ACCCHK_ASSERT( (type) (~(type)0) ==     (type) (-1) )
#endif
#if !defined(ACCCHK_ASSERT_IS_SIGNED_T)
#  define ACCCHK_ASSERT_IS_SIGNED_T(type)       ACCCHK_ASSERT_SIGN_T(type,<)
#endif
#if !defined(ACCCHK_ASSERT_IS_UNSIGNED_T)
#  if (ACC_BROKEN_INTEGRAL_PROMOTION)
#    define ACCCHK_ASSERT_IS_UNSIGNED_T(type) \
        ACCCHK_ASSERT( (type) (-1) > (type) 0 )
#  else
#    define ACCCHK_ASSERT_IS_UNSIGNED_T(type)   ACCCHK_ASSERT_SIGN_T(type,>)
#  endif
#endif
#if defined(ACCCHK_CFG_PEDANTIC)
#if (ACC_CC_BORLANDC && (__BORLANDC__ >= 0x0550) && (__BORLANDC__ < 0x0560))
#  pragma option push -w-8055
#elif (ACC_CC_BORLANDC && (__BORLANDC__ >= 0x0530) && (__BORLANDC__ < 0x0550))
#  pragma option push -w-osh
#endif
#endif
#if (ACC_0xffffffffL - ACC_UINT32_C(4294967294) != 1)
#  error "preprocessor error"
#endif
#if (ACC_0xffffffffL - ACC_UINT32_C(0xfffffffd) != 2)
#  error "preprocessor error"
#endif
#if +0
#  error "preprocessor error"
#endif
#if -0
#  error "preprocessor error"
#endif
#if +0 != 0
#  error "preprocessor error"
#endif
#if -0 != 0
#  error "preprocessor error"
#endif
#define ACCCHK_VAL  1
#define ACCCHK_TMP1 ACCCHK_VAL
#undef ACCCHK_VAL
#define ACCCHK_VAL  2
#define ACCCHK_TMP2 ACCCHK_VAL
#if (ACCCHK_TMP1 != 2)
#  error "preprocessor error 3a"
#endif
#if (ACCCHK_TMP2 != 2)
#  error "preprocessor error 3b"
#endif
#undef ACCCHK_VAL
#if (ACCCHK_TMP2)
#  error "preprocessor error 3c"
#endif
#if (ACCCHK_TMP2 + 0 != 0)
#  error "preprocessor error 3d"
#endif
#undef ACCCHK_TMP1
#undef ACCCHK_TMP2
#if 0 || defined(ACCCHK_CFG_PEDANTIC)
#  if (ACC_ARCH_MIPS) && defined(_MIPS_SZINT)
    ACCCHK_ASSERT((_MIPS_SZINT) == 8 * sizeof(int))
#  endif
#  if (ACC_ARCH_MIPS) && defined(_MIPS_SZLONG)
    ACCCHK_ASSERT((_MIPS_SZLONG) == 8 * sizeof(long))
#  endif
#  if (ACC_ARCH_MIPS) && defined(_MIPS_SZPTR)
    ACCCHK_ASSERT((_MIPS_SZPTR) == 8 * sizeof(void *))
#  endif
#endif
    ACCCHK_ASSERT(1 == 1)
    ACCCHK_ASSERT(__ACC_MASK_GEN(1u,1) == 1)
    ACCCHK_ASSERT(__ACC_MASK_GEN(1u,2) == 3)
    ACCCHK_ASSERT(__ACC_MASK_GEN(1u,3) == 7)
    ACCCHK_ASSERT(__ACC_MASK_GEN(1u,8) == 255)
#if (SIZEOF_INT >= 2)
    ACCCHK_ASSERT(__ACC_MASK_GEN(1,15) == 32767)
    ACCCHK_ASSERT(__ACC_MASK_GEN(1u,16) == 0xffffU)
    ACCCHK_ASSERT(__ACC_MASK_GEN(0u,16) == 0u)
#else
    ACCCHK_ASSERT(__ACC_MASK_GEN(1ul,16) == 0xffffUL)
    ACCCHK_ASSERT(__ACC_MASK_GEN(0ul,16) == 0ul)
#endif
#if (SIZEOF_INT >= 4)
    ACCCHK_ASSERT(__ACC_MASK_GEN(1,31) == 2147483647)
    ACCCHK_ASSERT(__ACC_MASK_GEN(1u,32) == 0xffffffffU)
    ACCCHK_ASSERT(__ACC_MASK_GEN(0u,32) == 0u)
#endif
#if (SIZEOF_LONG >= 4)
    ACCCHK_ASSERT(__ACC_MASK_GEN(1ul,32) == 0xffffffffUL)
    ACCCHK_ASSERT(__ACC_MASK_GEN(0ul,32) == 0ul)
#endif
#if (SIZEOF_LONG >= 8)
    ACCCHK_ASSERT(__ACC_MASK_GEN(1ul,64) == 0xffffffffffffffffUL)
    ACCCHK_ASSERT(__ACC_MASK_GEN(0ul,64) == 0ul)
#endif
#if !(ACC_BROKEN_INTEGRAL_PROMOTION)
    ACCCHK_ASSERT(__ACC_MASK_GEN(1u,SIZEOF_INT*8) == ~0u)
    ACCCHK_ASSERT(__ACC_MASK_GEN(1ul,SIZEOF_LONG*8) == ~0ul)
#endif
#if 1
    ACCCHK_ASSERT(__ACC_MASK_GEN(0,0) == 0)
    ACCCHK_ASSERT(__ACC_MASK_GEN(1,0) == 0)
    ACCCHK_ASSERT(__ACC_MASK_GEN(2,0) == 0)
    ACCCHK_ASSERT(__ACC_MASK_GEN(4,0) == 0)
#endif
#if 1
    ACCCHK_ASSERT(__ACC_MASK_GEN(2,1) == 2)
    ACCCHK_ASSERT(__ACC_MASK_GEN(4,1) == 4)
    ACCCHK_ASSERT(__ACC_MASK_GEN(8,1) == 8)
    ACCCHK_ASSERT(__ACC_MASK_GEN(2,2) == 2+4)
    ACCCHK_ASSERT(__ACC_MASK_GEN(4,2) == 4+8)
    ACCCHK_ASSERT(__ACC_MASK_GEN(8,2) == 8+16)
    ACCCHK_ASSERT(__ACC_MASK_GEN(2,3) == 2+4+8)
    ACCCHK_ASSERT(__ACC_MASK_GEN(4,3) == 4+8+16)
    ACCCHK_ASSERT(__ACC_MASK_GEN(8,3) == 8+16+32)
    ACCCHK_ASSERT(__ACC_MASK_GEN(7,1) == 7)
    ACCCHK_ASSERT(__ACC_MASK_GEN(7,2) == 7+14)
    ACCCHK_ASSERT(__ACC_MASK_GEN(7,3) == 7+14+28)
#endif
#if !(ACC_BROKEN_SIGNED_RIGHT_SHIFT)
    ACCCHK_ASSERT(((-1) >> 7) == -1)
#endif
    ACCCHK_ASSERT(((1)  >> 7) == 0)
    ACCCHK_ASSERT((~0l  & ~0)  == ~0l)
    ACCCHK_ASSERT((~0l  & ~0u) == ~0u)
    ACCCHK_ASSERT((~0ul & ~0)  == ~0ul)
    ACCCHK_ASSERT((~0ul & ~0u) == ~0u)
#if defined(__MSDOS__) && defined(__TURBOC__) && (__TURBOC__ < 0x0150)
#elif (SIZEOF_INT == 2)
    ACCCHK_ASSERT((~0l  & ~0u) == 0xffffU)
    ACCCHK_ASSERT((~0ul & ~0u) == 0xffffU)
#elif (SIZEOF_INT == 4)
    ACCCHK_ASSERT((~0l  & ~0u) == 0xffffffffU)
    ACCCHK_ASSERT((~0ul & ~0u) == 0xffffffffU)
#endif
    ACCCHK_ASSERT_IS_SIGNED_T(signed char)
    ACCCHK_ASSERT_IS_UNSIGNED_T(unsigned char)
    ACCCHK_ASSERT(sizeof(signed char) == sizeof(char))
    ACCCHK_ASSERT(sizeof(unsigned char) == sizeof(char))
    ACCCHK_ASSERT(sizeof(char) == 1)
#if (ACC_CC_CILLY) && (!defined(__CILLY__) || (__CILLY__ < 0x010302L))
#else
    ACCCHK_ASSERT(sizeof(char) == sizeof((char)0))
#endif
#if defined(__cplusplus)
    ACCCHK_ASSERT(sizeof('\0') == sizeof(char))
#else
#  if (ACC_CC_DMC)
#  else
    ACCCHK_ASSERT(sizeof('\0') == sizeof(int))
#  endif
#endif
#if defined(__acc_alignof)
    ACCCHK_ASSERT(__acc_alignof(char) == 1)
    ACCCHK_ASSERT(__acc_alignof(signed char) == 1)
    ACCCHK_ASSERT(__acc_alignof(unsigned char) == 1)
#if defined(acc_int16e_t)
    ACCCHK_ASSERT(__acc_alignof(acc_int16e_t) >= 1)
    ACCCHK_ASSERT(__acc_alignof(acc_int16e_t) <= 2)
#endif
#if defined(acc_int32e_t)
    ACCCHK_ASSERT(__acc_alignof(acc_int32e_t) >= 1)
    ACCCHK_ASSERT(__acc_alignof(acc_int32e_t) <= 4)
#endif
#endif
    ACCCHK_ASSERT_IS_SIGNED_T(short)
    ACCCHK_ASSERT_IS_UNSIGNED_T(unsigned short)
    ACCCHK_ASSERT(sizeof(short) == sizeof(unsigned short))
#if !(ACC_ABI_I8LP16)
    ACCCHK_ASSERT(sizeof(short) >= 2)
#endif
    ACCCHK_ASSERT(sizeof(short) >= sizeof(char))
#if (ACC_CC_CILLY) && (!defined(__CILLY__) || (__CILLY__ < 0x010302L))
#else
    ACCCHK_ASSERT(sizeof(short) == sizeof((short)0))
#endif
#if (SIZEOF_SHORT > 0)
    ACCCHK_ASSERT(sizeof(short) == SIZEOF_SHORT)
#endif
    ACCCHK_ASSERT_IS_SIGNED_T(int)
    ACCCHK_ASSERT_IS_UNSIGNED_T(unsigned int)
    ACCCHK_ASSERT(sizeof(int) == sizeof(unsigned int))
#if !(ACC_ABI_I8LP16)
    ACCCHK_ASSERT(sizeof(int) >= 2)
#endif
    ACCCHK_ASSERT(sizeof(int) >= sizeof(short))
    ACCCHK_ASSERT(sizeof(int) == sizeof(0))
    ACCCHK_ASSERT(sizeof(int) == sizeof((int)0))
#if (SIZEOF_INT > 0)
    ACCCHK_ASSERT(sizeof(int) == SIZEOF_INT)
#endif
    ACCCHK_ASSERT(sizeof(0) == sizeof(int))
    ACCCHK_ASSERT_IS_SIGNED_T(long)
    ACCCHK_ASSERT_IS_UNSIGNED_T(unsigned long)
    ACCCHK_ASSERT(sizeof(long) == sizeof(unsigned long))
#if !(ACC_ABI_I8LP16)
    ACCCHK_ASSERT(sizeof(long) >= 4)
#endif
    ACCCHK_ASSERT(sizeof(long) >= sizeof(int))
    ACCCHK_ASSERT(sizeof(long) == sizeof(0L))
    ACCCHK_ASSERT(sizeof(long) == sizeof((long)0))
#if (SIZEOF_LONG > 0)
    ACCCHK_ASSERT(sizeof(long) == SIZEOF_LONG)
#endif
    ACCCHK_ASSERT(sizeof(0L) == sizeof(long))
    ACCCHK_ASSERT_IS_UNSIGNED_T(size_t)
    ACCCHK_ASSERT(sizeof(size_t) >= sizeof(int))
    ACCCHK_ASSERT(sizeof(size_t) == sizeof(sizeof(0)))
#if (SIZEOF_SIZE_T > 0)
    ACCCHK_ASSERT(sizeof(size_t) == SIZEOF_SIZE_T)
#endif
    ACCCHK_ASSERT_IS_SIGNED_T(ptrdiff_t)
    ACCCHK_ASSERT(sizeof(ptrdiff_t) >= sizeof(int))
    ACCCHK_ASSERT(sizeof(ptrdiff_t) >= sizeof(size_t))
#if !(ACC_BROKEN_SIZEOF)
    ACCCHK_ASSERT(sizeof(ptrdiff_t) == sizeof((char*)0 - (char*)0))
# if (ACC_HAVE_MM_HUGE_PTR)
    ACCCHK_ASSERT(4 == sizeof((char __huge*)0 - (char __huge*)0))
# endif
#endif
#if (SIZEOF_PTRDIFF_T > 0)
    ACCCHK_ASSERT(sizeof(ptrdiff_t) == SIZEOF_PTRDIFF_T)
#endif
    ACCCHK_ASSERT(sizeof(void*) >= sizeof(char*))
#if (SIZEOF_VOID_P > 0)
    ACCCHK_ASSERT(sizeof(void*) == SIZEOF_VOID_P)
    ACCCHK_ASSERT(sizeof(char*) == SIZEOF_VOID_P)
#endif
#if (ACC_HAVE_MM_HUGE_PTR)
    ACCCHK_ASSERT(4 == sizeof(void __huge*))
    ACCCHK_ASSERT(4 == sizeof(char __huge*))
#endif
#if (ACC_ABI_I8LP16)
    ACCCHK_ASSERT((((1u  <<  7) + 1) >>  7) == 1)
    ACCCHK_ASSERT((((1ul << 15) + 1) >> 15) == 1)
#else
    ACCCHK_ASSERT((((1u  << 15) + 1) >> 15) == 1)
    ACCCHK_ASSERT((((1ul << 31) + 1) >> 31) == 1)
#endif
#if defined(ACCCHK_CFG_PEDANTIC)
#if defined(__MSDOS__) && defined(__TURBOC__) && (__TURBOC__ < 0x0150)
#else
    ACCCHK_ASSERT((1   << (8*SIZEOF_INT-1)) < 0)
#endif
#endif
    ACCCHK_ASSERT((1u  << (8*SIZEOF_INT-1)) > 0)
#if defined(ACCCHK_CFG_PEDANTIC)
    ACCCHK_ASSERT((1l  << (8*SIZEOF_LONG-1)) < 0)
#endif
    ACCCHK_ASSERT((1ul << (8*SIZEOF_LONG-1)) > 0)
#if defined(acc_int16e_t)
    ACCCHK_ASSERT(sizeof(acc_int16e_t) == 2)
    ACCCHK_ASSERT(sizeof(acc_int16e_t) == ACC_SIZEOF_ACC_INT16E_T)
    ACCCHK_ASSERT(sizeof(acc_uint16e_t) == 2)
    ACCCHK_ASSERT(sizeof(acc_int16e_t) == sizeof(acc_uint16e_t))
    ACCCHK_ASSERT_IS_SIGNED_T(acc_int16e_t)
    ACCCHK_ASSERT_IS_UNSIGNED_T(acc_uint16e_t)
#if defined(__MSDOS__) && defined(__TURBOC__) && (__TURBOC__ < 0x0150)
#else
    ACCCHK_ASSERT(((acc_uint16e_t)(~(acc_uint16e_t)0ul) >> 15) == 1)
#endif
    ACCCHK_ASSERT( (acc_int16e_t) (1 + ~(acc_int16e_t)0) == 0)
#if defined(ACCCHK_CFG_PEDANTIC)
    ACCCHK_ASSERT( (acc_uint16e_t)(1 + ~(acc_uint16e_t)0) == 0)
#endif
#endif
#if defined(acc_int32e_t)
    ACCCHK_ASSERT(sizeof(acc_int32e_t) == 4)
    ACCCHK_ASSERT(sizeof(acc_int32e_t) == ACC_SIZEOF_ACC_INT32E_T)
    ACCCHK_ASSERT(sizeof(acc_uint32e_t) == 4)
    ACCCHK_ASSERT(sizeof(acc_int32e_t) == sizeof(acc_uint32e_t))
    ACCCHK_ASSERT_IS_SIGNED_T(acc_int32e_t)
    ACCCHK_ASSERT(((( (acc_int32e_t)1 << 30) + 1) >> 30) == 1)
    ACCCHK_ASSERT_IS_UNSIGNED_T(acc_uint32e_t)
    ACCCHK_ASSERT(((( (acc_uint32e_t)1 << 31) + 1) >> 31) == 1)
    ACCCHK_ASSERT(((acc_uint32e_t)(~(acc_uint32e_t)0ul) >> 31) == 1)
    ACCCHK_ASSERT( (acc_int32e_t) (1 + ~(acc_int32e_t)0) == 0)
#if defined(ACCCHK_CFG_PEDANTIC)
    ACCCHK_ASSERT( (acc_uint32e_t)(1 + ~(acc_uint32e_t)0) == 0)
#endif
#endif
#if defined(acc_int32e_t)
    ACCCHK_ASSERT(sizeof(acc_int32l_t) >= sizeof(acc_int32e_t))
#endif
    ACCCHK_ASSERT(sizeof(acc_int32l_t) >= 4)
    ACCCHK_ASSERT(sizeof(acc_int32l_t) == ACC_SIZEOF_ACC_INT32L_T)
    ACCCHK_ASSERT(sizeof(acc_uint32l_t) >= 4)
    ACCCHK_ASSERT(sizeof(acc_int32l_t) == sizeof(acc_uint32l_t))
    ACCCHK_ASSERT_IS_SIGNED_T(acc_int32l_t)
    ACCCHK_ASSERT(((( (acc_int32l_t)1 << 30) + 1) >> 30) == 1)
    ACCCHK_ASSERT_IS_UNSIGNED_T(acc_uint32l_t)
    ACCCHK_ASSERT(((( (acc_uint32l_t)1 << 31) + 1) >> 31) == 1)
    ACCCHK_ASSERT(sizeof(acc_int32f_t) >= sizeof(int))
#if defined(acc_int32e_t)
    ACCCHK_ASSERT(sizeof(acc_int32f_t) >= sizeof(acc_int32e_t))
#endif
    ACCCHK_ASSERT(sizeof(acc_int32f_t) >= sizeof(acc_int32l_t))
    ACCCHK_ASSERT(sizeof(acc_int32f_t) >= 4)
    ACCCHK_ASSERT(sizeof(acc_int32f_t) >= sizeof(acc_int32l_t))
    ACCCHK_ASSERT(sizeof(acc_int32f_t) == ACC_SIZEOF_ACC_INT32F_T)
    ACCCHK_ASSERT(sizeof(acc_uint32f_t) >= 4)
    ACCCHK_ASSERT(sizeof(acc_uint32f_t) >= sizeof(acc_uint32l_t))
    ACCCHK_ASSERT(sizeof(acc_int32f_t) == sizeof(acc_uint32f_t))
    ACCCHK_ASSERT_IS_SIGNED_T(acc_int32f_t)
    ACCCHK_ASSERT(((( (acc_int32f_t)1 << 30) + 1) >> 30) == 1)
    ACCCHK_ASSERT_IS_UNSIGNED_T(acc_uint32f_t)
    ACCCHK_ASSERT(((( (acc_uint32f_t)1 << 31) + 1) >> 31) == 1)
#if defined(acc_int64e_t)
    ACCCHK_ASSERT(sizeof(acc_int64e_t) == 8)
    ACCCHK_ASSERT(sizeof(acc_int64e_t) == ACC_SIZEOF_ACC_INT64E_T)
    ACCCHK_ASSERT(sizeof(acc_uint64e_t) == 8)
    ACCCHK_ASSERT(sizeof(acc_int64e_t) == sizeof(acc_uint64e_t))
    ACCCHK_ASSERT_IS_SIGNED_T(acc_int64e_t)
#if (ACC_CC_BORLANDC && (__BORLANDC__ < 0x0530))
#else
    ACCCHK_ASSERT_IS_UNSIGNED_T(acc_uint64e_t)
#endif
#endif
#if defined(acc_int64l_t)
#if defined(acc_int64e_t)
    ACCCHK_ASSERT(sizeof(acc_int64l_t) >= sizeof(acc_int64e_t))
#endif
    ACCCHK_ASSERT(sizeof(acc_int64l_t) >= 8)
    ACCCHK_ASSERT(sizeof(acc_int64l_t) == ACC_SIZEOF_ACC_INT64L_T)
    ACCCHK_ASSERT(sizeof(acc_uint64l_t) >= 8)
    ACCCHK_ASSERT(sizeof(acc_int64l_t) == sizeof(acc_uint64l_t))
    ACCCHK_ASSERT_IS_SIGNED_T(acc_int64l_t)
    ACCCHK_ASSERT(((( (acc_int64l_t)1 << 62) + 1) >> 62) == 1)
    ACCCHK_ASSERT(((( ACC_INT64_C(1) << 62) + 1) >> 62) == 1)
#if (ACC_CC_BORLANDC && (__BORLANDC__ < 0x0530))
#else
    ACCCHK_ASSERT_IS_UNSIGNED_T(acc_uint64l_t)
    ACCCHK_ASSERT(ACC_UINT64_C(18446744073709551615)     > 0)
#endif
    ACCCHK_ASSERT(((( (acc_uint64l_t)1 << 63) + 1) >> 63) == 1)
    ACCCHK_ASSERT(((( ACC_UINT64_C(1) << 63) + 1) >> 63) == 1)
#if (ACC_CC_GNUC && (ACC_CC_GNUC < 0x020600ul))
    ACCCHK_ASSERT(ACC_INT64_C(9223372036854775807)       > ACC_INT64_C(0))
#else
    ACCCHK_ASSERT(ACC_INT64_C(9223372036854775807)       > 0)
#endif
    ACCCHK_ASSERT(ACC_INT64_C(-9223372036854775807) - 1  < 0)
    ACCCHK_ASSERT( ACC_INT64_C(9223372036854775807) % ACC_INT32_C(2147483629)  == 721)
    ACCCHK_ASSERT( ACC_INT64_C(9223372036854775807) % ACC_INT32_C(2147483647)  == 1)
    ACCCHK_ASSERT(ACC_UINT64_C(9223372036854775807) % ACC_UINT32_C(2147483629) == 721)
    ACCCHK_ASSERT(ACC_UINT64_C(9223372036854775807) % ACC_UINT32_C(2147483647) == 1)
#endif
#if defined(acc_int64f_t)
#if defined(acc_int64e_t)
    ACCCHK_ASSERT(sizeof(acc_int64f_t) >= sizeof(acc_int64e_t))
#endif
    ACCCHK_ASSERT(sizeof(acc_int64f_t) >= sizeof(acc_int64l_t))
    ACCCHK_ASSERT(sizeof(acc_int64f_t) >= 8)
    ACCCHK_ASSERT(sizeof(acc_int64f_t) >= sizeof(acc_int64l_t))
    ACCCHK_ASSERT(sizeof(acc_int64f_t) == ACC_SIZEOF_ACC_INT64F_T)
    ACCCHK_ASSERT(sizeof(acc_uint64f_t) >= 8)
    ACCCHK_ASSERT(sizeof(acc_uint64f_t) >= sizeof(acc_uint64l_t))
    ACCCHK_ASSERT(sizeof(acc_int64f_t) == sizeof(acc_uint64f_t))
    ACCCHK_ASSERT_IS_SIGNED_T(acc_int64f_t)
#if (ACC_CC_BORLANDC && (__BORLANDC__ < 0x0530))
#else
    ACCCHK_ASSERT_IS_UNSIGNED_T(acc_uint64f_t)
#endif
#endif
#if !defined(__ACC_INTPTR_T_IS_POINTER)
    ACCCHK_ASSERT_IS_SIGNED_T(acc_intptr_t)
    ACCCHK_ASSERT_IS_UNSIGNED_T(acc_uintptr_t)
#endif
    ACCCHK_ASSERT(sizeof(acc_intptr_t) >= sizeof(void *))
    ACCCHK_ASSERT(sizeof(acc_intptr_t) == ACC_SIZEOF_ACC_INTPTR_T)
    ACCCHK_ASSERT(sizeof(acc_intptr_t) == sizeof(acc_uintptr_t))
#if defined(acc_word_t)
    ACCCHK_ASSERT(ACC_WORDSIZE == ACC_SIZEOF_ACC_WORD_T)
    ACCCHK_ASSERT_IS_UNSIGNED_T(acc_word_t)
    ACCCHK_ASSERT_IS_SIGNED_T(acc_sword_t)
    ACCCHK_ASSERT(sizeof(acc_word_t) == ACC_SIZEOF_ACC_WORD_T)
    ACCCHK_ASSERT(sizeof(acc_word_t) == sizeof(acc_sword_t))
#endif
    ACCCHK_ASSERT(sizeof(acc_int8_t) == 1)
    ACCCHK_ASSERT(sizeof(acc_uint8_t) == 1)
    ACCCHK_ASSERT(sizeof(acc_int8_t) == sizeof(acc_uint8_t))
    ACCCHK_ASSERT_IS_SIGNED_T(acc_int8_t)
    ACCCHK_ASSERT_IS_UNSIGNED_T(acc_uint8_t)
#if defined(ACC_INT16_C)
    ACCCHK_ASSERT(sizeof(ACC_INT16_C(0)) >= 2)
    ACCCHK_ASSERT(sizeof(ACC_UINT16_C(0)) >= 2)
    ACCCHK_ASSERT((ACC_UINT16_C(0xffff) >> 15) == 1)
#endif
#if defined(ACC_INT32_C)
    ACCCHK_ASSERT(sizeof(ACC_INT32_C(0)) >= 4)
    ACCCHK_ASSERT(sizeof(ACC_UINT32_C(0)) >= 4)
    ACCCHK_ASSERT((ACC_UINT32_C(0xffffffff) >> 31) == 1)
#endif
#if defined(ACC_INT64_C)
#if (ACC_CC_BORLANDC && (__BORLANDC__ < 0x0560))
#else
    ACCCHK_ASSERT(sizeof(ACC_INT64_C(0)) >= 8)
    ACCCHK_ASSERT(sizeof(ACC_UINT64_C(0)) >= 8)
#endif
    ACCCHK_ASSERT((ACC_UINT64_C(0xffffffffffffffff) >> 63) == 1)
    ACCCHK_ASSERT((ACC_UINT64_C(0xffffffffffffffff) & ~0)  == ACC_UINT64_C(0xffffffffffffffff))
    ACCCHK_ASSERT((ACC_UINT64_C(0xffffffffffffffff) & ~0l) == ACC_UINT64_C(0xffffffffffffffff))
#if (SIZEOF_INT == 4)
# if (ACC_CC_GNUC && (ACC_CC_GNUC < 0x020000ul))
# else
    ACCCHK_ASSERT((ACC_UINT64_C(0xffffffffffffffff) & (~0u+0u)) == 0xffffffffu)
# endif
#endif
#if (SIZEOF_LONG == 4)
# if (ACC_CC_GNUC && (ACC_CC_GNUC < 0x020000ul))
# else
    ACCCHK_ASSERT((ACC_UINT64_C(0xffffffffffffffff) & (~0ul+0ul)) == 0xfffffffful)
# endif
#endif
#endif
#if (ACC_MM_TINY || ACC_MM_SMALL || ACC_MM_MEDIUM)
    ACCCHK_ASSERT(sizeof(void*) == 2)
    ACCCHK_ASSERT(sizeof(ptrdiff_t) == 2)
#elif (ACC_MM_COMPACT || ACC_MM_LARGE || ACC_MM_HUGE)
    ACCCHK_ASSERT(sizeof(void*) == 4)
#endif
#if (ACC_MM_TINY || ACC_MM_SMALL || ACC_MM_COMPACT)
    ACCCHK_ASSERT(sizeof(void (*)(void)) == 2)
#elif (ACC_MM_MEDIUM || ACC_MM_LARGE || ACC_MM_HUGE)
    ACCCHK_ASSERT(sizeof(void (*)(void)) == 4)
#endif
#if (ACC_ABI_ILP32)
    ACCCHK_ASSERT(sizeof(int) == 4)
    ACCCHK_ASSERT(sizeof(long) == 4)
    ACCCHK_ASSERT(sizeof(void*) == 4)
    ACCCHK_ASSERT(sizeof(ptrdiff_t) == sizeof(void*))
    ACCCHK_ASSERT(sizeof(size_t) == sizeof(void*))
    ACCCHK_ASSERT(sizeof(acc_intptr_t) == sizeof(void *))
#endif
#if (ACC_ABI_ILP64)
    ACCCHK_ASSERT(sizeof(int) == 8)
    ACCCHK_ASSERT(sizeof(long) == 8)
    ACCCHK_ASSERT(sizeof(void*) == 8)
    ACCCHK_ASSERT(sizeof(ptrdiff_t) == sizeof(void*))
    ACCCHK_ASSERT(sizeof(size_t) == sizeof(void*))
    ACCCHK_ASSERT(sizeof(acc_intptr_t) == sizeof(void *))
#endif
#if (ACC_ABI_IP32L64)
    ACCCHK_ASSERT(sizeof(int) == 4)
    ACCCHK_ASSERT(sizeof(long) == 8)
    ACCCHK_ASSERT(sizeof(void*) == 4)
    ACCCHK_ASSERT(sizeof(ptrdiff_t) == sizeof(void*))
    ACCCHK_ASSERT(sizeof(size_t) == sizeof(void*))
    ACCCHK_ASSERT(sizeof(acc_intptr_t) == sizeof(void *))
#endif
#if (ACC_ABI_LLP64)
    ACCCHK_ASSERT(sizeof(int) == 4)
    ACCCHK_ASSERT(sizeof(long) == 4)
    ACCCHK_ASSERT(sizeof(void*) == 8)
    ACCCHK_ASSERT(sizeof(ptrdiff_t) == sizeof(void*))
    ACCCHK_ASSERT(sizeof(size_t) == sizeof(void*))
    ACCCHK_ASSERT(sizeof(acc_intptr_t) == sizeof(void *))
#endif
#if (ACC_ABI_LP32)
    ACCCHK_ASSERT(sizeof(int) == 2)
    ACCCHK_ASSERT(sizeof(long) == 4)
    ACCCHK_ASSERT(sizeof(void*) == 4)
    ACCCHK_ASSERT(sizeof(acc_intptr_t) == sizeof(void *))
#endif
#if (ACC_ABI_LP64)
    ACCCHK_ASSERT(sizeof(int) == 4)
    ACCCHK_ASSERT(sizeof(long) == 8)
    ACCCHK_ASSERT(sizeof(void*) == 8)
    ACCCHK_ASSERT(sizeof(ptrdiff_t) == sizeof(void*))
    ACCCHK_ASSERT(sizeof(size_t) == sizeof(void*))
    ACCCHK_ASSERT(sizeof(acc_intptr_t) == sizeof(void *))
#endif
#if (ACC_ARCH_I086)
    ACCCHK_ASSERT(sizeof(size_t) == 2)
    ACCCHK_ASSERT(sizeof(acc_intptr_t) == sizeof(void *))
#elif (ACC_ARCH_I386 || ACC_ARCH_M68K)
    ACCCHK_ASSERT(sizeof(size_t) == 4)
    ACCCHK_ASSERT(sizeof(ptrdiff_t) == 4)
    ACCCHK_ASSERT(sizeof(acc_intptr_t) == sizeof(void *))
#endif
#if (ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_WIN32)
    ACCCHK_ASSERT(sizeof(size_t) == 4)
    ACCCHK_ASSERT(sizeof(ptrdiff_t) == 4)
    ACCCHK_ASSERT(sizeof(void (*)(void)) == 4)
#elif (ACC_OS_WIN64)
    ACCCHK_ASSERT(sizeof(size_t) == 8)
    ACCCHK_ASSERT(sizeof(ptrdiff_t) == 8)
    ACCCHK_ASSERT(sizeof(void (*)(void)) == 8)
#endif
#if (ACC_CC_NDPC)
#elif (SIZEOF_INT > 1)
    ACCCHK_ASSERT( (int) ((unsigned char) ((signed char) -1)) == 255)
#endif
#if defined(ACCCHK_CFG_PEDANTIC)
#if (ACC_CC_KEILC)
#elif (ACC_CC_NDPC)
#elif !(ACC_BROKEN_INTEGRAL_PROMOTION) && (SIZEOF_INT > 1)
    ACCCHK_ASSERT( (((unsigned char)128) << (int)(8*sizeof(int)-8)) < 0)
#endif
#endif
#if defined(ACCCHK_CFG_PEDANTIC)
#if (ACC_CC_BORLANDC && (__BORLANDC__ >= 0x0530) && (__BORLANDC__ < 0x0560))
#  pragma option pop
#endif
#endif
#endif
#if defined(ACC_WANT_ACCLIB_UA)
#  undef ACC_WANT_ACCLIB_UA
#define __ACCLIB_UA_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif
ACCLIB_PUBLIC(unsigned, acc_ua_get_be16) (const acc_hvoid_p p)
{
#if defined(ACC_UA_GET_BE16)
    return ACC_UA_GET_BE16(p);
#else
    const acc_hbyte_p b = (const acc_hbyte_p) p;
    return ((unsigned)b[1]) | ((unsigned)b[0] << 8);
#endif
}
ACCLIB_PUBLIC(acc_uint32l_t, acc_ua_get_be24) (const acc_hvoid_p p)
{
    const acc_hbyte_p b = (const acc_hbyte_p) p;
    return ((acc_uint32l_t)b[2]) | ((acc_uint32l_t)b[1] << 8) | ((acc_uint32l_t)b[0] << 16);
}
ACCLIB_PUBLIC(acc_uint32l_t, acc_ua_get_be32) (const acc_hvoid_p p)
{
#if defined(ACC_UA_GET_BE32)
    return ACC_UA_GET_BE32(p);
#else
    const acc_hbyte_p b = (const acc_hbyte_p) p;
    return ((acc_uint32l_t)b[3]) | ((acc_uint32l_t)b[2] << 8) | ((acc_uint32l_t)b[1] << 16) | ((acc_uint32l_t)b[0] << 24);
#endif
}
ACCLIB_PUBLIC(void, acc_ua_set_be16) (acc_hvoid_p p, unsigned v)
{
#if defined(ACC_UA_SET_BE16)
    ACC_UA_SET_BE16(p, v);
#else
    acc_hbyte_p b = (acc_hbyte_p) p;
    b[1] = (unsigned char) ((v >>  0) & 0xff);
    b[0] = (unsigned char) ((v >>  8) & 0xff);
#endif
}
ACCLIB_PUBLIC(void, acc_ua_set_be24) (acc_hvoid_p p, acc_uint32l_t v)
{
    acc_hbyte_p b = (acc_hbyte_p) p;
    b[2] = (unsigned char) ((v >>  0) & 0xff);
    b[1] = (unsigned char) ((v >>  8) & 0xff);
    b[0] = (unsigned char) ((v >> 16) & 0xff);
}
ACCLIB_PUBLIC(void, acc_ua_set_be32) (acc_hvoid_p p, acc_uint32l_t v)
{
#if defined(ACC_UA_SET_BE32)
    ACC_UA_SET_BE32(p, v);
#else
    acc_hbyte_p b = (acc_hbyte_p) p;
    b[3] = (unsigned char) ((v >>  0) & 0xff);
    b[2] = (unsigned char) ((v >>  8) & 0xff);
    b[1] = (unsigned char) ((v >> 16) & 0xff);
    b[0] = (unsigned char) ((v >> 24) & 0xff);
#endif
}
ACCLIB_PUBLIC(unsigned, acc_ua_get_le16) (const acc_hvoid_p p)
{
#if defined(ACC_UA_GET_LE16)
    return ACC_UA_GET_LE16(p);
#else
    const acc_hbyte_p b = (const acc_hbyte_p) p;
    return ((unsigned)b[0]) | ((unsigned)b[1] << 8);
#endif
}
ACCLIB_PUBLIC(acc_uint32l_t, acc_ua_get_le24) (const acc_hvoid_p p)
{
    const acc_hbyte_p b = (const acc_hbyte_p) p;
    return ((acc_uint32l_t)b[0]) | ((acc_uint32l_t)b[1] << 8) | ((acc_uint32l_t)b[2] << 16);
}
ACCLIB_PUBLIC(acc_uint32l_t, acc_ua_get_le32) (const acc_hvoid_p p)
{
#if defined(ACC_UA_GET_LE32)
    return ACC_UA_GET_LE32(p);
#else
    const acc_hbyte_p b = (const acc_hbyte_p) p;
    return ((acc_uint32l_t)b[0]) | ((acc_uint32l_t)b[1] << 8) | ((acc_uint32l_t)b[2] << 16) | ((acc_uint32l_t)b[3] << 24);
#endif
}
ACCLIB_PUBLIC(void, acc_ua_set_le16) (acc_hvoid_p p, unsigned v)
{
#if defined(ACC_UA_SET_LE16)
    ACC_UA_SET_LE16(p, v);
#else
    acc_hbyte_p b = (acc_hbyte_p) p;
    b[0] = (unsigned char) ((v >>  0) & 0xff);
    b[1] = (unsigned char) ((v >>  8) & 0xff);
#endif
}
ACCLIB_PUBLIC(void, acc_ua_set_le24) (acc_hvoid_p p, acc_uint32l_t v)
{
    acc_hbyte_p b = (acc_hbyte_p) p;
    b[0] = (unsigned char) ((v >>  0) & 0xff);
    b[1] = (unsigned char) ((v >>  8) & 0xff);
    b[2] = (unsigned char) ((v >> 16) & 0xff);
}
ACCLIB_PUBLIC(void, acc_ua_set_le32) (acc_hvoid_p p, acc_uint32l_t v)
{
#if defined(ACC_UA_SET_LE32)
    ACC_UA_SET_LE32(p, v);
#else
    acc_hbyte_p b = (acc_hbyte_p) p;
    b[0] = (unsigned char) ((v >>  0) & 0xff);
    b[1] = (unsigned char) ((v >>  8) & 0xff);
    b[2] = (unsigned char) ((v >> 16) & 0xff);
    b[3] = (unsigned char) ((v >> 24) & 0xff);
#endif
}
#if defined(acc_int64l_t)
ACCLIB_PUBLIC(acc_uint64l_t, acc_ua_get_be64) (const acc_hvoid_p p)
{
#if defined(ACC_UA_GET_BE64)
    return ACC_UA_GET_BE64(p);
#elif defined(ACC_UA_GET_BE32)
    const acc_hbyte_p b = (const acc_hbyte_p) p;
    acc_uint32e_t v0, v1;
    v1 = ACC_UA_GET_BE32(b + 0);
    v0 = ACC_UA_GET_BE32(b + 4);
    return ((acc_uint64l_t)v0) | ((acc_uint64l_t)v1 << 32);
#elif (ACC_SIZEOF_LONG >= 8) || (ACC_SIZEOF_SIZE_T >= 8)
    const acc_hbyte_p b = (const acc_hbyte_p) p;
    return ((acc_uint64l_t)b[7]) | ((acc_uint64l_t)b[6] << 8) | ((acc_uint64l_t)b[5] << 16) | ((acc_uint64l_t)b[4] << 24) | ((acc_uint64l_t)b[3] << 32) | ((acc_uint64l_t)b[2] << 40) | ((acc_uint64l_t)b[1] << 48) | ((acc_uint64l_t)b[0] << 56);
#else
    const acc_hbyte_p b = (const acc_hbyte_p) p;
    acc_uint32l_t v0, v1;
    v1 = ((acc_uint32l_t)b[3]) | ((acc_uint32l_t)b[2] << 8) | ((acc_uint32l_t)b[1] << 16) | ((acc_uint32l_t)b[0] << 24);
    b += 4;
    v0 = ((acc_uint32l_t)b[3]) | ((acc_uint32l_t)b[2] << 8) | ((acc_uint32l_t)b[1] << 16) | ((acc_uint32l_t)b[0] << 24);
    return ((acc_uint64l_t)v0) | ((acc_uint64l_t)v1 << 32);
#endif
}
ACCLIB_PUBLIC(void, acc_ua_set_be64) (acc_hvoid_p p, acc_uint64l_t v)
{
#if defined(ACC_UA_SET_BE64)
    ACC_UA_SET_BE64(p, v);
#elif defined(ACC_UA_SET_BE32)
    acc_hbyte_p b = (acc_hbyte_p) p;
    ACC_UA_SET_BE32(b + 4, v);
    v >>= 32;
    ACC_UA_SET_BE32(b + 0, v);
#elif (ACC_SIZEOF_LONG >= 8) || (ACC_SIZEOF_SIZE_T >= 8)
    acc_hbyte_p b = (acc_hbyte_p) p;
    b[7] = (unsigned char) v; v >>= 8;
    b[6] = (unsigned char) v; v >>= 8;
    b[5] = (unsigned char) v; v >>= 8;
    b[4] = (unsigned char) v; v >>= 8;
    b[3] = (unsigned char) v; v >>= 8;
    b[2] = (unsigned char) v; v >>= 8;
    b[1] = (unsigned char) v; v >>= 8;
    b[0] = (unsigned char) v;
#else
    acc_hbyte_p b = (acc_hbyte_p) p;
    acc_uint32l_t x;
    x = (acc_uint32l_t) v;
    b[7] = (unsigned char) x; x >>= 8;
    b[6] = (unsigned char) x; x >>= 8;
    b[5] = (unsigned char) x; x >>= 8;
    b[4] = (unsigned char) x;
    v >>= 32;
    x = (acc_uint32l_t) v;
    b[3] = (unsigned char) x; x >>= 8;
    b[2] = (unsigned char) x; x >>= 8;
    b[1] = (unsigned char) x; x >>= 8;
    b[0] = (unsigned char) x;
#endif
}
#endif
#if defined(acc_int64l_t)
ACCLIB_PUBLIC(acc_uint64l_t, acc_ua_get_le64) (const acc_hvoid_p p)
{
#if defined(ACC_UA_GET_LE64)
    return ACC_UA_GET_LE64(p);
#elif defined(ACC_UA_GET_LE32)
    const acc_hbyte_p b = (const acc_hbyte_p) p;
    acc_uint32e_t v0, v1;
    v0 = ACC_UA_GET_LE32(b + 0);
    v1 = ACC_UA_GET_LE32(b + 4);
    return ((acc_uint64l_t)v0) | ((acc_uint64l_t)v1 << 32);
#elif (ACC_SIZEOF_LONG >= 8) || (ACC_SIZEOF_SIZE_T >= 8)
    const acc_hbyte_p b = (const acc_hbyte_p) p;
    return ((acc_uint64l_t)b[0]) | ((acc_uint64l_t)b[1] << 8) | ((acc_uint64l_t)b[2] << 16) | ((acc_uint64l_t)b[3] << 24) | ((acc_uint64l_t)b[4] << 32) | ((acc_uint64l_t)b[5] << 40) | ((acc_uint64l_t)b[6] << 48) | ((acc_uint64l_t)b[7] << 56);
#else
    const acc_hbyte_p b = (const acc_hbyte_p) p;
    acc_uint32l_t v0, v1;
    v0 = ((acc_uint32l_t)b[0]) | ((acc_uint32l_t)b[1] << 8) | ((acc_uint32l_t)b[2] << 16) | ((acc_uint32l_t)b[3] << 24);
    b += 4;
    v1 = ((acc_uint32l_t)b[0]) | ((acc_uint32l_t)b[1] << 8) | ((acc_uint32l_t)b[2] << 16) | ((acc_uint32l_t)b[3] << 24);
    return ((acc_uint64l_t)v0) | ((acc_uint64l_t)v1 << 32);
#endif
}
ACCLIB_PUBLIC(void, acc_ua_set_le64) (acc_hvoid_p p, acc_uint64l_t v)
{
#if defined(ACC_UA_SET_LE64)
    ACC_UA_SET_LE64(p, v);
#elif defined(ACC_UA_SET_LE32)
    acc_hbyte_p b = (acc_hbyte_p) p;
    ACC_UA_SET_LE32(b + 0, v);
    v >>= 32;
    ACC_UA_SET_LE32(b + 4, v);
#elif (ACC_SIZEOF_LONG >= 8) || (ACC_SIZEOF_SIZE_T >= 8)
    acc_hbyte_p b = (acc_hbyte_p) p;
    b[0] = (unsigned char) v; v >>= 8;
    b[1] = (unsigned char) v; v >>= 8;
    b[2] = (unsigned char) v; v >>= 8;
    b[3] = (unsigned char) v; v >>= 8;
    b[4] = (unsigned char) v; v >>= 8;
    b[5] = (unsigned char) v; v >>= 8;
    b[6] = (unsigned char) v; v >>= 8;
    b[7] = (unsigned char) v;
#else
    acc_hbyte_p b = (acc_hbyte_p) p;
    acc_uint32l_t x;
    x = (acc_uint32l_t) v;
    b[0] = (unsigned char) x; x >>= 8;
    b[1] = (unsigned char) x; x >>= 8;
    b[2] = (unsigned char) x; x >>= 8;
    b[3] = (unsigned char) x;
    v >>= 32;
    x = (acc_uint32l_t) v;
    b[4] = (unsigned char) x; x >>= 8;
    b[5] = (unsigned char) x; x >>= 8;
    b[6] = (unsigned char) x; x >>= 8;
    b[7] = (unsigned char) x;
#endif
}
#endif
#endif
#if defined(ACC_WANT_ACCLIB_VGET)
#  undef ACC_WANT_ACCLIB_VGET
#define __ACCLIB_VGET_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)                r __ACCLIB_FUNCNAME(f)
#endif
#if !defined(ACCLIB_PUBLIC_NOINLINE)
#  if !defined(__acc_noinline)
#    define ACCLIB_PUBLIC_NOINLINE(r,f)     r __ACCLIB_FUNCNAME(f)
#  elif (ACC_CC_CLANG || (ACC_CC_GNUC >= 0x030400ul) || ACC_CC_LLVM)
#    define ACCLIB_PUBLIC_NOINLINE(r,f)     __acc_noinline __attribute__((__used__)) r __ACCLIB_FUNCNAME(f)
#  else
#    define ACCLIB_PUBLIC_NOINLINE(r,f)     __acc_noinline r __ACCLIB_FUNCNAME(f)
#  endif
#endif
extern void* volatile acc_vget_ptr__;
#if (ACC_CC_CLANG || (ACC_CC_GNUC >= 0x030400ul) || ACC_CC_LLVM)
void* volatile __attribute__((__used__)) acc_vget_ptr__ = (void *) 0;
#else
void* volatile acc_vget_ptr__ = (void *) 0;
#endif
#ifndef __ACCLIB_VGET_BODY
#define __ACCLIB_VGET_BODY(T) \
    if __acc_unlikely(acc_vget_ptr__) { \
        * (T *) acc_vget_ptr__ = v; \
        * (int *) acc_vget_ptr__ = expr; \
        v = * (T *) acc_vget_ptr__; \
    } \
    return v;
#endif
ACCLIB_PUBLIC_NOINLINE(short, acc_vget_short) (short v, int expr)
{
    __ACCLIB_VGET_BODY(short)
}
ACCLIB_PUBLIC_NOINLINE(int, acc_vget_int) (int v, int expr)
{
    __ACCLIB_VGET_BODY(int)
}
ACCLIB_PUBLIC_NOINLINE(long, acc_vget_long) (long v, int expr)
{
    __ACCLIB_VGET_BODY(long)
}
#if defined(acc_int64l_t)
ACCLIB_PUBLIC_NOINLINE(acc_int64l_t, acc_vget_acc_int64l_t) (acc_int64l_t v, int expr)
{
    __ACCLIB_VGET_BODY(acc_int64l_t)
}
#endif
ACCLIB_PUBLIC_NOINLINE(acc_hsize_t, acc_vget_acc_hsize_t) (acc_hsize_t v, int expr)
{
    __ACCLIB_VGET_BODY(acc_hsize_t)
}
#if !(ACC_CFG_NO_FLOAT)
ACCLIB_PUBLIC_NOINLINE(float, acc_vget_float) (float v, int expr)
{
    __ACCLIB_VGET_BODY(float)
}
#endif
#if !(ACC_CFG_NO_DOUBLE)
ACCLIB_PUBLIC_NOINLINE(double, acc_vget_double) (double v, int expr)
{
    __ACCLIB_VGET_BODY(double)
}
#endif
ACCLIB_PUBLIC_NOINLINE(acc_hvoid_p, acc_vget_acc_hvoid_p) (acc_hvoid_p v, int expr)
{
    __ACCLIB_VGET_BODY(acc_hvoid_p)
}
#if (ACC_ARCH_I086 && ACC_CC_TURBOC && (__TURBOC__ == 0x0295)) && !defined(__cplusplus)
ACCLIB_PUBLIC_NOINLINE(acc_hvoid_p, acc_vget_acc_hvoid_cp) (const acc_hvoid_p vv, int expr)
{
    acc_hvoid_p v = (acc_hvoid_p) vv;
    __ACCLIB_VGET_BODY(acc_hvoid_p)
}
#else
ACCLIB_PUBLIC_NOINLINE(const acc_hvoid_p, acc_vget_acc_hvoid_cp) (const acc_hvoid_p v, int expr)
{
    __ACCLIB_VGET_BODY(const acc_hvoid_p)
}
#endif
#endif
#if defined(ACC_WANT_ACCLIB_HMEMCPY)
#  undef ACC_WANT_ACCLIB_HMEMCPY
#define __ACCLIB_HMEMCPY_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif
ACCLIB_PUBLIC(int, acc_hmemcmp) (const acc_hvoid_p s1, const acc_hvoid_p s2, acc_hsize_t len)
{
#if (ACC_HAVE_MM_HUGE_PTR) || !(HAVE_MEMCMP)
    const acc_hbyte_p p1 = (const acc_hbyte_p) s1;
    const acc_hbyte_p p2 = (const acc_hbyte_p) s2;
    if __acc_likely(len > 0) do
    {
        int d = *p1 - *p2;
        if (d != 0)
            return d;
        p1++; p2++;
    } while __acc_likely(--len > 0);
    return 0;
#else
    return memcmp(s1, s2, len);
#endif
}
ACCLIB_PUBLIC(acc_hvoid_p, acc_hmemcpy) (acc_hvoid_p dest, const acc_hvoid_p src, acc_hsize_t len)
{
#if (ACC_HAVE_MM_HUGE_PTR) || !(HAVE_MEMCPY)
    acc_hbyte_p p1 = (acc_hbyte_p) dest;
    const acc_hbyte_p p2 = (const acc_hbyte_p) src;
    if (!(len > 0) || p1 == p2)
        return dest;
    do
        *p1++ = *p2++;
    while __acc_likely(--len > 0);
    return dest;
#else
    return memcpy(dest, src, len);
#endif
}
ACCLIB_PUBLIC(acc_hvoid_p, acc_hmemmove) (acc_hvoid_p dest, const acc_hvoid_p src, acc_hsize_t len)
{
#if (ACC_HAVE_MM_HUGE_PTR) || !(HAVE_MEMMOVE)
    acc_hbyte_p p1 = (acc_hbyte_p) dest;
    const acc_hbyte_p p2 = (const acc_hbyte_p) src;
    if (!(len > 0) || p1 == p2)
        return dest;
    if (p1 < p2)
    {
        do
            *p1++ = *p2++;
        while __acc_likely(--len > 0);
    }
    else
    {
        p1 += len;
        p2 += len;
        do
            *--p1 = *--p2;
        while __acc_likely(--len > 0);
    }
    return dest;
#else
    return memmove(dest, src, len);
#endif
}
ACCLIB_PUBLIC(acc_hvoid_p, acc_hmemset) (acc_hvoid_p s, int c, acc_hsize_t len)
{
#if (ACC_HAVE_MM_HUGE_PTR) || !(HAVE_MEMSET)
    acc_hbyte_p p = (acc_hbyte_p) s;
    if __acc_likely(len > 0) do
        *p++ = (unsigned char) c;
    while __acc_likely(--len > 0);
    return s;
#else
    return memset(s, c, len);
#endif
}
#endif
#if defined(ACC_WANT_ACCLIB_RAND)
#  undef ACC_WANT_ACCLIB_RAND
#define __ACCLIB_RAND_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif
ACCLIB_PUBLIC(void, acc_srand31) (acc_rand31_p r, acc_uint32l_t seed)
{
    r->seed = seed & ACC_UINT32_C(0xffffffff);
}
ACCLIB_PUBLIC(acc_uint32l_t, acc_rand31) (acc_rand31_p r)
{
    r->seed = r->seed * ACC_UINT32_C(1103515245) + 12345;
    r->seed &= ACC_UINT32_C(0x7fffffff);
    return r->seed;
}
#if defined(acc_int64l_t)
ACCLIB_PUBLIC(void, acc_srand48) (acc_rand48_p r, acc_uint32l_t seed)
{
    r->seed = seed & ACC_UINT32_C(0xffffffff);
    r->seed <<= 16; r->seed |= 0x330e;
}
ACCLIB_PUBLIC(acc_uint32l_t, acc_rand48) (acc_rand48_p r)
{
    acc_uint64l_t a;
    r->seed = r->seed * ACC_UINT64_C(25214903917) + 11;
    r->seed &= ACC_UINT64_C(0xffffffffffff);
    a = r->seed >> 17;
    return (acc_uint32l_t) a;
}
ACCLIB_PUBLIC(acc_uint32l_t, acc_rand48_r32) (acc_rand48_p r)
{
    acc_uint64l_t a;
    r->seed = r->seed * ACC_UINT64_C(25214903917) + 11;
    r->seed &= ACC_UINT64_C(0xffffffffffff);
    a = r->seed >> 16;
    return (acc_uint32l_t) a;
}
#endif
#if defined(acc_int64l_t)
ACCLIB_PUBLIC(void, acc_srand64) (acc_rand64_p r, acc_uint64l_t seed)
{
    r->seed = seed & ACC_UINT64_C(0xffffffffffffffff);
}
ACCLIB_PUBLIC(acc_uint32l_t, acc_rand64) (acc_rand64_p r)
{
    acc_uint64l_t a;
    r->seed = r->seed * ACC_UINT64_C(6364136223846793005) + 1;
#if (ACC_SIZEOF_ACC_INT64L_T > 8)
    r->seed &= ACC_UINT64_C(0xffffffffffffffff);
#endif
    a = r->seed >> 33;
    return (acc_uint32l_t) a;
}
ACCLIB_PUBLIC(acc_uint32l_t, acc_rand64_r32) (acc_rand64_p r)
{
    acc_uint64l_t a;
    r->seed = r->seed * ACC_UINT64_C(6364136223846793005) + 1;
#if (ACC_SIZEOF_ACC_INT64L_T > 8)
    r->seed &= ACC_UINT64_C(0xffffffffffffffff);
#endif
    a = r->seed >> 32;
    return (acc_uint32l_t) a;
}
#endif
ACCLIB_PUBLIC(void, acc_srandmt) (acc_randmt_p r, acc_uint32l_t seed)
{
    unsigned i = 0;
    do {
        r->s[i++] = (seed &= ACC_UINT32_C(0xffffffff));
        seed ^= seed >> 30;
        seed = seed * ACC_UINT32_C(0x6c078965) + i;
    } while (i != 624);
    r->n = i;
}
ACCLIB_PUBLIC(acc_uint32l_t, acc_randmt) (acc_randmt_p r)
{
    return (__ACCLIB_FUNCNAME(acc_randmt_r32)(r)) >> 1;
}
ACCLIB_PUBLIC(acc_uint32l_t, acc_randmt_r32) (acc_randmt_p r)
{
    acc_uint32l_t v;
    if __acc_unlikely(r->n == 624) {
        unsigned i = 0, j;
        r->n = 0;
        do {
            j = i - 623; if ((int) j < 0) j += 624;
            v = (r->s[i] & ACC_UINT32_C(0x80000000)) ^ (r->s[j] & ACC_UINT32_C(0x7fffffff));
            j = i - 227; if ((int) j < 0) j += 624;
            r->s[i] = r->s[j] ^ (v >> 1);
            if (v & 1) r->s[i] ^= ACC_UINT32_C(0x9908b0df);
        } while (++i != 624);
    }
    v = r->s[r->n++];
    v ^= v >> 11; v ^= (v & ACC_UINT32_C(0x013a58ad)) << 7;
    v ^= (v & ACC_UINT32_C(0x0001df8c)) << 15; v ^= v >> 18;
    return v;
}
#if defined(acc_int64l_t)
ACCLIB_PUBLIC(void, acc_srandmt64) (acc_randmt64_p r, acc_uint64l_t seed)
{
    unsigned i = 0;
    do {
        r->s[i++] = (seed &= ACC_UINT64_C(0xffffffffffffffff));
        seed ^= seed >> 62;
        seed = seed * ACC_UINT64_C(0x5851f42d4c957f2d) + i;
    } while (i != 312);
    r->n = i;
}
#if 0
ACCLIB_PUBLIC(acc_uint32l_t, acc_randmt64) (acc_randmt64_p r)
{
    acc_uint64l_t v;
    v = (__ACCLIB_FUNCNAME(acc_randmt64_r64)(r)) >> 33;
    return (acc_uint32l_t) v;
}
#endif
ACCLIB_PUBLIC(acc_uint64l_t, acc_randmt64_r64) (acc_randmt64_p r)
{
    acc_uint64l_t v;
    if __acc_unlikely(r->n == 312) {
        unsigned i = 0, j;
        r->n = 0;
        do {
            j = i - 311; if ((int) j < 0) j += 312;
            v = (r->s[i] & ACC_UINT64_C(0xffffffff80000000)) ^ (r->s[j] & ACC_UINT64_C(0x7fffffff));
            j = i - 156; if ((int) j < 0) j += 312;
            r->s[i] = r->s[j] ^ (v >> 1);
            if (v & 1) r->s[i] ^= ACC_UINT64_C(0xb5026f5aa96619e9);
        } while (++i != 312);
    }
    v = r->s[r->n++];
    v ^= (v & ACC_UINT64_C(0xaaaaaaaaa0000000)) >> 29;
    v ^= (v & ACC_UINT64_C(0x38eb3ffff6d3)) << 17;
    v ^= (v & ACC_UINT64_C(0x7ffbf77)) << 37;
    return v ^ (v >> 43);
}
#endif
#endif
#if defined(ACC_WANT_ACCLIB_RDTSC)
#  undef ACC_WANT_ACCLIB_RDTSC
#define __ACCLIB_RDTSC_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif
#if defined(acc_int32e_t)
#if (ACC_OS_WIN32 && ACC_CC_PELLESC && (__POCC__ >= 290))
#  pragma warn(push)
#  pragma warn(disable:2007)
#endif
#if (ACC_ARCH_AMD64 || ACC_ARCH_I386) && (ACC_ASM_SYNTAX_GNUC)
#if (ACC_ARCH_AMD64 && ACC_CC_INTELC)
#  define __ACCLIB_RDTSC_REGS   : : "c" (t) : "memory", "rax", "rdx"
#elif (ACC_ARCH_AMD64)
#  define __ACCLIB_RDTSC_REGS   : : "c" (t) : "cc", "memory", "rax", "rdx"
#elif (ACC_ARCH_I386 && ACC_CC_GNUC && (ACC_CC_GNUC < 0x020000ul))
#  define __ACCLIB_RDTSC_REGS   : : "c" (t) : "ax", "dx"
#elif (ACC_ARCH_I386 && ACC_CC_INTELC)
#  define __ACCLIB_RDTSC_REGS   : : "c" (t) : "memory", "eax", "edx"
#else
#  define __ACCLIB_RDTSC_REGS   : : "c" (t) : "cc", "memory", "eax", "edx"
#endif
#endif
ACCLIB_PUBLIC(int, acc_tsc_read) (acc_uint32e_t* t)
{
#if (ACC_ARCH_AMD64 || ACC_ARCH_I386) && (ACC_ASM_SYNTAX_GNUC)
    __asm__ __volatile__(
        "clc \n" ".byte 0x0f,0x31\n"
        "movl %%eax,(%0)\n" "movl %%edx,4(%0)\n"
        __ACCLIB_RDTSC_REGS
    );
    return 0;
#elif (ACC_ARCH_I386) && (ACC_ASM_SYNTAX_MSC)
    ACC_UNUSED(t);
    __asm {
        mov ecx, t
        clc
#  if (ACC_CC_MSC && (_MSC_VER < 1200))
        _emit 0x0f
        _emit 0x31
#  else
        rdtsc
#  endif
        mov [ecx], eax
        mov [ecx+4], edx
    }
    return 0;
#else
    t[0] = t[1] = 0; return -1;
#endif
}
#if (ACC_OS_WIN32 && ACC_CC_PELLESC && (__POCC__ >= 290))
#  pragma warn(pop)
#endif
#endif
#endif
#if defined(ACC_WANT_ACCLIB_DOSALLOC)
#  undef ACC_WANT_ACCLIB_DOSALLOC
#define __ACCLIB_DOSALLOC_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif
#if (ACC_OS_OS216)
ACC_EXTERN_C unsigned short __far __pascal DosAllocHuge(unsigned short, unsigned short, unsigned short __far *, unsigned short, unsigned short);
ACC_EXTERN_C unsigned short __far __pascal DosFreeSeg(unsigned short);
#endif
#if (ACC_OS_DOS16 || ACC_OS_WIN16)
#if !(ACC_CC_AZTECC)
ACCLIB_PUBLIC(void __far*, acc_dos_alloc) (unsigned long size)
{
    void __far* p = 0;
    union REGS ri, ro;
    if ((long)size <= 0)
        return p;
    size = (size + 15) >> 4;
    if (size > 0xffffu)
        return p;
    ri.x.ax = 0x4800;
    ri.x.bx = (unsigned short) size;
    int86(0x21, &ri, &ro);
    if ((ro.x.cflag & 1) == 0)
        p = (void __far*) ACC_PTR_MK_FP(ro.x.ax, 0);
    return p;
}
ACCLIB_PUBLIC(int, acc_dos_free) (void __far* p)
{
    union REGS ri, ro;
    struct SREGS rs;
    if (!p)
        return 0;
    if (ACC_PTR_FP_OFF(p) != 0)
        return -1;
    segread(&rs);
    ri.x.ax = 0x4900;
    rs.es = ACC_PTR_FP_SEG(p);
    int86x(0x21, &ri, &ro, &rs);
    if (ro.x.cflag & 1)
        return -1;
    return 0;
}
#endif
#endif
#if (ACC_OS_OS216)
ACCLIB_PUBLIC(void __far*, acc_dos_alloc) (unsigned long size)
{
    void __far* p = 0;
    unsigned short sel = 0;
    if ((long)size <= 0)
        return p;
    if (DosAllocHuge((unsigned short)(size >> 16), (unsigned short)size, &sel, 0, 0) == 0)
        p = (void __far*) ACC_PTR_MK_FP(sel, 0);
    return p;
}
ACCLIB_PUBLIC(int, acc_dos_free) (void __far* p)
{
    if (!p)
        return 0;
    if (ACC_PTR_FP_OFF(p) != 0)
        return -1;
    if (DosFreeSeg(ACC_PTR_FP_SEG(p)) != 0)
        return -1;
    return 0;
}
#endif
#endif
#if defined(ACC_WANT_ACCLIB_GETOPT)
#  undef ACC_WANT_ACCLIB_GETOPT
#define __ACCLIB_GETOPT_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif
ACCLIB_PUBLIC(void, acc_getopt_init) (acc_getopt_p g,
                                      int start_argc, int argc, char** argv)
{
    memset(g, 0, sizeof(*g));
    g->optind = start_argc;
    g->argc = argc; g->argv = argv;
    g->optopt = -1;
}
static int __ACCLIB_FUNCNAME(acc_getopt_rotate) (char** p, int first, int middle, int last)
{
    int i = middle, n = middle - first;
    if (first >= middle || middle >= last) return 0;
    for (;;)
    {
        char* t = p[first]; p[first] = p[i]; p[i] = t;
        if (++first == middle)
        {
            if (++i == last) break;
            middle = i;
        }
        else if (++i == last)
            i = middle;
    }
    return n;
}
static int __ACCLIB_FUNCNAME(acc_getopt_perror) (acc_getopt_p g, int ret, const char* f, ...)
{
    if (g->opterr)
    {
#if (HAVE_STDARG_H)
        struct { va_list ap; } s;
        va_start(s.ap, f);
        g->opterr(g, f, &s);
        va_end(s.ap);
#else
        g->opterr(g, f, NULL);
#endif
    }
    ++g->errcount;
    return ret;
}
ACCLIB_PUBLIC(int, acc_getopt) (acc_getopt_p g,
                                const char* shortopts,
                                const acc_getopt_longopt_p longopts,
                                int* longind)
{
#define pe  __ACCLIB_FUNCNAME(acc_getopt_perror)
    int ordering = ACC_GETOPT_PERMUTE;
    int missing_arg_ret = g->bad_option;
    char* a;
    if (shortopts)
    {
        if (*shortopts == '-' || *shortopts == '+')
            ordering = *shortopts++ == '-' ? ACC_GETOPT_RETURN_IN_ORDER : ACC_GETOPT_REQUIRE_ORDER;
        if (*shortopts == ':')
            missing_arg_ret = *shortopts++;
    }
    g->optarg = NULL;
    if (g->optopt == -1)
        g->optopt = g->bad_option;
    if (longind)
        *longind = -1;
    if (g->eof)
        return -1;
    if (g->shortpos)
        goto acc_label_next_shortopt;
    g->optind -= __ACCLIB_FUNCNAME(acc_getopt_rotate)(g->argv, g->pending_rotate_first, g->pending_rotate_middle, g->optind);
    g->pending_rotate_first = g->pending_rotate_middle = g->optind;
    if (ordering == ACC_GETOPT_PERMUTE)
    {
        while (g->optind < g->argc && !(g->argv[g->optind][0] == '-' && g->argv[g->optind][1]))
            ++g->optind;
        g->pending_rotate_middle = g->optind;
    }
    if (g->optind >= g->argc)
    {
        g->optind = g->pending_rotate_first;
        goto acc_label_eof;
    }
    a = g->argv[g->optind];
    if (a[0] == '-' && a[1] == '-')
    {
        size_t l = 0;
        const acc_getopt_longopt_p o;
        const acc_getopt_longopt_p o1 = NULL;
        const acc_getopt_longopt_p o2 = NULL;
        int need_exact = 0;
        ++g->optind;
        if (!a[2])
            goto acc_label_eof;
        for (a += 2; a[l] && a[l] != '=' && a[l] != '#'; )
            ++l;
        for (o = longopts; l && o && o->name; ++o)
        {
            if (strncmp(a, o->name, l) != 0)
                continue;
            if (!o->name[l])
                goto acc_label_found_o;
            need_exact |= o->has_arg & ACC_GETOPT_EXACT_ARG;
            if (o1) o2 = o;
            else    o1 = o;
        }
        if (!o1 || need_exact)
            return pe(g, g->bad_option, "unrecognized option '--%s'", a);
        if (o2)
            return pe(g, g->bad_option, "option '--%s' is ambiguous (could be '--%s' or '--%s')", a, o1->name, o2->name);
        o = o1;
    acc_label_found_o:
        a += l;
        switch (o->has_arg & 0x2f)
        {
        case ACC_GETOPT_OPTIONAL_ARG:
            if (a[0])
                g->optarg = a + 1;
            break;
        case ACC_GETOPT_REQUIRED_ARG:
            if (a[0])
                g->optarg = a + 1;
            else if (g->optind < g->argc)
                g->optarg = g->argv[g->optind++];
            if (!g->optarg)
                return pe(g, missing_arg_ret, "option '--%s' requires an argument", o->name);
            break;
        case ACC_GETOPT_REQUIRED_ARG | 0x20:
            if (a[0] && a[1])
                g->optarg = a + 1;
            if (!g->optarg)
                return pe(g, missing_arg_ret, "option '--%s=' requires an argument", o->name);
            break;
        default:
            if (a[0])
                return pe(g, g->bad_option, "option '--%s' doesn't allow an argument", o->name);
            break;
        }
        if (longind)
            *longind = (int) (o - longopts);
        if (o->flag)
        {
            *o->flag = o->val;
            return 0;
        }
        return o->val;
    }
    if (a[0] == '-' && a[1])
    {
        unsigned char c;
        const char* s;
    acc_label_next_shortopt:
        a = g->argv[g->optind] + ++g->shortpos;
        c = (unsigned char) *a++; s = NULL;
        if (c != ':' && shortopts)
            s = strchr(shortopts, c);
        if (!s || s[1] != ':')
        {
            if (!a[0])
                ++g->optind, g->shortpos = 0;
            if (!s)
            {
                g->optopt = c;
                return pe(g, g->bad_option, "invalid option '-%c'", c);
            }
        }
        else
        {
            ++g->optind, g->shortpos = 0;
            if (a[0])
                g->optarg = a;
            else if (s[2] != ':')
            {
                if (g->optind < g->argc)
                    g->optarg = g->argv[g->optind++];
                else
                {
                    g->optopt = c;
                    return pe(g, missing_arg_ret, "option '-%c' requires an argument", c);
                }
            }
        }
        return c;
    }
    if (ordering == ACC_GETOPT_RETURN_IN_ORDER)
    {
        ++g->optind;
        g->optarg = a;
        return 1;
    }
acc_label_eof:
    g->optind -= __ACCLIB_FUNCNAME(acc_getopt_rotate)(g->argv, g->pending_rotate_first, g->pending_rotate_middle, g->optind);
    g->pending_rotate_first = g->pending_rotate_middle = g->optind;
    g->eof = 1;
    return -1;
#undef pe
}
#endif
#if defined(ACC_WANT_ACCLIB_HALLOC)
#  undef ACC_WANT_ACCLIB_HALLOC
#define __ACCLIB_HALLOC_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif
#if (ACC_HAVE_MM_HUGE_PTR)
#if 1 && (ACC_OS_DOS16 && defined(BLX286))
#  define __ACCLIB_HALLOC_USE_DAH 1
#elif 1 && (ACC_OS_DOS16 && defined(DOSX286))
#  define __ACCLIB_HALLOC_USE_DAH 1
#elif 1 && (ACC_OS_OS216)
#  define __ACCLIB_HALLOC_USE_DAH 1
#elif 1 && (ACC_OS_WIN16)
#  define __ACCLIB_HALLOC_USE_GA 1
#elif 1 && (ACC_OS_DOS16) && (ACC_CC_BORLANDC) && defined(__DPMI16__)
#  define __ACCLIB_HALLOC_USE_GA 1
#endif
#endif
#if (__ACCLIB_HALLOC_USE_DAH)
#if 0 && (ACC_OS_OS216)
#include <os2.h>
#else
ACC_EXTERN_C unsigned short __far __pascal DosAllocHuge(unsigned short, unsigned short, unsigned short __far *, unsigned short, unsigned short);
ACC_EXTERN_C unsigned short __far __pascal DosFreeSeg(unsigned short);
#endif
#endif
#if (__ACCLIB_HALLOC_USE_GA)
#if 0
#define STRICT 1
#include <windows.h>
#else
ACC_EXTERN_C const void __near* __far __pascal GlobalAlloc(unsigned, unsigned long);
ACC_EXTERN_C const void __near* __far __pascal GlobalFree(const void __near*);
ACC_EXTERN_C unsigned long __far __pascal GlobalHandle(unsigned);
ACC_EXTERN_C void __far* __far __pascal GlobalLock(const void __near*);
ACC_EXTERN_C int __far __pascal GlobalUnlock(const void __near*);
#endif
#endif
ACCLIB_PUBLIC(acc_hvoid_p, acc_halloc) (acc_hsize_t size)
{
    acc_hvoid_p p = 0;
    if (!(size > 0))
        return p;
#if 0 && defined(__palmos__)
    p = MemPtrNew(size);
#elif !(ACC_HAVE_MM_HUGE_PTR)
    if (size < (size_t) -1)
        p = malloc((size_t) size);
#else
    if ((long)size <= 0)
        return p;
{
#if (__ACCLIB_HALLOC_USE_DAH)
    unsigned short sel = 0;
    if (DosAllocHuge((unsigned short)(size >> 16), (unsigned short)size, &sel, 0, 0) == 0)
        p = (acc_hvoid_p) ACC_PTR_MK_FP(sel, 0);
#elif (__ACCLIB_HALLOC_USE_GA)
    const void __near* h = GlobalAlloc(2, size);
    if (h) {
        p = GlobalLock(h);
        if (p && ACC_PTR_FP_OFF(p) != 0) {
            GlobalUnlock(h);
            p = 0;
        }
        if (!p)
            GlobalFree(h);
    }
#elif (ACC_CC_MSC && (_MSC_VER >= 700))
    p = _halloc(size, 1);
#elif (ACC_CC_MSC || ACC_CC_WATCOMC)
    p = halloc(size, 1);
#elif (ACC_CC_DMC || ACC_CC_SYMANTECC || ACC_CC_ZORTECHC)
    p = farmalloc(size);
#elif (ACC_CC_BORLANDC || ACC_CC_TURBOC)
    p = farmalloc(size);
#elif (ACC_CC_AZTECC)
    p = lmalloc(size);
#else
    if (size < (size_t) -1)
        p = malloc((size_t) size);
#endif
}
#endif
    return p;
}
ACCLIB_PUBLIC(void, acc_hfree) (acc_hvoid_p p)
{
    if (!p)
        return;
#if 0 && defined(__palmos__)
    MemPtrFree(p);
#elif !(ACC_HAVE_MM_HUGE_PTR)
    free(p);
#else
#if (__ACCLIB_HALLOC_USE_DAH)
    if (ACC_PTR_FP_OFF(p) == 0)
        DosFreeSeg((unsigned short) ACC_PTR_FP_SEG(p));
#elif (__ACCLIB_HALLOC_USE_GA)
    if (ACC_PTR_FP_OFF(p) == 0) {
        const void __near* h = (const void __near*) (unsigned) GlobalHandle(ACC_PTR_FP_SEG(p));
        if (h) {
            GlobalUnlock(h);
            GlobalFree(h);
        }
    }
#elif (ACC_CC_MSC && (_MSC_VER >= 700))
    _hfree(p);
#elif (ACC_CC_MSC || ACC_CC_WATCOMC)
    hfree(p);
#elif (ACC_CC_DMC || ACC_CC_SYMANTECC || ACC_CC_ZORTECHC)
    farfree((void __far*) p);
#elif (ACC_CC_BORLANDC || ACC_CC_TURBOC)
    farfree((void __far*) p);
#elif (ACC_CC_AZTECC)
    lfree(p);
#else
    free(p);
#endif
#endif
}
#endif
#if defined(ACC_WANT_ACCLIB_HFREAD)
#  undef ACC_WANT_ACCLIB_HFREAD
#define __ACCLIB_HFREAD_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif
ACCLIB_PUBLIC(acc_hsize_t, acc_hfread) (void* vfp, acc_hvoid_p buf, acc_hsize_t size)
{
    FILE* fp = (FILE *) vfp;
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
        n = ACC_PTR_FP_OFF(b); n = (n <= 1) ? 0x8000u : (0u - n);
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
ACCLIB_PUBLIC(acc_hsize_t, acc_hfwrite) (void* vfp, const acc_hvoid_p buf, acc_hsize_t size)
{
    FILE* fp = (FILE *) vfp;
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
        n = ACC_PTR_FP_OFF(b); n = (n <= 1) ? 0x8000u : (0u - n);
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
#endif
#if defined(ACC_WANT_ACCLIB_HSREAD)
#  undef ACC_WANT_ACCLIB_HSREAD
#define __ACCLIB_HSREAD_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif
ACCLIB_PUBLIC(long, acc_safe_hread) (int fd, acc_hvoid_p buf, long size)
{
    acc_hbyte_p b = (acc_hbyte_p) buf;
    long l = 0;
    int saved_errno;
    saved_errno = errno;
    while (l < size)
    {
        long n = size - l;
#if (ACC_HAVE_MM_HUGE_PTR)
#  define __ACCLIB_REQUIRE_HREAD_CH 1
        errno = 0; n = acc_hread(fd, b, n);
#elif (ACC_OS_DOS32) && defined(__DJGPP__)
        errno = 0; n = _read(fd, b, n);
#else
        errno = 0; n = read(fd, b, n);
#endif
        if (n == 0)
            break;
        if (n < 0) {
#if defined(EAGAIN)
            if (errno == (EAGAIN)) continue;
#endif
#if defined(EINTR)
            if (errno == (EINTR)) continue;
#endif
            if (errno == 0) errno = 1;
            return l;
        }
        b += n; l += n;
    }
    errno = saved_errno;
    return l;
}
ACCLIB_PUBLIC(long, acc_safe_hwrite) (int fd, const acc_hvoid_p buf, long size)
{
    const acc_hbyte_p b = (const acc_hbyte_p) buf;
    long l = 0;
    int saved_errno;
    saved_errno = errno;
    while (l < size)
    {
        long n = size - l;
#if (ACC_HAVE_MM_HUGE_PTR)
#  define __ACCLIB_REQUIRE_HREAD_CH 1
        errno = 0; n = acc_hwrite(fd, b, n);
#elif (ACC_OS_DOS32) && defined(__DJGPP__)
        errno = 0; n = _write(fd, b, n);
#else
        errno = 0; n = write(fd, b, n);
#endif
        if (n == 0)
            break;
        if (n < 0) {
#if defined(EAGAIN)
            if (errno == (EAGAIN)) continue;
#endif
#if defined(EINTR)
            if (errno == (EINTR)) continue;
#endif
            if (errno == 0) errno = 1;
            return l;
        }
        b += n; l += n;
    }
    errno = saved_errno;
    return l;
}
#endif
#if defined(ACC_WANT_ACCLIB_PCLOCK)
#  undef ACC_WANT_ACCLIB_PCLOCK
#define __ACCLIB_PCLOCK_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif
#if 1 && (ACC_OS_POSIX_LINUX && ACC_ARCH_AMD64 && ACC_ASM_SYNTAX_GNUC)
#ifndef acc_pclock_syscall_clock_gettime
#define acc_pclock_syscall_clock_gettime acc_pclock_syscall_clock_gettime
#endif
static __acc_noinline long acc_pclock_syscall_clock_gettime(long clockid, struct timespec *ts)
{
    unsigned long r = 228;
    __asm__ __volatile__("syscall\n" : "=a" (r) : "0" (r), "D" (clockid), "S" (ts) : __ACC_ASM_CLOBBER);
    return (long) r;
}
#endif
#if 1 && (ACC_OS_POSIX_LINUX && ACC_ARCH_I386 && ACC_ASM_SYNTAX_GNUC) && defined(acc_int64l_t)
#ifndef acc_pclock_syscall_clock_gettime
#define acc_pclock_syscall_clock_gettime acc_pclock_syscall_clock_gettime
#endif
static __acc_noinline long acc_pclock_syscall_clock_gettime(long clockid, struct timespec *ts)
{
    unsigned long r = 265;
    __asm__ __volatile__("pushl %%ebx\n pushl %%edx\n popl %%ebx\n int $0x80\n popl %%ebx\n" : "=a" (r) : "0" (r), "d" (clockid), "c" (ts) : __ACC_ASM_CLOBBER);
    return (long) r;
}
#endif
#if 0 && defined(acc_pclock_syscall_clock_gettime)
#ifndef acc_pclock_read_clock_gettime_r_syscall
#define acc_pclock_read_clock_gettime_r_syscall acc_pclock_read_clock_gettime_r_syscall
#endif
static int acc_pclock_read_clock_gettime_r_syscall(acc_pclock_handle_p h, acc_pclock_p c)
{
     struct timespec ts;
    if (acc_pclock_syscall_clock_gettime(0, &ts) != 0)
        return -1;
    c->tv_sec = ts.tv_sec;
    c->tv_nsec = (acc_uint32l_t) ts.tv_nsec;
    ACC_UNUSED(h); return 0;
}
#endif
#if (HAVE_GETTIMEOFDAY)
#ifndef acc_pclock_read_gettimeofday
#define acc_pclock_read_gettimeofday acc_pclock_read_gettimeofday
#endif
static int acc_pclock_read_gettimeofday(acc_pclock_handle_p h, acc_pclock_p c)
{
    struct timeval tv;
    if (gettimeofday(&tv, 0) != 0)
        return -1;
#if defined(acc_int64l_t)
    c->tv_sec = tv.tv_sec;
#else
    c->tv_sec_high = 0;
    c->tv_sec_low = tv.tv_sec;
#endif
    c->tv_nsec = (acc_uint32l_t) (tv.tv_usec * 1000u);
    ACC_UNUSED(h); return 0;
}
#endif
#if defined(CLOCKS_PER_SEC) && !(ACC_CFG_NO_DOUBLE)
#ifndef acc_pclock_read_clock
#define acc_pclock_read_clock acc_pclock_read_clock
#endif
static int acc_pclock_read_clock(acc_pclock_handle_p h, acc_pclock_p c)
{
    clock_t ticks;
    double secs;
#if defined(acc_int64l_t)
    acc_uint64l_t nsecs;
    ticks = clock();
    secs = (double)ticks / (CLOCKS_PER_SEC);
    nsecs = (acc_uint64l_t) (secs * 1000000000.0);
    c->tv_sec = (acc_int64l_t) (nsecs / 1000000000ul);
    nsecs = (nsecs % 1000000000ul);
    c->tv_nsec = (acc_uint32l_t) nsecs;
#else
    ticks = clock();
    secs = (double)ticks / (CLOCKS_PER_SEC);
    c->tv_sec_high = 0;
    c->tv_sec_low = (acc_uint32l_t) (secs + 0.5);
    c->tv_nsec = 0;
#endif
    ACC_UNUSED(h); return 0;
}
#endif
#if 1 && defined(acc_pclock_syscall_clock_gettime)
#ifndef acc_pclock_read_clock_gettime_m_syscall
#define acc_pclock_read_clock_gettime_m_syscall acc_pclock_read_clock_gettime_m_syscall
#endif
static int acc_pclock_read_clock_gettime_m_syscall(acc_pclock_handle_p h, acc_pclock_p c)
{
     struct timespec ts;
    if (acc_pclock_syscall_clock_gettime(1, &ts) != 0)
        return -1;
    c->tv_sec = ts.tv_sec;
    c->tv_nsec = (acc_uint32l_t) ts.tv_nsec;
    ACC_UNUSED(h); return 0;
}
#endif
#if (ACC_OS_DOS32 && ACC_CC_GNUC) && defined(__DJGPP__) && defined(UCLOCKS_PER_SEC) && !(ACC_CFG_NO_DOUBLE)
#ifndef acc_pclock_read_uclock
#define acc_pclock_read_uclock acc_pclock_read_uclock
#endif
static int acc_pclock_read_uclock(acc_pclock_handle_p h, acc_pclock_p c)
{
    acc_uint64l_t ticks;
    double secs;
    acc_uint64l_t nsecs;
    ticks = uclock();
    secs = (double)ticks / (UCLOCKS_PER_SEC);
    nsecs = (acc_uint64l_t) (secs * 1000000000.0);
    c->tv_sec = nsecs / 1000000000ul;
    c->tv_nsec = (acc_uint32l_t) (nsecs % 1000000000ul);
    ACC_UNUSED(h); return 0;
}
#endif
#if 1 && (HAVE_CLOCK_GETTIME) && defined(CLOCK_PROCESS_CPUTIME_ID) && defined(acc_int64l_t)
#ifndef acc_pclock_read_clock_gettime_p_libc
#define acc_pclock_read_clock_gettime_p_libc acc_pclock_read_clock_gettime_p_libc
#endif
static int acc_pclock_read_clock_gettime_p_libc(acc_pclock_handle_p h, acc_pclock_p c)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts) != 0)
        return -1;
    c->tv_sec = ts.tv_sec;
    c->tv_nsec = (acc_uint32l_t) ts.tv_nsec;
    ACC_UNUSED(h); return 0;
}
#endif
#if 1 && defined(acc_pclock_syscall_clock_gettime)
#ifndef acc_pclock_read_clock_gettime_p_syscall
#define acc_pclock_read_clock_gettime_p_syscall acc_pclock_read_clock_gettime_p_syscall
#endif
static int acc_pclock_read_clock_gettime_p_syscall(acc_pclock_handle_p h, acc_pclock_p c)
{
     struct timespec ts;
    if (acc_pclock_syscall_clock_gettime(2, &ts) != 0)
        return -1;
    c->tv_sec = ts.tv_sec;
    c->tv_nsec = (acc_uint32l_t) ts.tv_nsec;
    ACC_UNUSED(h); return 0;
}
#endif
#if (ACC_OS_CYGWIN || ACC_OS_WIN32 || ACC_OS_WIN64) && (ACC_HAVE_WINDOWS_H) && defined(acc_int64l_t)
#ifndef acc_pclock_read_getprocesstimes
#define acc_pclock_read_getprocesstimes acc_pclock_read_getprocesstimes
#endif
static int acc_pclock_read_getprocesstimes(acc_pclock_handle_p h, acc_pclock_p c)
{
    FILETIME ct, et, kt, ut;
    acc_uint64l_t ticks;
    if (GetProcessTimes(GetCurrentProcess(), &ct, &et, &kt, &ut) == 0)
        return -1;
    ticks = ((acc_uint64l_t)ut.dwHighDateTime << 32) | ut.dwLowDateTime;
    if __acc_unlikely(h->ticks_base == 0)
        h->ticks_base = ticks;
    else
        ticks -= h->ticks_base;
    c->tv_sec = (acc_int64l_t) (ticks / 10000000ul);
    ticks = (ticks % 10000000ul) * 100u;
    c->tv_nsec = (acc_uint32l_t) ticks;
    ACC_UNUSED(h); return 0;
}
#endif
#if (HAVE_GETRUSAGE) && defined(RUSAGE_SELF)
#ifndef acc_pclock_read_getrusage
#define acc_pclock_read_getrusage acc_pclock_read_getrusage
#endif
static int acc_pclock_read_getrusage(acc_pclock_handle_p h, acc_pclock_p c)
{
    struct rusage ru;
    if (getrusage(RUSAGE_SELF, &ru) != 0)
        return -1;
#if defined(acc_int64l_t)
    c->tv_sec = ru.ru_utime.tv_sec;
#else
    c->tv_sec_high = 0;
    c->tv_sec_low = ru.ru_utime.tv_sec;
#endif
    c->tv_nsec = (acc_uint32l_t) (ru.ru_utime.tv_usec * 1000u);
    ACC_UNUSED(h); return 0;
}
#endif
#if 1 && (HAVE_CLOCK_GETTIME) && defined(CLOCK_THREAD_CPUTIME_ID) && defined(acc_int64l_t)
#ifndef acc_pclock_read_clock_gettime_t_libc
#define acc_pclock_read_clock_gettime_t_libc acc_pclock_read_clock_gettime_t_libc
#endif
static int acc_pclock_read_clock_gettime_t_libc(acc_pclock_handle_p h, acc_pclock_p c)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts) != 0)
        return -1;
    c->tv_sec = ts.tv_sec;
    c->tv_nsec = (acc_uint32l_t) ts.tv_nsec;
    ACC_UNUSED(h); return 0;
}
#endif
#if 1 && defined(acc_pclock_syscall_clock_gettime)
#ifndef acc_pclock_read_clock_gettime_t_syscall
#define acc_pclock_read_clock_gettime_t_syscall acc_pclock_read_clock_gettime_t_syscall
#endif
static int acc_pclock_read_clock_gettime_t_syscall(acc_pclock_handle_p h, acc_pclock_p c)
{
     struct timespec ts;
    if (acc_pclock_syscall_clock_gettime(3, &ts) != 0)
        return -1;
    c->tv_sec = ts.tv_sec;
    c->tv_nsec = (acc_uint32l_t) ts.tv_nsec;
    ACC_UNUSED(h); return 0;
}
#endif
#if (ACC_OS_CYGWIN || ACC_OS_WIN32 || ACC_OS_WIN64) && (ACC_HAVE_WINDOWS_H) && defined(acc_int64l_t)
#ifndef acc_pclock_read_getthreadtimes
#define acc_pclock_read_getthreadtimes acc_pclock_read_getthreadtimes
#endif
static int acc_pclock_read_getthreadtimes(acc_pclock_handle_p h, acc_pclock_p c)
{
    FILETIME ct, et, kt, ut;
    acc_uint64l_t ticks;
    if (GetThreadTimes(GetCurrentThread(), &ct, &et, &kt, &ut) == 0)
        return -1;
    ticks = ((acc_uint64l_t)ut.dwHighDateTime << 32) | ut.dwLowDateTime;
    if __acc_unlikely(h->ticks_base == 0)
        h->ticks_base = ticks;
    else
        ticks -= h->ticks_base;
    c->tv_sec = (acc_int64l_t) (ticks / 10000000ul);
    ticks = (ticks % 10000000ul) * 100u;
    c->tv_nsec = (acc_uint32l_t) ticks;
    ACC_UNUSED(h); return 0;
}
#endif
ACCLIB_PUBLIC(int, acc_pclock_open) (acc_pclock_handle_p h, int mode)
{
    acc_pclock_t c;
    int i;
    h->h = (acclib_handle_t) 0;
    h->mode = -1;
    h->read_error = 2;
    h->name = NULL;
    h->gettime = 0;
#if defined(acc_int64l_t)
    h->ticks_base = 0;
#endif
    switch (mode)
    {
    case ACC_PCLOCK_REALTIME:
#     if defined(acc_pclock_read_clock_gettime_r_syscall)
        if (acc_pclock_read_clock_gettime_r_syscall(h, &c) == 0) {
            h->gettime = acc_pclock_read_clock_gettime_r_syscall;
            h->name = "CLOCK_REALTIME/syscall";
            break;
        }
#     endif
#     if defined(acc_pclock_read_gettimeofday)
        if (acc_pclock_read_gettimeofday(h, &c) == 0) {
            h->gettime = acc_pclock_read_gettimeofday;
            h->name = "gettimeofday";
            break;
        }
#     endif
        break;
    case ACC_PCLOCK_MONOTONIC:
#     if defined(acc_pclock_read_clock_gettime_m_syscall)
        if (acc_pclock_read_clock_gettime_m_syscall(h, &c) == 0) {
            h->gettime = acc_pclock_read_clock_gettime_m_syscall;
            h->name = "CLOCK_MONOTONIC/syscall";
            break;
        }
#     endif
#     if defined(acc_pclock_read_uclock)
        if (acc_pclock_read_uclock(h, &c) == 0) {
            h->gettime = acc_pclock_read_uclock;
            h->name = "uclock";
            break;
        }
#     endif
#     if defined(acc_pclock_read_clock)
        if (acc_pclock_read_clock(h, &c) == 0) {
            h->gettime = acc_pclock_read_clock;
            h->name = "clock";
            break;
        }
#     endif
        break;
    case ACC_PCLOCK_PROCESS_CPUTIME_ID:
#     if defined(acc_pclock_read_getprocesstimes)
        if (acc_pclock_read_getprocesstimes(h, &c) == 0) {
            h->gettime = acc_pclock_read_getprocesstimes;
            h->name = "GetProcessTimes";
            break;
        }
#     endif
#     if defined(acc_pclock_read_clock_gettime_p_syscall)
        if (acc_pclock_read_clock_gettime_p_syscall(h, &c) == 0) {
            h->gettime = acc_pclock_read_clock_gettime_p_syscall;
            h->name = "CLOCK_PROCESS_CPUTIME_ID/syscall";
            break;
        }
#     endif
#     if defined(acc_pclock_read_clock_gettime_p_libc)
        if (acc_pclock_read_clock_gettime_p_libc(h, &c) == 0) {
            h->gettime = acc_pclock_read_clock_gettime_p_libc;
            h->name = "CLOCK_PROCESS_CPUTIME_ID/libc";
            break;
        }
#     endif
#     if defined(acc_pclock_read_getrusage)
        if (acc_pclock_read_getrusage(h, &c) == 0) {
            h->gettime = acc_pclock_read_getrusage;
            h->name = "getrusage";
            break;
        }
#     endif
        break;
    case ACC_PCLOCK_THREAD_CPUTIME_ID:
#     if defined(acc_pclock_read_getthreadtimes)
        if (acc_pclock_read_getthreadtimes(h, &c) == 0) {
            h->gettime = acc_pclock_read_getthreadtimes;
            h->name = "GetThreadTimes";
        }
#     endif
#     if defined(acc_pclock_read_clock_gettime_t_syscall)
        if (acc_pclock_read_clock_gettime_t_syscall(h, &c) == 0) {
            h->gettime = acc_pclock_read_clock_gettime_t_syscall;
            h->name = "CLOCK_THREAD_CPUTIME_ID/syscall";
            break;
        }
#     endif
#     if defined(acc_pclock_read_clock_gettime_t_libc)
        if (acc_pclock_read_clock_gettime_t_libc(h, &c) == 0) {
            h->gettime = acc_pclock_read_clock_gettime_t_libc;
            h->name = "CLOCK_THREAD_CPUTIME_ID/libc";
            break;
        }
#     endif
        break;
    }
    if (!h->gettime)
        return -1;
    if (!h->h)
        h->h = (acclib_handle_t) 1;
    h->mode = mode;
    h->read_error = 0;
    if (!h->name)
        h->name = "unknown";
    for (i = 0; i < 10; i++) {
        __ACCLIB_FUNCNAME(acc_pclock_read)(h, &c);
    }
    return 0;
}
ACCLIB_PUBLIC(int, acc_pclock_open_default) (acc_pclock_handle_p h)
{
    if (__ACCLIB_FUNCNAME(acc_pclock_open)(h, ACC_PCLOCK_PROCESS_CPUTIME_ID) == 0)
        return 0;
    if (__ACCLIB_FUNCNAME(acc_pclock_open)(h, ACC_PCLOCK_MONOTONIC) == 0)
        return 0;
    if (__ACCLIB_FUNCNAME(acc_pclock_open)(h, ACC_PCLOCK_REALTIME) == 0)
        return 0;
    if (__ACCLIB_FUNCNAME(acc_pclock_open)(h, ACC_PCLOCK_THREAD_CPUTIME_ID) == 0)
        return 0;
    return -1;
}
ACCLIB_PUBLIC(int, acc_pclock_close) (acc_pclock_handle_p h)
{
    h->h = (acclib_handle_t) 0;
    h->mode = -1;
    h->name = NULL;
    h->gettime = 0;
    return 0;
}
ACCLIB_PUBLIC(void, acc_pclock_read) (acc_pclock_handle_p h, acc_pclock_p c)
{
    if (h->gettime) {
        if (h->gettime(h, c) == 0)
            return;
    }
    h->read_error = 1;
#if defined(acc_int64l_t)
    c->tv_sec = 0;
#else
    c->tv_sec_high = 0;
    c->tv_sec_low = 0;
#endif
    c->tv_nsec = 0;
}
#if !(ACC_CFG_NO_DOUBLE)
ACCLIB_PUBLIC(double, acc_pclock_get_elapsed) (acc_pclock_handle_p h, const acc_pclock_p start, const acc_pclock_p stop)
{
    double tstop, tstart;
    if (!h->h) {
        h->mode = -1;
        return 0.0;
    }
#if defined(acc_int64l_t)
    tstop  = stop->tv_sec  + stop->tv_nsec  / 1000000000.0;
    tstart = start->tv_sec + start->tv_nsec / 1000000000.0;
#else
    tstop  = stop->tv_sec_low  + stop->tv_nsec  / 1000000000.0;
    tstart = start->tv_sec_low + start->tv_nsec / 1000000000.0;
#endif
    return tstop - tstart;
}
#endif
ACCLIB_PUBLIC(int, acc_pclock_flush_cpu_cache) (acc_pclock_handle_p h, unsigned flags)
{
    ACC_UNUSED(h); ACC_UNUSED(flags);
    return -1;
}
#if defined(__ACCLIB_PCLOCK_NEED_WARN_POP)
#  if (ACC_CC_MSC && (_MSC_VER >= 1200))
#    pragma warning(pop)
#  else
#    error "__ACCLIB_PCLOCK_NEED_WARN_POP"
#  endif
#  undef __ACCLIB_PCLOCK_NEED_WARN_POP
#endif
#endif
#if defined(ACC_WANT_ACCLIB_MISC)
#  undef ACC_WANT_ACCLIB_MISC
#define __ACCLIB_MISC_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)                r __ACCLIB_FUNCNAME(f)
#endif
#if !defined(ACCLIB_PUBLIC_NOINLINE)
#  if !defined(__acc_noinline)
#    define ACCLIB_PUBLIC_NOINLINE(r,f)     r __ACCLIB_FUNCNAME(f)
#  elif (ACC_CC_CLANG || (ACC_CC_GNUC >= 0x030400ul) || ACC_CC_LLVM)
#    define ACCLIB_PUBLIC_NOINLINE(r,f)     __acc_noinline __attribute__((__used__)) r __ACCLIB_FUNCNAME(f)
#  else
#    define ACCLIB_PUBLIC_NOINLINE(r,f)     __acc_noinline r __ACCLIB_FUNCNAME(f)
#  endif
#endif
#if (ACC_OS_WIN32 && ACC_CC_PELLESC && (__POCC__ >= 290))
#  pragma warn(push)
#  pragma warn(disable:2007)
#endif
ACCLIB_PUBLIC(const char *, acc_getenv) (const char *s)
{
#if (HAVE_GETENV)
    return getenv(s);
#else
    ACC_UNUSED(s); return (const char *) 0;
#endif
}
ACCLIB_PUBLIC(acclib_handle_t, acc_get_osfhandle) (int fd)
{
    if (fd < 0)
        return -1;
#if (ACC_OS_CYGWIN)
    return get_osfhandle(fd);
#elif (ACC_OS_EMX && defined(__RSXNT__))
    return -1;
#elif (ACC_OS_WIN32 && ACC_CC_GNUC) && defined(__PW32__)
    return -1;
#elif (ACC_OS_WIN32 || ACC_OS_WIN64)
# if (ACC_CC_PELLESC && (__POCC__ < 280))
    return -1;
# elif (ACC_CC_WATCOMC && (__WATCOMC__ < 1000))
    return -1;
# elif (ACC_CC_WATCOMC && (__WATCOMC__ < 1100))
    return _os_handle(fd);
# else
    return _get_osfhandle(fd);
# endif
#else
    return fd;
#endif
}
ACCLIB_PUBLIC(int, acc_set_binmode) (int fd, int binary)
{
#if (ACC_ARCH_M68K && ACC_OS_TOS && ACC_CC_GNUC) && defined(__MINT__)
    FILE* fp; int old_binary;
    if (fd == STDIN_FILENO) fp = stdin;
    else if (fd == STDOUT_FILENO) fp = stdout;
    else if (fd == STDERR_FILENO) fp = stderr;
    else return -1;
    old_binary = fp->__mode.__binary;
    __set_binmode(fp, binary ? 1 : 0);
    return old_binary ? 1 : 0;
#elif (ACC_ARCH_M68K && ACC_OS_TOS)
    ACC_UNUSED(fd); ACC_UNUSED(binary);
    return -1;
#elif (ACC_OS_DOS16 && (ACC_CC_AZTECC || ACC_CC_PACIFICC))
    ACC_UNUSED(fd); ACC_UNUSED(binary);
    return -1;
#elif (ACC_OS_DOS32 && ACC_CC_GNUC) && defined(__DJGPP__)
    int r; unsigned old_flags = __djgpp_hwint_flags;
    ACC_COMPILE_TIME_ASSERT(O_BINARY > 0)
    ACC_COMPILE_TIME_ASSERT(O_TEXT > 0)
    if (fd < 0) return -1;
    r = setmode(fd, binary ? O_BINARY : O_TEXT);
    if ((old_flags & 1u) != (__djgpp_hwint_flags & 1u))
        __djgpp_set_ctrl_c(!(old_flags & 1));
    if (r == -1) return -1;
    return (r & O_TEXT) ? 0 : 1;
#elif (ACC_OS_WIN32 && ACC_CC_GNUC) && defined(__PW32__)
    if (fd < 0) return -1;
    ACC_UNUSED(binary);
    return 1;
#elif (ACC_OS_DOS32 && ACC_CC_HIGHC)
    FILE* fp; int r;
    if (fd == fileno(stdin)) fp = stdin;
    else if (fd == fileno(stdout)) fp = stdout;
    else if (fd == fileno(stderr)) fp = stderr;
    else return -1;
    r = _setmode(fp, binary ? _BINARY : _TEXT);
    if (r == -1) return -1;
    return (r & _BINARY) ? 1 : 0;
#elif (ACC_OS_WIN32 && ACC_CC_MWERKS) && defined(__MSL__)
    ACC_UNUSED(fd); ACC_UNUSED(binary);
    return -1;
#elif (ACC_OS_CYGWIN && (ACC_CC_GNUC < 0x025a00ul))
    ACC_UNUSED(fd); ACC_UNUSED(binary);
    return -1;
#elif (ACC_OS_CYGWIN || ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_EMX || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)
    int r;
#if !(ACC_CC_ZORTECHC)
    ACC_COMPILE_TIME_ASSERT(O_BINARY > 0)
#endif
    ACC_COMPILE_TIME_ASSERT(O_TEXT > 0)
    if (fd < 0) return -1;
    r = setmode(fd, binary ? O_BINARY : O_TEXT);
    if (r == -1) return -1;
    return (r & O_TEXT) ? 0 : 1;
#else
    if (fd < 0) return -1;
    ACC_UNUSED(binary);
    return 1;
#endif
}
ACCLIB_PUBLIC(int, acc_isatty) (int fd)
{
    if (fd < 0)
        return 0;
#if (ACC_OS_DOS16 && !(ACC_CC_AZTECC))
    {
        union REGS ri, ro;
        ri.x.ax = 0x4400; ri.x.bx = fd;
        int86(0x21, &ri, &ro);
        if ((ro.x.cflag & 1) == 0)
            if ((ro.x.ax & 0x83) != 0x83)
                return 0;
    }
#elif (ACC_OS_DOS32 && ACC_CC_WATCOMC)
    {
        union REGS ri, ro;
        ri.w.ax = 0x4400; ri.w.bx = (unsigned short) fd;
        int386(0x21, &ri, &ro);
        if ((ro.w.cflag & 1) == 0)
            if ((ro.w.ax & 0x83) != 0x83)
                return 0;
    }
#elif (ACC_HAVE_WINDOWS_H)
    {
        acclib_handle_t h = __ACCLIB_FUNCNAME(acc_get_osfhandle)(fd);
        if ((HANDLE)h != INVALID_HANDLE_VALUE)
        {
            DWORD d = 0;
            if (GetConsoleMode((HANDLE)h, &d) == 0)
                return 0;
        }
    }
#endif
#if (HAVE_ISATTY)
    return (isatty(fd)) ? 1 : 0;
#else
    return 0;
#endif
}
ACCLIB_PUBLIC(int, acc_mkdir) (const char* name, unsigned mode)
{
#if !(HAVE_MKDIR)
    ACC_UNUSED(name); ACC_UNUSED(mode);
    return -1;
#elif (ACC_ARCH_M68K && ACC_OS_TOS && (ACC_CC_PUREC || ACC_CC_TURBOC))
    ACC_UNUSED(mode);
    return Dcreate(name);
#elif (ACC_OS_DOS32 && ACC_CC_GNUC) && defined(__DJGPP__)
    return mkdir(name, mode);
#elif (ACC_OS_WIN32 && ACC_CC_GNUC) && defined(__PW32__)
    return mkdir(name, mode);
#elif (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)
    ACC_UNUSED(mode);
# if (ACC_CC_HIGHC || ACC_CC_PACIFICC)
    return mkdir((char*) name);
# else
    return mkdir(name);
# endif
#elif (ACC_CC_WATCOMC)
    return mkdir(name, (mode_t) mode);
#else
    return mkdir(name, mode);
#endif
}
ACCLIB_PUBLIC(int, acc_rmdir) (const char* name)
{
#if !(HAVE_RMDIR)
    ACC_UNUSED(name);
    return -1;
#elif ((ACC_OS_DOS16 || ACC_OS_DOS32) && (ACC_CC_HIGHC || ACC_CC_PACIFICC))
    return rmdir((char *) name);
#else
    return rmdir(name);
#endif
}
#if defined(acc_int32e_t)
ACCLIB_PUBLIC(acc_int32e_t, acc_muldiv32s) (acc_int32e_t a, acc_int32e_t b, acc_int32e_t x)
{
    acc_int32e_t r = 0;
    if __acc_likely(x != 0)
    {
#if defined(acc_int64l_t)
        acc_int64l_t rr = (((acc_int64l_t) a * b) / x);
        r = (acc_int32e_t) rr;
#else
        ACC_UNUSED(a); ACC_UNUSED(b);
#endif
    }
    return r;
}
ACCLIB_PUBLIC(acc_uint32e_t, acc_muldiv32u) (acc_uint32e_t a, acc_uint32e_t b, acc_uint32e_t x)
{
    acc_uint32e_t r = 0;
    if __acc_likely(x != 0)
    {
#if defined(acc_int64l_t)
        acc_uint64l_t rr = (((acc_uint64l_t) a * b) / x);
        r = (acc_uint32e_t) rr;
#else
        ACC_UNUSED(a); ACC_UNUSED(b);
#endif
    }
    return r;
}
#endif
#if 0
ACCLIB_PUBLIC_NOINLINE(int, acc_syscall_clock_gettime) (int c)
{
}
#endif
#if (ACC_OS_WIN16)
ACC_EXTERN_C void __far __pascal DebugBreak(void);
#endif
ACCLIB_PUBLIC_NOINLINE(void, acc_debug_break) (void)
{
#if (ACC_OS_WIN16)
    DebugBreak();
#elif (ACC_ARCH_I086)
#elif (ACC_OS_WIN64) && (ACC_HAVE_WINDOWS_H)
    DebugBreak();
#elif (ACC_ARCH_AMD64 || ACC_ARCH_I386) && (ACC_ASM_SYNTAX_GNUC)
    __asm__ __volatile__("int $3\n" : : : __ACC_ASM_CLOBBER);
#elif (ACC_ARCH_I386) && (ACC_ASM_SYNTAX_MSC)
    __asm { int 3 }
#elif (ACC_OS_WIN32) && (ACC_HAVE_WINDOWS_H)
    DebugBreak();
#else
    volatile acc_uintptr_t a = 0;
    * (volatile unsigned long *) (volatile void *) a = ~0ul;
#endif
}
ACCLIB_PUBLIC_NOINLINE(void, acc_debug_nop) (void)
{
}
ACCLIB_PUBLIC_NOINLINE(int, acc_debug_align_check_query) (void)
{
#if (ACC_ARCH_AMD64 || ACC_ARCH_I386) && (ACC_ASM_SYNTAX_GNUC)
# if (ACC_ARCH_AMD64)
    acc_uint64e_t r = 0;
# else
    size_t r = 0;
# endif
    __asm__ __volatile__("pushf\n pop %0\n" : "=a" (r) : : __ACC_ASM_CLOBBER);
    return (int)(r >> 18) & 1;
#elif (ACC_ARCH_I386) && (ACC_ASM_SYNTAX_MSC)
    unsigned long r;
    __asm {
        pushf
        pop eax
        mov r,eax
    }
    return (int)(r >> 18) & 1;
#else
    return -1;
#endif
}
ACCLIB_PUBLIC_NOINLINE(int, acc_debug_align_check_enable) (int v)
{
#if (ACC_ARCH_AMD64) && (ACC_ASM_SYNTAX_GNUC)
    if (v) {
        __asm__ __volatile__("pushf\n orl $262144,(%%rsp)\n popf\n" : : : __ACC_ASM_CLOBBER);
    } else {
        __asm__ __volatile__("pushf\n andl $-262145,(%%rsp)\n popf\n" : : : __ACC_ASM_CLOBBER);
    }
    return 0;
#elif (ACC_ARCH_I386) && (ACC_ASM_SYNTAX_GNUC)
    if (v) {
        __asm__ __volatile__("pushf\n orl $262144,(%%esp)\n popf\n" : : : __ACC_ASM_CLOBBER);
    } else {
        __asm__ __volatile__("pushf\n andl $-262145,(%%esp)\n popf\n" : : : __ACC_ASM_CLOBBER);
    }
    return 0;
#elif (ACC_ARCH_I386) && (ACC_ASM_SYNTAX_MSC)
    if (v) { __asm {
        pushf
        or dword ptr [esp],262144
        popf
    }} else { __asm {
        pushf
        and dword ptr [esp],-262145
        popf
    }}
    return 0;
#else
    ACC_UNUSED(v); return -1;
#endif
}
ACCLIB_PUBLIC_NOINLINE(unsigned, acc_debug_running_on_qemu) (void)
{
    unsigned r = 0;
#if (ACC_OS_POSIX_LINUX || ACC_OS_WIN32 || ACC_OS_WIN64)
    const char* p;
    p = __ACCLIB_FUNCNAME(acc_getenv)(ACC_PP_STRINGIZE(ACC_ENV_RUNNING_ON_QEMU));
    if (p) {
        if (p[0] == 0) r = 0;
        else if ((p[0] >= '0' && p[0] <= '9') && p[1] == 0) r = p[0] - '0';
        else r = 1;
    }
#endif
    return r;
}
ACCLIB_PUBLIC_NOINLINE(unsigned, acc_debug_running_on_valgrind) (void)
{
#if (ACC_ARCH_AMD64 && ACC_ABI_ILP32)
    return 0;
#elif (ACC_ARCH_AMD64 || ACC_ARCH_I386) && (ACC_ASM_SYNTAX_GNUC)
    volatile size_t a[6];
    size_t r = 0;
    a[0] = 0x1001; a[1] = 0; a[2] = 0; a[3] = 0; a[4] = 0; a[5] = 0;
#  if (ACC_ARCH_AMD64)
    __asm__ __volatile__(".byte 0x48,0xc1,0xc7,0x03,0x48,0xc1,0xc7,0x0d,0x48,0xc1,0xc7,0x3d,0x48,0xc1,0xc7,0x33,0x48,0x87,0xdb\n" : "=d" (r) : "a" (&a[0]), "d" (r) : __ACC_ASM_CLOBBER);
#  elif (ACC_ARCH_I386)
    __asm__ __volatile__(".byte 0xc1,0xc7,0x03,0xc1,0xc7,0x0d,0xc1,0xc7,0x1d,0xc1,0xc7,0x13,0x87,0xdb\n" : "=d" (r) : "a" (&a[0]), "d" (r) : __ACC_ASM_CLOBBER);
#  endif
    return (unsigned) r;
#else
    return 0;
#endif
}
#if (ACC_OS_WIN32 && ACC_CC_PELLESC && (__POCC__ >= 290))
#  pragma warn(pop)
#endif
#endif
#if defined(ACC_WANT_ACCLIB_WILDARGV)
#  undef ACC_WANT_ACCLIB_WILDARGV
#define __ACCLIB_WILDARGV_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif
#if (ACC_OS_DOS16 || ACC_OS_OS216 || ACC_OS_WIN16)
#if 0 && (ACC_CC_MSC)
ACC_EXTERN_C int __acc_cdecl __setargv(void);
ACC_EXTERN_C int __acc_cdecl _setargv(void);
ACC_EXTERN_C int __acc_cdecl _setargv(void) { return __setargv(); }
#endif
#endif
#if (ACC_OS_WIN32 || ACC_OS_WIN64)
#if (ACC_CC_INTELC || ACC_CC_MSC)
ACC_EXTERN_C int __acc_cdecl __setargv(void);
ACC_EXTERN_C int __acc_cdecl _setargv(void);
ACC_EXTERN_C int __acc_cdecl _setargv(void) { return __setargv(); }
#endif
#endif
#if (ACC_OS_EMX)
#define __ACCLIB_HAVE_ACC_WILDARGV 1
ACCLIB_PUBLIC(void, acc_wildargv) (int* argc, char*** argv)
{
    if (argc && argv) {
        _response(argc, argv);
        _wildcard(argc, argv);
    }
}
#endif
#if (ACC_OS_CONSOLE_PSP) && defined(__PSPSDK_DEBUG__)
#define __ACCLIB_HAVE_ACC_WILDARGV 1
ACC_EXTERN_C int acc_psp_init_module(int*, char***, int);
ACCLIB_PUBLIC(void, acc_wildargv) (int* argc, char*** argv)
{
    acc_psp_init_module(argc, argv, -1);
}
#endif
#if !(__ACCLIB_HAVE_ACC_WILDARGV)
#define __ACCLIB_HAVE_ACC_WILDARGV 1
ACCLIB_PUBLIC(void, acc_wildargv) (int* argc, char*** argv)
{
#if 1 && (ACC_ARCH_I086PM)
    if (ACC_MM_AHSHIFT != 3) { exit(1); }
#elif 1 && (ACC_ARCH_M68K && ACC_OS_TOS && ACC_CC_GNUC) && defined(__MINT__)
    __binmode(1);
    if (isatty(1)) __set_binmode(stdout, 0);
    if (isatty(2)) __set_binmode(stderr, 0);
#endif
    ACC_UNUSED(argc); ACC_UNUSED(argv);
}
#endif
#endif

/* vim:set ts=4 sw=4 et: */
