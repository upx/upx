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


/*************************************************************************
//
**************************************************************************/

#if !defined(ACCCHK_ASSERT)
#  define ACCCHK_ASSERT(expr)   ACC_COMPILE_TIME_ASSERT(expr)
#endif

/* compile-time sign */
#if !defined(ACCCHK_ASSERT_SIGN_T)
#  define ACCCHK_ASSERT_SIGN_T(type,relop) \
        ACCCHK_ASSERT( (type) (-1)       relop  (type) 0 ) \
        ACCCHK_ASSERT( (type) (~(type)0) relop  (type) 0 ) \
        ACCCHK_ASSERT( (type) (~(type)0) ==     (type) (-1) )
#endif

#if !defined(ACCCHK_IS_SIGNED_T)
#  define ACCCHK_ASSERT_IS_SIGNED_T(type)       ACCCHK_ASSERT_SIGN_T(type,<)
#endif

#if !defined(ACCCHK_IS_UNSIGNED_T)
#  if (ACC_BROKEN_INTEGRAL_PROMOTION)
#    define ACCCHK_ASSERT_IS_UNSIGNED_T(type) \
        ACCCHK_ASSERT( (type) (-1) > (type) 0 )
#  else
#    define ACCCHK_ASSERT_IS_UNSIGNED_T(type)   ACCCHK_ASSERT_SIGN_T(type,>)
#  endif
#endif


/*************************************************************************
// check preprocessor
**************************************************************************/

#if (ACC_0xffffffffL - ACC_UINT32L_C(4294967294) != 1)
#  error "preprocessor error 1"
#endif
#if (ACC_0xffffffffL - ACC_UINT32L_C(0xfffffffd) != 2)
#  error "preprocessor error 2"
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


/*************************************************************************
// check basic arithmetics
**************************************************************************/

    ACCCHK_ASSERT(1 == 1)

    ACCCHK_ASSERT(__ACC_INT_MAX(2) == 1)
    ACCCHK_ASSERT(__ACC_INT_MAX(8) == 127)
    ACCCHK_ASSERT(__ACC_INT_MAX(16) == 32767)

    ACCCHK_ASSERT(__ACC_UINT_MAX(2) == 3)
    ACCCHK_ASSERT(__ACC_UINT_MAX(16) == 0xffffU)
    ACCCHK_ASSERT(__ACC_UINT_MAX(32) == 0xffffffffUL)
#if !defined(ACC_BROKEN_INTEGRAL_PROMOTION)
    ACCCHK_ASSERT(__ACC_UINT_MAX(__ACC_INT_BIT) == ~(0u))
    ACCCHK_ASSERT(__ACC_UINT_MAX(__ACC_LONG_BIT) == ~(0ul))
#endif


/*************************************************************************
// check basic types
**************************************************************************/

    ACCCHK_ASSERT_IS_SIGNED_T(signed char)
    ACCCHK_ASSERT_IS_UNSIGNED_T(unsigned char)
    ACCCHK_ASSERT(sizeof(signed char) == sizeof(char))
    ACCCHK_ASSERT(sizeof(unsigned char) == sizeof(char))
    ACCCHK_ASSERT(sizeof(char) == 1)
#if (ACC_CC_CILLY)
    /* CIL is broken */
#else
    ACCCHK_ASSERT(sizeof(char) == sizeof((char)0))
#endif
#if defined(__cplusplus)
    ACCCHK_ASSERT(sizeof('\0') == sizeof(char))
#else
#  if (ACC_CC_DMC)
    /* Digital Mars C is broken */
#  else
    ACCCHK_ASSERT(sizeof('\0') == sizeof(int))
#  endif
#endif
#if defined(acc_alignof)
    ACCCHK_ASSERT(acc_alignof(char) == 1)
    ACCCHK_ASSERT(acc_alignof(signed char) == 1)
    ACCCHK_ASSERT(acc_alignof(unsigned char) == 1)
#endif

    ACCCHK_ASSERT_IS_SIGNED_T(short)
    ACCCHK_ASSERT_IS_UNSIGNED_T(unsigned short)
    ACCCHK_ASSERT(sizeof(short) == sizeof(unsigned short))
    ACCCHK_ASSERT(sizeof(short) >= 2)
    ACCCHK_ASSERT(sizeof(short) >= sizeof(char))
#if (ACC_CC_CILLY)
    /* CIL is broken */
#else
    ACCCHK_ASSERT(sizeof(short) == sizeof((short)0))
#endif
#if (SIZEOF_SHORT > 0)
    ACCCHK_ASSERT(sizeof(short) == SIZEOF_SHORT)
#endif

    ACCCHK_ASSERT_IS_SIGNED_T(int)
    ACCCHK_ASSERT_IS_UNSIGNED_T(unsigned int)
    ACCCHK_ASSERT(sizeof(int) == sizeof(unsigned int))
    ACCCHK_ASSERT(sizeof(int) >= 2)
    ACCCHK_ASSERT(sizeof(int) >= sizeof(short))
    ACCCHK_ASSERT(sizeof(int) == sizeof(0))
    ACCCHK_ASSERT(sizeof(int) == sizeof((int)0))
#if (SIZEOF_INT > 0)
    ACCCHK_ASSERT(sizeof(int) == SIZEOF_INT)
#endif

    ACCCHK_ASSERT_IS_SIGNED_T(long)
    ACCCHK_ASSERT_IS_UNSIGNED_T(unsigned long)
    ACCCHK_ASSERT(sizeof(long) == sizeof(unsigned long))
    ACCCHK_ASSERT(sizeof(long) >= 4)
    ACCCHK_ASSERT(sizeof(long) >= sizeof(int))
    ACCCHK_ASSERT(sizeof(long) == sizeof(0L))
    ACCCHK_ASSERT(sizeof(long) == sizeof((long)0))
#if (SIZEOF_LONG > 0)
    ACCCHK_ASSERT(sizeof(long) == SIZEOF_LONG)
#endif

    ACCCHK_ASSERT_IS_UNSIGNED_T(size_t)
    ACCCHK_ASSERT(sizeof(size_t) >= sizeof(int))
    ACCCHK_ASSERT(sizeof(size_t) == sizeof(sizeof(0))) /* sizeof() returns size_t */
#if (SIZEOF_SIZE_T > 0)
    ACCCHK_ASSERT(sizeof(size_t) == SIZEOF_SIZE_T)
#endif

    ACCCHK_ASSERT_IS_SIGNED_T(ptrdiff_t)
    ACCCHK_ASSERT(sizeof(ptrdiff_t) >= sizeof(int))
    ACCCHK_ASSERT(sizeof(ptrdiff_t) >= sizeof(size_t))
#if !defined(ACC_BROKEN_SIZEOF)
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
#endif
#if (SIZEOF_CHAR_P > 0)
    ACCCHK_ASSERT(sizeof(char*) == SIZEOF_CHAR_P)
#endif
#if (ACC_HAVE_MM_HUGE_PTR)
    ACCCHK_ASSERT(4 == sizeof(void __huge*))
    ACCCHK_ASSERT(4 == sizeof(char __huge*))
#endif


/*************************************************************************
// check arithmetics
**************************************************************************/

    ACCCHK_ASSERT((((1u  << 15) + 1) >> 15) == 1)
    ACCCHK_ASSERT((((1ul << 31) + 1) >> 31) == 1)

#if (ACC_CC_TURBOC && (__TURBOC__ < 0x0150))
    /* TC 1.0 bug, probably due to ACC_BROKEN_INTEGRAL_PROMOTION ?? */
#else
    ACCCHK_ASSERT((1   << (8*SIZEOF_INT-1)) < 0)
#endif
    ACCCHK_ASSERT((1u  << (8*SIZEOF_INT-1)) > 0)

    ACCCHK_ASSERT((1l  << (8*SIZEOF_LONG-1)) < 0)
    ACCCHK_ASSERT((1ul << (8*SIZEOF_LONG-1)) > 0)

#if defined(acc_int32e_t)
    ACCCHK_ASSERT(sizeof(acc_int32e_t) == 4)
    ACCCHK_ASSERT(sizeof(acc_int32e_t) == SIZEOF_ACC_INT32E_T)
    ACCCHK_ASSERT(sizeof(acc_uint32e_t) == 4)
    ACCCHK_ASSERT(sizeof(acc_int32e_t) == sizeof(acc_uint32e_t))

    ACCCHK_ASSERT_IS_SIGNED_T(acc_int32e_t)
    ACCCHK_ASSERT(((( (acc_int32e_t)1 << 30) + 1) >> 30) == 1)
    ACCCHK_ASSERT(((( ACC_INT32E_C(1) << 30) + 1) >> 30) == 1)

    ACCCHK_ASSERT_IS_UNSIGNED_T(acc_uint32e_t)
    ACCCHK_ASSERT(((( (acc_uint32e_t)1 << 31) + 1) >> 31) == 1)
    ACCCHK_ASSERT(((( ACC_UINT32E_C(1) << 31) + 1) >> 31) == 1)

    ACCCHK_ASSERT( (acc_int32e_t) (1 + ~(acc_int32e_t)0) == 0)
#if defined(ACCCHK_CONFIG_PEDANTIC)
    /* compiler may warn about overflow */
    ACCCHK_ASSERT( (acc_uint32e_t)(1 + ~(acc_uint32e_t)0) == 0)
#endif /* ACCCHK_CONFIG_PEDANTIC */

#if (SIZEOF_ACC_INT32E_T < SIZEOF_INT)
    ACCCHK_ASSERT(sizeof(ACC_INT32E_C(0)) == sizeof(int))
    ACCCHK_ASSERT(sizeof(ACC_UINT32E_C(0)) == sizeof(int))
#else
    ACCCHK_ASSERT(sizeof(ACC_INT32E_C(0)) == SIZEOF_ACC_INT32E_T)
    ACCCHK_ASSERT(sizeof(ACC_UINT32E_C(0)) == SIZEOF_ACC_INT32E_T)
#endif
    ACCCHK_ASSERT((ACC_INT32E_C(1)  << (8*SIZEOF_ACC_INT32E_T-1)) < 0)
    ACCCHK_ASSERT((ACC_UINT32E_C(1) << (8*SIZEOF_ACC_INT32E_T-1)) > 0)
    ACCCHK_ASSERT((ACC_INT32E_C(1)  << (int)(8*sizeof(ACC_INT32E_C(1))-1)) < 0)
    ACCCHK_ASSERT((ACC_UINT32E_C(1) << (int)(8*sizeof(ACC_UINT32E_C(1))-1)) > 0)
    ACCCHK_ASSERT(ACC_INT32E_C(2147483647)      > 0)
    ACCCHK_ASSERT(ACC_INT32E_C(-2147483647) -1  < 0)
    ACCCHK_ASSERT(ACC_UINT32E_C(4294967295)     > 0)
    ACCCHK_ASSERT(ACC_UINT32E_C(4294967295) == ACC_0xffffffffL)
#endif


    ACCCHK_ASSERT(sizeof(acc_int32l_t) >= sizeof(int))
#if defined(acc_int32e_t)
    ACCCHK_ASSERT(sizeof(acc_int32l_t) >= sizeof(acc_int32e_t))
#endif

    ACCCHK_ASSERT(sizeof(acc_int32l_t) >= 4)
    ACCCHK_ASSERT(sizeof(acc_int32l_t) == SIZEOF_ACC_INT32L_T)
    ACCCHK_ASSERT(sizeof(acc_uint32l_t) >= 4)
    ACCCHK_ASSERT(sizeof(acc_int32l_t) == sizeof(acc_uint32l_t))

    ACCCHK_ASSERT_IS_SIGNED_T(acc_int32l_t)
    ACCCHK_ASSERT(((( (acc_int32l_t)1 << 30) + 1) >> 30) == 1)
    ACCCHK_ASSERT(((( ACC_INT32L_C(1) << 30) + 1) >> 30) == 1)

    ACCCHK_ASSERT_IS_UNSIGNED_T(acc_uint32l_t)
    ACCCHK_ASSERT(((( (acc_uint32l_t)1 << 31) + 1) >> 31) == 1)
    ACCCHK_ASSERT(((( ACC_UINT32L_C(1) << 31) + 1) >> 31) == 1)

#if (SIZEOF_ACC_INT32L_T < SIZEOF_INT)
    ACCCHK_ASSERT(sizeof(ACC_INT32L_C(0)) == sizeof(int))
    ACCCHK_ASSERT(sizeof(ACC_UINT32L_C(0)) == sizeof(int))
#else
    ACCCHK_ASSERT(sizeof(ACC_INT32L_C(0)) == SIZEOF_ACC_INT32L_T)
    ACCCHK_ASSERT(sizeof(ACC_UINT32L_C(0)) == SIZEOF_ACC_INT32L_T)
#endif
    ACCCHK_ASSERT((ACC_INT32L_C(1)  << (8*SIZEOF_ACC_INT32L_T-1)) < 0)
    ACCCHK_ASSERT((ACC_UINT32L_C(1) << (8*SIZEOF_ACC_INT32L_T-1)) > 0)
    ACCCHK_ASSERT((ACC_INT32L_C(1)  << (int)(8*sizeof(ACC_INT32L_C(1))-1)) < 0)
    ACCCHK_ASSERT((ACC_UINT32L_C(1) << (int)(8*sizeof(ACC_UINT32L_C(1))-1)) > 0)
    ACCCHK_ASSERT(ACC_INT32L_C(2147483647)      > 0)
    ACCCHK_ASSERT(ACC_INT32L_C(-2147483647) -1  < 0)
    ACCCHK_ASSERT(ACC_UINT32L_C(4294967295)     > 0)
    ACCCHK_ASSERT(ACC_UINT32L_C(4294967295) == ACC_0xffffffffL)


    ACCCHK_ASSERT(sizeof(acc_int32f_t) >= sizeof(int))
#if defined(acc_int32e_t)
    ACCCHK_ASSERT(sizeof(acc_int32f_t) >= sizeof(acc_int32e_t))
#endif
    ACCCHK_ASSERT(sizeof(acc_int32f_t) >= sizeof(acc_int32l_t))

    ACCCHK_ASSERT(sizeof(acc_int32f_t) >= 4)
    ACCCHK_ASSERT(sizeof(acc_int32f_t) >= sizeof(acc_int32l_t))
    ACCCHK_ASSERT(sizeof(acc_int32f_t) == SIZEOF_ACC_INT32F_T)
    ACCCHK_ASSERT(sizeof(acc_uint32f_t) >= 4)
    ACCCHK_ASSERT(sizeof(acc_uint32f_t) >= sizeof(acc_uint32l_t))
    ACCCHK_ASSERT(sizeof(acc_int32f_t) == sizeof(acc_uint32f_t))

    ACCCHK_ASSERT_IS_SIGNED_T(acc_int32f_t)
    ACCCHK_ASSERT(((( (acc_int32f_t)1 << 30) + 1) >> 30) == 1)
    ACCCHK_ASSERT(((( ACC_INT32F_C(1) << 30) + 1) >> 30) == 1)

    ACCCHK_ASSERT_IS_UNSIGNED_T(acc_uint32f_t)
    ACCCHK_ASSERT(((( (acc_uint32f_t)1 << 31) + 1) >> 31) == 1)
    ACCCHK_ASSERT(((( ACC_UINT32F_C(1) << 31) + 1) >> 31) == 1)

#if (SIZEOF_ACC_INT32F_T < SIZEOF_INT)
    ACCCHK_ASSERT(sizeof(ACC_INT32F_C(0)) == sizeof(int))
    ACCCHK_ASSERT(sizeof(ACC_UINT32F_C(0)) == sizeof(int))
#else
    ACCCHK_ASSERT(sizeof(ACC_INT32F_C(0)) == SIZEOF_ACC_INT32F_T)
    ACCCHK_ASSERT(sizeof(ACC_UINT32F_C(0)) == SIZEOF_ACC_INT32F_T)
#endif
    ACCCHK_ASSERT((ACC_INT32F_C(1)  << (8*SIZEOF_ACC_INT32F_T-1)) < 0)
    ACCCHK_ASSERT((ACC_UINT32F_C(1) << (8*SIZEOF_ACC_INT32F_T-1)) > 0)
    ACCCHK_ASSERT((ACC_INT32F_C(1)  << (int)(8*sizeof(ACC_INT32F_C(1))-1)) < 0)
    ACCCHK_ASSERT((ACC_UINT32F_C(1) << (int)(8*sizeof(ACC_UINT32F_C(1))-1)) > 0)
    ACCCHK_ASSERT(ACC_INT32F_C(2147483647)      > 0)
    ACCCHK_ASSERT(ACC_INT32F_C(-2147483647) -1  < 0)
    ACCCHK_ASSERT(ACC_UINT32F_C(4294967295)     > 0)
    ACCCHK_ASSERT(ACC_UINT32F_C(4294967295) == ACC_0xffffffffL)


#if defined(acc_int64l_t)
    ACCCHK_ASSERT(sizeof(acc_int64l_t) >= 8)
    ACCCHK_ASSERT(sizeof(acc_int64l_t) == SIZEOF_ACC_INT64L_T)
    ACCCHK_ASSERT(sizeof(acc_uint64l_t) >= 8)
    ACCCHK_ASSERT(sizeof(acc_int64l_t) == sizeof(acc_uint64l_t))

    ACCCHK_ASSERT_IS_SIGNED_T(acc_int64l_t)
    ACCCHK_ASSERT(((( (acc_int64l_t)1 << 62) + 1) >> 62) == 1)
    ACCCHK_ASSERT(((( ACC_INT64L_C(1) << 62) + 1) >> 62) == 1)

#if (ACC_CC_BORLANDC && (__BORLANDC__ < 0x0530))
    /* Borland C is broken */
#else
    ACCCHK_ASSERT_IS_UNSIGNED_T(acc_uint64l_t)
    ACCCHK_ASSERT(ACC_UINT64L_C(18446744073709551615)     > 0)
#endif
    ACCCHK_ASSERT(((( (acc_uint64l_t)1 << 63) + 1) >> 63) == 1)
    ACCCHK_ASSERT(((( ACC_UINT64L_C(1) << 63) + 1) >> 63) == 1)

#if defined(ACCCHK_CONFIG_PEDANTIC)
#if (ACC_CC_BORLANDC && (__BORLANDC__ < 0x0560))
    /* Borland C is broken */
#elif (SIZEOF_ACC_INT64L_T < SIZEOF_INT)
    ACCCHK_ASSERT(sizeof(ACC_INT64L_C(0)) == sizeof(int))
    ACCCHK_ASSERT(sizeof(ACC_UINT64L_C(0)) == sizeof(int))
#else
    ACCCHK_ASSERT(sizeof(ACC_INT64L_C(0)) == SIZEOF_ACC_INT64L_T)
    ACCCHK_ASSERT(sizeof(ACC_UINT64L_C(0)) == SIZEOF_ACC_INT64L_T)
#endif
#endif /* ACCCHK_CONFIG_PEDANTIC */
    ACCCHK_ASSERT((ACC_INT64L_C(1)  << (8*SIZEOF_ACC_INT64L_T-1)) < 0)
    ACCCHK_ASSERT((ACC_UINT64L_C(1) << (8*SIZEOF_ACC_INT64L_T-1)) > 0)
#if (ACC_CC_GNUC && (ACC_CC_GNUC < 0x020600ul))
    /* avoid pedantic warning */
    ACCCHK_ASSERT(ACC_INT64L_C(9223372036854775807)       > ACC_INT64L_C(0))
#else
    ACCCHK_ASSERT(ACC_INT64L_C(9223372036854775807)       > 0)
#endif
    ACCCHK_ASSERT(ACC_INT64L_C(-9223372036854775807) - 1  < 0)

    ACCCHK_ASSERT( ACC_INT64L_C(9223372036854775807) % 2147483629l  == 721)
    ACCCHK_ASSERT( ACC_INT64L_C(9223372036854775807) % 2147483647l  == 1)
    ACCCHK_ASSERT(ACC_UINT64L_C(9223372036854775807) % 2147483629ul == 721)
    ACCCHK_ASSERT(ACC_UINT64L_C(9223372036854775807) % 2147483647ul == 1)
#endif /* acc_int64l_t */


    ACCCHK_ASSERT_IS_SIGNED_T(acc_intptr_t)
    ACCCHK_ASSERT_IS_UNSIGNED_T(acc_uintptr_t)
    ACCCHK_ASSERT(sizeof(acc_intptr_t) >= sizeof(void *))
    ACCCHK_ASSERT(sizeof(acc_intptr_t) == SIZEOF_ACC_INTPTR_T)
    ACCCHK_ASSERT(sizeof(acc_intptr_t) == sizeof(acc_uintptr_t))


/*************************************************************************
// check memory model ACC_MM
**************************************************************************/

#if (ACC_MM_FLAT)
#if 0
    /* this is not a valid assumption -- disabled */
    ACCCHK_ASSERT(sizeof(void*) == sizeof(void (*)(void)))
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


/*************************************************************************
// check ACC_ARCH and ACC_OS
**************************************************************************/

#if (ACC_ARCH_IA16)
    ACCCHK_ASSERT(sizeof(size_t) == 2)
    ACCCHK_ASSERT(sizeof(acc_intptr_t) == sizeof(void *))
#elif (ACC_ARCH_IA32 || ACC_ARCH_M68K)
    ACCCHK_ASSERT(sizeof(size_t) == 4)
    ACCCHK_ASSERT(sizeof(ptrdiff_t) == 4)
    ACCCHK_ASSERT(sizeof(acc_intptr_t) == sizeof(void *))
#elif (ACC_ARCH_AMD64 || ACC_ARCH_IA64)
    ACCCHK_ASSERT(sizeof(size_t) == 8)
    ACCCHK_ASSERT(sizeof(ptrdiff_t) == 8)
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


/*************************************************************************
// check promotion rules
**************************************************************************/

#if (ACC_CC_NDPC)
    /* NDP C is broken */
#else
    /* check that the compiler correctly casts signed to unsigned */
    ACCCHK_ASSERT( (int) ((unsigned char) ((signed char) -1)) == 255)
#endif

#if (ACC_CC_KEILC)
    /* Keil C is broken */
#elif (ACC_CC_NDPC)
    /* NDP C is broken */
#elif !defined(ACC_BROKEN_INTEGRAL_PROMOTION) && (SIZEOF_INT > 1)
    /* check that the compiler correctly promotes integrals */
    ACCCHK_ASSERT( (((unsigned char)128) << (int)(8*sizeof(int)-8)) < 0)
#endif



/*
vi:ts=4:et
*/
