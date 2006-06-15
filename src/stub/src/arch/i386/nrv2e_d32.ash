;  n2e_d32.ash -- ucl_nrv2e_decompress_le32 in 32-bit assembly
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


%macro          getbit_n2e 1
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
%define getbit  getbit_n2e


%ifdef  __N2ESMA10__
                jmps    dcl1_n2e
decompr_literals_n2e:
                movsb
%else;  __N2EFAS10__
                jmps    dcl1_n2e
                align   8
;       __N2EFAS11__
decompr_literalb_n2e:
                mov     al, [esi]
                inc     esi
                mov     [edi], al
                inc     edi
%endif; __N2EDEC10__


decompr_loop_n2e:
                add     ebx, ebx
                jnz     dcl2_n2e
dcl1_n2e:
                getbit  32
dcl2_n2e:
%ifdef  __N2ESMA20__
                jc      decompr_literals_n2e
                xor     eax, eax
                inc     eax
%else;  __N2EFAS20__
    %ifndef UPX102
        mov al, [edi]  ;; force data cache allocate (PentiumPlain or MMX)
    %endif
                jc      decompr_literalb_n2e
                mov     eax, 1
%endif; __N2EDEC20__
loop1_n2e:
                getbit  1
                adc     eax, eax
%ifdef  __N2ESMA30__
                getbit  1
                jc      loopend1_n2e
%else;  __N2EFAS30__
                add     ebx, ebx
                jnc     loopcontinue1_n2e
                jnz     loopend1_n2e
                getbit  32
                jc      loopend1_n2e
loopcontinue1_n2e:
%endif; __N2EDEC30__
                dec     eax
                getbit  1
                adc     eax, eax
                jmps    loop1_n2e

decompr_mlen1_n2e:
                getbit  1
                adc     ecx, ecx
                jmps    decompr_got_mlen_n2e

loopend1_n2e:
                xor     ecx, ecx
                sub     eax, byte 3
                jb      decompr_prev_dist_n2e
                shl     eax, 8
                mov     al, [esi]
                inc     esi
                xor     eax, byte -1
                jz      decompr_end_n2e
                sar     eax, 1                  ; shift low-bit into carry
                mov     ebp, eax
                jmps    decompr_ebpeax_n2e
decompr_prev_dist_n2e:
                getbit  1
decompr_ebpeax_n2e:
                jc      decompr_mlen1_n2e
                inc     ecx
                getbit  1
                jc      decompr_mlen1_n2e
loop2_n2e:
                getbit  1
                adc     ecx, ecx
%ifdef  __N2ESMA40__
                getbit  1
                jnc     loop2_n2e
%else;  __N2EFAS40__
                add     ebx, ebx
                jnc     loop2_n2e
                jnz     loopend2_n2e
                getbit  32
                jnc     loop2_n2e
loopend2_n2e:
%endif; __N2EDUMM1__
%ifdef  __N2ESMA50__
                inc     ecx
                inc     ecx
%else;  __N2EFAS50__
                add     ecx, byte 2
%endif; __N2EDEC50__
decompr_got_mlen_n2e:
                cmp     ebp, -0x500
                adc     ecx, byte 2
%ifdef  __N2ESMA60__
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
                jmpn    decompr_loop_n2e
%else;  __N2EFAS60__
                lea     edx, [edi+ebp]
                cmp     ebp, byte -4
    %ifndef UPX102
        mov al, [edi+ecx]  ;; force data cache allocate (PentiumPlain or MMX)
    %endif
                jbe     decompr_copy4_n2e
loop3_n2e:
                mov     al, [edx]
                inc     edx
                mov     [edi], al
                inc     edi
                dec     ecx
                jnz     loop3_n2e
                jmpn    decompr_loop_n2e
;       __N2EFAS61__
                align   4
decompr_copy4_n2e:
                mov     eax, [edx]
                add     edx, byte 4
                mov     [edi], eax
                add     edi, byte 4
                sub     ecx, byte 4
                ja      decompr_copy4_n2e
                add     edi, ecx
                jmpn    decompr_loop_n2e
%endif; __N2EDEC60__
decompr_end_n2e:
;       __NRV2EEND__

; vi:ts=8:et

