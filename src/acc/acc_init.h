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
// preprocessor
************************************************************************/

/* workaround for preprocessor bugs in some compilers */
#if 0
#define ACC_0xffffL             0xfffful
#define ACC_0xffffffffL         0xfffffffful
#else
#define ACC_0xffffL             65535ul
#define ACC_0xffffffffL         4294967295ul
#endif

/* some things we cannot work around */
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


/***********************************************************************
// try to detect specific compilers
************************************************************************/

#if (UINT_MAX == ACC_0xffffL)
#if defined(__ZTC__) && defined(__I86__) && !defined(__OS2__)
#  if !defined(MSDOS)
#    define MSDOS 1
#  endif
#  if !defined(_MSDOS)
#    define _MSDOS 1
#  endif
#elif defined(__VERSION) && defined(MB_LEN_MAX)
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


/***********************************************************************
// fix incorrect and missing stuff
************************************************************************/

/* Microsoft C does not correctly define ptrdiff_t for
 * the 16-bit huge memory model.
 */
#if defined(_MSC_VER) && defined(M_I86HM) && (UINT_MAX == ACC_0xffffL)
#  define ptrdiff_t long
#  define _PTRDIFF_T_DEFINED
#endif


/* Fix old compiler versions. */
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


/***********************************************************************
// ANSI C preprocessor macros (cpp)
************************************************************************/

#define ACC_CPP_STRINGIZE(x)            #x
#define ACC_CPP_MACRO_EXPAND(x)         ACC_CPP_STRINGIZE(x)

/* concatenate */
#define ACC_CPP_CONCAT2(a,b)            a ## b
#define ACC_CPP_CONCAT3(a,b,c)          a ## b ## c
#define ACC_CPP_CONCAT4(a,b,c,d)        a ## b ## c ## d
#define ACC_CPP_CONCAT5(a,b,c,d,e)      a ## b ## c ## d ## e

/* expand and concatenate (by using one level of indirection) */
#define ACC_CPP_ECONCAT2(a,b)           ACC_CPP_CONCAT2(a,b)
#define ACC_CPP_ECONCAT3(a,b,c)         ACC_CPP_CONCAT3(a,b,c)
#define ACC_CPP_ECONCAT4(a,b,c,d)       ACC_CPP_CONCAT4(a,b,c,d)
#define ACC_CPP_ECONCAT5(a,b,c,d,e)     ACC_CPP_CONCAT5(a,b,c,d,e)


/***********************************************************************
// stdc macros
************************************************************************/

#if defined(__cplusplus)
#  undef __STDC_CONSTANT_MACROS
#  undef __STDC_LIMIT_MACROS
#  define __STDC_CONSTANT_MACROS 1
#  define __STDC_LIMIT_MACROS 1
#endif


/***********************************************************************
// misc macros
************************************************************************/

#if defined(__cplusplus)
#  define ACC_EXTERN_C extern "C"
#else
#  define ACC_EXTERN_C extern
#endif


/*
vi:ts=4:et
*/
