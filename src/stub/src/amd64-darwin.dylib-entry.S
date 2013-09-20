/*
;  amd64-darwin.dylib-entry.S -- shlib entry point & decompressor (amd64 Mach-o)
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
// We have been CALLed as a subroutine from dyld; C-language rules apply.
// -4*4+_start: .int32 offset(user_init_function)
// -3*4+_start: .int32 offset(&b_info of compressed Mach_headers)
// -2*4+_start: .int32 length(compressed __TEXT)
// -1*4+_start: .int32 total_length  # of preceding bytes in file
**************************************************************************/

section MACHMAINX
_start: .globl _start
L100: pause; jmp L100  // FIXME
    int3  // FIXME
        push %rax  // space for &user_init function
        push %rbp  // callee-save registers
        push %rbx
        call main  // push &decompress

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

#undef src
#undef lsrc
#undef dst
#undef ldst
#undef meth
#undef methb
        /* IDENTSTR goes here */

  section MACHMAINZ
SYS_mmap = 0x02000000 + 197
PAGE_SIZE= ( 1<<12)

sz_Mach_header= 7*4
mh_sizeofcmds=5*4

seg_vmaddr=2*4+16
seg_vmsize=4+seg_vmaddr
seg_filesize=2*4+seg_vmsize

sz_l_info=3*4
sz_p_info=3*4
sz_b_info=3*4
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

MAP_FIXED=     0x10
MAP_PRIVATE=   0x02
MAP_ANON=    0x1000
PROT_READ=      1
PROT_WRITE=     2
PROT_EXEC=      4


bad_mmap:
        hlt
        jmp bad_mmap
main:
        pop %rbp  # &decompress
        lea -4+ _start - decompress(%rbp),%rbx  # &total_length
        mov -1*4(%rbx),%eax  # length(compressed __TEXT)
        add $dy_top,%eax
        sub $decompress,%eax

        push %rax  # length for eventual munmap

        mov $0,%arg6l  # offset
        mov $0,%arg5l  # fd
        mov $MAP_ANON|MAP_PRIVATE,%sys4l
        mov $PROT_READ|PROT_WRITE,%arg3l
        mov %eax,%arg2l  # length
        sub %arg1l,%arg1l  # 0  addr
        mov $SYS_mmap,%eax
        syscall
        jc bad_mmap

        push %rax  # addr for eventual munmap

        // Copy interval [decompress, dy_top).
        mov %rbp,%rsi  # decompressor
        mov %rax,%rbp  # new location
        mov %rax,%rdi  # dst for decompressor
        mov $dy_top,%ecx
        sub $decompress,%ecx
        cld; rep movsb

        // Goto the copied dy_reloc.
        lea -(dy_top - dy_reloc)(%rdi),%rax
        jmp *%rax
dy_reloc:

        // Copy compressed __TEXT.
        push %rdi  # remember start of compressed __TEXT
        mov %rbx,%rdx  # &total_length
          mov -4*4(%rbx),%eax  # offset(user_init_function)
        sub (%rbx),%edx  # runtime base address
          add %rdx,%rax; mov %rax,(1+2+2)*4(%rsp)  # relocate &user_init_function
        mov -2*4(%rbx),%esi; add %rdx,%rsi
        mov -1*4(%rbx),%ecx
        rep movsb
        pop %rsi  # &b_info for Mach_header
        mov %rdx,%rdi  # runtime base address

        // Decompress __TEXT, but do not overwrite Mach_headers
        // in order to maintain consistency with dyld partial caching of them.
        // So, skip the first compressed block.
        lodsl; add %rax,%rdi  # sz_unc
        lodsl; add %rax,%rsi  # sz_cpr
        lodsl  # junk {b_method}
dy_uncpr:
        push %rsi; push %rdi  # save in case unfilter

        lodsl; test %eax,%eax; jz dy_done
          push %rax  // sz_uncompressed  (maximum dstlen for lzma)
        mov %rsp,%arg4  // &dstlen
        mov %rdi,%arg3  // dst
          add %rdi,%rax; push %rax  // next dst
        lodsl; mov %eax,%arg2l  // sz_compressed  (srclen)
        mov %rax,%rcx
        lodsl; mov %eax,%arg5l // last 4 bytes of b_info
        mov %rsi,%arg1  // &compressed __TEXT
          add %rsi,%rcx; push %rcx  // next src
        call *%rbp  // decompress(1=rdi=src, 2=rsi=srclen, 3=rdx=dst, 4=rcx=&dstlen, 5=r8=b_info.misc)
          pop %rsi  // next src
          pop %rdi  // next dst
          pop %rcx  // dstlen (junk)

        pop %rdx; pop %rax  # rdx= old dst; rax= old &b_info
        movzbl 1+ b_method(%rax),%arg4l  # ftid
          test %arg4l,%arg4l; je dy_uncpr  # no filter
        movzbl 2+ b_method(%rax),%arg3l  # cto8
#if 0
        mov sz_unc(%rax),%arg2l
#else
        // sz_unc == 0
        mov (%rax),%arg2l
#endif
        mov %rdx,%arg1    # dst
        call f_unfilter  # f_unfilter(1=rdi=dst, 2=rsi=dstlen, 3=rdx=cto8, 4=rcx=ftid)
        jmp dy_uncpr

SYS_munmap= 0x02000000 + 73
dy_done:
        pop %rax; pop %rcx  # discard
        mov $PAGE_SIZE,%arg2l  // length for munmap
        pop %arg1  // addr for munmap
        pop %rbx; pop %rbp  // saved registers
        mov $SYS_munmap,%eax
        lea -4(%rdi),%rdx  # steal some space at high end of __TEXT
        movl $0x90c3050f,(%rdx)  # syscall; ret; nop
        jmp *%rdx

#undef off
#undef len
#undef lenq
#undef bits
#undef disp

f_unfilter:
#include "arch/amd64/bxx.S"

dy_top:



// vi:ts=8:et:nowrap
