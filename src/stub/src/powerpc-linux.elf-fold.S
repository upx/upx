/*  powerpc-linux.elf-fold.S -- linkage to C code to process ELF binary
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2017 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2017 Laszlo Molnar
*  Copyright (C) 2000-2017 John F. Reiser
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

#include "arch/powerpc/32/macros.S"
#include "arch/powerpc/32/ppc_regs.h"

#define szElf32_Ehdr 0x34
#define szElf32_Phdr 0x20
#define e_phnum 44

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4

sz_l_info= 12
sz_p_info= 12

OVERHEAD= 2048

// http://refspecs.linuxfoundation.org/elf/elfspec_ppc.pdf  p.3-17 (pdf p.33)
// SZ_LINKA= (2 + max(0, #params - 8))*4  // (sp,lr, arg9,arg10, ... )
SZ_LINKA= (2 + 2)*4  // (sp,pc, arg9,arg10,... ) and 16-byte aligned
  ARG9= 2*4
  ARG10= 3*4
SZ_FRAME= SZ_LINKA

/* In:
   r31= &decompress; also 2*4+ (char *)&(#bytes which preceed &-8(r31)
   r30= reserved for &this page
   r29= reserved for &our_Elf32_Ehdr
   r28= &Elf32_auxv_t
   r27= actual page size (4K or 64K)
   r26= o_binfo
   r25= ADRX
   r24= ADRU
   r23= LENU
   r22= slide
*/
fold_begin:
  teq r0,r0  // debugging
        call L90
#include "arch/powerpc/32/bxx.S"

L90:
        li r0,0
r_reloc= 32 - 1  # use a slot in register save area
        la a7,SZ_FRAME+r_reloc*4(sp)  // &reloc
        stw r22,0(a7)  // reloc === slide
        sub r0,r0,r27  // -pagesize == pagemask
        mr a7,r27  // pagesize
        mr a6,r28  // &Elf32_auxv_t
        mflr a5  // &ppcbxx: f_unfilter
          and r30,a5,r0  // r30= &this_page
        mr a4,r31  // &decompress: f_expand

        la sp,-OVERHEAD(sp)  // -SZ_FRAME + &Elf32_Ehdr temporary space
        lwz a1,-2*4(r31)  // sz_pack2 == #bytes which preceed -2*4(r31)
        subf r29,a1,r31  // 2*4+ (char *)&our_Elf32_Ehdr
        addi r29,r29,-2*4  // &our_Elf32_Ehdr
          stw r29,ARG9(sp)  // reloc

        add a0,r29,r26  // &b_info
        sub a1,a1,r26  // total_size
        la  a2,SZ_FRAME(sp)  // &Elf32_Ehdr temporary space
        lwz a3,sz_unc(a0)  // sz_elf_headers

teq r0,r0
        call upx_main  // Out: a0= entry
// entry= upx_main(b_info *a0, total_size a1, Elf32_Ehdr *a2, sz_ehdr a3,
//      f_exp a4, f_unf a5, Elf32_auxv_t *a6, page_size a7, reloc ARG9)
        mr r31,a0  // save &entry

        mr a0,r29  // &our_Elf32_Ehdr
        subf a1,r29,r30  // size
        call munmap  // unmap compressed program; /proc/self/exe disappears

        mtlr r31  // entry address
        lmw r2,OVERHEAD+SZ_FRAME+2*4-4(sp)  // restore registers r2 thru r31
        lwz sp,OVERHEAD(sp)  // restore r1; deallocate space
        ret  // enter /lib/ld.so.1

SYS_exit=  1
SYS_fork=  2
SYS_read=  3
SYS_write= 4
SYS_open=  5
SYS_close= 6

SYS_brk=       45
SYS_mmap=      90
SYS_munmap=    91
SYS_mprotect= 125

mmap: .globl mmap
        li r0,SYS_mmap
sysgo:
        sc
        bns+ no_fail  // 'bns': branch if No Summary[Overflow]
        li a0,-1  // failure; IGNORE errno
no_fail:
        ret

exit: .globl exit
        li r0,SYS_exit;        b 5f
read: .globl read
        li r0,SYS_read;     5: b 5f
open: .globl open
        li r0,SYS_open;     5: b 5f
close: .globl close
        li r0,SYS_close;    5: b 5f
mprotect: .globl mprotect
        li r0,SYS_mprotect; 5: b 5f
munmap: .globl munmap
        li r0,SYS_munmap;   5: b 5f
brk: .globl brk
        li r0,SYS_brk;      5: b sysgo

/* vim:set ts=8 sw=8 et: */
