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

#if (ACC_CC_GNUC >= 0x020800l)     /* 2.8.0 */
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
#if (SIZEOF___INT64 > 0)
__acc_gnuc_extension__ typedef __int64 acc_int64_t;
#endif
#if (SIZEOF_UNSIGNED___INT64 > 0)
__acc_gnuc_extension__ typedef unsigned __int64 acc_uint64_t;
#endif


#if !defined(ACC_UINT32_C)
#  if defined(__PACIFIC__) && defined(DOS)
     /* workaround Pacific C */
#    define ACC_UINT32_C(c)     c
#  elif (UINT_MAX < __ACC_UINT_MAX(32))
#    define ACC_UINT32_C(c)     c ## UL
#  else
#    define ACC_UINT32_C(c)     c ## U
#  endif
#endif



/*
vi:ts=4:et
*/
