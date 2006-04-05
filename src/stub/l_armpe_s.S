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


#undef STUB_IN_THUMB_MODE
#if defined(__ARM_ARCH_4T__) || defined(__ARM_ARCH_5T__)
# define STUB_IN_THUMB_MODE
#endif

        .text
        .align  0
        .globl  upx_main
        .globl  _start

#ifdef STUB_IN_THUMB_MODE
_start:
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
        adr     lr, .Larmret
        adr     ip, call_upxmain + 1
        bx      ip
.Larmret:
        ldmfd   sp!, {r0 - r3, lr}
        ldr     ip, ENTR
        bx      ip
# else
        blx     upx_main                @ call thumb armv5t style
        ldmfd   sp!, {r0 - r3, lr}
        ldr     pc, ENTR                @ original entry
# endif

@@@@@@@@@@@@@@@@

        .code   16
        .global GetProcAddressA
        .thumb_func

GetProcAddressA:
        ldr     r3, GETP
.Lcallr3:
        ldr     r3, [r3]
        bx      r3

@@@@@@@@@@@@@@@@

        .global LoadLibraryW
        .thumb_func

LoadLibraryW:
        ldr     r3, LLIB
        b       .Lcallr3

# ifdef __ARM_ARCH_4T__
call_upxmain:
        b       upx_main
# endif

@@@@@@@@@@@@@@@@

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

# if 0

// debugging stuff - helpers for dumping memory to a file or deleting a file

// !!!!! check the lines with "system dependent" below before trying

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

_start:
# ifdef STUB_FOR_DLL
        cmp     r1, #1
        bne     .Lstart_orig
# endif
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

        .global get_le32
get_le32:                               @ optimized for size
        mov     r2, #3
.Lg0:
        ldrb    r3, [r0, r2]
        subs    r2, r2, #1
        add     r1, r3, r1, asl #8
        bpl     .Lg0
        mov     r0, r1
gret:
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
        beq     gret
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
# endif

# if 0

// debugging stuff - helpers for dumping memory to a file or deleting a file

// !!!!! check the lines with "system dependent" below before trying

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
SRC0:   .ascii  "SRC0"
SRCL:   .ascii  "SRCL"
DST0:   .ascii  "DST0"
        .ascii  "DSTL"
        .ascii  "BIMPONAM"
GETP:   .ascii  "GETP"
LLIB:   .ascii  "LOAD"
ENTR:   .ascii  "ENTR"
BREL:   .ascii  "BREL"

#ifndef STUB_IN_THUMB_MODE
# include "l_armpe_c.s"
#endif
