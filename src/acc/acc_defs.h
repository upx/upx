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


/***********************************************************************
//
************************************************************************/

#if !defined(ACC_UNUSED)
#  if (ACC_CC_BORLANDC || ACC_CC_TURBOC)
#    define ACC_UNUSED(var)         if (&var) ; else
#  elif (ACC_CC_MSC && (_MSC_VER < 900))
#    define ACC_UNUSED(var)         if (&var) ; else
#  elif (ACC_CC_GCC)
#    define ACC_UNUSED(var)         ((void) var)
#  else
#    define ACC_UNUSED(var)         ((void) &var)
#  endif
#endif
#if !defined(ACC_UNUSED_FUNC)
#  if (ACC_CC_BORLANDC || ACC_CC_TURBOC)
#    define ACC_UNUSED_FUNC(func)   if (func) ; else
#  elif (ACC_CC_MSC && (_MSC_VER < 900))
#    define ACC_UNUSED_FUNC(func)   if (func) ; else
#  else
#    define ACC_UNUSED_FUNC(func)   ((void) func)
#  endif
#endif


/***********************************************************************
// compile-time-assertions
//
// The "switch" version works on all compilers, whereas the "typedef" gets
// ignored or misinterpreted (e.g. implicit cast from -1 to unsigned long)
// on some systems. OTOS, on modern compilers, the "switch" version
// may produce a pedantic warning about "selector expr. is constant".
************************************************************************/

/* This can be put into a header file but may get ignored by some compilers */
#if !defined(ACC_COMPILE_TIME_ASSERT_HEADER)
#  if (ACC_CC_AZTECC || ACC_CC_ZORTECHC)
#    define ACC_COMPILE_TIME_ASSERT_HEADER(e)  {typedef int __acc_cta[1-!(e)];}
#  else
#    define ACC_COMPILE_TIME_ASSERT_HEADER(e)  {typedef int __acc_cta[1-2*!(e)];}
#  endif
#endif

/* This must appear within a function body */
#if !defined(ACC_COMPILE_TIME_ASSERT)
#  if (ACC_CC_DMC || ACC_CC_PACIFICC || ACC_CC_SYMANTECC || ACC_CC_ZORTECHC)
#    define ACC_COMPILE_TIME_ASSERT(e)  switch(0) case 1:case !(e):break;
#  elif (ACC_CC_MSC && (_MSC_VER < 900))
#    define ACC_COMPILE_TIME_ASSERT(e)  switch(0) case 1:case !(e):break;
#  else
#    define ACC_COMPILE_TIME_ASSERT(e)  ACC_COMPILE_TIME_ASSERT_HEADER(e)
#  endif
#endif


/***********************************************************************
// macros
************************************************************************/

#if !defined(__ACC_UINT_MAX)
#  define __ACC_INT_MAX(b)      ((((1l  << ((b)-2)) - 1l)  * 2l)  + 1l)
#  define __ACC_UINT_MAX(b)     ((((1ul << ((b)-1)) - 1ul) * 2ul) + 1ul)
#endif


/***********************************************************************
// get sizes of builtin integral types from <limits.h>
************************************************************************/

#if !defined(__ACC_SHORT_BIT)
#  if (USHRT_MAX == ACC_0xffffL)
#    define __ACC_SHORT_BIT     16
#  elif (USHRT_MAX == ACC_0xffffffffL)
#    define __ACC_SHORT_BIT     32
#  elif (USHRT_MAX == __ACC_UINT_MAX(64))
#    define __ACC_SHORT_BIT     64
#  elif (USHRT_MAX == __ACC_UINT_MAX(128))
#    define __ACC_SHORT_BIT     128
#  else
#    error "check your compiler installation: USHRT_MAX"
#  endif
#endif

#if !defined(__ACC_INT_BIT)
#  if (UINT_MAX == ACC_0xffffL)
#    define __ACC_INT_BIT       16
#  elif (UINT_MAX == ACC_0xffffffffL)
#    define __ACC_INT_BIT       32
#  elif (UINT_MAX == __ACC_UINT_MAX(64))
#    define __ACC_INT_BIT       64
#  elif (UINT_MAX == __ACC_UINT_MAX(128))
#    define __ACC_INT_BIT       128
#  else
#    error "check your compiler installation: UINT_MAX"
#  endif
#endif

#if !defined(__ACC_LONG_BIT)
#  if (ULONG_MAX == ACC_0xffffffffL)
#    define __ACC_LONG_BIT      32
#  elif (ULONG_MAX == __ACC_UINT_MAX(64))
#    define __ACC_LONG_BIT      64
#  elif (ULONG_MAX == __ACC_UINT_MAX(128))
#    define __ACC_LONG_BIT      128
#  else
#    error "check your compiler installation: ULONG_MAX"
#  endif
#endif


/***********************************************************************
//
************************************************************************/

#if (ACC_CC_GNUC)
#  define acc_alignof(e)        __alignof__(e)
#elif (ACC_CC_INTELC && (__INTEL_COMPILER >= 700))
#  define acc_alignof(e)        __alignof__(e)
#elif (ACC_CC_MSC && (_MSC_VER >= 1300))
#  define acc_alignof(e)        __alignof(e)
#endif



/*
vi:ts=4:et
*/
