;  l_vmlinz.asm -- loader & decompressor for the vmlinuz/i386 format
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2004 Laszlo Molnar
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

; =============
; ============= ENTRY POINT
; =============

start:
;       __LINUZ000__
                ;cli    ;this must be true already

        ; The only facts about segments here, that are true for all kernels:
        ; %cs is a valid "flat" code segment; no other segment reg is valid;
        ; the next segment after %cs is a valid "flat" data segment, but
        ; no segment register designates it yet.
                mov eax, cs
                add eax, byte 1<<3      ; the next segment after %cs
                mov ds, eax
                mov es, eax
        ; fs, gs set by startup_32 in arch/i386/kernel/head.S

        ; Linux Documentation/i386/boot.txt "SAMPLE BOOT CONFIGURATION" says
        ; 0x8000-0x8FFF  Stack and heap  [inside the "real mode segment",
        ; just below the command line at offset 0x9000].

        ; arch/i386/boot/compressed/head.S "Do the decompression ..." says
        ; %esi contains the "real mode pointer" [as a 32-bit addr].

        ; In any case, avoid EBDA (Extended BIOS Data Area) below 0xA0000.
        ; boot.txt says 0x9A000 is the limit.  LILO goes up to 0x9B000.

                lea ecx, ['STAK' + esi]  ; (0x9000 + 0x90000) typical
                mov      [-8 + ecx], ecx  ; 32-bit offset for stack pointer
                mov      [-4 + ecx], eax  ; segment for stack
                lss esp, [-8 + ecx]       ; %ss:%esp= %ds:0x99000

                push    byte 0
                popf            ; BIOS can leave random flags (such as NT)

; do not clear .bss: at this point, .bss comes only from
; arch/i386/boot/compressed/*.o  which we are replacing entirely

                or      ebp, byte -1    ; decompressor assumption
                mov     eax, 'KEIP'     ; 0x100000 : address of startup_32
                push    cs      ; MATCH00
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
;;%include      "cl1_d32.ash"

; =============
; ============= UNFILTER
; =============

%ifdef  __LZCKLLT9__
                pop     ecx     ; MATCH05  len
                pop     edx     ; MATCH04  cto
                pop     edi     ; MATCH03  src

                ckt32   0, dl   ; dl has cto8
        ;0: Filter.addvalue = kernel_entry already did the 'add' at filter time
        ;[the runtime address of the destination was known], so we save 4 bytes
        ;(plus 1 cycle per instance) by not doing the 'add' when unfiltering.
        ;If .addvalue was 0, then use 'edi' instead of 0 in call to ckt32,
        ;to compensate for difference in origin of buffer.

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
