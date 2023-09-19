/*  amd64-linux.elf-entry.S -- Linux program entry point & decompressor (Elf binary)
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

#include "arch/amd64/macros.S"
#include "arch/amd64/regs.h"
NBPW= 8
.macro          jmps    target
                .byte   0xeb, \target - . - 1
.endm

/* These from /usr/include/unistd_64.h */
__NR_memfd_create= 319  // 0x13f
__NR_ftruncate= 77
__NR_exit=     60
__NR_mprotect= 10
__NR_mmap=      9
__NR_close=     3
__NR_open=      2
__NR_write=     1

sz_Ehdr= 64
e_phnum= 56
sz_Phdr= 56

sz_l_info= 12
  l_lsize= 8

sz_p_info= 12

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

PROT_READ=  1
PROT_WRITE= 2
PROT_EXEC=  4

MAP_SHARED=  1
MAP_PRIVATE= 2
MAP_FIXED=     0x10
MAP_ANONYMOUS= 0x20

SYS_mmap= 9  // 64-bit mode only!

FD_stderr= 2

PAGE_SHIFT= 12
PAGE_MASK= (~0<<PAGE_SHIFT)
PAGE_SIZE= -PAGE_MASK

M_NRV2B_LE32=2  // ../conf.h
M_NRV2D_LE32=5
M_NRV2E_LE32=8


// https://www.uclibc.org/docs/psABI-x86_64.pdf
  section ELFMAINX
sz_pack2= .-4
_start: .globl _start
    nop; int3  # uncomment for debugging
        movq %rsp,%r13
        push %rax  // space for entry address
        push %rdx  // register this function pointer with 'atexit'

// Create anonymous temporary file on mfd
        push $'u'|('p'<<8)|('x'<<16)|(0<<24)
        push %rsp; pop %arg1  // "upx"
        subl %arg2l,%arg2l  // 0
        movl $__NR_memfd_create,%eax; call sys_check; mov %eax,%r12d  // mfd
        pop %rcx  // discard "upx"

// Decompress the rest of this loader, and jump to it.
        lea o_binfo(%rip),%rsi
        lodsl; and $~1,%eax; movl %eax,%r14d  // O_BINFO
        push %rsi; pop %rbx  // &b_info of folded decompressor
        movl (%rsi),%edx  // .sz_unc LENU

        lea sz_pack2(%rip),%rcx  // &sz_pack2
        movl (%rcx),%r15d  // sz_pack2: length before stub
        subq %r15,%rcx  // elfaddr= &Elf64_Ehdr of this stub
        subl %r14d,%r15d  // LENX= sz_pack2 - O_BINFO
        addq %rcx,%r14  // ADRX= elfaddr + O_BINFO
        push %rdx  // P_30  LENU
        push %rcx  // P_31  elfaddr

// alloca()
        subq %rdx,%rsp; and $-2*NBPW,%rsp  // aligned space for unfolded stub

        push %rsp; pop %arg3  // dst= decompress onto stack
        push %rbx; pop %rsi
        lodsl; push %rax  // P_13  .sz_unc
        push %rsp; pop %arg4  // &dstlen
        lodsl; push %rax  // P_14  tmp= .sz_cpr
        lodsl; xchg %eax,%arg5l  // .b_method
        movq %rsi,%arg1  // src
        pop %arg2  // P_14  srclen
        lea f_exp(%rip),%rbp
        call *%rbp  // f_exp

// Write de-compressed 'fold' to file
        push %r12; pop %arg1  // mfd
        pop %arg3  // P_13  LENU
        push %rsp; pop %arg2  // buffer
        push $__NR_write; pop %rax; call sys_check
// de-alloca()
        lea -4*NBPW(%r13),%rsp  // elfaddr,LENU,atexit.rdx,&entry

// Map unfolded code the SELinux way
        sub %arg6l,%arg6l  // 0  offset
        mov %r12,%arg5  // mfd
        push $MAP_SHARED; pop %sys4
        push %arg3; pop %arg2  // length
        push $PROT_READ|PROT_EXEC; pop %arg3
        subl %edi,%edi  // (%arg1)dst = 0;  // kernel chooses addr
        push $__NR_mmap; pop %rax; call sys_check
        pop %rcx  // P_31  elfaddr
        push %rax  // P_32  ADRU
        push $0  // P_33  %fd
        push %rcx  // P_34  elfaddr

        push %arg5; pop %arg1  // mfd
        push $__NR_close; pop %rax; syscall

// Use the copy.
// %r14= ADRX; %r15= LENX;
// rsp/ elfaddr,%fd,ADRU,LENU,atexit.rdx,%entry
        jmp * -4*NBPW(%r13)  // ADRU

sys_check:
        push %rax  // save __NR_ for debug
        syscall
        pop %rcx  // recover __NR_ for debug
        cmp $PAGE_MASK,%rax; jb 0f; hlt; 0:
        ret

here:  // OUT: %rdi= (2+ retaddrj) without disturbing shadow stack
        mov (%rsp),%rdi
        scasw  // over jmp.s
        ret

/* Returns 0 on success; non-zero on failure. */
f_exp:  // shorter name
decompress:  // (uchar const *src, size_t lsrc, uchar *dst, u32 &ldst, uint method)

/* Arguments according to calling convention */
#define src  %arg1
#define lsrc %arg2
#define dst  %arg3
#define ldst %arg4  /* Out: actually a reference: &len_dst */
#define meth %arg5l
#define methb %arg5b

        push %rbp; push %rbx  // C callable
        push ldst
        push dst
        addq src,lsrc; push lsrc  // &input_eof
        subq src,lsrc  // restore the value of lsrc

  section NRV_HEAD

/* Working registers */
#define off  %eax  /* XXX: 2GB */
#define len  %ecx  /* XXX: 2GB */
#define lenq %rcx
#define bits %ebx
#define displ %ebp
#define dispq %rbp

        movq src,%rsi  // hardware src for movsb, lodsb
        movq dst,%rdi  // hardware dst for movsb
        xor bits,bits  // empty; force refill
        xor len,len  // create loop invariant
        orq $(~0),dispq  // -1: initial displacement
        call setup  // push &getbit [TUNED]
ra_setup:

/* AMD64 branch prediction is much worse if there are more than 3 branches
   per 16-byte block.  The jnextb would suffer unless inlined.  getnextb is OK
   using closed subroutine to save space, and should be OK on cycles because
   CALL+RET should be predicted.  getnextb could partially expand, using closed
   subroutine only for refill.
*/
/* jump on next bit {0,1} with prediction {y==>likely, n==>unlikely} */
/* Prediction omitted for now. */
/* On refill: prefetch next byte, for latency reduction on literals and offsets. */
#define jnextb0np jnextb0yp
#define jnextb0yp GETBITp; jnc
#define jnextb1np jnextb1yp
#define jnextb1yp GETBITp; jc
#define GETBITp \
        addl bits,bits; jnz 0f; \
        movl (%rsi),bits; subq $-4,%rsi; \
        adcl bits,bits; movb (%rsi),%dl; \
0:
/* Same, but without prefetch (not useful for length of match.) */
#define jnextb0n jnextb0y
#define jnextb0y GETBIT; jnc
#define jnextb1n jnextb1y
#define jnextb1y GETBIT; jc
#define GETBIT \
        addl bits,bits; jnz 0f; \
        movl (%rsi),bits; subq $-4,%rsi; \
        adcl bits,bits; \
0:

/* rotate next bit into bottom bit of reg */
#define getnextbp(reg) call *%r11; adcl reg,reg
#define getnextb(reg)  getnextbp(reg)


getbit:
        addl bits,bits; jz refill  // Carry= next bit
        rep; ret
refill:
        movl (%rsi),bits; subq $-4,%rsi  // next 32 bits; set Carry
        adcl bits,bits  // LSB= 1 (CarryIn); CarryOut= next bit
        movb (%rsi),%dl  // speculate: literal, or bottom 8 bits of offset
        rep; ret

copy:  // In: len, %rdi, dispq;  Out: 0==len, %rdi, dispq;  trashes %rax, %rdx
        leaq (%rdi,dispq),%rax; cmpl $5,len  // <=3 is forced
        movb (%rax),%dl; jbe copy1  // <=5 for better branch predict
        cmpq $-4,dispq;   ja  copy1  // 4-byte chunks would overlap
        subl $4,len  // adjust for termination cases
copy4:
        movl (%rax),%edx; addq $4,      %rax; subl $4,len
        movl %edx,(%rdi); leaq  4(%rdi),%rdi; jnc copy4
        addl $4,len; movb (%rax),%dl; jz copy0
copy1:
        incq %rax; movb %dl,(%rdi); subl $1,len
                   movb (%rax),%dl
        leaq 1(%rdi),%rdi;          jnz copy1
copy0:
        rep; ret

setup:
        cld
        pop %r11  // addq $ getbit - ra_setup,%r11  # &getbit

  section NRV2E
#include "arch/amd64/nrv2e_d.S"

  section NRV2D
#include "arch/amd64/nrv2d_d.S"

  section NRV2B
#include "arch/amd64/nrv2b_d.S"

#include "arch/amd64/lzma_d.S"

  section NRV_TAIL
        // empty

  section ELFMAINY
eof:
        pop %rcx  // &input_eof
        movq %rsi,%rax; subq %rcx,%rax  // src -= eof;  // return 0: good; else: bad
        pop %rdx;       subq %rdx,%rdi  // dst -= original dst
        pop %rcx;            movl %edi,(%rcx)  // actual length used at dst  XXX: 4GB
        pop %rbx; pop %rbp
        ret

        // IDENTSTR goes here

  section ELFMAINZ
        .balign 4
o_binfo:
        .long O_BINFO  // offset of b_info for .text
FOLD:
        // { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...}

/*__XTHEENDX__*/

/* vim:set ts=8 sw=8 et: */
