/*  amd64-linux.elf-fold.S -- linkage to C code to process Elf binary
*
*  This file is part of the UPX executable compressor.
*
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
#include "arch/amd64/regs.h"

PATH_MAX= 4096  // /usr/include/linux/limits.h
PAGE_SHIFT= 12  // /usr/include/sys/user.h
PAGE_SIZE=  1 << PAGE_SHIFT
PAGE_MASK= ~0 << PAGE_SHIFT

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4

sz_l_info= 12
sz_p_info= 12

MAP_PRIVATE=   0x02
MAP_FIXED=     0x10

PROT_READ=     0x1

O_RDONLY=       0

OVERHEAD=2048
MAX_ELF_HDR=1024

/* 64-bit mode only! */
__NR_read=  0
__NR_write= 1
__NR_open=  2
__NR_close= 3

__NR_mmap=      9
__NR_mprotect= 10
__NR_munmap=   11
__NR_brk=      12

__NR_exit= 60
__NR_readlink= 89

// IN: [ADRX,+LENX): compressed data; [ADRU,+LENU): expanded fold (w/ upx_main)
// %rbx= 4+ &O_BINFO; %rbp= f_exp; %r14= ADRX; %r15= LENX;
// rsp/ elfaddr,fd,ADRU,LENU,rdx,%entry,  argc,argv,0,envp,0,auxv,0,strings

fold_begin:
////    int3  // DEBUG only
        call L90
#include "arch/amd64/bxx.S"
L90:
        pop %rax  # &f_unf
        push %r14  // ADRX
        push %r15  // LENX
        push %rax  # &amdbxx: f_unf
        mov %rsp,%rsi
        sub $PATH_MAX,%rsp
        push %rsp; pop %rdi
        push $10; pop %rcx; rep movsq  # f_unf,LENX,ADRX,elfaddr,fd,ADRU,LENU,rdx,%entry,argc
0:
        cmpq $0,(%rsi); movsq; jne 0b  # move past argv
        movq %rdi,%r14  # remember &new_env[0]
        stosq  # space for new_env[0]
0:
        cmpq $0,(%rsi); movsq; jne 0b  # move past env
        mov %rdi,%r12  # &old Elf64_auxv
0:
        cmpq $0,(%rsi); movsq; movsq; jne 0b  # move past auxv
        mov %rdi,%r15  # beyond auxv
        mov %rsi,%r13  # beginning of strings
        sub %r15,%r12  # -length of auxv

        movl $-1+ PATH_MAX,%arg3l # buflen
        push %rdi; pop %arg2  #buffer
        lea -4-15(%rbx),%arg1 # "/proc/self/exe"
        push $ __NR_readlink; pop %rax; syscall; testl %eax,%eax; jns 0f
// readlink() failed. Set the result equal to the argument.
        push %arg1; pop %arg2  # result= "/proc/self/exe"
        push $15; pop %rax  # 1+ strlen( )
0:
        xchg %eax,%ecx  # %ecx= byte count
     std
        lea -1(%r13),%rdi  # dst last byte
        movb $0,%al; stosb  # terminate
        lea -1(%arg2,%rcx),%rsi  # src last byte
        rep movsb  # slide up
        sub $3,%rdi; movl $('='<<24)|(' '<<16)|(' '<<8)|(' '<<0),(%rdi)  # env var name
        mov %rdi,(%r14)  # new_env[0]
        and $~0<<3,%rdi  # align
        mov %r15,%rcx
        sub %rsp,%rcx  # byte count

        mov %ecx,%eax
        xor %edi,%eax
        xor   $NBPW,%eax
        and   $NBPW,%eax
        sub %rax,%rdi  # keep 16-byte alignment of %rsp

        mov %rdi,%r14  # end of new auxv
        sub $NBPW,%rdi        # &last qword of new auxv
        lea -NBPW(%r15),%rsi  # &last qword of old auxv
        shr   $3,%rcx; rep movsq
        lea NBPW(%rdi),%rsp
     cld

        lea (%r14,%r12),%arg4  # &new Elf64_auxv
        movl -4(%rbx),%eax; and $1,%eax; add %rax,%arg4  # is_ptinterp
        pop %arg6  # f_unf
        pop %arg2  # LENX
        pop %arg1  # ADRX

        pop %rax  # elfaddr
        subq $ OVERHEAD,%rsp
        movq %rsp,%arg3  # &ELf64_Ehdr temporary space
        push %rax; mov %rax,%r13  # elfaddr  7th arg

        movq %rbp,%arg5  # &decompress: f_expand
        call upx_main  # Out: %rax= entry
/* entry= upx_main(b_info *arg1, total_size arg2, Elf64_Ehdr *arg3,
                Elf32_Auxv_t *arg4, f_decompr arg5, f_unf arg6,
                Elf64_Addr elfaddr )
*/
// rsp/ elfaddr,{OVERHEAD},fd,ADRU,LENU,rdx,%entry,  argc,argv,0,envp,0,auxv,0,strings
        addq $1*NBPW+OVERHEAD,%rsp  # also discard elfaddr
        movq %rax,4*NBPW(%rsp)  # entry
        pop %rbx  # fd

sz_Ehdr= 8*NBPW
sz_Phdr= 7*NBPW
p_memsz= 4+4+ 4*NBPW
// Discard pages of compressed data (includes [ADRX,+LENX) )
        movq p_memsz+sz_Phdr+sz_Ehdr(%r13),%arg2  #   Phdr[C_TEXT= 1].p_memsz
        movq %r13,%arg1  # hi elfaddr
        call munmap  # discard C_TEXT compressed data

// Map 1 page of /proc/self/exe so that the symlink does not disappear.
        subq %arg6,%arg6  # 0 offset
        mov %ebx,%arg5l  # fd
        push $MAP_PRIVATE; pop %arg4
        push $PROT_READ; pop %arg3
        mov $PAGE_SIZE,%arg2l
        subl %arg1l,%arg1l  # 0
        call mmap

        mov %ebx,%edi  # fd
        call close

        pop %arg1  # ADRU: unfolded upx_main etc.
        pop %arg2  # LENU
        push $__NR_munmap; pop %rax
        jmp *-NBPW(%r14)  # goto: syscall; pop %rdx; ret

mmap: .globl mmap
        movb $ __NR_mmap,%al
        movq %arg4,%sys4
sysgo:  # NOTE: kernel demands 4th arg in %sys4, NOT %arg4
        movzbl %al,%eax
        syscall
        cmpq $ PAGE_MASK,%rax; jc no_fail
        orq $~0,%rax  # failure; IGNORE errno
no_fail:
        ret

exit: .globl exit
        movb $ __NR_exit,%al; 5: jmp 5f
brk: .globl brk
        movb $ __NR_brk,%al; 5: jmp 5f
close: .globl close
        movb $ __NR_close,%al; 5: jmp 5f
open: .globl open
        movb $ __NR_open,%al; 5: jmp 5f
munmap: .globl munmap
        movb $ __NR_munmap,%al; 5: jmp 5f
mprotect: .globl mprotect
        movb $ __NR_mprotect,%al; 5: jmp 5f
write: .globl write
        mov $__NR_write,%al; 5: jmp 5f
read: .globl read
        movb $ __NR_read,%al; 5: jmp sysgo

/* vim:set ts=8 sw=8 et: */
