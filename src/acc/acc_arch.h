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
 * CPU architecture - exactly one of:
 *
 *   ACC_ARCH_UNKNOWN       [default]
 *   ACC_ARCH_ALPHA
 *   ACC_ARCH_AMD64         aka x86-64 or ia32e
 *   ACC_ARCH_C166
 *   ACC_ARCH_IA16          Intel Architecture (8088, 8086, 80186, 80286)
 *   ACC_ARCH_IA32          Intel Architecture (80386+)
 *   ACC_ARCH_IA64          Intel Architecture (Itanium)
 *   ACC_ARCH_M68K          Motorola 680x0
 *   ACC_ARCH_MCS251
 *   ACC_ARCH_MCS51
 *   ACC_ARCH_PPC64         Power PC
 *   ACC_ARCH_SPARC64
 *
 * Optionally define one of:
 *   ACC_ENDIAN_LITTLE_ENDIAN
 *   ACC_ENDIAN_BIG_ENDIAN
 *
 * Note that this list is not exhaustive - actually we only really care
 * about architectures which allow unaligned memory access at reasonable
 * speed (for the moment this means IA16, IA32 and AMD64).
 */

#if (ACC_OS_DOS16 || ACC_OS_OS216 || ACC_OS_WIN16)
#  define ACC_ARCH_IA16             1
#  define ACC_INFO_ARCH             "ia16"
#elif defined(__amd64__) || defined(__x86_64__) || defined(_M_AMD64)
#  define ACC_ARCH_AMD64            1
#  define ACC_INFO_ARCH             "amd64"
#elif (UINT_MAX <= ACC_0xffffL) && defined(__AVR__)
#  define ACC_ARCH_AVR              1
#  define ACC_INFO_ARCH             "avr"
#elif (UINT_MAX == ACC_0xffffL) && defined(__C166__)
#  define ACC_ARCH_C166             1
#  define ACC_INFO_ARCH             "c166"
#elif (UINT_MAX == ACC_0xffffL) && defined(__C251__)
#  define ACC_ARCH_MCS251           1
#  define ACC_INFO_ARCH             "mcs-251"
#elif (UINT_MAX == ACC_0xffffL) && defined(__C51__)
#  define ACC_ARCH_MCS51            1
#  define ACC_INFO_ARCH             "mcs-51"
#elif defined(__386__) || defined(__i386__) || defined(__i386) || defined(_M_IX86) || defined(_M_I386)
#  define ACC_ARCH_IA32             1
#  define ACC_INFO_ARCH             "ia32"
#elif (ACC_CC_ZORTECHC && defined(__I86__))
#  define ACC_ARCH_IA32             1
#  define ACC_INFO_ARCH             "ia32"
#elif defined(__ia64__) || defined(__ia64) || defined(_M_IA64)
#  define ACC_ARCH_IA64             1
#  define ACC_INFO_ARCH             "ia64"
#elif (ACC_OS_DOS32 && ACC_CC_HIGHC) && defined(_I386)
#  define ACC_ARCH_IA32             1
#  define ACC_INFO_ARCH             "ia32"
#elif (ACC_OS_DOS32 || ACC_OS_OS2)
#  error "missing define for CPU architecture"
#elif (ACC_OS_WIN32)
#  error "missing define for CPU architecture"
#elif (ACC_OS_WIN64)
/* #  error "missing define for CPU architecture" */
#elif (ACC_OS_TOS) || defined(__m68000__)
#  define ACC_ARCH_M68K             1
#  define ACC_INFO_ARCH             "m68k"
#elif defined(__alpha__) || defined(__alpha)
#  define ACC_ARCH_ALPHA            1
#  define ACC_INFO_ARCH             "alpha"
#elif defined(__ppc64__) || defined(__ppc64)
#  define ACC_ARCH_PPC64            1
#  define ACC_INFO_ARCH             "ppc64"
#elif defined(__sparc64__) || defined(__sparc64)
#  define ACC_ARCH_SPARC64          1
#  define ACC_INFO_ARCH             "sparc64"
#else
#  define ACC_ARCH_UNKNOWN          1
#  define ACC_INFO_ARCH             "unknown"
#endif


#if (ACC_ARCH_AMD64 || ACC_ARCH_IA16 || ACC_ARCH_IA32)
#  define ACC_ENDIAN_LITTLE_ENDIAN  1
#  define ACC_INFO_ENDIAN           "little-endian"
#elif (ACC_ARCH_M68K)
#  define ACC_ENDIAN_BIG_ENDIAN     1
#  define ACC_INFO_ENDIAN           "big-endian"
#endif


#if (ACC_ARCH_IA16)
#  if (UINT_MAX != ACC_0xffffL)
#    error "this should not happen"
#  endif
#  if (ULONG_MAX != ACC_0xffffffffL)
#    error "this should not happen"
#  endif
#endif
#if (ACC_ARCH_IA32)
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
