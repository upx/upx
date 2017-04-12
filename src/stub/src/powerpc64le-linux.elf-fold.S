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

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 0
#endif

#define szElf64_Ehdr 0x40
#define szElf64_Phdr 0x38
#define e_phnum 56

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4

sz_l_info= 12
sz_p_info= 12

OVERHEAD= 2048
LINKAREA= 6*8  // (sp,cr,lr, tmp.xlc,tmp.ld,save.toc) save area per calling convention
/* In:
   r31= &decompress; also 8+ (char *)&(#bytes which preceed &-8(r31)
   r28= &Elf64_auxv_t
   r27= actual page size
   r26= o_binfo
   sp/ LINKAREA,regsav(r1-31),&eof_src,argc,argv,0,env,0,auxv,0,chars
*/
fold_begin:
        call L90
#include "arch/powerpc/64le/bxx.S"

zfind:
        ld t0,0(a6)        // parameters are 16byte aligned
        addi a6,a6,8
        cmpldi cr7,t0,0; bne+ cr7,zfind
        ret
L90:
        mflr a5  // &ppcbxx: f_unfilter
        ld a6,LINKAREA(sp)  // sp from regsav at execve
        call zfind  // a6= &env
        call zfind  // a6= &Elf64_auxv; t0= 0
        la a7,LINKAREA+(32-1)*8(sp)  // &reloc
        std t0,0(a7)  // reloc= 0
        la sp,-OVERHEAD(sp)  // -LINKAREA + &Elf64_Ehdr temporary space
        lwz a1,-2*4(r31)  // sz_pack2 == #bytes which preceed -8(r31)
        rlwinm r30,a5,0,0,31-12  // r30= &this_page
        subf r29,a1,r31  // 8+ (char *)&our_Elf64_Ehdr
        mr  a4,r31  // &decompress: f_expand
        addi r29,r29,-2*4  // &our_Elf64_Ehdr
        sub a1,a1,r26  // total_size
        add a0,r29,r26  // &b_info
        la  a2,LINKAREA(sp)  // &Elf64_Ehdr temporary space
        lwz a3,sz_unc(a0)  // sz_elf_headers
#if BIG_ENDIAN  //{
        std a4,3*8(sp); la a4,3*8(sp)  // use tmp.xlc
        std a5,4*8(sp); la a5,4*8(sp)  // use tmp.ld
#endif  //}
        call upx_main
        // a0=entry= upx_main(b_info *a0, total_size a1, Elf64_Ehdr *a2, sz_ehdr a3,
        // f_decomp a4, f_unf a5, Elf64_auxv_t *a6, void *a7=p_reloc)
#if BIG_ENDIAN  //{
        ld r0,LINKAREA+OVERHEAD +(32-1)*8(sp)  // reloc for ET_DYN
        ld r2,8(r3); add r2,r2,r0  // toc
        ld r3,0(r3); add r3,r3,r0  // .func
#endif  //}
        mr r31,a0  // save &entry

        mr a0,r29  // &our_Elf64_Ehdr
        subf a1,r29,r30  // size
        call munmap  // unmap compressed program; /proc/self/exe disappears

        mtlr r31  // entry address

/* Restore Save Area     */
rest:
#if !BIG_ENDIAN  //{
        ld  2,2*8-8+LINKAREA+OVERHEAD(sp)
#endif  //}
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
        li r0,0
        mtcr r0
        mtctr r0
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
