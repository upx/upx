/*  arm-linux.shlib-init.S -- Linux Elf shared library init & decompressor
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2023 Laszlo Molnar
*  Copyright (C) 2000-2023 John F. Reiser
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

sz_Elf64_Ehdr = 16 + 2*2 + 4 + 3*NBPW + 4 + 6*2
sz_Elf64_Phdr = 2*4 + 6*NBPW

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

a_type = 0*NBPW
a_val  = 1*NBPW
AT_NULL=   0
AT_PAGESZ= 6

__NR_exit =   93
__NR_write =  64
__NR_mmap64   = 0xde  // 222
__NR_munmap   = 0xd7  // 215
__NR_mprotect = 0xe2  // 226

__ARM_NR_cacheflush =  (1<<31)  // FIXME

#define arg1 x0
#define arg2 x1
#  define arg2w w1
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

N_SLOT= 0
sp_frame = 24 * NBPW
.macro slot  symbol, n
  \symbol = N_SLOT*NBPW
  .ifnb n
    N_SLOT = \n + N_SLOT
  .else
    N_SLOT =  1 + N_SLOT
  .endif
  .if sp_frame < N_SLOT*NBPW
    error "too many slots"
  .endif
.endm

  section ELFMAINX
//  .long offset(b_info)|(asl_delta>>12)  src of f_exp
//  .long offset(.)  // detect relocation
//  .long offset(user DT_INIT)
//  .long offset(escape_hatch)
//  .long offset(xct_off)    dst of f_exp
_start: .globl _start
//    brk #0  // debugging

  slot f_argc  // 0
  slot f_argv  // 1
  slot f_envp  // 2
  slot f_uinit  // 3  user DT_INIT
  slot f_PMASK  // 4  PAGE_MASK
  slot f_my_ra  // 5

        stp arg1,arg2,[sp,#f_argc - sp_frame]!  // f_argv
        stp arg3,x0,  [sp,#f_envp]  // %f_uinit
        str lr,       [sp,#f_my_ra]

        bl main  // ra= &f_decompress
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
main:

  section ELFMAJNZ
// Calculate PAGE_MASK
0:  // Advance envp to auxp
        ldr x3,[arg3],#NBPW; cbnz x3,0b

        mov x3,#1<<PAGE_SHIFT  // default
0:  // Find AT_PAGESZ
        ldp x0,x1,[arg3],#2*NBPW; cbz x0,5f  // AT_NULL==.a_type; use default
        cmp x0,#AT_PAGESZ; bne 0b
5:  // auxp cannot be empty (must have AT_UID), so 'cmp' above sets nzvc
        csel x1,x1,x3,eq  // x1 if found, else x3
        neg x1,x1

  section ANDMAJNZ  // Android passes junk args to DT_INIT?
        mov x1,#~0<<12  // -4096

  section ELFMAKNZ
        str x1,[sp,#f_PMASK]

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

        mov     rdx,lr  //    &f_decompress
        add rsi,rdx,# _start - f_decompress - 5*4

#define lodsl ldr eax,[rsi],#4
#define lodslu lodsl

        lodsl; and tmp2w,eax,#1  // 1 iff ET_DYN --android-shlib
               bic tmp,  eax,tmp2w  // offset(b_info)
               mov rcx,rsi
        lodsl; sub rcx,rcx,rax; //str ecx,[sp,#o_reloc]
        lodsl; add rax,rax,tmp2x,lsl 12  // +asl_delta
               add rax,rax,rcx; str rax,[sp,#f_uinit]  // reloc DT_INIT  for step 12
  slot o_hatch  // 6
        lodsl; add rax,rax,rcx; str rax,[sp,#o_hatch]  // reloc &hatch   for step 10
        lodsl; add rdi,rax,rcx  // destination for decompress
               add rsi,tmpx,rcx  // &b_info  src for f_exp

  slot p_unmap,2  // 7

        ldr eax,[rsi,#sz_cpr]; add rsi,rsi,#sz_b_info
        add rsi,rsi,rax  // skip unpack helper block

        ldr ecx,[sp,#f_PMASK]
        lodslu  // eax=dstlen
        bic ecx,edi,ecx  // ecx= fragment
        add ecx,ecx,#3
        bic ecx,ecx,#3  // w_frag  [can be PAGE_SIZE !]

        add eax,eax,ecx  // dstlen + fragment
        sub rdi,rdi,ecx,uxtw  // page boundary
  slot p_mprot,2  // 9
        stp rdi,rax,[sp,#p_mprot]  // params: mprotect restored pages  step 8
        add rdi,rdi,ecx,uxtw  // restore dst
        sub eax,eax,ecx  // restore dstlen

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

        lsr ecx,ecx,#2  // w_frag
  slot o_unflt  // 11
  slot o_wfrag  // 12
        stp lr,rcx,[sp,#o_unflt]  // o_wfrag
        ldrb tmp1w,[rsi,# b_method-4+1]  // ftid
        ldrb tmp2w,[rsi,# b_method-4+2]  // cto8
  slot p_unflt,4  // 13
        stp rdi,rax,[sp,#0*NBPW + p_unflt]  // dst, dstlen
        stp tmp2x,tmp1x,[sp,2*NBPW + p_unflt]  // cto8, ftid  for unfilter  step 7

        lodslu; mov ecx,eax  // ecx= srclen
        lodslu
  slot o_uncpr,2  // 17
        stp rdx,rax,[sp,#o_uncpr]  // &decompress, {method,filter,cto,junk}
        add tmpx,sp,#1* NBPW + p_unflt  // &dstlen
  slot p_uncpr,4  // 19
        stp rsi,rcx, [sp,#0*NBPW + p_uncpr]  // src, srclen
        stp rdi,tmpx,[sp,#2*NBPW + p_uncpr]  // dst, &dstlen  arglist ready for decompress  step 6

        add rcx,rcx,#3  // allow  suffix alignment
        ldr tmp,[sp,#o_wfrag]; add rdx,tmpx,rcx,lsr #2  // w_srclen + w_frag
        ldr tmpx,[sp,#o_uncpr]; bl wlen_subr
        ldr tmpx,[sp,#o_unflt]; bl wlen_subr

        bl L220
supervise:
        // Allocate pages for result of decompressing.
        // These replace the compressed source and the following hole.
        mov arg6,#0
        mov arg5,#-1  // cater to *BSD for fd of MAP_ANON
        mov arg4,#MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED
        mov arg3,#PROT_READ|PROT_WRITE
        ldp arg1,arg2,[sp,#p_mprot]  // dst, dstlen
        mov x6,arg1  // required result
        do_sys __NR_mmap64; cmp x0,x6; beq 0f; brk #0; 0:

        // Restore fragment of page below dst
        ldr ecx,[sp,#o_wfrag]
        //mov edi,r0  // NOP: edi==r0
        ldr rsi,[sp,#0*NBPW + p_unmap]
        bl movsl

//p_uncpr
        ldr rax,[sp,#o_uncpr]
        ldp arg1,arg2,[sp,#0*NBPW + p_uncpr]
        ldp arg3,arg4,[sp,#2*NBPW + p_uncpr]
        blr rax  // decompress

        bl L620
//hatch:  IN: lr= f_my_ra; {arg1,arg2}= p_unmap; arg3= f_envp; arg4= f_uinit
        svc #0  // munmap(arg1,arg2)
        ldp arg1,arg2,[sp],#sp_frame  // f_argc, f_argv
        br arg4

L620:  // Implant escape hatch at end of .text
        ldr rax,[sp,#o_hatch]
        ldp arg1,arg2,[lr]    // 4 instr
        str arg1, [rax]       // 2 instr
        str arg2w,[rax,#2*4]  // 1 instr

//p_unflt
        ldr rax,[sp,#o_unflt]
        ldp arg3,arg4,[sp,#2*NBPW + p_unflt]
        ldp arg1,arg2,[sp,#0*NBPW + p_unflt]
        cbz arg4,0f  // 0==ftid ==> no filter
        blr rax  // unfilter
0:
//p_mprot
        ldp arg1,arg2,[sp,#p_mprot]  // dst, dstlen
        mov arg3,#PROT_READ|PROT_EXEC
        do_sys __NR_mprotect

//p_unmap
        ldp lr,arg5,[sp,#f_my_ra]  // lr= f_my_ra; arg5= o_hatch
        ldp arg1,arg2,[sp,#0*NBPW + p_unmap]
          mov w8,#__NR_munmap
          ldp arg3,arg4,[sp,#2*NBPW + f_argc]  // f_uinit
        br arg5  // goto hatch

movsl_subr:
        ldr ecx,[rsi,#-4]  // 'bl <over>' instruction word
        bic ecx,ecx,#0xff<<24  // displacment field
movsl:  // rdi= 4-byte aligned dst; rsi= 4-byte aligned src; ecx= word count
        tbz ecx,#0,5f  // goto 5f if ecx is even
        ldr w3,[rsi],#4
        sub ecx,ecx,#1
        str w3,[rdi],#4
5:
        cbz ecx,9f
7:
        ldp w2,w3,[rsi],#2*4; sub  ecx,ecx,#2
        stp w2,w3,[rdi],#2*4; cbnz ecx,7b
9:
        ret

L220:
  slot f_super  // 23
        str lr,[sp,#f_super]
        mov tmpx,lr; bl wlen_subr  // wlen_supervise
        lsl arg2,rdx,#2  // convert to bytes

        // Allocate pages to hold temporary copy.
        mov arg6,#0
        mov arg5,#-1  // cater to *BSD for fd of MAP_ANON
        mov arg4,#MAP_PRIVATE|MAP_ANONYMOUS
        mov arg3,#PROT_READ|PROT_WRITE  // some OS prohibit PROT_WRITE && PROT_EXEC
        str arg2,[sp,#1*NBPW + p_unmap]  // length to unmap
        mov arg1,#0  // any addr
        do_sys __NR_mmap64; cmn x0,#4096; bcc 0f; brk #0; 0:
        str x0,[sp,#0*NBPW + p_unmap]  // address to unmap

        ldr rsi,[sp,#p_mprot]  // dst
        //mov edi,r0  // edi= dst  NOP: edi==r0
        ldr ecx,[sp,#o_wfrag]  // w_fragment
        bl movsl  // copy the fragment

        ldp rsi,rcx,[sp,#p_uncpr]  // src, len
        str rdi,    [sp,#p_uncpr]  // relocated src
        add ecx,ecx,#3; lsr ecx,ecx,#2
        bl movsl  // copy compressed data

        mov rdx,rdi  // lo(dst) of copied code

        ldr rsi,[sp,#o_uncpr]
        str rdi,[sp,#o_uncpr]
        bl movsl_subr  // copy decompressor

        ldr rsi,[sp,#o_unflt]
        str rdi,[sp,#o_unflt]
        bl movsl_subr  // copy unfilter

        ldr rsi,[sp,#f_super]
        str rdi,[sp,#f_super]
        bl movsl_subr  // copy supervisor

        ldp arg1,arg2,[sp,#0*NBPW + p_unmap]  // PROT_EXEC for supervise
        mov arg3,#PROT_READ|PROT_EXEC  // some OS prohibit PROT_WRITE && PROT_EXEC
        do_sys __NR_mprotect; cmn x0,#4096; bcc 0f; brk #0; 0:

        ldr lr,[sp,#f_super]
        br  lr  // goto copied supervisor

wlen_subr:  // rdx+= nwords of inline subr at *tmp
        ldr tmp,[tmpx,#-4]  // 'bl <over>' instruction word
        bic tmp,tmp,#0xff<<24  // displacment field
        add rdx,rdx,tmpx
        ret

/*__XTHEENDX__*/

/* vim:set ts=8 sw=8 et: */
