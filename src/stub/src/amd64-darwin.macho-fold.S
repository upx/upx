/*  i386-darwin.macho-fold.S -- linkage to C code to process Mach-o binary
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2013 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2013 Laszlo Molnar
*  Copyright (C) 2000-2013 John F. Reiser
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

#include "arch/amd64/macros.S"

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4

sz_l_info= 12
sz_p_info= 12

_start: .globl _start  # ignored, but silence "cannot find entry symbol _start" from ld


fold_begin:  // In: %rbx= &total_length; %rbp= &decompress
//    int3
        call L90  # push &L90
#include "arch/amd64/bxx.S"
L90:
        pop %arg6  # L90; &amdbxx: f_unfilter
        movq %rbx,%arg1
        movl (%rbx),%arg2l  // total length; offset to {l_info; p_info; b_info}
        subq %arg2,%arg1  // &{l_info; p_ifo, b_info}
        push $0  # default value for mhdrp
        movl $2048,%eax
        movl sz_unc+sz_p_info+sz_l_info(%arg1),%arg4l  // sz_mhdr
        cmpl %eax,%arg4l; cmovb %eax,%arg4l  // at least 2KiB for /usr/lib/dyld
        movq %rbp,%arg5  // f_decompress
        movq %rsp,%r12  // remember for un-alloca
        subq %arg4,%rsp  // alloca
        movq %rsp,%arg3  // temp char mhdr[sz_mhdr]
        push %r12  // (arg7) mhdrpp= &mhdrp
        call upx_main  # Out: rax= &Mach_AMD64_thread_state of dyld
        movq %r12,%rsp  // unalloca
AMD64_ts_rip= 16*8
        jmp *AMD64_ts_rip(%rax)  # %rsp: mhdrp, argc, argv...

bswap: .globl bswap
0:
        mov (%arg1),%eax
        .byte 0x0f,0xc8  // bswap eax
        mov %eax,(%arg1)
        sub $4,%arg2l
        lea 4(%arg1),%arg1
        ja 0b
        ret

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

        movzbl %al,%eax; or $0x02000000,%eax
        mov %rcx,%r10
        syscall  // .byte 0x0f,0x05
        jnc 0f
        or $~0,%rax  //mov errno,eax
0:
        ret

/*
vi:ts=8:et:nowrap
*/

