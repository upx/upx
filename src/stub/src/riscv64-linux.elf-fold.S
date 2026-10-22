/*  riscv64-linux.elf-fold.S -- linkage to C code to process Elf binary
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) John F. Reiser
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
#include "arch/riscv/64/macros.S"
#include "arch/riscv/64/regs.h"
#include "../../MAX_ELF_HDR.h"

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
FD_CWD= -100

/* 64-bit mode only! */
__NR_brk=       214
__NR_close=      57
__NR_exit=       93
__NR_ftruncate=  46
__NR_memfd_create= 279
__NR_mmap=      222
__NR_mprotect=  226
__NR_msync=     227
  MS_SYNC= 4
__NR_munmap=    215
__NR_openat=     56
__NR_read=       63
__NR_readlinkat= 78
__NR_write=      64

unmap_all_pages= (1<<1)
is_ptinterp=     (1<<0)

// same as in riscv64-linux.elf-entry.S
F_FRAME= 8*NBPW
F_EOFDST=7*NBPW
F_EOFSRC=6*NBPW; F_ENTR= F_EOFSRC
F_RDX=  5*NBPW
F_LENU= 4*NBPW  // length of Uncompressed stub
F_ADRU= 3*NBPW; F_PMASK= F_ADRU
F_ELFA= 2*NBPW
 F_MFD= 1*NBPW + 4  // 32-bit file descriptor
F_LENC= 1*NBPW  // 32-bit length of Compressed stub
F_ADRC= 0*NBPW

D_FOLD=  2*NBPW  // .data space at start of unfold
D_PMASK= 0*NBPW
D_XSIGSEGV= 1*NBPW

#define call jal

#define rdi a5  /* x15    arg6 */
#define rsi a4  /* x14    arg5 */
#define tb  a3  /* x13    arg4 */
#define ta  a2  /* x12    arg3 */
#define rdx a1  /* x11    arg2 */
#define rax a0  /* x10 rv arg1 */
#define rbp s0  /* x8     fp   */

#define naux x5

        //.balign 8
get_page_mask: .globl get_page_mask
L_get_page_mask:
        lui rv,0xfffff  // default 4K
        ret
        .int 0

cancel_sigsegv: .quad 0  // subroutine that cancels sigaction(SIGSEGV,)

// IN: [ADRX,+LENX): compressed data; [ADRU,+LENU): expanded fold (w/ upx_main2)
// %sp= %rbp= &F_ADRC

// no 'section', thus '.text'; also loaded first in riscv64-linux.elf-fold.bin.
// Code from riscv64-linux2.elf-main.c is also .text, and is next.
fold_begin:
        mv rbp,sp
        j 6f

Pprotect: .globl Pprotect
        li a7,__NR_mprotect
        j 3f

Punmap: .globl Punmap  // page-align the lo end
munmap: .global munmap
L_munmap:
        li a7,__NR_munmap
        j 3f

Pmap: .globl Pmap  // page-align the lo end
mmap: .globl mmap
L_mmap:
        li a7,__NR_mmap
3:
Palign:
        mv t1,arg1  # arg1 clobbered by rv
        mv t0,ra
            call L_get_page_mask; xori t2,rv,~0  // frag mask
        mv ra,t0
        mv arg1,t1
        and t2,t2,arg1  // frag
        sub arg1,arg1,t2
        add arg2,arg2,t2
        j sysgo

6: // (fold begin) F_ADRC(rbp) has is_ptinerp | unmap_all_pages
        ld s1,F_ADRC(rbp)

// if !unmap_all_pages then need space on stack for result of readlinkat("/proc/self/exe",...)
        andi ta,s1,unmap_all_pages; bnez ta,no_PSE

        li ta,2*PATH_MAX + NBPW + 4 +1
        sub ta,sp,ta; andi sp,ta,-2*NBPW
        jal rdi,0f
proc_self_exe:
        .asciz "/proc/self/exe"
        .balign 4
0:
        li arg3,O_RDONLY
        mv arg2,rdi  // "proc_self_exe"
        call openat_CWD; sw rv,F_MFD(rbp)

        li arg4,PATH_MAX  # buflen
        mv arg3,sp  # buffer
        mv arg2,rdi  # "/proc/self/exe"
        li arg1,FD_CWD
        syscall __NR_readlinkat; bgez rv,0f
// readlinkat() failed. Set the result equal to the argument.
        mv arg2,rdi  # failure result= "/proc/self/exe"
        li rv,15  # sizeof "/proc/self/exe"
0:

//
        sub rdi,rbp,rv
        addi rdi,rdi,-(NBPW + 4 + 1)  # new env_var, strlen("   ="), terminator
        andi rdi,rdi,-2*NBPW  # 16-byte align
// copy down F_FRAME
        mv rsi,rbp  # current frame
        mv rbp,rdi  # new frame
        add rdx,rdi,F_FRAME  # limit
0:
        ld ta,0(rsi); addi rsi,rsi,NBPW
        sd ta,0(rdi); addi rdi,rdi,NBPW; bne rdx,rdi,0b
// copy down argc,argv
        ld ta,0(rsi); addi rsi,rsi,NBPW  # argc
        sd ta,0(rdi); addi rdi,rdi,NBPW
        addi ta,ta,1
        slli ta,ta,3
        add rdx,rdi,ta  # limit
0:
        ld ta,0(rsi); addi rsi,rsi,NBPW
        sd ta,0(rdi); addi rdi,rdi,NBPW; bne rdx,rdi,0b
// rdx= &new env_var
        sd zero,(rdi); addi rdi,rdi,NBPW
// copy down env
0:
        ld ta,0(rsi); addi rsi,rsi,NBPW
        sd ta,0(rdi); addi rdi,rdi,NBPW; bnez ta,0b
// copy down auxv
        mv naux,rdi
0:
        ld ta,0(rsi); ld tb,NBPW(rsi); addi rsi,rsi,2*NBPW
        sd ta,0(rdi); sd tb,NBPW(rdi); addi rdi,rdi,2*NBPW; bnez ta,0b
// new env_var
        sd  rdi,0(rdx)
        li ta,' '; sb ta,0(rdi); sb ta,1(rdi); sb ta,2(rdi)
        li ta,'='; sb ta,3(rdi); add rdi,rdi,4
        mv rsi,sp  # filename
        add rdx,rdi,rv  # limit of result of readlink()
// copy up text of new env_var
0:
        lbu ta,0(rsi); addi rsi,rsi,1
        sb  ta,0(rdi); addi rdi,rdi,1; bne rdx,rdi,0b
        sb zero,0(rdi)  # terminate
// de-allocate result of readlink
        mv sp,rbp

no_PSE:
        ld  arg1,F_ADRC(rbp)
        lwu arg2,F_LENC(rbp)
        andi arg1,arg1,~(is_ptinterp | unmap_all_pages)

        ld arg5,F_ELFA(rbp)  # ELFA | is_ptinterp
        andi ta,arg5,is_ptinterp; or arg4,naux,ta  # transfer is_ptinterp to &new_ELF64_auxv
        sub arg5,arg5,ta  # and clear from  ELFA
        mv  s1,arg5  # save ELFA
        li ta,MAX_ELF_HDR_64 >> 8; sll ta,ta,8
        sub sp,sp,ta  // alloc Elf64_Ehdr temporary space
        mv arg3,sp  # &ELf64_Ehd temporary space
        call upx_main2  # Out: rv= entry

// entry= upx_main2(
// b_info *arg1,
// total_size arg2,
// Elf64_Ehdr *arg3
// Elf32_Auxv_t *arg4
// Elf64_Addr elfaddr
// )

// sp/ {MAX_ELF_HDR_64},ADRU,LENU,rdx,%entry,  argc,argv,0,envp,0,auxv,0,strings
        li ta,MAX_ELF_HDR_64 >> 8; sll ta,ta,8
        add sp,sp,ta  # remove Elf64_Ehdr temporary space
        sd rv,F_ENTR(sp)  # entry

        //NYI call *cancel_sigsegv(%rip)

#if 0  // { OLD?  subsumed by brk(v_brk) in upx_main2
sz_Ehdr= 8*NBPW
e_type= 16
ET_EXEC= 2
sz_Phdr= 7*NBPW
p_memsz= 5*NBPW
// Discard pages of compressed data (includes [ADRX,+LENX) )
        //cmpw $ET_EXEC, e_type(%r13); jne 0f
        mv arg1,s1; call brk  // also sets the brk
0:
        ld arg2,p_memsz+sz_Phdr+sz_Ehdr(s1)  #   Phdr[C_TEXT= 1].p_memsz
        mv arg1,s1; call L_munmap  # discard C_TEXT compressed data
#endif  // }

// Map 1 page of /proc/self/exe so that the symlink does not disappear.
        lwu ta,F_MFD(rbp); bltz ta,no_pse_map
        li arg6,0  # 0 offset
        lwu arg5,F_MFD(rbp)
        li arg4,MAP_PRIVATE
        li arg3,PROT_READ
        li arg2,1<<12
        li arg1,0
        call L_mmap

        lwu arg1,F_MFD(rbp)
        call L_close

no_pse_map:
        ld arg1,F_ADRU(rbp)  # ADRU: unfolded upx_main2 etc.
        lwu arg2,F_LENU(rbp)
        li a7,__NR_munmap

        ld x15,F_ENTR(rbp)
        addi sp,sp,F_FRAME
        mv x14,sp
// skip argc,argv
        ld x13,(x14)  # argc
        addi x13,x13,2  # argc,argv,0
        sll x13,x13,3  # *NBPW
        add x14,x14,x13
// skip env
0:
        ld x13,(x14); addi x14,x14,NBPW; bnez x13,0b
// skip auxv
0:
        ld x13,(x14); addi x14,x14,2*NBPW; bnez x13,0b

        ld  x14,-NBPW(x14)  # &hatch
// Why is this store prohibited by no PF_W access?
//        sd zero,-NBPW(x14)  # clean the hatch
// Also, ld.so complains rtld.c: 1683: dl_main: GL(dl_rtld_map).l_libname->next != NULL
        mv x31,sp
        li x30,1<<20
        sub sp,sp,x30
        mv x30,sp
0:
        sd zero,0(x30); addi x30,x30,NBPW
        bne x30,x31,0b
        mv sp,x31

        jalr x14  # escape to:
        // ecall  # munmap
        // jr x15  # goto entry

        section SYSCALLS
my_bkpt: .globl my_bkpt
        ebreak  // my_bkpt
        ret

memset: .globl memset  // memset(ptr, c, n)
        beqz arg3,9f
        add arg3,arg3,arg1  // sentinel
        mv tb,arg1  // keep rv == arg1
0:
        sb arg2,0(tb); addi tb,tb,1
        bne arg3,tb,0b
9:
        ret

memcpy: .globl memcpy  // memcpr(dst, src, n)
        beqz arg3,9f
        mv x14,arg2  // src
        mv x15,arg1  // dst
        add x13,arg3,x14  // end x14
0:
        lb x12,0(x14); addi x14,x14,1
        sb x12,0(x15); addi x15,x15,1
        bne x14,x13,0b
9:
        ret

// Bare syscall memfd_create() for upx_mmap_and_fd(), which does
// its own retry with no MFD_EXEC and fallback to /dev/shm,
// plus calling openat() and ftruncate() and mmap()
// Allowed to fail, so do not trap on failure.
memfd_create: .globl memfd_create; syscall __NR_memfd_create; ret

upxfd_create: .globl upxfd_create // (char *tag, unsigned flags)
// try memfd_create
        syscall __NR_memfd_create; j ok_memfd  //NYI
O_RDWR= 2
O_DIRECTORY= 0200000  // 0x010000
O_TMPFILE= 020000000  // 0x400000
        li arg4,0700  # mode
        li arg3,O_RDWR|O_DIRECTORY|O_TMPFILE
        jal arg2, openat_CWD
        .asciz "/dev/shm"; .balign 4
ok_memfd:
        ret

sysgo:
sys_check:
        ecall; bltz rv,0f
        ret
0:
        ebreak; j 0b  # sys_check error


Psync: .globl Psync
        li a7,__NR_msync
        j sysgo

open: .globl open
        mv arg4,arg3
        mv arg3,arg2
        mv arg2,arg1
openat_CWD:
        li arg1,FD_CWD
openat: .globl openat
L_openat:
        li a7,__NR_openat;       j 0f
exit: .globl exit
        li a7,__NR_exit;      0: j 0f
brk: .globl brk
        li a7,__NR_brk;       0: j 0f
close: .globl close
L_close:
        li a7,__NR_close;     0: j 0f
ftruncate: .globl ftruncate
        li a7,__NR_ftruncate; 0: j 0f
mprotect: .globl mprotect
        li a7,__NR_mprotect;  0: j 0f
msync: .globl msync
        li a7,__NR_msync;     0: j 0f
Pwrite: .globl Pwrite
write: .globl write
        li a7,__NR_write;     0: j 0f
read: .globl read
        li a7,__NR_read;      0: j sysgo

// vim:set ts=8 sw=8 et:
