;  lzma_d.ash -- 32-bit assembly
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 2006-2006 Markus Franz Xaver Johannes Oberhumer
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
;  Markus F.X.J. Oberhumer
;  <markus@oberhumer.com>
;  http://www.oberhumer.com/opensource/upx/
;


; ------------- DECOMPRESSION -------------

; Input:
;   esi - source
;   edi - dest
;   ebp - -1
;   cld

; Output:
;   eax - 0
;   ecx - 0


    %ifndef jmps
    %define jmps    jmp short
    %endif
    %ifndef jmpn
    %define jmpn    jmp near
    %endif

    CPU 386

;
; init
; __LZMA_DEC00__
;

        mov     ebp, esp                ; save stack

        lea     ecx, [esp + 'UPXa']
        xor     eax, eax
.clearstack1:
        push    eax
        cmp     esp, ecx
        jnz     .clearstack1

        inc     esi

        push    ecx                     ; &outSizeProcessed
        push    'UPXb'                  ; outSize
        push    edi                     ; out
        add     ecx, 4
        push    ecx                     ; &inSizeProcessed
        push    'UPXc'                  ; inSize
        push    esi                     ; in
        add     ecx, 4
        push    ecx                     ; &CLzmaDecoderState
        push    eax                     ; dummy for call

        mov     dword [ecx], 'UPXd'     ; lc, lp, pb, dummy


; __LZMA_DEC10__
%include "arch/i386/lzma_d_cs.ash"

; __LZMA_DEC20__
%include "arch/i386/lzma_d_cf.ash"


;
; cleanup
; __LZMA_DEC30__
;

        add     esi, [esp + 32 + 4]
        add     edi, [esp + 32 + 0]
        xor     eax, eax

        lea     ecx, [esp - 1024]
        mov     esp, ebp                ; restore stack
.clearstack2:
        push    eax
        cmp     esp, ecx
        jnz     .clearstack2

        mov     esp, ebp                ; restore stack
        xor     ecx, ecx


; vi:ts=8:et

