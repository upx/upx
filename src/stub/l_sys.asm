;  l_sys.asm -- loader & decompressor for the dos/sys format
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2002 Laszlo Molnar
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
;  <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>
;


%define         SYS     1
%define         COM     0
%define         CJT16   1
%define         jmps    jmp short
%define         jmpn    jmp near
%include        "macros.ash"

                BITS    16
                ORG     0
                SECTION .text

; =============
; ============= ENTRY POINT
; =============

;       __SYSMAIN1__
start:
                dd      -1
                dw      0
                dw      strategy        ; .sys header
                dw      0               ; opendos wants this field untouched
strategy:
%ifdef  __SYSI2861__
                pusha
%else;  __SYSI0861__
                push    ax
                push    bx
                push    cx
                push    dx
                push    si
                push    di
                push    bp
%endif; __SYSMAIN2__
                mov     si, 'SI'
                mov     di, 'DI'

                mov     cx, si          ; at the end of the copy si will be 0

                push    es
                push    ds
                pop     es

                std
                rep
                movsb
                cld

                mov     bx, 0x8000

                xchg    si, di
                sub     si, byte start - cutpoint
;       __SYSSUBSI__
;       __SYSSBBBP__
                sbb     bp, bp
%ifdef  __SYSCALLT__
                push    di
%endif; __SYSMAIN3__
                jmpn    .1+'JM'         ; jump to the decompressor
.1:
%include        "header.ash"

cutpoint:
;       __SYSCUTPO__

; =============
; ============= DECOMPRESSION
; =============

%include        "n2b_d16.ash"

; =============
; ============= CALLTRICK
; =============


; =============

;       __SYSMAIN5__
                pop     es
%ifdef  __SYSI2862__
                popa
%else;  __SYSI0862__
                pop     bp
                pop     di
                pop     si
                pop     dx
                pop     cx
                pop     bx
                pop     ax
%endif; __SYSJUMP1__
                jmpn    eof+'JO'
eof:
;       __SYSTHEND__
                section .data
                dd      -1
                dw      eof


; vi:ts=8:et:nowrap
