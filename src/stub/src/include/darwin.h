/* darwin.h -- common stuff for the Darwin stub loaders

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2013 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2013 Laszlo Molnar
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
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
 */


// NOTE:
//   to avoid endless problems with moving libc and kernel headers, this
//   section is now completely freestanding


#if defined(__GNUC__)
#  if defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
#    define ACC_CC_GNUC         (__GNUC__ * 0x10000L + __GNUC_MINOR__ * 0x100 + __GNUC_PATCHLEVEL__)
#  elif defined(__GNUC_MINOR__)
#    define ACC_CC_GNUC         (__GNUC__ * 0x10000L + __GNUC_MINOR__ * 0x100)
#  else
#    define ACC_CC_GNUC         (__GNUC__ * 0x10000L)
#  endif
#endif

#define ACC_UNUSED(var)         ((void) var)


/*************************************************************************
//
**************************************************************************/

// <stddef.h>
typedef long ptrdiff_t;
typedef long ssize_t;
typedef unsigned long size_t;
// <stdint.h>
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned uint32_t;
#if (ACC_CC_GNUC >= 0x020800ul)
__extension__ typedef long long int64_t;
__extension__ typedef unsigned long long uint64_t;
#elif defined(_WIN32)
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
typedef long long int64_t;
typedef unsigned long long uint64_t;
#endif
typedef size_t uintptr_t;

// XXX: restrict ourselves to 4GB for off_t.  Some versions of gcc
// have bugs in handling 64-bit integers (such as passing as argument
// the wrong registers) and it takes more code anyway.
// Adjust in system call wrappers, particularly mmap() and pread().
typedef unsigned off_t;

// misc constants

#define PAGE_MASK       (~0ul<<12)
#define PAGE_SIZE       (-PAGE_MASK)

#define O_RDONLY        0


/*************************************************************************
// syscalls
**************************************************************************/

int close(int);
void exit(int) __attribute__((__noreturn__,__nothrow__));
int mprotect(void *, size_t, int);
extern int munmap(char *, size_t);
int open(char const *, unsigned, unsigned);


/*************************************************************************
// UPX stuff
**************************************************************************/

#define UPX_MAGIC_LE32  0x21585055          // "UPX!"


#define nrv_byte unsigned char
typedef unsigned int nrv_uint;


#define CONST_CAST(type, var) \
    ((type) ((uintptr_t) (var)))


#if !defined(__attribute_cdecl)
#if defined(__i386__)
#  if (ACC_CC_GNUC >= 0x030300)
#    define __attribute_cdecl   __attribute__((__cdecl__, __used__))
#  elif (ACC_CC_GNUC >= 0x020700)
#    define __attribute_cdecl   __attribute__((__cdecl__))
#  endif
#endif
#endif
#if !defined(__attribute_cdecl)
#  define __attribute_cdecl /*empty*/
#endif


/*
vi:ts=4:et:nowrap
*/

