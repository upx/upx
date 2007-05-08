/*  mipsel-linux.elf-fold.S -- linkage to C code to process Elf binary
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 2000-2007 John F. Reiser
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
*  <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>
*
*  John F. Reiser
*  <jreiser@users.sourceforge.net>
*/

#include "arch/mips/mipsel.r3000/macros.ash"
#include "arch/mips/mipsel.r3000/bits.ash"

        .set mips1
        .set noreorder
        .set noat
        .altmacro

PAGE_SHIFT= 12
PAGE_MASK= 0xffffffffffffffff<<PAGE_SHIFT

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4

sz_l_info= 12
sz_p_info= 12

OVERHEAD=2048
MAX_ELF_HDR=1024

#define sp_frame 0x20
BAL=0x04110000

/* In:
    s7= &decompress
    s5= sz_pack2
    s4= JMPU
    s3= ADRU
    s2= LENU
    s1= ADRX
    s0= LENX
    sp= -sp_frame + &{argc,argv...,0,env...,0,auxv...,0...,strings}
*/
fold_begin:
        addiu a3,sp,4+ sp_frame  # &argv[0]
L10:  # stop when a3= &env[0]
        lw tmp,0(a3)
          addiu a3,a3,4
        bne tmp,zero,L10
          move t1,zero  # &f_unfilter
L20:  # stop when a3= &auxv[0]
        lw tmp,0(a3)
          addiu a3,a3,4
        bne tmp,zero,L20
          move t0,s7  # &f_decompress

        addiu sp,sp,sp_frame - MAX_ELF_HDR  # alloca
        move a2,sp  # &Elf32_Ehdr tmp space
        move a1,s1  # LENX  FIXME 2007-05-07: bad value

/* We need a position-independent call of upx_main, which is external.
   "bal upx_main" cannot be assembled by mipsel-elf-as-20060406.
   ".long BAL + upx_main" then changing R_MIPS_32 to R_MIPS_PC16
     in a utility program, is botched when loaded by multiarch-ld-2.17
     (relocates as if R_MIPS_32, changing the opcode and not
     subtracting the current location).
   So do it the hard way.
*/
        bal 9f  # ra= &9f
          li v0,%lo(9f)
9:
        subu v0,ra,v0
        addiu v0,v0,%lo(upx_main)
        jalr v0
          move a0,s0  # ADRX  FIXME: 2007-05-07 bad value
/* entry= upx_main(b_info *a0, total_size a1, Elf32_Ehdr *a2,
                Elf32_Auxv_t *a3, f_decompr t0, f_unfilter t1 )
*/
        addiu sp,sp, MAX_ELF_HDR
        move a0,s3  # ADRU
        move a1,s2  # LENU
        jr s4       # JMPU
          move ra,v0  # &entry
pic_table:

__NR_Linux = 4000
__NR_brk      =  45+ __NR_Linux
__NR_close    =   6+ __NR_Linux
__NR_exit     =   1+ __NR_Linux
__NR_mmap     =  90+ __NR_Linux
__NR_mprotect = 125+ __NR_Linux
__NR_munmap   =  91+ __NR_Linux
__NR_open     =   5+ __NR_Linux
__NR_read     =   3+ __NR_Linux

err_syscall:
        li a0,-1
exit: .globl exit
        li v0,__NR_exit
sysgo:
   break  # debug only
        syscall
sysret:
        sltiu tmp,v0,PAGE_MASK
        addiu tmp,tmp,-1
        j ra
          or v0,v0,tmp
read: .globl read
        b sysgo; li v0,__NR_read
open: .globl open
        b sysgo; li v0,__NR_open
close: .globl close
        b sysgo; li v0,__NR_close
brk: .globl brk
        b sysgo; li v0,__NR_brk
munmap: .globl munmap
        b sysgo; li v0,__NR_munmap
mprotect: .globl mprotect
        b sysgo; li v0,__NR_mprotect

#define a4_sys 0x10
#define a5_sys 0x14

mmap: .globl mmap
    break  # debug only
        addiu sp,sp,-sp_frame
        sw t0,a4_sys(sp)
        sw t1,a5_sys(sp)
        li v0,__NR_mmap
        syscall
        b sysret
          addiu sp,sp, sp_frame

# vi:ts=8:et:nowrap
