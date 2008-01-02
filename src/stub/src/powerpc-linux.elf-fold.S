/*  powerpc-linux.elf-fold.S -- linkage to C code to process ELF binary
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2008 Laszlo Molnar
*  Copyright (C) 2000-2008 John F. Reiser
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
*  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
*
*  John F. Reiser
*  <jreiser@users.sourceforge.net>
*/

#include "arch/powerpc/32/macros.S"
#include "arch/powerpc/32/ppc_regs.h"

#define szElf32_Ehdr 0x34
#define szElf32_Phdr 0x20

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4

sz_l_info= 12
sz_p_info= 12

OVERHEAD= 2048
LINKAREA= 6*4  // (sp,cr,pc, xx,yy.zz) save area per calling convention
/* In:
   r31= &decompress; also 8+ (char *)&(#bytes which preceed &-8(r31)
*/
fold_begin:
        call L90
#include "arch/powerpc/32/bxx.S"

/* The SysV convention for argument registers after execve is nice:
        a0= argc
        a1= argv
        a2= envp
        a3= auxvp
        a4= fini
        sp= ~0xf & (-2*4 + (void *)&argc)  // 0(sp): old_sp, pc
   Instead, Linux gives only
        sp= &{argc,argv...,0,env...,0,auxv...,strings}  // 16-byte aligned?
   We must figure out the rest, particularly auxvp.
*/
zfind:
        lwz t0,0(a6); addi a6,a6,4
        cmpi cr7,t0,0; bne+ cr7,zfind
        ret
L90:
        la sp,6*4(sp)  // trim save area used by decompressor
        mflr a5  // &ppcbxx: f_unfilter
        lwz a6,0(sp)  // sp at execve
        call zfind  // a6= &env
        call zfind  // a6= &Elf32_auxv
        lwz a1,-8(r31)  // #bytes which preceed -8(r31)
          rlwinm r30,a5,0,0,31-12  // r30= &this_page
        mr a4,r31  // &decompress: f_expand
        subf r29,a1,r31  // 8+ (char *)&our_Elf32_Ehdr
        la a2,-OVERHEAD(sp)  // &Elf32_Ehdr temporary space
        addi r29,r29,-8  // &our_Elf32_Ehdr
        addi a1,a1,-(szElf32_Ehdr + 2*szElf32_Phdr)
        addi a0,r29,(szElf32_Ehdr + 2*szElf32_Phdr)  // &{l_info; p_info; b_info}
        addi sp,sp,-(LINKAREA+OVERHEAD)
        lwz a3,sz_unc+sz_p_info+sz_l_info(a0)  // sz_elf_headers
        call upx_main  // Out: a0= entry
        /* entry= upx_main(l_info *a0, total_size a1, Elf32_Ehdr *a2, sz_ehdr a3,
      f_decomp a4, f_unf a5, Elf32_auxv_t *a6)
*/
        mr r31,a0  // save &entry

        mr a0,r29  // &our_Elf32_Ehdr
        subf a1,r29,r30  // size
        call munmap  // unmap compressed program; /proc/self/exe disappears

        mtlr r31  // entry address
        lmw r2,4+LINKAREA+OVERHEAD(sp)  // restore registers r2 thru r31
        lwz r1,  LINKAREA+OVERHEAD(sp)  // restore r1; deallocate space
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
        li 0,SYS_mmap
sysgo:
        sc
        bns+ no_fail  // 'bns': branch if No Summary[Overflow]
        li a0,-1  // failure; IGNORE errno
no_fail:
        ret

exit: .globl exit
        li 0,SYS_exit; b sysgo
read: .globl read
        li 0,SYS_read; b sysgo
open: .globl open
        li 0,SYS_open; b sysgo
close: .globl close
        li 0,SYS_close; b sysgo
mprotect: .globl mprotect
        li 0,SYS_mprotect; b sysgo
munmap: .globl munmap
        li 0,SYS_munmap; b sysgo
brk: .globl brk
        li 0,SYS_brk; b sysgo

/*
vi:ts=8:et:nowrap
*/

