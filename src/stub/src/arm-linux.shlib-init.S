/*  arm-linux.shlib-init.S -- Linux Elf shared library init & decompressor
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2013 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2013 Laszlo Molnar
*  Copyright (C) 2000-2013 John F. Reiser
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
*  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
*
*  John F. Reiser
*  <jreiser@users.sourceforge.net>
*/

#define ARM_OLDABI 1
#include "arch/arm/v5a/macros.S"

#define bkpt .long 0xe7f001f0  /* reserved instr; Linux GNU eabi breakpoint */
#define bkpt_th .short 0xde01  /* reserved instr; Linux GNU eabi breakpoint */
sz_Elf32_Ehdr = 13*4
sz_Elf32_Phdr =  8*4

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

__NR_exit =      1 + __NR_SYSCALL_BASE
__NR_write =     4 + __NR_SYSCALL_BASE
__NR_mmap64 = 0xc0 + __NR_SYSCALL_BASE
__NR_mprotect =125 + __NR_SYSCALL_BASE
__NR_munmap =   91 + __NR_SYSCALL_BASE

__ARM_NR_BASE  = 0xf0000 + __NR_SYSCALL_BASE
__ARM_NR_cacheflush =  2 + __ARM_NR_BASE

#define arg1 r0
#define arg2 r1
#define arg3 r2
#define arg4 r3
#define arg5 r4

#define edi r0
#define esi r1
#define edx r2
#define tmp r3
#define eax r4
#define ecx r5

  section ELFMAINX
//  .long offset(.)  // detect relocation
//  .long offset(user DT_INIT)
//  .long offset(escape_hatch)
//  .long offset({p_info; b_info; compressed data})

_start: .globl _start
        nop  //; bkpt  // for debugging
        stmdb sp!,{arg1,arg2,arg3, eax,ecx,r6,r7, fp,lr,pc}
        mov fp,sp
o_uinit= (3+4+2)*4  // pc

        bl main  // push &f_decompress
f_decompress:
#define LINUX_ARM_CACHEFLUSH 1

  section NRV_HEAD
        // empty
  section NRV_TAIL
        // empty

  section NRV2E
#include "arch/arm/v4a/nrv2e_d8.S"

  section NRV2D
#include "arch/arm/v4a/nrv2d_d8.S"

  section NRV2B
#include "arch/arm/v4a/nrv2b_d8.S"

#include "arch/arm/v4a/lzma_d.S"

  section ELFMAINY
end_decompress: .globl end_decompress

msg_SELinux:
        mov r2,#L71 - L70  // length
        adr r1,L70  // message text
        mov r0,#2  // fd stderr
#if defined(ARMEL_EABI4)  /*{*/
        mov r7,#__NR_write
        swi 0
#else  /*}{*/
        swi __NR_write
#endif  /*}*/
die:
        mov r0,#127
#if defined(ARMEL_EABI4)  /*{*/
        mov r7,#__NR_exit
        swi 0
#else  /*}{*/
        swi __NR_exit
#endif  /*}*/
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
.macro lsl dst,src,cnt
        mov \dst,\src,lsl \cnt
.endm
.macro lsls dst,src,cnt
        movs \dst,\src,lsl \cnt
.endm
.macro lsr dst,src,cnt
        mov \dst,\src,lsr \cnt
.endm
.macro lsrs dst,src,cnt
        movs \dst,\src,lsr \cnt
.endm

#define lodsl ldr eax,[esi],#4
#define lodslu bl get4u

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

        mov edx,lr  // &f_decompress

        add esi,edx,# _start - f_decompress - 4*4
               mov ecx,esi
        lodsl; sub ecx,ecx,eax; //str ecx,[fp,#o_reloc]
        lodsl; add eax,ecx,eax; str eax,[fp,#o_uinit]  // reloc DT_INIT  for step 12
        lodsl; add eax,ecx,eax; push eax               // reloc &hatch   for step 10
o_hatch= -1*4
        lodsl; add edi,ecx,eax  // &l_info; also destination for decompress
        add esi,edi,#sz_l_info + sz_p_info  // &b_info

        sub sp,sp,#2*4  // param space: munmap temp pages  step 9
p_unmap= -3*4

        ldr eax,[esi,#4]; add esi,esi,#3*4  // sz_cpr
        add esi,esi,eax  // skip unpack helper block

        lodslu  // eax=dstlen
        lsl ecx,edi,#  (32-PAGE_SHIFT)
        lsr ecx,ecx,#2+(32-PAGE_SHIFT)  // ecx= w_fragment
        add eax,eax,ecx,lsl #2; push eax  // params: mprotect restored pages  step 8
        sub edi,edi,ecx,lsl #2; push edi
p_mprot= -5*4
        sub eax,eax,ecx,lsl #2  // dstlen
        add edi,edi,ecx,lsl #2  // dst
        push ecx  // w_fragment
o_wfrag= -6*4

        bl L610
f_unfilter:  // (char *ptr, uint len, uint cto, uint fid)
#define ptr r0
#define len r1
#define cto r2
#define fid r3

#define t1 r2
#define t2 r3

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
        ret

#undef ptr
#undef len
#undef cto
#undef fid

#undef t1
#undef t2

L610:
        push lr
o_unflt= -7*4
        ldrb tmp,[esi,#b_method-4+1]; push tmp  // ftid
        ldrb tmp,[esi,#b_method-4+2]; push tmp  // cto8
        push eax  // dstlen  also for unfilter  step 7
        push edi  // dst    param for unfilter  step 7
p_unflt= -11*4

        lodslu; mov ecx,eax  // ecx= srclen
        lodslu; push eax  // method,filter,cto,junk
        push edx  // &decompress
o_uncpr= -13*4
        add tmp,fp,#p_unflt+1*4; push tmp  // &dstlen
        push edi  // dst
        push ecx  // srclen
        push esi  // src;  arglist ready for decompress  step 6
p_uncpr= -17*4

        and tmp,esi,#3  // length of prefix alignment
        add ecx,ecx,#3  // allow  suffix alignment
        add ecx,ecx,tmp  // prefix increases byte length
        ldr tmp,[fp,#o_wfrag]; add edx,tmp,ecx,lsr #2  // w_srclen + w_frag
        ldr tmp,[fp,#o_uncpr]; bl wlen_subr
        ldr tmp,[fp,#o_unflt]; bl wlen_subr

        bl L220
supervise:
        // Allocate pages for result of decompressing.
        // These replace the compressed source and the following hole.
        mvn arg5,#0  // -1; cater to *BSD for fd of MAP_ANON
        mov arg4,#MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED
        mov arg3,#PROT_READ|PROT_WRITE
        ldr arg2,[fp,#p_mprot+4]  // dstlen
        ldr arg1,[fp,#p_mprot  ]  // dst
        mov r6,arg1  // required result
        do_sys7t __NR_mmap64; cmp r0,r6; beq 0f; bkpt; 0:

        // Restore fragment of page below dst
        ldr ecx,[fp,#o_wfrag]
        //mov edi,r0  // NOP: edi==r0
        ldr esi,[fp,#p_unmap]
        bl movsl

        ldmia sp!,{arg1,arg2,arg3,arg4, eax}
        blx eax  // decompress
        add sp,sp,#4  // toss arg5

        bl L620
//hatch:
#if defined(ARMEL_EABI4)  //{
        swi 0  // last part of  do_sys7t __NR_munmap
#elif defined(ARM_OLDABI)  //}{
        do_sys __NR_munmap
#else  //}{
        error ARMEL_EABI4, ARM_OLDABI
#endif  //}
        ldmia sp!,{arg1,arg2,arg3, eax,ecx,r6,r7, fp,lr,pc}

L620:  // Implant escape hatch at end of .text
        ldr eax,[fp,#o_hatch]
        ldmia  lr,{arg1,arg2}
        stmia eax,{arg1,arg2}

//p_unflt
        ldmia sp!,{arg1,arg2,arg3,arg4, eax, r12}  // r12= w_fragment [toss]
        tst arg4,arg4; beq 0f  // 0==ftid ==> no filter
        blx eax  // unfilter
0:
//p_mprot
        ldr arg1,[sp,#0*4]  // lo(dst)
        ldr arg2,[sp,#1*4]  // len
        mov arg3,#0
        add arg2,arg2,arg1  // hi(dst)
        add arg2,arg2,#2*4  // len(hatch)
        do_sys7t2 __ARM_NR_cacheflush

        ldmia sp!,{arg1,arg2}
        mov arg3,#PROT_READ|PROT_EXEC
        do_sys7t __NR_mprotect

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
        ldmia sp!,{arg1,arg2, pc}  // goto hatch

movsl_subr:
        ldr ecx,[esi,#-4]  // 'bl <over>' instruction word
        bic ecx,ecx,#0xff<<24  // displacment field
        add ecx,ecx,#1  // displ omits one word
// FALL THROUGH to the part of 'movsl' that trims to a multiple of 8 words.
// 7/8 of the time this is faster; 1/8 of the time it's slower.
9:
        ldr tmp,[esi],#4; sub ecx,ecx,#1
        str tmp,[edi],#4
movsl:  // edi= 4-byte aligned dst; esi= 4-byte aligned src; ecx= word count
        tst ecx,#7; bne 9b  // work ecx down to multiple of 8
        lsrs ecx,ecx,#3; beq 9f
        stmdb sp!, {r2,   r4,r6, r7,r8,r9}  // tmp===r3, ecx===r5
7:
        ldmia esi!,{r2,r3,r4,r6, r7,r8,r9,r12}; subs ecx,ecx,#1
        stmia edi!,{r2,r3,r4,r6, r7,r8,r9,r12}; bne 7b
        ldmia sp!, {r2,   r4,r6, r7,r8,r9}
9:
        ret

L220:
        push lr  // &supervise
o_super= -18*4
        mov tmp,lr; bl wlen_subr  // wlen_supervise
        lsl arg2,edx,#2  // convert to bytes

        // Allocate pages to hold temporary copy.
        mvn arg5,#0  // -1; cater to *BSD for fd of MAP_ANON
        mov arg4,#MAP_PRIVATE|MAP_ANONYMOUS
        mov arg3,#PROT_READ|PROT_WRITE|PROT_EXEC
        str arg2,[fp,#p_unmap+1*4]  // length to unmap
        mov arg1,#0  // any addr
        do_sys7t __NR_mmap64; cmn r0,#4096; bcc 0f; bkpt; 0:
        str r0,[fp,#p_unmap+0*4]  // address to unmap

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
        add ecx,ecx,#3  // round up to full words
        lsr ecx,ecx,#2
        bl movsl  // copy all aligned words that contain compressed data

        mov edx,edi  // lo(dst) of copied code

        ldr esi,[fp,#o_uncpr]
        str edi,[fp,#o_uncpr]
        bl movsl_subr  // copy decompressor

        ldr esi,[fp,#o_unflt]
        str edi,[fp,#o_unflt]
        bl movsl_subr  // copy unfilter

        pop esi   // &supervise
        push edi  // &copied
        bl movsl_subr  // copy supervisor

        mov arg2,edi  // hi(dst) of copied code
        mov arg1,edx  // lo(dst) of copied code
        mov arg3,#0
        do_sys7t2 __ARM_NR_cacheflush

        pop pc  // goto copied supervisor

get4u:
        ldrb eax,[esi],#1;
        ldrb tmp,[esi],#1; orr  eax,eax,tmp,lsl # 8
        ldrb tmp,[esi],#1; orr  eax,eax,tmp,lsl #16
        ldrb tmp,[esi],#1; orr  eax,eax,tmp,lsl #24
        ret

wlen_subr:  // edx+= nwords of inline subr at *tmp
        ldr tmp,[tmp,#-4]  // 'bl <over>' instruction word
        bic tmp,tmp,#0xff<<24  // displacment field
        add tmp,tmp,#1  // displ omits one word
        add edx,edx,tmp
        ret

/*__XTHEENDX__*/
/*
vi:ts=8:et:nowrap
*/
