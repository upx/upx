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

#if (ACC_CC_GNUC >= 0x020800ul)     /* 2.8.0 */
#  define __acc_gnuc_extension__ __extension__
#else
#  define __acc_gnuc_extension__
#endif

#if (SIZEOF_LONG_LONG > 0)
__acc_gnuc_extension__ typedef long long acc_llong_t;
#endif
#if (SIZEOF_UNSIGNED_LONG_LONG > 0)
__acc_gnuc_extension__ typedef unsigned long long acc_ullong_t;
#endif

/* acc_int64l_t is int_least64_t in <stdint.h> terminology */
#if (SIZEOF_LONG >= 8)
#  define acc_int64l_t      long int
#  define acc_uint64l_t     unsigned long int
#elif (SIZEOF_LONG_LONG >= 8 && SIZEOF_UNSIGNED_LONG_LONG >= 8)
#  define acc_int64l_t      acc_llong_t
#  define acc_uint64l_t     acc_ullong_t
#elif (SIZEOF___INT64 >= 8 && SIZEOF_UNSIGNED___INT64 >= 8)
#  define acc_int64l_t      __int64
#  define acc_uint64l_t     unsigned __int64
#endif


#if !defined(ACC_UINT32_C)
#  if (ACC_OS_DOS16 && ACC_CC_PACIFICC)
     /* workaround for Pacific C */
#    define ACC_UINT32_C(c)     c
#  elif (UINT_MAX < ACC_0xffffffffL)
#    define ACC_UINT32_C(c)     c ## UL
#  else
#    define ACC_UINT32_C(c)     c ## U
#  endif
#endif


/***********************************************************************
// calling conventions
************************************************************************/

#if (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)
#  if (ACC_CC_GNUC || ACC_CC_PACIFICC || ACC_CC_WATCOMC)
#  else
#    define __acc_cdecl_atexit          __cdecl
#    define __acc_cdecl_main            __cdecl
#    define __acc_cdecl_qsort           __cdecl
#  endif
#  if (ACC_CC_GNUC || ACC_CC_PACIFICC || ACC_CC_WATCOMC)
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
#endif

#if !defined(__acc_cdecl_atexit)
#  define __acc_cdecl_atexit
#endif
#if !defined(__acc_cdecl_main)
#  define __acc_cdecl_main
#endif
#if !defined(__acc_cdecl_qsort)
#  define __acc_cdecl_qsort
#endif
#if !defined(__acc_cdecl_sighandler)
#  define __acc_cdecl_sighandler
#endif

#if (ACC_CC_AZTECC) || (ACC_CC_TURBOC && __TURBOC__ < 0x150)
typedef void __acc_cdecl_sighandler (*acc_sighandler_t)(int);
#else
typedef void (__acc_cdecl_sighandler *acc_sighandler_t)(int);
#endif



/*
vi:ts=4:et
*/
