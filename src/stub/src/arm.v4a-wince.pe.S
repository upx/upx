/* arm.v4a-wince.pe.S -- ARM/PE decompressor assembly startup (arm mode)

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

#include "arch/arm/v4a/macros.S"

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

// p_armpe.cpp uses the following symbols, so they should be global

        .globl SRC0
        .globl DST0
        .globl IATT
        .globl ENTR
        .globl FIBS
        .globl FIBE
        .globl BREL
        .globl BIMP
        .globl ONAM

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

        .arm
        dst0    .req r9                 @ global register

section DllStart
        cmp     r1, #1
        bne     .Lstart_orig

section ExeStart
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

SRC0:   .long   start_of_compressed
SRCL:   .long   compressed_length
DST0:   .long   start_of_uncompressed
DSTL:   .long   uncompressed_length
IATT:   .long   0, 0, 0, 0
ENTR:   .long   original_entry

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

section ProcessEnd
        ldmia   sp!, {pc}


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

section Unfilter_0x50
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

FIBS:   .long   filter_buffer_start
FIBE:   .long   filter_buffer_end

.Luf_end:
        DDUMP   (#'F')

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

section Relocs
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

BREL:   .long   start_of_relocs

        .unreq  buffer
        .unreq  addval
        .unreq  dest

.Lreloc_end:
        DDUMP   (#'R')

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

section Imports
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

BIMP:   .long   start_of_imports
ONAM:   .long   start_of_dll_names

.Lhi_end:
        add     sp, sp, #bufsize
        DDUMP   (#'I')

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

#include "include/header.S"

section .ucl_nrv2e_decompress_8

#include "arch/arm/v4a/nrv2e_d8.S"

section Call2E
        bl      ucl_nrv2e_decompress_8
        DDUMP   (#'C')

section .ucl_nrv2d_decompress_8

#include "arch/arm/v4a/nrv2d_d8.S"

section Call2D
        bl      ucl_nrv2d_decompress_8
        DDUMP   (#'C')

section .ucl_nrv2b_decompress_8

#include "arch/arm/v4a/nrv2b_d8.S"

section Call2B
        bl      ucl_nrv2b_decompress_8
        DDUMP   (#'C')

////////////////////////////////////////

section CallLZMA
        // r0=src0, r1=slen, r2=dst0, r3=addr dstl

        parb    .req r3
        para    .req r4
        parc    .req r5
        pard    .req r6

        adr     r7, .LzmaParams
        ldmia   r7, {parb, para, parc, pard}    // load params

        mov     r7, sp                  // save stack
        add     para, para, sp
        mov     ip, #0
.Lclearstack:
        stmfd   sp!, {ip}
        cmp     sp, para
        bne     .Lclearstack

        stmfd   sp!, {r2, parb, para}   // out, outSize, &outSizeProcessed
        add     r3, para, #4            // &inSizeProcessed
        mov     r2, parc                // inSize
        add     r1, r0, #2              // in
        add     r0, para, #8            // &CLzmaDecoderState
        str     pard, [r0]              // lc, lp, pb, dummy
        bl      LZMA_DECODE
        mov     sp, r7
        b       .LLZMA_end

.LzmaParams:
        .long   lzma_u_len, lzma_stack_adjust, lzma_c_len, lzma_properties

.LLZMA_end:

section LZMA_DECODE

section LZMA_DEC10
#include "arch/arm/v4a/lzma_d_cs.S"

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

