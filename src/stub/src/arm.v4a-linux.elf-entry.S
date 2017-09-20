/*  arm-linux.elf-entry.S -- Linux program entry point & decompressor (Elf binary)
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2017 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2017 Laszlo Molnar
*  Copyright (C) 2000-2017 John F. Reiser
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

#define bkpt .long 0xe7f001f0  /* reserved instr; Linux GNU eabi breakpoint */
sz_Elf32_Phdr =  8*4
  p_vaddr = 2*4
sz_Elf32_Ehdr = 13*4
  e_type= 16
    ET_DYN= 3
  e_phnum= 16 + 2*2 + 5*4 + 2*2

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8
sz_l_info= 12
sz_p_info= 12

PROT_READ=  1
PROT_WRITE= 2
PROT_EXEC=  4

MAP_FIXED=     0x10

PAGE_SHIFT= 12
PAGE_SIZE = -(~0<<PAGE_SHIFT)

__NR_exit =      1 + __NR_SYSCALL_BASE
__NR_write =     4 + __NR_SYSCALL_BASE
__NR_mmap64 = 0xc0 + __NR_SYSCALL_BASE

__ARM_NR_BASE  = 0xf0000 + __NR_SYSCALL_BASE
__ARM_NR_cacheflush =  2 + __ARM_NR_BASE

#ifndef DEBUG  /*{*/
#define DEBUG 0
#endif  /*}*/

#if DEBUG  //{
#define TRACE_REGS r0-r12,r14,r15
// sp (r13) is not included because the write-back might cause UNDEFINED behavior
// if the write-back register is not first or last.  The actual value of sp
// usually does not matter.  Just remember that lr (r14) and pc (r15) are stored
// one word closer to the stack pointer because r13 has been omitted.
#endif  //

        //.long sz_pack2  // placed there by ::pack3()
  section ELFMAINX
start_params:
        .long ADRM  // dst  for map
        .long LENF  // end_decompress - (start_params -4)
        .long CPR0  //           cpr0 - (start_params -4)
mflg:
        .long MFLG  // MAP_{PRIVATE|ANONYMOUS}  // QNX vs linux
_start: .globl _start
////    nop; bkpt
        bl main  // lr= &f_exp
f_exp:
#define LINUX_ARM_CACHEFLUSH 1

  section NRV_HEAD
        // empty
  section NRV_TAIL
        // empty

  section NRV2E
#include "arch/arm/v4a/nrv2e_d8.S"

  section NRV2D
#include "arch/arm/v4a/nrv2d_d8.S"

  section NRV2B
#include "arch/arm/v4a/nrv2b_d8.S"

#include "arch/arm/v4a/lzma_d.S"

  section ELFMAINY
end_decompress: .globl end_decompress

msg_SELinux:
        mov r2,#L71 - L70  // length
        adr r1,L70  // message text
        mov r0,#2  // fd stderr
#if defined(ARMEL_EABI4)  /*{*/
        mov r7,#__NR_write
        swi 0
#else  /*}{*/
        swi __NR_write
#endif  /*}*/
die:
        mov r0,#127
#if defined(ARMEL_EABI4)  /*{*/
        mov r7,#__NR_exit
        swi 0
#else  /*}{*/
        swi __NR_exit
#endif  /*}*/
L70:
        .asciz "PROT_EXEC|PROT_WRITE failed.\n"
L71:
        /* IDENTSTR goes here */

section ELFMAINZ
unfold:  // in: r11= &f_exp; lr= &O_BINFO
#if DEBUG  /*{*/
        stmdb sp!,{TRACE_REGS}; mov r0,#1; bl trace
#endif  /*}*/
        add r0,r11,#-4 + start_params - f_exp
        ldr r5,[r0]  @ sz_pack2; LENX
        ldr r6,[lr],#4  @ O_BINFO
        sub r7,r0,r5  @ &Elf_Ehdr  dynbase
        ldrh r0,[r7,#e_type]
        ldr r9,[r7,#p_vaddr + sz_Elf32_Phdr + sz_Elf32_Ehdr]  @ PT_LOAD[1].p_vaddr
        cmp r0,#ET_DYN
          addeq r9,r9,r7  @ brk(0)
        add r9,r9,#PAGE_SIZE
        sub r9,r9,#1
        ldr r8,[lr]  @ {fold_begin}.sz_unc
        sub r0,lr,r7  @ &eof - &Elf_Ehdr
        add r10,r8,r0  @ .sz_unc + sizeof(this_stub) == LENU
        add r10,r10,#2*4  @ 2-instr subr for mflg

        sub r0,sp,#8<<20  @ allow 8MB stack (qemu ET_DYN puts below Elf_Ehdr)
        cmp r0,#0xf7<<24  @ qemu wants for itself 0xf7000000 and above
          movhs r0,#0xf7<<24  @ min(0xf7000000, sp - 8M)
        sub r0,r0,r10  @ LENU must fit
        cmp r9,r0
          movhs r9,r0  @ move below

        mov r9,r9,lsr #PAGE_SHIFT  @ round down
        mov r9,r9,lsl #PAGE_SHIFT  @ new ADRU
        add r6,r6,r9  @ ADRX= O_BINFO + new_base(==ADRU)
        ldr r3,[r11, #mflg - f_exp]  // MAP_{PRIVATE|ANON}
        sub r11,r11,r7  @ offset(f_exp)
        add r11,r11,r9  @ new f_exp
        stmdb sp!,{r3,r5,r6,r7,r8,r9,r10,r11,r12}
        // MFLG, LENX,ADRX, dynbase,%fd, ADRU,LENU, f_exp,%entry

// alloc new pages via mmap
        mov r5,#0  // offset
        mvn r4,#0  // fd= -1; cater to *BSD for fd when MAP_ANON
        orr r3,r3,#MAP_FIXED
        mov r2,#PROT_READ | PROT_WRITE | PROT_EXEC
        mov r1,r10  @ LENU
        mov r0,r9  @ ADRU
#if defined(ARMEL_EABI4)  /*{*/
        mov r7,#__NR_mmap64
        swi 0
#else  /*}{*/
        swi __NR_mmap64
#endif  /*}*/
#if DEBUG  /*{*/
        stmdb sp!,{TRACE_REGS}; mov r0,#2; bl trace
#endif  /*}*/
        cmn r0,#4096
        bcs msg_SELinux

// copy to new pages
        ldr r2,[sp,#3*4]  @ dynbase
        sub r1,r1,r8  @ omit {fold_begin}.sz_unc
        add r12,r0,r1  @ end dst
        add r1, r2,r1  @ end src
cop2:  // overrun OK: dst is page aligned, fold_begin follows src
        ldmia r2!,{r3,r4,r5,r6,r7,r8,r9,r10}; cmp r2,r1
        stmia r0!,{r3,r4,r5,r6,r7,r8,r9,r10}; blo cop2

// decompress fold_begin and jump to it
        ldr r6,[sp],#4  @ MFLG
        ldr r0,mflg_subr
        ldr r1,mflg_subr+4
        orr r0,r0,r6
        str r0,[r12],#4
        str r1,[r12],#4
        ldr r5,[lr],#4  @ sz_unc
        ldr r1,[lr],#4  @ 2nd arg: .sz_cpr
        ldr r4,[lr],#4  @ b_method
        stmdb sp!,{r4,r5,r6,r12}  @ 5th arg, dstlen, MFLG, retaddr
        add r3,sp,#1*4  @ 4th arg: &dstlen  (used by lzma)
        mov r2,r12  @ 3rd arg: dst
        mov r0,lr  @ 1st arg: &payload
#if DEBUG  /*{*/
        stmdb sp!,{TRACE_REGS}; mov r0,#3; bl trace
#endif  /*}*/
        mov lr,pc; mov pc,r11  // decompress folded code [arm.v4a lacks 'blx']
        ldmia sp!,{r1,r2,r3,pc}  // toss 5th arg and dstlen; goto unfolded

mflg_subr:
        orr r3,r3,#0
        ret

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

main:
        mov r11,lr  // r11= &f_exp
        call unfold
o_binfo:
        .long O_BINFO  // .int4 offset of b_info for text
cpr0: .globl cpr0
        /* { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...} */
eof:

/* vim:set ts=8 sw=8 et: */
