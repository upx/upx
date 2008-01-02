/*
;  n2d_d32.ash -- ucl_nrv2d_decompress_le32 in 32-bit assembly
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

.macro          getbit_n2d one
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
#define getbit  getbit_n2d


section         N2DSMA10
                jmps    dcl1_n2d
decompr_literals_n2d:
                movsb
section         N2DFAS10
                jmps    dcl1_n2d
section         N2DFAS11
decompr_literalb_n2d:
                mov     al, [esi]
                inc     esi
                mov     [edi], al
                inc     edi

section         N2DDEC10
decompr_loop_n2d:
                add     ebx, ebx
                jnz     dcl2_n2d
dcl1_n2d:
                getbit  32
dcl2_n2d:
section         N2DSMA20
                jcs     decompr_literals_n2d
                xor     eax, eax
                inc     eax
section         N2DFAS20
    #ifndef UPX102
        mov al, [edi]  // force data cache allocate (PentiumPlain or MMX)
    #endif
                jcs     decompr_literalb_n2d
                mov     eax, 1
section         N2DDEC20
loop1_n2d:
                getbit  1
                adc     eax, eax
section         N2DSMA30
                getbit  1
                jcs     loopend1_n2d
section         N2DFAS30
                add     ebx, ebx
                jnc     loopcontinue1_n2d
                jnzs    loopend1_n2d
                getbit  32
                jcs     loopend1_n2d
loopcontinue1_n2d:
section         N2DDEC30
                dec     eax
                getbit  1
                adc     eax, eax
                jmps    loop1_n2d
loopend1_n2d:
                xor     ecx, ecx
                sub     eax, 3
                jb      decompr_prev_dist_n2d
                shl     eax, 8
                mov     al, [esi]
                inc     esi
                xor     eax, -1
                jzs     decompr_end_n2d
                sar     eax, 1                  // shift low-bit into carry
                mov     ebp, eax
                jmps    decompr_ebpeax_n2d
decompr_prev_dist_n2d:
                getbit  1
decompr_ebpeax_n2d:
                adc     ecx, ecx
                getbit  1
                adc     ecx, ecx
                jnzs    decompr_got_mlen_n2d
                inc     ecx
loop2_n2d:
                getbit  1
                adc     ecx, ecx
section         N2DSMA40
                getbit  1
                jncs    loop2_n2d
section         N2DFAS40
                add     ebx, ebx
                jncs    loop2_n2d
                jnz     loopend2_n2d
                getbit  32
                jncs    loop2_n2d
loopend2_n2d:
section         N2DSMA50
                inc     ecx
                inc     ecx
section         N2DFAS50
                add     ecx, 2
section         N2DDEC50
decompr_got_mlen_n2d:
                cmp     ebp, -0x500
                adc     ecx, 1
section         N2DSMA60
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
                jmp     decompr_loop_n2d
section         N2DFAS60
                lea     edx, [edi+ebp]
                cmp     ebp, -4
    #ifndef UPX102
        mov al, [edi+ecx]  // force data cache allocate (PentiumPlain or MMX)
    #endif
                jbes    decompr_copy4_n2d
loop3_n2d:
                mov     al, [edx]
                inc     edx
                mov     [edi], al
                inc     edi
                dec     ecx
                jnz     loop3_n2d
                jmp     decompr_loop_n2d
section         N2DFAS61
decompr_copy4_n2d:
                mov     eax, [edx]
                add     edx, 4
                mov     [edi], eax
                add     edi, 4
                sub     ecx, 4
                ja      decompr_copy4_n2d
                add     edi, ecx
                jmp     decompr_loop_n2d
section         N2DDEC60
decompr_end_n2d:

// vi:ts=8:et

