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

/* 64-bit mode only! */
__NR_read=  0
__NR_write= 1
__NR_open=  2
__NR_close= 3

__NR_mmap=      9
__NR_mprotect= 10
__NR_munmap=   11
__NR_brk=      12
__NR_memfd_create= 0x13f  // 319
__NR_ftruncate= 0x4d  // 77

__NR_exit= 60
__NR_readlink= 89

F_FRAME= 7*NBPW
F_ENTR= 6*NBPW; F_PMASK= F_ENTR
F_RDX=  5*NBPW
F_LENU= 4*NBPW
F_ADRU= 3*NBPW
F_ELFA= 2*NBPW
F_LENX= 1*NBPW
F_ADRX= 0*NBPW

        .balign 4
PAGE_MASK: .quad -1<<12  // default

// IN: [ADRX,+LENX): compressed data; [ADRU,+LENU): expanded fold (w/ upx_main)
// %rsp= %rbp= &F_ADRX; %r13= O_BINFO | is_ptinterp

// no 'section', thus '.text'; also loaded first in amd64-linux.elf-fold.bin.
// Code from amd64-linux.elf-main.c is also .text, and is next.
fold_begin:
        mov %r13,%rax; and $1,%eax; or %eax,F_ELFA(%rbp)  // is_ptinterp; %r13 dead
        mov %rsp,%rsi
        sub $PATH_MAX,%rsp
        mov %rsp,%rdi
        push $8; pop %rcx; rep movsq  # ADRX,LDRX,ELFA,ADRU,LENU,rdx,%entry,argc
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

          push %rdi  # buffer
        lea proc_self_exe(%rip),%arg1; sub %arg2l,%arg2l  # O_RDONLY
        call open; mov %eax,%ebp  # fd
          pop %arg2  #buffer

        movl $-1+ PATH_MAX,%arg3l # buflen
        push $ __NR_readlink; pop %rax; syscall; testl %eax,%eax; jns 0f
// readlink() failed. Set the result equal to the argument.
        push %arg1; pop %arg2  # failure result= "/proc/self/exe"
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
        and $-NBPW,%rdi  # align
        mov %r15,%rcx
        sub %rsp,%rcx  # byte count

        mov %rdi,%r14  # end of new auxv
        sub $NBPW,%rdi        # &last qword of new auxv
        lea -NBPW(%r15),%rsi  # &last qword of old auxv

        // end of auxv must move by even number of NBPW
        mov %edi,%eax
        xor %esi,%eax
        and $NBPW,%eax
        sub %rax,%rdi

        shr   $3,%rcx; rep movsq
        lea NBPW(%rdi),%rsp
     cld

        lea (%r14,%r12),%arg4  # &new Elf64_auxv  %r12 dead
        pop %arg1  # ADRX
        pop %arg2  # LENX

        pop %arg5  # ELFA | is_ptinterp
        mov %arg5l,%eax; and $1,%eax  # is_ptinterp
        or  %rax,%arg4  # transfer is_ptinterp to &new_ELF64_auxv
        sub %rax,%arg5  # and clear from  ELFA
        mov %arg5,%r13  # save ELFA
        subq $ OVERHEAD,%rsp
        movq %rsp,%arg3  # &ELf64_Ehdr temporary space
        call upx_main  # Out: %rax= entry
/* entry= upx_main(
   b_info *arg1,       {%rdi}
   total_size arg2,    {%rsi}
   Elf64_Ehdr *arg3,   {%rdx}
   Elf32_Auxv_t *arg4, {%rcx}
   Elf64_Addr elfaddr  {%r8}
   )
*/
// rsp/ {OVERHEAD},ADRU,LENU,rdx,%entry,  argc,argv,0,envp,0,auxv,0,strings
        addq $OVERHEAD,%rsp  # Elf64_Ehdr temporary space
        mov       %ebp,%ebx  # fd
        movq %rax,3*NBPW(%rsp)  # entry

sz_Ehdr= 8*NBPW
e_type= 16
ET_EXEC= 2
sz_Phdr= 7*NBPW
p_memsz= 5*NBPW
// Discard pages of compressed data (includes [ADRX,+LENX) )
        movq p_memsz+sz_Phdr+sz_Ehdr(%r13),%arg2  #   Phdr[C_TEXT= 1].p_memsz
        //cmpw $ET_EXEC, e_type(%r13); jne 1f
        movq %r13,%arg1; call brk  // also sets the brk
1:
        movq %r13,%arg1; call munmap  # discard C_TEXT compressed data

// Map 1 page of /proc/self/exe so that the symlink does not disappear.
        test %ebx,%ebx; js no_pse_map
        subq %arg6,%arg6  # 0 offset
        mov %ebx,%arg5l  # fd
        push $MAP_PRIVATE; pop %arg4
        push $PROT_READ; pop %arg3
        mov $1<<12,%arg2l
        subl %arg1l,%arg1l  # 0
        call mmap

        mov %ebx,%edi  # fd
        call close
no_pse_map:
        pop %arg1  # ADRU: unfolded upx_main etc.
        pop %arg2  # LENU
        push $__NR_munmap; pop %rax
        jmp *-NBPW(%r14)  # goto: syscall; pop %rdx; ret

get_page_mask: .globl get_page_mask
        mov PAGE_MASK(%rip),%rax
        ret

        section SYSCALLS
my_bkpt: .globl my_bkpt
        int3  // my_bkpt
        ret

proc_self_exe:
        .asciz "/proc/self/exe"

memfd_create: .globl memfd_create
        mov $__NR_memfd_create,%eax; jmp sysgo2

Pmap: .globl Pmap  // page-align the lo end
        mov PAGE_MASK(%rip),%rax; not %rax  // frag mask
        and %arg1l,%eax  // frag
        sub %rax,%arg1
        add %rax,%arg2
mmap: .globl mmap
        movb $ __NR_mmap,%al
        movq %arg4,%sys4
sysgo:  # NOTE: kernel demands 4th arg in %sys4, NOT %arg4
        movzbl %al,%eax
sysgo2:
        push %rax  // save __NR_
        syscall  // %rax= -errno
        pop %rcx  // recover __NR_
        cmp $-1<<12,%rax; jb sysOK
        cmp $__NR_open,%ecx; je sysOK  # ENOENT etc
        int3  // %rax= errno; %rcx= __NR_
sysOK:
        ret

Pprotect: .globl Pprotect
        mov PAGE_MASK(%rip),%eax; not %eax  // frag mask
        and %arg1l,%eax  // frag
        sub %rax,%rdi
        add %rax,%rsi
        jmp mprotect

Punmap: .globl Punmap  // page-align the lo end
        mov PAGE_MASK(%rip),%eax; not %eax  // frag mask
        and %arg1l,%eax  // frag
        sub %rax,%arg1
        add %rax,%arg2
munmap: .globl munmap
        movb $ __NR_munmap,%al; 5: jmp 5f
exit: .globl exit
        movb $ __NR_exit,%al; 5: jmp 5f
brk: .globl brk
        movb $ __NR_brk,%al; 5: jmp 5f
close: .globl close
        movb $ __NR_close,%al; 5: jmp 5f
ftruncate: .globl ftruncate
        movb $__NR_ftruncate,%al; 5: jmp 5f
open: .globl open
        movb $ __NR_open,%al; 5: jmp 5f
mprotect: .globl mprotect
        movb $ __NR_mprotect,%al; 5: jmp 5f
Pwrite: .globl Pwrite
write: .globl write
        mov $__NR_write,%al; 5: jmp 5f
read: .globl read
        movb $ __NR_read,%al; 5: jmp sysgo

/* vim:set ts=8 sw=8 et: */
