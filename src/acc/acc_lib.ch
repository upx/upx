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



#if !defined(ACC_VERSION)
#  if defined ACC_CONFIG_INCLUDE
#    include ACC_CONFIG_INCLUDE("acc.h")
#  else
#    include "acc.h"
#  endif
#endif
#ifndef __ACC_INCD_H_INCLUDED
#  include ACC_CONFIG_INCLUDE("acc_incd.h")
#endif
#ifndef __ACC_INCE_H_INCLUDED
#  include ACC_CONFIG_INCLUDE("acc_ince.h")
#endif
#ifndef __ACC_INCI_H_INCLUDED
#  include ACC_CONFIG_INCLUDE("acc_inci.h")
#endif
#ifndef __ACC_LIB_H_INCLUDED
#  include ACC_CONFIG_INCLUDE("acc_lib.h")
#endif


#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif

#include ACC_CONFIG_INCLUDE("acclib/bele.ch")
#include ACC_CONFIG_INCLUDE("acclib/hmemcpy.ch")
#include ACC_CONFIG_INCLUDE("acclib/hstring.ch")
#include ACC_CONFIG_INCLUDE("acclib/halloc.ch")
#include ACC_CONFIG_INCLUDE("acclib/dosalloc.ch")
#include ACC_CONFIG_INCLUDE("acclib/hfread.ch")
#include ACC_CONFIG_INCLUDE("acclib/hread.ch")
#include ACC_CONFIG_INCLUDE("acclib/opendir.ch")
#include ACC_CONFIG_INCLUDE("acclib/rand.ch")
#include ACC_CONFIG_INCLUDE("acclib/misc.ch")

#if 0
/* modules which use floating point are not included by default */
#include ACC_CONFIG_INCLUDE("acclib/uclock.ch")
#endif


/*
vi:ts=4:et
*/
