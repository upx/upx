;  l_djgpp2.asm -- loader & decompressor for the djgpp2/coff format
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2004 Laszlo Molnar
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


%define         jmps    jmp short
%define         jmpn    jmp near
%include        "macros.ash"

                BITS    32
                SECTION .text
                ORG     0
                CPU     386

; =============
; ============= ENTRY POINT
; =============

;       __DJ2MAIN1__
start:
                push    ds
                pop     es

                mov     esi, 'INPP'             ; input pointer
                mov     edi, 'OUTP'             ; output pointer
%ifdef  __DJCALLT1__
                push    edi
%endif; __DJ2MAIN2__
;               cld                             ; the stub sets this
                or      ebp, byte -1

; =============
; ============= DECOMPRESSION
; =============

%include      "n2b_d32.ash"
%include      "n2d_d32.ash"
%include      "n2e_d32.ash"

; =============

;       __DJ2BSS00__
                mov     ecx, 'BSSL'
                rep
                stosd
%ifdef  __DJCALLT2__

; =============
; ============= CALLTRICK
; =============

                pop     edi
                cjt32   0
%endif; __DJRETURN__

; =============

                push    dword 'ENTR'            ; entry point
                ret

; because of a feature of the djgpp loader, the size of this stub must be
; a multiple of 4 and as the upx decompressor depends on the fact that
; the compressed data stream begins just after the header, i must
; use an alignment here - ML
                align   4
%include        "header.ash"
eof:
;       __DJTHEEND__
                section .data
                dd      -1
                dw      eof


; vi:ts=8:et:nowrap
