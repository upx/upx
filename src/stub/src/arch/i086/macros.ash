;  macros.ash --
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


; =============
; ============= 16-BIT CALLTRICK & JUMPTRICK
; =============


%macro          cjt16   1
%ifdef  __CALLTR16__
                pop     si
                mov     cx, 'CT'
cjt16_L1:
                lodsb
                sub     al, 0xe8
                cmp     al, 1
                ja      cjt16_L1

%ifdef  __CT16I286__
                rol     word [si], 8
;       __CT16SUB0__
                sub     [si], si
%else;  __CT16I086__
                mov     bx, [si]
                xchg    bl, bh
                sub     bx, si
                mov     [si], bx
%endif; __CALLTRI2__
                lodsw
                loop    cjt16_L1
%endif; __CT16DUM1__

; =============

%ifdef  __CT16E800__
                mov     al, 0xe8
%else;  __CT16E900__
                mov     al, 0xe9
%endif; __CALLTRI5__
                pop     di
                mov     cx, 'CT'
cjt16_L11:
                repne
                scasb
%ifdef  __CT16JEND__
                jnz     %1              ; FIXME: this doesn't get relocated
%else;  __CT16JUL2__
                jnz     cjt16_L2
%endif; __CT16DUM2__

%ifdef  __CT16I287__
                rol     word [di], 8
;       __CT16SUB1__
                sub     [di], di
%else;  __CT16I087__
                mov     bx, [di]
                xchg    bl, bh
                sub     bx, di
                mov     [di], bx
%endif; __CALLTRI6__
                scasw
                jmps    cjt16_L11
cjt16_L2:
;       __CT16DUMM3__
%endmacro

; vi:ts=8:et:nowrap
