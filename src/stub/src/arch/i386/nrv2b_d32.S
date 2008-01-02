/*
;  n2b_d32.ash -- ucl_nrv2b_decompress_le32 in 32-bit assembly
;
;  This file is part of the UCL data compression library.
;
;  Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
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
*/

.macro          getbit_n2b one
                local   l1
        .ifc    \one, 1
                add     ebx, ebx
                jnzs    l1
        .endif
                mov     ebx, [esi]
                sub     esi, -4
                adc     ebx, ebx
l1:
.endm

#undef getbit
#define getbit  getbit_n2b


section         N2BSMA10
                jmps    dcl1_n2b
decompr_literals_n2b:
                movsb
section         N2BFAS10
                jmps    dcl1_n2b
section         N2BFAS11
decompr_literalb_n2b:
                mov     al, [esi]
                inc     esi
                mov     [edi], al
                inc     edi
section         N2BDEC10


decompr_loop_n2b:
                add     ebx, ebx
                jnz     dcl2_n2b
dcl1_n2b:
                getbit  32
dcl2_n2b:
section         N2BSMA20
                jcs     decompr_literals_n2b
                xor     eax, eax
                inc     eax
section         N2BFAS20
    #ifndef UPX102
        mov al, [edi]  // force data cache allocate (PentiumPlain or MMX)
    #endif
                jcs     decompr_literalb_n2b
                mov     eax, 1
section         N2BDEC20
loop1_n2b:
                getbit  1
                adc     eax, eax
section         N2BSMA30
                getbit  1
                jncs    loop1_n2b
section         N2BFAS30
                add     ebx, ebx
                jncs    loop1_n2b
                jnz     loopend1_n2b
                getbit  32
                jncs    loop1_n2b
loopend1_n2b:
section         N2BDEC30
                xor     ecx, ecx
                sub     eax, 3
                jb      decompr_ebpeax_n2b
                shl     eax, 8
                mov     al, [esi]
                inc     esi
                xor     eax, -1
                jzs     decompr_end_n2b
                mov     ebp, eax
decompr_ebpeax_n2b:
                getbit  1
                adc     ecx, ecx
                getbit  1
                adc     ecx, ecx
                jnzs    decompr_got_mlen_n2b
                inc     ecx
loop2_n2b:
                getbit  1
                adc     ecx, ecx
section         N2BSMA40
                getbit  1
                jncs    loop2_n2b
section         N2BFAS40
                add     ebx, ebx
                jncs    loop2_n2b
                jnz     loopend2_n2b
                getbit  32
                jncs    loop2_n2b
loopend2_n2b:
section         N2BSMA50
                inc     ecx
                inc     ecx
section         N2BFAS50
                add     ecx, 2
section         N2BDEC50
decompr_got_mlen_n2b:
                cmp     ebp, -0xd00
                adc     ecx, 1
section         N2BSMA60
    #ifndef UPX102
                push    esi
    #else
                mov     edx, esi
    #endif
                lea     esi, [edi+ebp]
                rep
                movsb
    #ifndef UPX102
                pop     esi
    #else
                mov     esi, edx
    #endif
                jmp    decompr_loop_n2b
section         N2BFAS60
                lea     edx, [edi+ebp]
                cmp     ebp, -4
    #ifndef UPX102
        mov al, [edi+ecx]  // force data cache allocate (PentiumPlain or MMX)
    #endif
                jbes    decompr_copy4_n2b
loop3_n2b:
                mov     al, [edx]
                inc     edx
                mov     [edi], al
                inc     edi
                dec     ecx
                jnz     loop3_n2b
                jmp     decompr_loop_n2b
section         N2BFAS61
decompr_copy4_n2b:
                mov     eax, [edx]
                add     edx, 4
                mov     [edi], eax
                add     edi, 4
                sub     ecx, 4
                ja      decompr_copy4_n2b
                add     edi, ecx
                jmp     decompr_loop_n2b
section         N2BDEC60
decompr_end_n2b:

// vi:ts=8:et

