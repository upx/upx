/*
;  macros.S --
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2024 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2024 Laszlo Molnar
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

                //.altmacro

.macro          section name
                .section \name
.endm

.macro PUSH4 p1,p2,p3,p4
    stwu \p1,-4*NBPW(sp)
    stw \p2, 1*NBPW(sp)
    stw \p3, 2*NBPW(sp)
    stw \p4, 3*NBPW(sp)
.endm
.macro POP4 p1,p2,p3,p4
    lwz  \p1, 0*NBPW(sp)
    lwz  \p2, 1*NBPW(sp)
    lwz  \p3, 2*NBPW(sp)
    lwz  \p4, 3*NBPW(sp)
    addi sp,sp,4*NBPW
.endm
.macro PUSH3 p1,p2,p3
    stwu \p1,-3*NBPW(sp)
    stw \p2, 1*NBPW(sp)
    stw \p3, 2*NBPW(sp)
.endm
.macro POP3 p1,p2,p3
    lwz  \p1, 0*NBPW(sp)
    lwz  \p2, 1*NBPW(sp)
    lwz  \p3, 2*NBPW(sp)
    addi sp,sp,3*NBPW
.endm
.macro POP2 p1,p2
    lwz  \p1, 0*NBPW(sp)
    lwz  \p2, 1*NBPW(sp)
    addi sp,sp,2*NBPW
.endm
.macro POP1 p1
    lwz  \p1, 0*NBPW(sp)
    addi sp,sp,1*NBPW
.endm

// vi:ts=8:et:nowrap
