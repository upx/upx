/*  aarch64-linux.elf-entry.S -- Linux program entry point & decompressor (Elf binary)
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
*  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
*
*  John F. Reiser
*  <jreiser@users.sourceforge.net>
*/

#include "arch/arm64/v8/macros.S"
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

MAP_PRIVATE= 2
MAP_FIXED=     0x10
MAP_ANONYMOUS= 0x20

PAGE_SHIFT= 16  // 64KiB PAGE_SIZE
PAGE_SIZE = -(~0<<PAGE_SHIFT)

// /usr/include/asm-generic/unistd.h
__NR_exit     = 0x5d  //  93
__NR_mmap     = 0xde  // 222
__NR_mprotect = 0xe2  // 226
__NR_munmap   = 0xd7  // 215
__NR_openat   = 0x38  //  56
__NR_write    = 0x40  //  64

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

//lr   .req x30
wLENU  .req w29  // un-folded fold_begin and upx_main
  xLENU  .req x29
xADRU  .req x28

wfd    .req w27  // file descriptor of /proc/self/exe
  xfd    .req x27
xPMASK .req x26  // PAGE_MASK
  // The above 4 registers are passed on stack to unfolded code.

xelfa  .req x25  // hi &Elf64_Ehdr
xfexp  .req x24  // de-compressor
  // xPMASK, xelfa, xfexp still are used here.

wLENC  .req w29  // compressed input  OVERLAPS wLENU
  xLENC  .req x29
xADRC  .req x28  // OVERLAPS xADRU
xADRM  .req x27  // conveneinence; OVERLAPS wfd, xfd
wLENM  .req w23
  xLENM  .req x23

xauxv  .req x22
wszuf  .req w21
  xszuf  .req x21
xFOLD  .req x20

// http://infocenter.arm.com/help/topic/com.arm.doc.ihi0055b/IHI0055B_aapcs64.pdf
// x18 is CLOBBERED: "The platform register"

wmeth  .req w4
xmeth  .req x4
ldst   .req x3
 dst   .req x2
lsrc   .req w1
 src   .req x0

#define call bl
#define callr blr
#define jr br

  section ELFMAINX
sz_pack2= .-4  // placed there by ::pack3()
mflg:
        .long MFLG  // MAP_{PRIVATE|ANONYMOUS}  // QNX vs linux
_start: .globl _start
#if DEBUG  /*{*/
    brk #0  // DEBUG  0xd4200000
        TRACE(#0)
#endif  /*}*/
        call main

f_exp:
f_decompress:

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
        .balign 4
mmapRW0:
        mov x5,#0  // offset
mmapRW:
        mov w2,#PROT_WRITE|PROT_READ
mmap:
        do_sys __NR_mmap
        ret

unfold:
        mov xFOLD,lr
// Reserve enough space to decompress the folded code of the stub
        ldr w0,  [xfexp,#sz_pack2 - f_exp]
        add xelfa,xfexp,#sz_pack2 - f_exp
        sub xelfa,xelfa,x0  // &Elf64_Ehdr of stub
        ldr w1,[xFOLD,#sz_unc + LBINFO - LxFOLD]  // .sz_unc of fold
        mov w4,#-1  // fd
        mov w3,#MAP_PRIVATE|MAP_ANONYMOUS
        mov x0,#0  // kernel chooses addr
        call mmapRW0
          PUSH2(x0,x1)  // ADRU, LENU
          PUSH2(xPMASK,xfd)
        mov xADRM,x0  // convenience
// Decompress folded code
        mov dst,x0  // ADRU
        PUSH1(x1); mov ldst,sp  // &slot on stack; lzma uses for EOF
        add src,    xFOLD,#sz_b_info + LBINFO - LxFOLD  // folded code
        ldr lsrc,  [XFOLD,#sz_cpr    + LBINFO - LxFOLD]
        ldrb wmeth,[xFOLD,#b_method  + LBINFO - LxFOLD]
    TRACE(#1)
        callr xfexp  // decompress it
        POP1(x8)  // discard lzma EOF

// Generate code to compute PAGE_MASK
        ldr x0,[xADRM,#1*4]  // 2nd instr
        rbit x1,xPMASK
        clz x1,x1  // # trailing 0's in xPMASK
        add x1,x1,#1
        lsl x1,x1,#2*5  // past two register numbers
        orr x0,x0,x1
        str x0,[xADRM,#1*4]  // modify 2nd instr: "add x1,x0,#1+ count"

// PROT_EXEC
        mov w2,#PROT_EXEC|PROT_READ
        ldr w1,[sp,3*NBPW]  // LENU
        mov x0,xADRM  // base
    TRACE(#2)
        do_sys __NR_mprotect

// Use the unfolded code
        ldr w0,[xFOLD, #LOBINFO - LxFOLD]  // O_BINFO
        ldr wLENC,[xfexp,#sz_pack2 - f_exp]
        add xADRC,xelfa,x0  // &b_info of compressed input data
        add x0,xADRM,#4*4  // jmp over get_page_mask()
        jr x0

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

proc_self_exe:
        .asciz "/proc/self/exe"; .balign 4

zfind:
        ldr x1,[x0],#NBPW; cbnz x1,zfind
        ret

L75:
        mov w2,#14  // length
        adr x1,proc_self_exe
        mov w0,#FD_stderr
        do_sys __NR_write
        mov w0,#127
        do_sys __NR_exit

main:
        mov xfexp,lr
        mov x0,sp; call zfind  // x0= &envp
        call zfind; mov xauxv,x0  // &Elf64_auxv

// set xPMASK by finding actual page size in Elf64_auxv
1:
        ldp x1,x2,[x0],#8
        cmp w1,#AT_PAGESZ; beq 2f
        cbnz w1,1b  // AT_NULL
        mov x2,#PAGE_SIZE  // default
2:
        neg xPMASK,x2  // save for folded code

// Open /proc/self/exe
        mov w0,#AT_FDCWD
        adr x1,proc_self_exe
        mov w2,O_RDONLY
        do_sys __NR_openat; mov wfd,w0
        cmp w0,#0; blt L75

        call unfold
LxFOLD:
LOBINFO:
        .int O_BINFO
LBINFO:
        // { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...}
/*
vi:ts=8:et:nowrap
*/

