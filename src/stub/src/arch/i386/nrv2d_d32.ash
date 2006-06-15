;  n2d_d32.ash -- ucl_nrv2d_decompress_le32 in 32-bit assembly
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


%macro          getbit_n2d 1
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
%define getbit  getbit_n2d


%ifdef  __N2DSMA10__
                jmps    dcl1_n2d
decompr_literals_n2d:
                movsb
%else;  __N2DFAS10__
                jmps    dcl1_n2d
                align   8
;       __N2DFAS11__
decompr_literalb_n2d:
                mov     al, [esi]
                inc     esi
                mov     [edi], al
                inc     edi
%endif; __N2DDEC10__


decompr_loop_n2d:
                add     ebx, ebx
                jnz     dcl2_n2d
dcl1_n2d:
                getbit  32
dcl2_n2d:
%ifdef  __N2DSMA20__
                jc      decompr_literals_n2d
                xor     eax, eax
                inc     eax
%else;  __N2DFAS20__
    %ifndef UPX102
        mov al, [edi]  ;; force data cache allocate (PentiumPlain or MMX)
    %endif
                jc      decompr_literalb_n2d
                mov     eax, 1
%endif; __N2DDEC20__
loop1_n2d:
                getbit  1
                adc     eax, eax
%ifdef  __N2DSMA30__
                getbit  1
                jc      loopend1_n2d
%else;  __N2DFAS30__
                add     ebx, ebx
                jnc     loopcontinue1_n2d
                jnz     loopend1_n2d
                getbit  32
                jc      loopend1_n2d
loopcontinue1_n2d:
%endif; __N2DDEC30__
                dec     eax
                getbit  1
                adc     eax, eax
                jmps    loop1_n2d
loopend1_n2d:
                xor     ecx, ecx
                sub     eax, byte 3
                jb      decompr_prev_dist_n2d
                shl     eax, 8
                mov     al, [esi]
                inc     esi
                xor     eax, byte -1
                jz      decompr_end_n2d
                sar     eax, 1                  ; shift low-bit into carry
                mov     ebp, eax
                jmps    decompr_ebpeax_n2d
decompr_prev_dist_n2d:
                getbit  1
decompr_ebpeax_n2d:
                adc     ecx, ecx
                getbit  1
                adc     ecx, ecx
                jnz     decompr_got_mlen_n2d
                inc     ecx
loop2_n2d:
                getbit  1
                adc     ecx, ecx
%ifdef  __N2DSMA40__
                getbit  1
                jnc     loop2_n2d
%else;  __N2DFAS40__
                add     ebx, ebx
                jnc     loop2_n2d
                jnz     loopend2_n2d
                getbit  32
                jnc     loop2_n2d
loopend2_n2d:
%endif; __N2DDUMM1__
%ifdef  __N2DSMA50__
                inc     ecx
                inc     ecx
%else;  __N2DFAS50__
                add     ecx, byte 2
%endif; __N2DDEC50__
decompr_got_mlen_n2d:
                cmp     ebp, -0x500
                adc     ecx, byte 1
%ifdef  __N2DSMA60__
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
                jmpn    decompr_loop_n2d
%else;  __N2DFAS60__
                lea     edx, [edi+ebp]
                cmp     ebp, byte -4
    %ifndef UPX102
        mov al, [edi+ecx]  ;; force data cache allocate (PentiumPlain or MMX)
    %endif
                jbe     decompr_copy4_n2d
loop3_n2d:
                mov     al, [edx]
                inc     edx
                mov     [edi], al
                inc     edi
                dec     ecx
                jnz     loop3_n2d
                jmpn    decompr_loop_n2d
;       __N2DFAS61__
                align   4
decompr_copy4_n2d:
                mov     eax, [edx]
                add     edx, byte 4
                mov     [edi], eax
                add     edi, byte 4
                sub     ecx, byte 4
                ja      decompr_copy4_n2d
                add     edi, ecx
                jmpn    decompr_loop_n2d
%endif; __N2DDEC60__
decompr_end_n2d:
;       __NRV2DEND__

; vi:ts=8:et

