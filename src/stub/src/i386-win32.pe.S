/*
;  i386-win32.pe.S -- loader & decompressor for the w32/pe format
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2008 Laszlo Molnar
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
;  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
;
*/

#define         UPX102  1
#include "arch/i386/macros.S"


// =============
// ============= ENTRY POINT
// =============

section         PEISDLL1
                cmpb    [esp + 8], 1
                jnz     reloc_end_jmp
section         PEMAIN01
                pusha
                mov     esi, offset start_of_compressed       // relocated
                lea     edi, [esi + start_of_uncompressed]
section         PEICONS1
                incw    [edi + icon_offset]
section         PEICONS2
                addw    [edi + icon_offset], offset icon_delta
section         PETLSHAK
                mov     dword ptr [edi + tls_address], offset tls_value
section         PEMAIN02
                push    edi
section         PEMAIN03
                or      ebp, -1

// =============
// ============= DECOMPRESSION
// =============

#include "arch/i386/nrv2b_d32.S"
#include "arch/i386/nrv2d_d32.S"
#include "arch/i386/nrv2e_d32.S"
#include "arch/i386/lzma_d.S"

// =============
section         PEMAIN10
                pop     esi             // load vaddr

// =============
// ============= CALLTRICK
// =============

section         PECTTPOS
                lea     edi, [esi + filter_buffer_start]
section         PECTTNUL
                mov     edi, esi

                cjt32   esi

section         ctok32.00
                mov     ecx, offset filter_length
                ctok32  esi, (offset filter_cto)

// =============
// ============= IMPORTS
// =============

section PEIMPORT
                lea     edi, [esi + compressed_imports]
next_dll:
                mov     eax, [edi]
                or      eax, eax
                jzs     imports_done
                mov     ebx, [edi+4]    // iat
                lea     eax, [eax + esi + start_of_imports]
                add     ebx, esi
                push    eax
                add     edi, 8
                call    [esi + LoadLibraryA]
                xchg    eax, ebp
next_func:
                mov     al, [edi]
                inc     edi
                or      al, al
                jz      next_dll
                mov     ecx, edi        // something > 0
section         PEIBYORD
                jnss    byname
section         PEK32ORD
                jpe     not_kernel32
                mov     eax, [edi]
                add     edi, 4
                mov     eax, [eax + esi + kernel32_ordinals]
                jmps    next_imp
not_kernel32:
section         PEIMORD1
                movzxw  eax, [edi]
                inc     edi
                push    eax
                inc     edi
                .byte   0xb9            // mov ecx,xxxx
byname:
section         PEIMPOR2
                push    edi
                dec     eax
                repne
                scasb

                push    ebp
                call    [esi + GetProcAddress]
                or      eax, eax
                jz      imp_failed
next_imp:
                mov     [ebx], eax
                add     ebx, 4
                jmps    next_func
imp_failed:
section         PEIERDLL
                popa
                xor     eax, eax
                ret     0x0c
section         PEIEREXE
                call    [esi + ExitProcess]
section         PEIMDONE
imports_done:

// =============
// ============= RELOCATION
// =============

section         PERELOC1
                lea     edi, [esi + start_of_relocs]
section         PERELOC2
                add     edi, 4
section         PERELOC3
                lea     ebx, [esi - 4]
                reloc32 edi, ebx, esi

// =============

// FIXME: depends on that in PERELOC1 edi is set!!
section         PERLOHI0
                xchg    edi, esi
                lea     ecx, [edi + reloc_delt]

section         PERELLO0
                .byte   0xA9
rello0:
                add     [edi + eax], cx
                lodsd
                or      eax, eax
                jnz     rello0

// =============

section         PERELHI0
                shr     ecx, 16
                .byte   0xA9
relhi0:
                add     [edi + eax], cx
                lodsd
                or      eax, eax
                jnz     relhi0

// =============
section         PEDEPHAK
                mov     ebp, [esi + VirtualProtect]
                lea     edi, [esi + vp_base]
                mov     ebx, offset vp_size     // 0x1000 or 0x2000

                push    eax                     // provide 4 bytes stack

                push    esp                     // &lpflOldProtect on stack
                push    4                       // PAGE_READWRITE
                push    ebx
                push    edi
                call    ebp

  #if 0
                or      eax, eax
                jz      pedep9                  // VirtualProtect failed
  #endif

                lea     eax, [edi + swri]
                andb    [eax], 0x7f             // marks UPX0 non writeable
                andb    [eax + 0x28], 0x7f      // marks UPX1 non writeable

  #if 0
                push    esp
                push    2                       // PAGE_READONLY
  #else
                pop     eax
                push    eax
                push    esp
                push    eax                     // restore protection
  #endif
                push    ebx
                push    edi
                call    ebp

pedep9:
                pop     eax                     // restore stack

section         PEMAIN20
                popa


// clear the dirty stack
.macro          clearstack128  tmp_reg
                local   loop
                lea     \tmp_reg, [esp - 128]
loop:
                push    0
                cmp     esp, \tmp_reg
                jnzs    loop
                sub     esp, -128
.endm

section         CLEARSTACK
                clearstack128 eax

section         PEMAIN21
reloc_end_jmp:

section         PERETURN
                xor     eax, eax
                inc     eax
                ret     0x0C
section         PEDOJUMP
                jmp    original_entry

// =============
// ============= CUT HERE
// =============

#include        "include/header.S"

// vi:ts=8:et:nowrap
