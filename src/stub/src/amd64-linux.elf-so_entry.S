/*  amd64-linux.elf-so_entry.S -- Linux DT_INIT & decompressor (Elf shared lib)
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2021 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2021 Laszlo Molnar
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

sz_Ehdr= 64
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

MAP_PRIVATE= 2
MAP_FIXED=     0x10
MAP_ANONYMOUS= 0x20

__NR_mmap=      9  // 64-bit mode only!  /usr/include/asm/unistd_64.h
__NR_mprotect= 10
__NR_munmap=   11
__NR_memfd_create= 0x13f  // 319

__NR_write=  1
__NR_close=  3
__NR_exit=  60

PAGE_SHIFT= 12
PAGE_MASK= (~0<<PAGE_SHIFT)
PAGE_SIZE= -PAGE_MASK

M_NRV2B_LE32=2  // ../conf.h
M_NRV2D_LE32=5
M_NRV2E_LE32=8

/* Arguments to decompress() */
#define src  %rsi
#define lsrc %rcx
#define dst  %rdi
//#define ldst %rdx  /* Out: actually a reference: &len_dst */

// Written by PackLinuxElf::pack3():
//  .long offset(.)  // detect relocation
//  .long offset(user DT_INIT)
//  .long offset(escape_hatch)
//  .long offset({l_info; p_info; b_info; compressed data})
  section ELFMAINX
_start:
    nop  // int3  // DEBUG
        push %arg3  // MATCH_07  envp (glibc)
        push %arg2  // MATCH_01  argv
        push %arg1  // MATCH_00  argc
        push %rbx  // MATCH_03  saved register
        push %rbp  // MATCH_02  saved register
        mov %rsp,%rbp
        call L70  // MATCH_08  push $&getbit
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

getbit:
        addl bits,bits; jz refill  // Carry= next bit
        rep; ret  // rep: stop instruction pipeline (spend 1 byte for speed)
refill:
        movl (%rsi),bits; subq $-4,%rsi  // next 32 bits; set Carry
        adcl bits,bits  // LSB= 1 (CarryIn); CarryOut= next bit
        ret  // infrequent (1/32)

L20:  // %rdx == &getbit
        pop %rbx  // MATCH_09  &fold_info
        lea _start - 4*4 - getbit(%rdx),%rax  // &so_info
        push %rax  // MATCH_14  &so_info

//        cmpw $M_NRV2B_LE32|(0<<8),b_method(%rbx); je 0f; hlt; 0:  // check method and filter bytes

// De-compress folded code onto the stack
        movl /*sz_unc*/(%rbx),%eax; push %rax  // MATCH_40  len unfolded code
        sub %rax,%rsp; and $-2*NBPW,%rsp

// This is nrv2b_d32, inlined and optimized for small space (about 160 bytes).
// The task is to de-compress the folded pieces for shared library init:
// the de-compressor(s) of the PT_LOAD pieces, and the C-code supervisor
// which adjusts the placement and mapping of the address space.
// The output length is a couple KB for NRV, a few KB for Lzma, 64KB for Zstd.
// This is motivated by the possibility of using multiple de-compressors
// depending on the characteristics of each PT_LOAD, and by the increased size
// and compressability of C-coded de-compressors for Lzma and Zstd
// in contrast to the simple and small assembly-coded NRV.

        push %rsp; pop dst  // &unfolded_code
        push %rbp  // MATCH_45
        movl    sz_cpr(%rbx),len  // lsrc
        lea  sz_b_info(%rbx),src
decompress:  // inlined: (uchar const *src, uint len, uchar *dst /*, u32 &ldst, uint method */)
        addq src,lsrc; push lsrc  // MATCH_05  &input_eof
        //subq src,lsrc //restore the value of lsrc; dead for inlined nrv2b

//%rsp:
//  MATCH_05  &input_eof
//  MATCH_04  ptr unfolded_code
//  MATCH_10  len unfolded_code
//%rbp:
//  MATCH_02  saved %rbp
//  MATCH_03  saved %rbx
//  MATCH_00  argc
//  MATCH_01  argv
//  MATCH_07  envp

        xor bits,bits  // empty; force refill
        xor len,len  // create loop invariant
        push $~0; pop dispq  // -1: initial displacement
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
        pop %rcx  // MATCH_05  &input_eof
        cmp %rcx,%rsi; je 0f; hlt; 0:  // test for ending in correct place

        pop %rbp  // MATCH_45

        push $0; pop %arg2
        call 0f; .asciz "upx"; 0: pop %arg1
        push $__NR_memfd_create; call do_sys

        push %rax; pop %arg1  // mfd
        push %rsp; pop %arg2  // buffer
        push %rax  // MATCH_47  save mfd
        mov -2*NBPW(%rbp),%arg3  // length
        push $__NR_write; call do_sys  // scribbles %rcx !!

// Map unfolded code the SELinux way
        pop %arg5  // MATCH_47  mfd
        lea -2*NBPW(%rbp),%rsp
        pop %arg2; push %arg2  // MATCH_40  len unfolded code
        sub %arg6l,%arg6l  // 0
        push $MAP_PRIVATE; pop %sys4
        push $PROT_READ|PROT_EXEC; pop %arg3
        subl %edi,%edi  // (%arg1)dst = 0;  // kernel chooses addr
        push $__NR_mmap; call do_sys
        push %rax  // MATCH_11  ptr unfolded code

        push %arg5; pop %arg1  // mfd
        push $__NR_close; call do_sys

// %rsp:
//  MATCH_11  ptr unfolded_code; for escape hatch
//  MATCH_10  len unfolded code; for escape hatch
//  MATCH_14  &so_info
// %rbp:
//  MATCH_02  saved %rbp
//  MATCH_03  saved %rbx
//  MATCH_00  argc
//  MATCH_01  argv
//  MATCH_07  envp

        pop %rax; push %rax  // MATCH_11 ptr unfolded code
        jmp *%rax  // enter C code

do_sys: // on-stack parameter: hint on error
        mov NBPW(%rsp),%rax; syscall
        cmp $-4096,%rax; jb 0f; int3; 0:
        ret $NBPW

// IDENTSTR goes here

  section ELFMAINZ
L70:
        pop %rdx  // MATCH_08  &getbit
        call L20  // MATCH_09  push $&fold_info
fold_info:
//  b_info (sz_unc, sz_cpr, method) of folded code (C-language, etc.)

/* vim:set ts=8 sw=8 et: */
