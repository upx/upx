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

#if (UINT_MAX != ACC_0xffffL)
#  error "this should not happen"
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
#  error "unknown memory model"
#endif


/* ACC_HAVE_MM_HUGE_PTR   ... working __huge pointers
 * ACC_HAVE_MM_HUGE_ARRAY ... char __huge x[256*1024L] works */
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
#elif (ACC_CC_WATCOMC && (__WATCOMC__ >= 1200))
   /* pointer arithmetics with __huge arrays seems broken in OpenWatcom 1.x */
#  undef ACC_HAVE_MM_HUGE_ARRAY
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
} /* extern "C" */
#endif


#elif (ACC_ARCH_C166)
#if !defined(__MODEL__)
#  error "FIXME - C166 __MODEL__"
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
#  error "FIXME - C166 __MODEL__"
#endif


#elif (ACC_ARCH_MCS251)
#if !defined(__MODEL__)
#  error "FIXME - MCS251 __MODEL__"
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
#  error "FIXME - MCS251 __MODEL__"
#endif


#elif (ACC_ARCH_MCS51)
#if !defined(__MODEL__)
#  error "FIXME - MCS51 __MODEL__"
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
#  error "FIXME - MCS51 __MODEL__"
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
