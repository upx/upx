/*  arm64-linux.elf-so_entry.S -- Linux DT_INIT & decompressor (Elf shared lib)
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2021 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2021 Laszlo Molnar
*  Copyright (C) 2000-2025 John F. Reiser
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
lr .req x30

#define DEBUG 0

sz_Elf64_Ehdr = 8*NBPW
sz_Elf64_Phdr = 7*NBPW

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

// /usr/include/asm-generic/unistd.h
__NR_close    = 0x39  //  57
__NR_exit     = 0x5d  //  93
__NR_memfd_create = 0x117  // 279
__NR_mmap     = 0xde  // 222
__NR_mprotect = 0xe2  // 226
__NR_munmap   = 0xd7  // 215
__NR_openat   = 0x38  //  56
__NR_read     = 0x3f  //  63
__NR_write    = 0x40  //  64

arg1  .req x0
arg1w .req w0
arg2  .req x1
arg2w .req w1
arg3  .req x2
arg3w .req w2
arg4  .req x3
arg4w .req w3
arg5  .req x4
arg5w .req w4
arg6  .req x5
arg6w .req w5

#define bkpt brk #0
#define call bl

  section ELFMAINX
//  .long offset(b_info)|(asl_delta>>12)  src for f_exp  FIXME: really?
//SO_INFO:
//  .word offset(.)  // detect relocation
//  .word offset(user DT_INIT)
//  .word offset(escape_hatch)  // override with round_up(2, PT_LOAD[0]{.p_memsz + .p_vaddr})
//  .word offset(dst for f_exp)

        .balign 4
_start: .globl _start
        // bkpt // DEBUG
        PUSH4 (x0,x1,x2,lr)  // MATCH_00

        sub sp,sp,#2*NBPW  // space for ADRU, LENU
F_ADRU= 0 * NBPW
F_LENU= 1 * NBPW
F_ARGC= 2 * NBPW

        adr x30,fold_info  // assembler bug: 'lr' not an integer register
    foldi   .req x30
    old_sp  .req x14  // busy: lr,x14
        mov old_sp,sp
        ldr w0,[foldi,#sz_unc]
        str x0,[old_sp,#F_LENU]
        sub x0,sp,x0  // alloca
        and sp,x0,#-2*NBPW  // align stack

psa_fd  .req w5
aux_end .req x4
page_m  .req x7

AT_PAGESZ= 6
O_RDONLY= 0
        mov w2,#O_RDONLY
        adr x1,str_psa
        mov w0,#0
        do_sys __NR_openat; mov psa_fd,w0
        mov x1,sp  // buffer
        mov x2,#512  // len
        do_sys __NR_read; add aux_end,x1,x0  // end
        mov w0,psa_fd; do_sys __NR_close
        .unreq psa_fd
0:
        ldr x0,[x1,#NBPW]  // value
        ldr x2,[x1],#2*NBPW  // tag
        cmp x2,#AT_PAGESZ; beq 1f
        cmp x1,aux_end; blo 0b; mov x0,#1<<12  // default 4KiB
        .unreq aux_end
1:
        sub page_m,xzr,x0  // PAGE_MASK

        add arg4,old_sp,#F_LENU  // &dstlen
        mov arg3,sp  // dst for decompress
        ldr arg2w,[foldi,#sz_cpr]  // srclen
        add arg1,foldi,#sz_b_info  // src
    .unreq foldi  // busy: x14
        bl f_decompress

        mov arg2w,#0
        adr arg1,str_upx
    mfd     .req w15  // busy: x15,x14
        do_sys __NR_memfd_create; mov mfd, w0

        str page_m, [sp,#0]
        mov arg2,sp
        ldr arg3,[old_sp,#F_LENU]
        do_sys __NR_write
        mov sp,old_sp  // de-alloca
    .unreq old_sp  // busy: x15

        mov arg6,#0  // beginning of file
        mov arg5w,mfd
        mov arg4w,#MAP_PRIVATE  // modes
        mov arg3w,#PROT_READ|PROT_EXEC  // prot
        ldr arg2,[sp,#F_LENU]
        mov arg1,#0  // addr (kernel chooses)
        do_sys __NR_mmap; str x0,[sp,#F_ADRU]
    u_ptr   .req x14  // busy: x15,x14
        mov u_ptr,x0

        mov arg1w,mfd
    .unreq mfd  // busy: x14
        do_sys __NR_close

        adr arg1,_start - 4*4  // &SO_INFO
        add arg2,sp,#F_ARGC  // &{argc, argv, envp}
        add u_ptr,u_ptr,2*NBPW
        br u_ptr
        .unreq u_ptr

str_upx:
        .asciz "upx"
str_psa:
        .asciz "/proc/self/auxv"
        .balign 4

//%esp:
//  MATCH_04  ptr unfolded_code
//  MATCH_10  len unfolded_code
//  MATCH_00  argc,argv,envp,lr(_start)

f_decompress:
// nrv2b code is hard-wired here
#define NO_METHOD_CHECK 1

// only one de-compressor; build 'eof' return
#undef DAISY_CHAIN

// use of mmap() forces implicit cache sync
#define NO_SYNC_CACHE 1

off     .req w5
#include "arch/arm64/v8/nrv2b_d32.S"

// IDENTSTR goes here

  section ELFMAINZ
fold_info:
//  b_info (sz_unc, sz_cpr, method) of folded code (C-language, etc.)

/* vim:set ts=8 sw=8 et: */
