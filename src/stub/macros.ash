;  macros.ash --
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2001 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2001 Laszlo Molnar
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
;  Markus F.X.J. Oberhumer   Laszlo Molnar
;  markus@oberhumer.com      ml1050@cdata.tvnet.hu
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



;; =============
;; ============= 32-BIT CALLTRICK & JUMPTRICK
;; =============

;;  call & jump trick : 2 in 1
%macro          cjt32   1
%ifdef  __CALLTR00__
                mov     ecx, 'TEXL'
calltrickloop:
                mov     al, [edi]
                inc     edi
                sub     al, 0xE8
ct1:
                cmp     al, 1
                ja      calltrickloop
%ifdef  __CTCLEVE1__
                cmp     byte [edi], '?'
                jnz     calltrickloop
%endif; __CALLTR01__
                mov     eax, [edi]
                mov     bl, [edi + 4]
%ifdef  __CTDUMMY1__
%ifdef  __CTBSHR01__
                shr     ax, 8
%else;  __CTBROR01__
                xchg    al, ah
%endif; __CTBSWA01__
                rol     eax, 16
                xchg    al, ah
%endif; __CALLTR02__
                sub     eax, edi
                sub     bl, 0xE8
 %ifnidn %1,0
                add     eax, %1
 %endif
                mov     [edi], eax
                add     edi, byte 5
                mov     al, bl
                loop    ct1
%else;  __CALLTR10__
;; 32-bit call XOR jump trick
                mov     ecx, 'TEXL'
ctloop1:
%ifdef  __CALLTRE8__
                mov     al,0xE8
%else;  __CALLTRE9__
                mov     al,0xE9
%endif; __CALLTR11__
ctloop2:
                repnz
                scasb
                jnz     ctend
%ifdef  __CTCLEVE2__
                cmp     byte [edi], '?'
                jnz     ctloop2
%endif; __CALLTR12__
                mov     eax, [edi]
%ifdef  __CTDUMMY2__
%ifdef  __CTBSHR11__
                shr     ax, 8
%else;  __CTBROR11__
                xchg    al, ah
%endif; __CTBSWA11__
                rol     eax, 16
                xchg    al, ah
%endif; __CALLTR13__
                sub     eax, edi
 %ifnidn %1,0
                add     eax, %1
 %endif
                stosd
                jmps    ctloop1
ctend:
%endif; __CTTHEEND__
%endmacro



;; =============
;; ============= 32-BIT RELOCATIONS
;; =============

%macro          reloc32 3
;       __RELOC320__
reloc_main:
                xor     eax, eax
                mov     al, [%1]
                inc     %1
                or      eax, eax
                jz      reloc_endx
                cmp     al, 0xEF
                ja      reloc_fx
reloc_add:
                add     %2, eax
 %if 1
                mov     eax, [%2]
                xchg    al, ah
                rol     eax, 16
                xchg    al, ah
                add     eax, %3
                mov     [%2], eax
 %else
                add     [%2], %3
 %endif
                jmps    reloc_main
reloc_fx:
                and     al, 0x0F
                shl     eax, 16
                mov     ax, [%1]
                add     %1, byte 2
%ifdef  __REL32BIG__
                or      eax, eax
                jnz     reloc_add
                mov     eax, [%1]
                add     %1, byte 4
%endif; __RELOC32J__
                jmps    reloc_add
reloc_endx:
;       __REL32END__
%endmacro


; vi:ts=8:et:nowrap
