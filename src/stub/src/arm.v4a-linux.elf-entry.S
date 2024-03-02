/*  arm-linux.elf-entry.S -- Linux program entry point & decompressor (Elf binary)
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2024 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2024 Laszlo Molnar
*  Copyright (C) 2000-2024 John F. Reiser
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
NBPW= 4
#include "arch/arm/v4a/macros.S"
#define call bl

arg1 .req r0
arg2 .req r1
arg3 .req r2
arg4 .req r3
arg5 .req r4
arg6 .req r5

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

AT_NULL=   0
AT_PAGESZ= 6

NAME_MAX=  255  // # chars in file name; linux/include/uapi/linux/limits.h

// Defaults unless overridden by AT_PAGESZ
PAGE_SHIFT= 12
PAGE_MASK=  (~0<<PAGE_SHIFT)
PAGE_SIZE = ( 1<<PAGE_SHIFT)

__NR_SYSCALL_BASE = 0

__NR_exit=      1 + __NR_SYSCALL_BASE
__NR_open=      5 + __NR_SYSCALL_BASE
__NR_close=     6 + __NR_SYSCALL_BASE
__NR_ftruncate=93 + __NR_SYSCALL_BASE
__NR_getpid=   20 + __NR_SYSCALL_BASE
__NR_lseek=    19 + __NR_SYSCALL_BASE
__NR_memfd_create= 385 + __NR_SYSCALL_BASE
__NR_mkdir=    39 + __NR_SYSCALL_BASE
__NR_mmap2=   192 + __NR_SYSCALL_BASE
__NR_mprotect=125 + __NR_SYSCALL_BASE
__NR_open=      5 + __NR_SYSCALL_BASE
__NR_read=      3 + __NR_SYSCALL_BASE
__NR_stat=    106 + __NR_SYSCALL_BASE
__NR_uname=   122 + __NR_SYSCALL_BASE
__NR_unlink=   10 + __NR_SYSCALL_BASE
__NR_write=     4 + __NR_SYSCALL_BASE

__ARM_NR_BASE= 0xf0000 + __NR_SYSCALL_BASE
__ARM_NR_cacheflush=  2 + __ARM_NR_BASE

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

  section ELFMAINX
F_ADRX= 0*NBPW
F_LENX= 1*NBPW
F_ELFA= 2*NBPW
F_ADRU= 3*NBPW
F_LENU= 4*NBPW
F_ENTR= 5*NBPW
  F_QFLG= F_ENTR
F_SIZE= 6*NBPW

// (read-only) .data space at start of unfolded code
D_PMASK= 0*NBPW
D_FNAME= 1*NBPW
D_QFLG=  2*NBPW
D_FOLD=  3*NBPW  // start of code

r_pack2 .req r9
r_pmask .req r8

start_params:
sz_pack2= . - NBPW
mflg:
        .word MFLG  // MAP_{PRIVATE|ANONYMOUS}  // QNX vs linux
_start: .globl _start
////  nop; bkpt  // DEBUG
        stmdb sp!,{r0,r1}  // ABI crt0 -static may have data here
        add arg1,sp,#(1+ 2)*NBPW
        call zfind  // avoid feint of 0==argc;  out: r0= &envp
        call zfind
        sub sp,sp,#F_SIZE

// set xPMASK by finding actual page size in Elf64_auxv
1:
        ldmia r0!,{r1,r2}
        cmp r1,#AT_PAGESZ; beq 2f
        cmp r1,#AT_NULL; bne 1b
        mov r2,#PAGE_SIZE  // default
2:
        mvn r2,r2; add r2,r2,#1  // neg
        mov r_pmask,r2

        ldr r1,sz_pack2
        adr r0,sz_pack2
        sub r0,r0,r1  @ elfaddr= &Elf_Ehdr of this program
        str r0,[sp,#F_ELFA]
        mov r_pack2,r1  @ save sz_pack2

        ldr r0,mflg
        str r0,[sp,#F_QFLG]

        call get_data  // cpr0 can be too far away
old_sp .req r11
binfo  .req r10
r_unc  .req r7
u_len  .req r6
        add binfo,lr,#NBPW  // skip o_binfo for now
unfold:
        ldr r_unc,[binfo,#sz_unc]
        mov r0,    #(1+ NAME_MAX + 2*NBPW + D_FOLD + (1+ 11 + 13)) & ~0xff
        add r0, r0,#(1+ NAME_MAX + 2*NBPW + D_FOLD + (1+ 11 + 13)) &  0xff
        add u_len,r_unc,r0
        str u_len,[sp,#F_LENU]
// alloca for generated pathname (necessary if memfd_create fails)
        mov old_sp,sp
        sub sp,sp,r0
        and sp,sp,#-2*NBPW  // align stack

        mov r0,#0
        mov r1,sp
0:
        stmia r1!,{r0}
        cmp r1,old_sp; blo 0b
        
        mov arg3,sp  // &pathname (currently a null string "")
        mov arg2,u_len
        mov arg1,#0  // kernel chooses addr
        call upx_mmap_and_fd  // (addr + (1+ fd)) = (ptr, len, pathname)
        mov r1,r0,lsr #12
        mov r1,r1,lsl #12
        sub r0,r0,r1
        sub r0,r0,#1
//{ r0 and r1 are busy!

// copy generated pathname (if any) from stack into mapped region
        add r2,r1,r_unc
        add r2,r2,# 2*NBPW
        and r2,r2,#-2*NBPW
0:
        ldmia sp!,{r3}  // gradual de-alloca
        stmia r2!,{r3}
        cmp sp,old_sp; blo 0b
mfd  .req old_sp
   .unreq old_sp
        str r1,[sp,#F_ADRU]
        mov mfd,r0
//}{ r0 free, r1 busy
        add r4,r_unc,# 2*NBPW + D_FOLD
        and r4,r4,   #-2*NBPW  // align displacement
        ldr r3,[sp,#F_QFLG]
        mov r2,r_pmask
        stmia r1,{r2,r3,r4}  // D_PMASK, D_QFLG, D_FNAME
  .unreq r_pmask

// De-compress folded stage of stub
        stmdb sp!,{r_unc}  @ P_01
        mov arg4,sp  @ &sz_unc = &dstlen
        add arg3,r1,#D_FOLD  @ dst
//} r1 free
        ldrb r0, [binfo,#b_method]
        stmdb sp!,{r0}  @ P_02  5th param to f_exp
        ldr arg2,[binfo,#sz_cpr]  @ srclen
        add arg1, binfo,#sz_b_info  @ src
#if DEBUG  /*{*/
        stmdb sp!,{TRACE_REGS}; mov r0,#2; bl trace
#endif  /*}*/
        call f_expand
        add sp,sp,#2*NBPW  @ P_02, P_01  remove 5th param

        mov r5,#0
        mov r4,mfd
        mov r3,#MAP_PRIVATE|MAP_FIXED
        mov r2,#PROT_EXEC|PROT_READ
        ldr r1,[sp,#F_LENU]
        ldr r0,[sp,#F_ADRU]
        do_sys7t __NR_mmap2  // clobbers r7  r_unc

        mov r0,mfd
        do_sys7t __NR_close

// jmp unfolded_code
        ldr r0,[sp,#F_ELFA]
        ldr r1,[binfo,#-NBPW]  @ O_BINFO
        add r4,r0,r1  @ ADRX= &b_info
        sub r5,r_pack2,r1  @ LENX= sz_pack2 - O_BINFO
        stmia sp,{r4,r5}  // F_ADRX, F_LENX
#if DEBUG  /*{*/
        stmdb sp!,{TRACE_REGS}; mov r0,#3; bl trace
#endif  /*}*/
        ldr r12,[sp,#F_ADRU]
        add pc,r12,#D_FOLD  // goto unfolded code

zfind:
        ldr r1,[r0],#NBPW
        cmp r1,#0; bne zfind
        ret

f_expand:
#define LINUX_ARM_CACHEFLUSH 1
#define NO_METHOD_CHECK 1
#include "arch/arm/v4a/nrv2b_d8.S"

  section ELFMAINY
end_decompress: .globl end_decompress
        /* IDENTSTR goes here */

section ELFMAINZ
#if DEBUG  /*{*/
TRACE_BUFLEN=512
trace:
        str lr,[sp,#(-1+ 15)*4]  @ return pc; [remember: sp is not stored]
        mov r4,sp  @ &saved_r0
        sub sp,sp,#TRACE_BUFLEN
        mov r2,sp  @ output string

        mov r1,#'\n'; bl trace_hex  @ In: r0 as label
        mov r1,#'>';  strb r1,[r2],#1

        mov r5,#3  @ rows to print
L600:  @ each row
        sub r0,r4,#TRACE_BUFLEN
        sub r0,r0,sp
        mov r0,r0,lsr #2; mov r1,#'\n'; bl trace_hex  @ which block of 8

        mov r6,#8  @ words per row
L610:  @ each word
        ldr r0,[r4],#NBPW; mov r1,#' '; bl trace_hex  @ next word
        subs r6,r6,#1; bgt L610

        subs r5,r5,#1; bgt L600

        mov r0,#'\n'; strb r0,[r2],#1
        sub r2,r2,sp  @ count
        mov r1,sp  @ buf
        mov r0,#2  @ FD_STDERR
#if defined(ARMEL_EABI4)  /*{*/
        mov r7,#__NR_write
        swi 0
#else  /*}{*/
        swi __NR_write
#endif  /*}*/
        add sp,sp,#TRACE_BUFLEN
        ldmia sp!,{TRACE_REGS}

trace_hex:  // In: r0=val, r1=punctuation before, r2=ptr; Uses: r3, ip
        strb r1,[r2],#1  @ punctuation
        mov r3,#NBPW*(8 -1)  @ shift count
        adr ip,hex
L620:
        mov r1,r0,lsr r3
        and r1,r1,#0xf
        ldrb r1,[ip, r1]
        strb r1,[r2],#1
        subs r3,r3,#NBPW; bge L620
        ret
hex:
        .ascii "0123456789abcdef"
        .balign 4
#endif  /*}*/

get_upxfn_path: .globl get_upxfn_path
        bkpt  // get_upx_fn_path should not be called in this phase
        ret

mempcpy: .globl mempcpy  // (dst, src, n)
        cmp r2,#0; beq 9f
0:
        ldrb r3,[r1],#1
        strb r3,[r0],#1
        subs r2,r2,#1; bne 0b
9:
        ret

my_memset: .globl my_memset  // (dst, val, n)
        cmp r2,#0; beq 9f
0:
        strb r1,[r0],#1
        subs r2,r2,#1
        bne 0b
9:
        ret

// These Linux system calls are called from upxfd_android.c
// in order to work around problems with memfd_create and ftruncate on Android.
// Because called from C, then r7 is live; cannot use do_sys7t.
.globl memfd_create; memfd_create: do_sys2 __NR_memfd_create; ret
.globl close;     close:     do_sys __NR_close; ret
.globl ftruncate; ftruncate: do_sys __NR_ftruncate; ret
.globl getpid;    getpid:    do_sys __NR_getpid; ret
.globl lseek;     lseek:     do_sys __NR_lseek; ret
.globl mkdir;     mkdir:     do_sys __NR_mkdir; ret
.globl open;      open:      do_sys __NR_open; ret
.globl read;      read:      do_sys __NR_read; ret
.globl stat;      stat:      do_sys __NR_stat; ret
.globl uname;     uname:     do_sys __NR_uname; ret
.globl unlink;    unlink:    do_sys __NR_unlink; ret
.globl write;     write:     do_sys __NR_write; ret

// __NR_oldmmap gets ENOSYS!  Must use __NR_mmap2 with all args in registers
// Called from C (5th and 6th arg on stack), so must preserve r4 and r5
mmap: .globl mmap
        stmdb sp!,{r4,r5,lr}  // called from C: only 4 args in registers
        ldr arg6,[sp,#4*NBPW]
        ldr arg5,[sp,#3*NBPW]
        mov arg6,arg6,lsr #12  @ FIXME?  convert to page offset in file
mmap_do: // sp: saved r4,r5,lr
        mov r12,#~0; mov r12,r12,lsl #12  // PAGE_MASK
        bic r12,arg1,r12  // lo frag
        sub arg1,arg1,r12  // page align lo end
        add arg2,arg2,r12
        do_sys __NR_mmap2
        ldmia sp!,{r4,r5,pc}

get_data:
        mov r12,lr; mov lr,pc; mov pc,r12  // blx lr
o_binfo:
        .word O_BINFO  // .int4 offset of b_info for text
cpr0: .globl cpr0
        /* { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...} */
eof:

/* vim:set ts=8 sw=8 et: */
