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


/***********************************************************************
// acc_alignof() / acc_inline
************************************************************************/

#if (ACC_CC_CILLY || ACC_CC_GNUC || ACC_CC_PGI)
#  define acc_alignof(e)        __alignof__(e)
#elif (ACC_CC_INTELC && (__INTEL_COMPILER >= 700))
#  define acc_alignof(e)        __alignof__(e)
#elif (ACC_CC_MSC && (_MSC_VER >= 1300))
#  define acc_alignof(e)        __alignof(e)
#endif

#if (ACC_CC_TURBOC && (__TURBOC__ <= 0x0295))
#elif defined(__cplusplus)
#  define acc_inline            inline
#elif (ACC_CC_BORLANDC && (__BORLANDC__ >= 0x0550))
#  define acc_inline            __inline
#elif (ACC_CC_CILLY || ACC_CC_GNUC || ACC_CC_PGI)
#  define acc_inline            __inline__
#elif (ACC_CC_DMC)
#  define acc_inline            __inline
#elif (ACC_CC_INTELC)
#  define acc_inline            __inline
#elif (ACC_CC_MSC && (_MSC_VER >= 900))
#  define acc_inline            __inline
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#  define acc_inline            inline
#endif


/***********************************************************************
// ACC_UNUSED / ACC_UNUSED_FUNC
************************************************************************/

#if !defined(ACC_UNUSED)
#  if (ACC_CC_BORLANDC && (__BORLANDC__ >= 0x0600))
#    define ACC_UNUSED(var)         ((void) &var)
#  elif (ACC_CC_BORLANDC || ACC_CC_HIGHC || ACC_CC_NDPC || ACC_CC_TURBOC)
#    define ACC_UNUSED(var)         if (&var) ; else
#  elif (ACC_CC_MSC && (_MSC_VER < 900))
#    define ACC_UNUSED(var)         if (&var) ; else
#  elif (ACC_CC_GNUC)
#    define ACC_UNUSED(var)         ((void) var)
#  elif (ACC_CC_KEILC)
#    define ACC_UNUSED(var)
#  else
#    define ACC_UNUSED(var)         ((void) &var)
#  endif
#endif
#if !defined(ACC_UNUSED_FUNC)
#  if (ACC_CC_BORLANDC && (__BORLANDC__ >= 0x0600))
#    define ACC_UNUSED_FUNC(func)   ((void) func)
#  elif (ACC_CC_BORLANDC || ACC_CC_NDPC || ACC_CC_TURBOC)
#    define ACC_UNUSED_FUNC(func)   if (func) ; else
#  elif (ACC_CC_GNUC == 0x030400ul) && defined(__llvm__)
#    define ACC_UNUSED_FUNC(func)   ((void) &func)
#  elif (ACC_CC_MSC && (_MSC_VER < 900))
#    define ACC_UNUSED_FUNC(func)   if (func) ; else
#  elif (ACC_CC_MSC)
#    define ACC_UNUSED_FUNC(func)   ((void) &func)
#  elif (ACC_CC_KEILC)
#    define ACC_UNUSED_FUNC(func)
#  else
#    define ACC_UNUSED_FUNC(func)   ((void) func)
#  endif
#endif


/***********************************************************************
// compile-time-assertions
************************************************************************/

/* This can be put into a header file but may get ignored by some compilers. */
#if !defined(ACC_COMPILE_TIME_ASSERT_HEADER)
#  if (ACC_CC_AZTECC || ACC_CC_ZORTECHC)
#    define ACC_COMPILE_TIME_ASSERT_HEADER(e)  extern int __acc_cta[1-!(e)];
#  elif (ACC_CC_DMC || ACC_CC_SYMANTECC)
#    define ACC_COMPILE_TIME_ASSERT_HEADER(e)  extern int __acc_cta[1u-2*!(e)];
#  elif (ACC_CC_TURBOC && (__TURBOC__ == 0x0295))
#    define ACC_COMPILE_TIME_ASSERT_HEADER(e)  extern int __acc_cta[1-!(e)];
#  else
#    define ACC_COMPILE_TIME_ASSERT_HEADER(e)  extern int __acc_cta[1-2*!(e)];
#  endif
#endif

/* This must appear within a function body. */
#if !defined(ACC_COMPILE_TIME_ASSERT)
#  if (ACC_CC_AZTECC)
#    define ACC_COMPILE_TIME_ASSERT(e)  {typedef int __acc_cta_t[1-!(e)];}
#  elif (ACC_CC_DMC || ACC_CC_PACIFICC || ACC_CC_SYMANTECC || ACC_CC_ZORTECHC)
#    define ACC_COMPILE_TIME_ASSERT(e)  switch(0) case 1:case !(e):break;
#  elif (ACC_CC_MSC && (_MSC_VER < 900))
#    define ACC_COMPILE_TIME_ASSERT(e)  switch(0) case 1:case !(e):break;
#  elif (ACC_CC_TURBOC && (__TURBOC__ == 0x0295))
#    define ACC_COMPILE_TIME_ASSERT(e)  switch(0) case 1:case !(e):break;
#  else
#    define ACC_COMPILE_TIME_ASSERT(e)  {typedef int __acc_cta_t[1-2*!(e)];}
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
// acc_auto.h supplements
************************************************************************/

#if (ACC_OS_CYGWIN || (ACC_OS_EMX && defined(__RSXNT__)) || ACC_OS_WIN32 || ACC_OS_WIN64)
#  if (ACC_CC_WATCOMC && (__WATCOMC__ < 1000))
#  elif (ACC_OS_WIN32 && ACC_CC_GNUC) && defined(__PW32__)
     /* ancient pw32 version */
#  elif ((ACC_OS_CYGWIN || defined(__MINGW32__)) && (ACC_CC_GNUC && (ACC_CC_GNUC < 0x025f00ul)))
     /* ancient cygwin/mingw version */
#  else
#    define ACC_HAVE_WINDOWS_H 1
#  endif
#endif



/*
vi:ts=4:et
*/
