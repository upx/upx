;  l_vmlinx.asm -- loader & decompressor for the vmlinux/i386 format
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2004 Laszlo Molnar
;  Copyright (C)      2004 John Reiser
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
;  John Reiser
;  <jreiser@users.sourceforge.net>


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
;  In:
;       %eax= &uncompressed [and final entry]; %ds= %es= __BOOT_DS
;       %esp: &compressed; __BOOT_CS
;       __LINUX000__
                pop     edx  ; &compressed; length at -4(%edx)

                push    eax     ; MATCH00(1/2)  entry address; __BOOT_CS
                push    edi     ; MATCH01  save
                push    esi     ; MATCH02  save

%ifdef  __LXCALLT1__
                push    eax     ; MATCH03  src unfilter
%endif; __LXDUMMY0__
%ifdef  __LXCKLLT1__
                push    eax     ; MATCH03  src unfilter
                push    byte '?'  ; MATCH04  cto unfilter
%endif; __LXMOVEUP__
                push    'ULEN'  ; MATCH05  uncompressed length
                call move_up    ; MATCH06

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

%ifdef  __LXCKLLT9__
                pop     ecx     ; MATCH05  len
                pop     edx     ; MATCH04  cto
                pop     edi     ; MATCH03  src

                ckt32   edi, dl ; dl has cto8
        ;edi: adjust for the difference between 0 origin of buffer at filter,
        ;and actual origin of destination at unfilter.
        ;Filter.addvalue is 0: destination origin is unknown at filter time.
        ;The input data is still relocatable, and address is assigned later
        ;[as of 2004-12-15 it is 'always' 0x100000].

%endif; __LXDUMMY2__
%ifdef  __LXCALLT9__
                pop     ecx     ; MATCH05  len
                pop     edi     ; MATCH03  src
                cjt32   0
%endif; __LINUX990__
                pop     esi     ; MATCH02  restore
                pop     edi     ; MATCH01  restore
                xor     ebx, ebx        ; booting the 1st cpu
                retf    ; MATCH00  set cs

%define UNLAP 0x10
%define ALIGN (~0<<4)
        ; must have 0==(UNLAP &~ ALIGN)

move_up:
                pop esi           ; MATCH06  &decompressor
                mov ecx,[-4+ esi] ; length of decompressor+unfilter
                mov ebp,eax       ; &uncompressed
                add eax,[esp]     ; MATCH05  ULEN + base; entry to decompressor
                add eax, byte ~ALIGN + UNLAP
                and eax, byte  ALIGN

                std
        ; copy decompressor
                lea esi,[-1+ ecx + esi]  ; unmoved top -1 of decompressor
                lea edi,[-1+ ecx + eax]  ;   moved top -1 of decompressor
                rep
                movsb

                mov ecx,[-4+ edx]  ; length of compressed data
                add ecx, byte  3
                shr ecx,2          ; count of .long
        ; copy compressed data
                lea esi,[-4+ 4*ecx + edx] ; unmoved top -4 of compressed data
                lea edi,[-4+         eax] ;   moved top -4 of compressed data
                rep
                movsd

                cld
                lea esi,[4+ edi]   ;   &compressed [after move]
                mov edi,ebp        ; &uncompressed
                or  ebp, byte -1   ; decompressor assumption
                jmp eax            ; enter moved decompressor

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
