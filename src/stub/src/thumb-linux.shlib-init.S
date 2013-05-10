/*  thumb-linux.shlib-init.S -- Linux Elf shared library init & decompressor
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

#define SP(d) sp,#4*(_-d)  /* stack addressing mode */

.macro thumb_sys7t N
#if defined(ARMEL_EABI4)
        mov r7,#\N
        swi 0
#elif defined(ARM_OLDABI)
        blx x\N
#else
        error \N  // ARM_OLDABI, ARMEL_EABI4, ARMEL_DARWIN ?
#endif

.endm

.macro call4 label
        .balign 4
        bl \label
.endm

.macro push_ reg
        push {\reg}
_= 1+_  // one more word on stack
.endm

.macro pop_ reg
        pop {\reg}
_=-1+_  // one less word on stack
.endm

//#define lodsl ldr eax,[esi],#4
#define lodslu bl get4u

  section ELFMAINX
//D_INFO:
//  .long offset(.)  // detect relocation
//  .long offset(user DT_INIT)
//  .long offset(escape_hatch)
//  .long offset({l_info; p_info; b_info; compressed data})

#define DEBUG 0
        .code 16  //; .balign 4
.real_start_ofELFMAINX:
        .thumb_func
_start: .globl _start
        nop
#if DEBUG  //{
        bkpt_th  // for debugging
#endif  //}
        push {r0,r1,r2,r3,r4,r5,r6,r7,lr}
_=9
o_uinit= 1  // lr
        adr esi,here
        sub esi,esi,#4*4 + 2*(4+ DEBUG)  // D_INFO
here:
               mov ecx,esi
        ldr eax,[esi,#0*4]; sub ecx,eax; //str ecx,[SP(o_reloc)]
// reloc DT_INIT  for step 12
        ldr tmp,[esi,#1*4]; add tmp,ecx; str tmp,[SP(o_uinit)]
        ldr edi,[esi,#3*4]; add edi,ecx  // &l_info; also destination for decompress
// reloc &hatch (thumb mode)  for step 10
        ldr tmp,[esi,#2*4]; add tmp,ecx
        mov esi,#sz_l_info+sz_p_info
        add esi,edi  // &b_info

        push_ tmp
o_hatch=_   // 10
        sub sp,#2*4; _=2+_  // param space: munmap temp pages  step 9
p_unmap=_  // 12

        push_ lr
o_lr=_  // 13
        ldr eax,[esi,#sz_cpr]; add esi,#sz_b_info
        add esi,eax  // skip unpack helper block

        lodslu  // eax=dstlen
        lsl tmp,edi,#(32-PAGE_SHIFT)
        lsr tmp,tmp,#(32-PAGE_SHIFT)  // tmp= fragment
        add eax,tmp; push_ eax  // params: mprotect restored pages  step 8
        sub edi,tmp; push_ edi
p_mprot=_  // 15
        sub eax,tmp  // dstlen
        add edi,tmp  // dst
        lsr tmp,tmp,#2; push_ tmp  // w_fragment
o_wfrag=_  // 16

        call4 L610
f_unfilter:  // (char *ptr, uint len, uint cto, uint fid)
#define ptr r0
#define len r1
#define cto r2  /* FIXME: unused */
#define fid r3

#define t1 r2
#define t2 r3

#ifndef FILTER_ID  /*{*/
#define FILTER_ID 0x50  /* little-endian */
#endif  /*}*/
        lsl fid,fid,#24; tst len,len; beq unf_done
        lsr fid,fid,#24; tst ptr,ptr; beq unf_done
        cmp fid,#FILTER_ID; bne unf_done  // last use of fid
        b tst_unf
top_unf:
        sub len,len,#4
        ldr t1,[ptr,len]
        lsl t2,t1,#4
        lsr t2,t2,#4+24  // bits 27..24
        cmp t2,#0x0b; bne tst_unf  // not 'bl' subroutine call
        lsr len,len,#2; sub t2,t1,len  // convert to word-relative displacement
        lsl len,len,#2
        lsr t1,t1,#24; lsl t2,t2,#8
        lsl t1,t1,#24; lsr t2,t2,#8
        orr t1,t1,t2  // re-combine
        str t1,[ptr,len]
tst_unf:
        cmp len,#0
        bne top_unf
unf_done:
        ret

#undef ptr
#undef len
#undef cto
#undef fid

#undef t1
#undef t2

        .thumb_func
L610:
        push_ lr  // &f_unfilter (thumb mode)
o_unflt=_  // 17
        ldrb tmp,[esi,#b_method-4+1]; push_ tmp  // ftid
        ldrb tmp,[esi,#b_method-4+2]; push_ tmp  // cto8
        push_ eax  // dstlen  also for unfilter  step 7
        push_ edi  // dst    param for unfilter  step 7
p_unflt=_  // 21

        lodslu; mov ecx,eax  // ecx= srclen
        lodslu; push_ eax  // method,filter,cto,junk

        call4 L710
        .arm
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
        .code 16; .balign 2
.real_start_ofELFMAINZ:

        .thumb_func
L710:
.real_start_ofL710:

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

        mov tmp,lr; sub tmp,#1; push_ tmp  // &f_decompress (ARM mode)
o_uncpr=_  // 23
        add tmp,SP(p_unflt)+1*4; push_ tmp  // &dstlen
        push_ edi  // dst
        push_ ecx  // srclen
        push_ esi  // src;  arglist ready for decompress  step 6
p_uncpr=_  // 27

        mov tmp,#3
        and tmp,esi  // length of prefix alignment
        add ecx,#3  // allow  suffix alignment
        add ecx,tmp  // prefix increases byte length
        lsr ecx,#2  // w_srclen
        ldr tmp,[SP(o_wfrag)]; add edx,tmp,ecx  // w_srclen + w_frag
        ldr tmp,[SP(o_uncpr)]; bl wlen_subr; add edx,ecx
        ldr tmp,[SP(o_unflt)]; bl wlen_subr; add edx,ecx

        call4 L220
SAVE_=_
supervise:  // moved at runtime before being executed
        // Allocate pages for result of decompressing.
        // These replace the compressed source and the following hole.
        mov arg5,#0; mvn arg5,arg5  // -1; cater to *BSD for fd of MAP_ANON
        mov arg4,#MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED
        mov arg3,#PROT_READ|PROT_WRITE
        ldr arg2,[SP(p_mprot)+4]  // dstlen
        ldr arg1,[SP(p_mprot)  ]  // dst
        mov r6,arg1  // required result
        thumb_sys7t __NR_mmap64; cmp r0,r6; beq 0f; bkpt_th; 0:

        // Restore fragment of page below dst
        ldr ecx,[SP(o_wfrag)]
        //mov edi,r0  // NOP: edi==r0
        ldr esi,[SP(p_unmap)]
        bl movsl

        pop {arg1,arg2,arg3,arg4, eax}
_=-5+_  // 22
        blx eax  // decompress
        pop_ tmp  // toss arg5

// Place the escape hatch
        ldr edi,[SP(o_hatch)]
        ldr tmp,hatch  // the 2 instructions
        str tmp,[edi]
        add edi,#1  // thumb mode
        str edi,[SP(o_hatch)]

//p_unflt  // 21
        pop {arg1,arg2,arg3,arg4, eax, r5}  // r5= w_fragment [discard]
_=-6+_  // 15
        tst arg4,arg4; beq 0f  // 0==ftid ==> no filter
        blx eax  // f_unfilter
0:
        ldr arg1,[sp,#0*4]  // lo(dst)
        ldr arg2,[sp,#1*4]  // len
        add arg2,arg1  // hi(dst)
        bl x__ARM_NR_cacheflush

//p_mprot  // 15
        pop {arg1,arg2, tmp}; mov lr,tmp
_=-3+_  // 12
        mov arg3,#PROT_READ|PROT_EXEC
        thumb_sys7t __NR_mprotect

//p_unmap
.if __NR_munmap <= 0xff
        mov r7,#__NR_munmap
.else
        mov r7,#__NR_munmap>>16
        lsl r7,#16
        add r7,#__NR_munmap - ((__NR_munmap>>16)<<16)
.endif
        pop {arg1,arg2, pc}  // goto hatch
_=-3+_  // 9
        .balign 4
hatch:
        swi 0  // 0xdf00; munmap
        pop {r0,r1,r2,r3,r4,r5,r6,r7,pc}  // 0xbdff; goto user DT_INIT

        .thumb_func
movsl_subr:
.real_start_ofmovsl_subr:
        lsr esi,esi,#2
        lsl esi,esi,#2  // word align [corrects for thumb-mode]
        push {lr}; mov tmp,esi; bl wlen_subr
        pop {tmp}; mov lr,tmp
// FALL THROUGH to the part of 'movsl' that trims to a multiple of 8 words.
// 7/8 of the time this is faster; 1/8 of the time it's slower.
9:
        ldr tmp,[esi,#0]; add esi,#4
        str tmp,[edi,#0]; add edi,#4
        sub ecx,#1
        .thumb_func
movsl:  // In:  edi= 4-byte aligned dst; esi= 4-byte aligned src; ecx= word count
.real_start_ofmovsl:
        mov tmp,#7; tst ecx,tmp; bne 9b
        lsr ecx,#3; beq 9f
        .balign 4; bx pc; nop  // enter ARM mode
        .arm
        stmdb  sp!,{r2,   r4,r6, r7,r8,r9}  // tmp===r3, ecx===r5
7:
        ldmia esi!,{r2,r3,r4,r6, r7,r8,r9,r12}; subs ecx,ecx,#1
        stmia edi!,{r2,r3,r4,r6, r7,r8,r9,r12}; bne 7b
        ldmia  sp!,{r2,   r4,r6, r7,r8,r9}
9:
        ret

#if !defined(ARMEL_EABI4)  /*{*/
        .arm
        .balign 4
x__NR_mmap:
        do_sys7t __NR_mmap64
        bx lr
x__NR_munmap:
        do_sys7t __NR_munmap
        bx lr
x__NR_mprotect:
        do_sys7t __NR_mprotect
        bx lr
        .thumb
#endif  /*}*/

        .thumb_func
x__ARM_NR_cacheflush:
.real_start_ofx__ARM_NR_cacheflush:
        mov arg3,#0
        mov r7,#__ARM_NR_BASE>>16
        lsl r7,#16
        add r7,# __ARM_NR_cacheflush - __ARM_NR_BASE
        swi 0
        bx lr

        .thumb_func
L220:
.real_start_ofL220:
_=SAVE_  // 27
        mov tmp,lr; sub tmp,#1; push_ tmp  // &supervise
o_super=_  // 28
        bl wlen_subr; add edx,ecx  // wlen_supervise
        lsl arg2,edx,#2  // convert to bytes

        // Allocate pages to hold temporary copy.
        mov arg5,#0; mvn arg5,arg5  // -1; cater to *BSD for fd of MAP_ANON
        mov arg4,#MAP_PRIVATE|MAP_ANONYMOUS
        mov arg3,#PROT_READ|PROT_WRITE|PROT_EXEC
        str arg2,[SP(p_unmap)+1*4]  // length to unmap
        mov arg1,#0  // any addr
        thumb_sys7t __NR_mmap64; asr tmp,r0,#12; add tmp,#1; bne 0f; bkpt_th; 0:
        str r0,[SP(p_unmap)]  // address to unmap

        ldr esi,[SP(p_mprot)]
        //mov edi,r0  // edi= dst  NOP: edi==r0
        ldr ecx,[SP(o_wfrag)]  // w_fragment
        bl movsl  // copy the fragment

        ldr esi,[SP(p_uncpr)+0*4]  // src
        ldr ecx,[SP(p_uncpr)+1*4]  // len
        mov tmp,#3
        and tmp,esi  // length of prefix alignment
        sub esi,tmp  // down to word aligned
        add ecx,tmp  // prefix increases byte length
        add tmp,edi // skip prefix at destination
        str tmp,[SP(p_uncpr)+0*4]  // dst
        add ecx,#3  // round up to full words
        lsr ecx,#2
        bl movsl  // copy all aligned words that contain compressed data

        mov edx,edi  // lo(dst) of copied code

        ldr esi,[SP(o_uncpr)]
        str edi,[SP(o_uncpr)]
        bl movsl_subr  // copy decompressor

        add tmp,edi,#1  // dst f_unfilter thumb mode
        ldr esi,[SP(o_unflt)]
        str tmp,[SP(o_unflt)]
        bl movsl_subr  // copy f_unfilter

        pop_ esi   // &supervise
        add r7,edi,#1  // &copied (thumb mode)
        bl movsl_subr  // copy supervisor

        mov arg2,edi  // hi(dst) of copied code
        mov arg1,edx  // lo(dst) of copied code
        mov lr,r7  // return address for ...
        b x__ARM_NR_cacheflush  // call with continuation return

get4u:
        ldrb eax,[esi,#3];
        ldrb tmp,[esi,#2]; lsl eax,#8; orr eax,tmp
        ldrb tmp,[esi,#1]; lsl eax,#8; orr eax,tmp
        ldrb tmp,[esi,#0]; lsl eax,#8; orr eax,tmp
        add esi,#4
        ret

wlen_subr:  // Out: ecx= nwords of inline subr at *tmp
        lsr tmp,tmp,#2
        lsl tmp,tmp,#2  // word align (correct for thumb mode)
        sub tmp,#4
        ldrh ecx,[tmp,#0]; lsl ecx,#32-11; lsr ecx,#32-11-11  // hi(disp)
        ldrh tmp,[tmp,#2]; lsl tmp,#32-11; lsr tmp,#32-11- 0  // lo(disp)
        add ecx,tmp  // disp
        add ecx,#1+1  // disp omits 1 word; prepare to round
        lsr ecx,#1  // round up to whole 32-bit words
        ret

/*__XTHEENDX__*/
/*
vi:ts=8:et:nowrap
*/
