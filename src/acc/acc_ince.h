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


#ifndef __ACC_INCE_H_INCLUDED
#define __ACC_INCE_H_INCLUDED 1

/* extra system includes */

#if defined(HAVE_STDARG_H)
#  include <stdarg.h>
#endif
#if defined(HAVE_CTYPE_H)
#  include <ctype.h>
#endif
#if defined(HAVE_ERRNO_H)
#  include <errno.h>
#endif
#if defined(HAVE_MALLOC_H)
#  include <malloc.h>
#endif
#if defined(HAVE_ALLOCA_H)
#  include <alloca.h>
#endif
#if defined(HAVE_FCNTL_H)
#  include <fcntl.h>
#endif
#if defined(HAVE_DIRENT_H)
#  include <dirent.h>
#endif
#if defined(HAVE_SETJMP_H)
#  include <setjmp.h>
#endif
#if defined(HAVE_SIGNAL_H)
#  include <signal.h>
#endif
#if defined(TIME_WITH_SYS_TIME)
#  include <sys/time.h>
#  include <time.h>
#elif defined(HAVE_TIME_H)
#  include <time.h>
#endif
#if defined(HAVE_UTIME_H)
#  include <utime.h>
#elif defined(HAVE_SYS_UTIME_H)
#  include <sys/utime.h>
#endif

/* DOS, OS/2 & Windows */
#if defined(HAVE_IO_H)
#  include <io.h>
#endif
#if defined(HAVE_DOS_H)
#  include <dos.h>
#endif
#if defined(HAVE_DIRECT_H)
#  include <direct.h>
#endif
#if defined(HAVE_SHARE_H)
#  include <share.h>
#endif
#if defined(ACC_CC_NDPC)
#  include <os.h>
#endif

/* TOS */
#if defined(__TOS__) && (defined(__PUREC__) || defined(__TURBOC__))
#  include <ext.h>
#endif

#endif /* already included */


/*
vi:ts=4:et
*/
