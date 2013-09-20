/*
;  amd64-darwin.macho-entry.S -- program entry point & decompressor (amd64 Mach-o)
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2013 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2013 Laszlo Molnar
;  Copyright (C) 2000-2013 John F. Reiser
;  All Rights Reserved.
;
;  UPX and the UCL library are free software; you can redistribute them
;  and/or modify them under the terms of the GNU General Public License as
;  published by the Free Software Foundation; either version 2 of
;  the License, or (at your option) any later version.
;
;  This program is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this program; see the file COPYING.
;  If not, write to the Free Software Foundation, Inc.,
;  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
;
;  Markus F.X.J. Oberhumer              Laszlo Molnar
;  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
;
;  John F. Reiser
;  <jreiser@users.sourceforge.net>
;
*/

#include "arch/amd64/macros.S"
#include "arch/amd64/regs.h"


/*************************************************************************
// program entry point
// see glibc/sysdeps/amd64/elf/start.S
**************************************************************************/

section MACHMAINX
_start: .globl _start
//    int3
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

M_NRV2B_LE32=2  // ../conf.h
M_NRV2D_LE32=5
M_NRV2E_LE32=8

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

/* lzma has its own 'section's */
#include "arch/amd64/lzma_d.S"

  section NRV_TAIL
/* NRV_TAIL is empty */

  section MACHMAINY
eof:
        pop %rcx  // &input_eof
        movq %rsi,%rax; subq %rcx,%rax  // src -= eof;  // return 0: good; else: bad
        pop %rdx;       subq %rdx,%rdi  // dst -= original dst
        pop %rcx;            movl %edi,(%rcx)  // actual length used at dst  XXX: 4GB
        pop %rbx; pop %rbp
        ret

end_decompress: .globl end_decompress

        /* IDENTSTR goes here */

  section MACHMAINZ
#define PAGE_SIZE ( 1<<12)

GAP= 128  // > farthest prefetch;               must match ../../p_mach.cpp
NO_LAP= 64  // avoid overlap for folded loader; must match ../../p_mach.cpp

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

// Decompress the rest of this loader, and jump to it.
unfold:
        pop %rbx  // &{ b_info:{sz_unc, sz_cpr, 4{byte}}, compressed_data...}
        mov sz_cpr(%rbx),%ecx
        lea              -1+ sz_b_info(%rcx,%rbx),%rsi  // &hi_byte folded original
#if 0
        mov sz_unc(%rbx),%edi
#else
        // sz_unc == 0
        mov (%rbx),%edi
#endif
        lea GAP + NO_LAP -1+ sz_b_info(%rdi,%rbx),%rdi  // &hi_byte folded copy
        std; rep movsb  // copy descending

        lea GAP                            (%rbx),%arg3 // &unfolded
        mov %rbx,%rsi  // &b_info
        lea 1(%rdi),%arg1  // &lo_byte folded copy
        push %arg3  // return to unfolded code

        cld
        lodsl; push %rax          // allocate slot on stack
               movq  %rsp,%arg4   // &len_dst ==> used by lzma for EOF
        lodsl; push %rax          // sz_cpr  XXX: 4GB
        lodsl; movzbl %al,%arg5l  // b_method
               pop %arg2          // sz_cpr
        call *%rbp  // decompress(&src, srclen, &dst, &dstlen, b_info.misc)
               pop %rcx  // discard len_dst
        lea -4+ _start - ret_main(%rbp),%rbx  // &total_length for fold:
        ret
main:
        pop %rbp  // &decompress
        call unfold
            // compressed fold_elf86 follows

// vi:ts=8:et:nowrap
