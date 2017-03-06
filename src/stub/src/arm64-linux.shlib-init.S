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

#include "arch/arm/v8a/macros.S"

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

  section ELFMAINX
//  .long offset(.)  // detect relocation
//  .long offset(user DT_INIT)
//  .long offset(escape_hatch)
//  .long offset({p_info; b_info; compressed data})

_start: .globl _start
        brk #0  // for debugging
        PUSH3(x29,x30,x0)
        PUSH4(arg4,arg5,x6,x7)
        PUSH3(arg1,arg2,arg3)
        mov fp,sp
o_uinit= (3+4+2)*8  // pc

        bl main  // push &f_decompress
f_decompress:
#define LINUX_ARM_CACHEFLUSH 1

  section NRV_HEAD
        // empty
  section NRV_TAIL
        // empty

  section NRV2E
#include "arch/arm/v8a/nrv2e_d8.S"

  section NRV2D
#include "arch/arm/v8a/nrv2d_d8.S"

  section NRV2B
#include "arch/arm/v8a/nrv2b_d8.S"

#include "arch/arm/v8a/lzma_d.S"

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
.macro push reg
        str \reg,[sp,#-4]!
.endm
.macro pop reg
        ldr \reg,[sp],#4
.endm

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
        lodsl; add rax,rcx,rax; str eax,[fp,#o_uinit]  // reloc DT_INIT  for step 12
        lodsl; add rax,rcx,rax; push rax               // reloc &hatch   for step 10
o_hatch= -1*4
        lodsl; add edi,ecx,eax  // &l_info; also destination for decompress
        add esi,edi,#sz_l_info + sz_p_info  // &b_info

        sub sp,sp,#2*4  // param space: munmap temp pages  step 9
p_unmap= -3*4

        ldr eax,[rsi,#4]; add rsi,rsi,#3*4  // sz_cpr
        add rsi,rsi,rax  // skip unpack helper block

        lodslu  // eax=dstlen
        lsl ecx,edi,#  (32-PAGE_SHIFT)
        lsr ecx,ecx,#2+(32-PAGE_SHIFT)  // ecx= w_fragment
        add eax,eax,ecx,lsl #2; push eax  // params: mprotect restored pages  step 8
        sub edi,edi,ecx,lsl #2; push edi
p_mprot= -5*8
        sub eax,eax,ecx,lsl #2  // dstlen
        add edi,edi,ecx,lsl #2  // dst
        push ecx  // w_fragment
o_wfrag= -6*8

        bl L610
f_unfilter:  // (char *ptr, uint len, uint cto, uint fid)
        ptr  .req x0
        fid  .req w3

#ifndef FILTER_ID  /*{*/
#define FILTER_ID 0x50  /* little-endian */
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
        sub w2,w2,w1  // word displ; hi 0b.00101 preserves bit 31
        ubfm w2,w3,#26,#5
        str w2,[ptr,x1,lsl #2]
tst_unf:
        cbnz x1,top_unf
unfret:
        ret

L610:
        push lr
o_unflt= -7*8
        ldrb tmp,[rsi,#b_method-4+1]; push tmpx  // ftid
        ldrb tmp,[rsi,#b_method-4+2]; push tmpx  // cto8
        push rax  // dstlen  also for unfilter  step 7
        push rdi  // dst    param for unfilter  step 7
p_unflt= -11*8

        lodslu; mov ecx,eax  // ecx= srclen
        lodslu; push rax  // method,filter,cto,junk
        push rdx  // &decompress
o_uncpr= -13*8
        add tmpx,fp,#p_unflt+1*4; push tmpx  // &dstlen
        push rdi  // dst
        push rcx  // srclen
        push rsi  // src;  arglist ready for decompress  step 6
p_uncpr= -17*8

        and tmpx,rsi,#3  // length of prefix alignment
        add rcx,rcx,#3  // allow  suffix alignment
        add rcx,rcx,tmpx  // prefix increases byte length
        ldr tmp,[fp,#o_wfrag]; add rdx,tmpx,rcx,lsr #2  // w_srclen + w_frag
        ldr tmp,[fp,#o_uncpr]; bl wlen_subr
        ldr tmp,[fp,#o_unflt]; bl wlen_subr

        bl L220
supervise:
        // Allocate pages for result of decompressing.
        // These replace the compressed source and the following hole.
        mov arg5,#-1  // cater to *BSD for fd of MAP_ANON
        mov arg4,#MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED
        mov arg3,#PROT_READ|PROT_WRITE
        ldr arg2,[fp,#p_mprot+4]  // dstlen
        ldr arg1,[fp,#p_mprot  ]  // dst
        mov x6,arg1  // required result
        do_sys __NR_mmap64; cmp x0,x6; beq 0f; brk #0; 0:

        // Restore fragment of page below dst
        ldr ecx,[fp,#o_wfrag]
        //mov edi,r0  // NOP: edi==r0
        ldr rsi,[fp,#p_unmap]
        bl movsl

        POP4(arg1,arg2,arg3,arg4)
        POP1(rax)
        blr rax  // decompress
        add sp,sp,#8  // toss arg5

        bl L620
//hatch:
        do_sys __NR_munmap
        POP3(arg1,arg2,arg3)
        POP4(rax,rcx,x6,x7)
        POP3(fp,lr,x1)
        br x1

L620:  // Implant escape hatch at end of .text
        ldr eax,[fp,#o_hatch]
        ldp arg1,arg2,[lr]
        stp arg1,arg2,[rax]

//p_unflt
        POP4(arg1,arg2,arg3,arg4)
        POP2(rax,x12)  // x12= w_fragment [toss]
        cbz arg4,0f  // 0==ftid ==> no filter
        blr rax  // unfilter
0:
//p_mprot
        ldr arg1,[sp,#0*4]  // lo(dst)
        ldr arg2,[sp,#1*4]  // len
        mov arg3,#0
        add arg2,arg2,arg1  // hi(dst)
        add arg2,arg2,#2*4  // len(hatch)
        do_sys __ARM_NR_cacheflush

        POP2(arg1,arg2)
        mov arg3,#PROT_READ|PROT_EXEC
        do_sys __NR_mprotect

//p_unmap
#if defined(ARMEL_EABI4)  //{
// first part of  do_sys7t __NR_munmap
.if __NR_munmap <= 0xff
        mov r7,#__NR_munmap
.else
        mov r7,#__NR_munmap>>16
        lsl r7,r7,#16
        add r7,r7,#__NR_munmap - ((__NR_munmap>>16)<<16)
.endif
#endif  //}
        POP3(arg1,arg2,lr)
        br lr  // togo hatch

movsl_subr:
        ldr ecx,[rsi,#-4]  // 'bl <over>' instruction word
        bic ecx,ecx,#0xff<<24  // displacment field
        add ecx,ecx,#1  // displ omits one word
// FALL THROUGH to the part of 'movsl' that trims to a multiple of 8 words.
// 7/8 of the time this is faster; 1/8 of the time it's slower.
9:
        ldr tmp,[rsi],#4; sub ecx,ecx,#1
        str tmp,[rdi],#4
movsl:  // rdi= 4-byte aligned dst; esi= 4-byte aligned src; ecx= word count
        tst ecx,#7; bne 9b  // work ecx down to multiple of 8
        lsr ecx,ecx,#3; cbz ecx,9f
7:
        ldp x2,x3,[rsi],#2*8; subs ecx,ecx,#1
        stp x2,x3,[rdi],#2*8; cbnz ecx,7b
9:
        ret

L220:
        push lr  // &supervise
o_super= -18*8
        mov tmpx,lr; bl wlen_subr  // wlen_supervise
        lsl arg2,rdx,#2  // convert to bytes

        // Allocate pages to hold temporary copy.
        mov arg5,#-1  // cater to *BSD for fd of MAP_ANON
        mov arg4,#MAP_PRIVATE|MAP_ANONYMOUS
        mov arg3,#PROT_READ|PROT_WRITE|PROT_EXEC
        str arg2,[fp,#p_unmap+1*8]  // length to unmap
        mov arg1,#0  // any addr
        do_sys __NR_mmap64; cmn x0,#4096; bcc 0f; brk #0; 0:
        str x0,[fp,#p_unmap+0*8]  // address to unmap

        ldr esi,[fp,#p_mprot]
        //mov edi,r0  // edi= dst  NOP: edi==r0
        ldr ecx,[fp,#o_wfrag]  // w_fragment
        bl movsl  // copy the fragment

        ldr esi,[fp,#p_uncpr+0*4]  // src
        ldr ecx,[fp,#p_uncpr+1*4]  // len
        and tmp,esi,#3  // length of prefix alignment
        sub esi,esi,tmp  // down to word aligned
        add ecx,ecx,tmp  // prefix increases byte length
        add tmp,tmp,edi // skip prefix at destination
        str tmp,[fp,#p_uncpr+0*4]  // dst
        add ecx,ecx,#7  // round up to full words
        lsr ecx,ecx,#3
        bl movsl  // copy all aligned words that contain compressed data

        mov rdx,rdi  // lo(dst) of copied code

        ldr rsi,[fp,#o_uncpr]
        str rdi,[fp,#o_uncpr]
        bl movsl_subr  // copy decompressor

        ldr rsi,[fp,#o_unflt]
        str rdi,[fp,#o_unflt]
        bl movsl_subr  // copy unfilter

        pop rsi   // &supervise
        push rdi  // &copied
        bl movsl_subr  // copy supervisor

        mov arg2,rdi  // hi(dst) of copied code
        mov arg1,rdx  // lo(dst) of copied code
        mov arg3,#0
        do_sys __ARM_NR_cacheflush

        pop lr; br lr  // goto copied supervisor

wlen_subr:  // edx+= nwords of inline subr at *tmp
        ldr tmp,[tmpx,#-4]  // 'bl <over>' instruction word
        bic tmp,tmp,#0xff<<24  // displacment field
        add tmp,tmp,#1  // displ omits one word
        add rdx,rdx,tmpx
        ret

/*__XTHEENDX__*/

/* vim:set ts=8 sw=8 et: */
