/*
;  i086-dos16.sys.S -- loader & decompressor for the dos/sys format
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

#define         SYS     1
#define         COM     0
#define         CJT16   1
#include        "arch/i086/macros.S"


/*
; =============
; ============= ENTRY POINT
; =============
*/

section         SYSMAIN1
start:
                .long   -1
                .short  attribute
                .short  strategy        /* .sys header */
                .short  interrupt       /* opendos wants this field untouched */
strategy:
section         SYSI2861
                CPU     286
                pusha
                CPU     8086
section         SYSI0861
                push    ax
                push    bx
                push    cx
                push    dx
                push    si
                push    di
                push    bp
section         SYSMAIN2
                mov     si, offset copy_source
                mov     di, offset copy_destination

                mov     cx, si          /* at the end of the copy si will be 0 */

                push    es
                push    ds
                pop     es

                std
                rep
                movsb
                cld

                mov     bx, 0x8000

                xchg    di, si
                .byte   0x83, 0xee, neg_e_len /* sub si, xxx -- sets bit C for sbb below */
section         SYSSBBBP
                sbb     bp, bp
section         SYSCALLT
                push    di
section         SYSMAIN3
                jmp     decomp_start_n2b

#include        "include/header.S"

section         SYSCUTPO

#include        "arch/i086/nrv2b_d16.S"

section         SYSMAIN5
                pop     es
section         SYSI2862
                CPU     286
                popa
                CPU     8086
section         SYSI0862
                pop     bp
                pop     di
                pop     si
                pop     dx
                pop     cx
                pop     bx
                pop     ax
section         SYSJUMP1
                jmp     original_strategy

/*
; vi:ts=8:et:nowrap
*/
