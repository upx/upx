/*  arm-linux.shlib-init.S -- Linux Elf shared library init & decompressor
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

#include "arch/arm64/v8/macros.S"

sz_Elf64_Ehdr = 16 + 2*2 + 4 + 3*8 + 4 + 6*2
sz_Elf64_Phdr = 2*4 + 6*8

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
PAGE_MASK=  (~0<<PAGE_SHIFT)
PAGE_SIZE= -PAGE_MASK

__NR_exit =   93
__NR_write =  64
__NR_mmap64   = 0xde  // 222
__NR_munmap   = 0xd7  // 215
__NR_mprotect = 0xe2  // 226

__ARM_NR_cacheflush =  (1<<31)  // FIXME

#define arg1 x0
#define arg2 x1
#define arg3 x2
#define arg4 x3
#define arg5 x4
#define arg6 x5

#define edi w0
#define esi w1
#define edx w2
#define tmp w3
#define eax w4
#define ecx w5

#define rdi x0
#define rsi x1
#define rdx x2

#define rax x4
#define rcx x5

#define lr  x30
#define fp  x29

#define src  x0
#define len  w1
#define dst  x2
#define dstw w2
#define tmp  w3
#define tmpx x3
#define bits w4
#define off  w5

#define tmp1w w3
#define tmp1x x3
#define tmp2w w6
#define tmp2x x6

  section ELFMAINX
//  .long offset(.)  // detect relocation
//  .long offset(user DT_INIT)
//  .long offset(escape_hatch)
//  .long offset({l_info; p_info; b_info; compressed data})

_start: .globl _start
//      brk #0  // debugging
        PUSH2(lr,x0)  // x0= placeholder for user DT_INIT
        PUSH4(arg1,arg2,arg3,fp)
        mov fp,sp
o_uinit= 5*8  // pc

        bl main  // push &f_decompress
f_decompress:
#define LINUX_ARM_CACHEFLUSH 1

  section NRV_HEAD
        // empty
  section NRV_TAIL
        // empty

  section NRV2E
#include "arch/arm64/v8/nrv2e_d32.S"

  section NRV2D
#include "arch/arm64/v8/nrv2d_d32.S"

  section NRV2B
#include "arch/arm64/v8/nrv2b_d32.S"

#include "arch/arm64/v8/lzma_d.S"

  section ELFMAINY
end_decompress: .globl end_decompress

msg_SELinux:
        mov w2,#L71 - L70  // length
        adr x1,L70  // message text
        mov w0,#2  // fd stderr
        do_sys __NR_write
die:
        mov w0,#127
        do_sys __NR_exit
L70:
        .asciz "PROT_EXEC|PROT_WRITE failed.\n"
L71:
        /* IDENTSTR goes here */

  section ELFMAINZ
#define lodsl ldr eax,[rsi],#4
#define lodslu lodsl

main:
//  1. allocate temporary pages
//  2. copy to temporary pages:
//       fragment of page below dst; compressed src;
//       decompress+unfilter; supervise
//  3. mmap destination pages for decompressed data
//  4. create escape hatch
//  5. jump to temporary pages
//  6. uncompress
//  7. unfilter
//  8. mprotect decompressed pages
//  9  setup args for unmap of temp pages
// 10. jump to escape hatch
// 11. unmap temporary pages
// 12. goto user DT_INIT

        mov rdx,lr  // &f_decompress

        add rsi,rdx,# _start - f_decompress - 4*4
               mov rcx,rsi
        lodsl; sub rcx,rcx,rax; //str ecx,[fp,#o_reloc]
        lodsl; add rax,rcx,rax; str rax,[fp,#o_uinit]  // reloc DT_INIT  for step 12
        lodsl; add rax,rcx,rax; PUSH1(rax)             // reloc &hatch   for step 10
o_hatch= -2*8  // HOLE
        lodsl; add rdi,rcx,rax  // &l_info; also destination for decompress
        add rsi,rdi,#sz_l_info + sz_p_info  // &b_info

        sub sp,sp,#2*8  // param space: munmap temp pages  step 9
p_unmap= -4*8

        ldr eax,[rsi,#4]; add rsi,rsi,#3*4  // sz_cpr
        add rsi,rsi,rax  // skip unpack helper block

        lodslu  // eax=dstlen
        lsl ecx,edi,#  (32-PAGE_SHIFT)
        lsr ecx,ecx,#2+(32-PAGE_SHIFT)  // ecx= w_fragment
        add eax,eax,ecx,lsl #2
        sub rdi,rdi,rcx,lsl #2
        PUSH2(rdi,rax)  // params: mprotect restored pages  step 8
p_mprot= -6*8
        sub eax,eax,ecx,lsl #2  // dstlen
        add rdi,rdi,rcx,lsl #2  // dst

        bl L610
f_unfilter:  // (char *ptr, uint len, uint cto, uint fid)
        ptr  .req x0
        fid  .req w3

#ifndef FILTER_ID  /*{*/
#define FILTER_ID 0x52  /* little-endian */
#endif  /*}*/
        and fid,fid,#0xff
        cmp fid,#FILTER_ID  // last use of fid
        bne unfret
        lsr x1,x1,#2  // word count
        cbz x1,unfret
top_unf:
        sub x1,x1,#1
        ldr w2,[ptr,x1,lsl #2]
        ubfx w3,w2,#26,#5
        cmp w3,#5; bne tst_unf  // not unconditional branch
        sub w3,w2,w1  // word displ
        bfi w2,w3,#0,#26  // change displacement
        str w2,[ptr,x1,lsl #2]
tst_unf:
        cbnz x1,top_unf
unfret:
        ret

L610:
        PUSH2(lr,rcx)  // f_unf, w_frag
o_wfrag = -7*8
o_unflt= -8*8
        ldrb tmp1w,[rsi,# b_method-4+1]  // ftid
        ldrb tmp2w,[rsi,# b_method-4+2]  // cto8
        PUSH4(rdi,rax,tmp2x,tmp1x)  // dst, dstlen, cto8, ftid  for unfilter  step 7
p_unflt= -12*8

        lodslu; mov ecx,eax  // ecx= srclen
        lodslu
        PUSH2(rdx,rax)  // &decompress, {method,filter,cto,junk}
o_uncpr= -14*8
        add tmpx,fp,#p_unflt+1*8
        PUSH4(rsi,rcx,rdi,tmpx)  // src,srclen,dst,&dstlen  arglist ready for decompress step 6
p_uncpr= -18*8

        add rcx,rcx,#3  // allow  suffix alignment
        ldr tmp,[fp,#o_wfrag]; add rdx,tmpx,rcx,lsr #2  // w_srclen + w_frag
        ldr tmpx,[fp,#o_uncpr]; bl wlen_subr
        ldr tmpx,[fp,#o_unflt]; bl wlen_subr

        bl L220
supervise:
        // Allocate pages for result of decompressing.
        // These replace the compressed source and the following hole.
        mov arg6,#0
        mov arg5,#-1  // cater to *BSD for fd of MAP_ANON
        mov arg4,#MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED
        mov arg3,#PROT_READ|PROT_WRITE
        ldr arg2,[fp,#p_mprot+8]  // dstlen
        ldr arg1,[fp,#p_mprot  ]  // dst
        mov x6,arg1  // required result
        do_sys __NR_mmap64; cmp x0,x6; beq 0f; brk #0; 0:

        // Restore fragment of page below dst
        ldr ecx,[fp,#o_wfrag]
        //mov edi,r0  // NOP: edi==r0
        ldr rsi,[fp,#p_unmap]
        bl movsl

//p_uncpr
        POP4(arg1,arg2,arg3,arg4)
        POP1(rax)
        blr rax  // decompress

        bl L620
//hatch:
        do_sys __NR_munmap  // 2 instr
        POP4(arg1,arg2,arg3,fp)  // 2 instr
        POP2(lr,arg4)  // 1 instr; arg4= user DT_INIT
        br arg4

L620:  // Implant escape hatch at end of .text
        ldr rax,[fp,#o_hatch]
        ldp arg1,arg2,[lr]  // 4 instr
        stp arg1,arg2,[rax]
        ldr arg1,[lr,#2*8]  // 2 instr
        str arg1,[rax,#2*8]

//p_unflt
        POP4(arg1,arg2,arg3,arg4)
        POP1(rax)  // f_unf
        cbz arg4,0f  // 0==ftid ==> no filter
        blr rax  // unfilter
0:
//p_mprot
        POP2(arg1,arg2)
        mov arg3,#PROT_READ|PROT_EXEC
        do_sys __NR_mprotect

//p_unmap
        POP3(arg1,arg2,lr)
        br lr  // goto hatch

movsl_subr:
        ldr ecx,[rsi,#-4]  // 'bl <over>' instruction word
        bic ecx,ecx,#0xff<<24  // displacment field
movsl:  // rdi= 4-byte aligned dst; rsi= 4-byte aligned src; ecx= word count
        tst ecx,#1; lsr ecx,ecx,#1; beq 5f
        ldr tmp,[rsi],#4
        str tmp,[rdi],#4
5:
        cbz ecx,9f
7:
        ldp w2,w3,[rsi],#2*4; sub  ecx,ecx,#1
        stp w2,w3,[rdi],#2*4; cbnz ecx,7b
9:
        ret

L220:
        PUSH1(lr)  // &supervise
o_super= -20*8  // HOLE
        mov tmpx,lr; bl wlen_subr  // wlen_supervise
        lsl arg2,rdx,#2  // convert to bytes

        // Allocate pages to hold temporary copy.
        mov arg6,#0
        mov arg5,#-1  // cater to *BSD for fd of MAP_ANON
        mov arg4,#MAP_PRIVATE|MAP_ANONYMOUS
        mov arg3,#PROT_READ|PROT_WRITE|PROT_EXEC
        str arg2,[fp,#p_unmap+1*8]  // length to unmap
        mov arg1,#0  // any addr
        do_sys __NR_mmap64; cmn x0,#4096; bcc 0f; brk #0; 0:
        str x0,[fp,#p_unmap+0*8]  // address to unmap

        ldr rsi,[fp,#p_mprot]
        //mov edi,r0  // edi= dst  NOP: edi==r0
        ldr ecx,[fp,#o_wfrag]  // w_fragment
        bl movsl  // copy the fragment

        ldr rsi,[fp,#p_uncpr]  // src
        ldr ecx,[fp,#p_uncpr+1*8]  // len
        str rdi,[fp,#p_uncpr]
        add ecx,ecx,#3; lsr ecx,ecx,#2
        bl movsl  // copy compressed data

        mov rdx,rdi  // lo(dst) of copied code

        ldr rsi,[fp,#o_uncpr]
        str rdi,[fp,#o_uncpr]
        bl movsl_subr  // copy decompressor

        ldr rsi,[fp,#o_unflt]
        str rdi,[fp,#o_unflt]
        bl movsl_subr  // copy unfilter

        POP1(rsi)   // &supervise
        PUSH1(rdi)  // &copied
        bl movsl_subr  // copy supervisor

        ldp arg1,arg2,[fp,#p_unmap+0*8]
        mov arg3,#PROT_READ|PROT_EXEC  // some OS prohibit PROT_WRITE && PROT_EXEC
        do_sys __NR_mprotect; cmn x0,#4096; bcc 0f; brk #0; 0:

        POP1(lr); br lr  // goto copied supervisor

wlen_subr:  // rdx+= nwords of inline subr at *tmp
        ldr tmp,[tmpx,#-4]  // 'bl <over>' instruction word
        bic tmp,tmp,#0xff<<24  // displacment field
        add rdx,rdx,tmpx
        ret

/*__XTHEENDX__*/

/* vim:set ts=8 sw=8 et: */
