@  arm-linux.elf-so_fold.S -- linkage to C code to process Elf shared library
@
@  This file is part of the UPX executable compressor.
@
@  Copyright (C) 2000-2023 John F. Reiser
@  All Rights Reserved.
@
@  UPX and the UCL library are free software; you can redistribute them
@  and/or modify them under the terms of the GNU General Public License as
@  published by the Free Software Foundation; either version 2 of
@  the License, or (at your option) any later version.
@
@  This program is distributed in the hope that it will be useful,
@  but WITHOUT ANY WARRANTY; without even the implied warranty of
@  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@  GNU General Public License for more details.
@
@  You should have received a copy of the GNU General Public License
@  along with this program; see the file COPYING.
@  If not, write to the Free Software Foundation, Inc.,
@  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
@
@  Markus F.X.J. Oberhumer              Laszlo Molnar
@  <markus@oberhumer.com>               <ezerotven+github@gmail.com>
@
@  John F. Reiser
@  <jreiser@users.sourceforge.net>
@

#define ARM_OLDABI 1
#include "arch/arm/v4a/macros.S"
#include "MAX_ELF_HDR.S"
#define bkpt .long 0xe7f001f0  /* reserved instr "udf #0x10"; Linux GNU eabi breakpoint */
NBPW= 4

sz_Elf32_Ehdr = 13*4
sz_Elf32_Phdr =  8*4
p_vaddr= 4+4
sz_l_info = 12
sz_p_info = 12
sz_b_info = 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

MAP_ANONYMOUS= 0x20
MAP_PRIVATE=   0x02
MAP_FIXED=     0x10

PROT_READ=     0x1
PROT_WRITE=    0x2
PROT_EXEC=     0x4

PF_X= (1 << 0)  /* Segment is executable */
PF_W= (1 << 1)  /* Segment is writable */
PF_R= (1 << 2)  /* Segment is readable */

O_RDONLY=       0

PAGE_SHIFT= 12
PAGE_SIZE = -(~0<<PAGE_SHIFT)
PATHMAX=  4096

PATH_MAX= 4096

#ifndef DEBUG  /*{*/
#define DEBUG 0
#define TRACE_REGS r0-r12,r14,r15
#endif  /*}*/

#define OVERHEAD 2048
#define call bl

mflg_data: .int MAP_PRIVATE|MAP_ANONYMOUS  @ overwritten for QNX vs Linux

// %esp:
//   MATCH_13  ptr unfolded_code; for escape hatch
//   MATCH_12  len unfolded code; for escape hatch
//   MATCH_99  9 saved registers {r0= argc, r1= argv, r2= envp, r3-r7, lr)

F_ADRU=  0 * NBPW
F_LENU=  2 * NBPW

  .globl upx_so_main  // in arm.v?a-linux.elf-so_main.c

  section SO_HEAD
fold: .globl fold
    sub sp,sp,#MAX_ELF_HDR_32; mov r2,sp  // &elf_tmp
    call upx_so_main  // (&so_info, &argc); returns &escape_hatch
    add sp,sp,#MAX_ELF_HDR_32
    mov lr,r0  // save &escape_hatch
    ldmia sp!,{r0,r1}  // F_ADRU, F_LENU  (unfolded region)
    mov r7,#0xff & __NR_munmap  // FIXME depends on HW and ABI of OS
    mov pc,lr  // goto &escape_hatch

L10:
  section ptr_NEXT
    mov r0,pc
    ret
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
#if defined(ARMEL_DARWIN)  /*{*/
        mov r7,#0
        mov r12,#0xff & __NR_munmap
#elif defined(ARMEL_EABI4)  /*}{*/
        mov r12,#0
        mov r7, #0xff & __NR_munmap
#elif defined(ARM_OLDABI)  /*{*/
        mov r7,#0
        mov r12,#0
#endif  /*}*/
        ldr pc,[r2,#4 -2*4]  @ Elf32_auxv_t[AT_NULL@.a_type].a_val

__NR_exit  =  1 + __NR_SYSCALL_BASE
__NR_read  =  3 + __NR_SYSCALL_BASE
__NR_write =  4 + __NR_SYSCALL_BASE
__NR_open  =  5 + __NR_SYSCALL_BASE
__NR_close =  6 + __NR_SYSCALL_BASE
__NR_unlink= 10 + __NR_SYSCALL_BASE
__NR_lseek=  19 + __NR_SYSCALL_BASE
__NR_getpid= 20 + __NR_SYSCALL_BASE
__NR_brk   = 45 + __NR_SYSCALL_BASE
__NR_readlink=  85 + __NR_SYSCALL_BASE  // 0x55
__NR_ftruncate= 93 + __NR_SYSCALL_BASE  // 0x5d


__NR_memfd_create= 0x181 + __NR_SYSCALL_BASE  // 385
__NR_mmap2    = 192 + __NR_SYSCALL_BASE  // 0xc0
__NR_mprotect = 125 + __NR_SYSCALL_BASE  // 0x7d
__NR_munmap   =  91 + __NR_SYSCALL_BASE  // 0x5b

__ARM_NR_BASE       = 0x0f0000 + __NR_SYSCALL_BASE
__ARM_NR_cacheflush = 2 + __ARM_NR_BASE

        .globl my_bkpt
my_bkpt:
        bkpt  // my_bkpt
        ret

        .globl exit
exit:
        do_sys __NR_exit

        .globl read
read:
        do_sys __NR_read; ret

Pwrite: .globl Pwrite
write: .globl write
        do_sys __NR_write; ret

lseek: .globl lseek
        do_sys __NR_lseek; ret

        .globl open
open:
        do_sys __NR_open; ret

        .globl close
close:
        do_sys __NR_close; ret

        .globl unlink
unlink:
        do_sys __NR_unlink; ret

        .globl getpid
getpid:
        do_sys __NR_getpid; ret

        .globl brk
brk:
        do_sys __NR_brk; ret

        .globl readlink
readlink:
        do_sys __NR_readlink; ret

ftruncate: .globl ftruncate
        do_sys __NR_ftruncate; ret

memfd_create: .globl memfd_create
        do_sys7t2 __NR_memfd_create; ret

O_RDWR= 2
O_DIRECTORY= 040000
O_TMPFILE= 020000000
        .balign 4
table3:
        .word O_RDWR | O_DIRECTORY | O_TMPFILE
        .word 0700
        .asciz "/dev/shm"; .balign 4

upxfd_create: .globl upxfd_create
        adr r0,table3
        ldmia r0!,{r1,r2}
        do_sys7t2 __NR_open
        ret

// Sometimes Linux enforces page-aligned address
Pprotect: .globl Pprotect
        ldr r12,m_off4k
        and r12,r12,r0
        sub r0,r0,r12
        add r1,r1,r12
mprotect: .globl mprotect
        do_sys __NR_mprotect; ret

Pmap: .globl Pmap
        ldr r12,m_off4k
        and r12,r12,r0
        sub r0,r0,r12
        add r1,r1,r12
mmap: .globl mmap
        stmdb sp!,{r4,r5,lr}
        ldr r5,[sp,#4*4]
        ldr r4,[sp,#3*4]
        mov r5,r5,lsr #12  @ convert to page number
mmap_do:
        ldr r12,m_off4k
        and r12,r12,r0  // lo frag
        sub r0,r0,r12  // page align lo end
        add r1,r1,r12
        do_sys __NR_mmap2
        ldmia sp!,{r4,r5,pc}

Punmap: .globl Punmap
        ldr r12,m_off4k
        and r12,r12,r0
        sub r0,r0,r12
        add r1,r1,r12
munmap: .globl munmap
        do_sys __NR_munmap; ret

m_off4k:
        .word -1+ (1<<PAGE_SHIFT)  // offset mask for 4KiB

        .globl __clear_cache
__clear_cache:
        mov r2,#0
        do_sys2 __ARM_NR_cacheflush; ret

get_sys_munmap: .globl get_sys_munmap  // r0= system call instruction
#if defined(ARMEL_DARWIN)  /*{*/
        ldr r0,4*1 + munmap
#elif defined(ARMEL_EABI4)  /*}{*/
        ldr r0,4*2 + munmap
#elif defined(ARM_OLDABI)  /*}{*/
        ldr r0,4*0 + munmap
#else  /*}{*/
        mov r0,#0
#endif  /*}*/
        ret

mmap_privanon: .globl mmap_privanon
        stmdb sp!,{r4,r5,lr}
        mov r4,#MAP_PRIVATE|MAP_ANONYMOUS  @ Linux: MAP_PRIVATE|MAP_ANON; QNX:MAP_PRIVANON
        mov r5,#0  @ offset= 0
        orr r3,r3,r4  @ combine with input (such as MAP_FIXED)
        mvn r4,#0  @ fd= -1
        b mmap_do

memcpy: .globl memcpy  // void *memcpy(void *dst, void const *src, size_t len);
dst .req r0
src .req r1
len .req r2
lim .req r12  // limit value of src for current code loop
        tst len,len; moveq pc,lr  // 0==len is a no_op
        mov lim,len
        //b mc_1prep  // optimizations not tested!

// Optimize for dst that is 4-aligned,
// such as fetching unaligned struct to local aligned copy.
        tst dst,#-1+ NBPW; beq mc_dst4
mc_1prep:
        add lim,len,src
mc_1:
        ldrb r3,[src],#1; cmp src,lim
        strb r3,[dst],#1; bne mc_1
        ret

mc_dst4:  // dst is 4-aligned.  Store words of 4 bytes.
        tst src,#-1+ NBPW; beq mc_44prep
mc_d4prep:
        bics r3,len,#-1+ NBPW  // length of full words
        add lim,src,r3
        beq mc_1prep  // no full words
        str len,[sp,#-NBPW]!  // save register (mc_s1d4 needs 2 registers)
mc_s1d4:  // src is not 4-aligned; dst is 4-aligned
        ldrb r2,[src],#1
        ldrb r3,[src],#1; orr r2,r2,r3,lsl #1*8
        ldrb r3,[src],#1; orr r2,r2,r3,lsl #2*8
        ldrb r3,[src],#1; orr r2,r2,r3,lsl #3*8
        str r2,[dst],#4
        cmp src,lim; bne mc_s1d4
        ldr len,[sp],#NBPW  // restore register
        ands len,len,#-1+ NBPW; bne mc_1prep  // 0 < len <= 3
        ret

mc_44prep: // both src and dst are 4-aligned. Store blocks of 16 bytes.
        bics r3,len,#-1+ 4*NBPW  // length of 16-byte blocks
        add lim,src,r3
        bne mc_s4d4go  // 16-byte blocks
mc_s4d1prep:
        bics r3,len,#-1+ NBPW  // length of full words
        add lim,src,r3
        beq mc_1prep  // no full words
mc_s4d1:
        ldr r3,[src],#NBPW
        str r3,[dst],#NBPW
        cmp src,lim; bne mc_s4d1
        ands len,len,#-1+ NBPW; bne mc_1prep  // 0 < len <= 3
        ret

mc_s4d4go:
        stmdb sp!,{r4,r5,r6,r7}  // save original contents of working storage
mc_s4d4:
        ldmia src!,{r4,r5,r6,r7}
        stmia dst!,{r4,r5,r6,r7}
        cmp src,lim; bne mc_s4d4
        ldmia  sp!,{r4,r5,r6,r7}  // restore original contents of working storage
        ands len,len,#-1+ 4*NBPW; bne mc_s4d1prep  // 0 < len <= 15
        ret
    .unreq dst
    .unreq src
    .unreq len
    .unreq lim


tmp  .req r3

size .req r4
ptr  .req r5
ulen .req r6
pflg .req r7

underlay: .globl underlay // (unsigned size, char *ptr, unsigned ulen, unsigned p_flags)  // ulen <= PAGE_SIZE
    stmdb sp!,{r0,r1,r2,r3,r4,r5,r6,r7, lr}
    ldmia sp!, {r4,r5,r6,r7}  // r4= r0; r5= r1; r6= r2; r7= r3;
    mov r0,sp

    sub tmp,sp,ulen
    and sp,tmp,#-2*NBPW
    str r0,[sp,#-NBPW]!  // save sp for pop at return

    add r0,sp,#NBPW
    // mov r1,ptr  // not yet overwritten
    // mov r2,ulen  // not yet overwritten
    bl memcpy

    mov r0,ptr
    mov r1,size
    mov r2,#PROT_WRITE|PROT_READ
    tst pflg,#PF_X  // if eventually PROT_EXEC,
    orrne r2,r2,#PROT_EXEC  // ... then Linux ARM wants it now, too
    mov r3,#MAP_FIXED
    bl mmap_privanon  // r0= ptr because MAP_FIXED

    add r1,sp,#NBPW
    mov r2,ulen
    bl memcpy

    ldr sp,[sp]
    ldmia sp!,{r4,r5,r6,r7, pc}

    .unreq tmp

    .unreq size
    .unreq ptr
    .unreq ulen
    .unreq pflg

memset: .globl memset  // void *memset(void *s, int c, size_t n);
        tst r2,r2; moveq pc,lr
        strb r1,[r0],#1; subs r2,r2,#1
        bgt memset
        ret

my_alloca: .globl my_alloca
        sub r0,sp,r0
        and r0,r0,#-2*NBPW
        mov sp,r0
        ret

#if 1|DEBUG  /*{*/

__udivsi3: .globl __udivsi3
div10: .globl div10
        mov ip,r0  @ extra copy used at end
        sub r1,r1,r1  @ hi

        mov r2,r0  @ copy lo
        adds r0,r0,r0,lsl #3   @ 9*lo
        adc  r1,r1,r1,lsl #3   @ 9*hi + C
        add  r1,r1,r2,lsr #(32 - 3)  @ bits shifted from lo to hi

        mov r2,r0  @ copy lo
        adds r0,r0,r0,lsl #4
        adc  r1,r1,r1,lsl #4
        add  r1,r1,r2,lsr #(32 - 4)  @ * 0x99

        mov r2,r0  @ copy lo
        adds r0,r0,r0,lsl #8
        adc  r1,r1,r1,lsl #8
        add  r1,r1,r2,lsr #(32 - 8)  @ * 0x9999

        mov r2,r0  @ copy lo
        adds r0,r0,r0,lsl #16
        adc  r1,r1,r1,lsl #16
        add  r1,r1,r2,lsr #(32 - 16)  @ * 0x99999999

        subs r0,r0,ip,lsl #(32 - 1)  @ - * 0x80000000
        sbc  r1,r1,ip,lsr #1         @   * 0x19999999

        adds r0,r0,ip
        adc  r0,r1,#0  @ * 0x0.1999999a
        ret

#endif  /*}*/

/* vim:set ts=8 sw=8 et: */
