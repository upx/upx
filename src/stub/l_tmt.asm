;  l_tmt.asm -- loader & decompressor for the tmt/adam format
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
;       __TMTMAIN1__
                mov     edi, 0          ; relocation offset
                push    edi
                lea     esi, [edi + 'ESI0']
                lea     edi, [edi + 'EDI0']
                mov     ecx, 'ECX0'

                std
                rep
                movsb
                cld

                lea     esi, [edi + 1]
                pop     edi
                or      ebp, byte -1
                push    edi
%ifdef  __TMTCALT1__
                push    edi
%endif; __TMTMAIN2__
                jmpn    .1+'JMPD'
.1:
%include        "header.ash"

cutpoint:
;       __TMTCUTPO__

; =============
; ============= DECOMPRESSION
; =============

%include      "n2b_d32.ash"
%include      "n2d_d32.ash"
%include      "n2e_d32.ash"

;       __TMTMAIN5__
                pop     ebp
                mov     esi, edi
                sub     esi, [edi - 4]

; =============
; ============= CALLTRICK
; =============

%ifdef  __TMTCALT2__
                pop     edi
                cjt32   ebp
%endif; __TMTRELOC__

; =============
; ============= RELOCATION
; =============

                lea     edi, [ebp - 4]
                reloc32 esi, edi, ebp

; =============
;       __TMTJUMP1__
                jmpn    .1+'JMPO'
.1:
eof:
;       __TMTHEEND__
                section .data
                dd      -1
                dw      eof


; vi:ts=8:et:nowrap
