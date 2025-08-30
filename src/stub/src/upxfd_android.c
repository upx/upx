/* upxfd_android.c -- workaround memfd_create for 32-bit Android

   This file is part of the UPX executable compressor.

   Copyright (C) 2023 John F. Reiser
   All Rights Reserved.
 */

void my_bkpt(void const *, ...);

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
    asm(".set noreorder; bal 0f; nop; .asciz \"" string "\"; .balign 4\n0: move %0,$31; .set reorder" \
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
#else  //}{
#define ANDROID_FRIEND 0
#error  addr_string
#endif  //}

#if defined(__x86_64) || defined(__i386__) //}{
#define DPRINTF(fmt, args...) ({ \
    char const *r_fmt; \
    asm("call 0f; .asciz \"" fmt "\"; 0: pop %0" \
/*out*/ : "=r"(r_fmt) ); \
    dprintf(r_fmt, args); \
})
#else  //}{
#define DPRINTF(fmt, args...) /*empty*/
#endif  //}

#define dprintf my_bkpt

//#include <fcntl.h>
//#include <sys/stat.h>
typedef unsigned long long u64_t;
typedef unsigned int   u32_t;
typedef unsigned short u16_t;
// Observed by gdb for libc-bin 2.31-13+deb11u6 on Debian 5.10.158-2 (2022-12-13) armv7l
struct stat { // __NR_stat = 106 + NR_SYSCALL_BASE
    u32_t st_dev;
    u32_t st_ino;
    u16_t st_mode;
    u16_t st_nlink;
    u16_t st_uid;
    u16_t st_gid;
// 0x10
    u32_t st_rdev;
    u32_t st_size;
    u32_t st_blksize;
    u32_t st_blocks;
// 0x20
    u32_t st_atime;
    u32_t st_atime_nsec;
    u32_t st_mtime;
    u32_t st_mtime_nsec;
// 0x30
    u32_t st_ctime;
    u32_t st_ctime_nsec;
    u32_t pad1;
    u32_t pad2;
// 0x40
};
#define S_IFMT  00170000
#define S_IFDIR  0040000
#define S_IRWXU 00700
#define AT_FDCWD -100
#define restrict /**/

#include "include/linux.h"  // syscalls; i386 inlines via "int 0x80"
extern int open(char const *, int, int);

extern int fstatat(int dirfd, const char *restrict pathname,
    struct stat *restrict statbuf, int flags);

#define ENOENT 2   /* no such name */
#define EACCES 13  /* permission denied */
#define EINVAL 22  /* invalid argument */
#define ENOSPC 28  /* no space left on device */
#define ENOSYS 38  /* no such system call */

// ANDROID_TEST: Set to 1 for testing Android implementation using Linux on
// Raspberry Pi (arm32, perhaps running on actual arm64); else set to 0.
#define ANDROID_TEST 0

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


#define PATH_MAX 4096  /* linux/include/uapi/linux/limits.h */

extern int memfd_create(char const *name, unsigned flags);
extern int ftruncate(int fd, size_t length);
extern ssize_t write(int fd, void const *buf, size_t length);

// upx_mmap_and_fd_android() must be first in the .o when compiled,
// so prototype 'static' functions but put their definitions later.
#if ANDROID_FRIEND  //{
static int strncmplc(char const *s1, char const *s2, unsigned n);
static int create_upxfn_path(char *name, char *buf);
static unsigned sixbit(unsigned x);
static int dir_check(char const *path, int fatal);
#endif  //}

struct utsname {
    char sysname[65];
    char nodename[65];
    char release[65];
    char version[65];
    char machine[65];
    char domainname[65];
};
extern void *memset(void *dst, int val, size_t n);
extern int stat(char const *path, struct stat *statbuf);
extern int mkdir(char const *path, unsigned mode);
extern int uname(struct utsname *);
extern char * get_upxfn_path(void);
extern long get_page_mask(void);

unsigned long upx_mmap_and_fd_android( // returns (mapped_addr | (1+ fd))
    void *ptr  // desired address
    , unsigned datlen  // mapped length
    , char *pathname  // 0 ==> get_upxfn_path()
)
{
    unsigned long addr = 0;  // for result
    // Early 32-bit Android did not implement memfd_create
    int fd = -ENOSYS;
    if (!ANDROID_TEST) {
        char const *name = addr_string("upx");
        fd = memfd_create(name, MFD_EXEC);
        if (-EINVAL == fd) { // MFD_EXEC unknown to ubuntu-20.04
            fd = memfd_create(name, 0);  // try again
        }
        if (fd < 0) { // last chance for Linux
            fd = open(addr_string("/dev/shm"), O_RDWR | O_DIRECTORY | O_TMPFILE, 0700);
            if (fd < 0) {
                my_bkpt(addr_string("memfd_create"));
            }
        }
    }

#if ANDROID_FRIEND  //{
    // Varying __NR_ftruncate on Android can hurt even if memfd_create() succeeds.
    // On Linux, struct utsname has 6 arrays of size 65; but size can be larger.
#define BUFLEN PATH_MAX
    union {
        struct utsname uts;
        char buf[BUFLEN];
    } u;

    uname(&u.uts);
    int const not_android = (ANDROID_TEST ? 0
        : (    0 != strncmplc(addr_string("and"), &u.uts.sysname[0], 3)
            && 0 == strncmplc(addr_string("Lin"), &u.uts.sysname[0], 3)
            && '4'<  u.uts.release[0] ));
          // 2024-08-01: TermUX on Android 14 arm64 running 32-bit program:
          // claims it is Linux, but kernel is 4.19, and ftruncate() {__NR_ 93}
          // gets signal SIGSYS instead of errno ENOSYS; so is not really Linux!

    // Work-around for missing memfd_create syscall on early 32-bit Android.
    if (!not_android && !pathname) { // must ask
        pathname = get_upxfn_path();
        if (!pathname) { // persistence not desired, so use this->u.buf;
            pathname = &u.buf[0];
            pathname[0] = '\0';
        }
    }
    if (!not_android && -ENOSYS == fd && pathname) {
        if ('\0' == pathname[0]) { // first time; create the pathname and file
            int rv = create_upxfn_path(pathname, &u.buf[BUFLEN / 2]);
            if (rv < 0) {
                return rv;
            }
        }
        // Use the constructed path.
        fd = open(pathname, O_CREAT|O_EXCL|O_RDWR, S_IRWXU);
        if (fd < 0) {
            return fd;
        }
        unlink(pathname);
    }
#else  //}{ !ANDROID_FRIEND: simple!
    int not_android = 1;
    (void)pathname;  // dead: obviated by memfd_create()
#endif  //}

    // Set the file length
    if (ptr) {
        unsigned const page_mask = get_page_mask();
        unsigned const frag = ~page_mask & (unsigned)(long)ptr;
        ptr -= frag;  // becomes page-aligned
        datlen += frag;
    }
    if (datlen) {
        if (not_android) { // Linux ftruncate() is well-behaved
            int rv = ftruncate(fd, datlen);
            if (rv < 0) {
                return rv;
            }
        }
#if ANDROID_FRIEND  //{
        else { // !not_android: ftruncate has varying system call number on 32-bit
            lseek(fd, -1+ datlen, SEEK_SET);  // last byte
            char zero = 0;
            write(fd, &zero, 1);  // force allocation
            lseek(fd, 0, SEEK_SET);  // go back to the beginning
        }
#endif  //}
    }
    if (ptr) {
        unsigned const page_mask = get_page_mask();
        if (~page_mask & (unsigned)(long)ptr) {
            // Preserve entire page that contains *ptr
            write(fd, ptr, -page_mask);
        }
    }
    addr = (unsigned long)mmap(ptr, datlen , PROT_WRITE | PROT_READ,
        MAP_SHARED | (ptr ? MAP_FIXED : 0), fd, 0);
    if ((~0ul<<12) < addr) { // error
        return addr;
    }
    return addr | (1+ fd);
}

#if ANDROID_FRIEND  //{

__attribute__((__noinline__))
static int dir_check(char const *path, int fatal)
{
    struct stat sb;
    memset(&sb,0xff, sizeof(sb));  // DEBUG aid
    int rv = stat(path, &sb);
    if (0 <= rv) {
        if (S_IFDIR == (sb.st_mode & S_IFMT)) {
            return 0;
        }
    }
    if (-ENOENT == rv) {
        rv = mkdir(path, S_IRWXU);
    }
    if (rv < 0 && fatal) {
        my_bkpt(path, rv);  // required path not available
    }
    return rv;
}

//#define S_IRWXU 00700  /* rwx------ User Read-Write-eXecute */
extern void *alloca(unsigned size);
//#include <string.h>  // we use "typedef unsigned size_t;"
//#include <sys/utsname.h>

extern unsigned getpid(void);
extern void *mempcpy(void *dst, void const *src, unsigned len);

// Upper half of ASCII (chars 0100-0177) are all legal in a Linux filename.
// So the shortest code is " return 0100 + (077 & x); "
// But the 5 chars which follow 'Z' ("[\\]^_") or 'z' ("{|}~\x7F")
// look ugly, so substitute digits.
__attribute__((__noinline__))
static unsigned sixbit(unsigned x)
{
    unsigned y = 037 & x;  // "upper case"
    x &= 077;
    if (033 <= y) { // last 5 chars in each 32 are ugly
        if (040 & x)  // "lower case"
            x += '4' - 'z';  // "56789" follows 'z';
        else
            x += '/' - 'Z';  // "01234" follows 'Z';
    }
    x += 0100;  // upper half ASCII: "@ABC"...
    return x;
}

// Where to put temp file when memfd_create() fails on early 32-bit Android
__attribute__((__noinline__))
static int create_upxfn_path(char *name, char *buf)
{
    // Construct path "/data/data/$APP_NAME/cache/upxAAA".
    // Note 'mempcpy' [with 'p' in the middle!] returns the end-of-string.
    char *p =  mempcpy(&name[0], addr_string("/data/data/"), 11);  // '/' sentinel at end
    p[0] = '\0'; dir_check(name, 1);

    // Append the name of the app
    char const *q = addr_string("/proc/self/cmdline");
    int fd = open(q, O_RDONLY, 0);
    int rlen = read(fd, p= buf, -1+ PATH_MAX);
    close(fd);
    if (rlen < 0) {
        my_bkpt(q);
    }
    p[rlen] = '\0';  // insurance sentinel
    // Kernel-parsed arguments are separated by '\0'.
    while (*p) ++p;  // advance to end of argv[0]

    {
        char *app_end = p;
        // Sentinel '/' at name[10] provides safety for backing up.
        while ('/' != *p) --p;  // find last component of argv[0]
        q = p;
        p = mempcpy(&name[10], p, app_end - p);
        p[0] = '\0';
        if (-EACCES == dir_check(name, 0)) {
            p = mempcpy(&name[11], addr_string("com.termux/files"), 16);
            p = mempcpy(p, q, app_end - q);
            p[0] = '\0';
            dir_check(name, 1);
        }
    }

    p = mempcpy(p, addr_string("/cache"), 6);
    p[0] = '\0'; dir_check(name, 1);
    p = mempcpy(p, addr_string("/upx"), 4);
    pid_t pid = getpid();
    p[0] = sixbit(pid >> 0*6);
    p[1] = sixbit(pid >> 1*6);
    p[2] = sixbit(pid >> 2*6);
    p[3]='\0';
    return 0;  // success
}

// memfd_create() gets ENOSYS on early Android.  There are 32-bit x86 Android
// such as Zenfone 2 (discontinued 2018?), x86 Chromebooks (2019 and later),
// FydeOS, Windows subsystem for Android.  But the main use is for developing,
// to make Android emulator running on x86_64 (Linux or Windows) run faster
// by emulating x86 instead of ARM.
//
// Try /data/data/$APP_NAME/cache/upxAAA
// where APP_NAME is discovered as basename(argv[0])
// and argv[0] is guessed from /proc/self/cmdline.
// Also 32-bit Android has inconsistent __NR_ftruncate,
// so use direct write()
//
// To work around bug in "i386-linux-gcc-3.4.6 -m32 -march=i386" .
// gcc optimized out this code:
//      uname((struct utsname *)buf);
//      int const is_android = ( (('r'<<3*8)|('d'<<2*8)|('n'<<1*8)|('a'<<0*8))
//          == (0x20202020 | *(int *)buf) );
// Specialized: does NOT consider early termination of either string, etc.
__attribute__((__noinline__))
static int strncmplc(char const *s1, char const *s2, unsigned n)
{
    while (n--) {
        int rv = (0x20 | *s1++) - (0x20 | *s2++);
        if (rv) return rv;
    }
    return 0;
}

#endif  //}  ANDROID_FRIEND

#if 0  //{ test
char name[1000];

int main(int argc, char *argv[])
{
    upx_mmap_and_fd(0, 5000, 0);
}

void *mempcpy(void *adst, void const *asrc, unsigned len)
{
    char *dst = adst;
    char const *src = asrc;
    if (len) do {
        *dst++ = *src++;
    } while (--len);
    return dst;
}

void *memset(void *adst, unsigned val, unsigned len)
{
    char *dst = adst;
    if (len) do {
        *dst++ = val;
    } while (--len);
    return adst;
}
#endif  //}

