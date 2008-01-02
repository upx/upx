/*  mipsel-linux.elf-fold.S -- linkage to C code to process Elf binary
*
*  This file is part of the UPX executable compressor.
*
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

#include "arch/mips/mipsel.r3000/macros.ash"
#include "arch/mips/mipsel.r3000/bits.ash"

        .set mips1
        .set noreorder
        .set noat
        .altmacro

PAGE_SHIFT= 12
PAGE_MASK= 0xffffffffffffffff<<PAGE_SHIFT

sz_Ehdr= 52
sz_Phdr= 32

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4

sz_l_info= 12
sz_p_info= 12

sz_auxv= 8
a_type = 0  # Elf32_auxv_t
a_val  = 4

__NR_Linux = 4000
__NR_brk      =  45+ __NR_Linux
__NR_close    =   6+ __NR_Linux
__NR_exit     =   1+ __NR_Linux
__NR_mmap     =  90+ __NR_Linux
__NR_mprotect = 125+ __NR_Linux
__NR_munmap   =  91+ __NR_Linux
__NR_open     =   5+ __NR_Linux
__NR_read     =   3+ __NR_Linux
__NR_readlink =  85+ __NR_Linux

PATHSIZE=4096
OVERHEAD=2048
MAX_ELF_HDR=512

#define sp_frame 0x20
BAL=0x04110000

/* In:
    s7= &decompress

    s5= sz_pack2

    s3= ADRU
    s2= LENU
    s1= ADRX

    sp= -sp_frame + &{argc,argv...,0,env...,0,auxv...,0,0,strings}
*/
fold_begin:
        addiu v0,sp,sp_frame  # &argc
        addiu sp,-(4+ 4+ PATHSIZE - sp_frame)  # alloca: new envp[0], "   =", buffer
        move v1,sp
L10:  # copy argc,argv
        lw tmp,0(v0); addiu v0,4
        sw tmp,0(v1); addiu v1,4
        bnez tmp,L10  # stop when v0= &env[0]
          move s0,v1  # new &env[0]
        addiu v1,4  # leave space for new env[0]
L20:  # copy env
        lw tmp,0(v0); addiu v0,4
        sw tmp,0(v1); addiu v1,4
        bnez tmp,L20  # stop when v0= &auxv[0]
          move s4,v1  # new &auxv[0]
L30:  # copy auxv
        lw tmp,0(v0); lw t0,4(v0); addiu v0,sz_auxv
        sw tmp,0(v1); sw t0,4(v1); addiu v1,sz_auxv
        bnez tmp,L30  # stop when v0= &auxv[N]
          move s6,v1  # new &auxv[N]

        sw v1,0(s0)  # new env[0]
        li tmp,' '
        sb tmp,0(v1)  # endian neutral!
        sb tmp,1(v1)
        sb tmp,2(v1)
        li tmp,'='
        sb tmp,3(v1)

        li a2,PATHSIZE-1
        addiu a1,v1,4  # &buf[0]
        bal 9f
          move a0,ra
        .asciz "/proc/self/exe"
        .balign 4
9:
        li v0,__NR_readlink
        syscall
        bltz a3,0f
          addu tmp,a1,v0
        sb $0,(tmp)  # null terminate the path
0:
        addiu sp,-MAX_ELF_HDR  # alloca
        move t1,zero  # &f_unfilter
        move t0,s7  # &f_decompress
        move a3,s4  # new &auxv[0]
        move a2,sp  # &Elf32_Ehdr tmp space
        addiu a1,s5,-(sz_Ehdr + 2*sz_Phdr + sz_l_info + sz_p_info)

/* We need a position-independent call of upx_main, which is external.
   "bal upx_main" cannot be assembled by mipsel-elf-as-20060406.
   ".long BAL + upx_main" then changing R_MIPS_32 to R_MIPS_PC16
     in a utility program, is botched when loaded by multiarch-ld-2.17
     (relocates as if R_MIPS_32, changing the opcode and not
     subtracting the current location).
   So do it the hard way.
*/
        bltzal $0,9f  # ra= &9f; no branch (condition is false!)
          li v0,%lo(9f)
9:
        subu v0,ra,v0
        addiu v0,v0,%lo(upx_main)
        jalr v0
          move a0,s1
/* entry= upx_main(b_info *a0, total_size a1, Elf32_Ehdr *a2,
                Elf32_Auxv_t *a3, f_decompr t0, f_unfilter t1 )
*/
        move jp,v0  # &entry
/* Workaround suspected glibc bug: elf/rtld.c assumes uninit local is zero.
   2007-11-24 openembedded.org mipsel-linux 2.6.12.6/glibc 2.3.2
*/
        addiu tmp, sp, MAX_ELF_HDR  # result of un-alloca
        addiu sp, -300  # estimated stack bound of upx_main and below
0:
        addiu sp,4
        bne sp,tmp,0b
          sw $0,-4(sp)

        lw tmp,-sz_auxv+ a_val(s6)
          move a1,s2  # LENU
        beqz tmp,L40  # could not make escape hatch
          move a0,s3  # ADRU
        jr tmp  # goto munmap escape hatch: [syscall; jr jp; nop]
          li v0,__NR_munmap
L40:
        jr jp  # omit munmap
          nop

#if 0  /*{ replaced by macros in include/linux.h because of 'bal' vs gcc */
err_syscall:
        li a0,-1
exit: .globl exit
        li v0,__NR_exit
sysgo:
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
        addiu sp,sp,-sp_frame
        sw t0,a4_sys(sp)
        sw t1,a5_sys(sp)
        li v0,__NR_mmap
        syscall
        b sysret
          addiu sp,sp, sp_frame
#endif  /*}*/

# vi:ts=8:et:nowrap
