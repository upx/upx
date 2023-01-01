/*  mipsel-linux.elf-fold.S -- linkage to C code to process Elf binary
*
*  This file is part of the UPX executable compressor.
*
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

NBPW=  4
#include "arch/mips/r3000/macros.ash"
#include "arch/mips/r3000/bits.ash"

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
__NR_write    =   4+ __NR_Linux

PATHSIZE=4096
OVERHEAD=2048
MAX_ELF_HDR=512

MAP_PRIVATE=  0x002
MAP_ANONYMOUS=0x800  # not same as i386
PROT_READ= 1

O_RDONLY= 0

get_page_mask:
        li v0,0  // modified to PAGE_MASK >> 9
        jr ra
          sll v0,v0,9
        nop

sp_frame= 0x20
F_PMASK= 4*NBPW
F_fd=    5*NBPW
F_ADRU=  6*NBPW
F_LENU=  7*NBPW
  // The above 4 registers are passed on stack to unfolded code.
a4_sys=  4*NBPW
a5_sys=  5*NBPW

// C-language offers 8 register args; syscall offers only 4
#define a4  t0
#define a5  t1

//ra             31
#define r_fexp   30  /* s8 */
//sp             29  /* hardware */
#define r_PMASK  28  /* gp */
//k1             27  /* trashed by syscall */
//k0             26  /* trashed by syscall */
//t9, jp         25  /* trashed by syscall ? */
//t8             24  /* trashed by syscall ? */
//s7 AVAIL       23  /* s7 */  # saved temp for /proc/self/exe
#define r_auxv   22  /* s6 */
#define r_elfa   21  /* s5 */
#define r_auxe   20  /* s4 */
//s3 AVAIL       19  /* s3 */  # previously r_LENU
//s2 AVAIL       18  /* s2 */  # previously r_ADRU
#define r_LENX   17  /* s1 */
#define r_ADRX   16  /* s0 */

/* In:
    r_ADRX,r_LENX,r_elfa,r_auxv,r_PMASK,r_fexp
    sp= -sp_frame{%,%,%,%,PMASK,%fd,ADRU,LENU}, {argc,argv...,0,env...,0,auxv...,0,0,strings}
*/
fold_begin:
////    break
        move v0,sp
        addiu sp,(~0<<4)&-(NBPW+ 4+ PATHSIZE - sp_frame)  # alloca: new envp[0], "   =", buffer

        move v1,sp
L10:  # copy until auxv
        lw tmp,0(v0); addiu v0,NBPW
        sw tmp,0(v1); addiu v1,NBPW
        bne v0,$r_auxv,L10
          addiu t1,v1,-NBPW  // new envp goes here
        sw zero,(v1); addiu v1,NBPW  // new terminator for envp
        move $r_auxv,v1  // new auxv
L30:  // copy auxv
        lw tmp,0(v0); lw t0,NBPW(v0); addiu v0,sz_auxv
        sw tmp,0(v1); sw t0,NBPW(v1); addiu v1,sz_auxv
        bnez tmp,L30  # AT_NULL: stop when v0= &auxv[N]
          move $r_auxe,v1  // end of new auxv

        sw v1,0(t1)  # new env var
        li tmp,' '
        sb tmp,0(v1)  # endian neutral!
        sb tmp,1(v1)
        sb tmp,2(v1)
        li tmp,'='
        sb tmp,3(v1)
        addiu s7,v1,4  # &buf[0]

        bal 9f
          sw ra,F_fd(sp)  # "/proc/self/exe"
        .asciz "/proc/self/exe"
        .balign 4
9:
        li a1,O_RDONLY
        move a0,ra  # "/proc/self/exe"
        li v0,__NR_open; syscall

        li a2,PATHSIZE-1
        move a1,s7  # &buf[0]
        lw a0,F_fd(sp)  # "/proc/self/exe"
        sw v0,F_fd(sp)  # result of __NR_open
        li v0,__NR_readlink; syscall
        bltz a3,0f
          addu tmp,a1,v0
        sb $0,(tmp)  # null terminate the path
0:
        addiu sp,-MAX_ELF_HDR  # alloca
        move t3,$r_PMASK  # page_mask
        move t2,$r_elfa  # &Elf32_Ehdr of stub
        move t1,zero  # &f_unfilter
        move t0,$r_fexp  # &f_decompress
        move a3,$r_auxv  # new &auxv[0]
        move a2,sp  # &Elf32_Ehdr tmp space
        move a1,$r_LENX  # total_size

BAL=0x04110000
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
          move a0,$r_ADRX
/* entry= upx_main(b_info *a0, total_size a1, Elf32_Ehdr *a2, Elf32_Auxv_t *a3,
        f_decompr t0, f_unfilter t1, Elf32_Ehdr &t2, page_mask t3 )
*/
        addiu sp,MAX_ELF_HDR  # un-alloca
        move $r_fexp,v0  # &entry

p_vaddr= 4+4
// Discard pages of compressed data (includes [ADRX,+LENX) )
        lw a1,p_vaddr+sz_Phdr+sz_Ehdr($r_elfa)  // Phdr[C_TEXT= 1].p_vaddr
        lw v0,p_vaddr        +sz_Ehdr($r_elfa)  // Phdr[C_BASE= 0].p_vaddr
        move a0,$r_elfa  // hi elfaddr
        subu a1,a1,v0  // length
        li v0,__NR_munmap; syscall

// Map 1 page of /proc/self/exe so that munmap does not remove all references
        lw   a4,F_fd(sp)
        move a5,$0  // offset
          sw a4,a4_sys(sp)
          sw a5,a5_sys(sp)
        li   a3,MAP_PRIVATE
        li   a2,PROT_READ
        neg  a1,$r_PMASK  // PAGE_SIZE
        move a0,$0  // addr
        li v0,__NR_mmap; syscall
// close /proc/self/exe
        lw a0,a4_sys(sp)  // fd
        li v0,__NR_close; syscall

        lw a1,F_LENU(sp)  # prepare for munmap() at escape hatch
        lw a0,F_ADRU(sp)
        addiu sp,sp,sp_frame

/* Workaround suspected glibc bug: elf/rtld.c assumes uninit local is zero.
   2007-11-24 openembedded.org mipsel-linux 2.6.12.6/glibc 2.3.2

   Also early uClibc/ldso/ldso/ldso.c function _dl_get_ready_to_run() forgot
     _dl_memset(app_tpnt, 0, sizeof(*app_tpnt));
   leaving garbage in
            if (app_tpnt->dynamic_info[DT_TEXTREL]) {
   leading to SIGSEGV.  Fixed in [git blame:]
27d501fdbf (Denis Vlasenko           2009-01-10 21:02:48 +0000
*/
        move tmp,sp
        addiu sp, -0x380  # estimated stack bound of upx_main and below
0:
        addiu sp,NBPW
        bne sp,tmp,0b
          sw $0,-NBPW(sp)

        lw tmp,-sz_auxv+ a_val($r_auxe)  // last .a_val
          nop  # needed? allegedly MIPS R3000 provides load delay in hardware when needed
        beqz tmp,L40  # could not make escape hatch
          nop  # even R3000 requires branch delay
        jr tmp  # goto munmap escape hatch: [syscall; jr $r_fexp; nop]
          li v0,__NR_munmap
L40:
        jr $r_fexp  # omit munmap
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
write: .globl write
        b sysgo; li v0,__NR_write
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

mmap_privanon: .globl mmap_privanon
        ori a3,a3,MAP_PRIVATE|MAP_ANONYMOUS
        li t0,-1  # fd
        li t1,0   # offset
mmap: .globl mmap
        addiu sp,sp,-sp_frame
        sw a4,a4_sys(sp)
        sw a5,a5_sys(sp)
        li v0,__NR_mmap; syscall
        b sysret
          addiu sp,sp,sp_frame
#endif  /*}*/

/* vim:set ts=8 sw=8 et: */
