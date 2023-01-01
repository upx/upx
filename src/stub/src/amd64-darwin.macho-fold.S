/*  i386-darwin.macho-fold.S -- linkage to C code to process Mach-o binary
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2023 Laszlo Molnar
*  Copyright (C) 2000-2023 John F. Reiser
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

NBPW= 8
#include "arch/amd64/macros.S"

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4

sz_l_info= 12
sz_p_info= 12

sz_Mach_header64= 8*4
sz_Mach_segment_command= 2*4 + 16 + 4*8 + 4*4

_start: .globl _start  # ignored, but silence "cannot find entry symbol _start" from ld


fold_begin:  // In: %rbp= &f_exp; %r14= ADRX; %r15= LENX
// rsp/ fd,ADRU,LENU,%entry,&Mach_header64, argc,argv,0,envp,0,apple,0,strings
//    int3
        call L90  # push &L90
#include "arch/amd64/bxx.S"
L90:
        pop %arg6  # &amdbxx: f_unfilter
        movq %r14,%arg1  // ADRX
        movq %r15,%arg2  // LENX
        movq %rsp,%r12  // remember for un-alloca
        movl sz_unc+sz_p_info+sz_l_info(%arg1),%arg4l  // sz_mhdr
        movl $2048,%eax  // >= 2KiB for /usr/lib/dyld
        cmpl  %eax,%arg4l
        cmovb %eax,%arg4l  // sz_mhdr
        lea 4*NBPW(%r12),%rax  // &&Mach_header64 for arg7
        movq %rbp,%arg5  // f_decompress
        subq %arg4,%rsp  // alloca(sz_mhdr)
        movq %rsp,%arg3  // temp char mhdr[sz_mhdr]
        push %rax  // (arg7) mhdrpp= &mhdrp
// upx_main(&l_info,len_cpr,temp[sz_mhdr],sz_mhdr,f_exp,f_unf,mhdr **)
        call upx_main  # Out: rax= &Mach_AMD64_thread_state of dyld

AMD64_ts_rip= 16*NBPW
        movq (%rax),%rbp  // &hatch
        movq AMD64_ts_rip(%rax),%rcx  // from temp char mhdr[] before unalloca
        movq %r12,%rsp  // unalloca
        movq %rcx,3*NBPW(%rsp)  // entry
        pop %arg1  // fd
        call close
        pop %arg1  // ADRU
        pop %arg2  // LENU
        mov $SYS_munmap+SYSBASE,%eax
        jmp *%rbp  // goto hatch: syscall.munmap(ADRU,LENU); ret

bswap: .globl bswap
0:
        mov (%arg1),%eax
        .byte 0x0f,0xc8  // bswap eax
        mov %eax,(%arg1)
        sub $4,%arg2l
        lea 4(%arg1),%arg1
        ja 0b
        ret

SYSBASE= 0x02000000

SYS_exit  =1
SYS_read  =3
SYS_write =4
SYS_open  =5
SYS_close =6

SYS_pread    =0x99
SYS_mmap     =0xc5
SYS_munmap   =0x49
SYS_mprotect =0x4a

// lazy jmps enable compression of this code
write: .globl write
        mov $SYS_write,%al;  jmp 2+ 0f; 0:
exit: .globl exit
        mov $SYS_exit,%al;  jmp 2+ 0f; 0:
mprotect: .globl mprotect
        mov $SYS_mprotect,%al; jmp 2+ 0f; 0:
munmap: .globl munmap
        mov $SYS_munmap,%al; jmp 2+ 0f; 0:
pread: .globl pread
        mov $SYS_pread,%al; jmp 2+ 0f; 0:
close: .globl close
        mov $SYS_close,%al; jmp 2+ 0f; 0:
open: .globl open
        mov $SYS_open,%al;  jmp 2+ 0f; 0:
mmap: .globl mmap
        mov $SYS_mmap,%al;  jmp 2+ 0f; 0:
read: .globl read
        mov $SYS_read,%al

        push %r9; push %r8; push %rcx; push %rdx; push %rsi; push %rdi
        movzbl %al,%eax; push %rax  // save SYS_nnn for error diagnosis
        or $SYSBASE,%eax
        mov %rcx,%r10
        syscall  // .byte 0x0f,0x05

#define DEBUG 0
#if !DEBUG  //{
        jc sysfail
        add $7*8,%rsp  // SYS_nnn, 6 args
        ret
#endif  //}

DFRAME=0x100
sysfail:
        pushf
        push %rax  // retval
        push %rdi; push %rsi  // registers used
        push %rbp; mov %rsp,%rbp
        cmpl $SYS_write,5*8+0*8(%rbp); je ignore_WRITE
        sub $DFRAME,%rsp  // alloca

        mov %rsp,%rdi  // output ptr
        movb $'\n',%al; stosb
        movb $'*',%al; stosb
        mov     3*8(%rbp),%rax; call pr_hex; movb $'=',%al; stosb  // retval
        mov 5*8+0*8(%rbp),%rax; call pr_hex; movb $'(',%al; stosb  // SYS_nnn
        mov 5*8+1*8(%rbp),%rax; call pr_hex; movb $',',%al; stosb  // arg1
        mov 5*8+2*8(%rbp),%rax; call pr_hex; movb $',',%al; stosb  // arg2
        mov 5*8+3*8(%rbp),%rax; call pr_hex; movb $',',%al; stosb  // arg3
        mov 5*8+4*8(%rbp),%rax; call pr_hex; movb $',',%al; stosb  // arg4
        mov 5*8+5*8(%rbp),%rax; call pr_hex; movb $',',%al; stosb  // arg5
        mov 5*8+6*8(%rbp),%rax; call pr_hex; movb $')',%al; stosb  // arg6
        movb $'\n',%al; stosb

        mov %rdi,%rdx
        sub %rsp,%rdx  // len
        mov %rsp,%rsi  // ptr
        mov $2,%edi  // FD_STDERR
        mov $SYS_write+SYSBASE,%eax
        syscall

        add $DFRAME,%rsp
ignore_WRITE:
        pop %rbp; pop %rsi; pop %rdi
        pop %rax  // retval
        popf
        lea 7*8(%rsp),%rsp  // SYS_nnn, args

        jnc sysOK
        or $~0,%rax  //mov errno,eax
sysOK:
        ret

pr_hex:
        lea hextab(%rip),%rsi
        mov %rax,%rdx
        mov $16,%ecx  // char count
1:
        rol $4,%rdx; mov %edx,%eax; and $0xf,%eax
        movzbl (%rsi,%rax),%eax
        stosb; sub $1,%ecx; jnz 1b
        ret

hextab:
        .ascii "0123456789abcdef"

/* vim:set ts=8 sw=8 et: */
