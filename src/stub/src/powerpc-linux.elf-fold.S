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
PATH_MAX= 4096  // /usr/include/linux/limits.h

szElf32_Ehdr= 0x34
szElf32_Phdr= 0x20
e_phnum= 44
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
// SZ_LINKA= (2 + max(0, #params - 8))*4  // (sp,lr, arg9,arg10, ... )
SZ_LINKA= (2 + 2)*4  // (sp,pc, arg9,arg10,... ) and 16-byte aligned
  ARG9= 2*4
  ARG10= 3*4
SZ_FRAME= SZ_LINKA

/* In:
   r31= &decompress; also 2*4+ (char *)&(#bytes which preceed &-8(r31)
   r30= reserved for &this page
   r29= reserved for &our_Elf32_Ehdr
   r28= &Elf32_auxv_t
   r27= actual page size (4K or 64K)
   r26= o_binfo

   r24= ADRU
   r23= LENU
   r22= slide

   r21= &f_unf
*/
#define av_lo  r28
#define adru   r24
#define lenu   r23

#define r_fd   r20
#define av_hi  r19
#define av_len r18
#define cblk   r17
#define pag    r16

fold_begin:
////    teq r0,r0  // debugging
        call L90
L90b:
#include "arch/powerpc/32/bxx.S"
L90a:
        .asciz "   =/proc/self/exe"
        .balign 4
L90:
        mflr r21  // L90b = &ppcbxx: f_unfilter
// open /proc/self/exe
        li a1,O_RDONLY
        la a0,4+ L90a - L90b(r21)  // "/proc/self/exe"
        call open; movr r_fd,a0
// slide {<<stuff>>,argc,argv,0,env,0,auxv} down with maximum room before strings
        la a1,-4(sp)  // src ready for lwzu
        la sp,-(4+ PATH_MAX)(sp)
        la a0,-4(sp)  // dst ready for stwu
        la av_lo,-4(av_lo)  // &end ready for lwzu
0: // copy upto auxv
        lwzu r0,4(a1); cmplw cr7,a1,av_lo
        stwu r0,4(a0); blt cr7,0b
        la  av_lo,8(a0)  // new &auxv
        stwu r0,4(a0)  // new  env_terminator
0: // copy auxv
        lwz  r0,4(a1); cmpli cr7,r0,AT_NULL
        stw  r0,4(a0)
        lwzu r0,2*4(a1)
        stwu r0,2*4(a0); bne cr7,0b
        la av_hi,4(a0)  // tmp end of auxv
        subf av_len,av_lo,av_hi  // length of auxv

        lwz  r0,L90a - L90b(r21)  // "    ="
        la cblk,4(a1)  // original &strings
        stwu r0,4(a0)
        la  r14,4(a0)  // buffer
// readlink("/proc/self/exe", buffer, -1+ PATH_MAX)
        li a2,-1+ PATH_MAX
        movr a1,r14  // buffer
        la a0,4+ L90a - L90b(r21)  //     "/proc/self/exe"
        call readlink; la a2,-4(r14)  // a0= len; a2= buffer
        cmpwi a0,0; bgt 0f  // success
        la a2,L90a - L90b(r21)  // "   =/proc/self/exe"
        li a0,L90  - (4+ L90a)  // round_up(4, strlen("/proc/self/exe"))
0:
        addi a0,a0,4  // len += strlen("   =");
        add a1,a2,a0  // beyond end of path
        mtctr a0
        movr a0,cblk  // old &strings
        li a2,0
        stbu a2,-1(a0)  // NUL terminate
0: // slide path up
        lbzu r0,-1(a1)
        stbu r0,-1(a0); bdnz 0b
        stw a0,-2*4(av_lo)  // &"   =<<path>>" for env

        clrrwi a0,a0,2  // word align
        xor r0,sp,a1  // parity of length
        xor r0,r0,a0  // length vs destination
        rlwinm r0,r0,0,29,29  // andi r0,r0,1<<(31-29)
        sub a0,a0,r0

// slide the rest of the stack up
        movr a1,av_hi
        sub av_lo,a0,av_len
0:
        lwzu r0,-4(a1); cmpl cr7,a1,sp
        stwu r0,-4(a0); bgt+ cr7,0b
        movr sp,a0

        li r0,0
r_reloc= 32 - 1  # use a slot in register save area
        la a7,SZ_FRAME+r_reloc*4(sp)  // &reloc
        stw r22,0(a7)  // reloc === slide
        sub r0,r0,r27  // -pagesize == pagemask
        mr a7,r27  // pagesize
        mr a6,av_lo  // &Elf32_auxv_t
        mr a5,r21  // &f_unf
          and r30,a5,r0  // r30= &this_page
        mr a4,r31  // &decompress: f_expand

        stwu sp,-(SZ_FRAME+OVERHEAD)(sp)  // allocate this frame
        lwz a1,-2*4(r31)  // sz_pack2 == #bytes which preceed -2*4(r31)
        subf r29,a1,r31  // 2*4+ (char *)&our_Elf32_Ehdr
        addi r29,r29,-2*4  // &our_Elf32_Ehdr
          stw r29,ARG9(sp)  // reloc

        add a0,r29,r26  // &b_info
        sub a1,a1,r26  // total_size
        la  a2,SZ_FRAME(sp)  // &Elf32_Ehdr temporary space
        lwz a3,sz_unc(a0)  // sz_elf_headers

        call upx_main  // Out: a0= entry
// entry= upx_main(b_info *a0, total_size a1, Elf32_Ehdr *a2, sz_ehdr a3,
//      f_exp a4, f_unf a5, Elf32_auxv_t *a6, page_size a7, reloc ARG9)
        la  sp,SZ_FRAME+OVERHEAD(sp)  // deallocate this frame
        mr r31,a0  // save &entry

// map the first page of /proc/self/exe
        li a5,0  // offset
        movr a4,r_fd
        li a3,MAP_PRIVATE
        li a2,PROT_READ
        movr a1,r27  // PAGE_SIZE
        li a0,0  // kernel chooses where
        call mmap
// close /proc/self/exe
        movr a0,r_fd
        call close

        movr a0,adru
        movr a1,lenu
        call munmap  // unmap compressed program; /proc/self/exe disappears

        mtlr r31  // entry address
        lmw r2,SZ_FRAME+2*4-4(sp) // restore registers r2 thru r31
        la sp, SZ_FRAME+32*4(sp)  // deallocate outer frame
        ret  // enter /lib/ld.so.1

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
readlink: .globl readlink
        li r0,SYS_readlink; 5: b 5f
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
