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


/*
 * Possible configuration values:
 *
 *   ACC_CONFIG_HEADER          if given, then use this as <config.h>
 *   ACC_CONFIG_INCLUDE         include path to acc_ files
 *   ACC_CONFIG_FRESSTANDING    only use <limits.h> and possibly <stdint.h>
 */


#define ACC_VERSION     20030325L

#if !defined(ACC_CONFIG_INCLUDE)
#  define ACC_CONFIG_INCLUDE(file)     file
#endif

#include ACC_CONFIG_INCLUDE("acc_init.h")
#include <limits.h>
#include ACC_CONFIG_INCLUDE("acc_os.h")
#include ACC_CONFIG_INCLUDE("acc_cc.h")
#include ACC_CONFIG_INCLUDE("acc_mm.h")
#include ACC_CONFIG_INCLUDE("acc_arch.h")
#include ACC_CONFIG_INCLUDE("acc_defs.h")

#if defined(ACC_CONFIG_HEADER)
#  include ACC_CONFIG_HEADER
#else
#  include ACC_CONFIG_INCLUDE("acc_auto.h")
#endif

#if !defined(ACC_CONFIG_FREESTANDING)
#  include ACC_CONFIG_INCLUDE("acc_incd.h")
#endif


/*
vi:ts=4:et
*/
