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


#ifndef __ACC_INCI_H_INCLUDED
#define __ACC_INCI_H_INCLUDED


/*************************************************************************
// internal system includes
**************************************************************************/

#if (ACC_OS_TOS && (ACC_CC_PUREC || ACC_CC_TURBOC))
#  include <tos.h>
#elif (ACC_OS_WIN32 || ACC_OS_WIN64 || ACC_OS_CYGWIN || (ACC_OS_EMX && defined(__RSXNT__)))
#  if (ACC_CC_WATCOMC && __WATCOMC__ < 1000)
#  else
#    if 1 && !defined(WIN32_LEAN_AND_MEAN)
#      define WIN32_LEAN_AND_MEAN 1
#    endif
#    include <windows.h>
#    define ACC_H_WINDOWS_H 1
#  endif
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
#  elif defined(__DJGPP__)
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
#  if 0
#    include <os2.h>
#  else
     unsigned short __far __pascal DosAllocHuge(unsigned short, unsigned short, unsigned short __far *, unsigned short, unsigned short);
     unsigned short __far __pascal DosFreeSeg(unsigned short);
#  endif
#endif


/*************************************************************************
//
**************************************************************************/

#if (ACC_OS_DOS16 || ACC_OS_OS216 || ACC_OS_WIN16)
#  if defined(FP_OFF)
#    define ACC_FP_OFF      FP_OFF
#  elif defined(_FP_OFF)
#    define ACC_FP_OFF      _FP_OFF
#  else
#    define ACC_FP_OFF(x)   (((const unsigned __far*)&(x))[0])
#  endif
#  if defined(FP_SEG)
#    define ACC_FP_SEG      FP_SEG
#  elif defined(_FP_SEG)
#    define ACC_FP_SEG      _FP_SEG
#  else
#    define ACC_FP_SEG(x)   (((const unsigned __far*)&(x))[1])
#  endif
#  if defined(MK_FP)
#    define ACC_MK_FP       MK_FP
#  elif defined(_MK_FP)
#    define ACC_MK_FP       _MK_FP
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
