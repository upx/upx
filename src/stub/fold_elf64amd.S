/*  fold_elf64amd.S -- linkage to C code to process Elf binary
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 2000-2005 John F. Reiser
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
*  <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>
*
*  John F. Reiser
*  <jreiser@users.sourceforge.net>
*/
#include "amd_regs.h"

PAGE_SHIFT= 12
PAGE_MASK= 0xffffffffffffffff<<PAGE_SHIFT

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4

sz_l_info= 12
sz_p_info= 12

OVERHEAD=2048
MAX_ELF_HDR=1024

__NR_munmap=  91

/* In:
    %rbp= &decompress; also 9+ (char *)&(offset to {l_info; p_info; b_info})
    %rsp= &{argc,argv...,0,env...,0,auxv...,strings}
*/
fold_begin:  
        call L90  # ret_addr is &f_unfilter
#include "amd_bxx.S"

zfind:
        lodsq; testq %rax,%rax; jnz zfind
        ret
L90:
        pop %arg6  # &amdbxx: f_unfilter
        movq %rsp,%rsi  # stack pointer at execve
        call zfind  # %rsi= &env
        call zfind  # %rsi= &Elf64_auxv
        subq $ OVERHEAD,%rsp
        movq %rsp,%arg3  # &ELf64_Ehdr temporary space
        push %rsi  # arg7
        movq $ PAGE_MASK,%rcx  # sign extend 32 bits to 64 bits
        movl -9(%rbp),%arg2l  # total size - offset to {l_info; p_info; b_info}
        movq %arg6,%r15; andq %rcx,%r15  # %r15= &this_page
        movq %rbp,%arg5  # &decompress: f_expand
        movq %rbp,%arg1; subq %arg2,%arg1  # &l_info
        movq %arg1,%r14; andq %rcx,%r14  # %r14= our_Elf64_Ehdr
        movl sz_unc+sz_p_info+sz_l_info(%arg1),%arg4l  # sz_elf_headers
        call upx_main  # Out: %rax= entry
/* entry= upx_main(l_info *arg1, total_size arg2, Elf64_Ehdr *arg3, sz_ehdr arg4,
        f_decompr arg5, f_unfilter arg6, Elf32_Auxv_t *arg7 )
*/
        addq $8+OVERHEAD,%rsp
        push %rax  # save &entry
        movq %r14,%arg1  # &our_Elf64_Ehdr
        movq %r15,%arg2
        subq %r14,%arg2  # size
        call munmap  # unmap compressed program; /proc/self/exe disappears
        ret

/* 64-bit mode only! */
__NR_read=  0
__NR_open=  2
__NR_close= 3

__NR_mmap=      9
__NR_mprotect= 10
__NR_munmap=   11
__NR_brk=      12

__NR_exit= 60

mmap: .globl mmap
        movq %arg4,%sys4
        movb $ __NR_mmap,%al
sysgo:  # NOTE: kernel demands 4th arg in %sys4, NOT %arg4
        movzbl %al,%eax
        syscall
        cmpq $ PAGE_MASK,%rax; jc no_fail
        orq $~0,%rax  # failure; IGNORE errno
no_fail:
        ret

read: .globl read
        movb $ __NR_read,%al; jmp sysgo
open: .globl open
        movb $ __NR_open,%al; jmp sysgo
close: .globl close
        movb $ __NR_close,%al; jmp sysgo

mprotect: .globl mprotect
        movb $ __NR_mprotect,%al; jmp sysgo
munmap: .globl munmap
        movb $ __NR_munmap,%al; jmp sysgo
brk: .globl brk
        movb $ __NR_brk,%al; jmp sysgo

exit: .globl exit
        movb $ __NR_exit,%al; jmp sysgo

# vi:ts=8:et:nowrap
