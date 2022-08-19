/* arm64-darwin.macho-entry.S -- iPhone program entry point & decompressor (Elf binary)
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
#else  /*}{ USUAL case */
  #define DARWIN_ARM_CACHEFLUSH 1
  #define ARMEL_DARWIN 1
#endif  /*}*/

#ifndef DEBUG  //{
#define DEBUG 0
#define TRACE(arg) /*empty*/
#else  //}{
#define TRACE(arg) \
        stp lr,x0,[sp,#-2*NBPW]!; mov x0,arg; bl trace; \
        ldp lr,x0,[sp],#2*NBPW
#endif  //}

NBPW= 8
#include "arch/arm64/v8/macros.S"

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
FD_ANON= -1

PAGE_SHIFT= 12
PAGE_SIZE = -(~0<<PAGE_SHIFT)

__NR_SYSCALL_BASE = 0


#if SIMULATE_ON_LINUX_EABI4  /*{*/
__NR_mmap     = 0xde  // 222 __GI___mmap64
__NR_munmap   = 0xd7  // 215
__NR_mprotect = 0xe2  // 226
__NR_openat   = 0x38  //  56
__NR_pread    = 0x43  //  67
__NR_write    = 0x40  //  64
#else  //}{
__NR_exit =      1 + __NR_SYSCALL_BASE
__NR_write =     4 + __NR_SYSCALL_BASE
__NR_open =      5 + __NR_SYSCALL_BASE
__NR_mmap =    197 + __NR_SYSCALL_BASE
__NR_mprotect=  74 + __NR_SYSCALL_BASE
#endif  /*}*/

FD_CWD = -100

// DEBUG ONLY:
__ARM_NR_BASE  = 0xf0000 + __NR_SYSCALL_BASE
__ARM_NR_cacheflush =  2 + __ARM_NR_BASE

        //.long sz_pack2  // placed there by ::pack3()
//0:    .word 0b - &Mach64_header  // backwards distance to Mach64_header
//0:    .word 0b - l_info  // backwards distance to l_info

  section MACHMAINX
_start: .globl _start
        mov x2,sp  // &argc
    TRACE(#0)
        bl main
L20:
f_decompress:

  section NRV_HEAD
        // empty

  section NRV2E
#include "arch/arm64/v8/nrv2e_d32.S"

  section NRV2D
#include "arch/arm64/v8/nrv2d_d32.S"

  section NRV2B
#include "arch/arm64/v8/nrv2b_d32.S"

  section NRV_TAIL
        // empty

#include "arch/arm64/v8/lzma_d.S"

  section MACHMAINY
end_decompress: .globl end_decompress

        /* IDENTSTR goes here */

r_RELOC .req x23  // same as r_MHDR
r_MHDR  .req x22
r_LENX  .req x21
r_ADRX  .req x20
r_FOLD  .req x19

arg1 .req x0; arg1w .req w0
arg2 .req x1; arg2w .req w1
arg3 .req x2; arg3w .req w2
arg4 .req x3; arg4w .req w3
arg5 .req x4; arg5w .req w4
arg6 .req x5; arg6w .req w5

rdi .req x0
//rsi .req x1
rdx .req x2
rcx .req x3

rax .req x7; eax .req w7
rbx .req x27
r_EXP .req x28

  section MACHMAINZ
lsrc .req w1
ldst .req w3
ldstx .req x3

unfold:  // lr= &b_info;  sp/ fd,%ADRU,%LENU,%entry,%&Mach_header,HOLE
        mov rbx,lr  // &b_info:{sz_unc, sz_cpr, 4{byte}}, compressed_fold...}
        ldr eax,[rbx,#sz_unc]  // LENU.dynamic
        sub rdx,rbx,r_MHDR  // LENU.static
        add rdx,rdx,eax,uxtw
        str rdx,[sp,#2*NBPW]  // LENU

// Reserve space for input file and unfolded stub.
        mov arg2,rdx  // len
        mov arg6w,#0  // offset
        mov arg5w,#-1  // FD_ANON
        mov arg4w,#MAP_PRIVATE|MAP_ANONYMOUS
        mov arg3w,#PROT_READ|PROT_WRITE
        mov arg1,#0  // kernel chooses addr
        do_sys __NR_mmap
        str x0,[sp,#1*NBPW]  // ADRU
        sub r_ADRX,r_ADRX,r_MHDR  // offset(&l_info)
        add r_ADRX,r_ADRX,x0  // new &l_info

// Duplicate the input data.
        mov arg6,#0  // offset
        ldr arg5w,[sp,#0*NBPW]  // fd
        mov arg4w,#MAP_PRIVATE|MAP_FIXED
        mov arg3w,#PROT_READ|PROT_WRITE
        sub arg2,rbx,r_MHDR  // len
        // mov arg1,x0  // same address
        do_sys __NR_mmap

// Remember new f_exp region for PROT_EXEC.
        ldr rdx,[sp,#2*NBPW]  // LENU
        ldr rcx,[sp,#4*NBPW]  // &Mach_header
        add rdx,rdx,x0  // new last of unfolded
        sub r_RELOC,x0,rcx  // relocation constant = new - old
        add rcx,r_EXP,r_RELOC
        str rcx,[sp,#-4*NBPW]!  // P_10  new f_exp
        and x0,rcx,#~0<<PAGE_SHIFT  // page boundary below new f_exp
        str x0,[sp,#1*NBPW]  // P_11  address
        sub rdx,rdx,x0
        str rdx,[sp,#2*NBPW]  // P_12  length
    TRACE(#1)

// Unfold
        ldr eax,[rbx,#sz_unc]  // dstlen
        add arg4,sp,#3*NBPW  // P_13  &dstlen
        str rax,[sp,#3*NBPW]
        add arg3,rbx,r_RELOC  // dst= new unfold
        mov r_FOLD,arg3  // execute here
        ldr  eax,[rbx,#sz_cpr]  // srclen
        ldr arg5w,[rbx,#b_method]
        add arg1,rbx,#sz_b_info  // src
        mov arg2,rax  // srclen
        blr r_EXP  // old f_exp; new f_exp lacks PROT_EXEC

// PROT_EXEC
        ldr  arg2,[sp, #2*NBPW]  // P_12  length
        ldr  arg1,[sp, #1*NBPW]  // P_11  addr
        ldr r_EXP,[sp],#4*NBPW  // P_10  new f_exp
        mov arg3w,#PROT_READ|PROT_EXEC
    TRACE(#2)
        do_sys __NR_mprotect

// Use the copy.
// x20=r_ADRX= ADRX = &l_info; x21=r_LENX= LENX; x28=r_EXP= f_exp
// rsp/ fd,ADRU,LENU,%entry,&Mach_header,HOLE
        br r_FOLD

#if DEBUG  //{
TRACE_BUFLEN=1024
trace:  // preserves condition code (thank you, CBNZ) [if write() does!]
        stp  x0, x1,[sp,#-32*NBPW]!
        stp  x2, x3,[sp,# 2*NBPW]
        stp  x4, x5,[sp,# 4*NBPW]
        stp  x6, x7,[sp,# 6*NBPW]
        stp  x8, x9,[sp,# 8*NBPW]
        stp x10,x11,[sp,#10*NBPW]
        stp x12,x13,[sp,#12*NBPW]
        stp x14,x15,[sp,#14*NBPW]
        stp x16,x17,[sp,#16*NBPW]
        stp x18,x19,[sp,#18*NBPW]
        stp x20,x21,[sp,#20*NBPW]
        stp x22,x23,[sp,#22*NBPW]
        stp x24,x25,[sp,#24*NBPW]
        stp x26,x27,[sp,#26*NBPW]
        stp x28,x29,[sp,#28*NBPW]
        add  x1,lr,#4  // u_pc
        add  x2,sp,     #32*NBPW + 2*NBPW  // u_sp
        stp  x1, x2,[sp,#30*NBPW]

        ldr x1,[sp,#(1+ 32)*NBPW]  // x1= u_x0
        str x1,[sp]  // u_x0

        mov x4,sp  // &u_x0
        sub sp,sp,#TRACE_BUFLEN
        mov x2,sp  // output string

        mov w1,#'\n'; bl trace_hex  // In: r0 as label
        mov w1,#'>';  strb w1,[x2],#1

        mov w5,#10  // nrows to print
L600:  // each row
        add x1,sp,#TRACE_BUFLEN
        sub x0,x4,x1
        lsr x0,x0,#3; mov w1,#'\n'; bl trace_hex2  // which block of 4

        mov w6,#4  // 64-bit words per row
L610:  // each word
        ldr x0,[x4],#8; mov w1,#(' '<<8)|' '; bl trace_hex  // next word
        sub w6,w6,#1; cbnz w6,L610

        sub w5,w5,#1; cbnz w5,L600

        mov w0,#'\n'; strb w0,[x2],#1
        mov x1,sp  // buf
        sub x2,x2,x1  // count
        mov w0,#2  // FD_STDERR
        do_sys __NR_write
        add sp,sp,#TRACE_BUFLEN

        ldp x16,x17,[sp,#16*NBPW]
        ldp x18,x19,[sp,#18*NBPW]
        ldp x20,x21,[sp,#20*NBPW]
        ldp x22,x23,[sp,#22*NBPW]
        ldp x24,x25,[sp,#24*NBPW]
        ldp x26,x27,[sp,#26*NBPW]
        ldp x28,x29,[sp,#28*NBPW]
        ldp x30, x0,[sp,#30*NBPW]
        sub  lr, lr,#4  // our lr

        ldp x14,x15,[sp,#14*NBPW]
        ldp x12,x13,[sp,#12*NBPW]
        ldp x10,x11,[sp,#10*NBPW]
        ldp  x8, x9,[sp,# 8*NBPW]
        ldp  x6, x7,[sp,# 6*NBPW]
        ldp  x4, x5,[sp,# 4*NBPW]
        ldp  x2, x3,[sp,# 2*NBPW]
        ldp  x0, x1,[sp],#32*NBPW
        ret

trace_hex2:
        mov w3,#2; b trace_hexwid
trace_hex:  // In: x0=value, w1=punctuation before, x2=ptr; Uses: w3, x8
        mov w3,#16  // ndigits
trace_hexwid:  // In: x0= value; w1= punctuation; x2= ptr; w3= number of low-order digits
        strb w1,[x2],#1; lsr w1,w1,#8; cbnz w1,trace_hexwid  // prefix punctuation
        adr x8,hex
L620:
        sub w3,w3,#1  // number of less-significant digits
        lsl w1,w3,#2  // 4 bits per hex digit
        lsr x1,x0,x1  // right justify this digit
        and x1,x1,#0xf
        ldrb w1,[x8, x1]
        strb w1,[x2],#1
        sub w1,w3,#8; cbnz w1,0f; mov w1,#'_'; strb w1,[x2],#1  // 8-digit readability
0:
        cbnz w3,L620
        ret
hex:
        .ascii "0123456789abcdef"
#endif  //}

execp:
        .ascii "executable_path="  // 16 bytes
L99:
        brk #0

main:  // IN: x2= &argc; lr= &f_exp
        mov r_EXP,lr  // &f_decompress
0:
        ldr  x0,[x2],#NBPW
        cbnz x0,0b  // past argv
0:
        ldr  x0,[x2],#NBPW
        cbnz x0,0b  // past envp
L30:
        ldr x0,[x2],#NBPW  // x0= *apple++
        cbz x0,L99
        adr x1,execp; mov w3,#16
L40:
        ldrb w4,[x0],#1
        ldrb w5,[x1],#1
        sub w3,w3,#1
        cmp w4,w5; bne L30  // mismatch ==> next apple[]
        cbnz w3,L40  // not entire prefix

#if SIMULATE_ON_LINUX_EABI4  //{
        mov arg3,#0  // O_RDONLY
        mov arg2,arg1  // path
        mov arg1w,#FD_CWD
        do_sys __NR_openat
#else  //}{
        mov arg2,#0  // O_RDONLY
        //mov arg1,x0  // path
        do_sys __NR_open
#endif  //}
        str x0,[sp,#-6*NBPW]!  // fd,%ADRU,%LENU,%entry,%&Mach_header,HOLE

        adr x1,-2*4 + _start

        mov r_MHDR,x1
        ldr w0,[x1],#4
        sub r_MHDR,r_MHDR,w0,uxtw // &Mach_header
        str r_MHDR,[sp,#4*NBPW]

        mov r_ADRX,x1
        ldr w0,[x1],#4
        sub r_ADRX,r_ADRX,w0,uxtw  // &l_info

        sub r_LENX,x0,#2*4  // omit words before _start
        bl unfold
          /* { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...} */

/* vim:set ts=8 sw=8 et: */
