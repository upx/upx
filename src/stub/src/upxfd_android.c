/* upxfd_android.c -- workaround memfd_create for 32-bit Android

   This file is part of the UPX executable compressor.

   Copyright (C) 2023 John F. Reiser
   All Rights Reserved.
 */

#if defined(__i386__) //}{
#define addr_string(string) ({ \
    char const *str; \
    asm("call 0f; .asciz \"" string "\"; 0: pop %0" \
/*out*/ : "=r"(str) ); \
    str; \
})
#elif defined(__arm__) //}{
#define addr_string(string) ({ \
    char const *str; \
    asm("bl 0f; .string \"" string "\"; .balign 4; 0: mov %0,lr" \
/*out*/ : "=r"(str) \
/* in*/ : \
/*und*/ : "lr"); \
    str; \
})
#else  //}{
#   error;  // only: __arm__ or __i386__ (for ARM emulator)
#endif  //}

// Upper half of ASCII (chars 0100-0177) are all legal in a Linux filename.
// So the shortest code is " return 0100 + (077 & x); "
// But the 5 chars which follow 'Z' ("[\\]^_") or 'z' ("{|}~\x7F")
// look ugly, so substitute digits.
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

#define S_IRWXU 00700  /* rwx------ User Read-Write-eXecute */
extern void *alloca(unsigned size);
#include "include/linux.h"  // syscalls; i386 inlines via "int 0x80"
extern void *mempcpy(void *dst, void const *src, unsigned len);

// Early 32-bit Android lacks memfd_create.
// Try /data/data/$APP_NAME/cache/upxAAA
// where APP_NAME is discovered as basename(argv[0])
// and argv[0] is guessed from /proc/self/cmdline.
// Also 32-bit Android has inconsistent __NR_ftruncate,
// so use direct write()
//
#define BUFLEN 0x2000  /* 4KiB for PATH_MAX; 8KiB for zeroing */
#define ENOSPC 28  /* no space left on device */

int upxfd_android(unsigned len)  // returns fd with (.st_size = len); else -ENOSPC
{
    char *buf = alloca(BUFLEN);
    char *p =  mempcpy(&buf[0], addr_string("/data/data/"), 11);  // '/' sentinel at end
    int fd = open(addr_string("/proc/self/cmdline"), O_RDONLY, 0);
    int rlen = read(fd, p, BUFLEN);
    if (rlen < 0) {
        return -1;
    }
    while (*p) { // advance to end of argv[0]
        if (' '==*p) break;
        ++p;
    }
    {
        char *app_end = p;
        while ('/' != *--p) ;  // last component of argv[0]
        p = mempcpy(&buf[10], p, app_end - p);
    }
    p = mempcpy(p, addr_string("/cache/upx"), 10);
    pid_t pid = getpid();
    p[0] = sixbit(pid >> 0*6);
    p[1] = sixbit(pid >> 1*6);
    p[2] = sixbit(pid >> 2*6);
    p[3]='\0';

    fd = open(&buf[0], O_CREAT|O_EXCL|O_RDWR, S_IRWXU);
    if (fd < 0) {
        return -1;
    }
    unlink(&buf[0]);

    extern void *memset(void *dst, int c, unsigned);
    memset(buf, 0, BUFLEN);
    while (0 < len) {
        int x = (len < BUFLEN) ? len : BUFLEN;
        if (x != write(fd, buf, BUFLEN)) {
            return -ENOSPC;
        }
        len -= x;
    }
    lseek(fd, 0, SEEK_SET);
    return fd;
}
