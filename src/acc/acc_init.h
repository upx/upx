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
// disable pedantic warnings
************************************************************************/

#if defined(__INTEL_COMPILER) && defined(__linux__)
#  pragma warning(disable: 193)     /* #193: zero used for undefined preprocessing identifier */
#endif

#if 0 && defined(__WATCOMC__)
#  if (__WATCOMC__ < 1000)
#    pragma warning 203 9           /* W203: Preprocessing symbol '%s' has not been declared */
#  endif
#endif


/***********************************************************************
// fix incorrect and missing stuff
************************************************************************/

#if defined(__CYGWIN32__) && !defined(__CYGWIN__)
#  define __CYGWIN__ __CYGWIN32__
#endif


/* Microsoft C does not correctly define ptrdiff_t for
 * the 16-bit huge memory model.
 */
#if defined(_MSC_VER) && defined(M_I86HM)
#  if (_MSC_VER <= 800)
#    define ptrdiff_t long
#    define _PTRDIFF_T_DEFINED
#  endif
#endif


/* Fix old compiler versions. */
#if defined(__PACIFIC__) && defined(DOS)
#  if !defined(__far)
#    define __far far
#  endif
#  if !defined(__near)
#    define __near near
#  endif
#elif defined(__TURBOC__) && defined(__MSDOS__)
#  if(__TURBOC__ < 0x0410)
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
#  endif
#endif


#if defined(__TOS__) && (defined(__PUREC__) || defined(__TURBOC__))
#  define ACC_BROKEN_SIZEOF 1
#endif


/***********************************************************************
// stdc macros
************************************************************************/

#if defined(__cplusplus)
#  if !defined(__STDC_LIMIT_MACROS)
#    define __STDC_LIMIT_MACROS 1
#  endif
#  if !defined(__STDC_CONSTANT_MACROS)
#    define __STDC_CONSTANT_MACROS 1
#  endif
#endif


/***********************************************************************
// preprocessor macros
************************************************************************/

#define ACC_STRINGIZE(x)        #x
#define ACC_MACRO_EXPAND(x)     ACC_STRINGIZE(x)


/*
vi:ts=4:et
*/
