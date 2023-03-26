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

__NR_exit =      1 + __NR_SYSCALL_BASE
__NR_write =     4 + __NR_SYSCALL_BASE
__NR_mmap64 = 0xc0 + __NR_SYSCALL_BASE
__NR_mprotect =125 + __NR_SYSCALL_BASE
__NR_munmap =   91 + __NR_SYSCALL_BASE

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
//  .long offset(b_info)|(asl_delta>>12)  src for f_exp  // FIXME: not true?
//SO_INFO:
//  .long offset(.)  // detect relocation
//  .long offset(user DT_INIT)
//  .long offset(xct_off)  // lowest executable instr
//  .long offset(dst for f_exp)

        //.arch armv7-a
        //.syntax unified
        .thumb
        .thumb_func
        .type _start,%function
        .balign 4
_start: .globl _start  // in Thumb mode  (via PackLinuxElf32::pack3)
        bx pc; nop  // but switch to ARM until debugged
        .arm
        nop
#if 0|DEBUG  //{
        bkpt  // DEBUG
#endif  //}
        // argc,argv,envp, r3 convenience, r4-r7 callee-saved, lr ret_addr
        stmfd sp!,{r0,r1,r2, r3, r4,r5,r6,r7, lr}

        sub sp,sp,#2*NBPW  // space for ADRU, LENU
F_ADRU= 0 * NBPW
F_LENU= 1 * NBPW
F_ARGC= 2 * NBPW

        call4 L70
L70_ret:
foldi .req lr  // &fold_info
        mov arg6,#0  // cleanliness
        mvn arg5,#~-1  // fd= -1
        mov arg4,#MAP_PRIVATE|MAP_ANONYMOUS  // modes
        mov arg3,#PROT_READ|PROT_WRITE  // prot
        ldr arg2,[foldi, #sz_unc]  // dstlen
        str arg2,[sp, #F_LENU]
        mov arg1,#0  // addr (kernel chooses)
      PUSH {foldi}  // defend against lr clobbered by syscall
        do_sys7t __NR_mmap64; cmn r0,#4096; bcc 0f; bkpt; 0:
      POP {foldi}
        str r0,[sp, #F_ADRU]

        add arg4,sp,#F_LENU  // &dstlen
        mov arg3,r0  // dst
        ldr arg2,[foldi, #sz_cpr]  // srclen
        add arg1,foldi,#sz_b_info  // src
        call4 f_decompress  // includes cache flush

        ldr arg1,[sp, #F_ADRU]
        ldr arg2,[sp, #F_LENU]
        mov arg3,#PROT_EXEC|PROT_READ
        do_sys7t __NR_mprotect

        adr r0,_start -4 *NBPW  // &SO_INFO
        add r1,sp,#F_ARGC
        ldr pc,[sp, #F_ADRU]  // invoke folded code

f_decompress:
#define LINUX_ARM_CACHEFLUSH 1
#include "arch/arm/v4a/nrv2b_d8.S"

//%esp:
//  MATCH_04  ptr unfolded_code
//  MATCH_10  len unfolded_code
//  MATCH_00  argc
//  MATCH_01  argv
//  MATCH_07  envp

// IDENTSTR goes here

  section ELFMAINZ
L70:
        call4 L70_ret
fold_info:
//  b_info (sz_unc, sz_cpr, method) of folded code (C-language, etc.)

/* vim:set ts=8 sw=8 et: */
