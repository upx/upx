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
/* In:
   r31= &decompress; also 8+ (char *)&(#bytes which preceed &-8(r31)
   r28= &Elf32_auxv_t
   r27= actual page size
*/
fold_begin:
////  teq r0,r0  // debugging
        call L90
#include "arch/powerpc/32/bxx.S"
L90:
        mflr a5  // &ppcbxx: f_unfilter
        mr a6,r28  // a6= &Elf32_auxv
        lwz a1,-8(r31)  // #bytes which preceed -8(r31)
          rlwinm r30,a5,0,0,31-12  // r30= &this_page
        mr a4,r31  // &decompress: f_expand
        subf r29,a1,r31  // 8+ (char *)&our_Elf32_Ehdr
        la a2,-OVERHEAD(sp)  // &Elf32_Ehdr temporary space
        addi r29,r29,-8  // &our_Elf32_Ehdr
        lhz a3,e_phnum(r29)
        mulli a3,a3,szElf32_Phdr
        addi  a3,a3,szElf32_Ehdr
        sub a1,a1,a3
        add a0,r29,a3  // &{l_info; p_info; b_info}
        mr  a7,r27  // pagesize
        stwu sp,-(SZ_FRAME+OVERHEAD)(sp)
        lwz a3,sz_unc+sz_p_info+sz_l_info(a0)  // sz_elf_headers
        call upx_main  // Out: a0= entry
        /* entry= upx_main(l_info *a0, total_size a1, Elf32_Ehdr *a2, sz_ehdr a3,
      f_decomp a4, f_unf a5, Elf32_auxv_t *a6, page_size a7)
*/
        mr r31,a0  // save &entry

        mr a0,r29  // &our_Elf32_Ehdr
        subf a1,r29,r30  // size
        call munmap  // unmap compressed program; /proc/self/exe disappears

        mtlr r31  // entry address
        lmw r2,4+SZ_FRAME+OVERHEAD(sp)  // restore registers r2 thru r31
        lwz r1,  SZ_FRAME+OVERHEAD(sp)  // restore r1; deallocate space
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
