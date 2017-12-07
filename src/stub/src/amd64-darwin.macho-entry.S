/*
;  amd64-darwin.macho-entry.S -- program entry point & decompressor (amd64 Mach-o)
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2017 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2017 Laszlo Molnar
;  Copyright (C) 2000-2017 John F. Reiser
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

NBPW= 8

#include "arch/amd64/macros.S"
#include "arch/amd64/regs.h"

mlc_cmd = 0
  LC_SEGMENT_64= 0x19
mlc_cmdsize = 4

sz_Mach_header64= 8*4
  mhdr_ncmds= 4*4

sz_Mach_segment= 2*4 + 16 + 4*NBPW + 4*4
  mseg_segname=  2*4
  mseg_vmaddr=   2*4 + 16
  mseg_vmsize=   2*4 + 16 + NBPW
  mseg_initprot= 2*4 + 16 + (4*NBPW) + 4

  msec_addr=    2*16
  msec_size=    2*16 + NBPW

/*************************************************************************
// program entry point
// see glibc/sysdeps/amd64/elf/start.S
**************************************************************************/

MAP_FIXED =   0x10
MAP_PRIVATE = 0x02
MAP_ANON =  0x1000
PROT_READ =    1
PROT_WRITE =   2
PROT_EXEC =    4
MAP_ANON_FD =  -1

SYSBASE= 0x02000000
SYS_mmap     =0xc5 + SYSBASE
SYS_mprotect =0x4a + SYSBASE
SYS_munmap   =0x49 + SYSBASE

#define __c4(a,b,c,d) (((a)<<(0*8)) | ((b)<<(1*8)) | ((c)<<(2*8)) | ((d)<<(3*8)))
#define __c8(a,b,c,d,e,f,g,h) (__c4(a,b,c,d) | (__c4(e,f,g,h) << 32))

  section AMD64BXX
0:     .word 9f - 0b
#include "arch/amd64/bxx.S"
9:

// FYI: Following the env[] vector there is another vector apple[] of strings.
// Contents from one actual instance on MacOS 10.13 HighSierra:
//      "executable_path=<rooted_path>"
//      "pfz=0x7ffffff84000"
//      "stack_guard=0x850795b0f36900c2"
//      "malloc_entropy=0x94a87434eb9e2c1,0xf6814219485392e8"
//      "main_stack=0x7ffeefc00000,0x800000,0x7ffeebc00000,0x4000000"
//      "executable_file=0x1000008,0x2209ce"
// when %rsp was 0x7ffeefbffaf0.

// Notes:
// Command-line debugger from Xcode: lldb foo; "process launch -s"

section MACHMAINX
_start: .globl _start
//    int3
#define r_cmdp rbx
#define r_ncmds r12d
#define r_reloc r15
        lea -2*4+_start(%rip),%rbp; movl (%rbp),%eax; sub %rax,%rbp  // &Mach_header64
        push %rbp  // P_01  &Mach_header64 before argc
        mov mhdr_ncmds(%rbp),%r_ncmds
        lea sz_Mach_header64(%rbp),%r_cmdp  // ptr
L20:
        cmpl $LC_SEGMENT_64,mlc_cmd(%r_cmdp); jne L50
        cmpl $__c4('T','E','X','T'),2+mseg_segname(%r_cmdp); jne L40
        sub %arg1l,%arg1l  // 0 addr
        mov %arg1,%arg6   // 0 off_t
        lea -1(%arg1),%arg5  // MAP_ANON_FD
        mov $MAP_PRIVATE|MAP_ANON,%sys4l
        mov $PROT_WRITE|PROT_READ,%arg3l
        mov mseg_vmsize(%r_cmdp),%arg2
        movl sz_unc+FOLD(%rip),%eax
        add %rax,%arg2; mov %arg2,%r14
        mov $SYS_mmap,%eax; syscall
        mov %rax,%r_reloc  // vmaddr
        sub %rbp,%r_reloc  // reloc
        push %rax  // P_02

// Copy only compression data, not Macho headers
        movl msec_size + sz_Mach_segment(%r_cmdp),%ecx
        movq msec_addr + sz_Mach_segment(%r_cmdp),%rsi
        mov %rax,%rdi
        add $7,%ecx; shr $3,%ecx; rep movsq
        jmp L60  // break;

goto_clone:
        addq %r_reloc,(%rsp)  // retaddr += reloc
        ret
L40:  // not TEXT

L50:  // not LC_SEGMENT64
        mov mlc_cmdsize(%r_cmdp),%eax; add %rax,%r_cmdp
        sub $1,%r_ncmds; jne L20
L60:
        call main  // push &decompress
ret_main:

  section MACH_UNC
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
        addq $1,%rax; movb %dl,(%rdi); subl $1,len
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
        pop %rsi  // &{ b_info:{sz_unc, sz_cpr, 4{byte}}, compressed_fold...}

        pop %rcx  // P_02
        push %rdi; pop %arg3  // dst
        push %rdi  // P_03  entry addr to folded stub
        push %rcx  // P_04  &copied data
        lodsl; push %rax          // P_05  allocate slot on stack
               movq  %rsp,%arg4   // &dstlen ==> used by lzma for EOF
        lodsl; push %rax          // P_06  sz_cpr  XXX: 4GB
        lodsl; movzbl %al,%arg5l  // b_method
        push %rsi; pop %arg1      // src
               pop %arg2          // P_06  sz_cpr
        call *%rbp  // decompress(src, srclen, dst, &dstlen, b_info.misc)
          pop %rcx  // P_05  discard len_dst

        push $PROT_EXEC|PROT_READ; pop %arg3
        mov %r14,%arg2  // len
        pop %arg1  // P_04  &copied data
        mov $SYS_mprotect,%rax; syscall

        lea -4+ _start - ret_main(%rbp),%rbx  // &total_length for fold:
        ret  // P_03
main:
        pop %rbp  // &decompress
        call unfold
FOLD:
            // compressed fold_elf86 follows

/* vim:set ts=8 sw=8 et: */
