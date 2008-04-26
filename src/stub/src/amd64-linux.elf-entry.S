/*  amd64-linux.elf-entry.S -- Linux program entry point & decompressor (Elf binary)
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2008 Laszlo Molnar
*  Copyright (C) 2000-2008 John F. Reiser
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

SYS_mmap= 9  // 64-bit mode only!

PAGE_SHIFT= 12
PAGE_MASK= (~0<<PAGE_SHIFT)
PAGE_SIZE= -PAGE_MASK

M_NRV2B_LE32=2  // ../conf.h
M_NRV2D_LE32=5
M_NRV2E_LE32=8


  section ELFMAINX
_start: .globl _start
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

  section ELFMAINY
eof:
        pop %rcx  // &input_eof
        movq %rsi,%rax; subq %rcx,%rax  // src -= eof;  // return 0: good; else: bad
        pop %rdx;       subq %rdx,%rdi  // dst -= original dst
        pop %rcx;            movl %edi,(%rcx)  // actual length used at dst  XXX: 4GB
        pop %rbx; pop %rbp
        ret

/* These from /usr/include/asm-x86_64/unistd.h */
__NR_write =  1
__NR_exit  = 60

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

/* Decompress the rest of this loader, and jump to it.
   Map a page to hold the decompressed bytes.  Logically this could
   be done by setting .p_memsz for our first PT_LOAD.  But as of 2005-11-09,
   linux 2.6.14 only does ".bss expansion" on the PT_LOAD that describes the
   highest address.  [I regard this as a bug, and it makes the kernel's
   fs/binfmt_elf.c complicated, buggy, and insecure.]  For us, that is the 2nd
   PT_LOAD, which is the only way that linux allows to set the brk() for the
   uncompressed program.  [This is a significant kernel misfeature.]
*/
unfold:
        pop %rbx  // &b_info

/* Get some pages.  If small, then get 1 page located just after the end
   of the first PT_LOAD of the compressed program.  This will still be below
   all of the uncompressed program.  If large (>=3 MiB uncompressed), then enough
   to duplicate the entire compressed PT_LOAD, plus 1 page, located just after
   the brk() of the _un_compressed program.  The address and length are pre-
   calculated by PackLinuxElf64amd::defineSymbols().
*/
        movl $ ADRM,%edi  // XXX: 4GB
        push $ PROT_READ | PROT_WRITE | PROT_EXEC; pop %arg3
        movl $ LENM,%esi  // XXX: 4GB
        push $ MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS; pop %sys4
        subl %arg5l,%arg5l  //; subl %arg6l,%arg6l  // MAP_ANON ==> ignore offset
        push $ SYS_mmap; pop %rax
        syscall  // %rax= result; trashes %rcx,%r11 only
        cmpl %eax,%edi; jne msg_SELinux  // XXX: 4GB

/* Load the addresses and lengths.
   XXX: 2GB  Note that  PUSH $imm32      sign-extends to 64 bits.
   XXX: 4GB  Note that  MOVL $imm32,reg  zero-extends to 64-bits.
   (Use an temporary register to obtain 4GB range on PUSH constant.)
*/
        push $ JMPU  // for unmap in fold
        push $ ADRU  // for unmap in fold
        movl $ ADRC,%esi
        push $ LENU  // for unmap in fold
        push $ ADRX  // for upx_main
        push %r15  // LENX for upx_main

        movl %edi,%edx  //  ADRM
        subl %esi,%edx  // (ADRM - ADRC) == relocation amount
        je L80  // no copy
        addl      %edx,%ebp  // update &decompress
        //addl      %edx,%ebx  // copy==>no overlap; unfolded before overwrite
        movl %ebx,%ecx  // XXX: 4GB  [&b_info] beyond end of decompressor
        subl %esi,%ecx  // byte length
        shrl $3,%ecx  // qwords to copy
        cld
        rep; movsq
L80:
        xchgl %eax,%edi

/* Decompress the folded part of this stub, then execute it. */
        movl %ebx,%esi  // %arg2l= &b_info
        push %rax  // ret_addr after decompression
               xchgl %eax,%arg3l  // %arg3= dst for unfolding  XXX: 4GB
        lodsl; push %rax          // allocate slot on stack
               movq  %rsp,%arg4   // &len_dst ==> used by lzma for EOF
        lodsl; xchgl %eax,%arg1l  // sz_cpr  XXX: 4GB
        lodsl; movzbl %al,%arg5l  // b_method
              xchg %arg1l,%arg2l  // XXX: 4GB
        call *%rbp  // decompress
               pop %rcx  // discard len_dst
        ret

main:
////    int3  # uncomment for debugging
        pop %rbp  // &decompress
        movl -4-(ret_main - _start)(%rbp),%r15d  // sz_pack2: length before stub
        subl $ sz_Ehdr + 2*sz_Phdr + sz_l_info + sz_p_info,%r15d  // XXX: 4GB
        call unfold  // push &b_info
        // { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...}

/*__XTHEENDX__*/

/*
vi:ts=8:et:nowrap
 */

