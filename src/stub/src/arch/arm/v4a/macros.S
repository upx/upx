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

                .altmacro
                .arm

.macro          section name
                .section \name
.endm

#if defined(ARMEL_EABI4)  /*{*/

__NR_SYSCALL_BASE = 0
.macro          do_sys N
                mov r12,r7  // save r7 in ip
                mov r7,#\N  // syscall number
                swi 0
                mov r7,r12  // restore r7 from ip
.endm
.macro          do_sys2 N
                mov r12,r7  // save r7 in ip
                mov r7,   #(\N) & 0xff  // syscall number
                orr r7,r7,#(\N) &~0xff  // high bits
                swi 0
                mov r7,r12  // restore r7 from ip
.endm

#else  /*}{*/

__NR_SYSCALL_BASE = 0x900000
.macro          do_sys N
                swi \N
.endm
.macro          do_sys2 N
                swi \N
.endm

#endif  /*}*/

.macro          ret
                mov pc,lr  /* armv4 fails for thumb interworking */
.endm

// vi:ts=8:et:nowrap
