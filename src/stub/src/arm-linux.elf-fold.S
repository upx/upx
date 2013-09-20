@  arm-linux.elf-fold.S -- linkage to C code to process Elf binary
@
@  This file is part of the UPX executable compressor.
@
@  Copyright (C) 2000-2013 John F. Reiser
@  All Rights Reserved.
@
@  UPX and the UCL library are free software; you can redistribute them
@  and/or modify them under the terms of the GNU General Public License as
@  published by the Free Software Foundation; either version 2 of
@  the License, or (at your option) any later version.
@
@  This program is distributed in the hope that it will be useful,
@  but WITHOUT ANY WARRANTY; without even the implied warranty of
@  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@  GNU General Public License for more details.
@
@  You should have received a copy of the GNU General Public License
@  along with this program; see the file COPYING.
@  If not, write to the Free Software Foundation, Inc.,
@  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
@
@  Markus F.X.J. Oberhumer              Laszlo Molnar
@  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
@
@  John F. Reiser
@  <jreiser@users.sourceforge.net>
@

#define ARM_OLDABI 1
#include "arch/arm/v4a/macros.S"
#define bkpt .long 0xe7f001f0  /* reserved instr; Linux GNU eabi breakpoint */

sz_Elf32_Ehdr = 13*4
sz_Elf32_Phdr =  8*4
sz_l_info = 12
sz_p_info = 12
sz_b_info = 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

#ifndef DEBUG  /*{*/
#define DEBUG 0
#endif  /*}*/

@@ control just falls through, after this part and compiled C code
@@ are uncompressed.

#define OVERHEAD 2048
#define MAX_ELF_HDR 512

fold_begin:
/* In:
   r10= &b_info of first compressed block (after move)
    r9= total size of compressed data
    r5= f_decompress
    sp/ ADRU,LENU,sz_unc,crumb,MFLG,argc,argv...
*/
        ldmia sp!,{r3,r4,r6,r7,r8,r11}  @ ADRU,LENU,sz_unc,crumb,MFLG,argc
        mov r1,sp  @ old_argv
        sub sp,sp,#4096  @ buffer for readlink
        sub sp,sp,#2*4  @ room for added env ptr and text
        mov r0,sp  @ new_argv
        stmdb sp!,{r3,r4,r6,r7,r11}  @ ADRU,LENU,sz_unc,crumb,argc;  drop MFLG
SP_sz_unc= 2*4
SP_argc= 4*4

        add r11,r11,#1  @ (1+argc) for end of argv
.Larg:  @ copy argv
        ldr r2,[r1],#4; subs r11,r11,#1
        str r2,[r0],#4; bne .Larg

.Lenv:  @ copy env
        ldr r2,[r1],#4
        str r2,[r0],#4
        cmp r2,#0; bne .Lenv

        sub r4,r0,#4  @ added env ptr goes here
        str r2,[r0],#4  @ terminator after added ptr
        mov r3,r0  @ new &Elf32_auxv_t

.Laux:  @ copy auxv
        ldr r2,[r1,#4]  @ .a_val
        str r2,[r0,#4]
        ldr r2,[r1],#2*4  @ .a_type
        str r2,[r0],#2*4
        cmp r2,#0; bne .Laux

        str r0,[sp,#SP_sz_unc]  @ clobber sz_unc with 1+ &Elf32_auxv_t[AT_NULL@.a_type]

        str r0,[r4]  @ new env ptr
        ldr r2,space3eq
        str r2,[r0],#4  @ "   =" of new env var

        mov r2,#4096  @ buffer length
        mov r1,r0  @ buffer for readlink
        adr r0,proc_self_exe
        sub r2,r2,#1  @ room for null terminator
        bl readlink; cmn r0,#4096; bcs 0f  @ error
        mov r2,#0
        strb r2,[r1,r0]  @ null terminate pathname
0:

/* Construct arglist for upx_main */
        sub sp,sp,#MAX_ELF_HDR + OVERHEAD  @ alloca
        mov r4,r9  @ total size of compressed data
        ldr r9,[r10,#sz_cpr]  @ xi.size  of ELF headers
        mov r8,sp  @ xo.ptr
        ldr r7,[r10,#sz_unc]  @ xo.size
        adr r6,f_unfilter
        add r9,r9,#sz_b_info  @ for unpackExtent
        mov r11,#0  @ dynbase for ET_EXEC;  FIXME for ET_DYN
#if defined(ARMEL_DARWIN)  /*{*/
        ldr r12,4*1 + munmap
#elif defined(ARMEL_EABI4)  /*}{*/
        ldr r12,4*2 + munmap
#elif defined(ARM_OLDABI)  /*}{*/
        ldr r12,4*0 + munmap
#else  /*}{*/
        mov r12,#0
#endif  /*}*/
        stmdb sp!,{r3,r4,r5,r6,r7,r8,r9,r10,r11,r12}
        ldmia sp!,{r0,r1,r2,r3}
        bl upx_main
        mov lr,r0  @ entry address
        add sp,sp,#(10-4)*4
        add sp,sp,#MAX_ELF_HDR + OVERHEAD  @ un-alloca

#if 0  //{ make_hatch_arm() does it then
  @ __clear_cache(hatch, &hatch[2]);
        ldr r2,[sp,#2*4]  @ 1+ &Elf32_auxv_t[AT_NULL@.a_type]
        ldr r0,[r2,#4 -2*4]   @ Elf32_auxv_t[AT_NULL@.a_type].a_val
        add r1,r0,#2*4  @ len= 2 instructions  the "escape hatch"
        mov r2,#0
        do_sys7t2 __ARM_NR_cacheflush  @ scribble r7
#endif  //}

        ldmia sp!,{r0,r1,r2,r3}  @ ADRU,LENU,1+ &Elf32_auxv_t[AT_NULL@.a_type],crumb
        add r0,r0,r3  @ ADRU+=crumb
        sub r1,r1,r3  @ LENU-=crumb

#if DEBUG  /*{*/
        ldr r3,[r2,#4 -2*4]  @ Elf32_auxv_t[AT_NULL@.a_type].a_val
        ldr r4,[r3,#0]  @ 1st instr
        ldr r5,[r3,#4]  @ 2nd instr
#define TRACE_REGS r0-r12,r14,r15
        stmdb sp!,{TRACE_REGS}; mov r0,#4; bl trace
#endif  /*}*/
        mov r3,#0  @ clear registers: paranoia
        mov r4,#0
        mov r5,#0
        mov r6,#0

        mov r8,#0
        mov r9,#0
        mov r10,#0
        mov r11,#0

/* Heuristic cache flush: sweep contiguous range to force collisions and evictions. */
        sub r12,sp,#(1<<19)  @ limit: 1/2 MB more
sweep:
        ldr r7,[sp],#-(1<<5)  @ extend stack; read allocate 32 bytes
        str r7,[sp]  @ make it dirty
        ldr r7,[sp]  @ read alocate again in case dirtying caused COW split
        cmp r12,sp; blo sweep

        add sp,sp,#(1<<19)  @ pop stack

#if defined(ARMEL_DARWIN)  /*{*/
        mov r7,#0
        mov r12,#0xff & __NR_munmap
#elif defined(ARMEL_EABI4)  /*}{*/
        mov r12,#0
        mov r7, #0xff & __NR_munmap
#elif defined(ARM_OLDABI)  /*{*/
        mov r7,#0
        mov r12,#0
#endif  /*}*/
        ldr pc,[r2,#4 -2*4]  @ Elf32_auxv_t[AT_NULL@.a_type].a_val

space3eq:
        .ascii "   ="
proc_self_exe:
        .asciz "/proc/self/exe"
        .balign 4

f_unfilter:  @ (char *ptr, uint len, uint cto, uint fid)
        ptr  .req r0
        len  .req r1
        cto  .req r2  @ unused
        fid  .req r3

        t1   .req r2
        t2   .req r3

#ifndef FILTER_ID  /*{*/
#define FILTER_ID 0x50  /* little-endian */
#endif  /*}*/
        and fid,fid,#0xff
        cmp fid,#FILTER_ID  @ last use of fid
        movne pc,lr  @ no-op if not filter 0x50

        movs  len,len,lsr #2  @ word count
        cmpne ptr,#0
        moveq pc,lr  @ no-op if either len or ptr is 0

top_unf:
        sub len,len,#1
        ldr t1,[ptr,len,lsl #2]
        and t2,t1,#0x0f<<24
        cmp t2,   #0x0b<<24; bne tst_unf  @ not 'bl' subroutine call
        and t2,t1,#0xff<<24  @ all the non-displacement bits
        sub t1,t1,len  @ convert to word-relative displacement
        bic t1,t1,#0xff<<24  @ restrict to displacement field
        orr t1,t1,t2  @ re-combine
        str t1,[ptr,len,lsl #2]
tst_unf:
        cmp len,#0
        bne top_unf
        ret

#if DEBUG  /*{*/
TRACE_BUFLEN=512
trace:
        str lr,[sp,#(-1+ 15)*4]  @ return pc; [remember: sp is not stored]
        mov r4,sp  @ &saved_r0
        sub sp,sp,#TRACE_BUFLEN
        mov r2,sp  @ output string

        mov r1,#'\n'; bl trace_hex  @ In: r0 as label
        mov r1,#'>';  strb r1,[r2],#1

        mov r5,#3  @ rows to print
L600:  @ each row
        sub r0,r4,#TRACE_BUFLEN
        sub r0,r0,sp
        mov r0,r0,lsr #2; mov r1,#'\n'; bl trace_hex  @ which block of 8

        mov r6,#8  @ words per row
L610:  @ each word
        ldr r0,[r4],#4; mov r1,#' '; bl trace_hex  @ next word
        subs r6,r6,#1; bgt L610

        subs r5,r5,#1; bgt L600

        mov r0,#'\n'; strb r0,[r2],#1
        sub r2,r2,sp  @ count
        mov r1,sp  @ buf
        mov r0,#2  @ FD_STDERR
#if defined(ARMEL_EABI4)  /*{*/
        mov r7,#__NR_write
        swi 0
#else  /*}{*/
        swi __NR_write
#endif  /*}*/
        add sp,sp,#TRACE_BUFLEN
        ldmia sp!,{TRACE_REGS}

trace_hex:  // In: r0=val, r1=punctuation before, r2=ptr; Uses: r3, ip
        strb r1,[r2],#1  @ punctuation
        mov r3,#4*(8 -1)  @ shift count
        adr ip,hex
L620:
        mov r1,r0,lsr r3
        and r1,r1,#0xf
        ldrb r1,[ip, r1]
        strb r1,[r2],#1
        subs r3,r3,#4; bge L620
        ret
hex:
        .ascii "0123456789abcdef"
#endif  /*}*/
        .unreq ptr
        .unreq len
        .unreq cto
        .unreq fid

__NR_exit  =  1 + __NR_SYSCALL_BASE
__NR_read  =  3 + __NR_SYSCALL_BASE
__NR_write =  4 + __NR_SYSCALL_BASE
__NR_open  =  5 + __NR_SYSCALL_BASE
__NR_close =  6 + __NR_SYSCALL_BASE
__NR_unlink= 10 + __NR_SYSCALL_BASE
__NR_getpid= 20 + __NR_SYSCALL_BASE
__NR_brk   = 45 + __NR_SYSCALL_BASE
__NR_readlink=85+ __NR_SYSCALL_BASE


__NR_mmap2    = 192 + __NR_SYSCALL_BASE
__NR_mprotect = 125 + __NR_SYSCALL_BASE
__NR_munmap   =  91 + __NR_SYSCALL_BASE

__ARM_NR_BASE       = 0x0f0000 + __NR_SYSCALL_BASE
__ARM_NR_cacheflush = 2 + __ARM_NR_BASE

        .globl my_bkpt
my_bkpt:
        bkpt
        ret

        .globl exit
exit:
        do_sys __NR_exit

        .globl read
read:
        do_sys __NR_read; ret

        .globl write
write:
        do_sys __NR_write; ret

        .globl open
open:
        do_sys __NR_open; ret

        .globl close
close:
        do_sys __NR_close; ret

        .globl unlink
unlink:
        do_sys __NR_unlink; ret

        .globl getpid
getpid:
        do_sys __NR_getpid; ret

        .globl brk
brk:
        do_sys __NR_brk; ret

        .globl readlink
readlink:
        do_sys __NR_readlink; ret

        .globl munmap
munmap:
        do_sys __NR_munmap; ret

        .globl mprotect
mprotect:
        do_sys __NR_mprotect; ret

        .globl __clear_cache
__clear_cache:
        mov r2,#0
        do_sys2 __ARM_NR_cacheflush; ret

        .globl mmap
mmap:
        str r5,[sp,#-4]!; ldr r5,[sp,#4+4]
        str r4,[sp,#-4]!; ldr r4,[sp,#4+4]
        mov r5,r5,lsr #12  @ convert to page number
mmap_do:
        do_sys __NR_mmap2
        ldr r4,[sp],#4
        ldr r5,[sp],#4
        ret

bits_privanon= -4+ fold_begin  // entry stores: MAP_{PRIVATE|ANON}  QNX vs linux

mmap_privanon: .globl mmap_privanon
        ldr r12,bits_privanon  @ r12 === ip
        str r5,[sp,#-4]!; mov r5,#0  @ offset= 0
        str r4,[sp,#-4]!; mvn r4,#0  @ fd= -1
        orr r3,r3,r12  @ flags |= MAP_{PRIVATE|ANON}  [QNX vs Linux]
        b mmap_do


#if DEBUG  /*{*/

div10: .globl div10
        mov ip,r0  @ extra copy used at end
        sub r1,r1,r1  @ hi

        mov r2,r0  @ copy lo
        adds r0,r0,r0,lsl #3   @ 9*lo
        adc  r1,r1,r1,lsl #3   @ 9*hi + C
        add  r1,r1,r2,lsr #(32 - 3)  @ bits shifted from lo to hi

        mov r2,r0  @ copy lo
        adds r0,r0,r0,lsl #4
        adc  r1,r1,r1,lsl #4
        add  r1,r1,r2,lsr #(32 - 4)  @ * 0x99

        mov r2,r0  @ copy lo
        adds r0,r0,r0,lsl #8
        adc  r1,r1,r1,lsl #8
        add  r1,r1,r2,lsr #(32 - 8)  @ * 0x9999

        mov r2,r0  @ copy lo
        adds r0,r0,r0,lsl #16
        adc  r1,r1,r1,lsl #16
        add  r1,r1,r2,lsr #(32 - 16)  @ * 0x99999999

        subs r0,r0,ip,lsl #(32 - 1)  @ - * 0x80000000
        sbc  r1,r1,ip,lsr #1         @   * 0x19999999

        adds r0,r0,ip
        adc  r0,r1,#0  @ * 0x0.1999999a
        ret

#endif  /*}*/

@ vi:ts=8:et:nowrap

