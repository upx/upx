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


#define __ACCLIB_WILDARGV_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif


/*************************************************************************
//
**************************************************************************/

#if (ACC_OS_DOS16 || ACC_OS216 || ACC_OS_WIN16)
#if 0 && (ACC_CC_MSC)

/* FIXME */
ACC_EXTERN_C int __acc_cdecl __setargv(void);
ACC_EXTERN_C int __acc_cdecl _setargv(void);
ACC_EXTERN_C int __acc_cdecl _setargv(void) { return __setargv(); }

#endif
#endif


#if (ACC_OS_WIN32 || ACC_OS_WIN64)
#if (ACC_CC_INTELC || ACC_CC_MSC)

ACC_EXTERN_C int __acc_cdecl __setargv(void);
ACC_EXTERN_C int __acc_cdecl _setargv(void);
ACC_EXTERN_C int __acc_cdecl _setargv(void) { return __setargv(); }

#endif
#endif


#if (ACC_OS_EMX)

#define __ACCLIB_HAVE_ACC_WILDARGV 1
ACCLIB_PUBLIC(void, acc_wildargv) (int* argc, char*** argv)
{
    _response(argc, argv);
    _wildcard(argc, argv);
}

#endif


#if !defined(__ACCLIB_HAVE_ACC_WILDARGV)
#define __ACCLIB_HAVE_ACC_WILDARGV 1
ACCLIB_PUBLIC(void, acc_wildargv) (int* argc, char*** argv)
{
    ACC_UNUSED(argc); ACC_UNUSED(argv);
}
#endif


/*
vi:ts=4:et
*/
