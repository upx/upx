/*
;  amd64-darwin.dylib-entry.S -- shlib entry point & decompressor (amd64 Mach-o)
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2023 Laszlo Molnar
;  Copyright (C) 2000-2023 John F. Reiser
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

#include "arch/amd64/macros.S"
#include "arch/amd64/regs.h"


/*************************************************************************
// We have been CALLed as a subroutine from dyld; C-language rules apply.
// init(%edi=argc, %rsi=argv, %rdx=envp, %rcx=apple, %r8=...)
// -4*4+_start: .int32 offset(user_init_function)
// -3*4+_start: .int32 offset(&b_info of compressed Mach_headers)
// -2*4+_start: .int32 segTEXT.vmsize
// -1*4+_start: .int32 total_length  # of preceding bytes in file
**************************************************************************/

section MACHMAINX
_start: .globl _start
        push %rax  // space for &user_init_fn
        push %rdi; push %rsi; push %rdx; push %rcx; push %r8  // args
        push %rbp  // callee-save registers
        push %rbx
        call main  // push &escape
escape:
        syscall
        pop %rcx; pop %rdx; pop %rsi; pop %rdi
        ret

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
#define displ %ebp
#define dispq %rbp

        movq src,%rsi  // hardware src for movsb, lodsb
        movq dst,%rdi  // hardware dst for movsb
        xor bits,bits  // empty; force refill
        xor len,len  // create loop invariant
        orq $(~0),dispq  // -1: initial displacement
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

copy:  // In: len, %rdi, dispq;  Out: 0==len, %rdi, dispq;  trashes %rax, %rdx
        leaq (%rdi,dispq),%rax; cmpl $5,len  // <=3 is forced
        movb (%rax),%dl; jbe copy1  // <=5 for better branch predict
        cmpq $-4,dispq;  ja  copy1  // 4-byte chunks would overlap
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
SYS_mmap =     0x2000000 + 197
SYS_mprotect = 0x2000000 + 0x4a
SYS_munmap =   0x2000000 + 73
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


main:
        pop %rbp  # &escape

// Get temp pages for compressed __TEXT and this stub
        lea -4*4 + _start - escape(%rbp),%rsi
        lodsl; xchg %eax,%edx  # offset(user_init_fn)
        lodsl; xchg %eax,%ecx  # offset(b_info)
        lodsl  # skip
        mov (%rsi),%eax  # #preceding bytes in file
        sub %rax,%rsi; push %rsi; pop %rbx  # our &Mach_header
        add %rbx,%rdx; mov %rdx,7*8(%rsp)  # reloc(user_init_fn)
        sub %ecx,%eax  # omit Mach_headers from copy
        add $4+dy_top,%eax; push %rax  # P_02  length(tmppag)
        add %rbx,%rcx;      push %rcx  # P_01  &b_info
        xchg %eax,%arg2l  # length

        xor %arg6,%arg6  # 0  offset
        or $~0,%arg5l  #  -1  fd
        mov $MAP_ANON|MAP_PRIVATE,%sys4l
        push $PROT_READ|PROT_WRITE; pop %arg3
        xor %arg1l,%arg1l  # 0  addr
        mov $SYS_mmap,%eax; syscall; jc bad_mmap

// Copy compressed__TEXT and this stub to temp pages
        pop %rsi  # P_01 &b_info  src
        pop %rcx; push %rcx  # P_02  length(tmppag)
        push %rax  # P_03  addr(tmppag)
        push %rax; pop %rdi  # dst
        add %rcx,%rax  # reloc(dy_top)
        sub %rsi,%rbp
        add %rdi,%rbp  # reloc(&escape)
        push %rdi  # P_05  reloc(&b_info)
        add $7,%ecx; shr $3,%ecx; rep movsq
        pop %rcx  # P_05 reloc(&b_info)

// Make temp pages executable, and go there
        pop %arg1  # P_03 src (tmppag)
        pop %arg2  # P_02 length(tmppag)
        push %arg2  # P_02 length(tmppag)
        push %arg1  # P_03 src(tmppag)
        push %rcx  # P_06 reloc(&b_info)
        push %rax  # P_04  reloc(dy_top)
        call mprot_RE

        pop %rax  # P_04 reloc(dy_top)
        add $dy_reloc - dy_top,%rax
        jmp *%rax

mprot_RW:
        push $PROT_READ|PROT_WRITE; jmp mprot
mprot_RE:
        push $PROT_READ|PROT_EXEC
mprot:
        mov $SYS_mprotect,%eax
mding:
        pop %arg3
        syscall; jc bad_mmap
        ret
bad_mmap:
        hlt
        jmp bad_mmap

dy_reloc:

// Make __TEXT writeable
        push %rbx; pop %arg1  # our &Mach_header
        mov -2*4 + _start - escape(%rbp),%arg2l
        call mprot_RW

        pop %rsi  # P_06  reloc(&b_info)
        push %rbx; pop %rdi  # our &Mach_header
        add $decompress - escape,%rbp

        // Decompress __TEXT, but do not overwrite Mach_headers
        // in order to maintain consistency with dyld partial caching of them.
        // So, skip the first compressed block.
        lodsl; add %rax,%rdi  # sz_unc
        lodsl; add %rax,%rsi  # sz_cpr
        lodsl  # junk {b_method}
dy_uncpr:
        push %rsi; push %rdi  # save in case unfilter

        lodsl; test %eax,%eax; jz dy_done
          push %rax  // P_09 sz_unc  (maximum dstlen for lzma)
        mov %rsp,%arg4  // &dstlen (%rcx)
          add %rdi,%rax; push %rax  // P_07 next dst
        lodsl; xchg %eax,%edx  // sz_cpr (srclen)
        lodsl; xchg %eax,%arg5l // last 4 bytes of b_info
        lea (%rsi,%rdx),%rax; push %rax  // P_08 next src
        push %rdx  // P_10 sz_cpr
        mov %rdi,%arg3  // %rdx dst
        mov %rsi,%arg1  // %rdi &compressed __TEXT
        pop %arg2  // P_10 sz_cpr (srclen)
        call *%rbp  // decompress(1=rdi=src, 2=rsi=srclen, 3=rdx=dst, 4=rcx=&dstlen, 5=r8=b_info.misc)
          pop %rsi  // P_08 next src
          pop %rdi  // P_07 next dst
          pop %rcx  // P_09 dstlen (junk)

        pop %rdi; pop %rcx  # rdi= old dst; rcx= old &b_info
        mov (%rcx),%esi  # sz_unc
        movzbl 2+ b_method(%rcx),%edx  # cto8
        movzbl 1+ b_method(%rcx),%ecx  # ftid
        test %ecx,%ecx; je dy_done
        call f_unfilter  # f_unfilter(1=rdi=dst, 2=rsi=dstlen, 3=rdx=cto8, 4=rcx=ftid)
        jmp dy_done

dy_done:
sz_Mach_header64 = 0x20
// Make __TEXT executable
        push %rbx; pop %arg1  # our &Mach_header
          mov escape - decompress(%rbp),%rax  # 8 bytes of instructions
          add $8+ 2*4 + sz_Mach_header64,%rbx  # &segname[8] after "__TEXT\0\0"
          mov %rax,(%rbx)
        mov -2*4 + _start - decompress(%rbp),%arg2l
        call mprot_RE

        pop %arg1  # P_03 tmppag
        pop %arg2  # P_02 len(tmppag)
        mov $SYS_munmap,%eax
        push %rbx; pop %rcx  # &hatch
        pop %rbx; pop %rbp  // saved registers
        pop %r8
        jmp *%rcx

#undef off
#undef len
#undef lenq
#undef bits
#undef displ
#undef dispq

f_unfilter:
#include "arch/amd64/bxx.S"

dy_top:

/* vim:set ts=8 sw=8 et: */
