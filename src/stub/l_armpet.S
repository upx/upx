/* l_armpet.S -- ARM/PE decompressor assembly startup (thumb mode)

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

// magic for the UPX linker
#define SECT(n)  .text 1; .asciz #n; .long n - _start; .text 0; n

#define BL(t) \
        .text 1; .long 0, bl##t - _start; .asciz #t; .long 0; \
        .text 0;  .byte 0, 0, 0; bl##t: .byte 0xf8

#define DEBUG 0

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
        stmfd   sp!, {r0 - r9, lr}

        @ touch all pages in ARM mode - this seems to be required
        @ otherwise the THUMB mode stuff fails

        adr     r3, SRC0
        ldmia   r3, {r5, r6, r7}       @ r5=src0, r6=slen, r7=dst0, r3=addr src0
        add     r5, r5, r6
.L01:
        ldr     r6, [r7]
        add     r7, r7, #4096
        cmp     r7, r5
        bls     .L01

        adr     ip, ProcessAll + 1
        mov     lr, pc
        bx      ip

        ldmfd   sp!, {r0 - r9, lr}
.Lstart_orig:
        ldr     ip, ENTR
        bx      ip

SRC0:   .ascii  "SRC0"                  @ start of compressed data
SRCL:   .ascii  "SRCL"                  @ compressed length
DST0:   .ascii  "DST0"                  @ start of uncompressed data
DSTL:   .ascii  "DSTL"                  @ uncompressed length
ENTR:   .ascii  "ENTR"                  @ original entry point
CSYN:   .ascii  "CSYN"                  @ pointer to CacheSync

        .thumb

ProcessAll:
        ldmia   r3!, {r0, r1, r2, r5, r6, r7} @ r0=src0, r1=slen, r2=dst0, r7=csyn
        sub     r3, #12
        mov     dst0, r2
        ldr     r7, [r7]
        mov     r6, #4                  @ parameter of CacheSync
        push    {r6, r7, lr}
        .align  2

        @@ uncompress/unfilter/imports/relocs are copied here by the upx linker

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

SECT(Unfilter_0x50):
        .align  2
        buffer  .req r0
        addval  .req r2
        bufend  .req r4

        ldr     buffer, FIBS
        mov     addval, #0
        ldr     bufend, FIBE
        mov     r5, #0x0f
        mov     r6, #0xff
        lsl     r6, #24
        mvn     r7, r6
.Luf50_0:
        cmp     buffer, bufend
        beq     .Luf50_ret

        ldrb    r3, [buffer, #3]
        and     r3, r5
        cmp     r3, #0x0b
        bne     .Luf50_1
        ldr     r3, [buffer]
        mov     r1, r3
        and     r1, r6
        sub     r3, addval
        and     r3, r7
        orr     r3, r1
        str     r3, [buffer]

.Luf50_1:
        add     buffer, #4
        add     addval, #1
        b       .Luf50_0

        .unreq  buffer
        .unreq  addval
        .unreq  bufend

        .align  2
FIBS:   .ascii  "FIBS"                  @ buffer start for filter
FIBE:   .ascii  "FIBE"                  @ buffer end for filter

.Luf50_ret:

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

SECT(Relocs):
        .align  2
        buffer  .req r0
        dest    .req r1
        addval  .req r2

        ldr     buffer, BREL
        mov     addval, dst0
        sub     dest, addval, #4

.Lreloc_loop:
        ldrb    r3, [buffer]
        add     buffer, #1
        cmp     r3, #0
        beq     .Lreloc_end
        cmp     r3, #0xf0
        blo     .Lreloc_add

        mov     r4, #0x0f
        and     r4, r3
        ldrb    r3, [buffer, #1]        @ get_le16
        lsl     r4, #8
        add     r4, r3
        ldrb    r3, [buffer]
        add     buffer, #2
        lsl     r4, #8
        add     r3, r4

.Lreloc_add:
        add     dest, r3
        mov     r5, #0

.Lread_be32:
        ldrb    r3, [dest, r5]
        lsl     r4, #8
        add     r4, r3
        add     r5, #1
        cmp     r5, #4
        bne     .Lread_be32

        add     r4, addval
        str     r4, [dest]
        b       .Lreloc_loop

        .align  2
BREL:   .ascii  "BREL"                  @ start of reloc info

.Lreloc_end:

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

SECT(Imports):
        .align  2
        imp     .req r4
        iat     .req r5
        dll     .req r6

        mov     r7, sp
        sub     sp, #508
        sub     sp, #508
        sub     sp, #508
        sub     sp, #508
        ldr     imp, BIMP
.Lhi_loop1:
        mov     r0, imp
        bl      get_le32
        beq     .Lhi_end

        ldr     r1, ONAM
        add     r0, r1
        mov     r1, sp
.Lhi_copyname:
        ldrb    r2, [r0]
        add     r0, #1
        strh    r2, [r1]
        add     r1, #2
        cmp     r2, #0
        bne     .Lhi_copyname

        mov     r0, sp
        bl      LoadLibraryW

        mov     dll, r0
        add     r0, imp, #4
        bl      get_le32
        mov     iat, dst0
        add     iat, r0
        add     imp, #8

.Lhi_gpa_loop:
        ldrb    r0, [imp]
        add     imp, #1
        cmp     r0, #1
        bmi     .Lhi_loop1
        bne     .Lhi_by_ord

        mov     r1, imp
.Lhi_by_name:
        ldrb    r0, [imp]
        add     imp, #1
        cmp     r0, #0
        bne     .Lhi_by_name
        b       .Lhi_call_gpa

.Lhi_by_ord:
        ldrb    r0, [imp]
        ldrb    r1, [imp, #1]
        add     imp, #2
        lsl     r1, #8
        add     r1, r0

.Lhi_call_gpa:
        mov     r0, dll
        bl      GetProcAddressA
        str     r0, [iat]
        add     iat, #4
        b       .Lhi_gpa_loop

        .unreq  iat
        .unreq  imp
        .unreq  dll

get_le32:                               @ optimized for size
        mov     r1, #3
.Lg0:
        ldrb    r3, [r0, r1]
        lsl     r2, r2, #8
        add     r2, r2, r3
        sub     r1, #1
        bpl     .Lg0
        mov     r0, r2                  @ sets the Z flag if zero
        bx      lr

GetProcAddressA:
        ldr     r3, GETP
.Lcallr3:
        ldr     r3, [r3]
        bx      r3

LoadLibraryW:
        ldr     r3, LLIB
        b       .Lcallr3

        .align  2
BIMP:   .ascii  "BIMP"                  @ start of import data
ONAM:   .ascii  "ONAM"                  @ start of dll names
GETP:   .ascii  "GETP"                  @ pointer to GetProcAddressA
LLIB:   .ascii  "LOAD"                  @ pointer to LoadLibraryW

.Lhi_end:
        mov     sp, r7

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

SECT(ProcessEnd):
        pop     {r0, r1, r2}            @ r0=4, r1=csyn, r2=lr
        mov     lr, r2
        bx      r1                      @ jump to CacheSync

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

#define XSECT(n)  .text 1; .asciz #n; .long n - _start; .text 0

SECT(dummy0):
        .align  2
        .arm
#include "arm_nrv2b_d8.S"

        XSECT   (go_thumb_n2b)

SECT(Call2B):
        BL      (go_thumb_n2b)

#undef wrnk
#undef GETBIT

SECT(dummy1):
        .align  2
        .arm
#include "arm_nrv2e_d8.S"

        XSECT   (thumb_nrv2e_d8)

SECT(Call2E):
        BL      (thumb_nrv2e_d8)

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

        .arm
SECT(eof):
        .text   1
        .long -1; .short eof - _start

