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

#if (!(SIZEOF_SHORT > 0 && SIZEOF_INT > 0 && SIZEOF_LONG > 0))
#  error "missing defines for sizes"
#endif
#if (!(SIZEOF_PTRDIFF_T > 0 && SIZEOF_SIZE_T > 0 && SIZEOF_VOID_P > 0 && SIZEOF_CHAR_P > 0))
#  error "missing defines for sizes"
#endif


/***********************************************************************
// some <stdint.h> types:
//   required: least & fast: acc_int32l_t, acc_int32f_t
//   optional: exact32 acc_int32e_t
//   optional: least64 acc_int64l_t
************************************************************************/

/* acc_int32e_t is int32_t in <stdint.h> terminology */
#if !defined(acc_int32e_t)
#if (SIZEOF_INT == 4)
#  define acc_int32e_t          int
#  define acc_uint32e_t         unsigned int
#  define ACC_INT32E_C(c)       c
#  define ACC_UINT32E_C(c)      c##U
#elif (SIZEOF_LONG == 4)
#  define acc_int32e_t          long int
#  define acc_uint32e_t         unsigned long int
#  define ACC_INT32E_C(c)       c##L
#  define ACC_UINT32E_C(c)      c##UL
#elif (SIZEOF_SHORT == 4)
#  define acc_int32e_t          short int
#  define acc_uint32e_t         unsigned short int
#  define ACC_INT32E_C(c)       c
#  define ACC_UINT32E_C(c)      c##U
#elif (SIZEOF_LONG_LONG == 4 && SIZEOF_UNSIGNED_LONG_LONG == 4)
#  define acc_int32e_t          acc_llong_t
#  define acc_uint32e_t         acc_ullong_t
#  define ACC_INT32E_C(c)       c##LL
#  define ACC_UINT32E_C(c)      c##ULL
#elif (SIZEOF___INT32 == 4 && SIZEOF_UNSIGNED___INT32 == 4)
#  define acc_int32e_t          __int32
#  define acc_uint32e_t         unsigned __int32
#  if (SIZEOF_INT > 4)
#    define ACC_INT32E_C(c)     c
#    define ACC_UINT32E_C(c)    c##U
#  elif (SIZEOF_LONG > 4)
#    define ACC_INT32E_C(c)     c##L
#    define ACC_UINT32E_C(c)    c##UL
#  else
#    define ACC_INT32E_C(c)     c##i32
#    define ACC_UINT32E_C(c)    c##ui32
#  endif
#else
  /* no exact 32-bit integral type on this machine */
#endif
#endif
#if defined(acc_int32e_t)
#  define SIZEOF_ACC_INT32E_T   4
#endif


/* acc_int32l_t is int_least32_t in <stdint.h> terminology */
#if !defined(acc_int32l_t)
#if defined(acc_int32e_t)
#  define acc_int32l_t          acc_int32e_t
#  define acc_uint32l_t         acc_uint32e_t
#  define ACC_INT32L_C(c)       ACC_INT32E_C(c)
#  define ACC_UINT32L_C(c)      ACC_UINT32E_C(c)
#  define SIZEOF_ACC_INT32L_T   SIZEOF_ACC_INT32E_T
#elif (SIZEOF_INT > 4)
#  define acc_int32l_t          int
#  define acc_uint32l_t         unsigned int
#  define ACC_INT32L_C(c)       c
#  define ACC_UINT32L_C(c)      c##U
#  define SIZEOF_ACC_INT32L_T   SIZEOF_INT
#elif (SIZEOF_LONG > 4)
#  define acc_int32l_t          long int
#  define acc_uint32l_t         unsigned long int
#  define ACC_INT32L_C(c)       c##L
#  define ACC_UINT32L_C(c)      c##UL
#  define SIZEOF_ACC_INT32L_T   SIZEOF_LONG
#else
#  error "acc_int32l_t"
#endif
#endif


/* acc_int32f_t is int_fast32_t in <stdint.h> terminology */
#if !defined(acc_int32f_t)
#if (SIZEOF_INT >= 4)
#  define acc_int32f_t          int
#  define acc_uint32f_t         unsigned int
#  define ACC_INT32F_C(c)       c
#  define ACC_UINT32F_C(c)      c##U
#  define SIZEOF_ACC_INT32F_T   SIZEOF_INT
#elif (SIZEOF_LONG >= 4)
#  define acc_int32f_t          long int
#  define acc_uint32f_t         unsigned long int
#  define ACC_INT32F_C(c)       c##L
#  define ACC_UINT32F_C(c)      c##UL
#  define SIZEOF_ACC_INT32F_T   SIZEOF_LONG
#elif defined(acc_int32e_t)
#  define acc_int32f_t          acc_int32e_t
#  define acc_uint32f_t         acc_uint32e_t
#  define ACC_INT32F_C(c)       ACC_INT32E_C(c)
#  define ACC_UINT32F_C(c)      ACC_UINT32E_C(c)
#  define SIZEOF_ACC_INT32F_T   SIZEOF_ACC_INT32E_T
#else
#  error "acc_int32f_t"
#endif
#endif


/* acc_int64l_t is int_least64_t in <stdint.h> terminology */
#if !defined(acc_int64l_t)
#if (SIZEOF___INT64 >= 8 && SIZEOF_UNSIGNED___INT64 >= 8)
#  if (ACC_CC_BORLANDC) && !defined(ACC_CONFIG_PREFER___INT64)
#    define ACC_CONFIG_PREFER___INT64 1
#  endif
#endif
#if (SIZEOF_INT >= 8)
#  define acc_int64l_t          int
#  define acc_uint64l_t         unsigned int
#  define ACC_INT64L_C(c)       c
#  define ACC_UINT64L_C(c)      c##U
#  define SIZEOF_ACC_INT64L_T   SIZEOF_INT
#elif (SIZEOF_LONG >= 8)
#  define acc_int64l_t          long int
#  define acc_uint64l_t         unsigned long int
#  define ACC_INT64L_C(c)       c##L
#  define ACC_UINT64L_C(c)      c##UL
#  define SIZEOF_ACC_INT64L_T   SIZEOF_LONG
#elif (SIZEOF_LONG_LONG >= 8 && SIZEOF_UNSIGNED_LONG_LONG >= 8) && !defined(ACC_CONFIG_PREFER___INT64)
#  define acc_int64l_t          acc_llong_t
#  define acc_uint64l_t         acc_ullong_t
#  if (ACC_CC_BORLANDC)
#    define ACC_INT64L_C(c)     ((c) + 0ll)
#    define ACC_UINT64L_C(c)    ((c) + 0ull)
#  else
#    define ACC_INT64L_C(c)     c##LL
#    define ACC_UINT64L_C(c)    c##ULL
#  endif
#  define SIZEOF_ACC_INT64L_T   SIZEOF_LONG_LONG
#elif (SIZEOF___INT64 >= 8 && SIZEOF_UNSIGNED___INT64 >= 8)
#  define acc_int64l_t          __int64
#  define acc_uint64l_t         unsigned __int64
#  if (ACC_CC_BORLANDC)
#    define ACC_INT64L_C(c)     ((c) + 0i64)
#    define ACC_UINT64L_C(c)    ((c) + 0ui64)
#  else
#    define ACC_INT64L_C(c)     c##i64
#    define ACC_UINT64L_C(c)    c##ui64
#  endif
#  define SIZEOF_ACC_INT64L_T   SIZEOF___INT64
#else
  /* no least 64-bit integral type on this machine */
#endif
#endif


#if !defined(acc_intptr_t)
#if (ACC_ARCH_IA32 && ACC_CC_MSC && (_MSC_VER >= 1300))
   typedef __w64 int            acc_intptr_t;
   typedef __w64 unsigned int   acc_uintptr_t;
#  define acc_intptr_t          acc_intptr_t
#  define acc_uintptr_t         acc_uintptr_t
#  define SIZEOF_ACC_INTPTR_T   SIZEOF_INT
#elif (SIZEOF_INT >= SIZEOF_VOID_P)
#  define acc_intptr_t          int
#  define acc_uintptr_t         unsigned int
#  define SIZEOF_ACC_INTPTR_T   SIZEOF_INT
#elif (SIZEOF_LONG >= SIZEOF_VOID_P)
#  define acc_intptr_t          long
#  define acc_uintptr_t         unsigned long
#  define SIZEOF_ACC_INTPTR_T   SIZEOF_LONG
#elif (SIZEOF_ACC_INT64L_T >= SIZEOF_VOID_P)
#  define acc_intptr_t          acc_int64l_t
#  define acc_uintptr_t         acc_uint64l_t
#  define SIZEOF_ACC_INTPTR_T   SIZEOF_ACC_INT64L_T
#else
#  error "acc_intptr_t"
#endif
#endif


/* workaround for broken compilers */
#if (ACC_BROKEN_INTEGRAL_CONSTANTS)
#  undef ACC_INT32E_C
#  undef ACC_UINT32E_C
#  undef ACC_INT32L_C
#  undef ACC_UINT32L_C
#  undef ACC_INT32F_C
#  undef ACC_UINT32F_C
#  if (SIZEOF_INT == 4)
#    define ACC_INT32E_C(c)     ((c) + 0)
#    define ACC_UINT32E_C(c)    ((c) + 0U)
#    define ACC_INT32L_C(c)     ((c) + 0)
#    define ACC_UINT32L_C(c)    ((c) + 0U)
#    define ACC_INT32F_C(c)     ((c) + 0)
#    define ACC_UINT32F_C(c)    ((c) + 0U)
#  elif (SIZEOF_LONG == 4)
#    define ACC_INT32E_C(c)     ((c) + 0L)
#    define ACC_UINT32E_C(c)    ((c) + 0UL)
#    define ACC_INT32L_C(c)     ((c) + 0L)
#    define ACC_UINT32L_C(c)    ((c) + 0UL)
#    define ACC_INT32F_C(c)     ((c) + 0L)
#    define ACC_UINT32F_C(c)    ((c) + 0UL)
#  else
#    error "integral constants"
#  endif
#endif


/***********************************************************************
// calling conventions
************************************************************************/

#if (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)
#  if (ACC_CC_GNUC || ACC_CC_HIGHC || ACC_CC_NDPC || ACC_CC_PACIFICC)
#  elif (ACC_CC_DMC || ACC_CC_SYMANTECC || ACC_CC_ZORTECHC)
#    define __acc_cdecl                 __cdecl
#    define __acc_cdecl_atexit
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
#elif (ACC_OS_TOS && (ACC_CC_PUREC || ACC_CC_TURBOC))
#  define __acc_cdecl                   cdecl
#endif

#if !defined(__acc_cdecl)
#  define __acc_cdecl
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
#if !defined(__acc_cdecl_va)
#  define __acc_cdecl_va                __acc_cdecl
#endif

#if (ACC_BROKEN_CDECL_ALT_SYNTAX)
typedef void __acc_cdecl_sighandler (*acc_sighandler_t)(int);
#elif defined(RETSIGTYPE)
typedef RETSIGTYPE (__acc_cdecl_sighandler *acc_sighandler_t)(int);
#else
typedef void (__acc_cdecl_sighandler *acc_sighandler_t)(int);
#endif


/*
vi:ts=4:et
*/
