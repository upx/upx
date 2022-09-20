/*
;  macros.S --
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2022 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2022 Laszlo Molnar
;  All Rights Reserved.
;
;  UPX and the UCL library are free software; you can redistribute them
;  and/or modify them under the terms of the GNU General Public License as
;  published by the Free Software Foundation; either version 2 of
;  the License, or (at your option) any later version.
;
;  This program is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with this program; see the file COPYING.
;  If not, write to the Free Software Foundation, Inc.,
;  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
;
;  Markus F.X.J. Oberhumer              Laszlo Molnar
;  <markus@oberhumer.com>               <ezerotven+github@gmail.com>
;
*/
#ifndef __MACROS_S__  // {
#define __MACROS_S__ 1

                .altmacro
lr .req x30
fp .req x29

// ARM64 (AARCH64) REQUIRES 16-BYTE ALIGNED STACK POINTER (SP)
// Therefore PUSH of an odd number of registers, inserts an 8-byte hole.

#define PUSH1(r1)             str r1,   [sp,#-2*8]! /* HOLE */
#define PUSH2(r1,r2)          stp r1,r2,[sp,#-2*8]!
#define PUSH3(r1,r2,r3)       stp r1,r2,[sp,#-4*8]!;\
                              str r3,   [sp,# 2*8]  /* HOLE */
#define PUSH4(r1,r2,r3,r4)    stp r1,r2,[sp,#-4*8]!; \
                              stp r3,r4,[sp,# 2*8]
#define PUSH5(r1,r2,r3,r4,r5) stp r1,r2,[sp,#-6*8]!;\
                              stp r3,r4,[sp,# 2*8];\
                              str r5,   [sp,# 4*8]  /* HOLE */

#define POP1(r1)                               ldr r1,   [sp],#2*8
#define POP2(r1,r2)                            ldp r1,r2,[sp],#2*8
#define POP3(r1,r2,r3)    ldr r3,   [sp,#2*8]; ldp r1,r2,[sp],#4*8
#define POP4(r1,r2,r3,r4) ldp r3,r4,[sp,#2*8]; ldp r1,r2,[sp],#4*8

.macro          section name
                .section \name //,"ax"
.endm

.macro          do_sys N
                mov w8,#\N
                svc #0 // sets Carry iff error
                // orrcs r0,r0,#(1<<31)  // force negative on error; FIXME: needed?
.endm

.macro          clear_cache  // In: x0= lo; x1= hi; uses x2,x3,x4
        mrs x3, ctr_el0
        tbnz w3, #28, 1f  // not needed
        ubfx x2, x3, #16, #4
        mov x4, -4  // word mask (why not 8?)
        lsl x4, x4, x2  // Dline_mask
        and x2, x0, x4  // round down
        cmp x2, x1
        bhs 1f  // empty range
0:
        dc cvau, x2
        sub x2, x2, x4
        cmp x1, x2
        bhi 0b
1:
        dsb ish

        tbnz w3, #29, 3f  // not needed
        and x3, x3, #0xf
        mov x2, -4  // word mask (why not 8?)
        lsl x2, x2, x3  // Iline_mask
        and x0, x0, x2  // round down
        cmp x1, x0
        bls 3f  // empty range
2:
        ic ivau, x0
        sub x0, x0, x2
        cmp x1, x0
        bhi 2b
3:
        dsb ish
        isb
.endm

#endif  //}
// vi:ts=8:et:nowrap
