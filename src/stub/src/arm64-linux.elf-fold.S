//  aarch64-linux.elf-fold.S -- linkage to C code to process Elf binary
//
//  This file is part of the UPX executable compressor.
//
//  Copyright (C) 2000-2022 John F. Reiser
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
#define call bl

#define ARM_OLDABI 1
#include "arch/arm64/v8/macros.S"

sz_Ehdr= 8*NBPW
sz_Phdr= 7*NBPW
  p_vaddr= 4+4+8
  p_filesz= 4+4+8+8+8
  p_memsz=  4+4+8+8+8+8

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
#if DEBUG  //{
#define TRACE(arg) \
        stp lr,x0,[sp,#-2*NBPW]!; mov x0,arg; call trace; \
        ldp lr,x0,[sp],#2*NBPW
#else  //}{
#define TRACE(arg) /*empty*/
#endif  //}

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

// In:  sp/ PMASK,fd,ADRU,LENU, argc,argv,...
//lr   .req x30
wLENC  .req w29  // compressed input
  xLENC  .req x29
xADRC  .req x28

wLENU  .req w27  // unfolded fold_begin and upx_main
  xLENU  .req x27
xADRU  .req x26

xelfa  .req x25  // hi &Elf64_Ehdr
xfexp  .req x24  // de-compressor
xauxe  .req x23  // past new auxv
xauxv  .req x22

wfd    .req w21
  xfd    .req x21
xPMASK .req x20

        POP4(xPMASK,xfd, xADRU,xLENU)

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
        call readlink
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
        mov w1,wLENC  // total size of compressed data
        mov x0,xADRC  // &b_info
        call upx_main
          add sp,sp,#MAX_ELF_HDR + OVERHEAD  // un-alloca
        mov xfexp,x0  // entry address

// Discard pages of compressed input data (includes [ADRC,+LENC) )
        ldr x1,[xelfa,#p_memsz+sz_Phdr+sz_Ehdr]  // Phdr[C_TEXT= 1].p_memsz
        mov x0,xelfa  // hi &Elf64_Ehdr
        call munmap  // discard C_TEXT compressed data

// Map 1 page of /proc/self/exe so that munmap does not remove all references
        mov x5,#0  // offset
        mov w4,wfd  // fd
        mov w3,#MAP_PRIVATE
        mov w2,#PROT_READ
        mov x1,#PAGE_SIZE
        mov x0,#0  // addr
        call mmap

        mov w0,wfd  // fd
        call close

        mov lr,xfexp  // entry
        mov x0,xADRU
        mov w1,wLENU
        ldr x2,[xauxe,#NBPW - sz_auxv]  // Elf64_auxv_t[AT_NULL@.a_type].a_val

                // paranoia: zero used registers except x0,x1,x2,x30(lr)
        stp xzr,xzr,[sp,#-2*NBPW]!
        mov x16,xzr
        ldp x3,x4,[sp,#0]
        ldp x6,x7,[sp,#0]
        ldp x8,x9,[sp,#0]
        ldp x20,x21,[sp,#0]
        ldp x22,x23,[sp,#0]
        ldp x24,x25,[sp,#0]
        ldp x26,x27,[sp,#0]
        ldp x28,x29,[sp],#2*NBPW

#if DEBUG  //{
        ldr w3,[x2,#0]  // 1st instr
        ldr w4,[x2,#4]  // 2nd instr
        TRACE(4)
#endif  //}

#if 1  //{
// This is a workaround for an intermittent problem. The "svc #0" in the escape hatch
// to invoke munmap() on the unfolded stub, sometimes (20% ?) gets SIGILL despite
// apparent good environment.  So do not unmap, which will waste 1 page.
// Example:
//     Program received signal SIGILL, Illegal instruction.
// 0x0000fffff7fde974 in ?? ()
// (gdb) info proc
// process 748
// cmdline = '/home/jreiser/611/hello.packed'
// (gdb) shell cat /proc/748/maps
// fffff7faa000-fffff7fcb000 r-xp 00000000 b3:02 8845                       /usr/lib/aarch64-linux-gnu/ld-2.31.so
// fffff7fcb000-fffff7fdb000 ---p 00000000 00:00 0
// fffff7fdb000-fffff7fde000 rw-p 00021000 b3:02 8845                       /usr/lib/aarch64-linux-gnu/ld-2.31.so
// fffff7fde000-fffff7fdf000 r-xp 00000000 00:00 0   ## pc in this page; has 'r-x' permission
// fffff7fdf000-fffff7fee000 ---p 00000000 00:00 0
// fffff7fee000-fffff7ff0000 rw-p 00000000 00:00 0
// fffff7ffa000-fffff7ffb000 r-xp 00000000 00:00 0   ## target of munmap [x0, +x1)
// fffff7ffb000-fffff7ffd000 r--p 00000000 00:00 0                          [vvar]
// fffff7ffd000-fffff7ffe000 r-xp 00000000 00:00 0                          [vdso]
// fffff7fff000-fffff8000000 r--p 00000000 b3:02 126504                     /home/jreiser/611/hello.packed
// fffffffdf000-1000000000000 rw-p 00000000 00:00 0                         [stack]
// (gdb) x/2i $pc
// => 0xfffff7fde974:   svc #0x0
//    0xfffff7fde978:   ret
// (gdb) info reg
// x0             0xfffff7ffa000      0xfffff7ffa000  ## address to unmap
// x1             0xa10               0xa10  # length to unmap
// x2             0xfffff7fde974      0xfffff7fde974  ## address of escape hatch
// x3 - x29       0x0                 0x0
// x30            0xfffff7fab140      0xfffff7fab140  ## entry to ld-linux
// sp             0xffffffffd7b0      0xffffffffd7b0
// pc             0xfffff7fde974      0xfffff7fde974
// cpsr           0x60000000          [ EL=0 C Z ]
// fpsr           0x0                 0x0
// fpcr           0x0                 0x0
// (gdb)
        ret  // "br lr"; just enter ld-linux, omitting the munmap of unfolded stub
#else  //}{  This should work, but fails intermittently
        mov w8,#__NR_munmap  // munmap(ADRU, LENU)  unfolded stub
        br x2  // goto escape hatch
#endif  //}

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

        mov w1,#'\n'; call trace_hex  // In: r0 as label
        mov w1,#'>';  strb w1,[x2],#1

        mov w5,#10  // nrows to print
L600:  // each row
        add x1,sp,#TRACE_BUFLEN
        sub x0,x4,x1
        lsr x0,x0,#3; mov w1,#'\n'; call trace_hex2  // which block of 4

        mov w6,#4  // 64-bit words per row
L610:  // each word
        ldr x0,[x4],#8; mov w1,#(' '<<8)|' '; call trace_hex  // next word
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
        brk #0  // my_bkpt
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

