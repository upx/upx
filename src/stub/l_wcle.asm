;  l_wcle.asm -- loader & decompressor for the watcom/le format
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

start:
;       __WCLEMAIN__
                mov     edi, 'alib'     ; address of obj#1:0 (filled by a fixup record)

; The following hack fools the lame protection of dos4g/w, which expects the
; 'WATCOM' string somewhere in the first 18 bytes after the entry point
; I use this imul thingy, because it's 1 byte shorter than a jump ;-)
; ... and "alibiWATCOM" looks cool
                db      'iWATCOM'       ; imul edx,[edi+0x41],'TCOM'

                push    es
                push    ds
                pop     es
                push    edi

                lea     esi, [edi + 'ESI0']
                lea     edi, [edi + 'EDI0']
                mov     ecx, 'ECX0'

                std
                rep
                movsd
                cld

                lea     esi, [edi + 4]
                pop     edi
                or      ebp, byte -1
                push    edi
                jmpn    .1+'JMPD'
.1:
%include        "header.ash"

cutpoint:
;       __WCLECUTP__

; =============
; ============= DECOMPRESSION
; =============

%include      "n2b_d32.ash"
%include      "n2d_d32.ash"
%include      "n2e_d32.ash"

; =============

;       __WCLEMAI2__
                pop     ebp
                push    esi
                lea     esi, [ebp + 'RELO']
                push    esi

; =============
; ============= CALLTRICK
; =============

%ifdef  __WCALLTRI__
%ifdef  __WCCTTPOS__
                lea     edi, [ebp + 'TEXV']
%else;  __WCCTTNUL__
                mov     edi, ebp
%endif; __WCALLTR1__
                cjt32   ebp
%endif; __WCDUMMY1__

; =============
; ============= RELOCATION
; =============

%ifdef  __WCRELOC1__
                lea     edi, [ebp - 4]
                reloc32 esi, edi, ebp
;               eax = 0
%endif; __WCDUMMY2__

%ifdef  __WCRELSEL__
                call    esi             ; selector fixup code (modifies bx)
%endif; __WCLEMAI4__

; =============

                pop     edi
                pop     ecx
                sub     ecx, edi
                shr     ecx, 2
                rep
                stosd                   ; clear dirty memory
                pop     es
                lea     esp, [ebp + 'ESP0']

                jmpn    .1+'JMPO'
.1:

; =============

eof:
;       __WCTHEEND__
                section .data
                dd      -1
                dw      eof


; vi:ts=8:et:nowrap
