/*  arm.v4a-linux.elf-so_entry.S -- Linux DT_INIT & decompressor (Elf shared lib)
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2021 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2021 Laszlo Molnar
*  Copyright (C) 2000-2023 John F. Reiser
*  All Rights Reserved.
*
*  UPX and the UCL library are free software; you can redistribute them
*  and/or modify them under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; see the file COPYING.
*  If not, write to the Free Software Foundation, Inc.,
*  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
*  Markus F.X.J. Oberhumer              Laszlo Molnar
*  <markus@oberhumer.com>               <ezerotven+github@gmail.com>
*
*  John F. Reiser
*  <jreiser@users.sourceforge.net>
*/

NBPW= 4
//#define ARM_OLDABI 1
#define ARMEL_EABI4 1
#include "arch/arm/v5a/macros.S"
#define DEBUG 0

#define bkpt .long 0xe7f001f0  /* reserved instr; Linux GNU eabi breakpoint */
#define bkpt_th .short 0xde01  /* reserved instr; Linux GNU eabi breakpoint */
// As of 2023-02-04, "gcc (Debian 10.2.1-6) 10.2.1 20210110" assembles 0xbe10 for:
//      asm("bkpt #0x10");
// and a RaspberryPi v2 (32-bit only) running Linux executes as an infinite loop
// with kernel message spewing:
//       Unhandled prefetch abort: breakpoint debug exception (0x002)
// That's running with "uname -a" that says
//      Linux <hostname> 5.10.0-20-armmp #1 SMP Debian 5.10.158-2 (2022-12-13) armv7l GNU/Linux

sz_Elf32_Ehdr = 13*NBPW
sz_Elf32_Phdr =  8*NBPW

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8
sz_l_info= 12
sz_p_info= 12

PROT_READ=  1
PROT_WRITE= 2
PROT_EXEC=  4

MAP_PRIVATE= 2
MAP_FIXED=     0x10
MAP_ANONYMOUS= 0x20

PAGE_SHIFT= 12
PAGE_MASK=  (~0<<PAGE_SHIFT)
PAGE_SIZE= -PAGE_MASK

NAME_MAX=  255  // # chars in file name; linux/include/uapi/linux/limits.h

__NR_memfd_create= 0x181 + __NR_SYSCALL_BASE  // 385
__NR_mmap64 = 0xc0 + __NR_SYSCALL_BASE
__NR_mprotect =125 + __NR_SYSCALL_BASE
__NR_munmap =   91 + __NR_SYSCALL_BASE

__NR_exit =      1 + __NR_SYSCALL_BASE
__NR_read  =     3 + __NR_SYSCALL_BASE
__NR_write =     4 + __NR_SYSCALL_BASE
__NR_open=       5 + __NR_SYSCALL_BASE
__NR_close=      6 + __NR_SYSCALL_BASE
__NR_unlink=    10 + __NR_SYSCALL_BASE
__NR_lseek=     19 + __NR_SYSCALL_BASE
__NR_getpid=    20 + __NR_SYSCALL_BASE
__NR_mkdir =    39 + __NR_SYSCALL_BASE  // 0x27

__NR_oldmmap=   90 + __NR_SYSCALL_BASE
__NR_ftruncate= 93 + __NR_SYSCALL_BASE
__NR_stat=     106 + __NR_SYSCALL_BASE
__NR_uname=    122 + __NR_SYSCALL_BASE
__NR_fstat=    197 + __NR_SYSCALL_BASE  // 0xc5
__NR_openat=   322 + __NR_SYSCALL_BASE
__NR_mkdirat=  323 + __NR_SYSCALL_BASE

__NR_fstatat=  383 + __NR_SYSCALL_BASE

__ARM_NR_BASE  = 0xf0000 + __NR_SYSCALL_BASE
__ARM_NR_cacheflush =  2 + __ARM_NR_BASE

arg1 .req r0
arg2 .req r1
arg3 .req r2
arg4 .req r3
arg5 .req r4
arg6 .req r5

esi .req r1
eax .req r4

#define SP(d) sp,#4*(_-d)  /* stack addressing mode */

.macro thumb_sys7t N
#if defined(ARMEL_EABI4)
        mov r7,#\N
        swi 0
#elif defined(ARM_OLDABI)
        blx x\N
#else
        error \N  // ARM_OLDABI, ARMEL_EABI4, ARMEL_DARWIN ?
#endif

.endm

.macro call4 label // ARM mode; not THUMB mode
        .balign 4
        bl \label
.endm

.macro blx reg  // armv4a only
        mov lr,pc  // .+2*NBPW
        b \reg  // reg must not be 'lr'
.endm

.macro push_ reg
        str reg,[sp,#-4]!
_= 1+_  // one more word on stack
.endm

.macro pop_ reg
        pop {\reg}
_=-1+_  // one less word on stack
.endm

  section ELFMAINX
        //.arch armv7-a
        //.syntax unified
        .type _start,%function
        .balign 4
_start:
        .thumb  // via PackLinuxElf32::pack3
        bx pc; nop  // but switch to ARM until debugged

        .arm
        nop
#if 0|DEBUG  //{
        bkpt  // DEBUG
#endif  //}
        // argc,argv,envp, r3 convenience, r4-r7 callee-saved, lr ret_addr
        stmfd sp!,{r0,r1,r2, r3, r4,r5,r6,r7, lr}  // MATCH_99
        stmfd sp!,{r8,r9,r10,r11}  // MATCH_98

        sub sp,sp,#3*NBPW  // space for MFD, ADRU, LENU
F_MFD=  0 * NBPW
F_ADRU= 1 * NBPW
F_LENU= 2 * NBPW
F_ARGC= 3 * NBPW

        call4 L70
L70_ret:
    foldi   .req r11  // &fold_info
        mov foldi,lr
    old_sp  .req r5
        mov old_sp,sp
    r_unc   .req r6
        ldr r_unc,[foldi,#sz_unc]
            // align + {page_mask, fd_name} + "/data/data/$APP_NAME/cache/upxAAA"
    u_len   .req r7
        add u_len,r_unc,#8 + 2*NBPW + (1+ 11 + 13)
        add u_len,u_len,#1+ NAME_MAX
        str u_len,[old_sp,#F_LENU]
        sub r0,sp,u_len  // alloca
        and sp,r0,#-2*NBPW  // align stack

        add arg3,sp,r_unc
        add arg3,arg3,#8 + 2*NBPW
        and arg3,arg3,#-2*NBPW  // where to put pathname
        sub r0,arg3,sp; str r0,[sp,#1*NBPW]  // offset(pathname)
        mvn r0,#0;      str r0,[sp,#0*NBPW]  // space for PAGE_MASK
        mov arg2,u_len
        mov arg1,#0  // any page address
        str arg1,[arg3]  // start with 0-length pathname
        bl upx_mmap_and_fd // (ptr, len, pathname)
        mov r1,r0,lsr #12; mov r1,r1,lsl #12; str r1,[old_sp,#F_ADRU]
        sub r0,r0,r1; sub r0,r0,#1;           str r0,[old_sp,#F_MFD]

        str r_unc,[sp,#-NBPW]!  // dstlen
        mov arg4,sp  // &dstlen
        add arg3,sp,#(1+ 2)*NBPW  // dst for decompress
        ldr arg2,[foldi,#sz_cpr]  // srclen
        add arg1,foldi,#sz_b_info  // src
    .unreq foldi
        bl f_decompress
        add sp,sp,#NBPW  // toss dstlen

        ldr arg1,[old_sp,#F_MFD]
        mov arg2,sp
        ldr arg3,[old_sp,#F_LENU]
        do_sys7t __NR_write
        mov sp,old_sp  // de-alloca
    .unreq old_sp

        mov arg6,#0  // beginning of file
        ldr arg5,[sp,#F_MFD]
        mov arg4,#MAP_PRIVATE  // modes
        mov arg3,#PROT_READ|PROT_EXEC  // prot
        ldr arg2,[sp,#F_LENU]
        ldr arg1,[sp,#F_ADRU]
        do_sys7t __NR_mmap64  // arg1-arg6 all in registers r0-r5
        str r0,[sp,#F_ADRU]

        ldr arg1,[sp,#F_MFD]
        do_sys7t __NR_close

// typedef struct {
//     long argc;
//     char **argv;
//     char **envp;
// } So_args;
//
// typedef struct {
//     unsigned off_reloc;  // distance back to &Elf32_Ehdr
//     unsigned off_user_DT_INIT;
//     unsigned off_xct_off;  // where un-compressed bytes end  [unused?]
//     unsigned off_info;  //  xct_off: {l_info; p_info; b_info; compressed data)
// } So_info;
//
// void *
// upx_so_main(  // returns &escape_hatch
//     So_info *so_info,
//     So_args *so_args,
//     Elf32_Ehdr *elf_tmp  // [supplied by unfolded] scratch for Elf32_Ehdr and Elf32_Phdrs
// )
F_MFD=  0 * NBPW
F_ADRU= 1 * NBPW
F_LENU= 2 * NBPW
        ldmia sp!,{r0,r2,r3}  // F_MFD,F_ADRU,F_LENU
        ldmia sp!,{r8,r9,r10,r11}  // MATCH_98
        add r1,r2,#3*NBPW  // skip offsets for {page_mask, file_name, mflg_data}
        adr r0,_start -4*4  // &So_info
        stmdb sp!,{r1,r2,r3}  // &fold,F_ADRU,F_LENU
        add r1,sp,#F_ARGC
        ldr pc,[sp],#NBPW  // invoke folded code
//%esp:
//  MATCH_04  ptr unfolded_code
//  MATCH_10  len unfolded_code
//  MATCH_00  argc
//  MATCH_01  argv
//  MATCH_07  envp

get_upxfn_path: .globl get_upxfn_path
        mov r0,#0  // should not be called!
        ret

f_decompress:
#define LINUX_ARM_CACHEFLUSH 1
#include "arch/arm/v4a/nrv2b_d8.S"

// section UPXFD_ANDROID goes here

my_bkpt: .globl my_bkpt
        bkpt  // my_bkpt
        ret

my_memset: .globl my_memset  // (dst, val, n)
        cmp r2,#0; beq 9f
0:
        strb r1,[r0],#1
        subs r2,r2,#1
        bne 0b
9:
        ret

mempcpy: .globl mempcpy  // (dst, src, n)
        cmp r2,#0; beq 9f
0:
        ldrb r3,[r1],#1
        strb r3,[r0],#1
        subs r2,r2,#1
        bne 0b
9:
        ret

// All of these syscalls must preserve r7 (cannot use 'do_sys7t')
// because they are called from C-language upxfd_android.c
fstat: .globl fstat
        do_sys __NR_fstat; ret
ftruncate: .globl ftruncate
        do_sys __NR_ftruncate; ret
getpid: .globl getpid
        do_sys __NR_getpid; ret
lseek: .globl lseek
        do_sys __NR_lseek; ret
mkdir: .globl mkdir
        do_sys __NR_mkdir; ret
open: .globl open
        do_sys __NR_open; ret
read: .globl read
        do_sys __NR_read; ret
stat: .globl stat
        do_sys __NR_stat; ret
uname: .globl uname
        do_sys __NR_uname; ret
memfd_create: .globl memfd_create
        do_sys2 __NR_memfd_create; ret
unlink: .globl unlink
        do_sys __NR_unlink; ret
write: .globl write
        do_sys __NR_write; ret

// __NR_oldmmap gets ENOSYS!  Must use __NR_mmap64 with all args in registers
// Called from C, so must preserve r4 and r5
mmap: .globl mmap
        stmdb sp!,{r4,r5,lr}
        ldr r4,[sp,#3*NBPW]
        ldr r5,[sp,#4*NBPW]  // lo32 of offset; where is hi32?
        mov r5,r5,lsr #12  // FIXME page number in file
        do_sys __NR_mmap64
        ldmia sp!,{r4,r5,pc}

// IDENTSTR goes here

  section ELFMAINZ
L70:
        call4 L70_ret
fold_info:
//  b_info (sz_unc, sz_cpr, method) of folded code (C-language, etc.)

/* vim:set ts=8 sw=8 et: */
