/* tailor.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2002 Laszlo Molnar
   All Rights Reserved.

   UPX and the UCL library are free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer              Laszlo Molnar
   <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>
 */


#if defined(__CYGWIN32__) && !defined(__CYGWIN__)
#  define __CYGWIN__ __CYGWIN32__
#endif


/*************************************************************************
//
**************************************************************************/

#if !defined(__MFX_DOS) && !defined(__MFX_WIN) && !defined(__MFX_OS2)
#if !defined(__MFX_TOS) && !defined(__MFX_PALMOS)
#  if defined(__WINDOWS__) || defined(_WINDOWS) || defined(_Windows)
#    define __MFX_WIN
#  elif defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
#    define __MFX_WIN
#  elif defined(__CYGWIN__) || defined(__MINGW32__)
#    define __MFX_WIN
#  elif defined(__NT__) || defined(__NT_DLL__) || defined(__WINDOWS_386__)
#    define __MFX_WIN
#  elif defined(__DOS__) || defined(__MSDOS__) || defined(MSDOS)
#    define __MFX_DOS
#  elif defined(__OS2__) || defined(__OS2V2__) || defined(OS2)
#      define __MFX_OS2
#  elif defined(__TOS__) || defined(__atarist__)
#    define __MFX_TOS
#  endif
#endif
#endif

#if defined(__MFX_DOS) && !defined(__MFX_DOS16) && !defined(__MFX_DOS32)
#  define __MFX_DOS32
#endif
#if defined(__MFX_WIN) && !defined(__MFX_WIN16) && !defined(__MFX_WIN32)
#  define __MFX_WIN32
#endif

#if !defined(DOSISH)
#  if defined(__MFX_DOS) || defined(__MFX_WIN)
#    define DOSISH
#  elif defined(__MFX_OS2) || defined(__EMX__)
#    define DOSISH
#  elif defined(__MFX_TOS)
#    define DOSISH
#  endif
#endif

#if defined(DOSISH)
#  define HAVE_SIGNAL_H 1
#  define HAVE_CTIME 1
#  define HAVE_FILENO 1
#  define HAVE_GMTIME 1
#  define HAVE_LOCALTIME 1
#  define HAVE_MEMCMP 1
#  define HAVE_MEMCPY 1
#  define HAVE_MEMMOVE 1
#  define HAVE_MEMSET 1
#  define HAVE_SETMODE 1
#  define HAVE_STRCHR 1
#  define HAVE_STRDUP 1
#  define HAVE_STRFTIME 1
#  if defined(__CYGWIN__)
#    define HAVE_STRCASECMP 1
#    define HAVE_STRNCASECMP 1
#  else
#    define HAVE_STRICMP 1
#    define HAVE_STRNICMP 1
#  endif
#  if !defined(DIR_SEP)
#    define DIR_SEP         "/\\"
#  endif
#  if !defined(fn_tolower)
#    define fn_tolower(x)   tolower(((unsigned char)(x)))
#  endif
#  undef __UNIX__
#  undef UNIX
#  undef __unix__
#  undef unix
#endif

#if defined(__DJGPP__) || defined(__EMX__) || defined(__CYGWIN__)
#  define TIME_WITH_SYS_TIME 1
#  define HAVE_IO_H 1
#  define HAVE_UNISTD_H 1
#  define HAVE_UTIME_H 1
#  define HAVE_MODE_T 1
#  define HAVE_CHMOD 1
#  define HAVE_GETTIMEOFDAY 1
#  define HAVE_UTIME 1
#elif defined(__MINGW32__)
#  define TIME_WITH_SYS_TIME 1
#  define HAVE_CONIO_H 1
#  define HAVE_IO_H 1
#  define HAVE_SHARE_H 1
#  define HAVE_UNISTD_H 1
#  define HAVE_SYS_UTIME_H 1
#  define HAVE_MODE_T 1
#  define HAVE_CHMOD 1
#  define HAVE_UTIME 1
#elif defined(__MINT__)
#  undef HAVE_SETMODE
#  define TIME_WITH_SYS_TIME 1
#  define HAVE_UNISTD_H 1
#  define HAVE_UTIME_H 1
#  define HAVE_CHMOD 1
#  define HAVE_CHOWN 1
#  define HAVE_UTIME 1
#elif defined(__BORLANDC__)
#  if (__BORLANDC__ < 0x551)
#    error "need Borland C 5.5.1 or newer"
#  endif
#  define __UPX_CDECL       __cdecl
#  define SIGTYPEENTRY      __cdecl
#  define HAVE_CONIO_H 1
#  define HAVE_IO_H 1
#  define HAVE_MALLOC_H 1
#  define HAVE_CHMOD 1
#  define HAVE_SHARE_H 1
#  define HAVE_UTIME_H 1
#  define HAVE_UTIME 1
#  define HAVE_VSNPRINTF 1
#  define vsnprintf _vsnprintf
#elif defined(__DMC__)
#  define __UPX_CDECL       __cdecl
#  define SIGTYPEENTRY      __cdecl
#  define HAVE_IO_H 1
#  define HAVE_MALLOC_H 1
#  define HAVE_UNISTD_H 1
#  define HAVE_UTIME_H 1
#  define HAVE_MODE_T 1
#  define HAVE_CHMOD 1
#  define HAVE_UTIME 1
#elif defined(_MSC_VER)
#  define __UPX_CDECL       __cdecl
#  define SIGTYPEENTRY      __cdecl
#  define HAVE_CONIO_H 1
#  define HAVE_IO_H 1
#  define HAVE_MALLOC_H 1
#  define HAVE_CHMOD 1
#  if (_MSC_VER >= 1000)
#    define HAVE_SHARE_H 1
#    define HAVE_SYS_UTIME_H 1
#    define HAVE_UTIME 1
#    define HAVE_VSNPRINTF 1
#    define vsnprintf _vsnprintf
//#    pragma warning(once: 4097 4710)
//#    pragma warning(disable: 4097 4511 4512 4710)
#    pragma warning(disable: 4097)      // W3: typedef-name 'A' used as synonym for class-name 'B'
#    pragma warning(disable: 4511)      // W3: 'class': copy constructor could not be generated
#    pragma warning(disable: 4512)      // W4: 'class': assignment operator could not be generated
#    pragma warning(disable: 4710)      // W4: 'function': function not inlined
#  endif
#elif defined(__WATCOMC__)
#  if (__WATCOMC__ < 1100)
#    error "need Watcom C 11.0c or newer"
#    define NO_BOOL 1
#  endif
#  define __UPX_CDECL       __cdecl
#  define HAVE_IO_H 1
#  define HAVE_SYS_UTIME_H 1
#  define HAVE_CHMOD 1
#  define HAVE_UTIME 1
#  define HAVE_VSNPRINTF 1
#  define vsnprintf _vsnprintf
#  if defined(__cplusplus)
#    pragma warning 656 9               // w5: define this function inside its class definition (may improve code quality)
#  endif
#endif

#if defined(__MFX_DOS)
#  define HAVE_DOS_H 1
#endif


/*************************************************************************
//
**************************************************************************/

#ifndef DIR_SEP
#  define DIR_SEP       "/"
#endif

#ifndef OPTIONS_VAR
#  define OPTIONS_VAR   "UPX"
#endif

#ifndef fn_tolower
#  define fn_tolower(x) (x)
#endif


/*
vi:ts=4:et
*/

