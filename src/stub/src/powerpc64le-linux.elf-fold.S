/*  powerpc64le-linux.elf-fold.S -- linkage to C code to process ELF binary
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

#include "arch/powerpc/64le/macros.S"
#include "arch/powerpc/64le/ppc_regs.h"

#define szElf64_Ehdr 0x40
#define szElf64_Phdr 0x38

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4

sz_l_info= 12
sz_p_info= 12

OVERHEAD= 2048
LINKAREA= 6*8  // (sp,pc,cr, xx,yy.zz) save area per calling convention
/* In:
   r31= &decompress; also 8+ (char *)&(#bytes which preceed &-8(r31)
*/
fold_begin:
        call L90
#include "arch/powerpc/64le/bxx.S"

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
        ld t0,0(a6)        // parameters are 16byte aligned
        addi a6,a6,8
        cmpi cr7,t0,0; bne+ cr7,zfind
        ret
L90:
        la sp,LINKAREA(sp)  // trim save area used by decompressor
        mflr a5  // &ppcbxx: f_unfilter
        ld a6,0(sp)  // sp at execve
        call zfind  // a6= &env
        call zfind  // a6= &Elf64_auxv
        lwz a1,-8(r31)  // #bytes which preceed -8(r31)
        rlwinm r30,a5,0,0,31-12  // r30= &this_page
        mr a4,r31  // &decompress: f_expand
        subf r29,a1,r31  // 8+ (char *)&our_Elf64_Ehdr
        la a2,-OVERHEAD(sp)  // &Elf64_Ehdr temporary space
        addi r29,r29,-8  // &our_Elf64_Ehdr
        addi a1,a1,-(szElf64_Ehdr + 2*szElf64_Phdr)
        addi a0,r29,(szElf64_Ehdr + 2*szElf64_Phdr)  // &{l_info; p_info; b_info}
        addi sp,sp,-(LINKAREA+OVERHEAD)
        lwz a3,sz_unc+sz_p_info+sz_l_info(a0)  // sz_elf_headers
        call upx_main  // Out: a0= entry
        /* entry= upx_main(l_info *a0, total_size a1, Elf64_Ehdr *a2, sz_ehdr a3,
      f_decomp a4, f_unf a5, Elf64_auxv_t *a6)
*/
        mr r31,a0  // save &entry

        mr a0,r29  // &our_Elf64_Ehdr
        subf a1,r29,r30  // size
        call munmap  // unmap compressed program; /proc/self/exe disappears

        mtlr r31  // entry address

        lis    15,0x1000
        addi   15,15,0x8c0  // Check if statically allocated i.e.
        cmpld  15,r31       // entry @ 0x10008c0
        beq+   rest

/* Dynamic load settings */
        addis r31,r31,5
        addi  r31,r31,27232    // @+0x57e00

        addi  12,r31,-27828
        std   12,-27984(r31)  // required : Inconsistency detected by ld.so: rtld.c:

        li    12,0
        std   12,-28192(r31)    // p 0x...d7e00-0x...d0fe0

//      lmw r2,4+LINKAREA+OVERHEAD(sp)  // restore registers r2 thru r31 32bits load
/* Restore Save Area     */
rest:
        ld  2,2*8-8+LINKAREA+OVERHEAD(sp)
        ld  3,3*8-8+LINKAREA+OVERHEAD(sp)
        ld  4,4*8-8+LINKAREA+OVERHEAD(sp)
        ld  5,5*8-8+LINKAREA+OVERHEAD(sp)
        ld  6,6*8-8+LINKAREA+OVERHEAD(sp)
        ld  7,7*8-8+LINKAREA+OVERHEAD(sp)
        ld  8,8*8-8+LINKAREA+OVERHEAD(sp)
        ld  9,9*8-8+LINKAREA+OVERHEAD(sp)
        ld  10,10*8-8+LINKAREA+OVERHEAD(sp)
        ld  11,11*8-8+LINKAREA+OVERHEAD(sp)
        ld  12,12*8-8+LINKAREA+OVERHEAD(sp)
        ld  13,13*8-8+LINKAREA+OVERHEAD(sp)
        ld  14,14*8-8+LINKAREA+OVERHEAD(sp)
        ld  15,15*8-8+LINKAREA+OVERHEAD(sp)
        ld  16,16*8-8+LINKAREA+OVERHEAD(sp)
        ld  17,17*8-8+LINKAREA+OVERHEAD(sp)
        ld  18,18*8-8+LINKAREA+OVERHEAD(sp)
        ld  19,19*8-8+LINKAREA+OVERHEAD(sp)
        ld  20,20*8-8+LINKAREA+OVERHEAD(sp)
        ld  21,21*8-8+LINKAREA+OVERHEAD(sp)
        ld  22,22*8-8+LINKAREA+OVERHEAD(sp)
        ld  23,23*8-8+LINKAREA+OVERHEAD(sp)
        ld  24,24*8-8+LINKAREA+OVERHEAD(sp)
        ld  25,25*8-8+LINKAREA+OVERHEAD(sp)
        ld  26,26*8-8+LINKAREA+OVERHEAD(sp)
        ld  27,27*8-8+LINKAREA+OVERHEAD(sp)
        ld  28,28*8-8+LINKAREA+OVERHEAD(sp)
        ld  29,29*8-8+LINKAREA+OVERHEAD(sp)
        ld  30,30*8-8+LINKAREA+OVERHEAD(sp)
        ld  31,31*8-8+LINKAREA+OVERHEAD(sp)

        ld  r1,LINKAREA+OVERHEAD(sp)  // restore r1; deallocate space
        mflr  12        // load ld.so.2 address
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

/* vim:set ts=8 sw=8 et: */
