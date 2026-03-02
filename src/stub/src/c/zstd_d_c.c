/* zstd_d_c.c -- zstd decompressor stub (C implementation)

   This file is part of the UPX executable compressor.

   Copyright (C) Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   UPX and the UCL library are free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.
*/

#define ZSTD_STATIC_LINKING_ONLY
#define ZSTD_DISABLE_ASM        1
#define DYNAMIC_BMI2            0
#define HUF_FORCE_DECOMPRESS_X2 1
#define ZSTD_TRACE              0

/* Include stddef.h BEFORE our #defines to avoid conflicts */
#include <stddef.h>

/* GCC builtins that some architectures (riscv64) need for zstd */
#if defined(__riscv)
int __clzdi2(long x) {
    int n = 0;
    if (!(x & 0xFFFFFFFF00000000UL)) { n += 32; x <<= 32; }
    if (!(x & 0xFFFF000000000000UL)) { n += 16; x <<= 16; }
    if (!(x & 0xFF00000000000000UL)) { n +=  8; x <<=  8; }
    if (!(x & 0xF000000000000000UL)) { n +=  4; x <<=  4; }
    if (!(x & 0xC000000000000000UL)) { n +=  2; x <<=  2; }
    if (!(x & 0x8000000000000000UL)) { n +=  1; }
    return n;
}
int __ctzdi2(long x) {
    if (!x) return 64;
    int n = 0;
    if (!(x & 0xFFFFFFFF)) { n += 32; x >>= 32; }
    if (!(x & 0xFFFF))     { n += 16; x >>= 16; }
    if (!(x & 0xFF))       { n +=  8; x >>=  8; }
    if (!(x & 0xF))        { n +=  4; x >>=  4; }
    if (!(x & 0x3))        { n +=  2; x >>=  2; }
    if (!(x & 0x1))        { n +=  1; }
    return n;
}
unsigned __bswapsi2(unsigned x) {
    return ((x >> 24) & 0xFF) | ((x >> 8) & 0xFF00) |
           ((x << 8) & 0xFF0000) | ((x << 24) & 0xFF000000U);
}
unsigned long __bswapdi2(unsigned long x) {
    return ((unsigned long)__bswapsi2(x) << 32) | __bswapsi2(x >> 32);
}
#endif

/* Direct mmap/munmap syscalls - no GOT/PLT needed.
 * mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) */
#define UPX_PAGE_ROUND(s) (((s)+4095)&~(size_t)4095)

#if defined(__x86_64__)
static void *upx_mmap(size_t size) {
    void *p;
    register long r10 __asm__("r10") = 0x22;  /* MAP_PRIVATE|MAP_ANONYMOUS */
    register long r8  __asm__("r8")  = -1;
    register long r9  __asm__("r9")  = 0;
    __asm__ __volatile__ ("syscall" : "=a"(p)
        : "a"(9L), "D"(0L), "S"(UPX_PAGE_ROUND(size)), "d"(3L),
          "r"(r10), "r"(r8), "r"(r9) : "memory", "rcx", "r11");
    return ((long)p < 0) ? (void *)0 : p;
}
static void upx_munmap(void *p, size_t size) {
    __asm__ __volatile__ ("syscall" : : "a"(11L), "D"(p), "S"(UPX_PAGE_ROUND(size))
        : "memory", "rcx", "r11");
}

#elif defined(__aarch64__)
static void *upx_mmap(size_t size) {
    register long x0 __asm__("x0") = 0;
    register long x1 __asm__("x1") = UPX_PAGE_ROUND(size);
    register long x2 __asm__("x2") = 3;       /* PROT_READ|PROT_WRITE */
    register long x3 __asm__("x3") = 0x22;    /* MAP_PRIVATE|MAP_ANONYMOUS */
    register long x4 __asm__("x4") = -1;
    register long x5 __asm__("x5") = 0;
    register long x8 __asm__("x8") = 222;     /* __NR_mmap */
    __asm__ __volatile__ ("svc 0" : "+r"(x0)
        : "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5), "r"(x8) : "memory");
    return ((long)x0 < 0) ? (void *)0 : (void *)x0;
}
static void upx_munmap(void *p, size_t size) {
    register long x0 __asm__("x0") = (long)p;
    register long x1 __asm__("x1") = UPX_PAGE_ROUND(size);
    register long x8 __asm__("x8") = 215;     /* __NR_munmap */
    __asm__ __volatile__ ("svc 0" : : "r"(x0), "r"(x1), "r"(x8) : "memory");
}

#elif defined(__arm__)
static void *upx_mmap(size_t size) {
    register long r0 __asm__("r0") = 0;
    register long r1 __asm__("r1") = UPX_PAGE_ROUND(size);
    register long r2 __asm__("r2") = 3;
    register long r3 __asm__("r3") = 0x22;
    register long r4 __asm__("r4") = -1;
    register long r5 __asm__("r5") = 0;
    register long r7 __asm__("r7") = 192;     /* __NR_mmap2 */
    __asm__ __volatile__ ("svc 0" : "+r"(r0)
        : "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5), "r"(r7) : "memory");
    return ((long)r0 < 0) ? (void *)0 : (void *)r0;
}
static void upx_munmap(void *p, size_t size) {
    register long r0 __asm__("r0") = (long)p;
    register long r1 __asm__("r1") = UPX_PAGE_ROUND(size);
    register long r7 __asm__("r7") = 91;      /* __NR_munmap */
    __asm__ __volatile__ ("svc 0" : : "r"(r0), "r"(r1), "r"(r7) : "memory");
}

#elif defined(__i386__)
static void *upx_mmap(size_t size) {
    void *p;
    struct { long a,b,c,d,e,f; } args = {0, UPX_PAGE_ROUND(size), 3, 0x22, -1, 0};
    __asm__ __volatile__ ("int $0x80" : "=a"(p) : "a"(192), "b"(&args) : "memory");
    return ((long)p < 0) ? (void *)0 : p;
}
static void upx_munmap(void *p, size_t size) {
    __asm__ __volatile__ ("int $0x80" : : "a"(91), "b"(p), "c"(UPX_PAGE_ROUND(size)) : "memory");
}

#elif defined(__powerpc64__) || defined(__powerpc__)
static void *upx_mmap(size_t size) {
    register long r0 __asm__("r0") = 90;      /* __NR_mmap */
    register long r3 __asm__("r3") = 0;
    register long r4 __asm__("r4") = UPX_PAGE_ROUND(size);
    register long r5 __asm__("r5") = 3;
    register long r6 __asm__("r6") = 0x22;
    register long r7 __asm__("r7") = -1;
    register long r8 __asm__("r8") = 0;
    __asm__ __volatile__ ("sc" : "+r"(r3)
        : "r"(r0), "r"(r4), "r"(r5), "r"(r6), "r"(r7), "r"(r8) : "memory", "cr0");
    return ((long)r3 < 0) ? (void *)0 : (void *)r3;
}
static void upx_munmap(void *p, size_t size) {
    register long r0 __asm__("r0") = 91;
    register long r3 __asm__("r3") = (long)p;
    register long r4 __asm__("r4") = UPX_PAGE_ROUND(size);
    __asm__ __volatile__ ("sc" : : "r"(r0), "r"(r3), "r"(r4) : "memory", "cr0");
}

#elif defined(__riscv)
static void *upx_mmap(size_t size) {
    register long a0 __asm__("a0") = 0;
    register long a1 __asm__("a1") = UPX_PAGE_ROUND(size);
    register long a2 __asm__("a2") = 3;
    register long a3 __asm__("a3") = 0x22;
    register long a4 __asm__("a4") = -1;
    register long a5 __asm__("a5") = 0;
    register long a7 __asm__("a7") = 222;     /* __NR_mmap */
    __asm__ __volatile__ ("ecall" : "+r"(a0)
        : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a7) : "memory");
    return ((long)a0 < 0) ? (void *)0 : (void *)a0;
}
static void upx_munmap(void *p, size_t size) {
    register long a0 __asm__("a0") = (long)p;
    register long a1 __asm__("a1") = UPX_PAGE_ROUND(size);
    register long a7 __asm__("a7") = 215;     /* __NR_munmap */
    __asm__ __volatile__ ("ecall" : : "r"(a0), "r"(a1), "r"(a7) : "memory");
}

#elif defined(__mips__)
static void *upx_mmap(size_t size) {
    register long v0 __asm__("$2");
    register long a0 __asm__("$4") = 0;
    register long a1 __asm__("$5") = UPX_PAGE_ROUND(size);
    register long a2 __asm__("$6") = 3;
    register long a3 __asm__("$7") = 0x0822;  /* MAP_PRIVATE|MAP_ANONYMOUS (MIPS) */
    /* fd=-1 and offset=0 go on stack for mips o32 */
    __asm__ __volatile__ (
        "addiu $sp,$sp,-16\n\t"
        "li $8,-1\n\tsw $8,16($sp)\n\t"  /* fd = -1 */
        "sw $0,20($sp)\n\t"               /* offset = 0 */
        "li $2,4090\n\tsyscall\n\t"        /* __NR_mmap */
        "addiu $sp,$sp,16"
        : "=r"(v0), "+r"(a0) : "r"(a1), "r"(a2), "r"(a3) : "memory", "$8");
    return ((long)a0 < 0) ? (void *)0 : (void *)a0;
}
static void upx_munmap(void *p, size_t size) {
    register long a0 __asm__("$4") = (long)p;
    register long a1 __asm__("$5") = UPX_PAGE_ROUND(size);
    __asm__ __volatile__ ("li $2,4091\n\tsyscall" : : "r"(a0), "r"(a1) : "memory", "$2");
}

#else
#error "Unsupported architecture for upx_mmap"
#endif

/* Local implementations of libc functions.
 * Defined BEFORE the #defines that rename them. */
static void *upx_zstd_memcpy(void *d, const void *s, size_t n) {
    unsigned char *dd=(unsigned char*)d; const unsigned char *ss=(const unsigned char*)s;
    while(n--) *dd++=*ss++; return d;
}
static void *upx_zstd_memmove(void *d, const void *s, size_t n) {
    unsigned char *dd=(unsigned char*)d; const unsigned char *ss=(const unsigned char*)s;
    if(dd<ss||dd>=ss+n){while(n--)*dd++=*ss++;}else{dd+=n;ss+=n;while(n--)*--dd=*--ss;}
    return d;
}
static void *upx_zstd_memset(void *d, int c, size_t n) {
    unsigned char *dd=(unsigned char*)d; while(n--)*dd++=(unsigned char)c; return d;
}
static int upx_zstd_memcmp(const void *a, const void *b, size_t n) {
    const unsigned char *p=(const unsigned char*)a,*q=(const unsigned char*)b;
    while(n--){if(*p!=*q)return *p-*q;p++;q++;} return 0;
}

/* Now rename libc symbols to our implementations.
 * The zstd source will use our local hidden functions instead of GOT-indirect calls. */
#define memmove upx_zstd_memmove
#define memcpy  upx_zstd_memcpy
#define memset  upx_zstd_memset
#define memcmp  upx_zstd_memcmp
/* malloc/calloc/free: should not be called when using ZSTD_initStaticDCtx.
 * Define as non-function-like macros to avoid conflicting with declarations. */
#define malloc  upx_zstd_malloc_unused
#define calloc  upx_zstd_calloc_unused
#define free    upx_zstd_free_unused
static void *upx_zstd_malloc_unused(size_t n) { (void)n; return (void*)0; }
static void *upx_zstd_calloc_unused(size_t n, size_t s) { (void)n; (void)s; return (void*)0; }
static void upx_zstd_free_unused(void *p) { (void)p; }

/* Override zstd's allocator macros */
#define ZSTD_malloc(s)    ((void*)0)
#define ZSTD_calloc(n,s)  ((void*)0)
#define ZSTD_free(p)      ((void)(p))
/* Override zstd_deps.h memory macros to use our local functions.
 * Define ZSTD_DEPS_COMMON to prevent zstd_deps.h from redefining them. */
#define ZSTD_DEPS_COMMON
#define ZSTD_memcpy(d,s,l)  upx_zstd_memcpy((d),(s),(l))
#define ZSTD_memmove(d,s,l) upx_zstd_memmove((d),(s),(l))
#define ZSTD_memset(d,c,l)  upx_zstd_memset((d),(c),(l))
#define ZSTD_memcmp(a,b,l)  upx_zstd_memcmp((a),(b),(l))

/* Prevent zstd_deps.h from including stdlib.h/string.h which would conflict */
#define ZSTD_DEPS_NEED_MALLOC 0
/* Prevent string.h from being included (we provide our own implementations) */
#define _STRING_H 1
#define _STRING_H_ 1
#define __STRING_H 1
#define _STRINGS_H 1

#include "zstd.h"
#include "common/zstd_common.c"
#include "common/xxhash.c"
#include "common/fse_decompress.c"
/* entropy_common.c provides FSE_readNCount, HUF_readStats_wksp, ERR_getErrorString.
 * Undefine the macro versions to avoid redefinition conflicts. */
#undef FSE_isError
#undef HUF_isError
#include "common/entropy_common.c"
#include "common/error_private.c"
#include "decompress/zstd_decompress.c"
#include "decompress/zstd_decompress_block.c"
#include "decompress/huf_decompress.c"
#include "decompress/zstd_ddict.c"

__attribute__((noinline, visibility("default")))
size_t upx_zstd_decompress(void *dst, size_t dst_len,
                            const void *src, size_t src_len)
{
    size_t ws_size = ZSTD_estimateDCtxSize();
    void *ws = upx_mmap(ws_size);
    if (!ws) return (size_t)-1;
    ZSTD_DCtx *dctx = ZSTD_initStaticDCtx(ws, ws_size);
    size_t result = (size_t)-1;
    if (dctx)
        result = ZSTD_decompressDCtx(dctx, dst, dst_len, src, src_len);
    upx_munmap(ws, ws_size);
    return result;
}
