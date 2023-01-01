/*  powerpc-linux.elf-fold.S -- linkage to C code to process ELF binary
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2023 Laszlo Molnar
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

NBPW= 4  // Number of Bytes Per Word
#include "arch/powerpc/32/macros.S"
#include "arch/powerpc/32/ppc_regs.h"

PATH_MAX= 4096  // /usr/include/linux/limits.h

AT_NULL= 0

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4

sz_l_info= 12
sz_p_info= 12

MAP_PRIVATE=   0x02
MAP_FIXED=     0x10

PROT_READ=     0x1

O_RDONLY=       0

OVERHEAD= 2048

// http://refspecs.linuxfoundation.org/elf/elfspec_ppc.pdf  p.3-17 (pdf p.33)
// SZ_LINKA= (2 + max(0, #params - 8))*NBPW  // (sp,lr, arg9,arg10, ... )
SZ_LINKA= (2 + 2)*NBPW  // (sp,pc, arg9,arg10,... ) and 16-byte aligned
SZ_FRAME= SZ_LINKA

// In:
r_exp=   31  // f_exp == &decompress
r_PMASK= 30  // PAGE_MASK
r_ADRU=  29  // &base to unmap
r_LENU=  28  // length to unmap
r_fd=    27  // open fd of /proc/self/exe
r_auxv=  26
r_elf=   25  // original &Elf32_Ehdr of stub
r_ADRX=  24  // compressed input
r_LENX=  23  // total size
// Local:
r_unf=   22  // f_unf == &unfilter()
av_hi=   21
av_len=  20
cblk=    19
r_buf=   18

get_page_mask: .globl get_page_mask
        li r3,0 -0x1000>>9  // modified by elf-entry.S
        slwi r3,r3,9
        blr
        nop

fold_begin:
        call L90
L90b:
#include "arch/powerpc/32/bxx.S"
L90a:
L_PFX= 4  // strlen("   =")
        .asciz "   =/proc/self/exe"
        .balign 4
L90:
        mflr r_unf  // L90b = &ppcbxx: f_unfilter
// slide {<<stuff>>,argc,argv,0,env,0,auxv} down with maximum room before strings
        la a1,-NBPW(sp)  // src ready for lwzu
        la sp,-(-NBPW & (-1+ NBPW + (L_PFX + PATH_MAX)))(sp)
        la a0,-NBPW(sp)  // dst ready for stwu
        la r_auxv,-NBPW(r_auxv)  // &end ready for lwzu
0: // copy upto auxv
        lwzu r0,NBPW(a1); cmplw cr7,a1,r_auxv
        stwu r0,NBPW(a0); blt cr7,0b
        la  r_auxv,2*NBPW(a0)  // new &auxv
        stwu r0,NBPW(a0)  // new  env_terminator
0: // copy auxv
        lwz  r0,NBPW(a1); cmplwi cr7,r0,AT_NULL
        stw  r0,NBPW(a0)
        lwzu r0,2*NBPW(a1)
        stwu r0,2*NBPW(a0); bne cr7,0b
        la av_hi,NBPW(a0)  // tmp end of auxv
        subf av_len,r_auxv,av_hi  // length of auxv

        lwz  r0,L90a - L90b(r_unf)  // "    ="
        la cblk,NBPW(a1)  // original &strings
        stwu r0,NBPW(a0)
        la  r_buf,L_PFX(a0)  // buffer
// r_fd = open("/proc/self/exe", O_RDONLY)
        li a1,O_RDONLY
        la a0,L_PFX+ L90a - L90b(r_unf)  //     "/proc/self/exe"
        call open; mr r_fd,a0
// readlink("/proc/self/exe", buffer, -1+ PATH_MAX)
        li a2,-1+ PATH_MAX
        movr a1,r_buf  // buffer
        la a0,L_PFX+ L90a - L90b(r_unf)  //     "/proc/self/exe"
        call readlink; la a2,-L_PFX(r_buf)  // a0= len; a2= buffer
        cmpwi a0,0; bgt 0f  // success
        la a2,L90a - L90b(r_unf)  // "   =/proc/self/exe"
        li a0,L90  - (NBPW+ L90a)  // round_up(NBPW, strlen("/proc/self/exe"))
0:
        addi a0,a0,L_PFX  // len += strlen("   =");
        add a1,a2,a0  // beyond end of path
        mtctr a0
        movr a0,cblk  // old &strings
        li a2,0
        stbu a2,-1(a0)  // NUL terminate
0: // slide path up
        lbzu r0,-1(a1)
        stbu r0,-1(a0); bdnz 0b
        stw a0,-2*NBPW(r_auxv)  // &"   =<<path>>" for env

        clrrwi a0,a0,2  // word align
        xor r0,sp,a1  // parity of length
        xor r0,r0,a0  // length vs destination
        clrlwi r0,r0,-3+ 8*NBPW  // keep bottom 3 bits
        sub a0,a0,r0

// slide the rest of the stack up
        movr a1,av_hi
        sub r_auxv,a0,av_len
0:
        lwzu r0,-NBPW(a1); cmplw cr7,a1,sp
        stwu r0,-NBPW(a0); bgt+ cr7,0b
        movr sp,a0

        stwu sp,-(SZ_FRAME+OVERHEAD)(sp)  // allocate this frame
        mr a0,r_ADRX  // &b_info
        mr a1,r_LENX  // total_size
        la a2,SZ_FRAME(sp)  // &Elf32_Ehdr temporary space
        mr a3,r_auxv  // &Elf32_auxv_t
        mr a4,r_exp  // &decompress: f_expand
        mr a5,r_unf  // &f_unf
        mr a6,r_elf  // elfaddr
        call upx_main  // Out: a0= entry
// entry= upx_main(b_info *a0, total_size a1, Elf32_Ehdr *a2, ELf32_auxv_t *a3,
//      f_exp a4, f_unf a5, elfaddr a6)

        la  sp,SZ_FRAME+OVERHEAD(sp)  // deallocate this frame
        mr r_exp,a0  // save &entry

sz_Ehdr= 13*NBPW
sz_Phdr= 8*NBPW
p_vaddr= 4+4
// Discard pages of compressed data (includes [ADRX,+LENX) )
        lwz a1,p_vaddr+sz_Phdr+sz_Ehdr(r_elf)  // Phdr[C_TEXT= 1].p_vaddr
        lwz r0,p_vaddr        +sz_Ehdr(r_elf)  // Phdr[C_BASE= 0].p_vaddr
        movr a0,r_elf  // hi elfaddr
        subf a1,r0,a1  // length
        call munmap  # discard C_TEXT compressed data

// first page of /proc/self/exe, to preserve it despite munmap(ADRU, LENU)
        li a5,0  // offset
        movr a4,r_fd
        li a3,MAP_PRIVATE
        li a2,PROT_READ
        neg a1,r_PMASK  // PAGE_SIZE
        li a0,0  // kernel chooses where
        call mmap
// close /proc/self/exe
        movr a0,r_fd
        call close

AT_NULL= 0  // <elf.h>
a_type= 0
a_val= NBPW
sz_auxv= 2*NBPW

// find the escape hatch in auxv[{AT_NULL}].a_val
        mr a0,r_auxv
0:
        lwz r0,a_type(a0); lwz a1,a_val(a0); addi a0,a0,sz_auxv
        cmplwi cr7,r0,AT_NULL; bne+ cr7,0b
        mtctr a1  // escape hatch

        mr a0,r_ADRU
        mr a1,r_LENU
        li r0,SYS_munmap
        mtlr r_exp  // entry address

        lmw a2,SZ_FRAME+(a2 - r2 + 2)*NBPW - NBPW(sp) // restore registers a2 thru r31
        la sp, SZ_FRAME+32*NBPW(sp)  // deallocate outer frame

        bctr  // goto escape hatch

SYS_exit=  1
SYS_fork=  2
SYS_read=  3
SYS_write= 4
SYS_open=  5
SYS_close= 6

SYS_brk=       45
SYS_readlink=  85
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
brk: .globl brk
        li r0,SYS_brk;      5: b 5f
readlink: .globl readlink
        li r0,SYS_readlink; 5: b 5f
write: .globl write
        li r0,SYS_write;    5: b 5f
read: .globl read
        li r0,SYS_read;     5: b 5f
open: .globl open
        li r0,SYS_open;     5: b 5f
close: .globl close
        li r0,SYS_close;    5: b 5f
mprotect: .globl mprotect
        li r0,SYS_mprotect; 5: b 5f
munmap: .globl munmap
        li r0,SYS_munmap;   5: b sysgo

my_bkpt: .globl my_bkpt
        teq r0,r0
        blr
/* vim:set ts=8 sw=8 et: */
