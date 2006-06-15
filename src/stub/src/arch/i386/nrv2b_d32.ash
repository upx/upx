;  n2b_d32.ash -- ucl_nrv2b_decompress_le32 in 32-bit assembly
;
;  This file is part of the UCL data compression library.
;
;  Copyright (C) 1996-2005 Markus Franz Xaver Johannes Oberhumer
;  All Rights Reserved.
;
;  The UCL library is free software; you can redistribute it and/or
;  modify it under the terms of the GNU General Public License as
;  published by the Free Software Foundation; either version 2 of
;  the License, or (at your option) any later version.
;
;  The UCL library is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with the UCL library; see the file COPYING.
;  If not, write to the Free Software Foundation, Inc.,
;  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
;
;  Markus F.X.J. Oberhumer
;  <markus@oberhumer.com>
;  http://www.oberhumer.com/opensource/ucl/
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


%macro          getbit_n2b 1
 %if %1==1
                add     ebx, ebx
                jnz     %%gotbit
 %endif
                mov     ebx, [esi]
                sub     esi, byte -4
                adc     ebx, ebx
%%gotbit:
%endmacro
%undef getbit
%define getbit  getbit_n2b


%ifdef  __N2BSMA10__
                jmps    dcl1_n2b
decompr_literals_n2b:
                movsb
%else;  __N2BFAS10__
                jmps    dcl1_n2b
                align   8
;       __N2BFAS11__
decompr_literalb_n2b:
                mov     al, [esi]
                inc     esi
                mov     [edi], al
                inc     edi
%endif; __N2BDEC10__


decompr_loop_n2b:
                add     ebx, ebx
                jnz     dcl2_n2b
dcl1_n2b:
                getbit  32
dcl2_n2b:
%ifdef  __N2BSMA20__
                jc      decompr_literals_n2b
                xor     eax, eax
                inc     eax
%else;  __N2BFAS20__
    %ifndef UPX102
        mov al, [edi]  ;; force data cache allocate (PentiumPlain or MMX)
    %endif
                jc      decompr_literalb_n2b
                mov     eax, 1
%endif; __N2BDEC20__
loop1_n2b:
                getbit  1
                adc     eax, eax
%ifdef  __N2BSMA30__
                getbit  1
                jnc     loop1_n2b
%else;  __N2BFAS30__
                add     ebx, ebx
                jnc     loop1_n2b
                jnz     loopend1_n2b
                getbit  32
                jnc     loop1_n2b
loopend1_n2b:
%endif; __N2BDEC30__
                xor     ecx, ecx
                sub     eax, byte 3
                jb      decompr_ebpeax_n2b
                shl     eax, 8
                mov     al, [esi]
                inc     esi
                xor     eax, byte -1
                jz      decompr_end_n2b
                mov     ebp, eax
decompr_ebpeax_n2b:
                getbit  1
                adc     ecx, ecx
                getbit  1
                adc     ecx, ecx
                jnz     decompr_got_mlen_n2b
                inc     ecx
loop2_n2b:
                getbit  1
                adc     ecx, ecx
%ifdef  __N2BSMA40__
                getbit  1
                jnc     loop2_n2b
%else;  __N2BFAS40__
                add     ebx, ebx
                jnc     loop2_n2b
                jnz     loopend2_n2b
                getbit  32
                jnc     loop2_n2b
loopend2_n2b:
%endif; __N2BDUMM1__
%ifdef  __N2BSMA50__
                inc     ecx
                inc     ecx
%else;  __N2BFAS50__
                add     ecx, byte 2
%endif; __N2BDEC50__
decompr_got_mlen_n2b:
                cmp     ebp, -0xd00
                adc     ecx, byte 1
%ifdef  __N2BSMA60__
    %ifndef UPX102
                push    esi
    %else
                mov     edx, esi
    %endif
                lea     esi, [edi+ebp]
                rep
                movsb
    %ifndef UPX102
                pop     esi
    %else
                mov     esi, edx
    %endif
                jmpn    decompr_loop_n2b
%else;  __N2BFAS60__
                lea     edx, [edi+ebp]
                cmp     ebp, byte -4
    %ifndef UPX102
        mov al, [edi+ecx]  ;; force data cache allocate (PentiumPlain or MMX)
    %endif
                jbe     decompr_copy4_n2b
loop3_n2b:
                mov     al, [edx]
                inc     edx
                mov     [edi], al
                inc     edi
                dec     ecx
                jnz     loop3_n2b
                jmpn    decompr_loop_n2b
;       __N2BFAS61__
                align   4
decompr_copy4_n2b:
                mov     eax, [edx]
                add     edx, byte 4
                mov     [edi], eax
                add     edi, byte 4
                sub     ecx, byte 4
                ja      decompr_copy4_n2b
                add     edi, ecx
                jmpn    decompr_loop_n2b
%endif; __N2BDEC60__
decompr_end_n2b:
;       __NRV2BEND__

; vi:ts=8:et

