;  l_w32pe.asm -- loader & decompressor for the w32/pe format
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2006 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2006 Laszlo Molnar
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


%define         UPX102  1
%define         jmps    jmp short
%define         jmpn    jmp near
%define         jnzn    jnz near
%define         jbn     jb near
%include        "macros.ash"

                BITS    32
                SECTION .text
                ORG     0
                CPU     386

; =============
; ============= ENTRY POINT
; =============

%ifdef  __PEISDLL1__
                cmp     byte [esp + 8], 1
                jnzn    reloc_end_jmp
%endif; __PEMAIN01__
                pushad
                mov     esi, 'ESI0'     ; relocated
                lea     edi, [esi + 'EDI0']
%ifdef  __PEICONS1__
                inc     word [edi + 'ICON']
%else;  __PEICONS2__
                add     word [edi + 'ICON'],'DR'
%endif; __PEICONSZ__
%ifdef  __PETLSHAK__
                mov     dword [edi + 'TLSA'],'TLSV'
%endif; __PEMAIN02__
                push    edi
mpass:
                or      ebp, byte -1

; =============
; ============= DECOMPRESSION
; =============

%include      "n2b_d32.ash"
%include      "n2d_d32.ash"
%include      "n2e_d32.ash"

; =============

%ifdef  __PEMULTIP__
                lodsd
                add     edi, eax
                jbn     mpass
%endif; __PEMAIN10__

; =============
                pop     esi             ; load vaddr

; =============
; ============= CALLTRICK
; =============

%ifdef  __PECALLTR__
%ifdef  __PECTTPOS__
                lea     edi, [esi + 'TEXV']
%else;  __PECTTNUL__
                mov     edi, esi
%endif; __PEDUMMY0__
                cjt32   esi
%endif; __PEDUMMY1__

; =============
; ============= IMPORTS
; =============

%ifdef  __PEIMPORT__
                lea     edi, [esi + 'BIMP']
next_dll:
                mov     eax, [edi]
                or      eax, eax
                jz      imports_done
                mov     ebx, [edi+4]    ; iat
                lea     eax, [eax + esi + 'IMPS']
                add     ebx, esi
                push    eax
                add     edi, byte 8
                call    [esi + 'LOAD']  ; LoadLibraryA
                xchg    eax, ebp
next_func:
                mov     al, [edi]
                inc     edi
                or      al, al
                jz      next_dll
                mov     ecx, edi        ; something > 0
%ifdef  __PEIBYORD__
                jns     byname
%ifdef  __PEK32ORD__
                jpe     not_kernel32
                mov     eax, [edi]
                add     edi, byte 4
                mov     eax, [eax + esi + 'K32O']
                jmps    next_imp
not_kernel32:
%endif; __PEIMORD1__
                movzx   eax, word [edi]
                inc     edi
                push    eax
                inc     edi
                db      0xb9            ; mov ecx,xxxx
byname:
%endif; __PEIMPOR2__
                push    edi
                dec     eax
                repne
                scasb

                push    ebp
                call    [esi + 'GETP']  ; GetProcAddr
                or      eax, eax
                jz      imp_failed
next_imp:
                mov     [ebx], eax
                add     ebx, byte 4
                jmps    next_func
imp_failed:
%ifdef  __PEIERDLL__
                popad
                xor     eax, eax
                retn    0x0c
%else;  __PEIEREXE__
                call    [esi + 'EXIT']  ; ExitProcess
%endif; __PEIMDONE__
imports_done:
%endif; __PEIMPOR9__

; =============
; ============= RELOCATION
; =============

%ifdef  __PERELOC1__
                lea     edi, [esi + 'BREL']
;       __PERELOC2__
                add     edi, byte 4
;       __PERELOC3__
                lea     ebx, [esi - 4]
                reloc32 edi, ebx, esi
%endif; __PERELOC9__

; =============

; FIXME: depends on that in PERELOC1 edi is set!!
%ifdef  __PERLOHI0__
                xchg    edi, esi
                lea     ecx, [edi + 'DELT']
%endif; __PERLOHIZ__

%ifdef  __PERELLO0__
                db      0xA9
rello0:
                add     [edi + eax], cx
                lodsd
                or      eax, eax
                jnz     rello0
%endif; __PERELLOZ__

; =============

%ifdef  __PERELHI0__
                shr     ecx, 16
                db      0xA9
relhi0:
                add     [edi + eax], cx
                lodsd
                or      eax, eax
                jnz     relhi0
%endif; __PERELHIZ__

; =============
%ifdef  __PEDEPHAK__
                mov     ebp, [esi + 'VPRO']     ; VirtualProtect
                lea     edi, [esi + 'IMGB']
                mov     ebx, 'IMGL'             ; 0x1000 or 0x2000

                push    eax                     ; provide 4 bytes stack

                push    esp                     ; &lpflOldProtect on stack
                push    byte 4                  ; PAGE_READWRITE
                push    ebx
                push    edi
                call    ebp

  %if 0
                or      eax, eax
                jz      pedep9                  ; VirtualProtect failed
  %endif

                lea     eax, [edi + 'SWRI']
                and     byte [eax], 0x7f        ; marks UPX0 non writeable
                and     byte [eax + 0x28], 0x7f ; marks UPX1 non writeable

  %if 0
                push    esp
                push    byte 2                  ; PAGE_READONLY
  %else
                pop     eax
                push    eax
                push    esp
                push    eax                     ; restore protection
  %endif
                push    ebx
                push    edi
                call    ebp

pedep9:
                pop     eax                     ; restore stack
%endif; __PEDEPHAX__

;       __PEMAIN20__
                popad


; clear the dirty stack
%macro          clearstack128  1
                lea     %1, [esp - 128]
%%clearst0:
                push    byte 0
                cmp     esp, %1
                jnz     %%clearst0
                sub     esp, byte -128
%endmacro

%ifdef  __PERETURN_CLEARSTACK__
                clearstack128 eax
%endif; __PERETURN_CLEARSTACK9__
%ifdef  __PEDOJUMP_CLEARSTACK__
                clearstack128 eax
%endif; __PEDOJUMP_CLEARSTACK9__


;       __PEMAIN21__
reloc_end_jmp:

%ifdef  __PERETURN__
                xor     eax, eax
                inc     eax
                retn    0x0C
%else;  __PEDOJUMP__
                jmpn    .1+'JMPO'
.1:
%endif; __PEDUMMY3__

; =============
; ============= CUT HERE
; =============

%include        "header.ash"

eof:
;       __PETHEEND__
                section .data
                dd      -1
                dw      eof


; vi:ts=8:et:nowrap
