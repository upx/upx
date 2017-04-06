/*  aarch64-linux.elf-entry.S -- Linux program entry point & decompressor (Elf binary)
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2015 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2015 Laszlo Molnar
*  Copyright (C) 2000-2015 John F. Reiser
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
*  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
*
*  John F. Reiser
*  <jreiser@users.sourceforge.net>
*/

#include "arch/arm64/v8/macros.S"

sz_Elf64_Ehdr= 64
sz_Elf64_Phdr= 56
e_phnum= 56

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

__NR_exit =   93
__NR_write =  64
__NR_mmap64 = 222

__ARM_NR_cacheflush =  255  // FIXME

#ifndef DEBUG  /*{*/
#define DEBUG 0
#endif  /*}*/

        //.long sz_pack2  // placed there by ::pack3()
  section ELFMAINX
start_params:
        .long LENF  // end_decompress - (start_params -4)
        .long CPR0  //           cpr0 - (start_params -4)
mflg:
        .long MFLG  // MAP_{PRIVATE|ANONYMOUS}  // QNX vs linux
        .xword ADRM  // dst  for map
_start: .globl _start
    brk #0
/* Get some pages: enough
   to duplicate the entire compressed PT_LOAD, plus 1 page, located just after
   the brk() of the _un_compressed program.  The address is pre-calculated
   calculated by PackLinuxElf64arm::addLinkerSymbols().
*/

#if DEBUG  /*{*/
#define TRACE_REGS r0-r12,r14,r15
// sp (r13) is not included because the write-back might cause UNDEFINED behavior
// if the write-back register is not first or last.  The actual value of sp
// usually does not matter.  Just remember that lr (r14) and pc (r15) are stored
// one word closer to the stack pointer because r13 has been omitted.

        stmdb sp!,{TRACE_REGS}; mov r0,#1; bl trace
#endif  /*}*/

        adr x12,start_params -4  // &sz_pack2
        ldp w1,w10,[x12]  // w1= sz_pack2; w10= LENF
        ldp w11,w13,[x12,#2*4]  // w11= CPRO; w13= MFLG
        add x11,x11,x12  // cpr0
        add x10,x10,x12  // end_decompress
        ldr w3,[x11,# sz_unc]
        sub x9,x12,x1  // &our_Elf64_Ehdr
        add w1,w1,w3  // sz_pack2 + cpr0.sz_unc
        ldr x0,[x12,#4*4]  // ADRM
        add x1,x1,# PAGE_SIZE
  section LUNMP000
        // unused
  section LUNMP001
        // unused
  section ELFMAINXu
        PUSH4(x0,x1,x2,x13)  // ADRU, LENU, space for sz_unc, MFLG
SP_MFLG= 4*8
D_sz_unc=2*8  // stack displacement to sz_unc
        mov w2,#PROT_READ | PROT_WRITE | PROT_EXEC
        orr w3,w13,#MAP_FIXED  // MFLG: MAP_{PRIVATE|ANON}
        mov x5,#0  // offset= 0
        mov w4,#-1  // fd= -1; cater to *BSD for fd when MAP_ANON
        do_sys __NR_mmap64
#if DEBUG  /*{*/
        stmdb sp!,{TRACE_REGS}; mov r0,#2; bl trace
#endif  /*}*/
        cmn x0,#4096
        bcs msg_SELinux
copy_cl:  // copy decompressor and folded code
          add x5,x0,#64;  // FIXME(memcheck) dc zva,x5  // clear FOLLOWING 64-byte line
        ldp x1,x2,[x9],#2*8
        ldp x3,x4,[x9],#2*8
        stp x1,x2,[x0],#2*8
        stp x3,x4,[x0],#2*8

        ldp x1,x2,[x9],#2*8
        ldp x3,x4,[x9],#2*8
        stp x1,x2,[x0],#2*8
        stp x3,x4,[x0],#2*8
        cmp x9,x10
          sub x5,x9,#64; // FIXME(memcheck) dc cvau,x5  // clean 64-byte line
        blo copy_cl  // all source lines

        sub x4,x0,x9  // relocation amount
        ldr w1,[sp,#SP_MFLG]
        adr x15,f_decompress
        str w1,[x0],#4  // MFLG at -4+ fold_begin
        mov lr,x0  // dst for unfolded code
        add x15,x15,x4  // relocated f_decompress

        ldr w7,o_binfo
        ldr w9,[x12]  // sz_pack2
        sub w9,w9,w7
        sub x10,x12,x9  // &b_info
        add x10,x10,x4  // relocated &b_info

        ldr w4,[x11,# b_method ]  // 5th param (whole word: endian issues!)
D_stm1=0*8
        ldr  w3,[x11,# sz_unc]
        add  x0, x11,# sz_b_info
        ldr  w1,[x11,# sz_cpr ]
        mov  x2,lr  // dst
        str  w3,[sp,#D_stm1 + D_sz_unc]  // sz_unc; lzma needs for EOF
        add  x3, sp,#D_stm1 + D_sz_unc   // &sz_unc
#if DEBUG  /*{*/
        stmdb sp!,{TRACE_REGS}; mov r0,#3; bl trace
#endif  /*}*/
        br x15  // decompress folded code, return to *lr

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
        do_sys __NR_write
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

f_decompress:
#define LINUX_ARM_CACHEFLUSH 1

  section NRV_HEAD
        // empty
  section NRV_TAIL
        // empty

  section NRV2E
#include "arch/arm64/v8/nrv2e_d32.S"

  section NRV2D
#include "arch/arm64/v8/nrv2d_d32.S"

  section NRV2B
#include "arch/arm64/v8/nrv2b_d32.S"

#include "arch/arm64/v8/lzma_d.S"

  section ELFMAINY
end_decompress: .globl end_decompress

msg_SELinux:
        mov w2,#L71 - L70  // length
        adr x1,L70  // message text
        mov w0,#2  // fd stderr
        do_sys __NR_write
die:
        mov w0,#127
        do_sys __NR_exit
L70:
        .asciz "PROT_EXEC|PROT_WRITE failed.\n"
L71:
        /* IDENTSTR goes here */

  section ELFMAINZ
o_binfo:
        .word O_BINFO
cpr0: .globl cpr0
        /* { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...} */

/*
vi:ts=8:et:nowrap
*/

