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

; ebx = alloca('UPXa');

        mov     ebp, esp                ; save stack

        lea     ebx, [esp + 'UPXa']
        xor     eax, eax
.clearstack1:
        push    eax
        cmp     esp, ebx
        jnz     .clearstack1


        inc     esi                     ; skip 2 bytes for properties
        inc     esi

        push    ebx                     ; &outSizeProcessed
        push    'UPXb'                  ; outSize
        push    edi                     ; out
        add     ebx, 4
        push    ebx                     ; &inSizeProcessed
        push    'UPXc'                  ; inSize
        push    esi                     ; in
        add     ebx, 4
        push    ebx                     ; &CLzmaDecoderState
        push    eax                     ; dummy for call

        ; hardwired LzmaDecodeProperties()
        mov     dword [ebx], 'UPXd'     ; lc, lp, pb, dummy


; __LZMA_ELF00__
;
LZMA_BASE_SIZE equ 1846
LZMA_LIT_SIZE  equ  768

  %ifndef O_OUTS  ; ELF defines them, others do not care
  %define O_OUTS 0
  %define O_INS  0
  %endif

        mov     ebp, esp                ; save stack
        mov     edx,[O_INS + ebp]       ; inSize

        lodsb           ; first byte, replaces LzmaDecodeProperties()
        dec edx
        mov cl,al       ; cl= ((lit_context_bits + lit_pos_bits)<<3) | pos_bits
        and al,7        ; al= pos_bits
        shr cl,3        ; cl= lit_context_bits + lit_pos_bits

        mov ebx, -LZMA_LIT_SIZE
        shl ebx,cl
; /* inSizeProcessed, outSizeProcessed, *_bits, CLzmaDecoderState */
        lea ebx,[-(2*4 +4) + 2*(-LZMA_BASE_SIZE + ebx) + esp]
        and ebx, byte (~0<<5)  ; 32-byte align
.elf_clearstack1:
        push byte 0
        cmp esp,ebx
        jne .elf_clearstack1

        push    ebx                     ; &outSizeProcessed
        add     ebx, 4
        mov     ecx,[O_OUTS + ebp]      ; &outSize
        push    dword [ecx]             ; outSize
        push    edi                     ; out
        push    ebx                     ; &inSizeProcessed
        add     ebx, 4

        mov [2+ ebx],al  ; store pos_bits
        lodsb           ; second byte, replaces LzmaDecodeProperties()
        dec edx
        mov cl,al       ; cl= (lit_pos_bits<<4) | lit_context_bits
        and al,0xf
        mov [   ebx],al  ; store lit_context_bits
        shr cl,4
        mov [1+ ebx],cl  ; store lit_pos_bits

        push    edx                     ; inSize -2
        push    esi                     ; in
        push    ebx                     ; &CLzmaDecoderState
        push    eax                     ; return address slot (dummy CALL)



; __LZMA_DEC10__
%include "arch/i386/lzma_d_cs.ash"

; __LZMA_DEC20__
%include "arch/i386/lzma_d_cf.ash"


;
; cleanup
; __LZMA_DEC30__
;

        add     esi, [ebx - 4]          ; inSizeProcessed
        add     edi, [ebx - 8]          ; outSizeProcessed
        xor     eax, eax

        lea     ecx, [esp - 256]
        mov     esp, ebp                ; restore stack
.clearstack2:
        push    eax
        cmp     esp, ecx
        jnz     .clearstack2

        mov     esp, ebp                ; restore stack
        xor     ecx, ecx


; vi:ts=8:et

