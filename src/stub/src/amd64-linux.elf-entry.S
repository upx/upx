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
////    nop; int3  # uncomment for debugging
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
        lodsl; movl %eax,-1*NBPW(%r13)  // is_ptinterp
        and $~0<<1,%eax; movl %eax,%r14d  // O_BINFO
        movl (%rsi),%edx; push %rdx  // P_30  .sz_unc LENU

        lea sz_pack2(%rip),%rcx  // &sz_pack2
        movl (%rcx),%r15d  // sz_pack2: length before stub
        subq %r15,%rcx  // elfaddr= &Elf64_Ehdr of this stub
        subl %r14d,%r15d  // LENX= sz_pack2 - O_BINFO
        addq %rcx,%r14  // ADRX= elfaddr + O_BINFO
        push %rdx  // P_30  LENU
        push %rcx  // P_31  elfaddr

// alloca()
        subq %rdx,%rsp; and $-2*NBPW,%rsp  // aligned space for unfolded stub

#define dst  %rdi
#define src  %rsi
#define lsrc %rcx
        push %rsp; pop dst  // dst= decompress onto stack
        lodsl; push %rax  // MATCH_10  .sz_unc
        lodsl; push %rax  // MATCH_11  .sz_cpr
        lodsl; cmpw $M_NRV2B_LE32|(0<<8),%ax; je 0f; hlt; 0:  // check method and filter bytes
        pop %rax; add src,%rax; push %rax  // MATCH_11  input_eof


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
//  MATCH_11  &input_eof
//  MATCH_10  len unfolded_code
//  P_11  rcx.atexit
//  P_10  %entry
//  MATCH_00  argc
//  MATCH_01  argv
//  MATCH_07  envp

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
        pop %rcx  // MATCH_11  &input_eof
        cmp %rcx,%rsi; je 0f; hlt; 0:  // test for ending in correct place
        //FIXME: check dst, too

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
        push $PROT_READ|PROT_EXEC; pop %arg3  // PROT_WRITE|: DEBUG only
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

  section ELFMAINZ
        .balign 4
o_binfo:
        .long O_BINFO  // offset of b_info for .text
FOLD:
        // { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...}

/*__XTHEENDX__*/

/* vim:set ts=8 sw=8 et: */
