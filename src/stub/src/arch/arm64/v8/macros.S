/*
;  macros.S --
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2017 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2017 Laszlo Molnar
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

#define PUSH1(r1)          str r1,   [sp,#-1*8]!
#define PUSH2(r1,r2)       stp r1,r2,[sp,#-2*8]!
#define PUSH3(r1,r2,r3)    stp r1,r2,[sp,#-3*8]!; str r3,   [sp,#2*8]
#define PUSH4(r1,r2,r3,r4) stp r1,r2,[sp,#-4*8]!; stp r3,r4,[sp,#2*8]
#define PUSH5(r1,r2,r3,r4,r5) PUSH2(r4,r5); PUSH3(r1,r2,r3)

#define POP1(r1)                               ldr r1,   [sp],#1*8
#define POP2(r1,r2)                            ldp r1,r2,[sp],#2*8
#define POP3(r1,r2,r3)    ldr r3,   [sp,#2*8]; ldp r1,r2,[sp],#3*8
#define POP4(r1,r2,r3,r4) ldp r3,r4,[sp,#2*8]; ldp r1,r2,[sp],#4*8

.macro          section name
                .section \name
.endm

__NR_SYSCALL_BASE = 0
.macro          do_sys N
                mov w8,#\N
                svc #0 // sets Carry iff error
                // orrcs r0,r0,#(1<<31)  // force negative on error; FIXME: needed?
                ret
.endm

.macro          do_dcache_flush  // In: r0=addr; r1=len
                mov w3,#1  // _sys_dcache_flush
                mov w8,#(1<<31)  // syscall number?
                svc #0
.endm

.macro          do_icache_invalidate  // In: r0=addr; r1=len
                mov w3,#0  // _sys_icache_invalidate
                mov w8,#(1<<31)  // syscall number?
                svc #0
.endm

#endif  //}
// vi:ts=8:et:nowrap
