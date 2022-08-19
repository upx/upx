/*  arm-darwin.macho-entry.S -- iPhone program entry point & decompressor (Elf binary)
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2022 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2022 Laszlo Molnar
*  Copyright (C) 2000-2022 John F. Reiser
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

#define SIMULATE_ON_LINUX_EABI4 0

#if SIMULATE_ON_LINUX_EABI4  /*{*/
  #define LINUX_ARM_CACHEFLUSH 1  /* SIMULATE_ON_LINUX_EABI4 */
  #define ARMEL_EABI4 1           /* SIMULATE_ON_LINUX_EABI4 */
#else  /*}{ native darwin usual case */
  #define DARWIN_ARM_CACHEFLUSH 1
  #define ARMEL_DARWIN 1
#endif  /*}*/

#if DEBUG  //{
#define TRACE_REGS r0-r12,r14,r15
// sp (r13) is not included because the write-back might cause UNDEFINED behavior
// if the write-back register is not first or last.  The actual value of sp
// usually does not matter.  Just remember that lr (r14) and pc (r15) are stored
// one word closer to the stack pointer because r13 has been omitted.
#endif  //}

NBPW= 4
#include "arch/arm/v5a/macros.S"

#define bkpt .long 0xe1200070

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
PAGE_MASK =  (~0<<PAGE_SHIFT)
PAGE_SIZE = -PAGE_MASK

__NR_SYSCALL_BASE = 0

__NR_exit =      1 + __NR_SYSCALL_BASE
__NR_write =     4 + __NR_SYSCALL_BASE
__NR_open  =     5 + __NR_SYSCALL_BASE
#if SIMULATE_ON_LINUX_EABI4  //{
__NR_mmap  =  0xc0 + __NR_SYSCALL_BASE
__NR_mprotect=0x7d + __NR_SYSCALL_BASE
#else  //}{  native darwin usual case
__NR_mmap  =   197 + __NR_SYSCALL_BASE
__NR_mprotect = 74 + __NR_SYSCALL_BASE
#endif  //}

// DEBUG ONLY:
__ARM_NR_BASE  = 0xf0000 + __NR_SYSCALL_BASE
__ARM_NR_cacheflush =  2 + __ARM_NR_BASE

//0:    .word 0b - &Mach_header  // backwards distance to Mach_header
//0:    .word 0b - l_info  // backwards distance to l_info
  section MACHMAINX
_start: .globl _start
#if DEBUG  /*{*/
        stmdb sp!,{TRACE_REGS}; mov r0,#1; bl trace
#endif  /*}*/
        mov r7,sp  // &argc
        adr r6,-2*NBPW + _start
        bl main
L20:
f_decompress:

  section NRV_HEAD
        // empty

  section NRV2E
#include "arch/arm/v4a/nrv2e_d8.S"

  section NRV2D
#include "arch/arm/v4a/nrv2d_d8.S"

  section NRV2B
#include "arch/arm/v4a/nrv2b_d8.S"

  section NRV_TAIL
        // empty

#include "arch/arm/v4a/lzma_d.S"

  section MACHMAINY
end_decompress: .globl end_decompress

        /* IDENTSTR goes here */

r_tmp   .req r12
rax     .req r12  // clobbered by do_sys

r_LENX  .req r11
r_ADRX  .req r10
r_EXP   .req r9

r_MHDR  .req r8
r_RELOC .req r8
r_FOLD  .req r8

rbx     .req r7

// r0,r1,r2,r3,r6,r7  used by _start+main

arg1 .req r0
arg2 .req r1
arg3 .req r2
arg4 .req r3
arg5 .req r4
arg6 .req r5

rdi .req r0
//rsi .req r1
rdx .req r2
rcx .req r3

  section MACHMAINZ
unfold:  // lr= &b_info;  sp/ fd,%ADRU,%LENU,%entry,%&Mach_header
#if DEBUG  /*{*/
        stmdb sp!,{TRACE_REGS}; mov r0,#2; bl trace
#endif  /*}*/
        mov rbx,lr  // &b_info:{sz_unc, sz_cpr, 4{byte}}, compressed_fold...}
        ldr rax,[rbx,#sz_unc]  // LENU.dynamic
        mov rdx,rbx
        sub rdx,rdx,r_MHDR  // LENU.static
        add rdx,rdx,rax
        str rdx,[sp,#2*NBPW]  // LENU

// Reserve space for input file and unfolded stub.
        mov arg2,rdx  // len
        mov arg6,#0  // offset
        mov arg5,#-1  // FD_ANON
        mov arg4,#MAP_PRIVATE|MAP_ANONYMOUS
        mov arg3,#PROT_READ|PROT_WRITE
        mov arg1,#0  // kernel chooses addr
        do_sys __NR_mmap
        str r0,[sp,#1*NBPW]  // ADRU
        sub r_ADRX,r_ADRX,r_MHDR  // offset(&l_info)
        add r_ADRX,r_ADRX,r0  // new &l_info

// Duplicate the input data.
        mov arg6,#0  // offset
        ldr arg5,[sp,#0*NBPW]  // fd
        mov arg4,#MAP_PRIVATE|MAP_FIXED
        mov arg3,#PROT_READ|PROT_WRITE
        sub arg2,rbx,r_MHDR  // len
        // mov arg1,r0  // same address
        do_sys __NR_mmap
#if DEBUG  /*{*/
        stmdb sp!,{TRACE_REGS}; mov r0,#3; bl trace
#endif  /*}*/

// Remember new f_exp region for PROT_EXEC.
        ldr rdx,[sp,#2*NBPW]  // LENU
        ldr rcx,[sp,#4*NBPW]  // &Mach_header
        add rdx,rdx,r0  // new last of unfolded
        sub r_RELOC,r0,rcx  // relocation constant = new - old
        add rcx,r_EXP,r_RELOC
        str rcx,[sp,#-NBPW]!  // P_10  new f_exp
        mov r0,rcx,lsr #PAGE_SHIFT
        mov r0,r0, lsl #PAGE_SHIFT  // page boundary below new f_exp
        str r0,[sp,#-NBPW]!  // P_11 address
        sub rdx,rdx,r0
        str rdx,[sp,#-NBPW]!  // P_12 length

// Unfold
        ldr rax,[rbx,#sz_unc]  // dstlen
        str rax,[sp,#-NBPW]!; mov arg4,sp  // P_13  &dstlen
        add arg3,rbx,r_RELOC  // dst= new unfold
        mov r_FOLD,arg3  // remember where to execute after unfold
        ldr arg5,[rbx,#b_method]
        ldr arg2,[rbx,#sz_cpr]  // srclen
        add arg1,rbx,#sz_b_info  // src
        str arg5,[sp,#-NBPW]!  // P_14  ABI: fifth arg goes on stack
#if DEBUG  /*{*/
        stmdb sp!,{TRACE_REGS}; mov r0,#4; bl trace
#endif  /*}*/
        blx r_EXP  // old f_exp; new f_exp lacks PROT_EXEC
        add sp,sp,#2*NBPW  // P_14,P_13  toss fifth arg and dstlen

// PROT_EXEC
        ldr  arg2,[sp],#NBPW  // P_12  length
        ldr  arg1,[sp],#NBPW  // P_11  addr
        ldr r_EXP,[sp],#NBPW  // P_10  new f_exp
        mov arg3,#PROT_READ|PROT_EXEC
        do_sys __NR_mprotect

// Use the copy.
// r9= f_exp; r10= ADRX; r11= LENX
// rsp/ fd,ADRU,LENU,%entry,&Mach_header
        bx r_FOLD

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
        ldr r0,[r4],#4; mov r1,#' '; bl trace_hex  @ next word
        subs r6,r6,#1; bgt L610

        subs r5,r5,#1; bgt L600

        mov r0,#'\n'; strb r0,[r2],#1
        sub r2,r2,sp  @ count
        mov r1,sp  @ buf
        mov r0,#2  @ FD_STDERR
        mov r7,#__NR_write
        swi 0
        add sp,sp,#TRACE_BUFLEN
        ldmia sp!,{TRACE_REGS}

trace_hex:  // In: r0=val, r1=punctuation before, r2=ptr; Uses: r3, ip
        strb r1,[r2],#1  @ punctuation
        mov r3,#4*(8 -1)  @ shift count
        adr ip,hex
L620:
        mov r1,r0,lsr r3
        and r1,r1,#0xf
        ldrb r1,[ip, r1]
        strb r1,[r2],#1
        subs r3,r3,#4; bge L620
        ret
hex:
        .ascii "0123456789abcdef"
#endif  /*}*/

execp:
        .ascii "executable_path="  // 16 bytes
L99:
        bkpt  // FIXME

main:  // lr= &f_exp;  r7= &argc; r6= -2*NBPW + _start
        mov r_EXP,lr  // &f_decompress
0:
        ldr r0,[r7],#NBPW
        cmp r0,#0; bne 0b  // past argv
0:
        ldr r0,[r7],#NBPW
        cmp r0,#0; bne 0b  // past envp
L30:
        ldr r0,[r7],#NBPW  // r0= *apple++
        cmp r0,#0; beq L99
        adr r1,execp; mov r_tmp,#16
L40:
        ldrb r2,[r0],#1
        ldrb r3,[r1],#1
        cmp r2,r3; bne L30  // mismatch ==> next apple[]
        subs r_tmp,r_tmp,#1; bne L40  // not entire prefix

        mov arg2,#0  // O_RDONLY
        do_sys __NR_open
        str r0,[sp,#-5*NBPW]!  // fd,%ADRU,%LENU,%entry,%&Mach_header

        mov r_MHDR,r6
        ldr r0,[r6],#4
        sub r_MHDR,r_MHDR,r0  // &Mach_header
        str r_MHDR,[sp,#4*NBPW]

        mov r_ADRX,r6
        ldr r0,[r6],#4
        sub r_ADRX,r_ADRX,r0  // &l_info

        sub r_LENX,r0,#2*4  // omit words before _start
        bl unfold
          /* { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...} */

/* vim:set ts=8 sw=8 et: */
