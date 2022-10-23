/*  powerpc64le-linux.elf-fold.S -- linkage to C code to process ELF binary
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2022 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2022 Laszlo Molnar
*  Copyright (C) 2000-2022 John F. Reiser
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

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 0
#endif

NBPW= 8  //Number of Bytes Per Word
#define USE_TOC 1
#include "arch/powerpc/64le/macros.S"
#include "arch/powerpc/64le/ppc_regs.h"

PATH_MAX= 4096  // /usr/include/linux/limits.h

szElf64_Ehdr= 0x40
szElf64_Phdr= 0x38
e_phnum= 56
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

// http://refspecs.linuxfoundation.org/ELF/ppc64/PPC-elf64abi.html#REG
// r0        Volatile register used in function prologs
// r1        Stack frame pointer
// r2        TOC pointer
// r3        Volatile parameter and return value register
// r4-r10    Volatile registers used for function parameters
// r11       Volatile register used in calls by pointer and as an
//             environment pointer for languages which require one
// r12       Volatile register used for exception handling and glink code
// r13       Reserved for use as system thread ID
// r14-r31   Nonvolatile registers used for local variables
//
// CR0-CR1   Volatile condition code register fields (CR0 '.' int; CR1 '.' floating)
// CR2-CR4   Nonvolatile condition code register fields
// CR5-CR7   Volatile condition code register fields

// http://refspecs.linuxfoundation.org/ELF/ppc64/PPC-elf64abi.html#STACK
//SZ_LINK= 6*8  // (sp,cr,lr, tmp.xlc,tmp.ld,save.toc)
//SZ_PSAV= 8*8  // for arg9, arg10, ...; and for spilling a0-a7 if necessary
// The parameter save area shall be allocated by the caller.
// It shall be doubleword aligned, and shall be at least 8 doublewords in length.

// In:
r_exp=   31  // f_exp == &decompress
r_PMASK= 30  // PAGE_MASK
r_ADRU=  29  // &base to unmap
r_LENU=  28  // length to unmap
r_fd=    27  // open fd of /proc/self/exe
r_auxv=  26
r_elfa=  25  // original &Elf64_Ehdr of stub
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
        sldi r3,r3,9
        blr
        nop

fold_begin:
////    teq r0,r0  // debugging
        call L90
L90b:
#include "arch/powerpc/64le/bxx.S"
L90a:
L_PFX= 4  // strlen("   =")
        .asciz "   =/proc/self/exe"
        .balign 4
L90:
        mflr r_unf  // L90b = &ppcbxx: f_unfilter
// slide {<<stuff>>,argc,argv,0,env,0,auxv} down with maximum room before strings
        la a1,-NBPW(sp)  // src ready for ldu
        la sp,-(-NBPW & (-1+ NBPW + (L_PFX + PATH_MAX)))(sp)  // FR_02
        la a0,-NBPW(sp)  // dst ready for stdu
        la r_auxv,-NBPW(r_auxv)  // &end ready for ldu
0: // copy upto auxv
        ldu  r0,NBPW(a1); cmpld cr7,a1,r_auxv
        stdu r0,NBPW(a0); blt cr7,0b
        la  r_auxv,2*NBPW(a0)  // new &auxv
        stdu r0,NBPW(a0)  // new  env_terminator
0: // copy auxv
        ld   r0,NBPW(a1); cmpldi cr7,r0,AT_NULL
        std  r0,NBPW(a0)
        ldu  r0,2*NBPW(a1)
        stdu r0,2*NBPW(a0); bne cr7,0b
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
        cmpdi a0,0; bgt 0f  // success
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
        std a0,-2*NBPW(r_auxv)  // &"   =<<path>>" for env

        clrrdi a0,a0,3  // word align
        xor r0,sp,a1  // parity of length
        xor r0,r0,a0  // length vs destination
        clrldi r0,r0,-4+ 8*NBPW  // keep bottom 4 bits
        sub a0,a0,r0

// slide the rest of the stack up; leave room for fake TOC of .e_entry
        movr a1,av_hi
        li r0,0; stdu r0,-NBPW(a0); stdu r0,-NBPW(a0)
        movr av_hi,a0  // remember location
        sub r_auxv,a0,av_len
0:
        ldu  r0,-NBPW(a1); cmpld cr7,a1,sp
        stdu r0,-NBPW(a0); bgt+ cr7,0b
        movr sp,a0  // FR_02

r_reloc= 32 - 1  # used slot in register save area
        la a6,SZ_FRAME + r_reloc*NBPW(sp)  // &reloc [adjacent to argc]
        std r_elfa,0(a6)  // elfaddr

        stdu sp,-(SZ_FRAME+OVERHEAD)(sp)  // FR_01  allocate this frame
        mr a0,r_ADRX  // &b_info
        mr a1,r_LENX  // total_size
        la a2,SZ_FRAME(sp)  // &Elf64_Ehdr temporary space
        mr a3,r_auxv  // &Elf64_auxv_t
        mr a4,r_exp  // &decompress: f_expand
        mr a5,r_unf  // &f_unf
#if USE_TOC  //{
        li r0,0

        std a4,F_TOC +    0 + 0*2*NBPW(sp)
        la  a4,F_TOC +    0 + 0*2*NBPW(sp)
        std r0,F_TOC + NBPW + 0*2*NBPW(sp)

        std a5,F_TOC +    0 + 1*2*NBPW(sp)
        la  a5,F_TOC +    0 + 1*2*NBPW(sp)
        std r0,F_TOC + NBPW + 1*2*NBPW(sp)
#endif  //}
        mr a7,r_PMASK  // PAGE_MASK
        call upx_main  // Out: a0= entry
// entry= upx_main(b_info *a0, total_size a1, Elf64_Ehdr *a2, ELf32_auxv_t *a3,
//      f_exp a4, f_unf a5, elfaddr &a6, page_mask a7)
        la  sp,SZ_FRAME+OVERHEAD(sp)  // FR_01  deallocate this frame
#if USE_TOC  //{
// Sometimes the "entry TOC" is not a TOC, particularly with musl.
        movr r2,av_hi  // default fake TOC
        andi. r0,a0,7; bne cr0,noTOC  // align(TOC) < 8;  [heuristic]
        lwz r0,0(a0); cmplwi r0,0; bne noTOC  // 4GiB <= .func;  an instruction?

        ld r0, SZ_FRAME + r_reloc*NBPW(sp)  // reloc for ET_DYN
        ld r2,NBPW(r3); add r2,r2,r0  // toc
        ld r3,   0(r3); add r3,r3,r0  // .func   NOTE: r3 === a0
noTOC:
        std r3,0(av_hi)  // .func for .e_entry
#endif  //}
        mr r_exp,a0  // save &entry (.entry when BIG_ENDIAN)

p_memsz= 4+4+ 4*NBPW
// Discard pages of compressed data (includes [ADRX,+LENX) )
        ld a1,p_memsz+szElf64_Phdr+szElf64_Ehdr(r_elfa)  // Phdr[C_TEXT= 1].p_memsz
        movr a0,r_elfa  // hi elfaddr
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

        ld r0,-NBPW(av_hi)  // &hatch
        mtctr r0
        ld r0,0(av_hi)  // entry address
        std r0,SZ_FRAME + (-1+ 31)*NBPW(sp)  // hatch: "movr r12,r31" for musl 1.1.16

        mr a0,r_ADRU
        mr a1,r_LENU
        li r0,SYS_munmap
        mtlr r_exp  // entry address

//    BIG_ENDIAN: r2 (TOC) already is live (set after return from upx_main)
// LITTLE_ENDIAN: r2 never is touched
//      ld   2,SZ_FRAME + (-1+  2)*NBPW(sp)
// r3,r4 are a0,a1 which are parameters to munmap()
//      ld  3,SZ_FRAME + (-1+  3)*NBPW(sp)
//      ld  4,SZ_FRAME + (-1+  4)*NBPW(sp)
        ld  5,SZ_FRAME + (-1+  5)*NBPW(sp)
        ld  6,SZ_FRAME + (-1+  6)*NBPW(sp)
        ld  7,SZ_FRAME + (-1+  7)*NBPW(sp)
        ld  8,SZ_FRAME + (-1+  8)*NBPW(sp)
        ld  9,SZ_FRAME + (-1+  9)*NBPW(sp)
        ld 10,SZ_FRAME + (-1+ 10)*NBPW(sp)
        ld 11,SZ_FRAME + (-1+ 11)*NBPW(sp)
        ld 12,SZ_FRAME + (-1+ 12)*NBPW(sp)
        ld 13,SZ_FRAME + (-1+ 13)*NBPW(sp)
        ld 14,SZ_FRAME + (-1+ 14)*NBPW(sp)
        ld 15,SZ_FRAME + (-1+ 15)*NBPW(sp)
        ld 16,SZ_FRAME + (-1+ 16)*NBPW(sp)
        ld 17,SZ_FRAME + (-1+ 17)*NBPW(sp)
        ld 18,SZ_FRAME + (-1+ 18)*NBPW(sp)
        ld 19,SZ_FRAME + (-1+ 19)*NBPW(sp)
        ld 20,SZ_FRAME + (-1+ 20)*NBPW(sp)
        ld 21,SZ_FRAME + (-1+ 21)*NBPW(sp)
        ld 22,SZ_FRAME + (-1+ 22)*NBPW(sp)
        ld 23,SZ_FRAME + (-1+ 23)*NBPW(sp)
        ld 24,SZ_FRAME + (-1+ 24)*NBPW(sp)
        ld 25,SZ_FRAME + (-1+ 25)*NBPW(sp)
        ld 26,SZ_FRAME + (-1+ 26)*NBPW(sp)
        ld 27,SZ_FRAME + (-1+ 27)*NBPW(sp)
        ld 28,SZ_FRAME + (-1+ 28)*NBPW(sp)
        ld 29,SZ_FRAME + (-1+ 29)*NBPW(sp)
        ld 30,SZ_FRAME + (-1+ 30)*NBPW(sp)
        ld 31,SZ_FRAME + (-1+ 31)*NBPW(sp)
        la sp,SZ_FRAME +      32 *NBPW(sp)  // FR_00  deallocate outer frame

        bctr  // goto escape hatch

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
SYS_readlink=  85

mmap: .globl mmap
        clrldi  a4,a4,32  // truncate fd to 32 bits; kernel is picky?
        li 0,SYS_mmap
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
//write: .globl write
//        li r0,SYS_write;    5: b 5f
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
        teq r0,r0  // my_bkpt
        blr

/* vim:set ts=8 sw=8 et: */
