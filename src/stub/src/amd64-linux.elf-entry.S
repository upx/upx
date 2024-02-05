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

AT_PAGESZ= 6

PROT_READ=  1
PROT_WRITE= 2
PROT_EXEC=  4

MAP_SHARED=  1
MAP_PRIVATE= 2
MAP_FIXED=     0x10
MAP_ANONYMOUS= 0x20

SYS_mmap= 9  // 64-bit mode only!

FD_stderr= 2

M_NRV2B_LE32=2  // ../conf.h
M_NRV2D_LE32=5
M_NRV2E_LE32=8


// https://www.uclibc.org/docs/psABI-x86_64.pdf
  section ELFMAINX
sz_pack2= .-4
_start: .globl _start
////    nop; int3  # uncomment for debugging

        pop %rcx  // argc
        push %rsp; pop %rdi  // argv
        push %rcx  // argc

#define old_sp %rbp
F_FRAME= 7*NBPW
F_ENTR= 6*NBPW; F_PMASK= F_ENTR
F_RDX=  5*NBPW
F_LENU= 4*NBPW
F_ADRU= 3*NBPW
F_ELFA= 2*NBPW
F_LENX= 1*NBPW
F_ADRX= 0*NBPW

D_FOLD=  1*NBPW  // .data space at start of unfold
D_PMASK= 0*NBPW

AT_PAGESZ= 6  // /usr/include/elf.h
        call zfind  // skip argv
        call zfind  // skip env
0: // find AT_PAGESZ in auxv
        mov     (%rdi),%rax
        mov NBPW(%rdi),%rcx; add $2*NBPW,%rdi
        cmp $AT_PAGESZ,%rax; je 5f
        mov $1<<12,%rcx  // default PAGE_SIZE
        test %rax,%rax; jne 0b
5:
        neg %rcx; push %rcx  // F_PMASK
        push %rdx  // F_RDX

// Create anonymous temporary file on mfd
        push $'u'|('p'<<8)|('x'<<16)|(0<<24)
        push %rsp; pop %arg1  // "upx"
        subl %arg2l,%arg2l  // 0
        movl $__NR_memfd_create,%eax; call sys_check; mov %rax,%r12  // mfd
        pop %rcx  // discard "upx"

        lea sz_pack2(%rip),%rdi
        mov (%rdi),%ecx  // sz_pack2: length before stub
        sub %rcx,%rdi  // elfaddr
        lea o_binfo(%rip),%rsi
        lodsl; xchg %eax,%ebx; mov %ebx,%r13d  // O_BINFO; advance to &b_info
        lodsl; xchg %eax,%edx; add $0x10,%edx; push %rdx  // F_LENU = sz_unc + x86_overrun + insurance
        push $-1  // space for F_ADRU
        push %rdi  // F_ELFA
        sub %rbx,%rcx; push %rcx  // F_LENX = sz_pack2 - O_BINFO
        add %rdi,%rbx; push %rbx  // F_ADRX =  elfaddr + O_BINFO

        push %rsp; pop old_sp
// alloca()
        sub %rdx,%rsp  // F_LENU space
CACHELINE= 8 * NBPW
        and $-CACHELINE,%rsp  // align

// Decompress the rest of this loader, and jump to it.

#define dst  %rdi
#define src  %rsi
#define lsrc %rcx
        push %rsp; pop dst  // dst= decompress onto stack
        lodsl; push %rax  // MATCH_11  .sz_cpr
        lodsl; cmpw $M_NRV2B_LE32|(0<<8),%ax; je 0f; hlt; 0:  // check method and filter bytes
        pop %rax; add src,%rax; push %rax  // MATCH_11  input_eof
        push old_sp  // MATCH_10


// This is nrv2b_d32, inlined and optimized for small space (about 160 bytes).
// The task is to de-compress the folded pieces for shared library init:
// the de-compressor(s) of the PT_LOAD pieces, and the C-code supervisor
// which adjusts the placement and mapping of the address space.
// The output length is a couple KB for NRV, a few KB for Lzma, 64KB for Zstd.
// This is motivated by the possibility of using multiple de-compressors
// depending on the characteristics of each PT_LOAD, and by the increased size
// and compressability of C-coded de-compressors for Lzma and Zstd
// in contrast to the simple and small assembly-coded NRV.

//%rsp:
//  MATCH_10  old_sp
//  MATCH_11  &input_eof

//%rbp  === old_sp:  array of F_FRAME

/* Working registers */
#define off  %eax  /* XXX: 2GB */
#define bits %ebx
#define len  %ecx  /* XXX: 2GB */
#define lenq %rcx
#define dispq %rbp
#define displ %ebp

#define GETBIT call *%rdx
#define jnextb0 GETBIT; jnc
#define jnextb1 GETBIT; jc

/* rotate next bit into bottom bit of reg */
#define getnextb(reg) GETBIT; adcl reg,reg

        xor bits,bits  // empty; force refill
        xor len,len  // create loop invariant
        lea getbit(%rip),%rdx
        push $-1; pop dispq  // initial displacement
        cld  // paranoia
        .byte 0xa8  // "testb $... ,%al" ==> "jmp top_n2b"
lit_n2b:
        movsb  // *dst++ = *src++;
top_n2b:
        jnextb1 lit_n2b
        lea 1(lenq),off  # [len= 0] off= 1
offmore_n2b:
        getnextb(off)
        jnextb0 offmore_n2b

        subl $ 3,off; jc len_n2b  # use previous offset
        shll $ 8,off; lodsb  # off is %eax, so 'lodsb' is "off |= *src++;"
        xorl $~0,off; jz eof_n2b
        movslq off,dispq  # XXX: 2GB; (note propagation of negative sign!)
// for 4GB, replace the 'movslq' with:
//      pushq $~0  # 64 bits all '1'
//      movl off,(%rsp)  # replace lo-order 32 bits
//      popq dispq
len_n2b:
        lea 1(lenq),off  # [len= 0] off= 1
        getnextb(len); getnextb(len)  # two bits; cc set on result
        jnz gotlen_n2b  # raw 1,2,3 ==> 2,3,4
        movl off,len  # len= 1, the msb
        addl $3-1,off  # raw 2.. ==> 5..
lenmore_n2b:
        getnextb(len)
        jnextb0 lenmore_n2b
gotlen_n2b:
        cmpl $-0xd00,displ  # XXX: 2GB;  for 4GB: use 'cmpq'
        adcl off,len  # len += off + (disp < -0xd00)

        push %rsi  // MATCH_06
          lea (%rdi,dispq),%rsi
          rep; movsb
        pop %rsi  // MATCH_06

        jmp top_n2b

eof_n2b:
        pop old_sp   // MATCH_10
        pop %rcx  // MATCH_11  &input_eof
        cmp %rcx,%rsi; je 0f; hlt; 0:  // test for ending in correct place
        //FIXME: check dst, too

// Write de-compressed 'fold' to file
        mov F_PMASK(old_sp),%rax; mov %rax,(%rsp)  // propagate PAGE_MASK
        mov F_LENU(old_sp),%arg3  // LENU
        push %rsp; pop %arg2  // buffer
        mov %r12,%arg1  // mfd
        push $__NR_write; pop %rax; call sys_check
// de-alloca()
        push old_sp; pop %rsp

// Map unfolded code the SELinux way
        sub %arg6l,%arg6l  // 0  offset
        mov %r12,%arg5  // mfd
        push $MAP_SHARED; pop %sys4
        push %arg3; pop %arg2  // LENU
        push $PROT_READ|PROT_EXEC; pop %arg3  // FIXME: add PROT_WRITE for DEBUG only
        subl %edi,%edi  // (%arg1)dst = 0;  // kernel chooses addr
        push $__NR_mmap; pop %rax; call sys_check
        push %rax  // MATCH_12
        mov %rax,F_ADRU(old_sp)

        push %arg5; pop %arg1  // mfd
        push $__NR_close; pop %rax; syscall

// Use the copy.
        pop %rax  // MATCH_12  ADRU
        add $D_FOLD,%rax  // beyond .data
        jmp *%rax  // goto unfolded stub

zfind:
        mov (%rdi),%rax; add $NBPW,%rdi
        test %rax,%rax; jne zfind
        ret

sys_check:
        push %rax  // save __NR_ for debug
        syscall
        pop %rcx  // recover __NR_ for debug
        cmp $-1<<12,%rax; jb 0f; hlt; 0:
        ret

here:  // OUT: %rdi= (2+ retaddrj) without disturbing shadow stack
        mov (%rsp),%rdi
        scasw  // over jmp.s
        ret

getbit:
        addl bits,bits; jz refill  // Carry= next bit
        rep; ret
refill:
        movl (%rsi),bits; subq $-4,%rsi  // next 32 bits; set Carry
        adcl bits,bits  // LSB= 1 (CarryIn); CarryOut= next bit
        rep; ret

        // IDENTSTR goes here

  section ELFMAINZ
        .balign 4
o_binfo:
        .long O_BINFO  // offset of b_info for .text
FOLD:
        // { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...}

/*__XTHEENDX__*/

/* vim:set ts=8 sw=8 et: */
