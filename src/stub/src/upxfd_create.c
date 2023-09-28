/* upxfd_create.c -- workaround for 32-bit Android

   This file is part of the UPX executable compressor.

   Copyright (C) 2023 John F. Reiser
   All Rights Reserved.
 */

// Like memfd_create, but early 32-bit Android lacks.
// Also 32-bit Android has inconsistent __NR_ftruncate,
// so use direct write().
extern void *alloca(unsigned long size);
#include "include/linux.h"  // syscalls; i386 inlines via "int 0x80"

// 1. Try memfd_create
// 2. If Android or emulator, try /data/data/$APP_NAME/cache/upxAAA
//    where APP_NAME is discovered as basename($(< /proc/self/cmdline))
//  If Android, then write zeroes to desired length, then lseek back to offset 0.
//  (Except make_hatch() just writes the actual data, and closes the file.)
//  If not Android then ftruncate() to desired length.
//  Return (mapped_addr | (1+ fd))
//
extern int upxfd_android(int len);

void *upxfd_create(void *addr, unsigned len)
{
    char str_upx[] = "upx";
    int fd = memfd_create(str_upx, 0);
#if defined(__arm__) || defined(__i386__) //{ workaround: android & emulators
    if (fd < 0) {
        fd = upxfd_android(len);  // also emulates ftruncate(fd, len)
    }
#else  //}{
    if (0 > ftruncate(fd, len)) {
        return (void *)-1ul;
    }
#endif  //}
    addr = mmap(addr, len, PROT_READ|PROT_WRITE,
        (addr ? MAP_FIXED : 0)|MAP_SHARED, fd, 0);
    if (PAGE_MASK <= (unsigned long)addr) {
        return addr;
    }
    return (void *)((long)addr + (1+ (unsigned)fd));
}

#if 0  //{ test
int main(int argc, char *argv[])
{
    upxfd_create(0, 100);
    return 0;
}
#endif  //}
