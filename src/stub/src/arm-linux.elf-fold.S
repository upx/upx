@  arm-linux.elf-fold.S -- linkage to C code to process Elf binary
@
@  This file is part of the UPX executable compressor.
@
@  Copyright (C) 2000-2010 John F. Reiser
@  All Rights Reserved.
@
@  UPX and the UCL library are free software; you can redistribute them
@  and/or modify them under the terms of the GNU General Public License as
@  published by the Free Software Foundation; either version 2 of
@  the License, or (at your option) any later version.
@
@  This program is distributed in the hope that it will be useful,
@  but WITHOUT ANY WARRANTY; without even the implied warranty of
@  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@  GNU General Public License for more details.
@
@  You should have received a copy of the GNU General Public License
@  along with this program; see the file COPYING.
@  If not, write to the Free Software Foundation, Inc.,
@  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
@
@  Markus F.X.J. Oberhumer              Laszlo Molnar
@  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
@
@  John F. Reiser
@  <jreiser@users.sourceforge.net>
@

#define ARM_OLDABI 1
#include "arch/arm/v4a/macros.S"

sz_Elf32_Ehdr = 13*4
sz_Elf32_Phdr =  8*4
sz_l_info = 12
sz_p_info = 12
sz_b_info = 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

@@ control just falls through, after this part and compiled C code
@@ are uncompressed.

#define OVERHEAD 2048
#define MAX_ELF_HDR 512

fold_begin:
/* In:
   r10= &b_info of first compressed block (after move)
    r9= total size of compressed data
    r5= f_decompress
    sp/ ADRU,LENU,sz_unc,crumb,argc,argv...
*/
        ldr r3,[sp,#4*4]  @ argc
        add r3,sp,r3,lsl #2  @ skip argv
        add r3,r3,#4*4 +4 +4  @ skip ADRU,LENU,sz_unc,crumb, argc, env_end
.Lenv:
        ldr r0,[r3],#4  @ skip env
        cmp r0,#0
        bne .Lenv
                /* r3= &Elf32_auxv_t */
        mov r2,r3
.Laux:
        ldr r0,[r2],#2*4  @ .a_type
        cmp r0,#0
        bne .Laux
        str r2,[sp,#2*4]  @ clobber sz_unc with 1+ &Elf32_auxv_t[AT_NULL@.a_type]

        sub sp,sp,#MAX_ELF_HDR + OVERHEAD  @ alloca
        mov r4,r9  @ total size of compressed data
        ldr r9,[r10,#sz_cpr]  @ xi.size  of ELF headers
        mov r8,sp  @ xo.ptr
        ldr r7,[r10,#sz_unc]  @ xo.size
        adr r6,f_unfilter
        add r9,r9,#sz_b_info  @ for unpackExtent
        mov r11,#0  @ dynbase for ET_EXEC;  FIXME for ET_DYN
        stmdb sp!,{r3,r4,r5,r6,r7,r8,r9,r10,r11}
        ldmia sp!,{r0,r1,r2,r3}
        bl upx_main
        mov lr,r0  @ entry address
        add sp,sp,#(9-4)*4
        add sp,sp,#MAX_ELF_HDR + OVERHEAD  @ un-alloca
        ldmia sp!,{r0,r1,r2,r3}  @ ADRU,LENU,1+ &Elf32_auxv_t[AT_NULL@.a_type],crumb
        add r0,r0,r3  @ ADRU+=crumb
        sub r1,r1,r3  @ LENU-=crumb
        ldr pc,[r2,#4 -2*4]  @ Elf32_auxv_t[AT_NULL@.a_type].a_val

f_unfilter:  @ (char *ptr, uint len, uint cto, uint fid)
        ptr  .req r0
        len  .req r1
        cto  .req r2  @ unused
        fid  .req r3

        t1   .req r2
        t2   .req r3

#ifndef FILTER_ID  /*{*/
#define FILTER_ID 0x50  /* little-endian */
#endif  /*}*/
        and fid,fid,#0xff
        cmp fid,#FILTER_ID  @ last use of fid
        movne pc,lr  @ no-op if not filter 0x50

        movs  len,len,lsr #2  @ word count
        cmpne ptr,#0
        moveq pc,lr  @ no-op if either len or ptr is 0

top_unf:
        sub len,len,#1
        ldr t1,[ptr,len,lsl #2]
        and t2,t1,#0x0f<<24
        cmp t2,   #0x0b<<24; bne tst_unf  @ not 'bl' subroutine call
        and t2,t1,#0xff<<24  @ all the non-displacement bits
        sub t1,t1,len  @ convert to word-relative displacement
        bic t1,t1,#0xff<<24  @ restrict to displacement field
        orr t1,t1,t2  @ re-combine
        str t1,[ptr,len,lsl #2]
tst_unf:
        cmp len,#0
        bne top_unf
        mov pc,lr

        .unreq ptr
        .unreq len
        .unreq cto
        .unreq fid

__NR_exit  =  1 + __NR_SYSCALL_BASE
__NR_read  =  3 + __NR_SYSCALL_BASE
__NR_write =  4 + __NR_SYSCALL_BASE
__NR_open  =  5 + __NR_SYSCALL_BASE
__NR_close =  6 + __NR_SYSCALL_BASE
__NR_brk   = 45 + __NR_SYSCALL_BASE

__NR_mmap2    = 192 + __NR_SYSCALL_BASE
__NR_mprotect = 125 + __NR_SYSCALL_BASE
__NR_munmap   =  91 + __NR_SYSCALL_BASE

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
        str r5,[sp,#-4]!; ldr r5,[sp,#4+4]
        str r4,[sp,#-4]!; ldr r4,[sp,#4+4]
        mov r5,r5,lsr #12  @ convert to page number
        do_sys __NR_mmap2
        ldr r4,[sp],#4
        ldr r5,[sp],#4
        ret

#if 0  /*{ "gcc-3.4 -fPIC" botches string constants when code moves! */
        .globl STR_0x
STR_0x:
        mov r0,pc; mov pc,lr
        .asciz "0x"
        .balign 4

        .globl STR_hex
STR_hex:
        mov r0,pc; mov pc,lr
        .asciz "0123456789abcdef"
        .balign 4

        .globl STR_xread
STR_xread:
        mov r0,pc; mov pc,lr
        .asciz "xread %p(%x %p) %p %x\n"
        .balign 4

        .globl STR_unpackExtent
STR_unpackExtent:
        mov r0,pc; mov pc,lr
        .asciz "unpackExtent in=%p(%x %p)  out=%p(%x %p)  %p %p\n"
        .balign 4

        .globl STR_make_hatch_arm
STR_make_hatch_arm:
        mov r0,pc; mov pc,lr
        .asciz "make_hatch_arm %p %x\n"
        .balign 4

        .globl STR_auxv_up
STR_auxv_up:
        mov r0,pc; mov pc,lr
        .asciz "auxv_up  %p %x %x\n"
        .balign 4

        .globl STR_xfind_pages
STR_xfind_pages:
        mov r0,pc; mov pc,lr
        .asciz "xfind_pages  %x  %p  %d  %p\n"
        .balign 4

        .globl STR_do_xmap
STR_do_xmap:
        mov r0,pc; mov pc,lr
        .asciz "do_xmap  fdi=%x  ehdr=%p  xi=%p(%x %p)  av=%p  p_reloc=%p  f_unf=%p\n"
        .balign 4

        .globl STR_upx_main
STR_upx_main:
        mov r0,pc; mov pc,lr
        .asciz "upx_main av=%p  szc=%x  f_dec=%p  f_unf=%p  xo=%p(%x %p)  xi=%p(%x %p)  dynbase=%x\n",
        .balign 4
#endif  /*}*/

@ vi:ts=8:et:nowrap

