/* l_armpe_s.S -- ARM/PE decompressor assembly startup

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

#define DEBUG 0

#undef STUB_IN_THUMB_MODE
#if defined(__ARM_ARCH_4T__) || defined(__ARM_ARCH_5T__)
# define STUB_IN_THUMB_MODE
#endif

#ifdef STUB_FOR_DLL
#define START _start: cmp r1, #1; bne     .Lstart_orig
#else
#define START _start:
#endif


        .text
        .align  0
        .globl  upx_main
        .globl  _start

#ifdef STUB_IN_THUMB_MODE

        START
        stmfd   sp!, {r0 - r3, lr}

        @ touch all pages in ARM mode - this seems to be required
        @ otherwise the THUMB mode stuff fails

        adr     r0, SRC0

        ldmia   r0, {r1, r2, r3}        @ r1 - src, r2 - slen, r3 - dst
        add     r1, r1, r2
.L01:
        ldr     r2, [r3]
        add     r3, r3, #4096
        cmp     r3, r1
        bls     .L01

# ifdef __ARM_ARCH_4T__
        adr     ip, call_upxmain + 1
        mov     lr, pc
        bx      ip
        ldmfd   sp!, {r0 - r3, lr}
.Lstart_orig:
        ldr     ip, ENTR
        bx      ip
# else
        blx     upx_main                @ call thumb armv5t style
        ldmfd   sp!, {r0 - r3, lr}
.Lstart_orig:
        ldr     pc, ENTR                @ original entry
# endif



        .code   16
        .global GetProcAddressA
        .thumb_func

GetProcAddressA:
        ldr     r3, GETP
.Lcallr3:
        ldr     r3, [r3]
        bx      r3

        .global LoadLibraryW
        .thumb_func

LoadLibraryW:
        ldr     r3, LLIB
        b       .Lcallr3

        .global CacheSync
        .thumb_func
CacheSync:
        ldr     r3, CSYN
        b       .Lcallr3

# ifdef __ARM_ARCH_4T__
call_upxmain:
        b       upx_main
# endif



        .global Unfilter_0x50
        .thumb_func

        buffer  .req r0
        addval  .req r2
        bufend  .req r4

Unfilter_0x50:
        push    {r4 - r7, lr}
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
.Luf50_ret:
        pop     {r4 - r7, pc}

        .unreq  buffer
        .unreq  addval
        .unreq  bufend



        .global get_le32
        .thumb_func
get_le32:                               @ optimized for size
        mov     r1, #3
.Lg0:
        ldrb    r3, [r0, r1]
        lsl     r2, r2, #8
        add     r2, r2, r3
        sub     r1, #1
        bpl     .Lg0
        mov     r0, r2
        bx      lr



# ifdef STUB_FOR_DLL
        .global reloc_main
        .thumb_func
        buffer  .req r0
        dest    .req r1
        addval  .req r2

reloc_main:
        push    {r4, r5, lr}
        ldr     buffer, BREL
        ldr     addval, DST0
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

.Lreloc_end:
        pop     {r4, r5, pc}

# endif

# if DEBUG

// debugging stuff - helpers for dumping memory to a file or deleting a file

// !!!!! check the lines with "system dependent" below before calling

        .global CFwrap
        .thumb_func
CFwrap:
        push    {r3}
        ldr     r3, createfilew
        mov     ip, r3
        pop     {r3}
        bx      ip

        .global WFwrap
        .thumb_func
WFwrap:
        push    {r3}
        ldr     r3, writefile
        mov     ip, r3
        pop     {r3}
        bx      ip

        .global CHwrap
        .thumb_func
CHwrap:
        push    {r3}
        ldr     r3, closehandle
        mov     ip, r3
        pop     {r3}
        bx      ip

        .global DelFile
        .thumb_func
DelFile:
        adr     r1, filename
        strb    r0, [r1, #2]
        mov     r0, r1
        ldr     r3, deleteffilew
        mov     ip, r3
        bx      ip

        .code   32
createfilew:
        .word   0x1f7927c               @ system dependent
closehandle:
        .word   0x1f71594               @ system dependent
deleteffilew:
        .word   0x1f7b920               @ system dependent
writefile:
        .word   0x1f79434               @ system dependent
filename:
        .byte   '\\', 0, 'r', 0, 0, 0

# endif

#else // stub in ARM mode

        START

        stmfd   sp!, {r0 - r3, lr}
        adr     r0, SRC0
	bl	upx_main
        ldmfd   sp!, {r0 - r3, lr}
.Lstart_orig:
        ldr	pc, ENTR



        .global LoadLibraryW
LoadLibraryW:
        ldr     r3, LLIB
        ldr     pc, [r3]

        .global GetProcAddressA
GetProcAddressA:
        ldr     r3, GETP
        ldr     pc, [r3]

        .global CacheSync
CacheSync:
        ldr     r3, CSYN
        ldr     pc, [r3]



        .global Unfilter_0x50
        buffer  .req r0
        addval  .req r2
        bufend  .req ip

Unfilter_0x50:
        ldr     buffer, FIBS
        mov     addval, #0
        ldr     bufend, FIBE
.Luf50_0:
        cmp     buffer, bufend

        moveq   pc, lr

        ldrb    r3, [buffer, #3]
        and     r3, r3, #0x0f
        cmp     r3, #0x0b
        bne     .Luf50_1
        ldr     r3, [buffer]
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



        .global get_le32
get_le32:                               @ optimized for size
        mov     r2, #3
.Lg0:
        ldrb    r3, [r0, r2]
        subs    r2, r2, #1
        add     r1, r3, r1, asl #8
        bpl     .Lg0
        mov     r0, r1
        mov     pc, lr



# ifdef STUB_FOR_DLL
        .global reloc_main
        buffer  .req r0
        dest    .req r1
        addval  .req r2

reloc_main:
        ldr     buffer, BREL
        ldr     addval, DST0
        sub     dest, addval, #4

.Lreloc_loop:
        ldrb    r3, [buffer], #1
        cmp     r3, #0
        moveq   pc, lr
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

        .unreq  buffer
        .unreq  addval
        .unreq  dest
# endif

# if DEBUG

// debugging stuff - helpers for dumping memory to a file or deleting a file

// !!!!! check the lines with "system dependent" below before calling

        .global CFwrap
CFwrap:
        ldr     pc, createfilew

        .global WFwrap
WFwrap:
        ldr     pc, writefile

        .global CHwrap
CHwrap:
        ldr     pc, closehandle

        .global DelFile
DelFile:
        adr     r1, filename
        strb    r0, [r1, #2]
        mov     r0, r1
        ldr     pc, deleteffilew

        .code   32
createfilew:
        .word   0x1f7927c               @ system dependent
closehandle:
        .word   0x1f71594               @ system dependent
deleteffilew:
        .word   0x1f7b920               @ system dependent
writefile:
        .word   0x1f79434               @ system dependent
filename:
        .byte   '\\', 0, 'r', 0, 0, 0

# endif

#endif


@@@@@@@@@@@@@@@@ paramater area for UPX

        .code   32
        .align  2
SRC0:   .ascii  "SRC0"                  @ start of compressed data
SRCL:   .ascii  "SRCL"                  @ compressed length
DST0:   .ascii  "DST0"                  @ start of uncompressed data
        .ascii  "DSTL"                  @ uncompressed length
        .ascii  "BIMPONAM"              @ start of import data & dll names
GETP:   .ascii  "GETP"                  @ pointer to GetProcAddressA
LLIB:   .ascii  "LOAD"                  @ pointer to LoadLibraryW
ENTR:   .ascii  "ENTR"                  @ original entry point
BREL:   .ascii  "BREL"                  @ start of reloc info
FIBS:   .ascii  "FIBS"                  @ buffer start for filter
FIBE:   .ascii  "FIBE"                  @ buffer end for filter
CSYN:   .ascii  "CSYN"                  @ pointer to CacheSync

