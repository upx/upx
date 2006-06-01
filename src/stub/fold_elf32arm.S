@  fold_elfarm.asm -- linkage to C code to process Elf binary
@
@  This file is part of the UPX executable compressor.
@
@  Copyright (C) 2000-2006 John F. Reiser
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
@  <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>
@
@  John F. Reiser
@  <jreiser@users.sourceforge.net>
@

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
    r8= JMPU
    r7= LENU
    r6= ADRU
    r5= f_decompress
    sp/ sz_unc_fold,argc,argv...
*/
        ldr r3,[sp,#4]  @ argc; sz_unc was pushed
        add sp,sp,#4  @ discard sz_unc
        add r3,sp,r3,lsl #2  @ skip argv
        add r3,r3,#4+4  @ skip argc, env_end
.Lenv:
        ldr r0,[r3],#4  @ skip env
        cmp r0,#0
        bne .Lenv
                /* r3= &Elf32_auxv_t */

        stmdb sp!,{r6,r7,r8}  @ ADRU,LENU,JMPU
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
        add sp,sp,#(9-4)*4
        add sp,sp,#MAX_ELF_HDR + OVERHEAD  @ un-alloca
        mov r2,r0  @ entry address
        ldmia sp!,{r0,r1,pc}  @ goto [sys $__NR_munmap; mov pc,r2]

f_unfilter:  @ (char *ptr, uint len, uint cto, uint fid)
        ptr  .req r0
        len  .req r1
        cto  .req r2  @ unused
        fid  .req r3

        tmp  .req fid  @ disjoint lifetimes

        and fid,fid,#0xff
        cmp fid,#0x50  @ last use of fid
        movne pc,lr  @ no-op if not filter 0x50

        movs  len,len,lsr #2  @ word count
        cmpne ptr,#0
        moveq pc,lr  @ no-op if either len or ptr is 0

top_unf:
        subs len,len,#1  @ condition code holds until tst_unf
        ldr r3,[ptr,len,lsl #2]
        and tmp,r3,#0x0f<<24
        cmp tmp,   #0x0b<<24
        bne tst_unf
        and tmp,r3,#0xff<<24
        sub r3,r3,len  @ convert to word-relative
        bic r3,r3, #0xff<<24
        orr r3,r3,tmp
        str r3,[ptr,len,lsl #2]
tst_unf:
        bne top_unf
        mov pc,lr

        .unreq ptr
        .unreq len
        .unreq cto
        .unreq fid

        .globl exit
exit:
        swi 0x00900001

        .globl read
read:
        swi 0x00900003; mov pc,lr

        .globl open
open:
        swi 0x00900005; mov pc,lr

        .globl close
close:
        swi 0x00900006; mov pc,lr

        .globl brk
brk:
        swi 0x0090002d; mov pc,lr

        .globl munmap
munmap:
        swi 0x0090005b; mov pc,lr

        .globl mprotect
mprotect:
        swi 0x0090007d; mov pc,lr

        .globl mmap
mmap:
        str r5,[sp,#-4]!; ldr r5,[sp,#4+4]
        str r4,[sp,#-4]!; ldr r4,[sp,#4+4]
        mov r5,r5,lsr #12  @ convert to page number
        swi 0x009000c0  @ mmap64
        ldr r4,[sp],#4
        ldr r5,[sp],#4
        mov pc,lr

@ vi:ts=8:et:nowrap

