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
 * Memory Model - exactly one of:
 *
 *   ACC_MM_FLAT            [default]
 *   ACC_MM_TINY
 *   ACC_MM_SMALL
 *   ACC_MM_MEDIUM
 *   ACC_MM_COMPACT
 *   ACC_MM_LARGE
 *   ACC_MM_HUGE
 */

#if (ACC_OS_DOS16 || ACC_OS_OS216 || ACC_OS_WIN16)

#if (UINT_MAX != 0xffffL)
#  error "this should not happen"
#endif
#if defined(__TINY__) || defined(M_I86TM) || defined(_M_I86TM)
#  define ACC_MM_TINY           1
#elif defined(__HUGE__) || defined(M_I86HM) || defined(_M_I86HM)
#  define ACC_MM_HUGE           1
#elif defined(__SMALL__) || defined(M_I86SM) || defined(_M_I86SM) || defined(SMALL_MODEL)
#  define ACC_MM_SMALL          1
#elif defined(__MEDIUM__) || defined(M_I86MM) || defined(_M_I86MM)
#  define ACC_MM_MEDIUM         1
#elif defined(__COMPACT__) || defined(M_I86CM) || defined(_M_I86CM)
#  define ACC_MM_COMPACT        1
#elif defined(__LARGE__) || defined(M_I86LM) || defined(_M_I86LM) || defined(LARGE_MODEL)
#  define ACC_MM_LARGE          1
#elif (ACC_CC_AZTEC_C)
#  if defined(_LARGE_CODE) && defined(_LARGE_DATA)
#    define ACC_MM_LARGE        1
#  elif defined(_LARGE_CODE)
#    define ACC_MM_MEDIUM       1
#  elif defined(_LARGE_DATA)
#    define ACC_MM_COMPACT      1
#  else
#    define ACC_MM_SMALL        1
#  endif
#else
#  error "unknown memory model"
#endif


#if (ACC_CC_AZTEC_C || ACC_CC_PACIFIC)
#elif (ACC_CC_DMC)
#  define ACC_HAVE_MM_HUGE_PTR      1
#elif (ACC_CC_TURBOC && __TURBOC__ < 0x0295)
#  define ACC_HAVE_MM_HUGE_PTR      1
#elif (ACC_CC_WATCOMC && __WATCOMC__ >= 1200)
   /* __huge pointers seem completely broken in OpenWatcom 1.0 */
#else
#  define ACC_HAVE_MM_HUGE_PTR      1   /* working __huge pointers */
#  define ACC_HAVE_MM_HUGE_ARRAY    1   /* char __huge x[256*1024L] works */
#endif
#if (ACC_MM_TINY)
#  undef ACC_HAVE_MM_HUGE_ARRAY
#endif


#else

#  define ACC_MM_FLAT           1

#endif


#if (ACC_MM_FLAT)
#  define ACC_INFO_MM           "flat"
#elif (ACC_MM_TINY)
#  define ACC_INFO_MM           "tiny"
#elif (ACC_MM_SMALL)
#  define ACC_INFO_MM           "small"
#elif (ACC_MM_MEDIUM)
#  define ACC_INFO_MM           "medium"
#elif (ACC_MM_COMPACT)
#  define ACC_INFO_MM           "compact"
#elif (ACC_MM_LARGE)
#  define ACC_INFO_MM           "large"
#elif (ACC_MM_HUGE)
#  define ACC_INFO_MM           "huge"
#else
#  error "unknown memory model"
#endif


/*
vi:ts=4:et
*/
