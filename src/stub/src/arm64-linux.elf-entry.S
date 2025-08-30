/*  aarch64-linux.elf-entry.S -- Linux program entry point & decompressor (Elf binary)
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2025 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2025 Laszlo Molnar
*  Copyright (C) 2000-2025 John F. Reiser
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

unmap_all_pages= (1<<1)

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
__NR_execve   = 0xdd  // 221
__NR_exit     = 0x5d  //  93
__NR_fork     = 0xdc  // 220
__NR_getppid  = 0xad  // 173
__NR_memfd_create= 0x117  // 279
__NR_mmap     = 0xde  // 222
__NR_mprotect = 0xe2  // 226
__NR_munmap   = 0xd7  // 215
__NR_openat   = 0x38  //  56
__NR_pipe2    = 0x3b  //  59
__NR_read     = 0x3f  //  63
__NR_sigaction = 0x86  // 134
  AT_FDCWD= -100
__NR_write    = 0x40  //  64

MFD_EXEC= 0x0010  // memfd_create should allow later PROT_EXEC

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
cancel_SEGV .req x18

D_PMASK=    0*NBPW
D_XSIGSEGV= 1*NBPW
D_GETPM=    2*NBPW
D_PRIVANON= 2*NBPW + 2*4
D_FOLD=     2*NBPW + 3*4  // .data space at start of unfold

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
        adr cancel_SEGV,cancel_dummy

  section ELFSIGSEGV
// install SIGSEGV handler for debugging hardware and de-compressor
SIGSEGV= 11
SA_SIGINFO= 4  // /usr/include/bits/sigaction.h
SA_RESTORER= 0x04000000
sa_sigaction = 0 * NBPW
sa_flags     = 1 * NBPW
sa_mask      = 3 * NBPW  // location of mask
sa_restorer  = 4 * NBPW
sa_frame = 8 * NBPW
        adr x0,sigsegv_sigaction  // handler
        mov x1,#SA_SIGINFO  // flag bits
        stp x0,x1,[sp,-sa_frame]!  // .sa_sigaction, .sa_flags
        str xzr,[sp,sa_mask]  // .sa_mask
        str xzr,[sp,sa_restorer]

        mov x3,#8  // 8 bytes  ==> 64 bits
        mov x2,xzr  // do not save old_sigaction
        mov x1,sp  // &new_sigaction
        mov w0,#SIGSEGV
        do_sys __NR_sigaction
        add sp,sp,sa_frame
        adr cancel_SEGV,cancel_sigsegv
        b L300

#if 0  //{ TEST ONLY
mov  x0,#0xc0;mov  x1,#0xc1;mov  x2,#0xc2;mov  x3,#0xc3;mov  x4,#0xc4;mov  x5,#0xc5;mov  x6,#0xc6;mov  x7,#0xc7
mov  x8,#0xc8;mov  x9,#0xc9;mov x10,#0xca;mov x11,#0xcb;mov x12,#0xcc;mov x13,#0xcd;mov x14,#0xce;mov x15,#0xcf
mov x16,#0xd0;mov x17,#0xd1;mov x18,#0xd2;mov x19,#0xd3;mov x20,#0xd4;mov x21,#0xd5;mov x22,#0xd6;mov x23,#0xd7
mov x24,#0xd8;mov x25,#0xd9;mov x26,#0xda;mov x27,#0xdb;mov x28,#0xdc;mov x29,#0xdd;mov x30,#0xde
    ldr x0,[x0]  // cause SIGSEGV
#endif  //}

proc_self_cmdline:
        .asciz "/proc/self/cmdline"
announce_sigaction:
        .asciz "\n\nSIGSEGV address space:\n"
end_announce_sigaction:

proc_self_maps:
        .asciz "/proc/self/maps"
minus_q:
        .asciz "-q"
path_gdb:
        .asciz "/usr/bin/gdb"
commands_gdb:
        .ascii "set prompt\n"  // null string prompt
        .ascii "info inferiors\n"
        .ascii "print \"x0-x7\"\n"
        .ascii "x/8xg $sp + 0x138\n"
        .ascii "print \"x8-x15\"\n"
        .ascii "x/8xg\n"
        .ascii "print \"x16-x23\"\n"
        .ascii "x/8xg\n"
        .ascii "print \"x24-x30\"\n"
        .ascii "x/7xg\n"
        .ascii "set $pc = *(long *)($sp + 0x238)\n"
        .ascii "print \"faulting instr\"\n"
        .ascii "x/i $pc\n"
        .ascii "print \"fault context\"\n"
        .ascii "x/15i $pc - 7*4\n"
      //.ascii "print \"intercept stack\"\n"
      //.ascii "x/76xg $sp\n"
        .ascii "set $sp = *(long *)($sp + 0x230)\n"
        .ascii "print \"user stack\"\n"
        .ascii "x/64xg $sp\n"
        .ascii "kill\n"
        .ascii "quit 1"
        .byte 0
commands_gdb_end:
        .balign 4

#define r_fd w19
PATH_MAX= 4096
child:
#if 1  //{ pipe input to gdb
        mov arg1w,#0; do_sys __NR_close  // fd_stdin
        stp xzr,xzr,[sp,-2 * NBPW]!  // &fd_pipe[2]; 4 bytes each; but stack align
        mov arg2w,#0
        mov arg1,sp
        do_sys __NR_pipe2  // read size will be 0 (fd_stdin)
        add x16,sp,#4
        ldr r_fd,[x16]  // write side of pipe
        add sp,sp,#2*NBPW

        mov w2,#commands_gdb_end - commands_gdb  // arg3 len
        adr x1,commands_gdb  // arg2
        mov w0,r_fd  // arg1 fd
        do_sys __NR_write
        mov w0,r_fd; do_sys __NR_close
#endif  //}

        sub sp,sp,#PATH_MAX
        mov arg3w,#O_RDONLY
        adr arg2,proc_self_cmdline
        mov arg1,#-1  // fake FD_CWD
        do_sys __NR_openat; mov r_fd,w0
        mov arg2,sp  // buffer
        mov arg3w,#PATH_MAX
        do_sys __NR_read
        mov arg1w,r_fd; do_sys __NR_close

        do_sys __NR_getppid
        stp xzr,xzr,[sp,-2 * NBPW]!  // decimal(pid) fits in 16 bytes
        mov w1,w0; mov x0,sp; call unsimal

        mov arg4,sp  // &"pid"  (result of unsimal)
        add arg3,sp,#2 * NBPW  // cmdline argv[0]  [skip "pid"]
        adr arg2,minus_q
        adr arg1,path_gdb  // arg1  path
        stp arg1,arg2,[sp,-6 *NBPW]!  // path_gdb, "-q"
        stp arg3,arg4,[sp,2 *NBPW]  // argv[0], "pid"
        str xzr,[sp,4 * NBPW]  // NULL
        mov arg3,#0  // _envviron  BUG
        mov arg2,sp  // &argv
        do_sys __NR_execve
0:
        b 0b

// ucontext_t, mcontext_t, gregset_t, gret_t, REG_xxx:   <sys/ucontext.h>
// stack_t  <bits/types/stack_t.h>
__restore_rt:
    nop

sigsegv_sigaction:
        stp x1,x2,[sp,-2 * NBPW]  // save siginfo *, ucontext *
// print /proc/self/maps of child (same as parent: the beauty of fork())
        mov x2,#end_announce_sigaction - announce_sigaction  // arg3 len
        adr x1,announce_sigaction  // arg2 buf
        mov w0,#2  // arg1 fd_stderr
        do_sys __NR_write

        mov w2,#O_RDONLY  // arg3 flags
        adr x1,proc_self_maps  // arg2 path
        mov w0,#-1  // fake FD_CWD
        do_sys __NR_openat
        mov r_fd,w0  // fd_maps
BUFLEN= 4096
        sub sp,sp,#BUFLEN
loop_maps:
        mov x2,#BUFLEN  // arg3 buflen
        mov x1,sp  // arg2 buffer
        mov w0,r_fd  // arg1 fd_maps
        do_sys __NR_read
        cbz w0,done_maps
        mov x2,x0  // arg3 buflen
        mov x1,sp  // arg2 buf
        mov w0,#2  // fd_stderr
        do_sys __NR_write
        b loop_maps
done_maps:
        add sp,sp,#BUFLEN  // discard buffer
        mov x0,x16; do_sys __NR_close
// end printing of /proc/self/maps

        mov arg1,#0; mov arg2,#0  // paranoia
        do_sys __NR_fork; cbz x0,child
parent:
        b parent  // spin; paused by gdb

unsimal:  // (dst, value)
        stp lr,xzr,[sp,-2 * NBPW]!; call 0f
        ldp lr,x1,[sp],#2 * NBPW
        strb w1,[x0]  // terminator
        ret
0:
        mov  w2,#0xcccd
        movk w2, 0xcccc, lsl 16  // 0xcccccccd ==> 4/5 as 32-bit fraction
        umull x2,w1,w2
        //add x2,x2,#0x1000  // BUG?  unsimal(p, 730)
        lsr x3,x2,35  // quo(value, 10)
        add w2,w3,w3,lsl 2  // 5 * quo
        sub w1,w1,w2,lsl 1  // rem = (val - (10 * quo))
        stp lr,x1,[sp,-2 * NBPW]!
        cbz x3,1f; mov w1,w3; call 0b
1:
        ldp lr,x1,[sp],#2 * NBPW; add w1,w1,#'0'; strb w1,[x0],#1
        ret

cancel_sigsegv:
        mov x3,#8  // 8 bytes  ==> 64 bits
        mov x2,xzr  // no old
        mov x1,xzr  // no new
        mov w0,#SIGSEGV
        do_sys __NR_sigaction
        ret

L300:

  section ELFMAINX2
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

        adr xelfa,sz_pack2
        ldr w0,[xelfa]  // sz_pack2 causes unknown R_AARCH64_LD_PREL_LO19
        str x0,[sp,#F_SZPK2]
        sub xelfa,xelfa,w0,uxtw
        call main
        mov xFOLD,lr

        mov w1,#MFD_EXEC  // flags
0: // try memfd_create
        adr x0,strupx
        do_sys __NR_memfd_create
        tbz w0,#31,ok_memfd  // success
        cbz w1,no_memfd  // already failed twice
        mov w1,wzr; b 0b  // try again without MFD_EXEC
no_memfd:  // so try /dev/shm
O_RDWR= 2
O_DIRECTORY= 040000   //  0x04000
O_TMPFILE= 020000000  // 0x400000
        call 0f; .int 0700, O_RDWR|O_DIRECTORY|O_TMPFILE; .asciz "/dev/shm"
        .balign 4; 0:
        ldr w3,[lr],#4  // mode
        ldr w2,[lr],#4  // flags
        mov x1,lr  // name
        mov w0,#AT_FDCWD
        do_sys __NR_openat
ok_memfd:
        mov mfd,w0

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

        str xPMASK,[sp]  // propagate PAGE_MASK  D_PMASK
        str cancel_SEGV,[sp, #D_XSIGSEGV]

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
        add lr,xADRU,#D_FOLD  // jmp to fold_begin
        sub wLENC,wLENC,w1
        str xLENC,[sp,#F_SZPK2]
        bic w1,w1,#unmap_all_pages
        add xADRC,xelfa,x1  // &b_info of compressed input data
        jr lr  // goto unfolded stub

cancel_dummy:
        ret

zfind:
        ldr x1,[x0],#NBPW; cbnz x1,zfind
        ret

f_expand:
// nrv2b code is hard-wired here
#define NO_METHOD_CHECK 1

// only one decompressor; build 'eof' return
#undef DAISY_CHAIN

// use of mmap() forces implicit cache sync
#define NO_SYNC_CACHE 1

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

strupx:
        .asciz "upx"
        .balign 4

main:  // In: w0= sz_pack2
lr .req x30
        callr lr
LxFOLD:
LOBINFO:
        .int O_BINFO
LBINFO:
        // { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...}
/*
vaddi:ts=8:et:nowrap
*/

