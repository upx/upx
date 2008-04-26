/*  powerpc-darwin.macho-fold.S -- linkage to C code to process Mach-o binary
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
retaddr = 2*4  // (sp,cr,pc, xx,yy,zz) save area per calling convention

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4

sz_l_info= 12
sz_p_info= 12

/* Mach_ppc_thread_state */
srr0 = 0*4
srr1 = 1*4
reg0 = 2*4

reg_cr  = 34*4
reg_xer = 35*4
reg_lr  = 36*4
reg_ctr = 37*4
reg_mq  = 38*4
reg_vrsave = 39*4


_start: .globl _start  # ignored, but silence "cannot find entry symbol _start" from ld
/* In:
   r31= &decompress; also 8+ (char *)&(offset to {l_info; p_info; b_info})
*/
fold_begin:
        call L90
#include "arch/powerpc/32/bxx.S"

L90:
        la sp,6*4(sp)  // trim save area used by decompressor
        li   a6,0
        stwu a6,-4(sp)  # _push_ default value for dyld
        movr a6,sp  # &mhdrp
        mflr a5  # &ppcbxx: f_unfilter

        lwz a1,-8(r31)  # offset to {l_info; p_info; b_info}
        subf a0,a1,r31  # &l_info
        lwz a3,sz_unc+sz_p_info+sz_l_info(a0)  # sz_mach_headers
        cmpli 0,a3,2048; bgt L100; li a3,2048  # at least 2 KiB for /usr/lib/dyld
L100:
        movr r29,sp  # remember for restoring later
        subf sp,a3,sp  # alloca
        movr a2,sp  # &temp char[sz_mach_headers]
        addi sp,sp,-4*6  # linkage area assumed by C code (sp,cr,lr, xx,yy,zz)
        movr a4,r31  # f_decompress
        call upx_main  # Out: a0= &Mach_ppc_thread_state of dyld

        movr sp,r29  # restore stack pointer
        lwz t0,   srr0(a0); mtctr t0  # entry address
                /* Next 3 lines probably are not needed, but ... */
        lwz t0, reg_cr(a0); mtcr  t0  # condition code
        lwz t0,reg_xer(a0); mtxer t0  # extended error reg (CArry, etc.)
        lwz t0, reg_lr(a0); mtlr  t0  # link register

        lmw 4,4*4+reg0(3)  # reg 4 thru 31
        lwz 0,0*4+reg0(3)
        lwz 2,2*4+reg0(3)
        lwz 3,3*4+reg0(3)
        bctr  # goto dyld

SYS_exit  =1
SYS_fork  =2
SYS_read  =3
SYS_write =4
SYS_open  =5
SYS_close =6

SYS_pread =153
SYS_mmap    =197
SYS_mprotect= 74

/* SYS_mmap takes a 64-bit off_t, but gcc-3.4.1-20040827 passes long long
   in wrong registers.  So change C interface to use size_t (32-bits) instead
   of off_t (64 bits), and convert here.
*/
pread: .globl pread
        movr a4,a3; li a3,0  # zero extend 4th arg size_t to off_t
        li 0,SYS_pread; b sysgo
mmap: .globl mmap
        movr a6,a5; li a5,0  # zero extend 6th arg size_t to off_t
        li 0,SYS_mmap
sysgo:
        sc
          li a0,-1  # failure return
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

/*
vi:ts=8:et:nowrap
*/

