/* linux.hh -- common stuff the the Linux stub loaders

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


#if !defined(__linux__) || !defined(__i386__)
#  error "this stub must be compiled under linux/i386"
#endif


/*************************************************************************
// includes
**************************************************************************/

struct timex;

#define __need_timeval
#include <sys/types.h>
#include <sys/resource.h>
#include <elf.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/time.h>
#include <time.h>
#include <linux/errno.h>
#include <linux/mman.h>
//#include <linux/personality.h>
//#include <linux/timex.h>
#include <linux/unistd.h>


#define CONST_CAST(type, var) \
    ((type) ((unsigned long) (var)))


/*************************************************************************
// constants and types
**************************************************************************/

// !!! must be the same as in p_unix.h !!!
#define OVERHEAD        2048

#define UPX_MAGIC_LE32  0x21585055          // "UPX!"

// patch constants for our loader (le32 format)
#define UPX1            0x31585055          // "UPX1"
#define UPX2            0x32585055          // "UPX2"
#define UPX3            0x33585055          // "UPX4"
#define UPX4            0x34585055          // "UPX4"
#define UPX5            0x35585055          // "UPX5"


#undef int32_t
#undef uint32_t
#define int32_t         int
#define uint32_t        unsigned int


typedef int nrv_int;
typedef int nrv_int32;
typedef unsigned int nrv_uint;
typedef unsigned int nrv_uint32;
#define nrv_byte unsigned char
#define nrv_bytep unsigned char *
#define nrv_voidp void *


// From ../p_unix.h
struct b_info { // 12-byte header before each compressed block
    unsigned sz_unc;  // uncompressed_size
    unsigned sz_cpr;  //   compressed_size
    unsigned char b_method;  // compression algorithm
    unsigned char b_ftid;  // filter id
    unsigned char b_cto8;  // filter parameter
    unsigned char b_unused;
};

struct l_info       // 12-byte trailer in header for loader (offset 116)
{
    uint32_t l_checksum;
    uint32_t l_magic;
    uint16_t l_lsize;
    uint8_t  l_version;
    uint8_t  l_format;
};

struct p_info       // 12-byte packed program header follows stub loader
{
    uint32_t p_progid;
    uint32_t p_filesize;
    uint32_t p_blocksize;
};


#define SEEK_SET    0
#define SEEK_CUR    1

#define PAGE_MASK   (~0u<<12)   // discards the offset, keeps the page
#define PAGE_SIZE   ( 1u<<12)


/*************************************************************************
// syscalls
//
// Because of different <asm/unistd.h> versions and subtle bugs
// in both gcc and egcs we define all syscalls manually.
//
// Also, errno conversion is not necessary in our case, and we
// use optimized assembly statements to further decrease the size.
**************************************************************************/

#undef _syscall0
#undef _syscall1
#undef _syscall2
#undef _syscall3
#ifndef __NR__exit
#  define __NR__exit __NR_exit
#endif

#define Z0(x)    (__builtin_constant_p(x) && (long)(x) == 0)
#define Z1(x)    (__builtin_constant_p(x) && (long)(x) >= -128 && (long)(x) <= 127)

#define _syscall0(type,name) \
type name(void) \
{ \
    long __res; \
    if (Z1(__NR_##name)) { \
        __asm__ __volatile__ ("push %1; popl %0; int $0x80" \
            : "=a" (__res) \
            : "g" (__NR_##name)); \
    } else { \
        __asm__ __volatile__ ("int $0x80" \
            : "=a" (__res) \
            : "a" (__NR_##name)); \
    } \
    return (type) __res; \
}

#define _syscall1(type,name,type1,arg1) \
type name(type1 arg1) \
{ \
    long __res; \
    if (Z1(__NR_##name)) { \
        if (Z0(arg1)) { \
            __asm__ __volatile__ ("xorl %%ebx,%%ebx; push %1; popl %0; int $0x80" \
                : "=a" (__res) \
                : "g" (__NR_##name) \
                : "ebx"); \
        } else if (Z1(arg1)) { \
            __asm__ __volatile__ ("push %2; popl %%ebx; push %1; popl %0; int $0x80" \
                : "=a" (__res) \
                : "g" (__NR_##name),"g" ((long)(arg1)) \
                : "ebx"); \
        } else { \
            __asm__ __volatile__ ("push %1; popl %0; int $0x80" \
                : "=a" (__res) \
                : "g" (__NR_##name),"b" ((long)(arg1))); \
        } \
    } else { \
        __asm__ __volatile__ ("int $0x80" \
            : "=a" (__res) \
            : "a" (__NR_##name),"b" ((long)(arg1))); \
    } \
    return (type) __res; \
}

// special for mmap; somehow Z0(arg1) and Z1(arg1) don't work
#define _syscall1m(type,name,type1,arg1) \
type name(type1 arg1) \
{ \
    long __res; \
            __asm__ __volatile__ ("push %1; popl %0; int $0x80" \
                : "=a" (__res) \
                : "g" (__NR_##name),"b" ((long)(arg1))); \
    return (type) __res; \
}

#define _syscall2(type,name,type1,arg1,type2,arg2) \
type name(type1 arg1,type2 arg2) \
{ \
    long __res; \
    if (Z1(__NR_##name)) { \
        if (Z0(arg1) && Z0(arg2)) { \
            __asm__ __volatile__ ("xorl %%ecx,%%ecx; xorl %%ebx,%%ebx; push %1; popl %0; int $0x80" \
                : "=a" (__res) \
                : "g" (__NR_##name) \
                : "ebx", "ecx"); \
        } else if (Z0(arg1) && Z1(arg2)) { \
            __asm__ __volatile__ ("push %2; popl %%ecx; xorl %%ebx,%%ebx; push %1; popl %0; int $0x80" \
                : "=a" (__res) \
                : "g" (__NR_##name),"g" ((long)(arg2)) \
                : "ebx", "ecx"); \
        } else if (Z1(arg1) && Z0(arg2)) { \
            __asm__ __volatile__ ("xorl %%ecx,%%ecx; push %2; popl %%ebx; push %1; popl %0; int $0x80" \
                : "=a" (__res) \
                : "g" (__NR_##name),"g" ((long)(arg1)) \
                : "ebx", "ecx"); \
        } else if (Z1(arg1) && Z1(arg2)) { \
            __asm__ __volatile__ ("push %3; popl %%ecx; push %2; popl %%ebx; push %1; popl %0; int $0x80" \
                : "=a" (__res) \
                : "g" (__NR_##name),"g" ((long)(arg1)),"g" ((long)(arg2)) \
                : "ebx", "ecx"); \
        } else if (Z0(arg1)) { \
            __asm__ __volatile__ ("xorl %%ebx,%%ebx; push %1; popl %0; int $0x80" \
                : "=a" (__res) \
                : "g" (__NR_##name),"c" ((long)(arg2)) \
                : "ebx"); \
        } else if (Z0(arg2)) { \
            __asm__ __volatile__ ("push %1; popl %0; xorl %%ecx,%%ecx; int $0x80" \
                : "=a" (__res) \
                : "g" (__NR_##name),"b" ((long)(arg1)) \
                : "ecx"); \
        } else if (Z1(arg1)) { \
            __asm__ __volatile__ ("push %1; popl %0; push %2; popl %%ebx; int $0x80" \
                : "=a" (__res) \
                : "g" (__NR_##name),"g" ((long)(arg1)),"c" ((long)(arg2)) \
                : "ebx"); \
        } else if (Z1(arg2)) { \
            __asm__ __volatile__ ("push %1; popl %0; push %3; popl %%ecx; int $0x80" \
                : "=a" (__res) \
                : "g" (__NR_##name),"b" ((long)(arg1)),"g" ((long)(arg2)) \
                : "ecx"); \
        } else { \
            __asm__ __volatile__ ("push %1; popl %0; int $0x80" \
                : "=a" (__res) \
                : "g" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2))); \
        } \
    } else { \
        __asm__ __volatile__ ("int $0x80" \
            : "=a" (__res) \
            : "a" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2))); \
    } \
    return (type) __res; \
}

#define _syscall3(type,name,type1,arg1,type2,arg2,type3,arg3) \
type name(type1 arg1,type2 arg2,type3 arg3) \
{ \
    long __res; \
    if (Z1(__NR_##name)) { \
        __asm__ __volatile__ ("push %1; popl %0; int $0x80" \
            : "=a" (__res) \
            : "g" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)), \
                                "d" ((long)(arg3))); \
    } else { \
        __asm__ __volatile__ ("int $0x80" \
            : "=a" (__res) \
            : "a" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)), \
                                "d" ((long)(arg3))); \
    } \
    return (type) __res; \
}

#define access          syscall_access
#define fcntl           syscall_fcntl
#define getcwd          syscall_getcwd
#define getrusage       syscall_getrusage
#define gettimeofday    syscall_gettimeofday
#define nanosleep       syscall_nanosleep
#define open            syscall_open
#define personality     syscall_personality

static inline _syscall2(int,access,const char *,file,int,mode)
static inline _syscall1(int,adjtimex,struct timex *,ntx)
static inline _syscall1(void *,brk,void *,high)
static inline _syscall1(int,close,int,fd)
static inline _syscall3(int,execve,const char *,file,char **,argv,char **,envp)
static inline _syscall1(int,_exit,int,exitcode)
static inline _syscall3(int,fcntl,int,fd,int,cmd,long,arg)
static inline _syscall2(int,ftruncate,int,fd,size_t,len)
static inline _syscall0(pid_t,fork)
static inline _syscall2(int,getcwd,char *,buf,unsigned long,size);
static inline _syscall0(pid_t,getpid)
static inline _syscall2(int,getrusage,int,who,struct rusage *,usage);
static inline _syscall2(int,gettimeofday,struct timeval *,tv,void *,tz)
static inline _syscall3(off_t,lseek,int,fd,off_t,offset,int,whence)
static inline _syscall1m(caddr_t,mmap,const int *,args)
static inline _syscall3(int,mprotect,void *,addr,size_t,len,int,prot)
static inline _syscall3(int,msync,const void *,start,size_t,length,int,flags)
static inline _syscall2(int,munmap,void *,start,size_t,length)
static inline _syscall2(int,nanosleep,const struct timespec *,rqtp,struct timespec *,rmtp)
static inline _syscall3(int,open,const char *,file,int,flag,int,mode)
static inline _syscall1(int,personality,unsigned long,persona)
static inline _syscall3(int,read,int,fd,char *,buf,off_t,count)
static inline _syscall3(pid_t,waitpid,pid_t,pid,int *,wait_stat,int,options)
static inline _syscall3(int,write,int,fd,const char *,buf,off_t,count)
static inline _syscall1(int,unlink,const char *,file)
#define exit _exit

#undef Z0
#undef Z1


/*
vi:ts=4:et:nowrap
*/

