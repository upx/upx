;  l_vmlinz.asm -- loader & decompressor for the vmlinuz/i386 format
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


%define         jmps    jmp short
%define         jmpn    jmp near
%include        "macros.ash"

                BITS    32
                SECTION .text
                ORG     0

; =============
; ============= ENTRY POINT
; =============

start:
;       __LINUZ000__
                cli
                xor     eax, eax
                mov     al, 0x18
                mov     ds, eax
                mov     es, eax
                mov     ss, eax
                mov     esp, 'STAK'     ; 0x90000

                or      ebp, byte -1
                mov     eax, 'KEIP'     ; 0x100000 - address of startup_32
                push    eax
                push    edi
                push    esi
%ifdef  __LZCALLT1__
                push    eax
%endif; __LZDUMMY0__
%ifdef  __LBZIMAGE__
                mov     esi, 'ESI0'
                mov     edi, 'EDI0'
                mov     ecx, 'ECX0'

                std
                rep
                movsd
                cld

                mov     esi, 'ESI1'
                xchg    eax, edi
                jmp     .1 + 'JMPD'     ; jump to the copied decompressor
.1:
%else;  __LZIMAGE0__

                cld
                mov     esi, 'ESI1'
                xchg    eax, edi

; this checka20 stuff looks very unneccessary to me
checka20:
                inc     eax
                mov     [ebp + 1], eax
                cmp     [edi], eax
                je      checka20

%endif; __LZCUTPOI__

; =============
; ============= DECOMPRESSION
; =============

%include      "n2b_d32.ash"
%include      "n2d_d32.ash"

; =============
%ifdef  __LZCALLT9__
                pop     edi
                cjt32   0
%endif; __LINUZ990__
                pop     esi
                pop     edi
                xor     ebx, ebx        ; booting the 1st cpu
                retn

; =============
; ============= CUT HERE
; =============

%include        "header.ash"

eof:
;       __LITHEEND__
                section .data
                dd      -1
                dw      eof
