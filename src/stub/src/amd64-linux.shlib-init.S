/*  amd64-linux.shlib-init.S -- Linux program entry point & decompressor (Elf shared lib)
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

#include "arch/amd64/macros.S"
#include "arch/amd64/regs.h"

sz_Ehdr= 64
sz_Phdr= 56

sz_l_info= 12
  l_lsize= 8

sz_p_info= 12

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

PROT_READ=  1
PROT_WRITE= 2
PROT_EXEC=  4

MAP_PRIVATE= 2
MAP_FIXED=     0x10
MAP_ANONYMOUS= 0x20

__NR_mmap=      9  // 64-bit mode only!  /usr/include/asm/unistd_64.h
__NR_mprotect= 10
__NR_munmap=   11

__NR_write=  1
__NR_exit=  60

PAGE_SHIFT= 12
PAGE_MASK= (~0<<PAGE_SHIFT)
PAGE_SIZE= -PAGE_MASK

M_NRV2B_LE32=2  // ../conf.h
M_NRV2D_LE32=5
M_NRV2E_LE32=8


//  .long offset(.)  // detect relocation
//  .long offset(user DT_INIT)
//  .long offset(escape_hatch)
//  .long offset({l_info; p_info; b_info; compressed data})
  section ELFMAINX
_start: .globl _start
////  nop; int3; int3
        push %rax  // space for &DT_INIT
o_uinit= 5*8
        push %arg2; push %arg1  // save first two args to DT_INIT()
        push %rax  // space for &hatch
o_hatch= 2*8
        push %arg3  // save third arg to DT_INIT()
        push %rbp; mov %rsp,%rbp  // frame pointer
        call main  // push &decompress
ret_main:

/* Returns 0 on success; non-zero on failure. */
decompress:  // (uchar const *src, size_t lsrc, uchar *dst, u32 &ldst, uint method)

/* Arguments according to calling convention */
#define src  %arg1
#define lsrc %arg2
#define dst  %arg3
#define ldst %arg4  /* Out: actually a reference: &len_dst */
#define meth %arg5l
#define methb %arg5b

        push %rbp; push %rbx  // C callable
        push ldst
        push dst
        addq src,lsrc; push lsrc  // &input_eof

  section NRV_HEAD

/* Working registers */
#define off  %eax  /* XXX: 2GB */
#define len  %ecx  /* XXX: 2GB */
#define lenq %rcx
#define bits %ebx
#define disp %rbp

        movq src,%rsi  // hardware src for movsb, lodsb
        movq dst,%rdi  // hardware dst for movsb
        xor bits,bits  // empty; force refill
        xor len,len  // create loop invariant
        orq $(~0),disp  // -1: initial displacement
        call setup  // push &getbit [TUNED]
ra_setup:

/* AMD64 branch prediction is much worse if there are more than 3 branches
   per 16-byte block.  The jnextb would suffer unless inlined.  getnextb is OK
   using closed subroutine to save space, and should be OK on cycles because
   CALL+RET should be predicted.  getnextb could partially expand, using closed
   subroutine only for refill.
*/
/* jump on next bit {0,1} with prediction {y==>likely, n==>unlikely} */
/* Prediction omitted for now. */
/* On refill: prefetch next byte, for latency reduction on literals and offsets. */
#define jnextb0np jnextb0yp
#define jnextb0yp GETBITp; jnc
#define jnextb1np jnextb1yp
#define jnextb1yp GETBITp; jc
#define GETBITp \
        addl bits,bits; jnz 0f; \
        movl (%rsi),bits; subq $-4,%rsi; \
        adcl bits,bits; movb (%rsi),%dl; \
0:
/* Same, but without prefetch (not useful for length of match.) */
#define jnextb0n jnextb0y
#define jnextb0y GETBIT; jnc
#define jnextb1n jnextb1y
#define jnextb1y GETBIT; jc
#define GETBIT \
        addl bits,bits; jnz 0f; \
        movl (%rsi),bits; subq $-4,%rsi; \
        adcl bits,bits; \
0:

/* rotate next bit into bottom bit of reg */
#define getnextbp(reg) call *%r11; adcl reg,reg
#define getnextb(reg)  getnextbp(reg)


getbit:
        addl bits,bits; jz refill  // Carry= next bit
        rep; ret
refill:
        movl (%rsi),bits; subq $-4,%rsi  // next 32 bits; set Carry
        adcl bits,bits  // LSB= 1 (CarryIn); CarryOut= next bit
        movb (%rsi),%dl  // speculate: literal, or bottom 8 bits of offset
        rep; ret

copy:  // In: len, %rdi, disp;  Out: 0==len, %rdi, disp;  trashes %rax, %rdx
        leaq (%rdi,disp),%rax; cmpl $5,len  // <=3 is forced
        movb (%rax),%dl; jbe copy1  // <=5 for better branch predict
        cmpq $-4,disp;   ja  copy1  // 4-byte chunks would overlap
        subl $4,len  // adjust for termination cases
copy4:
        movl (%rax),%edx; addq $4,      %rax; subl $4,len
        movl %edx,(%rdi); leaq  4(%rdi),%rdi; jnc copy4
        addl $4,len; movb (%rax),%dl; jz copy0
copy1:
        incq %rax; movb %dl,(%rdi); subl $1,len
                   movb (%rax),%dl
        leaq 1(%rdi),%rdi;          jnz copy1
copy0:
        rep; ret

setup:
        cld
        pop %r11  // addq $ getbit - ra_setup,%r11  # &getbit

  section NRV2E
#include "arch/amd64/nrv2e_d.S"

  section NRV2D
#include "arch/amd64/nrv2d_d.S"

  section NRV2B
#include "arch/amd64/nrv2b_d.S"

#include "arch/amd64/lzma_d.S"

  section NRV_TAIL
        // empty

#undef off
#undef len
#undef lenq
#undef bits
#undef disp

  section ELFMAINY
eof:
        pop %rcx  // &input_eof
        movq %rsi,%rax; subq %rcx,%rax  // src -= eof;  // return 0: good; else: bad
        pop %rdx;       subq %rdx,%rdi  // dst -= original dst
        pop %rcx;            movl %edi,(%rcx)  // actual length used at dst  XXX: 4GB
        pop %rbx; pop %rbp
        ret

msg_SELinux:
        push $ L71 - L70; pop %arg3  // length
        call L72
L70:
        .asciz "PROT_EXEC|PROT_WRITE failed.\n"
L71:
        // IDENTSTR goes here

  section ELFMAINZ
L72:
        pop %arg2  // message text
        push $2; pop %arg1  // fd stderr
        push $ __NR_write; pop %rax
        syscall
die:
        push $127; pop %arg1
        push $ __NR_exit; pop %rax
        syscall

main:
////  nop; int3; int3

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

        pop %rdx  // &decompress

        lea _start - decompress - 4*4(%rdx),%rsi
               mov %rsi,%rcx
        lodsl; sub %rax,%rcx; //mov %rcx,o_reloc(%rbp)
        lodsl; add %rcx,%rax; mov %rax,o_uinit(%rbp)  // reloc DT_INIT  for step 12
        lodsl; add %rcx,%rax; mov %rax,o_hatch(%rbp)  // reloc &hatch   for step 10
        lodsl; lea (%rcx,%rax),%rdi  // &l_info; also destination for decompress
        lea sz_l_info+sz_p_info(%rdi),%rsi  // &b_info

        push %rax; push %rax  // param space: munmap temp pages  step 9
p_unmap= -2*8

        lodsl; lodsl; add %rax,%rsi; lodsl  // skip unpack helper block

        lodsl  // eax=dstlen
        mov %rdi,%rcx
        and $~PAGE_MASK,%ecx  // %ecx= fragment
        add %rcx,%rax; push %rax  // params: mprotect restored pages  step 8
        sub %rcx,%rdi; push %rdi
p_mprot= -4*8
        sub %rcx,%rax  // restore
        add %rcx,%rdi
        push %rcx  // fragment
o_frag = -5*8

        call L210
#include "arch/amd64/bxx.S"
L210:
o_unflt= -6*8
        movzbl b_method-4+1(%rsi),%ecx; push %rcx  // ftid
        movzbl b_method-4+2(%rsi),%ecx; push %rcx  // cto8
        push %rax; mov %rsp,%rcx  // dstlen  also for unfilter  step 7
        push %rdi  // dst                 param for unfilter  step 7
p_unflt= -10*8

        push %rdx  // &decompress
o_uncpr= -11*8
        lodsl; mov %eax,%edx  // %rdx= srclen
        lodsl; push %rax  // method,filter,cto,junk
        push %rcx  // &dstlen
        push %rdi  // dst
        push %rdx  // srclen
        push %rsi  // src;  arglist ready for decompress  step 6
p_uncpr= -16*8

        mov o_uncpr(%rbp),%rax; add -4(%rax),%edx  // l_d_cpr + l_f_unc
        mov o_unflt(%rbp),%rax; add -4(%rax),%edx  // l_d_cpr + l_f_unc + l_f_unf

        call L220
supervise:
        // Allocate pages for result of decompressing.
        // These replace the compressed source and the following hole.
        push $0; pop %arg6
        push $0; pop %arg5
        push $MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED; pop %sys4
        push $PROT_READ|PROT_WRITE; pop %arg3
        movq p_mprot+8(%rbp),%arg2  // dstlen
        movq p_mprot  (%rbp),%arg1  // dst
        push $__NR_mmap; pop %rax; syscall
                cmp %arg1,%rax; je 0f; hlt; 0:

        // Restore fragment of page below dst
        movl o_frag(%rbp),%ecx
        mov %rax,%rdi
        mov p_unmap(%rbp),%rsi
        add $3,%ecx; shr $2,%ecx  // FIXME: is this safe?
        rep movsl

        pop %arg1
        pop %arg2
        pop %arg3
        pop %arg4
        pop %arg5
        pop %rax; call *%rax  // decompress
//p_unflt
        pop %arg1
        pop %arg2

        lea (%arg1,%arg2),%rax
        movl $0x5e5f050f, (%rax)  // "syscall; pop %rdi; pop %rsi"
        movb       $0xc3,4(%rax)  // "ret"
        mov %rax,o_hatch(%rbp)  // hatch beyond .text

        pop %arg3
        pop %arg4
        pop %rax;
        test %arg4,%arg4; je 0f  // 0==ftid ==> no filter
        call *%rax  // unfilter
0:
        pop %rcx  // toss fragment
//p_mprot
        pop %arg1  // dst including fragment
        pop %arg2  // dstlen
        push $PROT_READ|PROT_EXEC; pop %arg3
        push $__NR_mprotect; pop %rax; syscall
//p_unmap
        pop %arg1  // &temp pages
        pop %arg2  // length
        push $__NR_munmap; pop %rax

////  nop; int3; int3

        pop %rbp
        pop %arg3  // third arg to DT_INIT()
        ret  // goto escape hatch
//hatch:
//      syscall  // munmap temporary pages
//      pop %arg1  // first two args to DT_INIT()
//      pop %arg2
//      ret  // goto user DT_INIT

L220:
        mov o_frag(%rbp),%arg2l  // fragment
        add %edx,%arg2l  // + l_d_cpr + l_f_unc + l_f_unf
        pop %rax; push %rax  // &supervise
        add -4(%rax),%arg2l  // total length to allocate

        // Allocate pages to hold temporary copy.
        push $0; pop %arg6
        push $0; pop %arg5
        push $MAP_PRIVATE|MAP_ANONYMOUS; pop %sys4
        push $PROT_READ|PROT_WRITE|PROT_EXEC; pop %arg3
        mov %arg2,p_unmap+8(%rbp)  // length to unmap
        push $0; pop %arg1  // addr
        push $__NR_mmap; pop %rax; syscall
                cmpq $PAGE_MASK,%rax; jb 0f; hlt; 0:

        mov %rax,p_unmap  (%rbp)  // addr
        mov %rax,%rdi  // %rdi= dst
        pop %rax  // &supervise
        mov o_frag(%rbp),%ecx  // fragment
//p_uncpr
        mov p_mprot(%rbp),%rsi
        add $3,%ecx; shr $2,%ecx  // FIXME: is this safe?
        rep movsl  // copy the fragment

        pop %rsi  // &src data (after fragment)
                pop %rcx; push %rcx  // length
        push %rdi  // &copied data (after fragment)
        add $3,%ecx; shr $2,%ecx
        rep movsl  // copy compressed data

        mov      o_uncpr(%rbp),%rsi
        mov %rdi,o_uncpr(%rbp)
        mov -4(%rsi),%ecx
        rep movsb  // copy decompressor

        mov      o_unflt(%rbp),%rsi
        mov %rdi,o_unflt(%rbp)
        mov -4(%rsi),%ecx
        rep movsb  // copy unfilter

//o_super
        mov %rax,%rsi  // %rsi= &supervise
        push %rdi  // &copied
        mov -4(%rsi),%ecx
        rep movsb  // copy supervisor

        ret  // goto copied supervise:

/*__XTHEENDX__*/

/*
vi:ts=8:et:nowrap
 */

