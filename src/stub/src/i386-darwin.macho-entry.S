/*
;  i386-darwin.macho-entry.S -- program entry point & decompressor (i386 Mach-o)
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2022 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2022 Laszlo Molnar
;  Copyright (C) 2000-2022 John F. Reiser
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

NBPW= 4

//#include "arch/i386/macros.S"
        .altmacro
        .att_syntax
        .code32
.macro          section name
                .section \name
                .code32
.endm

// 2-byte bramches, even when destination is not known yet
.macro          jmps    target
                .byte   0xeb, \target - . - 1
.endm

.macro          jzs     target
                .byte   0x74, \target - . - 1
.endm

.macro          jnzs    target
                .byte   0x75, \target - . - 1
.endm

.macro          jcs     target
                .byte   0x72, \target - . - 1
.endm

.macro          jncs    target
                .byte   0x73, \target - . - 1
.endm

.macro          jnas    target
                .byte   0x76, \target - . - 1
.endm
#define         jbes    jnas

mlc_cmd = 0
  LC_SEGMENT= 0x01
mlc_cmdsize = 4

sz_Mach_header= 7*4
  mhdr_ncmds= 4*4
  mhdr_flags= 6*4
MH_PIE=0x200000

sz_Mach_segment= 2*NBPW + 16 + 4*NBPW + 4*4
  mseg_segname=  2*NBPW
  mseg_vmaddr=   2*NBPW + 16
  mseg_vmsize=   2*NBPW + 16 + NBPW
  mseg_initprot= 2*NBPW + 16 + (4*NBPW) + 4

  msec_addr=    2*16
  msec_size=    2*16 + NBPW

/*************************************************************************
// program entry point
// see glibc/sysdeps/amd64/elf/start.S
**************************************************************************/

MAP_FIXED =   0x10
MAP_PRIVATE = 0x02
MAP_ANON =  0x1000
PROT_NONE =    0
PROT_READ =    1
PROT_WRITE =   2
PROT_EXEC =    4
MAP_ANON_FD =  -1

SYSBASE= 0  // at runtime: 0xC0000
SYS_mmap     =0xc5 + SYSBASE
SYS_mprotect =0x4a + SYSBASE
SYS_munmap   =0x49 + SYSBASE
SYS_write    =   4 + SYSBASE

#define __c4(a,b,c,d) (((a)<<(0*8)) | ((b)<<(1*8)) | ((c)<<(2*8)) | ((d)<<(3*8)))
#define __c8(a,b,c,d,e,f,g,h) (__c4(a,b,c,d) | (__c4(e,f,g,h) << 32))

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

//0:    .word -0b + &Mach_header
//0:    .word -0b + l_info
section MACHMAINX
_start: .globl _start
//  int3
        call main  // push &f_exp
_start_end:

  section MACH_UNC
/* Returns 0 on success; non-zero on failure. */
decompress:  // (uchar const *src, size_t lsrc, uchar *dst, u32 &ldst, uint method)

/* Arguments according to calling convention */
#define src  %esi
#define lsrc INS
#define dst  %edi
#define ldst OUTS  /* Out: actually a reference: &len_dst */
#define meth PARM
#define methb PARM

// /*************************************************************************
// // C callable decompressor
// **************************************************************************/

// /* Offsets to parameters, allowing for {pusha + call} */
#define         O_INP   NBPW*(8+1)
#define         O_INS   NBPW*(8+2)
#define         O_OUTP  NBPW*(8+3)
#define         O_OUTS  NBPW*(8+4)
#define         O_PARAM NBPW*(8+5)

#define         INP     O_INP(%esp)
#define         INS     O_INS(%esp)
#define         OUTP    O_OUTP(%esp)
#define         OUTS    O_OUTS(%esp)
#define         PARM    O_PARAM(%esp)

M_NRV2B_LE32=2  // ../conf.h
M_NRV2D_LE32=5
M_NRV2E_LE32=8

  section NRV_HEAD

0:     .word 9f - 0b
#include "arch/i386/bxx.S"
9:

/* Working registers */
#define off  %eax  /* XXX: 2GB */
#define len  %ecx  /* XXX: 2GB */
#define bits %ebx
#define disp %ebp

        pusha
        movl INP,%esi  // hardware src for movsb, lodsb
        movl INS,%ecx  // srclen
        add %esi,%ecx; mov %ecx,INP  // src EOF
        movl OUTP,%edi  // hardware dst for movsb
        xorl bits,bits  // empty; force refill
        xorl len,len  // create loop invariant
        orl $(~0),disp  // -1: initial displacement
        jmp setup

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
        movl (%esi),bits; subl $-4,%esi; \
        adcl bits,bits; movb (%esi),%dl; \
0:
/* Same, but without prefetch (not useful for length of match.) */
#define jnextb0n jnextb0y
#define jnextb0y GETBIT; jnc
#define jnextb1n jnextb1y
#define jnextb1y GETBIT; jc
#define GETBIT \
        addl bits,bits; jnz 0f; \
        movl (%esi),bits; subl $-4,%esi; \
        adcl bits,bits; \
0:

/* rotate next bit into bottom bit of reg */
#define getnextbp(reg) GETBITp; adcl reg,reg
#define getnextb(reg)  getnextbp(reg)


getbit:
        addl bits,bits; jz refill  // Carry= next bit
        rep; ret
refill:
        movl (%esi),bits; subl $-4,%esi  // next 32 bits; set Carry
        adcl bits,bits  // LSB= 1 (CarryIn); CarryOut= next bit
        movb (%esi),%dl  // speculate: literal, or bottom 8 bits of offset
        rep; ret

copy:  // In: len, %edi, disp;  Out: 0==len, %edi, disp;  trashes %eax, %edx
        leal (%edi,disp),%eax; cmpl $5,len  // <=3 is forced
        movb (%eax),%dl; jbe copy1  // <=5 for better branch predict
        cmpl $-4,disp;   ja  copy1  // 4-byte chunks would overlap
        subl $4,len  // adjust for termination cases
copy4:
        movl (%eax),%edx; addl $4,      %eax; subl $4,len
        movl %edx,(%edi); leal  4(%edi),%edi; jnc copy4
        addl $4,len; movb (%eax),%dl; jz copy0
copy1:
        incl %eax; movb %dl,(%edi); decl len
            movb (%eax),%dl
                leal 1(%edi),%edi;  jnz copy1
copy0:
        rep; ret

setup:
        cld

  section NRV2E
#include "arch/i386/nrv2e_d32-easy.S"

  section NRV2D
#include "arch/i386/nrv2d_d32-easy.S"

  section NRV2B
#include "arch/i386/nrv2b_d32-easy.S"

/* lzma has its own 'section's */
        .intel_syntax noprefix
#include "arch/i386/lzma_d.S"
        .att_syntax

  section NRV_TAIL
/* NRV_TAIL is empty */

  section MACHMAINY
eof:
        subl OUTP,dst  // dst -= original dst
        movl OUTS,%ecx; movl dst,(%ecx)  // actual length used at dst  XXX: 4GB
        subl  INP,src  // src -= eof;  // return 0: good; else: bad
        movl      src,7*NBPW(%esp)  // %eax in 'popa'
        popa
        ret

end_decompress: .globl end_decompress

        /* IDENTSTR goes here */

  section MACHMAINZ
PAGE_SIZE= ( 1<<12)
PAGE_MASK= -PAGE_SIZE

GAP= 128  // > farthest prefetch;               must match ../../p_mach.cpp
NO_LAP= 64  // avoid overlap for folded loader; must match ../../p_mach.cpp

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

#define r_MHDR edi

main:
        pop %ebp  // &_start_end
        lea -2*NBPW + (_start - _start_end)(%ebp),%esi

        movzwl 0(%ebp),%eax  // displ to f_exp
        lea 2(%ebp),%ecx; push %ecx  // FUNF
        add %eax,%ebp  // &f_exp

        movl %esi,%edx
        movl %esi,%edi; lodsl; subl %eax,%r_MHDR
        movl %esi,%ecx; lodsl; subl %eax,%ecx  // ADRX= &{l_info; p_info; b_info}
// For the benefit of 'fold':
        push %ecx  // ADRX
        subl %ecx,%edx; push %edx  // LENX
        push %edi  // MHDR
        jmp unfold0

// Decompress the rest of this loader, and jump to it.
unfold:
        pop %ebx  // &{ b_info:{sz_unc, sz_cpr, 4{byte}}, compressed_fold...}
// Find __LINKEDIT
        lea sz_Mach_header(%r_MHDR),%esi
        sub %ecx,%ecx
L100:
        add %ecx,%esi  // next command
        movl mlc_cmdsize(%esi),%ecx
        cmpl $LC_SEGMENT,(%esi); jne L100
        cmpl $__c4('L','I','N','K'),2+mseg_segname(%esi); jne L100
        cmpl $__c4('E','D','I','T'),6+mseg_segname(%esi); jne L100
        movl mseg_vmaddr(%esi),%ecx  // arg1 for mprotect; dst= new unfold

// Unfold
        movl %ebx,%esi
        push $PROT_READ|PROT_EXEC  // arg3 for mprotect
        lodsl; push %eax           // arg2 for mprotect; .sz_unc == dstlen
        movl %esp,%edx  // remember &dstlen
                    push %ecx      // arg1 for mprotect; dst= new unfold

        push NBPW(%esi)   // arg5  b_method  (.b_method - .sz_cpr)
        push %edx         // arg4  &dstlen
        push %ecx         // arg3  dst
        lodsl; push %eax  // arg2  .sz_cpr
        lodsl; push %esi  // arg1  src
        call *%ebp        // decompress
        addl $NBPW*5,%esp  // 5 params to FEXP()

// PROT_EXEC
        pop  %edi  // arg1  dst= new unfold
        push %edi
        push %edi  // retaddr: dst is tail !!
                // Use the copy.
                // FIXME:  ADRX, LENX
                // rsp/ fd,ADRU,LENU,%entry,&Mach_header
        // FALL THROUGH to mprotect
mprotect:
        movb $SYS_mprotect,%al
L200:
        movzbl %al,%eax
        mov %esp,%ecx  // &{user_ra, arg1, arg2, ...}
        or $0xC0000,%eax
        call sysgo; jnc 0f
        or $~0,%eax  // mov %eax,errno
0:
        ret $3*NBPW  // always remove 3 arguments from stack
write:
        movb $SYS_write,%al; jmp L200
sysgo:
        pop %edx  // return address for sysenter
        .byte 0x0f,0x34  // sysenter

#define DEBUG 1
#if DEBUG  /*{*/
TRACE_BUFLEN=0x78
FD_STDERR=2
trace:  // push $__c4(...); call trace
        pusha; movl %esp,%esi  // input
        movl NBPW*(8+1)(%esp),%eax  // annotation
        subl $TRACE_BUFLEN,%esp
        movl %esp,%edi  // output

        pushl %eax; movb $'\n',%al; stosb; popl %eax
0:  // print annotation
        stosb; shr $8,%eax; jnz 0b

        pushl $8  // words per row
        popl %eax
L610:  // each word
        pushl %eax  // count
        movb $' ',%al; call trace_hex  // next word
        popl %eax; decl %eax; jnz L610

        movb $'\n',%al; stosb
        xchg %edi,%eax
        subl %esp,%eax  // count
        movl %esp,%ecx  // buf
        pushl %eax; pushl %ecx; push $FD_STDERR
        call write
        addl $TRACE_BUFLEN,%esp
        popa
        ret $NBPW  // remove annotation

trace_hex:
        stosb  // punctuation
        lodsl  // datum
        call 0f; .ascii "0123456789abcdef"; 0: pop %ebx
        push $8; pop %ecx  // iterations
1:
        rol $4,%eax; mov %eax,%edx
        andl $0xf,%eax; xlat; stosb; xchg %eax,%edx
        dec %ecx; jnz 1b
        ret

#endif  /*}*/

unfold0:
        call unfold
FOLD:
            // compressed fold_elf86 follows

/* vim:set ts=8 sw=8 et: */
