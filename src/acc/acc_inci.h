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


#ifndef __ACC_INCI_H_INCLUDED
#define __ACC_INCI_H_INCLUDED 1


/*************************************************************************
// internal system includes
**************************************************************************/

#if (ACC_OS_TOS && (ACC_CC_PUREC || ACC_CC_TURBOC))
#  include <tos.h>
#elif (ACC_HAVE_WINDOWS_H)
#  if 1 && !defined(WIN32_LEAN_AND_MEAN)
#    define WIN32_LEAN_AND_MEAN 1
#  endif
#  if 1 && !defined(_WIN32_WINNT)
     /* Restrict to a subset of Windows NT 4.0 header files */
#    define _WIN32_WINNT 0x0400
#  endif
#  include <windows.h>
#  if (ACC_CC_BORLANDC || ACC_CC_TURBOC)
#    include <dir.h>
#  endif
#elif (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_WIN16)
#  if (ACC_CC_AZTECC)
#    include <model.h>
#    include <stat.h>
#  elif (ACC_CC_BORLANDC || ACC_CC_TURBOC)
#    include <alloc.h>
#    include <dir.h>
#  elif (ACC_OS_DOS32 && ACC_CC_GNUC) && defined(__DJGPP__)
#    include <sys/exceptn.h>
#  elif (ACC_CC_PACIFICC)
#    include <unixio.h>
#    include <stat.h>
#    include <sys.h>
#  elif (ACC_CC_WATCOMC)
#    include <i86.h>
#  endif
#elif (ACC_OS_OS216)
#  if (ACC_CC_WATCOMC)
#    include <i86.h>
#  endif
#endif


/*************************************************************************
//
**************************************************************************/

#if (ACC_OS_DOS16 || ACC_OS_OS216 || ACC_OS_WIN16)
#  if defined(FP_OFF)
#    define ACC_FP_OFF(x)   FP_OFF(x)
#  elif defined(_FP_OFF)
#    define ACC_FP_OFF(x)   _FP_OFF(x)
#  else
#    define ACC_FP_OFF(x)   (((const unsigned __far*)&(x))[0])
#  endif
#  if defined(FP_SEG)
#    define ACC_FP_SEG(x)   FP_SEG(x)
#  elif defined(_FP_SEG)
#    define ACC_FP_SEG(x)   _FP_SEG(x)
#  else
#    define ACC_FP_SEG(x)   (((const unsigned __far*)&(x))[1])
#  endif
#  if defined(MK_FP)
#    define ACC_MK_FP(s,o)  MK_FP(s,o)
#  elif defined(_MK_FP)
#    define ACC_MK_FP(s,o)  _MK_FP(s,o)
#  else
#    define ACC_MK_FP(s,o)  ((void __far*)(((unsigned long)(s)<<16)+(unsigned)(o)))
#  endif
#  if 0
#    undef ACC_FP_OFF
#    undef ACC_FP_SEG
#    undef ACC_MK_FP
#    define ACC_FP_OFF(x)   (((const unsigned __far*)&(x))[0])
#    define ACC_FP_SEG(x)   (((const unsigned __far*)&(x))[1])
#    define ACC_MK_FP(s,o)  ((void __far*)(((unsigned long)(s)<<16)+(unsigned)(o)))
#  endif
#endif


#endif /* already included */


/*
vi:ts=4:et
*/
