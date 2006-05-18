/* l_armpea.S -- ARM/PE decompressor assembly startup (arm mode)

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2006 Laszlo Molnar
   Copyright (C) 2000-2006 John F. Reiser
   All Rights Reserved.

   UPX and the UCL library are free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer              Laszlo Molnar
   <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>

   John F. Reiser
   <jreiser@users.sourceforge.net>
*/

// DEBUG == 0 -> none
// DEBUG == 1 -> for armpe_tester
// DEBUG == 2 -> wince - dumps memory
// DEBUG == 3 -> wince - removes files

#define DEBUG 0
#if DEBUG == 0
# define DINIT
# define DDUMP(x)
# define DDONE
#else
 #define DDUMP(x)   stmfd sp!, {r0 - r3}; mov  r0, x; mov lr, pc; mov pc, r8; ldmia sp!, {r0 - r3}
 #if DEBUG == 1
 #define DINIT      mov  r8, r0
 #define DDONE      mov  pc, lr
 #elif DEBUG == 2
 #define DINIT      adr  r8, writefile
 #define DDONE
 #elif DEBUG == 3
 #define DINIT      adr  r8, DelFile
 #define DDONE
 #endif
#endif

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

// magic for the UPX linker
#define SECT(n)  .text 1; .asciz #n; .long n - _start; .text 0; n

#define BL(t) \
        .text 1; .long 0, bl##t - _start; .asciz #t; .long 0; \
        .text 0;  .byte 0, 0, 0; bl##t: .byte 0xeb

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

        .text
        .balign  0
        .globl  _start
        .arm
_start:

        dst0    .req r9                 @ global register

SECT(DllStart):
        cmp     r1, #1
        bne     .Lstart_orig

SECT(ExeStart):
        stmfd   sp!, {r0 - r11, lr}
        DINIT
        adr     r3, SRC0
        ldmia   r3!, {r0, r1, r2}       @ r0=src0, r1=slen, r2=dst0, r3=addr dstl
        ldmia   r3, {r4, r10, r11}      @ r10=LoadLibraryW, r11=GetProcAddressA
        mov     dst0, r2

        bl      ProcessAll
        mov     r0, #4
        bl      CacheSync
        ldmia   sp!, {r0 - r11, lr}
        DDONE
.Lstart_orig:
        ldr     pc, ENTR

CacheSync:
        ldr     pc, IATT + 8

SRC0:   .ascii  "SRC0"                  @ start of compressed data
SRCL:   .ascii  "SRCL"                  @ compressed length
DST0:   .ascii  "DST0"                  @ start of uncompressed data
DSTL:   .ascii  "DSTL"                  @ uncompressed length
IATT:   .ascii  "IATT"; .long 0, 0, 0   @ import address table
ENTR:   .ascii  "ENTR"                  @ original entry point

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

#if DEBUG == 2
writefile:
        stmfd   sp!, {r4, r5, r6, r7, lr}
        mov     r1, dst0
        sub     r2, r8, r1
        add     r2, r2, #4096+2048
        mov     r3, #2
        sub     sp, sp, #24
        mov     r4, #0
        strh    r0, [sp, #18]
        str     r3, [sp, #0]
        mov     r0, #128
        mov     r3, #92
        str     r0, [sp, #4]
        mov     r6, r1
        mov     r7, r2
        strh     r3, [sp, #16]
        strh     r4, [sp, #20]
        mov     r3, r4
        str     r4, [sp, #8]
        mov     r1, #1073741824
        mov     r2, #3
        add     r0, sp, #16
        ldr     ip, .L3
        mov     lr, pc
        mov     pc, ip
        add     r3, sp, #12
        mov     r5, r0
        str     r4, [sp, #0]
        mov     r1, r6
        mov     r2, r7
        ldr     ip, .L3+4
        mov     lr, pc
        mov     pc, ip
        mov     r0, r5
        ldr     r3, .L3+8
        mov     lr, pc
        mov     pc, r3
        add     sp, sp, #24
        ldmfd   sp!, {r4, r5, r6, r7, pc}
.L3:
        .word   33135704
        .word   33135968
        .word   33137392
#endif

#if DEBUG == 3
DelFile:
        adr     r1, filename
        strb    r0, [r1, #2]
        mov     r0, r1
        ldr     pc, deleteffilew

deleteffilew:
        .word   0x1f99bc8
filename:
        .byte   '\\', 0, 'r', 0, 0, 0
        .align  2
#endif

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

ProcessAll:
        stmfd   sp!, {lr}

        @@ uncompress/unfilter/imports/relocs are copied here by the upx linker

SECT(ProcessEnd):
        ldmia   sp!, {pc}


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

SECT(Unfilter_0x50):
        buffer  .req r0
        addval  .req r2
        bufend  .req ip

        ldr     buffer, FIBS
        mov     addval, #0
        ldr     bufend, FIBE
.Luf50_0:
        cmp     buffer, bufend
        beq     .Luf_end

        ldr     r3, [buffer]
        and     r1, r3, #0x0f000000
        cmp     r1, #0x0b000000
        bne     .Luf50_1
        and     r1, r3, #0xff000000
        sub     r3, r3, addval
        and     r3, r3, #0x00ffffff
        orr     r3, r3, r1
        str     r3, [buffer]

.Luf50_1:
        add     buffer, buffer, #4
        add     addval, addval, #1
        b       .Luf50_0

        .unreq  buffer
        .unreq  addval
        .unreq  bufend

FIBS:   .ascii  "FIBS"                  @ buffer start for filter
FIBE:   .ascii  "FIBE"                  @ buffer end for filter

.Luf_end:
        DDUMP   (#'F')

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

SECT(Relocs):
        dest    .req r0
        buffer  .req r1
        addval  .req dst0

        ldr     buffer, BREL
        sub     dest, addval, #4

.Lreloc_loop:
        ldrb    r3, [buffer], #1
        cmp     r3, #0
        beq    .Lreloc_end
        cmp     r3, #0xf0

        bichs   ip, r3, #0xf0
        ldrhsb  r3, [buffer, #1]        @ get_le16
        addhs   ip, r3, ip, lsl #8
        ldrhsb  r3, [buffer], #2
        addhs   r3, r3, ip, lsl #8

        add     dest, dest, r3
        ldrb    r3, [dest]              @ get_be32
        add     ip, r3, ip, lsl #8
        ldrb    r3, [dest, #1]
        add     ip, r3, ip, lsl #8
        ldrb    r3, [dest, #2]
        add     ip, r3, ip, lsl #8
        ldrb    r3, [dest, #3]
        add     ip, r3, ip, lsl #8
        add     ip, ip, addval
        str     ip, [dest]
        b       .Lreloc_loop

BREL:   .ascii  "BREL"                  @ start of reloc info

        .unreq  buffer
        .unreq  addval
        .unreq  dest

.Lreloc_end:
        DDUMP   (#'R')

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

SECT(Imports):
        imp     .req r4
        iat     .req r5
        dll     .req r6
        .equ    bufsize, 2048

        sub     sp, sp, #bufsize
        ldr     imp, BIMP
.Lhi_loop1:
        mov     r0, imp
        bl      get_le32
        beq     .Lhi_end

        ldr     r1, ONAM
        add     r0, r0, r1
        mov     r1, sp
.Lhi_copyname:
        ldrb    r2, [r0], #1
        strh    r2, [r1], #2
        cmp     r2, #0
        bne     .Lhi_copyname

        mov     r0, sp
        bl      LoadLibraryW

        mov     dll, r0
        add     r0, imp, #4
        bl      get_le32
        add     iat, dst0, r0
        add     imp, imp, #8

.Lhi_gpa_loop:
        ldrb    r0, [imp], #1
        cmp     r0, #1
        bmi     .Lhi_loop1
        bne     .Lhi_by_ord

        mov     r1, imp
.Lhi_by_name:
        ldrb    r0, [imp], #1
        cmp     r0, #0
        bne     .Lhi_by_name
        b       .Lhi_call_gpa

.Lhi_by_ord:
        ldrb    r0, [imp], #1
        ldrb    r1, [imp], #1
        add     r1, r0, r1, lsl #8

.Lhi_call_gpa:
        mov     r0, dll
        bl      GetProcAddressA
        str     r0, [iat], #4
        b       .Lhi_gpa_loop

        .unreq  iat
        .unreq  imp
        .unreq  dll

get_le32:                               @ optimized for size
        mov     r2, #3
.Lg0:
        ldrb    r3, [r0, r2]
        subs    r2, r2, #1
        add     r1, r3, r1, asl #8
        bpl     .Lg0
        movs    r0, r1                  @ set the Z flag if zero
        mov     pc, lr

LoadLibraryW:
        mov     pc, r10

GetProcAddressA:
        mov     pc, r11

BIMP:   .ascii  "BIMP"                  @ start of import data
ONAM:   .ascii  "ONAM"                  @ start of dll names

.Lhi_end:
        add     sp, sp, #bufsize
        DDUMP   (#'I')

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

SECT(ucl_nrv2e_decompress_8):

#include "armv4_n2e_d8.S"

SECT(Call2E):
        BL      (ucl_nrv2e_decompress_8)
        DDUMP   (#'C')

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

SECT(UPX1HEAD):
        .byte   85,80,88,33     @  0  UPX_MAGIC_LE32
        .byte   161,216,208,213 @  4  UPX_MAGIC2_LE32
        .long   0               @  8  uncompressed adler32
        .long   0               @ 12  compressed adler32
        .long   0               @ 16  uncompressed len
        .long   0               @ 20  compressed len
        .long   0               @ 24  original file size
        .byte   0               @ 28  filter id
        .byte   0               @ 29  filter cto
        .byte   0               @  unused
        .byte   45              @ 31  header checksum

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

SECT(eof):
        .text   1
        .long   -1; .short eof - _start
