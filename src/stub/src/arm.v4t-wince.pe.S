/* arm.v4t-wince.pe.S -- ARM/PE decompressor assembly startup (thumb mode)

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
   Copyright (C) 2000-2008 John F. Reiser
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
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>

   John F. Reiser
   <jreiser@users.sourceforge.net>
*/

#include "arch/arm/v4t/macros.S"

// p_armpe.cpp uses some symbols, so they should be global

        .globl SRC0
        .globl DST0
        .globl IATT
        .globl ENTR

        .arm

        dst0    .req r9                 @ global register

section DllStart
        cmp     r1, #1
        bne     .Lstart_orig

section ExeStart
        stmfd   sp!, {r0 - r11, lr}

        @ access all pages in ARM mode - this seems to be required
        @ otherwise the THUMB mode stuff fails

        adr     r3, SRC0
        ldmia   r3, {r5, r6, r7, r9, r10, r11, ip}  @ r7=dst0
        add     r5, pc, #4096           @ r3=addr src0, r10=LoadLibraryW
.L01:                                   @ r11=GetProcAddressA, ip=CacheSync
        ldr     r6, [r7]
        add     r7, r7, #4096
        cmp     r7, r5
        bls     .L01

        adr     r4, ProcessAll + 1
        mov     lr, pc
        bx      r4

        ldmfd   sp!, {r0 - r11, lr}
.Lstart_orig:
        ldr     ip, ENTR
        bx      ip

SRC0:   .long   start_of_compressed
SRCL:   .long   compressed_length
DST0:   .long   start_of_uncompressed
DSTL:   .long   uncompressed_length
IATT:   .long   0, 0, 0, 0
ENTR:   .long   original_entry

        .thumb

ProcessAll:
        ldmia   r3!, {r0, r1, r2}       @ r0=src0, r1=slen, r2=dst0, r3=addr dstl
        mov     dst0, r2
        mov     r4, ip                  @ CacheSync
        push    {r4, lr}

        @@ uncompress/unfilter/imports/relocs are copied here by the upx linker

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

section Unfilter_0x50
        buffer  .req r0
        addval  .req r2
        bufend  .req r4

        ldr     buffer, .LFIBS
        mov     addval, #0
        ldr     bufend, .LFIBE
        mov     r5, #0x0f
        mov     r6, #0xff
        lsl     r6, #24
        mvn     r7, r6
.Luf50_0:
        cmp     buffer, bufend
        beq     .Luf50_ret

        ldr     r3, [buffer]
        lsr     r1, r3, #24
        and     r1, r5
        cmp     r1, #0x0b
        bne     .Luf50_1
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
S(FIBS):.long   filter_buffer_start
S(FIBE):.long   filter_buffer_end

.Luf50_ret:

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

section Relocs
        buffer  .req r0
        dest    .req r1
        addval  .req r2

        ldr     buffer, .LBREL
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
S(BREL):.long   start_of_relocs

.Lreloc_end:

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

section Imports
        imp     .req r4
        iat     .req r5
        dll     .req r6

.real_start_ofImports:          // suppress silly warnings
        mov     r7, sp
        sub     sp, #508
        sub     sp, #508
        sub     sp, #508
        sub     sp, #508
        ldr     imp, .LBIMP
.Lhi_loop1:
        mov     r0, imp
        bl      get_le32
        beq     .Lhi_end

        ldr     r1, .LONAM
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
        stmia   iat!, {r0}
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

LoadLibraryW:
        bx      r10

GetProcAddressA:
        bx      r11

S(BIMP):.long   start_of_imports
S(ONAM):.long   start_of_dll_names

.Lhi_end:
        mov     sp, r7

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

section ProcessEnd
        pop     {r1, r2}                @ r1=CacheSync, r2=lr
        mov     r0, #4                  @ parameter of CacheSync
        mov     lr, r2
        bx      r1

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

#include "include/header.S"

#define PURE_THUMB 1

section .ucl_nrv2b_decompress_8
#include "arch/arm/v4t/nrv2b_d8.S"
#undef GETBIT
#undef wrnk

section Call2B
        bl      .thumb_nrv2b_d8


section .ucl_nrv2e_decompress_8
#include "arch/arm/v4t/nrv2e_d8.S"
#undef GETBIT
#undef wrnk

section Call2E
        bl      .thumb_nrv2e_d8


#if 0
section .ucl_nrv2b_decompress_32
#include "arch/arm/v4t/nrv2b_d32.S"
#undef GETBIT
#undef wrnk
#endif


#if 0
section .ucl_nrv2e_decompress_32
#include "arch/arm/v4t/nrv2e_d32.S"
#undef GETBIT
#undef wrnk
#endif

////////////////////////////////////////

section CallLZMA
        // r0=src0, r1=slen, r2=dst0, r3=addr dstl

        parb    .req r3
        para    .req r4
        parc    .req r5
        pard    .req r6

        adr     r7, .LzmaParams
        ldmia   r7!, {parb, para, parc, pard}    // load params

        mov     r7, sp                  // save stack
        add     para, sp
        mov     r1, #0
.Lclearstack:
        push    {r1}
        cmp     sp, para
        bne     .Lclearstack

        push    {r2, parb, para}        // out, outSize, &outSizeProcessed
        add     r3, para, #4            // &inSizeProcessed
        mov     r2, parc                // inSize
        add     r1, r0, #2              // in
        add     r0, r3, #4              // &CLzmaDecoderState
        str     pard, [r0]              // lc, lp, pb, dummy
        bl      LZMA_DECODE
        mov     sp, r7
        b       .LLZMA_end

        .align  2
.LzmaParams:
        .long   lzma_u_len, lzma_stack_adjust, lzma_c_len, lzma_properties

.LLZMA_end:

section LZMA_DECODE
.real_start_ofLZMA_DECODE:              // suppress silly warnings

section LZMA_DEC10
#include "arch/arm/v4t/lzma_d_cs.S"

