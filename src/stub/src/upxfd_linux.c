/* upxfd_create.c -- simplify upx_mmap_and_fd_linux for non-Android

   This file is part of the UPX executable compressor.

   Copyright (C) John F. Reiser
   All Rights Reserved.
 */

extern void my_bkpt(void const *, ...);

#if defined(__i386__) //}{
#define ANDROID_FRIEND 1
#define addr_string(string) ({ \
    char const *str; \
    asm("call 0f; .asciz \"" string "\"; 0: pop %0" \
/*out*/ : "=r"(str) ); \
    str; \
})
#elif defined(__arm__) //}{
#define ANDROID_FRIEND 1
#define addr_string(string) ({ \
    char const *str; \
    asm("bl 0f; .string \"" string "\"; .balign 4; 0: mov %0,lr" \
/*out*/ : "=r"(str) \
/* in*/ : \
/*und*/ : "lr"); \
    str; \
})
#elif defined(__mips__) //}{
#define ANDROID_FRIEND 0
#define addr_string(string) ({ \
    char const *str; \
    asm(".set noreorder; bal 0f; move %0,$31; .set reorder; .asciz \"" string "\"; .balign 4\n0:\n" \
/*out*/ : "=r"(str) \
/* in*/ : \
/*und*/ : "ra"); \
    str; \
})
#elif defined(__powerpc__)  /*}{*/
#define ANDROID_FRIEND 0
#define addr_string(string) ({ \
    char const *str; \
    asm("bl 0f; .asciz \"" string "\"; .balign 4; 0: mflr %0" \
/*out*/ : "=r"(str) \
/* in*/ : \
/*und*/ : "lr"); \
    str; \
})
#elif defined(__powerpc64__) //}{
#define ANDROID_FRIEND 0
#define addr_string(string) ({ \
    char const *str; \
    asm("bl 0f; .string \"" string "\"; .balign 4; 0: mflr %0" \
/*out*/ : "=r"(str) \
/* in*/ : \
/*und*/ : "lr"); \
    str; \
})
#elif defined(__x86_64) //}{
#define ANDROID_FRIEND 0
#define addr_string(string) ({ \
    char const *str; \
    asm("lea 9f(%%rip),%0; .section STRCON; 9:.asciz \"" string "\"; .previous" \
/*out*/ : "=r"(str) ); \
    str; \
})
#elif defined(__aarch64__) //}{
#define ANDROID_FRIEND 0
#define addr_string(string) ({ \
    char const *str; \
    asm("bl 0f; .string \"" string "\"; .balign 4; 0: mov %0,x30" \
/*out*/ : "=r"(str) \
/* in*/ : \
/*und*/ : "x30"); \
    str; \
})
#elif defined(__riscv) //}{
#define ANDROID_FRIEND 0
#define addr_string(string) ({ \
    char const *str; \
    asm("jal %0,0f; .string \"" string "\"; .balign 4; 0:" \
/*out*/ : "=r"(str) \
/* in*/ : \
/*und*/ : ); \
    str; \
})
#else  //}{
#define ANDROID_FRIEND 0
#error  addr_string
#endif  //}

#include "include/linux.h"  // syscall decls; i386 inlines via "int 0x80"

#if !DEBUG //{
#define DPRINTF(fmt, args...) /*empty*/
#else  //}{
// DPRINTF is defined as an expression using "({ ... })"
// so that DPRINTF can be invoked inside an expression,
// and then followed by a comma to ignore the return value.
// The only complication is that percent and backslash
// must be doubled in the format string, because the format
// string is processed twice: once at compile-time by 'asm'
// to produce the assembled value, and once at runtime to use it.
#define DPRINTF(fmt, args...) ({ dprintf(addr_string(fmt), args); })
static int dprintf(char const *fmt, ...); // forward
#endif  //} DEBUG

#define MFD_EXEC 0x10
//#define O_RDWR 2

#if defined(__aarch64__)  //{
// linux/arch/arm64/include/uapi/asm/fcntl.h:
#define O_DIRECTORY  040000 /* must be a directory */

#elif defined(__arm__)  //}{
// linux/arch/arm/include/uapi/asm/fcntl.h:
#define O_DIRECTORY  040000 /* must be a directory */

#elif defined(__powerpc__) || defined(__powerpc64__)  //}{
// linux/arch/powerpc/include/uapi/asm/fcntl.h:
#define O_DIRECTORY      040000 /* must be a directory */

#else  //}{ i386, amd64, mips
// linux/include/uapi/asm-generic/fcntl.h:
#define O_DIRECTORY 00200000 /* must be a directory */

#endif  //}

// linux/include/uapi/asm-generic/fcntl.h:
#define __O_TMPFILE 020000000
#define O_TMPFILE (__O_TMPFILE | O_DIRECTORY)

#define EISDIR 21 /* directory mismatch */
#define EINVAL 22 /* asm-generic/errno-base.h */

extern int memfd_create(char const *, unsigned);
extern int ftruncate(int, size_t);
extern int open(char const *, int, int);
extern unsigned long get_page_mask(void);
extern ssize_t write(int, void const *, size_t);
void exit(int) __attribute__((__noreturn__,__nothrow__));

// Implementation for Linux-native, where memfd_create
// (or /dev/shm) works.  Saves space in contrast to
// upxfd_android (or Android emulator), which must
// fall back to  /data/data/$APP_NAME/cache/upxAAA ,
// and also must work around inconsistent __NR_ftruncate.
// 1. Try memfd_create
// 2. Try /dev/shm
unsigned long upx_mmap_and_fd_linux( // returns (mapped_addr | (1+ fd))
    void *ptr  // desired address
    , unsigned datlen  // mapped length
    , char *pathname  // 0 ==> get_upxfn_path()
)
{
    (void)pathname;  // FIXME NYI
    char const *const name = addr_string("upx");
    int fd = memfd_create(name, MFD_EXEC);
    if (-EINVAL == fd) { // 2024-10-15 MFD_EXEC unknown to ubuntu-20.04
        fd = memfd_create(name, 0);  // try again
    }
// FIXME?  try upxfd_create
    if (fd < 0) { // such as ENOSYS for memfd_create on Linux < 3.17 (2014-10-05)
        // last chance for Linux; using /tmp often hits 'noexec' etc.
        fd = open(addr_string("/dev/shm"), O_RDWR | O_DIRECTORY | O_TMPFILE, 0700);
        if (fd < 0) {
            if (-EISDIR == fd) { // no memfd_create often lacks O_TMPFILE, too
                write(2, addr_string(
                    "UPX-5.0 wants memfd_create(), or needs /dev/shm(,O_TMPFILE,)\\n"), 61);
                exit(127);
            }
            // Error from open() is unknown. Cause later SIGSEGV.
            return (unsigned long)(long)fd;  // -errno; high bits are all 1
        }
        // Beware: /dev/shm might limit write() to 8KiB at a time.
    }
    int rv = ftruncate(fd, datlen);
    if (rv < 0) {
        return (unsigned long)(long)rv;
    }
    ptr = mmap(ptr, datlen, PROT_READ|PROT_WRITE,
        (ptr ? MAP_FIXED : 0)|MAP_SHARED, fd, 0);
    unsigned long const page_mask = get_page_mask();
    if (page_mask <= (unsigned long)ptr) {
        return (unsigned long)ptr;  // -errno
    }
    DPRINTF("upx_mmap_and_fd_linux= %%p\n",
           (unsigned long)ptr + (1+ (unsigned)fd));
    return (unsigned long)ptr + (1+ (unsigned)fd);
}
