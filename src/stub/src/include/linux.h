/* linux.h -- common stuff for the Linux stub loaders

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

// <sys/types.h>
typedef long off_t;
//typedef int64_t off64_t;
//typedef size_t caddr_t;
typedef void* caddr_t;
typedef unsigned pid_t;

struct rusage;
struct timex;
struct timeval {
    unsigned tv_sec;
    unsigned tv_usec;
};
struct timespec {
    unsigned tv_sec;
    long tv_nsec;
};


// misc constants

#if defined(__amd64__) || defined(__powerpc64__)
#define PAGE_MASK       (~0ul<<12)   // discards the offset, keeps the page
#define PAGE_SIZE       ( 1ul<<12)
#elif defined(__i386__) || defined(__powerpc__) || defined(__arm__)
#define PAGE_MASK       (~0ul<<12)   // discards the offset, keeps the page
#define PAGE_SIZE       ( 1ul<<12)
#elif defined(__mips__)
#define PAGE_MASK       (~0ul<<16)   // discards the offset, keeps the page
#define PAGE_SIZE       ( 1ul<<16)
#endif

#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2

#define O_RDONLY        00
#define O_WRONLY        01
#define O_RDWR          02
#define O_CREAT         0100
#define O_EXCL          0200

#define R_OK            4
#define W_OK            2
#define X_OK            1
#define F_OK            0

#define F_GETFD         1
#define F_SETFD         2
#define FD_CLOEXEC      1

// <errno.h>
#define ENOENT          2
#define EINTR           4

// <sys/mman.h>
#define PROT_READ       0x1
#define PROT_WRITE      0x2
#define PROT_EXEC       0x4
#define PROT_NONE       0x0

#define MAP_SHARED      0x01
#define MAP_PRIVATE     0x02
#define MAP_FIXED       0x10
#define MAP_ANONYMOUS   0x20
#define MAP_DENYWRITE 0x0800  /* ETXTBSY */


/*************************************************************************
// i386 syscalls
//
// Because of different <asm/unistd.h> versions and subtle bugs
// in both gcc and egcs we define all syscalls manually.
//
// Also, errno conversion is not necessary in our case, and we
// use optimized assembly statements to further decrease the size.
**************************************************************************/

#if defined(__i386__)  /*{*/

// <asm/unistd.h>
#define __NR_exit                 1
#define __NR_fork                 2
#define __NR_read                 3
#define __NR_write                4
#define __NR_open                 5
#define __NR_close                6
#define __NR_waitpid              7
#define __NR_link                 9
#define __NR_unlink              10
#define __NR_execve              11
#define __NR_lseek               19
#define __NR_getpid              20
#define __NR_access              33
#define __NR_brk                 45
#define __NR_fcntl               55
#define __NR_gettimeofday        78
#define __NR_mmap                90
#define __NR_munmap              91
#define __NR_ftruncate           93
#define __NR_adjtimex           124
#define __NR_mprotect           125
#define __NR_nanosleep          162

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
        __asm__ __volatile__ ("push %[sysN]; popl %0; int $0x80" \
            : "=a" (__res) \
            : [sysN] "g" (__NR_##name)); \
    } else { \
        __asm__ __volatile__ ("int $0x80" \
            : "=a" (__res) \
            : [sysN] "a" (__NR_##name)); \
    } \
    return (type) __res; \
}

#define _syscall1(type,name,type1,arg1) \
type name(type1 arg1) \
{ \
    long __res, junk; \
    if (Z1(__NR_##name)) { \
        if (Z0(arg1)) { \
            __asm__ __volatile__ ("xorl %%ebx,%%ebx; push %[sysN]; popl %0; int $0x80" \
                : "=a" (__res) \
                : [sysN] "g" (__NR_##name) \
                : "ebx"); \
        } else if (Z1(arg1)) { \
            __asm__ __volatile__ ("push %[a1]; popl %%ebx; push %[sysN]; popl %0; int $0x80" \
                : "=a" (__res) \
                : [sysN] "g" (__NR_##name), [a1] "g" ((long)(arg1)) \
                : "ebx"); \
        } else { \
            __asm__ __volatile__ ("push %[sysN]; popl %0; int $0x80" \
                : "=a" (__res), "=b" (junk) \
                : [sysN] "g" (__NR_##name),"b" ((long)(arg1))); \
        } \
    } else { \
        __asm__ __volatile__ ("int $0x80" \
            : "=a" (__res), "=b" (junk) \
            : "a" (__NR_##name),"b" ((long)(arg1))); \
    } \
    return (type) __res; \
}

#define _syscall1noreturn(name,type1,arg1) \
void __attribute__((__noreturn__,__nothrow__)) name(type1 arg1) \
{ \
    if (Z1(__NR_##name)) { \
        if (Z0(arg1)) { \
            __asm__ __volatile__ ("xorl %%ebx,%%ebx; push %0; popl %%eax; int $0x80" \
              : : "g" (__NR_##name) ); \
        } else if (Z1(arg1)) { \
            __asm__ __volatile__ ("push %[a1]; popl %%ebx; push %[sysN]; popl %%eax; int $0x80" \
              : : [sysN] "g" (__NR_##name), [a1] "g" ((long)(arg1)) ); \
        } else { \
            __asm__ __volatile__ ("push %0; popl %%eax; int $0x80" \
              : : "g" (__NR_##name),"b" ((long)(arg1))); \
        } \
    } else { \
        __asm__ __volatile__ ("int $0x80" \
            : : "a" (__NR_##name),"b" ((long)(arg1))); \
    } \
    for(;;) ; \
}

#define _syscall2(type,name,type1,arg1,type2,arg2) \
type name(type1 arg1,type2 arg2) \
{ \
    long __res, junkb, junkc; \
    if (Z1(__NR_##name)) { \
        if (Z0(arg1) && Z0(arg2)) { \
            __asm__ __volatile__ ("xorl %%ecx,%%ecx; xorl %%ebx,%%ebx; push %[sysN]; popl %0; int $0x80" \
                : "=a" (__res) \
                : [sysN] "g" (__NR_##name) \
                : "ebx", "ecx"); \
        } else if (Z0(arg1) && Z1(arg2)) { \
            __asm__ __volatile__ ("push %[a2]; popl %%ecx; xorl %%ebx,%%ebx; push %[sysN]; popl %0; int $0x80" \
                : "=a" (__res) \
                : [sysN] "g" (__NR_##name), [a2] "g" ((long)(arg2)) \
                : "ebx", "ecx"); \
        } else if (Z1(arg1) && Z0(arg2)) { \
            __asm__ __volatile__ ("xorl %%ecx,%%ecx; push %[a1]; popl %%ebx; push %[sysN]; popl %0; int $0x80" \
                : "=a" (__res) \
                : [sysN] "g" (__NR_##name), [a1] "g" ((long)(arg1)) \
                : "ebx", "ecx"); \
        } else if (Z1(arg1) && Z1(arg2)) { \
            __asm__ __volatile__ ("push %[a2]; popl %%ecx; push %[a1]; popl %%ebx; push %[sysN]; popl %0; int $0x80" \
                : "=a" (__res) \
                : [sysN] "g" (__NR_##name), [a1] "g" ((long)(arg1)), [a2] "g" ((long)(arg2)) \
                : "ebx", "ecx"); \
        } else if (Z0(arg1)) { \
            __asm__ __volatile__ ("xorl %%ebx,%%ebx; push %[sysN]; popl %0; int $0x80" \
                : "=a" (__res), "=c" (junkc) \
                : [sysN] "g" (__NR_##name),"c" ((long)(arg2)) \
                : "ebx"); \
        } else if (Z0(arg2)) { \
            __asm__ __volatile__ ("push %[sysN]; popl %0; xorl %%ecx,%%ecx; int $0x80" \
                : "=a" (__res), "=b" (junkb) \
                : [sysN] "g" (__NR_##name),"b" ((long)(arg1)) \
                : "ecx"); \
        } else if (Z1(arg1)) { \
            __asm__ __volatile__ ("push %[sysN]; popl %0; push %[a1]; popl %%ebx; int $0x80" \
                : "=a" (__res), "=c" (junkc) \
                : [sysN] "g" (__NR_##name), [a1] "g" ((long)(arg1)),"c" ((long)(arg2)) \
                : "ebx"); \
        } else if (Z1(arg2)) { \
            __asm__ __volatile__ ("push %[sysN]; popl %0; push %[a2]; popl %%ecx; int $0x80" \
                : "=a" (__res), "=b" (junkb) \
                : [sysN] "g" (__NR_##name),"b" ((long)(arg1)), [a2] "g" ((long)(arg2)) \
                : "ecx"); \
        } else { \
            __asm__ __volatile__ ("push %[sysN]; popl %0; int $0x80" \
                : "=a" (__res), "=b" (junkb), "=c" (junkc) \
                : [sysN] "g" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2))); \
        } \
    } else { \
        __asm__ __volatile__ ("int $0x80" \
            : "=a" (__res), "=b" (junkb), "=c" (junkc) \
            : [sysN] "a" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2))); \
    } \
    return (type) __res; \
}

#define _syscall3(type,name,type1,arg1,type2,arg2,type3,arg3) \
type name(type1 arg1,type2 arg2,type3 arg3) \
{ \
    long __res, junkb, junkc, junkd; \
    if (Z1(__NR_##name)) { \
        __asm__ __volatile__ ("push %[sysN]; popl %0; int $0x80" \
            : "=a" (__res), "=b" (junkb), "=c" (junkc), "=d" (junkd) \
            : [sysN] "g" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)), \
                                "d" ((long)(arg3))); \
    } else { \
        __asm__ __volatile__ ("int $0x80" \
            : "=a" (__res), "=b" (junkb), "=c" (junkc), "=d" (junkd) \
            : [sysN] "a" (__NR_##name),"b" ((long)(arg1)),"c" ((long)(arg2)), \
                                "d" ((long)(arg3))); \
    } \
    return (type) __res; \
}


#define _exit           syscall_exit
#define exit            syscall_exit

static inline _syscall2(int,access,const char *,file,int,mode)
static inline _syscall1(int,adjtimex,struct timex *,ntx)
static inline _syscall1(void *,brk,void *,high)
static inline _syscall1(int,close,int,fd)
static inline _syscall3(int,execve,const char *,file,char **,argv,char **,envp)
static inline _syscall1noreturn(_exit,int,exitcode)
static inline _syscall3(int,fcntl,int,fd,int,cmd,long,arg)
static inline _syscall0(pid_t,fork)
static inline _syscall2(int,ftruncate,int,fd,size_t,len)
static inline _syscall0(pid_t,getpid)
static inline _syscall2(int,gettimeofday,struct timeval *,tv,void *,tz)
static inline _syscall3(off_t,lseek,int,fd,off_t,offset,int,whence)
static inline _syscall3(int,mprotect,void *,addr,size_t,len,int,prot)
static inline _syscall2(int,munmap,void *,start,size_t,length)
static inline _syscall2(int,nanosleep,const struct timespec *,rqtp,struct timespec *,rmtp)
static inline _syscall3(int,open,const char *,file,int,flag,int,mode)
static inline _syscall3(ssize_t,read,int,fd,void *,buf,size_t,count)
static inline _syscall3(pid_t,waitpid,pid_t,pid,int *,wait_stat,int,options)
static inline _syscall3(ssize_t,write,int,fd,const void *,buf,size_t,count)
static inline _syscall1(int,unlink,const char *,file)
static inline _syscall2(int,link,const char *,src, const char *,dst)

#undef Z0
#undef Z1

#elif defined(__mips__)  /*}{*/

#undef  MAP_ANONYMOUS
#define MAP_ANONYMOUS 0x800

/*  Cannot write an inline assembler constraint for a specific register.
        http://gcc.gnu.org/ml/gcc-help/2007-02/msg00068.html
    Workaround:
        const register int num asm("v0") = 4;
        const register char *const str asm("a0") = string;
        asm volatile("syscall" : : "r"(num), "r"(str));
*/
static void *mmap(
    void *addr, size_t len,
    int prot, int flags,
    int fd, off_t offset
)
{
#define __NR_mmap (90+ 4000)
    register void * const a0 asm("a0") = addr;
    register size_t const a1 asm("a1") = len;
    register int    const a2 asm("a2") = prot;
    register int          a3 asm("a3") = flags;
    register int    const t0 asm("t0") = fd;
    register off_t  const t1 asm("t1") = offset;
    register int          v0 asm("v0") = __NR_mmap;
    __asm__ __volatile__(
      /*"break\n"*/  /* debug only */
        "addiu $29,$29,-0x20\n"
        "\tsw $8,0x10($29)\n"
        "\tsw $9,0x14($29)\n"
        "\tsyscall\n"
        "\taddiu $29,$29, 0x20\n"
        "\tb sysret\n"
    "sysgo:"
      /*"break\n"*/  /* debug only */
        "\tsyscall\n"
    "sysret:"
        "\tbnez $7,sysbad\n"  /* $7 === a3 */
        "\tjr $31\n"
    "sysbad:"
        "\tli $2,-1\n"  /* $2 === v0; overwrite 'errno' */
        "\tjr $31"
    : "+r"(v0), "+r"(a3)  /* "+r" ==> both read and write */
    :  "r"(a0), "r"(a1), "r"(a2),      "r"(t0), "r"(t1)
    );
    return (void *)v0;
}
static ssize_t read(int fd, void *buf, size_t len)
{
#define __NR_read (3+ 4000)
    register int    const a0 asm("a0") = fd;
    register void * const a1 asm("a1") = buf;
    register size_t const a2 asm("a2") = len;
    register size_t       v0 asm("v0") = __NR_read;
    __asm__ __volatile__(
        "bal sysgo"
    : "+r"(v0)
    : "r"(a0), "r"(a1), "r"(a2)
    : "a3"
    );
    return v0;
}

static void *brk(void *addr)
{
#define __NR_brk (45+ 4000)
    register void *const a0 asm("a0") = addr;
    register void *      v0 asm("v0") = (void *)__NR_brk;
    __asm__ __volatile__(
        "bal sysgo"
    : "+r"(v0)
    : "r"(a0)
    : "a3"
    );
    return v0;
}

static int close(int fd)
{
#define __NR_close (6+ 4000)
    register int const a0 asm("a0") = fd;
    register int       v0 asm("v0") = __NR_close;
    __asm__ __volatile__(
        "bal sysgo"
    : "+r"(v0)
    : "r"(a0)
    : "a3"
    );
    return v0;
}

static void exit(int code) __attribute__ ((__noreturn__));
static void exit(int code)
{
#define __NR_exit (1+ 4000)
    register int const a0 asm("a0") = code;
    register int       v0 asm("v0") = __NR_exit;
    __asm__ __volatile__(
        "bal sysgo"
    :
    : "r"(v0), "r"(a0)
    : "a3"
    );
    for (;;) {}
}

#if 0  /*{ UNUSED */
static int munmap(void *addr, size_t len)
{
#define __NR_munmap (91+ 4000)
    register  void *const a0 asm("a0") = addr;
    register size_t const a1 asm("a2") = len;
    register size_t       v0 asm("v0");
    __asm__ __volatile__(
        "bal sysgo"
    :      "=r"(v0)
    :  [v0] "r"(__NR_munmap), "r"(a0), "r"(a1)
    : "a3"
    );
    return v0;
}
#endif  /*}*/

static int mprotect(void const *addr, size_t len, int prot)
{
#define __NR_mprotect (125+ 4000)
    register void const *const a0 asm("a0") = addr;
    register size_t      const a1 asm("a1") = len;
    register int         const a2 asm("a2") = prot;
    register size_t            v0 asm("v0") = __NR_mprotect;
    __asm__ __volatile__(
        "bal sysgo"
    : "+r"(v0)
    : "r"(a0), "r"(a1), "r"(a2)
    : "a3"
    );
    return v0;
}

static ssize_t open(char const *path, int kind, int mode)
{
#define __NR_open (5+ 4000)
    register char const *const a0 asm("a0") = path;
    register int         const a1 asm("a1") = kind;
    register int         const a2 asm("a2") = mode;
    register size_t            v0 asm("v0") = __NR_open;
    __asm__ __volatile__(
        "bal sysgo"
    : "+r"(v0)
    : "r"(a0), "r"(a1), "r"(a2)
    : "a3"
    );
    return v0;
}

#if DEBUG  /*{*/
static ssize_t write(int fd, void const *buf, size_t len)
{
#define __NR_write (4+ 4000)
    register int          const a0 asm("a0") = fd;
    register void const * const a1 asm("a1") = buf;
    register size_t       const a2 asm("a2") = len;
    register size_t             v0 asm("v0") = __NR_write;
    __asm__ __volatile__(
        "b sysgo"
    : "+r"(v0)
    : "r"(a0), "r"(a1), "r"(a2)
    : "a3"
    );
    return v0;
}
#endif  /*}*/

#else  /*}{ generic */

void *brk(void *);
int close(int);
void exit(int) __attribute__((__noreturn__,__nothrow__));
void *mmap(void *, size_t, int, int, int, off_t);
int munmap(void *, size_t);
int mprotect(void const *, size_t, int);
int open(char const *, unsigned, unsigned);
ssize_t read(int, void *, size_t);
ssize_t write(int, void const *, size_t);

#endif  /*}*/
void __clear_cache(void *, void *);


/*************************************************************************
// <elf.h>
**************************************************************************/

typedef uint16_t Elf32_Half;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf32_Word;
typedef int32_t  Elf32_Sword;
typedef uint32_t Elf64_Word;
typedef int32_t  Elf64_Sword;
typedef uint64_t Elf32_Xword;
typedef int64_t  Elf32_Sxword;
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;
typedef uint32_t Elf32_Addr;
typedef uint64_t Elf64_Addr;
typedef uint32_t Elf32_Off;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf32_Section;
typedef uint16_t Elf64_Section;
typedef Elf32_Half Elf32_Versym;
typedef Elf64_Half Elf64_Versym;


#define EI_NIDENT 16

typedef struct
{
  unsigned char e_ident[EI_NIDENT];
  Elf32_Half    e_type;
  Elf32_Half    e_machine;
  Elf32_Word    e_version;
  Elf32_Addr    e_entry;
  Elf32_Off     e_phoff;
  Elf32_Off     e_shoff;
  Elf32_Word    e_flags;
  Elf32_Half    e_ehsize;
  Elf32_Half    e_phentsize;
  Elf32_Half    e_phnum;
  Elf32_Half    e_shentsize;
  Elf32_Half    e_shnum;
  Elf32_Half    e_shstrndx;
} Elf32_Ehdr;

typedef struct
{
  unsigned char e_ident[EI_NIDENT];
  Elf64_Half    e_type;
  Elf64_Half    e_machine;
  Elf64_Word    e_version;
  Elf64_Addr    e_entry;
  Elf64_Off     e_phoff;
  Elf64_Off     e_shoff;
  Elf64_Word    e_flags;
  Elf64_Half    e_ehsize;
  Elf64_Half    e_phentsize;
  Elf64_Half    e_phnum;
  Elf64_Half    e_shentsize;
  Elf64_Half    e_shnum;
  Elf64_Half    e_shstrndx;
} Elf64_Ehdr;

typedef struct
{
  Elf32_Word    p_type;
  Elf32_Off     p_offset;
  Elf32_Addr    p_vaddr;
  Elf32_Addr    p_paddr;
  Elf32_Word    p_filesz;
  Elf32_Word    p_memsz;
  Elf32_Word    p_flags;
  Elf32_Word    p_align;
} Elf32_Phdr;

typedef struct
{
  Elf64_Word    p_type;
  Elf64_Word    p_flags;
  Elf64_Off     p_offset;
  Elf64_Addr    p_vaddr;
  Elf64_Addr    p_paddr;
  Elf64_Xword   p_filesz;
  Elf64_Xword   p_memsz;
  Elf64_Xword   p_align;
} Elf64_Phdr;

typedef struct
{
  uint32_t a_type;
  union {
      uint32_t a_val;
  } a_un;
} Elf32_auxv_t;

typedef struct
{
  uint64_t a_type;
  union
    {
      uint64_t a_val;
    } a_un;
} Elf64_auxv_t;

#define AT_NULL         0
#define AT_IGNORE       1
#define AT_PHDR         3
#define AT_PHENT        4
#define AT_PHNUM        5
#define AT_PAGESZ       6
#define AT_BASE         7
#define AT_ENTRY        9
#define AT_RANDOM       25 /* Address of 16 random bytes.  */

#define ET_EXEC         2
#define ET_DYN          3

#define PF_X            1
#define PF_W            2
#define PF_R            4

#define PT_LOAD         1
#define PT_INTERP       3
#define PT_PHDR         6


/*************************************************************************
// UPX stuff
**************************************************************************/

// !!! must be the same as in p_unix.h !!!
#define OVERHEAD        2048

#define UPX_MAGIC_LE32  0x21585055          // "UPX!"

#if 1
// patch constants for our loader (le32 format)
//#define UPX1            0x31585055          // "UPX1"
#define UPX2            0x32585055          // "UPX2"
#define UPX3            0x33585055          // "UPX4"
#define UPX4            0x34585055          // "UPX4"
//#define UPX5            0x35585055          // "UPX5"
#else
// transform into relocations when using ElfLinker
extern const unsigned UPX2;
extern const unsigned UPX3;
extern const unsigned UPX4;
#define UPX2    ((unsigned) (const void *) &UPX2)
#define UPX3    ((unsigned) (const void *) &UPX3)
#define UPX4    ((unsigned) (const void *) &UPX4)
#endif


typedef int nrv_int;
typedef int nrv_int32;
typedef unsigned int nrv_uint;
typedef unsigned int nrv_uint32;
#define nrv_byte unsigned char
#define nrv_bytep unsigned char *
#define nrv_voidp void *


// From ../p_unix.h
struct b_info {     // 12-byte header before each compressed block
    uint32_t sz_unc;            // uncompressed_size
    uint32_t sz_cpr;            // compressed_size
    unsigned char b_method;     // compression algorithm
    unsigned char b_ftid;       // filter id
    unsigned char b_cto8;       // filter parameter
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

