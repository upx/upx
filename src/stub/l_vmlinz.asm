;  l_vmlinz.asm -- loader & decompressor for the vmlinuz/i386 format
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2003 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2003 Laszlo Molnar
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


%define         jmps    jmp short
%define         jmpn    jmp near
%include        "macros.ash"

                BITS    32
                SECTION .text
                ORG     0

; gdt segment 3 is flat data
%define __KERNEL_DS 3*8
; gdt segment 2 is flat code
%define __KERNEL_CS 2*8

; =============
; ============= ENTRY POINT
; =============

start:
;       __LINUZ000__
                cli
                xor     eax, eax
                mov     al, __KERNEL_DS
                mov     ds, eax
                mov     es, eax
; fs, gs set by startup_32 in arch/i386/kernel/head.S 
                mov     ss, eax
                mov     esp, 'STAK'     ; 0x90000

                push    byte 0
                popf            ; BIOS can leave random flags (such as NT)

; do not clear .bss: at this point, .bss comes only from
; arch/i386/boot/compressed/*.o  which we are replacing entirely

                or      ebp, byte -1    ; decompressor assumption
                mov     eax, 'KEIP'     ; 0x100000 : address of startup_32
                push    byte __KERNEL_CS        ; MATCH00
                push    eax     ; MATCH00  entry address
                push    edi     ; MATCH01  save
                push    esi     ; MATCH02  save

%ifdef  __LZCALLT1__
                push    eax     ; MATCH03  src unfilter
%endif; __LZDUMMY0__
%ifdef  __LZCKLLT1__
                push    eax     ; MATCH03  src unfilter
                push    byte '?'  ; MATCH04  cto unfilter
                push    'ULEN'    ; MATCH05  len unfilter
%endif; __LZDUMMY1__
%ifdef  __LBZIMAGE__
                mov     esi, 'ESI0'
                mov     edi, 'EDI0'
                mov     ecx, 'ECX0'

                std
                rep
                movsd
                cld

                mov     esi, 'ESI1'     ; esi = src for decompressor
                xchg    eax, edi        ; edi = dst for decompressor = 0x100000
                jmp     .1 + 'JMPD'     ; jump to the copied decompressor
.1:
%else;  __LZIMAGE0__

; this checka20 stuff looks very unneccessary to me
checka20:
                inc                edi  ; change value
                mov     [1 + ebp], edi  ; store to 0x000000 (even megabyte)
                cmp         [eax], edi  ; compare  0x100000 ( odd megabyte)
                je      checka20        ; addresses are [still] aliased

                cld
                mov     esi, 'ESI1'
                xchg    eax, edi        ; edi = dst for decompressor = 0x100000

%endif; __LZCUTPOI__

; =============
; ============= DECOMPRESSION
; =============

%include      "n2b_d32.ash"
%include      "n2d_d32.ash"
%include      "n2e_d32.ash"

; =============
; ============= UNFILTER
; =============

%ifdef  __LZCKLLT9__
                pop     ecx     ; MATCH05  len
                pop     edx     ; MATCH04  cto
                pop     edi     ; MATCH03  src
                ckt32   dl
%endif; __LZDUMMY2__
%ifdef  __LZCALLT9__
                pop     edi     ; MATCH03  src
                cjt32   0
%endif; __LINUZ990__
                pop     esi     ; MATCH02  restore
                pop     edi     ; MATCH01  restore
                xor     ebx, ebx        ; booting the 1st cpu
                retf    ; MATCH00  set cs

; =============
; ============= CUT HERE
; =============

%include        "header.ash"

eof:
;       __LITHEEND__
                section .data
                dd      -1
                dw      eof


; vi:ts=8:et:nowrap
