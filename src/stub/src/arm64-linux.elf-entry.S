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

sz_Elf64_Phdr= 56
sz_Elf64_Ehdr= 64
e_phnum= 16 + 2*2 + 4 + 3*8 + 4 + 2*2

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
#if DEBUG  //{
#define TRACE(arg) stp lr,x0,[sp,#-2*8]!; mov x0,arg; bl trace; ldr lr,[sp],#2*8
#else  //}{
#define TRACE(arg) /*empty*/
#endif  //}


        //.long sz_pack2  // placed there by ::pack3()
  section ELFMAINX
start_params:
        .long LENF  // end_decompress - (start_params -4)
        .long CPR0  //           cpr0 - (start_params -4)
mflg:
        .long MFLG  // MAP_{PRIVATE|ANONYMOUS}  // QNX vs linux
        .xword ADRM  // dst  for map
_start: .globl _start
////    brk #0  // DEBUG
/* Get some pages: enough
   to duplicate the entire compressed PT_LOAD, plus 1 page, located just after
   the brk() of the _un_compressed program.  The address is pre-calculated
   calculated by PackLinuxElf64arm::addLinkerSymbols().
*/

#if DEBUG  /*{*/
    brk #0
        TRACE(#0)
#endif  /*}*/

        adr x12,start_params -4  // &sz_pack2
        ldp w29,w10,[x12]  // w29= sz_pack2; w10= LENF
        ldp w11,w13,[x12,#2*4]  // w11= CPRO; w13= MFLG
        add x11,x11,x12  // cpr0
        ldr x0,[x12,#4*4]  // ADRM
        add x10,x10,x12  // end_decompress
        ldr w3,[x11,# sz_unc]
        sub x12,x12,w29,uxtw  // &our_Elf64_Ehdr
        add w1,w29,w3  // sz_pack2 + cpr0.sz_unc
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
        TRACE(#2)
#endif  /*}*/
        cmn x0,#4096
        bcs msg_SELinux
        mov x9,x12  // &our_Elf64_Ehdr
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
        str w1,[x0],#4  // MFLG at -4+ fold_begin
        adr x15,f_decompress
        mov lr,x0  // dst for unfolded code
        add x15,x15,x4  // relocated f_decompress

        ldrh w9,[x12,#e_phnum]
        mov w10,#sz_Elf64_Phdr
        mul w10,w10,w9
        add w10,w10,#sz_Elf64_Ehdr + sz_l_info + sz_p_info
        sub w9,w29,w10  // total compressed size
        add x10,x12,w10,uxtw  // &b_info
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
        TRACE(#3)
#endif  /*}*/
        br x15  // decompress folded code, return to *lr

#if DEBUG  /*{*/
TRACE_BUFLEN=1024
trace:  // DEFICIENCY: modifies condition code
        stp  x0, x1,[sp,#-32*8]!
        stp  x2, x3,[sp,# 2*8]
        stp  x4, x5,[sp,# 4*8]
        stp  x6, x7,[sp,# 6*8]
        stp  x8, x9,[sp,# 8*8]
        stp x10,x11,[sp,#10*8]
        stp x12,x13,[sp,#12*8]
        stp x14,x15,[sp,#14*8]
        stp x16,x17,[sp,#16*8]
        stp x18,x19,[sp,#18*8]
        stp x20,x21,[sp,#20*8]
        stp x22,x23,[sp,#22*8]
        stp x24,x25,[sp,#24*8]
        stp x26,x27,[sp,#26*8]
        stp x28,x29,[sp,#28*8]
        add  x1,lr,#4  // u_pc
        add  x2,sp,     #32*8 + 2*8  // u_sp
        stp  x1, x2,[sp,#30*8]

        ldr x1,[sp,#(1+ 32)*8]  // x1= u_x0
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

        ldp x16,x17,[sp,#16*8]
        ldp x18,x19,[sp,#18*8]
        ldp x20,x21,[sp,#20*8]
        ldp x22,x23,[sp,#22*8]
        ldp x24,x25,[sp,#24*8]
        ldp x26,x27,[sp,#26*8]
        ldp x28,x29,[sp,#28*8]
        ldp x30, x0,[sp,#30*8]
        sub  lr, lr,#4  // our lr

        ldp x14,x15,[sp,#14*8]
        ldp x12,x13,[sp,#12*8]
        ldp x10,x11,[sp,#10*8]
        ldp  x8, x9,[sp,# 8*8]
        ldp  x6, x7,[sp,# 6*8]
        ldp  x4, x5,[sp,# 4*8]
        ldp  x2, x3,[sp,# 2*8]
        ldp  x0, x1,[sp],#32*8
        ret

trace_hex2:
        mov w3,#2; b trace_hexwid
trace_hex:  // In: x0=val, w1=punctuation before, x2=ptr; Uses: w3, x7
        mov w3,#16  // ndigits
trace_hexwid:
        strb w1,[x2],#1; lsr w1,w1,#8; cbnz w1,trace_hexwid  // punctuation
        adr x7,hex
        sub w3,w3,#1
L620:
        cmp w3,#-1+ 8; mov w1,#'_'; strb w1,[x2]; cinc x2,x2,eq
        lsl w1,w3,#2  // 4 bits per hex digit
        lsr x1,x0,x1  // right justify next digit
        and x1,x1,#0xf
        ldrb w1,[x7, x1]
        strb w1,[x2],#1
        subs w3,w3,#1; bge L620
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
cpr0: .globl cpr0
        /* { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...} */

/*
vi:ts=8:et:nowrap
*/

