/*
;  i386-dos32.watcom.le.S -- loader & decompressor for the watcom/le format
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

#include        "arch/i386/macros.S"


section         WCLEMAIN
                .byte   0xbf            // mov edi, 'alib'
                .ascii  "alib"          // address of obj#1:0 (filled by a fixup record)
/*
; The following hack fools the lame protection of dos4g/w, which expects the
; 'WATCOM' string somewhere in the first 18 bytes after the entry point
; I use this imul thingy, because it's 1 byte shorter than a jump ;-)
; ... and "alibiWATCOM" looks cool
*/
                .ascii  "iWATCOM"       // imul edx,[edi+0x41],'TCOM'

                push    es
                push    ds
                pop     es
                push    edi

                lea     esi, [edi + copy_source]
                lea     edi, [edi + copy_dest]
                mov     ecx, offset words_to_copy

                std
                rep
                movsd
                cld

                lea     esi, [edi + 4]
                pop     edi
section         WCLEMAIN02
                or      ebp, -1
section         WCLEMAIN03
                push    edi
                jmp     decompressor

#include        "include/header.S"

section         WCLECUTP
decompressor:

// =============
// ============= DECOMPRESSION
// =============

#include "arch/i386/nrv2b_d32.S"
#include "arch/i386/nrv2d_d32.S"
#include "arch/i386/nrv2e_d32.S"
#include "arch/i386/lzma_d.S"

// =============

section WCLEMAI2
                pop     ebp
                push    esi
                lea     esi, [ebp + start_of_relocs]
                push    esi

// =============
// ============= CALLTRICK
// =============

section WCCTTPOS
                lea     edi, [ebp + filter_buffer_start]
section WCCTTNUL
                mov     edi, ebp

                cjt32   ebp

section         ctok32.00
                mov     ecx, offset filter_length
                ctok32  ebp, (offset filter_cto)

// =============
// ============= RELOCATION
// =============

section WCRELOC1
                lea     edi, [ebp - 4]
                reloc32 esi, edi, ebp
//               eax = 0

section WCRELSEL
                call    esi             // selector fixup code (modifies bx)

section WCLEMAI4
                pop     edi
                pop     ecx
                sub     ecx, edi
                shr     ecx, 2
                rep
                stosd                   // clear dirty memory
                pop     es
                lea     esp, [ebp + original_stack]

                jmp     original_entry

// vi:ts=8:et:nowrap
