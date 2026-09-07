//  arm64-linux.elf-so_fold.S -- linkage to C code to process Elf shared library
//
//  This file is part of the UPX executable compressor.
//
//  Copyright (C) John F. Reiser
//  All Rights Reserved.
//
//  UPX and the UCL library are free software; you can redistribute them
//  and/or modify them under the terms of the GNU General Public License as
//  published by the Free Software Foundation; either version 2 of
//  the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; see the file COPYING.
//  If not, write to the Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//  Markus F.X.J. Oberhumer              Laszlo Molnar
//  <markus@oberhumer.com>               <ezerotven+github@gmail.com>
//
//  John F. Reiser
//  <jreiser@users.sourceforge.net>
//

#include "arch/arm64/v8/macros.S"
#include "../../MAX_ELF_HDR.h"
NBPW= 8
#define bkpt brk #0

MAP_ANONYMOUS= 0x20
MAP_PRIVATE=   0x02
MAP_FIXED=     0x10

#ifndef DEBUG  /*{*/
#define DEBUG 0
#endif  /*}*/

#define call bl

  section SO_HEAD
ZERO:
PAGE_MASK: .quad 0xfffffffffffff000  // default
upxfn_path:.quad 0  // displacement from "zero"

fold_begin: .globl fold
        b fold

get_page_mask: .globl get_page_mask
        ldr x0,PAGE_MASK
        ret

Punmap: .globl Punmap
        ldr x8,PAGE_MASK
        bic x8,x0,x8
        sub x0,x0,x8
        add x1,x1,x8
munmap: .globl munmap
        do_sys __NR_munmap; ret

// Sometimes Linux enforces page-aligned address
Pprotect: .globl Pprotect
mprotect: .globl mprotect
        ldr x8,PAGE_MASK
        bic x8,x0,x8
        sub x0,x0,x8
        add x1,x1,x8
        do_sys __NR_mprotect; ret

mmap_privanon: .globl mmap_privanon
        mov w4,#MAP_PRIVATE|MAP_ANONYMOUS
        orr w3,w3,w4  // combine with input (such as MAP_FIXED)
        mov w4,#-1  // fd= -1
        mov x5,#0  // offset= 0
        // FALL THROUGH to mmap

Pmap: .globl Pmap
        ldr x8,PAGE_MASK
        bic x8,x0,x8
        sub x0,x0,x8
        add x1,x1,x8
mmap: .globl mmap
        do_sys __NR_mmap; ret

Psync: .globl Psync
        ldr x8,PAGE_MASK
        bic x8,x0,x8
        sub x0,x0,x8
        add x1,x1,x8
// Sync contents of data cache into RAM.
// Linux should do this implicitly, but apparently not.
CTR_IDC_SHIFT= 28
CTR_DIC_SHIFT= 29

        mov x3,x0  // lo
        add x4,x0,x1  // hi
sync_cache_range:  // (void *lo= x3, void *const hi= x4)
    mrs x6,ctr_el0
    tbnz w6,#CTR_IDC_SHIFT,dc_not_dirty
    ubfx x5,x6,#16,#4  // -2+ log2(dline_size)
    mov x8,#-4; lsl x8,x8,x5  // sz_dline mask
    and x5,x8,x3  // round down to dc line
//    cmp x5,x4; b.hs dc_done
dc_loop:
    dc cvau,x5  // sync dline
    sub x5,x5,x8  // next dline
    cmp x5,x4; b.lo dc_loop
//dc_done:
dc_not_dirty:
    dsb ish  // why here if dc not dirty?

    tbnz w6,#CTR_DIC_SHIFT,ic_not_dirty
    and x6,x6,#0xf  // -2+ log2(iline_size)
    mov x8,#-4; lsl x8,x8,x6  // sz_iline mask
    and x3,x8,x3  // round down to ic line
//    cmp x3,x4; b.hs ic_done
ic_loop:
    ic ivau,x3  // sync iline
    sub x3,x3,x8  // next iline
    cmp x3,x4; b.lo ic_loop
//ic_done:
    dsb ish
ic_not_dirty:
    isb
// fall into msync
        do_sys __NR_msync; ret

fold:  // enter here (x0= &so_info; x1= &{argc,argv,envp,lr}
//%esp:
//  MATCH_04  ptr unfolded_code
//  MATCH_10  len unfolded_code
//  MATCH_00  argc,argv,envp,lr(_start)
        sub sp,sp,#MAX_ELF_HDR_64; mov x2,sp  // space for Elf64_Ehdr and Elf64_Phdrs
        call upx_so_main  // (&so_info, &argc, elf_tmp); returns &escape_hatch
        add sp,sp,#MAX_ELF_HDR_64
        mov x2,x0  // save &escape_hatch
        POP2(x0,x1)  // F_ADRU, F_LENU  (unfolded region)
        mov w8,#__NR_munmap
        br x2  // svc #0; POP4(x0,x1,x2,lr}; ret

get4:  .globl get4  // un-aligned fetch (little endian)
    mov x1,x0
    ldrb w0,[x1],#1
    ldrb w2,[x1],#1; orr w0,w0,w2,lsl #8
    ldrb w2,[x1],#1; orr w0,w0,w2,lsl #16
    ldrb w2,[x1],#1; orr w0,w0,w2,lsl #24
    ret

  section ptr_NEXT
        call 0f; 0:
        mov x0,lr

// De-compressor sections inserted here:
// section EXP_HEAD
// section NRV_HEAD
// section NRV2B
// section NRV2D
// section NRV2E
// section NRV_TAIL
// section LZMA_*
// section ZSTD  future
// section EXP_TAIL

  section SO_TAIL

        .globl my_bkpt
my_bkpt:
        bkpt  // my_bkpt
        ret

// /usr/include/asm-generic/unistd.h
__NR_SYSCALL_BASE= 0

__NR_exit     = 0x5d + __NR_SYSCALL_BASE  // 93
__NR_read     = 0x3f + __NR_SYSCALL_BASE  // 63
__NR_write    = 0x40 + __NR_SYSCALL_BASE  // 64
__NR_openat   = 0x38 + __NR_SYSCALL_BASE  // 56
__NR_close    = 0x39 + __NR_SYSCALL_BASE  // 57
__NR_unlinkat = 0x23 + __NR_SYSCALL_BASE  // 35
__NR_getpid   = 0xad + __NR_SYSCALL_BASE  // 172
__NR_brk      = 0xd6 + __NR_SYSCALL_BASE  // 214
__NR_readlink = 0x4e + __NR_SYSCALL_BASE  // 78
__NR_unlinkat = 0x23 + __NR_SYSCALL_BASE  // 35


__NR_mmap     = 0xde + __NR_SYSCALL_BASE  // 222
__NR_mprotect = 0xe2 + __NR_SYSCALL_BASE  // 226
__NR_msync    = 0xe3 + __NR_SYSCALL_BASE  // 227
__NR_munmap   = 0xd7 + __NR_SYSCALL_BASE  // 215
__NR_memfd_create = 0x117 + __NR_SYSCALL_BASE  // 279
__NR_ftruncate= 0x2e + __NR_SYSCALL_BASE  // 46

        .globl exit
exit:
        do_sys __NR_exit

ftruncate:    .globl ftruncate;    do_sys __NR_ftruncate;    ret
memfd_create: .globl memfd_create; do_sys __NR_memfd_create; ret

        .globl read
read:
        do_sys __NR_read; ret

Pwrite: .globl Pwrite
        .globl write
write:
        do_sys __NR_write; ret

        .globl open
FD_CWD= -100
open:
        mov w3,w2
        mov w2,w1
        mov x1,x0
        mov w0,#FD_CWD
          // fall into 'openat'
        .globl openat
openat:
        do_sys __NR_openat; ret

        .globl close
close:
        do_sys __NR_close; ret

        .globl unlinkat
unlinkat:
        do_sys __NR_unlinkat; ret

        .globl getpid
getpid:
        do_sys __NR_getpid; ret

        .globl brk
brk:
        do_sys __NR_brk; ret

        .globl readlink
readlink:
        do_sys __NR_readlink; ret

get_sys_munmap: .globl get_sys_munmap  // r0= system call instruction
#if defined(ARMEL_DARWIN)  /*{*/
        ldr w0,4*1 + munmap
#elif defined(ARMEL_EABI4)  /*}{*/
        ldr w0,4*2 + munmap
#elif defined(ARM_OLDABI)  /*}{*/
        ldr w0,4*0 + munmap
#else  /*}{*/
        mov w0,#0
#endif  /*}*/
        ret

my_memcpy: .globl my_memcpy
memcpy: .globl memcpy  // void *memcpy(void *dst, void const *src, size_t len)
        mov x4,x0  // original dst
        cbz x2,9f
0:
        ldrb w3,[x1],#1
        strb w3,[x0],#1
        subs x2,x2,#1
        bgt 0b
9:
        mov x0,x4  // return value
        ret

memset: .globl memset  // void *memset(void *dst, int c, size_t len);
        mov x3,x0  // original dst
        cbz x2,9f
0:
        strb w1,[x0],#1
        subs x2,x2,#1
        bgt 0b
9:
        mov x0,x3  // return value
        ret

/* vim:set ts=8 sw=8 et: */
