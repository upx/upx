/* arm64-darwin.macho-entry.S -- iPhone program entry point & decompressor (Elf binary)
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

NBPW= 8
#include "arch/arm64/v8/macros.S"

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8
sz_l_info= 12
sz_p_info= 12

PROT_READ=  1
PROT_WRITE= 2
PROT_EXEC=  4

MAP_PRIVATE= 2
MAP_FIXED=     0x10
MAP_ANONYMOUS= 0x20

PAGE_SHIFT= 12
PAGE_SIZE = -(~0<<PAGE_SHIFT)

__NR_SYSCALL_BASE = 0

__NR_exit =      1 + __NR_SYSCALL_BASE
__NR_write =     4 + __NR_SYSCALL_BASE
__NR_open =      5 + __NR_SYSCALL_BASE
__NR_mmap =    197 + __NR_SYSCALL_BASE
__NR_mprotect=  74 + __NR_SYSCALL_BASE

// DEBUG ONLY:
__ARM_NR_BASE  = 0xf0000 + __NR_SYSCALL_BASE
__ARM_NR_cacheflush =  2 + __ARM_NR_BASE

        //.long sz_pack2  // placed there by ::pack3()
//0:    .word 0b - &Mach64_header  // backwards distance to Mach64_header
//0:    .word 0b - l_info  // backwards distance to l_info

  section MACHMAINX
_start: .globl _start
        sub sp,sp,#NBPW  // space for mhdrp
        stp x0,x1,[sp,#-30*8]!  // paranoia; omit r30(lr), r31(sp)
        stp x2,x3,[sp,#2*8]
        stp x4,x5,[sp,#4*8]
        stp x6,x7,[sp,#6*8]
        stp x8,x9,[sp,#8*8]
        stp x10,x11,[sp,#10*8]
        stp x12,x13,[sp,#12*8]
        stp x14,x15,[sp,#14*8]
        stp x16,x17,[sp,#16*8]
        stp x18,x19,[sp,#18*8]
        stp x20,x21,[sp,#20*8]
        stp x22,x23,[sp,#22*8]
        stp x24,x25,[sp,#24*8]
        stp x26,x27,[sp,#26*8]
        stp x28,x29,[sp,#28*8]
        bl main
L20:
f_decompress:

  section NRV_HEAD
        // empty

  section NRV2E
#include "arch/arm64/v8/nrv2e_d32.S"

  section NRV2D
#include "arch/arm64/v8/nrv2d_d32.S"

  section NRV2B
#include "arch/arm64/v8/nrv2b_d32.S"

  section NRV_TAIL
        // empty

#include "arch/arm64/v8/lzma_d.S"

  section MACHMAINY
end_decompress: .globl end_decompress

        /* IDENTSTR goes here */

r_RELOC .req x12  // same as r_MHDR
r_MHDR  .req x12
r_LENX  .req x11
r_ADRX  .req x10
r_FOLD  .req x6

arg1 .req x0; arg1w .req w0
arg2 .req x1; arg2w .req w1
arg3 .req x2; arg3w .req w2
arg4 .req x3; arg4w .req w3
arg5 .req x4; arg5w .req w4
arg6 .req x5; arg6w .req w5

rdi .req x0
//rsi .req x1
rdx .req x2
rcx .req x3

rax .req x7; eax .req w7
rbx .req x8
rbp .req x9

  section MACHMAINZ
lsrc .req w1
ldst .req w3
ldstx .req x3

unfold:  // lr= &b_info;  sp/ fd,%ADRU,%LENU,%entry,%&Mach_header
        mov rbx,lr  // &b_info:{sz_unc, sz_cpr, 4{byte}}, compressed_fold...}
        ldr eax,[rbx,#sz_unc]  // LENU.dynamic
        mov rdx,rbx
        sub rdx,rdx,r_MHDR  // LENU.static
        add rdx,rdx,eax,uxtw
        str rdx,[sp,#2*NBPW]  // LENU

// Reserve space for input file and unfolded stub.
        mov arg2,rdx  // len
        mov arg6w,#0  // offset
        mov arg5w,#-1  // FD_ANON
        mov arg4w,#MAP_PRIVATE|MAP_ANONYMOUS
        mov arg3w,#PROT_READ|PROT_WRITE
        mov arg1,#0  // kernel chooses addr
        do_sys __NR_mmap
        str x0,[sp,#1*NBPW]  // ADRU
        sub r_ADRX,r_ADRX,r_MHDR  // offset(&l_info)
        add r_ADRX,r_ADRX,x0  // new &l_info

// Duplicate the input data.
        mov arg6,#0  // offset
        ldr arg5w,[sp,#0*NBPW]  // fd
        mov arg4w,#MAP_PRIVATE|MAP_FIXED
        mov arg3w,#PROT_READ|PROT_WRITE
        sub arg2,rbx,r_MHDR  // len
        // mov arg1,x0  // same address
        do_sys __NR_mmap

// Remember new f_exp region for PROT_EXEC.
        ldr rdx,[sp,#2*NBPW]  // LENU
        ldr rcx,[sp,#4*NBPW]  // &Mach_header
        add rdx,rdx,rax  // new last of unfolded
        sub r_RELOC,rax,rcx  // relocation constant = new - old
        add rax,rax,rbp; str rbp,[sp,#-NBPW]!  // P_10  new f_exp
        lsr rax,rax,#PAGE_SHIFT
        lsl rax,rax,#PAGE_SHIFT
        str rax,[sp,#-NBPW]!  // P_11 address
        sub rdx,rdx,rax
        str rdx,[sp,#-NBPW]!  // P_12 length
// Unfold
        ldr eax,[rbx,#sz_unc]  // dstlen
        str rax,[sp,#-NBPW]; mov arg4,sp  // P_13  &dstlen
        add arg3,rbx,r_RELOC  // dst= new unfold
        mov r_FOLD,arg3  // execute here
        ldr  eax,[rbx,#sz_cpr]  // srclen
        ldr arg5w,[rbx,#b_method]
        add arg1,rbx,#sz_b_info  // src
        mov arg2,rax  // srclen
        blr rbp  // old f_exp
        add sp,sp,#NBPW  // P_13  toss dstlen

// PROT_EXEC
        ldr arg2,[sp],#NBPW  // P_12  length
        ldr arg1,[sp],#NBPW  // P_11  addr
        ldr  rbp,[sp],#NBPW  // P_10  new f_exp
        mov arg3w,#PROT_READ|PROT_EXEC
        do_sys __NR_mprotect

// Use the copy.
// r10= ADRX; r11= LENX; r9= f_exp
// rsp/ fd,ADRU,LENU,%entry,&Mach_header
        br r_FOLD

execp:
        .ascii "executable_path="  // 16 bytes
L99:
        brk #0

main:
        mov x5,lr  // &f_decompress
        add x2,sp,#NBPW  // &argv[0]
0:
        ldr  x0,[x2],#NBPW
        cbnz x0,0b  // past argv
0:
        ldr  x0,[x2],#NBPW
        cbnz x0,0b  // past envp
L30:
        ldr x0,[x2],#NBPW  // x0= *apple++
        cbz x0,L99
        adr x1,execp; mov w3,#16
L40:
        ldrb w4,[x0],#1
        ldrb w5,[x1],#1
        sub w3,w3,#1
        cmp w4,w5; bne L30  // mismatch ==> next apple[]
        cbnz w3,L40  // not entire prefix

        mov arg2,#0  // O_RDONLY
        do_sys __NR_open
        str x5,[sp,#-5*NBPW]!  // fd,%ADRU,%LENU,%entry,%&Mach_header

        adr x1,-2*4 + _start

        mov r_MHDR,x1
        ldr w0,[x1],#4
        sub r_MHDR,r_MHDR,w0,uxtw // &Mach_header
        str r_MHDR,[sp,#4*NBPW]

        mov r_ADRX,x1
        ldr w0,[x1],#4
        sub r_ADRX,r_ADRX,w0,uxtw  // &l_info

        sub r_LENX,x0,#2*4  // omit words before _start
        bl unfold
          /* { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...} */

/* vim:set ts=8 sw=8 et: */
