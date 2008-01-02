/*
;  macros.S --
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2008 Laszlo Molnar
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
;  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
;
*/

#undef amd64
#undef i386
#undef i486
#undef i586
#undef x86_64

                .altmacro
                .intel_syntax noprefix
                .arch   i8086, jumps
                .code16

.macro          section name
                .section \name
                .code16
                .set current_section,\name
.endm

.macro          ignore_reloc_overflow sym
                .pushsection ignore_reloc_overflow
                .byte   \sym
                .byte   current_section
                .popsection
.endm

.macro          CPU     id
                .ifc    \id, 8086
                .arch   i8086, jumps
                .endif
                .ifc    \id, 286
                .arch   i286, jumps
                .endif
.endm

.macro          jmps    target
                .byte   0xeb, \target - . - 1
.endm

.macro          jos     target
                .byte   0x70, \target - . - 1
.endm

.macro          jnos    target
                .byte   0x71, \target - . - 1
.endm

.macro          jcs     target
                .byte   0x72, \target - . - 1
.endm

.macro          jncs    target
                .byte   0x73, \target - . - 1
.endm

.macro          jzs     target
                .byte   0x74, \target - . - 1
.endm

.macro          jnzs    target
                .byte   0x75, \target - . - 1
.endm

.macro          jnas    target
                .byte   0x76, \target - . - 1
.endm

.macro          jas     target
                .byte   0x77, \target - . - 1
.endm

.macro          jss     target
                .byte   0x78, \target - . - 1
.endm

.macro          jnss    target
                .byte   0x79, \target - . - 1
.endm

.macro          jps     target
                .byte   0x7a, \target - . - 1
.endm

.macro          jnps    target
                .byte   0x7b, \target - . - 1
.endm

.macro          jls     target
                .byte   0x7c, \target - . - 1
.endm

.macro          jnls    target
                .byte   0x7d, \target - . - 1
.endm

.macro          jngs    target
                .byte   0x7e, \target - . - 1
.endm

.macro          jgs     target
                .byte   0x7f, \target - . - 1
.endm

#define         jaes    jncs
#define         jbes    jnas
#define         jbs     jcs
#define         jes     jzs
#define         jnbs    jncs
#define         jnes    jnzs


/*
; =============
; ============= 16-BIT CALLTRICK & JUMPTRICK
; =============
*/

.macro          cjt16   ct_end
section         CALLTR16
                pop     si
                mov     cx, offset calltrick_calls
cjt16_L1:
                lodsb
                sub     al, 0xe8
                cmp     al, 1
                jas     cjt16_L1

section         CT16I286
                CPU     286
                rolw    [si], 8
                CPU     8086
section         CT16SUB0
                sub     [si], si
section         CT16I086
                mov     bx, [si]
                xchg    bh, bl
                sub     bx, si
                mov     [si], bx
section         CALLTRI2
                lodsw
                loop    cjt16_L1


/*
; =============
*/

section         CT16E800
                mov     al, 0xe8
section         CT16E900
                mov     al, 0xe9
section         CALLTRI5
                pop     di
                mov     cx, offset calltrick_calls
cjt16_L11:
                repne
                scasb
section         CT16JEND
                jnzs    \ct_end
section         CT16JUL2
                jnzs    cjt16_L2
section         CT16I287
                CPU     286
                rolw    [di], 8
                CPU     8086
section         CT16SUB1
                sub     [di], di
section         CT16I087
                mov     bx, [di]
                xchg    bh, bl
                sub     bx, di
                mov     [di], bx
section         CALLTRI6
                scasw
                jmps    cjt16_L11
cjt16_L2:
.endm


/*
; vi:ts=8:et:nowrap
*/
