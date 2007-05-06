/*  mipsel-linux.elf-fold.S -- linkage to C code to process Elf binary
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 2000-2007 John F. Reiser
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

#include "arch/mips/mipsel.r3000/macros.ash"
#include "arch/mips/mipsel.r3000/bits.ash"

PAGE_SHIFT= 12
PAGE_MASK= 0xffffffffffffffff<<PAGE_SHIFT

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4

sz_l_info= 12
sz_p_info= 12

OVERHEAD=2048
MAX_ELF_HDR=1024

SYS_read=  3
SYS_open=  5
SYS_close= 6

SYS___syscall= 198
SYS_mmap=    197
SYS_mprotect= 74
SYS_munmap=   73

SYS_exit= 1
SYS_readlink= 58

        break
#if 0  /*{*/
/* In:
    cld
    %rbp= &decompress
    %rsp= &{LENX,ADRX,LENU,ADRU,JMPU,argc,argv...,0,env...,0,auxv...,0...,strings}
*/
fold_begin:
        call L90  # push &L90a
L90a:
        .asciz "/proc/self/exe"
L90b:
#include "arch/amd64/bxx.S"
L90:
        pop %arg6  # L90a; later, &amdbxx: f_unfilter
        movq %rsp,%rsi
        leaq -8 -4 -4 -4096(%rsi),%rdi  # 8:ptr, 4:"   =", 4:align, 4096:buffer
        movq %rdi,%rsp
0:
        cmpq $0,(%rsi); movsq; jne 0b  # move past argv
        movq %rdi,%arg3  # remember &new_env[0]
        stosq  # space for new_env[0]
0:
        cmpq $0,(%rsi); movsq; jne 0b  # move past env
        pushq %rdi  # &Elf64_auxv
0:
        cmpq $0,(%rsi); movsq; movsq; jne 0b  # move past auxv

        movq %rdi,(%arg3)  # new_env[0]
        movl $('='<<24)|(' '<<16)|(' '<<8)|(' '<<0),%eax  # "   ="
        stosl
        movl $4096,%arg3l # buflen
        movq  %rdi,%arg2  # buffer
        movq %arg6,%arg1  # "/proc/self/exe"
        push $ SYS_readlink; pop %rax
        syscall; testl %eax,%eax; js 0f; movb $0,(%arg2,%rax)
0:
        addq $ L90b - L90a,%arg6  # &amdbxx: f_unfilter
        pop %arg4  # &Elf64_auxv
        pop %arg2  # LENX
        pop %arg1  # ADRX

        subq $ OVERHEAD,%rsp
        movq %rsp,%arg3  # &ELf64_Ehdr temporary space
        movq %rbp,%arg5  # &decompress: f_expand
        call upx_main  # Out: %rax= entry
/* entry= upx_main(b_info *arg1, total_size arg2, Elf64_Ehdr *arg3,
                Elf32_Auxv_t *arg4, f_decompr arg5, f_unfilter arg6 )
*/
        addq $OVERHEAD,%rsp
        pop %arg2  # LENU
        pop %arg1  # ADRU
        pop %rcx   # JMPU
        push %rax  # &entry
        push $ SYS_munmap; pop %rax
        jmp *%rcx  # goto: syscall; ret

munmap: .globl munmap
        movb $ SYS_munmap,%al; jmp sysgo
mprotect: .globl mprotect
        movb $ SYS_mprotect,%al; jmp sysgo
brk: .globl brk
        movb $ SYS_brk,%al; jmp sysgo

mmap: .globl mmap
        movb $ SYS_mmap,%al
sysarg4:
        movq %arg4,%sys4
sysgo:  # NOTE: kernel demands 4th arg in %sys4, NOT %arg4
        movzbl %al,%eax
        syscall
        cmpq $ PAGE_MASK,%rax; jc no_fail
        orq $~0,%rax  # failure; IGNORE errno
no_fail:
        ret

read: .globl read
        movb $ SYS_read,%al; jmp sysgo
open: .globl open
        movb $ SYS_open,%al; jmp sysgo
close: .globl close
        movb $ SYS_close,%al; jmp sysgo

exit: .globl exit
        movb $ SYS_exit,%al; jmp sysgo
#endif  /*}*/

munmap: .globl munmap
        break
mprotect: .globl mprotect
        break
brk: .globl brk
        break
mmap: .globl mmap
        break
read: .globl read
        break
open: .globl open
        break
close: .globl close
        break
exit: .globl exit
        break

# vi:ts=8:et:nowrap
