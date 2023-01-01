@  arm-linux.elf-fold.S -- linkage to C code to process Elf binary
@
@  This file is part of the UPX executable compressor.
@
@  Copyright (C) 2000-2023 John F. Reiser
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
@  <markus@oberhumer.com>               <ezerotven+github@gmail.com>
@
@  John F. Reiser
@  <jreiser@users.sourceforge.net>
@

#define ARM_OLDABI 1
#include "arch/arm/v4a/macros.S"
#define bkpt .long 0xe7f001f0  /* reserved instr; Linux GNU eabi breakpoint */

sz_Elf32_Ehdr = 13*4
sz_Elf32_Phdr =  8*4
p_vaddr= 4+4
sz_l_info = 12
sz_p_info = 12
sz_b_info = 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

MAP_ANONYMOUS= 0x20
MAP_PRIVATE=   0x02
MAP_FIXED=     0x10

PROT_READ=     0x1

O_RDONLY=       0

PAGE_SHIFT= 12
PAGE_SIZE = -(~0<<PAGE_SHIFT)
PATHMAX=  4096

PATH_MAX= 4096

#ifndef DEBUG  /*{*/
#define DEBUG 0
#define TRACE_REGS r0-r12,r14,r15
#endif  /*}*/

#define OVERHEAD 2048
#define MAX_ELF_HDR 512
#define call bl

mflg_data: .int MAP_PRIVATE|MAP_ANONYMOUS  @ overwritten for QNX vs Linux

SP_fd= 3*4
/* In:
   r4= LENX
   r5= ADRX
   lr= "/proc/self/exe"
new sp/            elfaddr,fd, ADRU,LENU, f_exp,%entry, argc,argv,0,envp,0,auxv
        (ADRX,LENX) = extent of compressed program
        (ADRU,LENU) = params to munmap unfolded stub
*/
fold_begin:  // enter here
#if DEBUG  //{
#define TRACE_REGS r0-r12,r14,r15
        mov r0,sp  @ current stack pointer (extra clue)
        stmdb sp!,{TRACE_REGS}; mov r0,#0x10; bl trace
#endif  //}
        ldmia sp!,{      r6,r7,r8,r9,r10,r11,r12}  @ r12= argc
        mov r1,sp  @ src (argv)
        sub sp,sp,#PATH_MAX
        mov r0,sp  @ dst
        stmdb sp!,{r4,r5,r6,r7,r8,r9,r10,r11,r12}  @ r12= argc
F_LENX=    0*4
F_ADRX=    1*4
F_elfaddr= 2*4
F_fd=      3*4
F_ADRU=    4*4
F_LENU=    5*4
F_f_exp=   6*4
  F_e_auxv=  F_f_exp
F_entry=   7*4

0: // copy argv down
        ldr r3,[r1],#4; cmp r3,#0
        str r3,[r0],#4; bne 0b
     mov r4,r0  @ &new_env[0]
        str r3,[r0],#4  @ space for new_env[0]

0: // copy env down
        ldr r3,[r1],#4; cmp r3,#0
        str r3,[r0],#4; bne 0b
     mov r5,r1  @ &orig_auxv[0]

0: // copy auxv down
        ldmia r1!,{r2,r3}; cmp r2,#0
        stmia r0!,{r2,r3}; bne 0b
     mov r6,r1  @ &orig_auxv[end]; also &old_strings
     mov r9,r0  @ &down_auxv[end]

        mov r2,#PATH_MAX
        mov r1,r0  @ buffer
        mov r0,lr  @ arg1= "/proc/self/exe"
        sub r2,r2,#1  @ room for null terminator
        call readlink
        cmn r0,#4096
        movcs r1,lr  @ "/proc/self/exe"
        movcs r0,#14  @ strlen("/proc/self/exe")
link_ok:
        add r2,r1,r0  @ end
        mov r0,r6  @ &old_strings
        mov  r3,#0;        strb r3,[r0,#-1]!  @ terminate link name
0: // copy backwards to beginning
        ldrb r3,[r2,#-1]!; strb r3,[r0,#-1]!
        cmp r1,r2; bne 0b
        mov  r3,#'=';      strb r3,[r0,#-1]!
        mov  r3,#' ';      strb r3,[r0,#-1]!
                           strb r3,[r0,#-1]!
                           strb r3,[r0,#-1]!
        str r0,[r4]  @ new_env[0]

// preserve 8-byte alignment of stack pointer
        mov r0,r0,lsr #2
        mov r0,r0,lsl #2  @ &new_strings
        eor r3,r6,r0  @ word parity with &old_strings
        and r3,r3,#4  @ 0 or 4
        eor r3,r3,#4  @ we add 1 new_env[0]
        sub r0,r0,r3  @ align mod 8

// copy up auxv,env,argv
        sub r3,r0,r6  @ &new_auxv[end] - &orig_auxv[end]
        add r3,r3,r5  @ &new_auxv[0] = delta +  &orig_auxv[0]
        mov r1,r9  @ &down_auxv[end]
        ldr r5,[sp,#F_f_exp]  @ save f_exp
        str r0,[sp,#F_e_auxv]  @ replace f_exp with &new_auxv[end]
0:
        ldr r2,[r1,#-4]!; cmp r1,sp
        str r2,[r0,#-4]!; bne 0b
        mov sp,r0

#if DEBUG  //{
        stmdb sp!,{TRACE_REGS}; mov r0,#0x12; bl trace
#endif  //}

/* Construct arglist for upx_main */
        @ldr r5,[sp,#F_f_exp]
        ldmia sp!,{r4,r10,r11}  @ LENX, ADRX, elfaddr
F_delta= 3*4
        sub sp,sp,#MAX_ELF_HDR + OVERHEAD  @ alloca
        ldr r9,[r10,#sz_cpr]  @ xi.size  of ELF headers
        mov r8,sp  @ xo.ptr
        ldr r7,[r10,#sz_unc]  @ xo.size
        adr r6,f_unfilter
        //mov r3,r3  @ auxv
        add r9,r9,#sz_b_info  @ for unpackExtent

        stmdb sp!,{r3,r4,r5,r6,r7,r8,r9,r10,r11}
        ldmia sp!,{r0,r1,r2,r3}
#if DEBUG  //{
        stmdb sp!,{TRACE_REGS}; mov r0,#0x13; bl trace
#endif  //}
// r0=av; r1=sz_cpr; r2=f_decompress; r3=f_unfilter;
// xo={sz_unc, &tmp_ehdr}, xi={sz_cpr, &b_info}, elfaddr
        call upx_main
        add sp,sp,#(9-4)*4
        add sp,sp,#MAX_ELF_HDR + OVERHEAD  @ un-alloca
#if DEBUG  //{
        stmdb sp!,{TRACE_REGS}; mov r0,#0x14; bl trace
#endif  //}
        str r0,[sp,#F_entry - F_delta]  @ entry address

// Discard pages of compressed input data (includes [ADRC,+LENC) )
        ldr r1,[r11,#p_vaddr+sz_Elf32_Phdr+sz_Elf32_Ehdr]  // Phdr[C_TEXT= 1].p_vaddr
        ldr r2,[r11,#p_vaddr              +sz_Elf32_Ehdr]  // Phdr[C_BASE= 0].p_vaddr
        mov r0,r11  // hi &Elf32_Ehdr
        sub r1,r1,r2
        call munmap  // discard C_TEXT compressed data

// Map 1 page of /proc/self/exe so that it does not disappear
        ldr r4,[sp],#4  @ pop r4,F_fd
        mov r5,#0  @ SEEK_SET offset
        stmdb sp!,{r4,r5}  @ arg5,arg6 calling convention
        mov r3,#MAP_PRIVATE
        mov r2,#PROT_READ
        mov r1,#PAGE_SIZE
        mov r0,#0  @ any address
        call mmap  @ no error check: cannot recover
        ldmia sp!,{r0,r1}  @ fd, offset
        call close

#if DEBUG  //{
        stmdb sp!,{TRACE_REGS}; mov r0,#0x15; bl trace
#endif  //}
        ldmia sp!,{r0,r1,r2, lr}  @ ADRU,LENU,1+ &Elf32_auxv_t[AT_NULL@.a_type], entry
        // crumb is unused: replaced by mapping /proc/self/exe into a free page

#if DEBUG  /*{*/
        ldr r3,[r2,#4 -2*4]  @ Elf32_auxv_t[AT_NULL@.a_type].a_val
        ldr r4,[r3,#0]  @ 1st instr
        ldr r5,[r3,#4]  @ 2nd instr
        stmdb sp!,{TRACE_REGS}; mov r0,#0x15; bl trace
#endif  /*}*/
        mov r3,#0  @ clear registers: paranoia
        mov r4,#0
        mov r5,#0
        mov r6,#0

        mov r8,#0
        mov r9,#0
        mov r10,#0
        mov r11,#0

#if 1|DEBUG  //{
/* Heuristic cache flush: sweep contiguous range to force collisions and evictions. */
        sub r12,sp,#(1<<18)  @ limit: 1/4 MB more
sweep:
        ldr r7,[sp],#-(1<<5)  @ extend stack; read allocate 32 bytes
        str r7,[sp]  @ make it dirty
        ldr r7,[sp]  @ read alocate again in case dirtying caused COW split
        cmp r12,sp; blo sweep

        add sp,sp,#(1<<18)  @ pop stack
#endif  //}

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

proc_self_exe:
        .ascii "/proc/self/exe"  @ no terminator
proc_self_align:
        .asciz ""  @ terminator
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
        bkpt  // my_bkpt
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
        stmdb sp!,{r4,r5,lr}
        ldr r5,[sp,#4*4]
        ldr r4,[sp,#3*4]
        mov r5,r5,lsr #12  @ convert to page number
mmap_do:
        do_sys __NR_mmap2
        ldmia sp!,{r4,r5,pc}

get_sys_munmap: .globl get_sys_munmap  // r0= system call instruction
#if defined(ARMEL_DARWIN)  /*{*/
        ldr r0,4*1 + munmap
#elif defined(ARMEL_EABI4)  /*}{*/
        ldr r0,4*2 + munmap
#elif defined(ARM_OLDABI)  /*}{*/
        ldr r0,4*0 + munmap
#else  /*}{*/
        mov r0,#0
#endif  /*}*/
        ret

mmap_privanon: .globl mmap_privanon
        stmdb sp!,{r4,r5,lr}
        ldr r4,mflg_data  @ Map_PRIVATE|MAP_ANON for Linux; MAP_PRIVANON for QNX
        mov r5,#0  @ offset= 0
        orr r3,r3,r4  @ combine with input (such as MAP_FIXED)
        mvn r4,#0  @ fd= -1
        b mmap_do

#if 1|DEBUG  /*{*/

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

/* vim:set ts=8 sw=8 et: */
