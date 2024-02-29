/*  aarch64-linux.elf-entry.S -- Linux program entry point & decompressor (Elf binary)
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
*  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
*
*  John F. Reiser
*  <jreiser@users.sourceforge.net>
*/

#include "arch/arm64/v8/macros.S"
#define bkpt  brk #0
NBPW= 8

sz_Elf64_Phdr= 56
sz_Elf64_Ehdr= 64
e_phnum= 16 + 2*2 + 4 + 3*NBPW + 4 + 2*2

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8
sz_l_info= 12
sz_p_info= 12
sz_o_binfo= 4

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

AT_NULL= 0  // <elf.h>
AT_PAGESZ= 6
a_type= 0
a_val= NBPW
sz_auxv= 2*NBPW

AT_FDCWD= -100  // <fcntl.h>
O_RDONLY=  0
FD_stderr= 2

PROT_READ=  1
PROT_WRITE= 2
PROT_EXEC=  4

MAP_SHARED=  1
MAP_PRIVATE= 2
MAP_FIXED=     0x10
MAP_ANONYMOUS= 0x20

PAGE_SHIFT= 16  // 64KiB PAGE_SIZE
PAGE_SIZE = -(~0<<PAGE_SHIFT)

// /usr/include/asm-generic/unistd.h
__NR_close    = 0x39  //  57
__NR_exit     = 0x5d  //  93
__NR_memfd_create= 0x117  // 279
__NR_mmap     = 0xde  // 222
__NR_mprotect = 0xe2  // 226
__NR_munmap   = 0xd7  // 215
__NR_openat   = 0x38  //  56
__NR_write    = 0x40  //  64

arg1  .req x0
arg1w .req w0
arg2  .req x1
arg2w .req w1
arg3  .req x2
arg3w .req w2
arg4  .req x3
arg4w .req w3
arg5  .req x4
arg5w .req w4
arg6  .req x5
arg6w .req w5

#ifndef DEBUG  /*{*/
#define DEBUG 0
#endif  /*}*/
#if DEBUG  //{
#define TRACE(arg) \
        stp lr,x0,[sp,#-2*NBPW]!; mov x0,arg; bl trace; \
        ldp lr,x0,[sp],#2*NBPW
#else  //}{
#define TRACE(arg) /*empty*/
#endif  //}

//lr    .req x30
//fp    .req x29
wLENU  .req w29  // un-folded fold_begin and upx_main
  xLENU  .req x29
xADRU  .req x28
xADRC  .req x28  // OVERLAPS xADRU

wLENC   .req w29
  xLENC .req x29

mfd    .req w27  // file descriptor from memfd_create
  xfd    .req x27
xPMASK .req x26  // PAGE_MASK
  // The above 4 registers are passed on *stack* to unfolded code.

xelfa  .req x25  // hi &Elf64_Ehdr
  // xPMASK, xelfa still are used here.

xauxv  .req x22
wszuf  .req w21
  xszuf  .req x21
xFOLD  .req x20
wPrivAnon .req w19

// http://infocenter.arm.com/help/topic/com.arm.doc.ihi0055b/IHI0055B_aapcs64.pdf
// x18 is CLOBBERED: "The platform register"


#define call bl
#define callr blr
#define jr br

  section ELFMAINX
sz_pack2= .-4  // placed there by ::pack3()
 mflg:
         .long MFLG  // MAP_{PRIVATE|ANONYMOUS}  // QNX vs linux
_start: .globl _start
#if DEBUG  /*{*/
    bkpt  // DEBUG  0xd4200000
        TRACE(#0)
#endif  /*}*/
        stp x0,x1,[sp,#-2*NBPW]!  // ABI: -static crt0 might pass data in x0,x1
        ldr wPrivAnon,mflg

        add x0,sp,#(1+ 2)*NBPW
        call zfind  // avoid feint of 0==argc;  out: x0= &envp
        call zfind; mov xauxv,x0  // &Elf64_auxv

        sub sp,sp,#4*NBPW  // space for PMASK, SZPK2, ADRU, LENU
F_PMASK= 0 * NBPW
F_SZPK2= 1 * NBPW
F_ADRU= 2 * NBPW
F_LENU= 3 * NBPW

// set xPMASK by finding actual page size in Elf64_auxv
1:
        ldp x1,x2,[x0],#2*NBPW
        cmp w1,#AT_PAGESZ; beq 2f
        cbnz w1,1b  // AT_NULL
        mov x2,#PAGE_SIZE  // default
2:
        neg xPMASK,x2  // save for folded code
        str xPMASK,[sp,#F_PMASK]

        call main
L00:
        mov xFOLD,lr

        mov w1,#0  // flags
        add x0,xFOLD,#strupx - LxFOLD
        do_sys __NR_memfd_create; mov mfd,w0

// alloca() for de-compressed stub
        ldr w0,[xFOLD,#sz_unc + LBINFO - LxFOLD]  // .sz_unc of fold
        mov fp,sp
        sub x0,fp,x0
        and x0,x0,#-2*NBPW
        mov sp,x0

src    .req x0
lsrc   .req w1
dst    .req x2
ldst   .req x3
xmeth  .req x4
wmeth  .req w4

// Decompress folded code of this stub
        mov dst,x0  // ADRU
        PUSH1(x1); mov ldst,sp  // &slot on stack; lzma uses for EOF
        add src,    xFOLD,#sz_b_info + LBINFO - LxFOLD  // folded code
        ldr lsrc,  [XFOLD,#sz_cpr    + LBINFO - LxFOLD]
        ldrb wmeth,[xFOLD,#b_method  + LBINFO - LxFOLD]
    TRACE(#1)
        call f_expand  // decompress it
        POP1(x8)  // discard lzma EOF

    .unreq src
    .unreq lsrc
    .unreq dst
    .unreq ldst
    .unreq xmeth
    .unreq wmeth

        str xPMASK,[sp]  // propagate PAGE_MASK

// Write de-compressed stub
        ldr w2,[xFOLD,#sz_unc + LBINFO - LxFOLD]  // .sz_unc of fold
        mov x1,sp
        mov w0,mfd
        do_sys __NR_write
        mov sp,fp

        mov arg6,#0  // beginning of file
        mov arg5w,mfd
        mov arg4w,#MAP_SHARED  // modes
        mov arg3w,#PROT_READ|PROT_EXEC  // FIXME: add PROT_WRITE for DEBUG only
        ldr arg2w,[xFOLD,#sz_unc + LBINFO - LxFOLD]
        str arg2,[sp,#F_LENU]
        mov arg1,#0  // addr (kernel chooses)
        do_sys __NR_mmap; str x0,[sp,#F_ADRU]; mov xADRU,x0

        mov arg1w,mfd
        do_sys __NR_close

// Use the unfolded stub
        ldr wLENC,[sp,#F_SZPK2]
        ldr w1,[xFOLD, #LOBINFO - LxFOLD]  // O_BINFO
        add lr,xADRU,#4 + NBPW  // jmp over data
        sub wLENC,wLENC,w1
        str xLENC,[sp,#F_SZPK2]
        add xADRC,xelfa,x1  // &b_info of compressed input data
        jr lr  // goto unfolded stub

zfind:
        ldr x1,[x0],#NBPW; cbnz x1,zfind
        ret

f_expand:
#define NO_METHOD_CHECK 1
#include "arch/arm64/v8/nrv2b_d32.S"

  section ELFMAINY
end_decompress: .globl end_decompress

        /* IDENTSTR goes here */

  section ELFMAINZ
        .balign 4
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
        mov w0,#FD_stderr
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

        .balign 4
main:
lr .req x30
        add xelfa,lr,#sz_pack2 - L00  // &sz_pack2
        ldr w0,[xelfa]
        sub xelfa,xelfa,w0,uxtw
        str x0,[sp,#F_SZPK2]
        callr lr
LxFOLD:
strupx:
        .asciz "upx"
        .balign 4
LOBINFO:
        .int O_BINFO
LBINFO:
        // { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...}
/*
vaddi:ts=8:et:nowrap
*/

