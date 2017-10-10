//  aarch64-linux.elf-fold.S -- linkage to C code to process Elf binary
//
//  This file is part of the UPX executable compressor.
//
//  Copyright (C) 2000-2015 John F. Reiser
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
//  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
//
//  John F. Reiser
//  <jreiser@users.sourceforge.net>
//

NBPW= 8
#define ARM_OLDABI 1
#include "arch/arm64/v8/macros.S"

sz_Elf64_Ehdr= 64
sz_Elf64_Phdr= 56
sz_l_info = 12
sz_p_info = 12
sz_b_info = 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8
sz_auxv= 2*NBPW

MAP_PRIVATE=   0x02
MAP_FIXED=     0x10

PROT_READ=     0x1

O_RDONLY=       0

AT_FDCWD= -100

PAGE_SHIFT= 12
PAGE_SIZE = -(~0<<PAGE_SHIFT)
PATH_MAX= 4096

#ifndef DEBUG  /*{*/
#define DEBUG 0
#endif  /*}*/

//@ control just falls through, after this part and compiled C code
//@ are uncompressed.

#define OVERHEAD 2048
#define MAX_ELF_HDR 512

get_page_size:
        mov x0,#~0  // all 1's; -1
        add x1,x0,#0  // constant is modified to (1+ count)
        lsl x0,x0,x1  // generate the mask
        ret

fold_begin:
////    brk #0  // DEBUG

// In:  sp/ argc,argv,...
xfexp  .req x28
xPMASK .req x27
xADRU  .req x26
wLENU  .req w25
wfd    .req w24
xauxv  .req x23
xelfa  .req x22
xADRM  .req x21
  xADRX  .req xADRM
wLENM  .req w20
  wLENX  .req wLENM
xLENM  .req x20
  xLENX  .req xLENM
xFOLD  .req x19  // unused
  xauxe  .req xFOLD  // past new auxv
xrelo  .req x17
wobinf .req w16

        ldr x7,[sp,#0]  // argc
        mov x0,sp  // current stack pointer

        mov x1,xauxv
.L20:  // skip auxv
        ldr x2,[x1],#2*NBPW
        cbnz x2,.L20  // AT_NULL

        sub x0,x1,#PATH_MAX // buffer for readlink
        sub x6,x0,#4  // space for copy of space3eq
        sub x1,x1,x6  // amount needed for argc,argv,env,auxv
        sub x0,x6,x1  // space for argc,argv,env,auxv
        sub x0,x0,#NBPW  // room for added env ptr
        and x0,x0,#~0<<4  // sp must be 16-byte aligned
        add x1,sp,#NBPW  // old_argv; avoid 0==argc impostor for terminator of argv
        mov sp,x0  // new_argc
        str x7,[x0],#NBPW  // argc

.Larg:  // copy argv
        ldr x2,[x1],#NBPW
        str x2,[x0],#NBPW
        cbnz x2,.Larg

.Lenv:  // copy env
        ldr x2,[x1],#NBPW
        str x2,[x0],#NBPW
        cbnz x2,.Lenv

        sub x4,x0,#NBPW  // added env ptr goes here
        str xzr,[x0],#NBPW  // terminator after added ptr
        mov x7,x0  // new &Elf64_auxv_t

        mov xauxv,x0  // new auxv
.Laux:  // copy auxv
        ldp x2,x3,[x1],#2*NBPW  // .a_type, .a_val
        stp x2,x3,[x0],#2*NBPW
        cbnz x2,.Laux  // AT_NULL
        mov xauxe,x0  // last of new auxv

        mov x1,x6
        str x1,[x4]  // new env ptr
        ldr w2,space3eq
        str w2,[x1],#4  // "   =" of new env var

        mov w2,#PATH_MAX-1  // buffer length and null terminator
        adr x0,proc_self_exe
        bl readlink
        cmn x0,#4096; bcs 0f  // error
        strb wzr,[x2,x0]  // null terminate pathname (x2 is old x1)
0:

/* Construct arglist for upx_main */
        mov x7,xPMASK
        mov x6,xelfa  // Elf64_Ehdr (reloc if ET_DYN and not pre-link)
          sub sp,sp,#MAX_ELF_HDR + OVERHEAD  // alloca
        adr x5,f_unfilter
        mov x4,xfexp  // &f_decompress
        mov x3,xauxv  // new &Elf64_auxv_t
        mov x2,sp  // ehdr
        mov w1,wLENX  // total size of compressed data
        mov x0,xADRX  // &b_info
        bl upx_main
          add sp,sp,#MAX_ELF_HDR + OVERHEAD  // un-alloca
        mov xfexp,x0  // entry address

// Map 1 page of /proc/self/exe so that munmap does not remove all references
        mov x5,#0  // offset
        mov w4,wfd  // fd
        mov w3,#MAP_PRIVATE
        mov w2,#PROT_READ
        mov x1,#PAGE_SIZE
        mov x0,#0  // addr
        bl mmap

        mov w0,wfd  // fd
        bl close

        mov lr,xfexp  // entry
        mov x0,xADRU
        mov w1,wLENU
        ldr x2,[xauxe,#NBPW - sz_auxv]  // Elf64_auxv_t[AT_NULL@.a_type].a_val
        mov w8,#__NR_munmap
#if DEBUG  /*{*/
        ldr w3,[x4,#0]  // 1st instr
        ldr w4,[x4,#4]  // 2nd instr
#define TRACE_REGS r0-r12,r14,r15
        stmdb sp!,{TRACE_REGS}; mov r0,#4; bl trace
#endif  /*}*/

        br x2  // goto escape hatch

space3eq:
        .ascii "   ="
proc_self_exe:
        .asciz "/proc/self/exe"
        .balign 4

f_unfilter:  // (char *ptr, uint len, uint cto, uint fid)
        ptr  .req x0
        len  .req x1
        lenw .req w1
        cto  .req w2  // unused
        fid  .req w3

        t1   .req w2
        t2   .req w3

#ifndef FILTER_ID  /*{*/
#define FILTER_ID 0x52  /* little-endian */
#endif  /*}*/
        and fid,fid,#0xff
        cmp fid,#FILTER_ID  // last use of fid
        bne unfret
        lsr len,len,#2  // word count
        cbz len,unfret
top_unf:
        sub len,len,#1
        ldr t1,[ptr,len,lsl #2]
        ubfx t2,t1,#26,#5
        cmp t2,#5; bne tst_unf  // not unconditional branch
        sub t2,t1,lenw  // word displ
        bfi t1,t2,#0,#26  // replace
        str t1,[ptr,len,lsl #2]
tst_unf:
        cbnz len,top_unf
unfret:
        ret

#if DEBUG  /*{*/
TRACE_BUFLEN=512
trace:
        str lr,[sp,#(-1+ 15)*NBPW]  // return pc; [remember: sp is not stored]
        mov r4,sp  // &saved_r0
        sub sp,sp,#TRACE_BUFLEN
        mov r2,sp  // output string

        mov r1,#'\n'; bl trace_hex  // In: r0 as label
        mov r1,#'>';  strb r1,[r2],#1

        mov r5,#6  // rows to print
L600:  // each row
        sub r0,r4,#TRACE_BUFLEN
        sub r0,r0,sp
        mov r0,r0,lsr #2; mov r1,#'\n'; bl trace_hex  // which block of 8

        mov r6,#4  // words per row
L610:  // each word
        ldr r0,[r4],#NBPW; mov r1,#' '; bl trace_hex  // next word
        subs r6,r6,#1; bgt L610

        subs r5,r5,#1; bgt L600

        mov r0,#'\n'; strb r0,[r2],#1
        sub r2,r2,sp  // count
        mov r1,sp  // buf
        mov r0,#2  // FD_STDERR
#if defined(ARMEL_EABI4)  /*{*/
        mov r7,#__NR_write
        swi 0
#else  /*}{*/
        swi __NR_write
#endif  /*}*/
        add sp,sp,#TRACE_BUFLEN
        ldmia sp!,{TRACE_REGS}

trace_hex:  // In: r0=val, r1=punctuation before, r2=ptr; Uses: r3, ip
        strb r1,[r2],#1  // punctuation
        mov r3,#4*(2*NBPW -1)  // shift count
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
        .unreq ptr
        .unreq len
        .unreq cto
        .unreq fid

__NR_SYSCALL_BASE= 0

__NR_exit     = 0x5d + __NR_SYSCALL_BASE  // 93
__NR_read     = 0x3f + __NR_SYSCALL_BASE  // 63
__NR_write    = 0x40 + __NR_SYSCALL_BASE  // 64
__NR_openat   = 0x38 + __NR_SYSCALL_BASE  // 56
__NR_close    = 0x39 + __NR_SYSCALL_BASE  // 57
__NR_unlinkat = 0x23 + __NR_SYSCALL_BASE  // 35
__NR_getpid   = 0xad + __NR_SYSCALL_BASE  // 172
__NR_brk      = 0xd6 + __NR_SYSCALL_BASE  // 214
__NR_readlink = 0x4e + __NR_SYSCALL_BASE  // 78

__NR_mmap     = 0xde + __NR_SYSCALL_BASE  // 222
__NR_mprotect = 0xe2 + __NR_SYSCALL_BASE  // 226
__NR_munmap   = 0xd7 + __NR_SYSCALL_BASE  // 215

        .globl my_bkpt
my_bkpt:
        brk #0
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

        .globl close
close:
        do_sys __NR_close; ret

        .globl getpid
getpid:
        do_sys __NR_getpid; ret

        .globl brk
brk:
        do_sys __NR_brk; ret

        .globl munmap
munmap:
        do_sys __NR_munmap; ret

        .globl mprotect
mprotect:
        do_sys __NR_mprotect; ret

bits_privanon= -4+ fold_begin  // entry stores: MAP_{PRIVATE|ANON}  QNX vs linux

mmap_privanon: .globl mmap_privanon
        ldr w6,bits_privanon  // r12 === ip
        mov x5,#0  // offset= 0
        mov x4,#-1  // fd= -1
        orr w3,w3,w6  // flags |= MAP_{PRIVATE|ANON}  [QNX vs Linux]
    // FALL THROUGH to mmap

        .globl mmap
mmap:
        do_sys __NR_mmap; ret

        .globl unlink
unlink:
        mov x2,#0  // flags as last arg
        mov x1,x0  // path
        mov w0,#AT_FDCWD
        mov w8,#__NR_unlinkat
        b svc_AT

        .globl readlink
readlink:
        mov w8,#__NR_readlink
        b svc_AT

        .globl open
open:
        mov w8,#__NR_openat
svc_AT:
        //mov x4,x3
        mov x3,x2  // mode
        mov x2,x1  // flags
        mov x1,x0  // fname
        mov x0,#AT_FDCWD
        svc #0; ret

        .globl __clear_cache
__clear_cache:
        hlt #0
        mov w2,#0
        do_sys 0; ret  // FIXME


#if DEBUG  /*{*/

div10: .globl div10
        mov x1,#10
        udiv x0,x0,x1
        ret
#endif  /*}*/

// vi:ts=8:et:nowrap

