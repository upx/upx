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
#if defined(__TINY__) || defined(M_I86TM)
#  define ACC_MM_TINY           1
#  define ACC_INFO_MM           "tiny"
#elif defined(__SMALL__) || defined(M_I86SM) || defined(SMALL_MODEL)
#  define ACC_MM_SMALL          1
#  define ACC_INFO_MM           "small"
#elif defined(__MEDIUM__) || defined(M_I86MM)
#  define ACC_MM_MEDIUM         1
#  define ACC_INFO_MM           "medium"
#elif defined(__COMPACT__) || defined(M_I86CM)
#  define ACC_MM_COMPACT        1
#  define ACC_INFO_MM           "compact"
#elif defined(__HUGE__) || defined(M_I86HM)
#  define ACC_MM_HUGE           1
#  define ACC_INFO_MM           "huge"
#elif defined(__LARGE__) || defined(M_I86LM) || defined(LARGE_MODEL)
#  define ACC_MM_LARGE          1
#  define ACC_INFO_MM           "large"
#else
#  error "unknown memory model"
#endif

#else

#  define ACC_MM_FLAT           1
#  define ACC_INFO_MM           "flat"

#endif


/*
vi:ts=4:et
*/
