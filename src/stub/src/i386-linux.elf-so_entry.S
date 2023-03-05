/*  i386-linux.elf-so_entry.S -- Linux DT_INIT & decompressor (Elf shared lib)
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

//#include "arch/amd64/macros.S"
//#include "arch/amd64/regs.h"
#define section .section

NBPW= 4

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

__NR_mmap=     90
__NR_mprotect=125
__NR_munmap=   91

__NR_write= 4
__NR_exit=  1

PAGE_SHIFT= 12
PAGE_MASK= (~0<<PAGE_SHIFT)
PAGE_SIZE= -PAGE_MASK

M_NRV2B_LE32=2  // ../conf.h
M_NRV2D_LE32=5
M_NRV2E_LE32=8

/* Arguments to decompress() */
#define src  %esi
#define lsrc %ecx
#define dst  %edi
//#define ldst %edx  /* Out: actually a reference: &len_dst */

// Written by PackLinuxElf::pack3():
//  .long offset(.)  // detect relocation
//  .long offset(user DT_INIT)
//  .long offset(escape_hatch)
//  .long offset({l_info; p_info; b_info; compressed data})
  section ELFMAINX
_start:  // C-called: %esp: ret_addr,argc,argv,envp; must save %ebx,%esi,%edi,%ebp
    nop  // DEBUG i386 so_entry.S
    pusha  // MATCH_03
    call L70  // MATCH_08  push $&getbit
L70ret:

/* Working registers for local NRV2B */
#define off  %eax  /* XXX: 2GB */
#define bits %ebx
#define len  %ecx  /* XXX: 2GB */
#define disp %ebp

#define GETBIT call *%edx
#define jnextb0 GETBIT; jnc
#define jnextb1 GETBIT; jc

/* rotate next bit (now in Carry) into bottom bit of reg */
#define getnextb(reg) GETBIT; adcl reg,reg

getbit:
        addl bits,bits; jz refill  // Carry= next bit
        rep; ret  // rep: stop instruction pipeline (spend 1 byte for speed)
refill:
        mov (%esi),bits; sub $-NBPW,%esi  // next 32 bits; set Carry
        adc bits,bits  // LSB= 1 (CarryIn); CarryOut= next bit
        ret  // infrequent (1/32)

L20:
        pop %ebx  // MATCH_09  &fold_info
        cmpw $M_NRV2B_LE32|(0<<8),b_method(%ebx); je 0f; hlt; 0:  // check method and filter bytes
        lea _start - 4*NBPW - L70ret(%edx),%ecx  // &so_info

        push %ecx  // MATCH_14  &so_info
        push %ebx  // MATCH_15  old_mmap destroys

// Get pages for unfolded code
        push $0  // arg6 offset
        push $-1  // arg5 fd
        push $MAP_PRIVATE|MAP_ANONYMOUS  // arg4 modes
        push $PROT_READ|PROT_WRITE  // arg3 prot
        push /*sz_unc*/(%ebx)  // arg2 dstlen
        push $0  // arg1 addr (kernel chooses)
        call old_mmap
        pop %ebx  // MATCH_15  &fold_info
        push /*sz_unc*/(%ebx)  // MATCH_10   len unfolded cold
        push %eax  // MATCH_04  ptr unfolded_code

// This is nrv2b_d32, inlined and optimized for small space (about 160 bytes).
// The task is to de-compress the folded pieces for shared library init:
// the de-compressor(s) of the PT_LOAD pieces, and the C-code supervisor
// which adjusts the placement and mapping of the address space.
// The output length is a couple KB for NRV, a few KB for Lzma, 64KB for Zstd.
// This is motivated by the possibility of using multiple de-compressors
// depending on the characteristics of each PT_LOAD, and by the increased size
// and compressability of C-coded de-compressors for Lzma and Zstd
// in contrast to the simple and small assembly-coded NRV.

        mov %eax,dst  // &unfolded_code
        mov sz_cpr(%ebx),lsrc
        lea sz_b_info(%ebx),src
decompress:  // inlined: (uchar const *src, uint len, uchar *dst /*, u32 &ldst, uint method */)
        add src,lsrc; push lsrc  // MATCH_05  &input_eof

//%esp:
//  MATCH_05  &input_eof
//  MATCH_04  ptr unfolded_code
//  MATCH_10  len unfolded_code
//  MATCH_14  &so_info
//  MATCH_03  pusha regs {%edi,%esi,%ebp,%esp,%ebx,%edx,%ecx,%eax}
//            ret_addr
//  MATCH_00  argc
//  MATCH_01  argv
//  MATCH_07  envp

        xor bits,bits  // empty; force refill
        xor len,len  // create loop invariant
        or $~0,disp  // -1: initial dispacement
        cld  // paranoia
        .byte 0xa8  // "testb $... ,%al" ==> "jmp top_n2b"
lit_n2b:
        movsb  // *dst++ = *src++;
top_n2b:
        jnextb1 lit_n2b
        lea 1(len),off  # [len= 0] off= 1
offmore_n2b:
        getnextb(off)
        jnextb0 offmore_n2b

        sub $ 3,off; jc len_n2b  # use previous offset
        shl $ 8,off; lodsb  # off is %eax, so 'lodsb' is "off |= *src++;"
        xor $~0,off; jz eof_n2b
        mov off,disp  # XXX: 2GB
len_n2b:
        lea 1(len),off  # [len= 0] off= 1
        getnextb(len); getnextb(len)  # two bits; cc set on result
        jnz gotlen_n2b  # raw 1,2,3 ==> 2,3,4
        mov off,len  # len= 1, the msb
        add $3-1,off  # raw 2.. ==> 5..
lenmore_n2b:
        getnextb(len)
        jnextb0 lenmore_n2b
gotlen_n2b:
        cmp $-0xd00,disp  # XXX: 2GB
        adc off,len  # len += off + (disp < -0xd00)
        push %esi  // MATCH_06
          lea (%edi,disp),%esi
          rep; movsb
        pop %esi  // MATCH_06
        jmp top_n2b

eof_n2b:
        pop %ecx  // MATCH_05  &input_eof
        cmp %ecx,%esi; je 0f; hlt; 0:  // test for ending in correct place

        pop %ebx  // MATCH_04  arg1  ptr unfolded_code
        pop %ecx  // MATCH_10  arg2  len unfoded_code
          push %ecx  // MATCH_12  len unfolded code
          push %ebx  // MATCH_13  ptr unfolded code
        push $PROT_EXEC|PROT_READ; pop %edx  // arg3
        push $__NR_mprotect; pop %eax; int $0x80

// %esp:
//  MATCH_13  ptr unfolded_code; for escape hatch
//  MATCH_12  len unfolded code; for escape hatch
//  MATCH_14  &so_info
//  MATCH_03  pusha regs {%edi,%esi,%ebp,%esp,%ebx,%edx,%ecx,%eax}
//            ret_addr
//  MATCH_00  argc
//  MATCH_01  argv
//  MATCH_07  envp

        pop %eax; push %eax  // MATCH_13 ptr unfolded code
        jmp *%eax  // enter C code

old_mmap:  // oldmmap: ebx -> 6 arguments; remove arguments on return
        lea 4(%esp),%ebx
        push $__NR_mmap; pop %eax; int $0x80
        cmp $PAGE_MASK,%eax; jb 0f; hlt; 0:
        ret $6*4

// IDENTSTR goes here

  section ELFMAINZ
L70:
        pop %edx  // &getbit  (also L70ret)
        call L20  // MATCH_09  push $&fold_info
fold_info:
//  b_info (sz_unc, sz_cpr, method) of folded code (C-language, etc.)

/* vim:set ts=8 sw=8 et: */
