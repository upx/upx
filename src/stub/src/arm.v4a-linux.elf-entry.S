/*  arm-linux.elf-entry.S -- Linux program entry point & decompressor (Elf binary)
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2023 Laszlo Molnar
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

#define ARM_OLDABI 1
#include "arch/arm/v4a/macros.S"
NBPW= 4
#define call  bl
.macro callr reg  // armv4a lacks 'blx'
        mov r12,\reg
        mov lr,pc
        mov pc,r12
.endm

#define bkpt .long 0xe7f001f0  /* reserved instr; Linux GNU eabi breakpoint */
sz_Elf32_Phdr =  8*NBPW
  p_vaddr = 2*NBPW
sz_Elf32_Ehdr = 13*NBPW
  e_type= 16
    ET_DYN= 3
  e_phnum= 16 + 2*2 + 5*NBPW + 2*2

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8
sz_l_info= 12
sz_p_info= 12

PROT_READ=  1
PROT_WRITE= 2
PROT_EXEC=  4

MAP_ANONYMOUS= 0x20
MAP_FIXED=     0x10
MAP_PRIVATE=   0x02

FD_stderr=      2
O_RDONLY=       0

PAGE_SHIFT= 12
PAGE_SIZE = -(~0<<PAGE_SHIFT)

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
  // r4,r5 only for mmap64
arg5 .req r4
arg6 .req r5

#ifndef DEBUG  /*{*/
#define DEBUG 0
#endif  /*}*/

#if DEBUG  //{
#define TRACE_REGS r0-r12,r14,r15
// sp (r13) is not included because the write-back might cause UNDEFINED behavior
// if the write-back register is not first or last.  The actual value of sp
// usually does not matter.  Just remember that lr (r14) and pc (r15) are stored
// one word closer to the stack pointer because r13 has been omitted.
#endif  //}

        //.long sz_pack2  // placed there by ::pack3()
  section ELFMAINX
start_params:
sz_pack2= . - 4
mflg:
        .long MFLG  // MAP_{PRIVATE|ANONYMOUS}  // QNX vs linux
_start: .globl _start
////  nop; bkpt

// In:
// r4= LENX
// r5= ADRX
// lr= "/proc/self/exe"
//new sp/  elfaddr,fd, ADRU,LENU, f_exp,%entry, argc,argv,0,envp,0,auxv

F_FRAME= 7*NBPW
F_ENTR=  6*NBPW; F_PMASK= F_ENTR
F_MFD=   5*NBPW
F_LENU=  4*NBPW
F_ADRU=  3*NBPW
F_ELFA=  2*NBPW
F_LENX=  1*NBPW
F_ADRX=  0*NBPW

    old_sp  .req r6
        add r0,sp,#NBPW  // skip argc
        sub sp,sp,#F_FRAME; mov old_sp,sp
        call zfind  // skip argv
        call zfind  // skip env
AT_PAGESZ= 6  // /usr/include/elf.h
0: // find AT_PAGESZ in env
        ldr r1,[r0],#NBPW
        ldr r2,[r0],#NBPW
        cmp r1,#AT_PAGESZ; beq 5f
        mov r2,$1<<12  // default PAGE_SIZE
        cmp r1,#0; bne 0b
5:
        mov r0,#0; sub r7,r0,r2

        adr r2,sz_pack2
        ldr r1,[r2]  @ sz_pack2
        mov r9,r1  @ save sz_pack2
        sub r2,r2,r1; str r2,[old_sp,#F_ELFA]
        ldr r11,mflg
        call L70  // come back with lr= &fold_info

    foldi   .req r4  // &fold_info
        mov foldi,lr
        ldr r0,[foldi],#NBPW  // O_BINFO
        add r2,r2,r0; str r2,[old_sp,#F_ADRX]
        sub r2,r1,r0; str r2,[old_sp,#F_LENX]
    r_unc   .req r5
        ldr r_unc,[foldi,#sz_unc]
            // align + {page_mask, fd_name} + "/data/data/$APP_NAME/cache/upxAAA"
    u_len   .req arg2
        add u_len,r_unc,#8 + 3*NBPW + (1+ 11 + 13)
        add u_len,u_len,#1+ NAME_MAX
        str u_len,[old_sp,#F_LENU]
// alloca
        sub r0,sp,u_len
        and sp,r0,#-2*NBPW  // align stack

            str r7, [sp,#0*NBPW]  // PAGE_MASK
        add arg3,r_unc,#8 + 3*NBPW
        and arg3,arg3,#-2*NBPW  // offset for pathname
            str arg3, [sp,#1*NBPW]
        add arg3,arg3,sp
            str r11,[sp,#2*NBPW]  // mflg
        //mov arg2,u_len  // already there
    .unreq u_len
        mov arg1,#0  // any page address
        str arg1,[arg3]  // start with 0-length pathname
        bl upx_mmap_and_fd // (ptr, len, pathname)
        mov r1,r0,lsr #12; mov r1,r1,lsl #12; str r1,[old_sp,#F_ADRU]
        sub r0,r0,r1; sub r0,r0,#1;           str r0,[old_sp,#F_MFD]

        str r_unc,[sp,#-NBPW]!  // dstlen
        mov arg4,sp  // &dstlen
        add arg3,sp,#(1+ 3)*NBPW  // dst for decompress
        ldr arg2,[foldi,#sz_cpr]  // srclen
        add arg1,foldi,#sz_b_info  // src
    .unreq foldi
        bl f_decompress
        add sp,sp,#NBPW  // toss dstlen

        ldr arg1,[old_sp,#F_MFD]
        mov arg2,sp
        ldr arg3,[old_sp,#F_LENU]
        do_sys7t __NR_write
// de-alloca
        mov sp,old_sp
    .unreq old_sp

        ldr arg1,[sp,#F_ADRU]
        ldr arg2,[sp,#F_LENU]
        do_sys7t __NR_munmap  // remove old VMA; mfd has the bits

        mov arg6,#0  // beginning of file
        ldr arg5,[sp,#F_MFD]
        mov arg4,#MAP_PRIVATE|MAP_FIXED  // modes
        mov arg3,#PROT_READ|PROT_EXEC  // prot
        ldr arg2,[sp,#F_LENU]
        ldr arg1,[sp,#F_ADRU]
        do_sys7t __NR_mmap64  // arg1-arg6 all in registers r0-r5
        str r0,[sp,#F_ADRU]

        ldr arg1,[sp,#F_MFD]
        do_sys7t __NR_close

        ldr r0,[sp,#F_ADRU]
        add pc,r0,#3*NBPW  // jmp unfolded code

zfind:
        ldr r1,[r0],#NBPW
        cmp r1,#0; bne zfind
        ret

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
        stmdb sp!,{r4,r5}
        ldr r4,[sp,#2*NBPW]
        ldr r5,[sp,#3*NBPW]  // lo32 of offset; where is hi32?
        mov r5,r5,lsr #12  // FIXME page number in file
        do_sys __NR_mmap64
        ldmia sp!,{r4,r5}
        ret

// IDENTSTR goes here

  section ELFMAINZ

L70:
        callr lr
fold_info:
o_binfo:
        .long O_BINFO  // .int4 offset of b_info for text
//  b_info (sz_unc, sz_cpr, method) of folded code (C-language, etc.)

/* vim:set ts=8 sw=8 et: */
