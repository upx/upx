/*  l_lx_elf32arm.S -- Linux program entry point & decompressor (Elf binary)
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2004 Laszlo Molnar
*  Copyright (C) 2000-2006 John F. Reiser
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
*  <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>
*
*  John F. Reiser
*  <jreiser@users.sourceforge.net>
*/

sz_Elf32_Ehdr = 13*4
sz_Elf32_Phdr =  8*4

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

PAGE_SHIFT= 12
PAGE_SIZE = -(~0<<PAGE_SHIFT)

// magic for the UPX linker
#define SECT(n)  .text 1; .asciz #n; .long n - _start; .text 0; n

#define BL(t) \
        .text 1; .long 0, bl##t - _start; .asciz #t; .long 0; \
        .text 0;  .byte 0, 0, 0; bl##t: .byte 0xeb

/*__ARMOS000__*/
_start: .globl _start
/* Get some pages: enough
   to duplicate the entire compressed PT_LOAD, plus 1 page, located just after
   the brk() of the _un_compressed program.  The address and length are pre-
   calculated by PackLinuxElf32arm::pack3(), and patched in at compress time.
*/
        adr r12,start_params
        ldmia r12!,{r0,r1, r8,r10}  @ ADRM,LENM, ADRC,CNTC
        mov r2,#PROT_READ | PROT_WRITE | PROT_EXEC
        mov r3,#MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS
        mvn r4,#0  @ -1; cater to *BSD for fd when MAP_ANON
        swi 0x009000c0  @ mmap64
        cmn r0,#4096
        bcs msg_SELinux
        mov r9,r0  @ destination for copy
copy:
        ldmia r8!,{r0,r1,r2,r3,r4,r5,r6,r7}; subs r10,r10,#1
        stmia r9!,{r0,r1,r2,r3,r4,r5,r6,r7}; bne copy

        mov lr,r9  @ dst for unfolded code
        sub r11,r9,r8  @ relocation amount
        ldmia r12!,{r6,r7,r8, r9,r10}  @ ADRU,LENU,JMPU, LENX,ADRX

        sub sp,sp,#4  @ space for sz_unc
        /*ldrb r4,b_method+cpr0*/
        mov r3,sp  @ &sz_unc
        mov r2,lr  @ dst
        ldr r1,sz_cpr    +cpr0
        adr r0,sz_b_info +cpr0
        add r5,r12,r11  @ relocated f_decompress
        add pc,r12,r11  @ goto there; return to unfolded code

start_params:
        .ascii "ADRM"  @ dst  for map
        .ascii "LENM"  @ len  for map

        .ascii "ADRC"  @ src  for copy
        .ascii "CNTC"  @ cnt  for copy: number of 32-byte blocks

        .ascii "ADRU"  @ addr for unmap; same as ADRM
        .ascii "LENU"  @ len  for unmap; same as LENM
        .ascii "JMPU"  @ trampoline for unmap

        .ascii "LENX"  @ total size of compressed data
        .ascii "ADRX"  @ &b_info of 1st compressed block (after moving)
f_decompress:
#include "armv4_n2e_d8.S"

        .subsection 2
msg_SELinux:
        mov r2,#L71 - L70  @ length
        adr r1,L70  @ message text
        mov r0,#2  @ fd stderr
        swi 0x00900004  @ write
die:
        mov r0,#127
        swi 0x00900001  @ exit
        b die

/* Temporary until we get the buildLoader stuff working ... */
        .ascii "\n$Id: UPX (C) 1996-2006 the UPX Team. "
        .asciz "All Rights Reserved. http://upx.sf.net $\n"

L70:
        .asciz "PROT_EXEC|PROT_WRITE failed.\n"
L71:
        .balign 4
cpr0:
        /* { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...} */

eof:
/*__XTHEENDX__*/

/*
vi:ts=8:et:nowrap
*/

