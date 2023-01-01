//  arm-darwin.macho-fold.S -- linkage to C code to process Mach-O binary
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

#ifndef DEBUG  //{
#define DEBUG 0
#endif  //}
#define TRACE_REGS r0-r12,r14,r15

NBPW= 4
#include "arch/arm/v5a/macros.S"

arg1 .req r0
arg2 .req r1
arg3 .req r2
arg4 .req r3
arg5 .req r4
arg6 .req r5
arg7 .req r6

sz_l_info = 12
sz_p_info = 12
sz_b_info = 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

#if SIMULATE_ON_LINUX_EABI4  /*{*/
__NR_brk      =  45 + __NR_SYSCALL_BASE  // 0x2d
__NR_close    =   6 + __NR_SYSCALL_BASE  // 0x06
__NR_exit     =   1 + __NR_SYSCALL_BASE  //  ??
__NR_mmap     = 192 + __NR_SYSCALL_BASE  // 0xc0
__NR_mprotect = 125 + __NR_SYSCALL_BASE  // 0x7d
__NR_munmap   =  91 + __NR_SYSCALL_BASE  // 0x5b
__NR_open     =   5 + __NR_SYSCALL_BASE  //  ??
__NR_openat   = 322 + __NR_SYSCALL_BASE  // 0x142
__NR_pread    = 180 + __NR_SYSCALL_BASE  // 0xb4
__NR_read     =   3 + __NR_SYSCALL_BASE  // 0x03
__NR_write    =   4 + __NR_SYSCALL_BASE  // 0x04
#else  //}{  native darwin usual case
__NR_brk      =  45 + __NR_SYSCALL_BASE
__NR_close    =   6 + __NR_SYSCALL_BASE
__NR_exit     =   1 + __NR_SYSCALL_BASE
__NR_mmap     = 197 + __NR_SYSCALL_BASE
__NR_mprotect =  74 + __NR_SYSCALL_BASE
__NR_munmap   =  73 + __NR_SYSCALL_BASE
__NR_open     =   5 + __NR_SYSCALL_BASE
__NR_pread    = 153 + __NR_SYSCALL_BASE
__NR_read     =   3 + __NR_SYSCALL_BASE
__NR_write    =   4 + __NR_SYSCALL_BASE
#endif  /*}*/

_start: .globl _start  // ignored, but silence "cannot find entry symbol _start" from ld

// control just falls through, after this part and compiled C code
// are uncompressed.

fold_begin:
/* In:
  r9= f_exp; r10= ADRX; r11= LENX
  rsp/ fd,ADRU,LENU,%entry,&Mach_header
*/
r_t0   .req r12  // scratch
r_LENX .req r11
r_ADRX .req r10
r_EXP  .req r9
r_OSTK .req r8
#if DEBUG  /*{*/
        stmdb sp!,{TRACE_REGS}; mov r0,#0x11; bl trace
#endif  /*}*/

        ldr   arg4,[r_ADRX,#sz_unc + sz_l_info + sz_p_info]  // sz_unc of Mach_header
        mov        r_t0,#(1<<13)
        cmp   arg4,r_t0
        movlo arg4,r_t0  // at least 8KiB
        mov r_OSTK,sp  // where to un-alloca
        sub sp,sp,arg4  // alloca
        mov arg3,sp  // temp[sz_mhdr]
        mov arg2,r_LENX
        mov arg1,r_ADRX  // &{l_info; p_info; b_info}

        add arg7,r_OSTK,#4*NBPW  // & &Mach_header
        adr arg6,f_unfilter
        mov arg5,r_EXP
        stmdb sp!,{arg5,arg6,arg7}  // ABI: only arg1,arg2,arg3,arg4 in registers
#if DEBUG  /*{*/
        stmdb sp!,{TRACE_REGS}; mov r0,#0x12; bl trace
#endif  /*}*/
// upx_main(r0=&l_info, r1=len_cpr, r2=temp[sz_mhdr], r3=sz_mhdr, r4=f_exp, r5=f_unf, r6=mhdr **)
        bl upx_main  // OUT: r0= &Mach_thread_state of dyld; will be in temp mhdr[]

ARM_ts_pc= 14*NBPW
        ldr r_ADRX,[r0,#ARM_ts_pc]  // dyld.entry
        ldr r_EXP, [r0,#0]  // &hatch
        mov sp,r_OSTK  // un-alloc

        ldr r0,[sp,#0*NBPW]  // fd
        bl close

        ldr r0,[sp,#1*NBPW]  // ADRU
        ldr r1,[sp,#2*NBPW]  // LENU
        mov r7,#__NR_munmap
        mov lr,r_ADRX  // dyld.entry
        add sp,sp,#4*NBPW  // leave &Mach_header
#if DEBUG  /*{*/
        stmdb sp!,{TRACE_REGS}; mov r0,#0x13; bl trace
#endif  /*}*/
        bx r_EXP  // goto hatch: syscall.munmap(ADRU,LENU); ret

f_unfilter:  // (char *ptr, uint len, uint cto, uint fid)
        ptr  .req r0
        len  .req r1
        cto  .req r2  // unused
        fid  .req r3

        t1   .req r2
        t2   .req r3

#ifndef FILTER_ID  /*{*/
#define FILTER_ID 0x50  /* little-endian */
#endif  /*}*/
        and fid,fid,#0xff
        cmp fid,#FILTER_ID  // last use of fid
        movne pc,lr  // no-op if not filter 0x50

        movs  len,len,lsr #2  // word count
        cmpne ptr,#0
        moveq pc,lr  // no-op if either len or ptr is 0

top_unf:
        sub len,len,#1
        ldr t1,[ptr,len,lsl #2]
        and t2,t1,#0x0f<<24
        cmp t2,   #0x0b<<24; bne tst_unf  // not 'bl' subroutine call
        and t2,t1,#0xff<<24  // all the non-displacement bits
        sub t1,t1,len  // convert to word-relative displacement
        bic t1,t1,#0xff<<24  // restrict to displacement field
        orr t1,t1,t2  // re-combine
        str t1,[ptr,len,lsl #2]
tst_unf:
        cmp len,#0
        bne top_unf
        mov pc,lr

        .unreq ptr
        .unreq len
        .unreq cto
        .unreq fid

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
        do_sys __NR_open; ret

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
#if SIMULATE_ON_LINUX_EABI4  /*{*/
        stmdb sp!,{r4,r5}
        ldr r5,[sp,#3*4]  // off_t
        ldr r4,[sp,#2*4]  // fd
        movs r12,r5,lsl #(32-12); bne mmap_frag  // lo 12 bits of offset
        mov r5,r5,lsr #12  // convert to page number (avoid 64-bit argument)
        do_sys __NR_mmap
mmap_ret:
        ldmia sp!,{r4,r5}
        ret
mmap_frag:
EINVAL=22
        mov r0,#-EINVAL  // offset not a multiple of page size
        b mmap_ret
#else  /*}{ USUAL case */
        mov ip,sp
        stmdb sp!,{r4,r5,r6}
        ldmia ip ,{r4,r5 /*,r6*/}
        mov r6,#0  // XXX: convert 32-bit unsigned off_t to 64-bits
        do_sys __NR_mmap
        ldmia sp!,{r4,r5,r6}
        ret
#endif  /*}*/

        .globl pread
pread:
#if SIMULATE_ON_LINUX_EABI4  /*{*/
        stmdb sp!,{r4,r5}  // EABI4 wants 64-bit off_t in even,odd register pair
        mov r4,r3  //            32-bit off_t
        mov r5,#0  // hi bits of 64-bit off_t
        do_sys __NR_pread
        ldmia sp!,{r4,r5}
        ret
#else  /*}{ USUAL case */
        str r4,[sp,#-4]!  // PUSH r4
        mov r4,#0   // convert 32-bit unsigned off_t in r3 to 64 bits in (r3,r4)
        do_sys __NR_pread
        ldr r4,[sp],#4  // POP r4
        ret
#endif  /*}*/

        .globl bswap
bswap:
        mov ip,   #0xff
        orr ip,ip,#0xff<<16   // ip= 0x00ff00ff
        b bswap9
bswap0:
        ldr r2,[r0]           // r2= A B C D
        and r3,ip,r2          // r3= 0 B 0 D
        and r2,ip,r2,ror #24  // r2= 0 C 0 A
        orr r2,r2,r3,ror # 8  // r2= D C B A
        str r2,[r0],#4
bswap9:
        subs r1,r1,#4
        bge bswap0
        ret

bad__udivsi3:
        bkpt  // bad__udivsi3
__udivsi3: .globl __udivsi3
        cmp r1,#10
        bne bad__udivsi3
div10: .globl div10
        mov ip,r0  @ extra copy used at end
        sub r1,r1,r1  @ hi

        mov r2,r0  @ copy lo
        adds r0,r0,r0,lsl #3   @ 9*lo
        adc  r1,r1,r1,lsl #3   @ 9*hi + C
        add  r1,r1,r2,lsr #(32 - 3)  @ bits shifted from lo to hi

        mov r2,r0  @ copy lo
        adds r0,r0,r0,lsl #4
        adc  r1,r1,r1,lsl #4
        add  r1,r1,r2,lsr #(32 - 4)  @ * 0x99

        mov r2,r0  @ copy lo
        adds r0,r0,r0,lsl #8
        adc  r1,r1,r1,lsl #8
        add  r1,r1,r2,lsr #(32 - 8)  @ * 0x9999

        mov r2,r0  @ copy lo
        adds r0,r0,r0,lsl #16
        adc  r1,r1,r1,lsl #16
        add  r1,r1,r2,lsr #(32 - 16)  @ * 0x99999999

        subs r0,r0,ip,lsl #(32 - 1)  @ - * 0x80000000
        sbc  r1,r1,ip,lsr #1         @   * 0x19999999

        adds r0,r0,ip
        adc  r0,r1,#0  @ * 0x0.1999999a
        ret

/* vim:set ts=8 sw=8 et: */
