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

#define ARM_OLDABI 1
#include "arch/arm64/v8/macros.S"
#define bkpt .long 0xe7f001f0  /* reserved instr; Linux GNU eabi breakpoint */

sz_Elf64_Ehdr= 64
sz_Elf64_Phdr= 56
sz_l_info = 12
sz_p_info = 12
sz_b_info = 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

MAP_PRIVATE=   0x02
MAP_FIXED=     0x10

PROT_READ=     0x1

O_RDONLY=       0

PAGE_SHIFT= 12
PAGE_SIZE = -(~0<<PAGE_SHIFT)

#ifndef DEBUG  /*{*/
#define DEBUG 0
#endif  /*}*/

//@ control just falls through, after this part and compiled C code
//@ are uncompressed.

#define OVERHEAD 2048
#define MAX_ELF_HDR 512

fold_begin:
        brk #0
/* In:
   r10= &b_info of first compressed block (after move)
    r9= total size of compressed data
    r5= f_decompress
    sp/ ADRU,LENU,sz_unc,crumb,MFLG,argc,argv...
*/
        POP2(x3,x4)  // x3= ADRU; x4= LENU
        POP4(x6,x7,x8,x11)  // x6= sz_unc; x7= crumb; x8= MFLG; x11= argc
        mov x0,sp  // current stack pointer

        add x1,sp,w11,lsl #3  // skip argv pointers
        add x1,x1,#8  // skip argv terminator
.L10:  // skip env
        ldr x2,[x1],#8
        cbnz x2,.L10
.L20:  // skip auxv
        ldr x2,[x1],#2*8
        cbnz x2,.L20

        sub x0,x1,#4096 // buffer for readlink
        sub x0,x0,#8  // space for copy of space3eq
        sub x1,x1,x30  // amount needed for argv,env,auxv
        lsr x0,x0,#2+ PAGE_SHIFT  // 2+ because hardware might require
        sub x0,x0,#1  // one page
        lsl x0,x0,#2+ PAGE_SHIFT
        sub x0,x0,x1  // space for argv,env,auxv
        sub x0,x0,#8  // room for added env ptr
        mov x1,sp  // old_argv
        mov sp,x0  // new_argv
// FIXME: argc must be 16-byte aligned
        PUSH5(x2,x4,x6,x7,x11)  // ADRU, LENU, sz_unc, crumb, argc
SP_sz_unc= 2*8
SP_argc= 4*8

.Larg:  // copy argv
        ldr x2,[x1],#8
        str x2,[x0],#8
        cbnz x2,.Larg

.Lenv:  // copy env
        ldr x2,[x1],#8
        str x2,[x0],#8
        cbnz x2,.Lenv

        sub x4,x0,#8  // added env ptr goes here
        str x2,[x0],#8  // terminator after added ptr
        PUSH1(x0)  // new &Elf64_auxv_t

.Laux:  // copy auxv
        ldp x2,x3,[x1,#2*8]  // .a_type, .a_val
        stp x2,x3,[x0,#2*8]
        cbnz x1,.Laux

        str x0,[sp,#SP_sz_unc]  // clobber sz_unc with 1+ &Elf64_auxv_t[AT_NULL@.a_type]

        add x1,x0,#1<<(2+ PAGE_SHIFT)  // after the page
        str x1,[x4]  // new env ptr
        mov x4,x0  // save page address
        ldr w2,space3eq
        str w2,[x1],#8  // "   =" of new env var

        mov x2,#4096  // buffer length
        adr x0,proc_self_exe
        sub x2,x2,#1  // room for null terminator
        bl readlink; cmn x0,#4096; bcs 0f  // error
        mov w2,#0
        strb w2,[x1,x0]  // null terminate pathname
0:
/* Preserve symlink /proc/self/exe by mapping one page into stack */
        adr x0,proc_self_exe
        mov w1,#O_RDONLY
        bl open  // no error check: cannot recover

        mov x1,#0
        PUSH2(x0,x1)  // fd, offset
        mov x0,x4  // page address
        mov w1,#PAGE_SIZE
        mov w2,#PROT_READ
        mov w3,#MAP_PRIVATE|MAP_FIXED
        bl mmap  // no error check: cannot recover
        ldr x0,[sp],#8  // fd
        bl close
        POP2(x1,x3)  // junk, new &Elf64_auxv_t

/* Construct arglist for upx_main */
        sub sp,sp,#MAX_ELF_HDR + OVERHEAD  // alloca
        mov w4,w9  // total size of compressed data
        ldr w9,[x10,#sz_cpr]  // xi.size  of ELF headers
        mov x8,sp  // xo.ptr
        ldr w7,[x10,#sz_unc]  // xo.size
        adr x6,f_unfilter
        add w9,w9,#sz_b_info  // for unpackExtent
        mov x11,#0  // dynbase for ET_EXEC;  FIXME for ET_DYN
#if defined(ARMEL_DARWIN)  /*{*/
        ldr w12,4*1 + munmap
#elif defined(ARMEL_EABI4)  /*}{*/
        ldr w12,4*2 + munmap
#elif defined(ARM_OLDABI)  /*}{*/
        ldr w12,4*0 + munmap
#else  /*}{*/
        mov w12,#0
#endif  /*}*/
        PUSH5(x8,x9,x10,x11,x12)
        PUSH5(x3,x4,x5,x6,x7)
        POP4(x0,x1,x2,x3)
        bl upx_main
        mov x29,x0  // entry address
        add sp,sp,#(10-4)*8
        add sp,sp,#MAX_ELF_HDR + OVERHEAD  // un-alloca

        POP4(x0,x1,x2,x3)  // x0= ADRU; x1= LENU; x2= 1+ &Elf64_auxv_t[AT_NULL@.atype]; x3= crumb
        add x0,x0,x3  // ADRU+=crumb
        sub x1,x1,x3  // LENU-=crumb

#if DEBUG  /*{*/
        ldr x3,[x2,#8 -2*8]  // Elf64_auxv_t[AT_NULL@.a_type].a_val
        ldr x4,[x3,#0]  // 1st instr
        ldr x5,[x3,#4]  // 2nd instr
#define TRACE_REGS r0-r12,r14,r15
        stmdb sp!,{TRACE_REGS}; mov r0,#4; bl trace
#endif  /*}*/
        mov w3,#0  // clear registers: paranoia
        mov w4,#0
        mov w5,#0
        mov w6,#0

        mov w8,#0
        mov w9,#0
        mov w10,#0
        mov w11,#0

/* Heuristic cache flush: sweep contiguous range to force collisions and evictions. */
        sub x12,sp,#(1<<19)  // limit: 1/2 MB more
sweep:
        ldr x7,[sp],#-(1<<5)  // extend stack; read allocate 32 bytes
        str x7,[sp]  // make it dirty
        ldr x7,[sp]  // read alocate again in case dirtying caused COW split
        cmp x12,x30; blo sweep

        add sp,sp,#(1<<19)  // pop stack

        mov w8,#__NR_munmap
        mov w12,#0
        ldr x8,[x2,#8 -2*8]  // Elf64_auxv_t[AT_NULL@.a_type].a_val
        br x8

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
#define FILTER_ID 0x50  /* little-endian */
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
        sub t1,t1,lenw  // word displ; hi 0b.00101 preserves bit 31
        ubfm t1,t2,#26,#5
        str t1,[ptr,len,lsl #2]
tst_unf:
        cbnz len,top_unf
unfret:
        ret

#if DEBUG  /*{*/
TRACE_BUFLEN=512
trace:
        str lr,[sp,#(-1+ 15)*4]  // return pc; [remember: sp is not stored]
        mov r4,sp  // &saved_r0
        sub sp,sp,#TRACE_BUFLEN
        mov r2,sp  // output string

        mov r1,#'\n'; bl trace_hex  // In: r0 as label
        mov r1,#'>';  strb r1,[r2],#1

        mov r5,#3  // rows to print
L600:  // each row
        sub r0,r4,#TRACE_BUFLEN
        sub r0,r0,sp
        mov r0,r0,lsr #2; mov r1,#'\n'; bl trace_hex  // which block of 8

        mov r6,#8  // words per row
L610:  // each word
        ldr r0,[r4],#4; mov r1,#' '; bl trace_hex  // next word
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
        mov r3,#4*(8 -1)  // shift count
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

__NR_exit  = 0x5e + __NR_SYSCALL_BASE  // 94  exit_group
__NR_read  = 0x3f + __NR_SYSCALL_BASE  // 63
__NR_write = 0x40 + __NR_SYSCALL_BASE  // 64
__NR_open  = 0x38 + __NR_SYSCALL_BASE  // 56
__NR_close = 0x39 + __NR_SYSCALL_BASE  // 57
__NR_unlink= 0x23 + __NR_SYSCALL_BASE  // 35
__NR_getpid= 0xad + __NR_SYSCALL_BASE  // 172
__NR_brk   = 0xd6 + __NR_SYSCALL_BASE  // 214
__NR_readlink=0x4e+ __NR_SYSCALL_BASE  // 78


__NR_mmap2    = 0xde + __NR_SYSCALL_BASE  // 222
__NR_mprotect = 0xe2 + __NR_SYSCALL_BASE  // 226
__NR_munmap   = 0xd7 + __NR_SYSCALL_BASE  // 215

__ARM_NR_BASE       = 0x0f0000 + __NR_SYSCALL_BASE
__ARM_NR_cacheflush = 2 + __ARM_NR_BASE

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

        .globl open
open:
        do_sys __NR_open; ret

        .globl close
close:
        do_sys __NR_close; ret

        .globl unlink
unlink:
        do_sys __NR_unlink; ret

        .globl getpid
getpid:
        do_sys __NR_getpid; ret

        .globl brk
brk:
        do_sys __NR_brk; ret

        .globl readlink
readlink:
        do_sys __NR_readlink; ret

        .globl munmap
munmap:
        do_sys __NR_munmap; ret

        .globl mprotect
mprotect:
        do_sys __NR_mprotect; ret

        .globl __clear_cache
__clear_cache:
        hlt #0
        mov w2,#0
        do_sys 0; ret  // FIXME

        .globl mmap
mmap:
        PUSH2(x4,x5)
        ldp x4,x5,[sp,#2*8]
        lsr x5,x5,#12  // convert to page number
mmap_do:
        do_sys __NR_mmap2
        POP2(x4,x5)
        ret

bits_privanon= -4+ fold_begin  // entry stores: MAP_{PRIVATE|ANON}  QNX vs linux

mmap_privanon: .globl mmap_privanon
        ldr x12,bits_privanon  // r12 === ip
        PUSH2(x4,x5)
        mov w5,#0  // offset= 0
        mov x4,#-1  // fd= -1
        orr w3,w3,w12  // flags |= MAP_{PRIVATE|ANON}  [QNX vs Linux]
        b mmap_do


#if DEBUG  /*{*/

div10: .globl div10
        mov ip,r0  // extra copy used at end
        sub r1,r1,r1  // hi

        mov r2,r0  // copy lo
        adds r0,r0,r0,lsl #3   // 9*lo
        adc  r1,r1,r1,lsl #3   // 9*hi + C
        add  r1,r1,r2,lsr #(32 - 3)  // bits shifted from lo to hi

        mov r2,r0  // copy lo
        adds r0,r0,r0,lsl #4
        adc  r1,r1,r1,lsl #4
        add  r1,r1,r2,lsr #(32 - 4)  // * 0x99

        mov r2,r0  // copy lo
        adds r0,r0,r0,lsl #8
        adc  r1,r1,r1,lsl #8
        add  r1,r1,r2,lsr #(32 - 8)  // * 0x9999

        mov r2,r0  // copy lo
        adds r0,r0,r0,lsl #16
        adc  r1,r1,r1,lsl #16
        add  r1,r1,r2,lsr #(32 - 16)  // * 0x99999999

        subs r0,r0,ip,lsl #(32 - 1)  // - * 0x80000000
        sbc  r1,r1,ip,lsr #1         //   * 0x19999999

        adds r0,r0,ip
        adc  r0,r1,#0  // * 0x0.1999999a
        ret

#endif  /*}*/

// vi:ts=8:et:nowrap

