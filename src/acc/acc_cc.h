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
 * C/C++ Compiler - exactly one of:
 *
 *   ACC_CC_UNKNOWN         [default]
 *   ACC_CC_GNUC
 *   ...
 */

#if defined(CIL) && defined(_GNUCC) && defined(__GNUC__)
#  define ACC_CC_CILLY          1
#  define ACC_INFO_CC           "Cilly"
#  define ACC_INFO_CCVER        "unknown"
#elif defined(__INTEL_COMPILER)
#  define ACC_CC_INTELC         1
#  define ACC_INFO_CC           "Intel C"
#  define ACC_INFO_CCVER        ACC_CPP_MACRO_EXPAND(__INTEL_COMPILER)
#elif defined(__POCC__)
#  define ACC_CC_PELLESC        1
#  define ACC_INFO_CC           "Pelles C"
#  define ACC_INFO_CCVER        ACC_CPP_MACRO_EXPAND(__POCC__)
#elif defined(__GNUC__) && defined(__VERSION__)
#  if defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
#    define ACC_CC_GNUC         (__GNUC__ * 0x10000L + __GNUC_MINOR__ * 0x100 + __GNUC_PATCHLEVEL__)
#  elif defined(__GNUC_MINOR__)
#    define ACC_CC_GNUC         (__GNUC__ * 0x10000L + __GNUC_MINOR__ * 0x100)
#  else
#    define ACC_CC_GNUC         (__GNUC__ * 0x10000L)
#  endif
#  define ACC_INFO_CC           "gcc"
#  define ACC_INFO_CCVER        __VERSION__
#elif defined(__AZTEC_C__)
#  define ACC_CC_AZTECC         1
#  define ACC_INFO_CC           "Aztec C"
#  define ACC_INFO_CCVER        ACC_CPP_MACRO_EXPAND(__AZTEC_C__)
#elif defined(__BORLANDC__)
#  define ACC_CC_BORLANDC       1
#  define ACC_INFO_CC           "Borland C"
#  define ACC_INFO_CCVER        ACC_CPP_MACRO_EXPAND(__BORLANDC__)
#elif defined(__DMC__)
#  define ACC_CC_DMC            1
#  define ACC_INFO_CC           "Digital Mars C"
#  define ACC_INFO_CCVER        ACC_CPP_MACRO_EXPAND(__DMC__)
#elif defined(__DECC)
#  define ACC_CC_DECC           1
#  define ACC_INFO_CC           "DEC C"
#  define ACC_INFO_CCVER        ACC_CPP_MACRO_EXPAND(__DECC)
#elif defined(__HIGHC__)
#  define ACC_CC_HIGHC          1
#  define ACC_INFO_CC           "MetaWare High C"
#  define ACC_INFO_CCVER        "unknown"
#elif defined(__IBMC__)
#  define ACC_CC_IBMC           1
#  define ACC_INFO_CC           "IBM C"
#  define ACC_INFO_CCVER        ACC_CPP_MACRO_EXPAND(__IBMC__)
#elif defined(__KEIL__) && defined(__C166__)
#  define ACC_CC_KEILC          1
#  define ACC_INFO_CC           "Keil C"
#  define ACC_INFO_CCVER        ACC_CPP_MACRO_EXPAND(__C166__)
#elif defined(__LCC__)
#  define ACC_CC_LCC            1
#  define ACC_INFO_CC           "lcc"
#  define ACC_INFO_CCVER        "unknown"
#elif defined(_MSC_VER)
#  define ACC_CC_MSC            1
#  define ACC_INFO_CC           "Microsoft C"
#  define ACC_INFO_CCVER        ACC_CPP_MACRO_EXPAND(_MSC_VER)
#elif defined(__MWERKS__)
#  define ACC_CC_MWERKS         1
#  define ACC_INFO_CC           "Metrowerks C"
#  define ACC_INFO_CCVER        ACC_CPP_MACRO_EXPAND(__MWERKS__)
#elif (defined(__NDPC__) || defined(__NDPX__)) && defined(__i386)
#  define ACC_CC_NDPC           1
#  define ACC_INFO_CC           "Microway NDP C"
#  define ACC_INFO_CCVER        "unknown"
#elif defined(__PACIFIC__)
#  define ACC_CC_PACIFICC       1
#  define ACC_INFO_CC           "Pacific C"
#  define ACC_INFO_CCVER        ACC_CPP_MACRO_EXPAND(__PACIFIC__)
#elif defined(__PGI) && (defined(__linux__) || defined(__WIN32__))
#  define ACC_CC_PGI            1
#  define ACC_INFO_CC           "Portland Group PGI C"
#  define ACC_INFO_CCVER        "unknown"
#elif defined(__PUREC__)
#  define ACC_CC_PUREC          1
#  define ACC_INFO_CC           "Pure C"
#  define ACC_INFO_CCVER        ACC_CPP_MACRO_EXPAND(__PUREC__)
#elif defined(__SC__)
#  define ACC_CC_SYMANTECC      1
#  define ACC_INFO_CC           "Symantec C"
#  define ACC_INFO_CCVER        ACC_CPP_MACRO_EXPAND(__SC__)
#elif defined(__SUNPRO_C)
#  define ACC_CC_SUNPROC        1
#  define ACC_INFO_CC           "Sun C"
#  define ACC_INFO_CCVER        "unknown"
#elif defined(__TINYC__)
#  define ACC_CC_TINYC          1
#  define ACC_INFO_CC           "Tiny C"
#  define ACC_INFO_CCVER        ACC_CPP_MACRO_EXPAND(__TINYC__)
#elif defined(__TSC__)
#  define ACC_CC_TOPSPEEDC      1
#  define ACC_INFO_CC           "TopSpeed C"
#  define ACC_INFO_CCVER        ACC_CPP_MACRO_EXPAND(__TSC__)
#elif defined(__WATCOMC__)
#  define ACC_CC_WATCOMC        1
#  define ACC_INFO_CC           "Watcom C"
#  define ACC_INFO_CCVER        ACC_CPP_MACRO_EXPAND(__WATCOMC__)
#elif defined(__TURBOC__)
#  define ACC_CC_TURBOC         1
#  define ACC_INFO_CC           "Turbo C"
#  define ACC_INFO_CCVER        ACC_CPP_MACRO_EXPAND(__TURBOC__)
#elif defined(__ZTC__)
#  define ACC_CC_ZORTECHC       1
#  define ACC_INFO_CC           "Zortech C"
#  if (__ZTC__ == 0x310)
#    define ACC_INFO_CCVER      "0x310"
#  else
#    define ACC_INFO_CCVER      ACC_CPP_MACRO_EXPAND(__ZTC__)
#  endif
#else
#  define ACC_CC_UNKNOWN        1
#  define ACC_INFO_CC           "unknown"
#  define ACC_INFO_CCVER        "unknown"
#endif


/*
vi:ts=4:et
*/
