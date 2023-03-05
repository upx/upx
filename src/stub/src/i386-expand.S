/*
;  i386-expand.S -- decompressors for i386
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2021 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2021 Laszlo Molnar
;  Copyright (C) 2000-2021 John F. Reiser
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
;  <markus@oberhumer.com>               <ezerotven+github@gmail.com>
;
;  John F. Reiser
;  <jreiser@users.sourceforge.net>
;
*/

#define section .section

NBPW= 4

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
        movl (%esi),bits; sub $-4,%esi; \
        adcl bits,bits; movb (%esi),%dl; \
0:
/* Same, but without prefetch (not useful for length of match.) */
#define jnextb0n jnextb0y
#define jnextb0y GETBIT; jnc
#define jnextb1n jnextb1y
#define jnextb1y GETBIT; jc
#define GETBIT \
        addl bits,bits; jnz 0f; \
        movl (%esi),bits; sub $-4,%esi; \
        adcl bits,bits; \
0:

/* rotate next bit into bottom bit of reg */
#define getnextbp(reg) GETBITp; adcl reg,reg
#define getnextb(reg)  getnextbp(reg)

sz_unc= 0*4  // offsets in struct b_info
sz_cpr= 1*4
method= 2*4
sz_binfo= 3*4

  section EXP_HEAD

f_unfilter:  // (*f_unf)(xo->buf, out_len, h.b_cto8, h.b_ftid);
#include "arch/i386/bxx.S"

// int f_expand(nrv_byte const *src, nrv_byte *dst, size_t *dstlen)  // C-callable
// Includes unfilter and cache flush.
// Returns 0 on success.
//  *dstlen returns actual length
//  *src includes 3-word b_info (sz_unc, sz_cpr, {method, filter, cto8, extra})
//  src might not be 4-byte aligned.
// C-callable, so must save+restore %ebx,%esi,%edi,%ebp

/* Working registers for NRV2B/NRV2D/NRV2E (i386) */
#define off  %eax  /* XXX: 2GB */
#define len  %ecx  /* XXX: 2GB */
#define lenq %ecx
#define bits %ebx
#define src %esi
#define dst %edi
#define disp  %ebp
#define displ %ebp
#define dispq %ebp

f_expand: .globl f_expand   // start of code for actual de-compressor
    push %ebp; mov %esp,%ebp
    push %edi; push %esi; push %ebx  // MATCH_29
#define fx_src    2*NBPW(%ebp)
#define fx_dst    3*NBPW(%ebp)
#define fx_dstlen 4*NBPW(%ebp)
    call decompress
    push %eax  // MATCH_30  result of decompression: 0=>success

    mov fx_src,src
    movzbl 1+ method(src),%eax; test %eax,%eax; je no_unf; push %eax  // ftid
    movzbl 2+ method(src),%eax; push %eax  // cto8
    push /*sz_unc*/(src)
    push fx_dst
    call f_unfilter; add $4*NBPW,%esp

no_unf:  // i386 cache is coherent I+D, so no __clear_cache
    pop %eax  // MATCH_30
    pop %ebx; pop %esi; pop %edi; pop %ebp  // MATCH_29
    ret

// common subroutines for NRV2B,NRV2D,NRV2E
refill:
        movl (%esi),bits; sub $-4,%esi  // next 32 bits; set Carry
        adcl bits,bits  // LSB= 1 (CarryIn); CarryOut= next bit
        movb (%esi),%dl  // pre-fetch: literal, or bottom 8 bits of offset
        rep; ret
getbit:
        addl bits,bits; jz refill  // Carry= next bit
        rep; ret

copy:  // In: len, %edi, dispq;  Out: 0==len, %edi, dispq;  trashes %eax, %edx
        lea (%edi,dispq),%eax; cmpl $5,len  // <=3 is forced
        movb (%eax),%dl; jbe copy1  // <=5 for better branch predict
        cmpl $-4,displ;   ja  copy1  // 4-byte chunks would overlap
        subl $4,len  // adjust for termination cases
copy4:
        movl (%eax),%edx; add $4,      %eax; subl $4,len
        movl %edx,(%edi); lea  4(%edi),%edi; jnc copy4
        addl $4,len; movb (%eax),%dl; jz copy0
copy1:
        inc %eax; movb %dl,(%edi); dec len
           movb (%eax),%dl
                lea 1(%edi),%edi;  jnz copy1
copy0:
        rep; ret

decompress:  // sections NRV2B, etc, are inserted here
    mov fx_src,src
    mov fx_dst,dst
    lodsl  // %eax= sz_unc; [lodsl ==> %eax= *src++;]
    mov fx_dstlen,%edx; mov %eax,(%edx); push %edx  // MATCH_51  *eof_dst = dstlen
    lodsl; xchg %eax,%ecx  // %ecx= sz_cpr(src)
    lodsl; movzbl %al,%edx  // method
    add src,%ecx; push %ecx  // MATCH_52  eof_src
    push dst  // MATCH_53 dst_orig
    push %ebp  // MATCH_54  sanity
// initialize NRV2 working registers
    xor bits,bits  // empty; force refill
    xor len,len  // create loop invariant
    or $(~0),disp  // -1: initial displacement
    cld
    movzbl -NBPW(src),%edx  // method
#define methb %dl

  section EXP_TAIL
eof:  // decompressors jump here when done
        pop %ebp  // MATCH_54
        pop %eax  // MATCH_53 dst_orig
        sub %eax,%edi  // dst -= original dst
        pop %eax  // MATCH_52 src_EOF
        pop %ecx  // MATCH_51 &dstlen
        movl %edi,(%ecx)  // actual length used at dst  XXX: 4GB
        sub %esi,%eax  // src -= eof;  // return 0: good; else: bad
        ret  // back into f_expand, with sane %ebp

#define M_NRV2B_LE32    2
#define M_NRV2D_LE32    5
#define M_NRV2E_LE32    8
#define M_CL1B_LE32     11
#define M_LZMA          14

  section NRV2E
#include "arch/i386/nrv2e_d32-easy.S"

  section NRV2D
#include "arch/i386/nrv2d_d32-easy.S"

  section NRV2B
#include "arch/i386/nrv2b_d32-easy.S"

#undef off
#undef len
#undef bits
#undef displ
#undef dispq

#undef src
#undef dst
#undef meth

// lzma code is written in intel syntax!
///* lzma has its own 'section's */
    .intel_syntax noprefix
#include "arch/i386/lzma_d.S"

