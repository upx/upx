//  arm-darwin.macho-fold.S -- linkage to C code to process Mach-O binary
//
//  This file is part of the UPX executable compressor.
//
//  Copyright (C) 2000-2013 John F. Reiser
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
//  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
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

#include "arch/arm/v5a/macros.S"

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
   r11= &sz_pack2; follows compressed program {l_info; p_info; b_info; data...}
    r5= f_decompress
    sp/ junk1,junk2,{15 original regs (omit sp)},junk3,original_stack...
*/
        ldr r1,[r11]  // sz_pack2
        add r7,sp,#4*(2+(16-1))  // &junk3; will become &mhdrp
        sub r0,r11,r1  // &{l_info; p_info; b_info}
        ldr   r3,[r0,#sz_unc + sz_l_info + sz_p_info]  // sz_unc of Mach_header
        cmp   r3,#(1<<13)
        movls r3,#(1<<13)  // at least 8KiB
        sub sp,sp,r3  // alloca
        mov r2,sp  // Mach_header *tmp
        adr r6,f_unfilter
        stmdb sp!,{r5,r6,r7}
        bl upx_main // (r0=l_info *, r1=sz_compressed, r2=Mach_header *tmp,
                // r3=sz_mhdr, f_decompress, f_unfilter, Mach_header **)
        ldr r0,[r0,#15*4]  // entry: ((Mach_ARM_thread_state const *)dyld)->pc
        sub sp, r7,#4*(16-1)  // unalloca; keep 15 original registers and mhdrp
        str r0,[r7,#-4]  // .r15= entry
        ldmia sp!,{r0-r12,r14,r15}  // restore original registers; goto dynamic linker

f_unfilter:  // (char *ptr, uint len, uint cto, uint fid)
        ptr  .req r0
        len  .req r1
        cto  .req r2  // unused
        fid  .req r3

        t1   .req r2
        t2   .req r3

#ifndef FILTER_ID  /*{*/
#define FILTER_ID 0x50  /* little-endian */
#endif  /*}*/
        and fid,fid,#0xff
        cmp fid,#FILTER_ID  // last use of fid
        movne pc,lr  // no-op if not filter 0x50

        movs  len,len,lsr #2  // word count
        cmpne ptr,#0
        moveq pc,lr  // no-op if either len or ptr is 0

top_unf:
        sub len,len,#1
        ldr t1,[ptr,len,lsl #2]
        and t2,t1,#0x0f<<24
        cmp t2,   #0x0b<<24; bne tst_unf  // not 'bl' subroutine call
        and t2,t1,#0xff<<24  // all the non-displacement bits
        sub t1,t1,len  // convert to word-relative displacement
        bic t1,t1,#0xff<<24  // restrict to displacement field
        orr t1,t1,t2  // re-combine
        str t1,[ptr,len,lsl #2]
tst_unf:
        cmp len,#0
        bne top_unf
        mov pc,lr

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
#ifdef SIMULATE_ON_DEBIAN_EABI4  /*{*/
        stmdb sp!,{r4,r5}
        ldr r5,[sp,#3*4]  // off_t
        ldr r4,[sp,#2*4]  // fd
        movs r12,r5,lsl #(32-12); bne mmap_frag  // lo 12 bits of offset
        mov r5,r5,lsr #12  // convert to page number (avoid 64-bit argument)
        do_sys __NR_mmap
mmap_ret:
        ldmia sp!,{r4,r5}
        ret
mmap_frag:
EINVAL=22
        mov r0,#-EINVAL  // offset not a multiple of page size
        b mmap_ret
#else  /*}{ USUAL case */
        mov ip,sp
        stmdb sp!,{r4,r5,r6}
        ldmia ip ,{r4,r5 /*,r6*/}
        mov r6,#0  // XXX: convert 32-bit unsigned off_t to 64-bits
        do_sys __NR_mmap
        ldmia sp!,{r4,r5,r6}
        ret
#endif  /*}*/

        .globl pread
pread:
#ifdef SIMULATE_ON_DEBIAN_EABI4  /*{*/
        stmdb sp!,{r4,r5}  // EABI4 wants 64-bit off_t in even,odd register pair
        mov r4,r3  //            32-bit off_t
        mov r5,#0  // hi bits of 64-bit off_t
        do_sys __NR_pread
        ldmia sp!,{r4,r5}
        ret
#else  /*}{ USUAL case */
        str r4,[sp,#-4]!  // PUSH r4
        mov r4,#0   // convert 32-bit unsigned off_t in r3 to 64 bits in (r3,r4)
        do_sys __NR_pread
        ldr r4,[sp],#4  // POP r4
        ret
#endif  /*}*/

        .globl bswap
bswap:
        mov ip,   #0xff
        orr ip,ip,#0xff<<16   // ip= 0x00ff00ff
        b bswap9
bswap0:
        ldr r2,[r0]           // r2= A B C D
        and r3,ip,r2          // r3= 0 B 0 D
        and r2,ip,r2,ror #24  // r2= 0 C 0 A
        orr r2,r2,r3,ror # 8  // r2= D C B A
        str r2,[r0],#4
bswap9:
        subs r1,r1,#4
        bge bswap0
        ret

// vi:ts=8:et:nowrap

