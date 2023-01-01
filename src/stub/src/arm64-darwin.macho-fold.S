//  arm64-darwin.macho-fold.S -- linkage to C code to process Mach-O binary
//
//  This file is part of the UPX executable compressor.
//
//  Copyright (C) 2000-2023 John F. Reiser
//  All Rights Reserved.
//
//  UPX and the UCL library are free software; you can redistribute them
//  and/or modify them under the terms of the GNU General Public License as
//  published by the Free Software Foundation; either version 2 of
//  the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; see the file COPYING.
//  If not, write to the Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//  Markus F.X.J. Oberhumer              Laszlo Molnar
//  <markus@oberhumer.com>               <ezerotven+github@gmail.com>
//
//  John F. Reiser
//  <jreiser@users.sourceforge.net>
//

#define SIMULATE_ON_LINUX_EABI4 0

#if SIMULATE_ON_LINUX_EABI4  /*{*/
  #define LINUX_ARM_CACHEFLUSH 1  /* SIMULATE_ON_LINUX_EABI4 */
  #define ARMEL_EABI4 1           /* SIMULATE_ON_LINUX_EABI4 */
#else  /*}{ USUAL case */
  #define DARWIN_ARM_CACHEFLUSH 1
  #define ARMEL_DARWIN 1
#endif  /*}*/

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

NBPW= 8
#include "arch/arm64/v8/macros.S"

arg1 .req x0; arg1w .req w0
arg2 .req x1
arg3 .req x2
arg4 .req x3; arg4w .req w3
arg5 .req x4
arg6 .req x5
arg7 .req x6

sz_l_info = 12
sz_p_info = 12
sz_b_info = 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

#if SIMULATE_ON_LINUX_EABI4  /*{*/
__NR_brk      = 0xd6  // 214
__NR_close    = 0x39  //  57
__NR_exit     = 0x5d  //  93
__NR_mmap     = 0xde  // 222 __GI___mmap64
__NR_mprotect = 0xe2  // 226
__NR_munmap   = 0xd7  // 215
__NR_openat   = 0x38  //  56
__NR_pread    = 0x43  //  67
__NR_read     = 0x3f  //  63
__NR_write    = 0x40  //  64
#else  //}{  native darwin usual case
__NR_exit  =  1
__NR_read  =  3
__NR_write =  4
__NR_open  =  5
__NR_close =  6
__NR_brk   = 45

__NR_mmap     = 197
__NR_munmap   =  73
__NR_mprotect =  74
__NR_pread    = 153
#endif  /*}*/
FD_CWD = -100

_start: .globl _start  // ignored, but silence "cannot find entry symbol _start" from ld

// control just falls through, after this part and compiled C code
// are uncompressed.

fold_begin:
/* In:
  rsp/ fd,ADRU,LENU,%entry,&Mach_header,HOLE
  x20=r_ADRX= ADRX = &l_info
  x21=r_LENX= LENX
  x28=r_EXP= f_exp
*/
r_OSTK  .req x19
r_ADRX  .req x20
r_LENX  .req x21
r_EXP .req x28

#define t0w w9  /* scratch value */
        mov arg1,r_ADRX  // &{l_info; p_info; b_info}
        mov arg2,r_LENX
        ldr arg4w,[r_ADRX,#sz_unc + sz_l_info + sz_p_info]  // sz_unc of Mach_header
        mov t0w,#(1<<13)
        cmp arg4w,t0w
        csel arg4w,arg4w,t0w,hi  // at least 8KiB
        add arg4w,arg4w,# 0xf
        and arg4w,arg4w,#~0xf  // 16-byte align for stack pointer
        add arg7,sp,#4*NBPW
        mov r_OSTK,sp  // where to un-alloca
        sub sp,sp,arg4w,uxtw  // alloca
        mov arg3,sp  // Mach_header *tmp
        mov arg5,r_EXP
        adr arg6,f_unfilter
// upx_main(x0=&l_info, w1=len_cpr, x2=temp[sz_mhdr], w3=sz_mhdr, x4=f_exp, x5=f_unf, x6=mhdr **)
        bl upx_main  // OUT: x0= &Mach_thread_state of dyld; will be in temp mhdr[]

ARM64_ts_pc= 32*NBPW
        ldr r_ADRX,[x0,#ARM64_ts_pc]  // dyld.entry
        ldr r_EXP, [x0,#0]  // &hatch
        mov sp,r_OSTK  // un-alloca

        ldr w0,[sp,#0*NBPW]  // fd
        bl close

        ldr x0,[sp,#1*NBPW]  // ADRU
        ldr w1,[sp,#2*NBPW]  // LENU
        mov w8,#__NR_munmap
        mov lr,r_ADRX  // dyld.entry
        add sp,sp,#4*NBPW  // leave &Mach_header,HOLE
#if DEBUG  //{
        ldr w3,[r_EXP,#0]  // 1st instr
        ldr w4,[r_EXP,#4]  // 2nd instr
        TRACE(4)
#endif  //}
        br r_EXP  // goto hatch: syscall.munmap(ADRU,LENU); ret

f_unfilter:  // (char *ptr, uint len, uint cto, uint fid)
        ptr  .req x0
        len  .req w1
        cto  .req w2  // unused
        fid  .req w3

        t1   .req w2
        t2   .req w3

#ifndef FILTER_ID  /*{*/
#define FILTER_ID 0x52  /* little-endian */
#endif  /*}*/
        and fid,fid,#0xff
        cmp fid,#FILTER_ID  // last use of fid
        bne unf_ret  // no-op if not filter 0x52

        lsr len,len,#2  // word count
        cbz ptr,unf_ret  // no-op if ptr is NULL
        cbz len,unf_ret  // no-op if len is 0

top_unf:
        sub len,len,#1
        ldr t1,[ptr,len,uxtw #2]
        lsr t2,t1,#26
        cmp t2,   #045; bne tst_unf  // not 'bl' subroutine call
        and t2,t1,#077<<26  // all the non-displacement bits
        sub t1,t1,len  // convert to word-relative displacement
        bic t1,t1,#077<<26  // restrict to displacement field
        orr t1,t1,t2  // re-combine
        str t1,[ptr,len,uxtw #2]
tst_unf:
        cmp len,#0
        bne top_unf
unf_ret:
        ret

        .unreq ptr
        .unreq len
        .unreq cto
        .unreq fid

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
spin: .globl spin
        ret

        .globl exit
exit:
        do_sys __NR_exit

        .globl read
read:
        do_sys __NR_read; ret

        .globl write
write:
        do_sys __NR_write; ret

        .globl open
open:
#if SIMULATE_ON_LINUX_EABI4  //{
        mov arg3,#0  // O_RDONLY
        mov arg2,arg1
        mov arg1w,#FD_CWD
        do_sys __NR_openat
        ret
#else  //}{
        do_sys __NR_open; ret
#endif  //}

        .globl close
close:
        do_sys __NR_close; ret

        .globl brk
brk:
        do_sys __NR_brk; ret

        .globl munmap
munmap:
        do_sys __NR_munmap; ret

        .globl mprotect
mprotect:
        do_sys __NR_mprotect; ret

        .globl mmap
mmap:
        do_sys __NR_mmap; ret

        .globl pread
pread:
        do_sys __NR_pread; ret

        .globl bswap
bswap:
        mov w9,   #0xff
        orr w9,w9,#0xff<<16   // w9= 0x00ff00ff
        b bswap9
bswap0:
        ldr w2,[x0]           // r2= A B C D
        and w3,w9,w2          // r3= 0 B 0 D
        and w2,w9,w2,ror #24  // r2= 0 C 0 A
        orr w2,w2,w3,ror # 8  // r2= D C B A
        str w2,[x0],#4
bswap9:
        subs w1,w1,#4
        bge bswap0
        ret

/* vim:set ts=8 sw=8 et: */
