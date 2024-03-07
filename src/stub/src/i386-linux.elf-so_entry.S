/*  i386-linux.elf-so_entry.S -- Linux DT_INIT & decompressor (Elf shared lib)
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2021 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2021 Laszlo Molnar
*  Copyright (C) 2000-2024 John F. Reiser
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

__NR_memfd_create= 0x164 // 356
__NR_oldmmap=  90
__NR_mprotect=125
__NR_munmap=   91

__NR_close= 6
__NR_exit=  1
__NR_write= 4

NAME_MAX=  255  // # chars in file name; linux/include/uapi/linux/limits.h

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
    nop  //  int3  // DEBUG i386 so_entry.S
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

#define old_sp %ebp

#define foldi %ebx
#define r_unc %esi
#define u_len %edi
L20:
        pop foldi  // MATCH_09  &fold_info
//        cmpw $M_NRV2B_LE32|(0<<8),b_method(foldi); je 0f; hlt; 0:  // check method and filter bytes
        lea _start - 4*NBPW - L70ret(%edx),%ecx; push %ecx  // MATCH_14  &so_info
        sub $3*NBPW,%esp; mov %esp,old_sp
#define F_ENTR 3*NBPW
#define F_LENU 2*NBPW
#define F_ADRU 1*NBPW
#define F_MFD  0*NBPW
        mov /*sz_unc*/(foldi),r_unc

            // align + {page_mask, fd_name} + "/data/data/$APP_NAME/cache/upxAAA"
        lea (1+ NAME_MAX + 8 + 2*NBPW + (1+ 11 + 13))(r_unc),u_len
        mov u_len,F_LENU(old_sp)
        sub u_len,%esp  // alloca
        and $-2*NBPW,%esp  // align stack

        lea (8 + 2*NBPW)(r_unc),%eax; add %esp,%eax
        and $-2*NBPW,%eax; push %eax  // MATCH_31 where to put pathname
        push $-1  // MATCH_32  space for PAGE_MASK

        xor %ecx,%ecx  // zero
        push %edx  // MATCH_33  save &getbit
        push %ecx  // arg4  frag_mask
        push %eax  // arg3  &pathname
        mov %ecx,(%eax)  // empty string
        push u_len  // arg2  length required
        push %ecx  // arg1  0==> any page
        call upx_mmap_and_fd; add $4*NBPW,%esp  // (ptr, len, pathname, frag_mask)
        pop %edx  // MATCH_33  &getbit
        mov %eax,%ecx
        shr $12,%ecx; shl $12,%ecx; mov %ecx,F_ADRU(old_sp)
        sub %ecx,%eax; sub $1,%eax; mov %eax,F_MFD(old_sp)
#undef r_unc
#undef u_len


// This is nrv2b_d32, inlined and optimized for small space (about 160 bytes).
// The task is to de-compress the folded pieces for shared library init:
// the de-compressor(s) of the PT_LOAD pieces, and the C-code supervisor
// which adjusts the placement and mapping of the address space.
// The output length is a couple KB for NRV, a few KB for Lzma, 64KB for Zstd.
// This is motivated by the possibility of using multiple de-compressors
// depending on the characteristics of each PT_LOAD, and by the increased size
// and compressability of C-coded de-compressors for Lzma and Zstd
// in contrast to the simple and small assembly-coded NRV.

        lea (1+ 2)*NBPW(%esp),dst  // &unfolded_code  &so_info,PAGE_SIZE,F_MFD
        push old_sp  // MATCH_16  save register
        mov    sz_cpr(foldi),lsrc
        lea sz_b_info(foldi),src
#undef foldi
decompress:  // inlined: (uchar const *src, uint len, uchar *dst /*, u32 &ldst, uint method */)
        add src,lsrc; push lsrc  // MATCH_05  &input_eof
        //subq src,lsrc  // restore the value of lsrc; dead for inlined nrv2b

//%esp:
//  MATCH_05  &input_eof
//  MATCH_16  old_sp
//  space for de-compressed code

//old_sp:
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
        pop old_sp  // MATCH_16

#define mfd %edi
        mov F_MFD(old_sp),mfd
        mov F_LENU(old_sp),%edx
        mov %esp,%ecx
        mov %edi,%ebx
        push $__NR_write; pop %eax; call sys_check
        lea 1*NBPW(old_sp),%esp  // de-alloca; F_MFD

        push $0  // arg6
        push mfd  // arg5
        push $MAP_FIXED|MAP_PRIVATE  // arg4
        push $PROT_READ|PROT_EXEC  // arg3
        push F_LENU(%ebp)  // arg2  F_LENU
        push F_ADRU(%ebp)  // arg1  F_ADRU
        call oldmmap

        mov mfd,%ebx
        push $__NR_close; pop %eax; call sys_check

        mov F_ADRU(%ebp),%eax
        add $F_ENTR,%eax
        jmp *%eax
// %esp:
//  MATCH_13  ptr unfolded_code; for escape hatch
//  MATCH_12  len unfolded code; for escape hatch
//  MATCH_14  &so_info
//  MATCH_03  pusha regs {%edi,%esi,%ebp,%esp,%ebx,%edx,%ecx,%eax}
//            ret_addr
//  MATCH_00  argc
//  MATCH_01  argv
//  MATCH_07  envp


sys_check:  // syscall with error checking; if halt then %eax= -errno, %ecx= __NR_
        push %eax // save __NR_ for debug
        int $0x80
        pop %ecx  // recover __NR_ for debug
        cmp $PAGE_MASK,%eax; jb 0f; hlt; 0:  // -errno
        ret

mmap: .globl mmap  // for C-callable, such as from upx_mmap_and_fd()
oldmmap:  // oldmmap: 6 arguments on stack; remove arguments on return
        push %ebx  // save register
        lea 2*NBPW(%esp),%ebx
        push $__NR_oldmmap; pop %eax; call sys_check
        pop %ebx  // restore register
        ret $6*NBPW

get_upxfn_path: .globl get_upxfn_path
        sub %eax,%eax  // should not be called!
        ret

my_memset: .globl my_memset  // (dst, val, n)
        push %ebp; mov %esp,%ebp
        push %edi
        mov (2+ 2)*NBPW(%ebp),%ecx
        mov (2+ 1)*NBPW(%ebp),%eax
        mov (2+ 0)*NBPW(%ebp),%edi
        rep stosb
        pop %edi
        pop %ebp
        ret

mempcpy: .globl mempcpy  // (dst, src, n)
        push %ebp; mov %esp,%ebp
        push %edi; push %esi
        mov (2+ 2)*NBPW(%ebp),%ecx
        mov (2+ 1)*NBPW(%ebp),%esi
        mov (2+ 0)*NBPW(%ebp),%edi
        rep movsb
        mov %edi,%eax
        pop %esi; pop %edi; pop %ebp
        ret

// IDENTSTR goes here

  section ELFMAINZ
L70:
        pop %edx  // MATCH_08  &getbit  (also L70ret)
        call L20  // MATCH_09  push $&fold_info
fold_info:  // nice if 4-byte aligned
//  b_info (sz_unc, sz_cpr, method) of folded code (C-language, etc.)

/* vim:set ts=8 sw=8 et: */
