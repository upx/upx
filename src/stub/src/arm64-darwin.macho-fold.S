//  arm64-darwin.macho-fold.S -- linkage to C code to process Mach-O binary
//
//  This file is part of the UPX executable compressor.
//
//  Copyright (C) 2000-2017 John F. Reiser
//  All Rights Reserved.
//
//  UPX and the UCL library are free software; you can redistribute them
//  and/or modify them under the terms of the GNU General Public License as
//  published by the Free Software Foundation; either version 2 of
//  the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; see the file COPYING.
//  If not, write to the Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
//  Markus F.X.J. Oberhumer              Laszlo Molnar
//  <markus@oberhumer.com>               <ezerotven+github@gmail.com>
//
//  John F. Reiser
//  <jreiser@users.sourceforge.net>
//

//#define SIMULATE_ON_DEBIAN_EABI4 1
#undef  SIMULATE_ON_DEBIAN_EABI4

#ifdef SIMULATE_ON_DEBIAN_EABI4  /*{*/
  #define LINUX_ARM_CACHEFLUSH 1  /* SIMULATE_ON_DEBIAN_EABI4 */
  #define ARMEL_EABI4 1           /* SIMULATE_ON_DEBIAN_EABI4 */
#else  /*}{ USUAL case */
  #define DARWIN_ARM_CACHEFLUSH 1
  #define ARMEL_DARWIN 1
#endif  /*}*/

#include "arch/arm64/v8/macros.S"

sz_l_info = 12
sz_p_info = 12
sz_b_info = 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

_start: .globl _start  // ignored, but silence "cannot find entry symbol _start" from ld

// control just falls through, after this part and compiled C code
// are uncompressed.

fold_begin:
/* In:
   x11= &sz_pack2; follows compressed program {l_info; p_info; b_info; data...}
    x4= f_decompress
    sp/ junk1,junk2,{30 original regs (omit sp,lr)},junk3,original_stack...
*/
#define t0w w9  /* scratch value */
lr .req x30
        ldr x1,[x11]  // sz_pack2
        add x7,sp,#8*(2+(32-2))  // &junk3; will become &mhdrp
        sub x0,x11,x1,uxtw  // &{l_info; p_info; b_info}
        ldr w3,[x0,#sz_unc + sz_l_info + sz_p_info]  // sz_unc of Mach_header
        mov t0w,#(1<<13)
        cmp w3,t0w
        csel w3,w3,t0w,hi  // at least 8KiB
        sub sp,sp,w3,uxtw  // alloca
        mov x2,sp  // Mach_header *tmp
        adr x5,f_unfilter
        bl upx_main // (x0=l_info *, w1=sz_compressed, x2=Mach_header *tmp,
                // w3=sz_mhdr, x4=f_decompress, x5=f_unfilter, x6= &Mach_header *)
        ldr lr,[x0,#30*8]  // entry: ((Mach_ARM_thread_state const *)dyld)->pc
        ldp x28,x29,[sp,#8*28]
        ldp x26,x27,[sp,#8*26]
        ldp x24,x25,[sp,#8*24]
        ldp x22,x23,[sp,#8*22]
        ldp x20,x21,[sp,#8*20]
        ldp x18,x19,[sp,#8*18]
        ldp x16,x17,[sp,#8*16]
        ldp x14,x15,[sp,#8*14]
        ldp x12,x13,[sp,#8*12]
        ldp x10,x11,[sp,#8*10]
        ldp x8,x9,[sp,#8*8]
        ldp x6,x7,[sp,#8*6]
        ldp x4,x5,[sp,#8*4]
        ldp x2,x3,[sp,#8*2]
        ldp x0,x1,[sp],#8*30
        br lr

f_unfilter:  // (char *ptr, uint len, uint cto, uint fid)
        ptr  .req x0
        len  .req w1
        cto  .req w2  // unused
        fid  .req w3

        t1   .req w2
        t2   .req w3

#ifndef FILTER_ID  /*{*/
#define FILTER_ID 0x50  /* little-endian */
#endif  /*}*/
        and fid,fid,#0xff
        cmp fid,#FILTER_ID  // last use of fid
        bne unf_ret  // no-op if not filter 0x50

        lsr len,len,#2  // word count
        cbz ptr,unf_ret  // no-op if ptr is NULL
        cbz len,unf_ret  // no-op if len is 0

top_unf:
        sub len,len,#1
        ldr t1,[ptr,len,uxtw #2]
        lsr t2,t1,#26
        cmp t2,   #045; bne tst_unf  // not 'bl' subroutine call
        and t2,t1,#077<<26  // all the non-displacement bits
        sub t1,t1,len  // convert to word-relative displacement
        bic t1,t1,#077<<26  // restrict to displacement field
        orr t1,t1,t2  // re-combine
        str t1,[ptr,len,uxtw #2]
tst_unf:
        cmp len,#0
        bne top_unf
unf_ret:
        ret

        .unreq ptr
        .unreq len
        .unreq cto
        .unreq fid

spin: .globl spin
        ret

__NR_exit  =  1 + __NR_SYSCALL_BASE
__NR_read  =  3 + __NR_SYSCALL_BASE
__NR_write =  4 + __NR_SYSCALL_BASE
__NR_open  =  5 + __NR_SYSCALL_BASE
__NR_close =  6 + __NR_SYSCALL_BASE
__NR_brk   = 45 + __NR_SYSCALL_BASE

__NR_mmap     = 197 + __NR_SYSCALL_BASE
__NR_munmap   =  73 + __NR_SYSCALL_BASE
__NR_mprotect =  74 + __NR_SYSCALL_BASE
__NR_pread    = 153 + __NR_SYSCALL_BASE

#ifdef SIMULATE_ON_DEBIAN_EABI4  /*{*/
__NR_mmap     = 192 + __NR_SYSCALL_BASE  // mmap2
__NR_munmap   =  91 + __NR_SYSCALL_BASE
__NR_mprotect = 125 + __NR_SYSCALL_BASE
__NR_pread    = 180 + __NR_SYSCALL_BASE
#endif  /*}*/

        .globl exit
exit:
        do_sys __NR_exit

        .globl read
read:
        do_sys __NR_read; ret

        .globl write
write:
        do_sys __NR_write; ret

        .globl open
open:
        do_sys __NR_open; ret

        .globl close
close:
        do_sys __NR_close; ret

        .globl brk
brk:
        do_sys __NR_brk; ret

        .globl munmap
munmap:
        do_sys __NR_munmap; ret

        .globl mprotect
mprotect:
        do_sys __NR_mprotect; ret

        .globl mmap
mmap:
        do_sys __NR_mmap; ret

        .globl pread
pread:
        do_sys __NR_pread; ret

        .globl bswap
bswap:
        mov w9,   #0xff
        orr w9,w9,#0xff<<16   // w9= 0x00ff00ff
        b bswap9
bswap0:
        ldr w2,[x0]           // r2= A B C D
        and w3,w9,w2          // r3= 0 B 0 D
        and w2,w9,w2,ror #24  // r2= 0 C 0 A
        orr w2,w2,w3,ror # 8  // r2= D C B A
        str w2,[x0],#4
bswap9:
        subs w1,w1,#4
        bge bswap0
        ret

/* vim:set ts=8 sw=8 et: */
