/*
;  i386-dos32.djgpp2.S -- loader & decompressor for the djgpp2/coff format
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


section         DJ2MAIN1
                push    ds
                pop     es

                mov     esi, offset start_of_compressed
                mov     edi, offset start_of_uncompressed
section         DJCALLT1
                push    edi
section         DJ2MAIN2
                or      ebp, -1

section         LZMA_INIT_STACK

/*
; as this stub gets loaded from 0x000000a8, we have some scratch
; memory starting from 0x00000000 to store ss:esp
; note: NULL page protection is only activated by the
; uncompressed application later
*/
                xor     eax, eax
                mov     [eax], ss
                mov     [eax + 4], esp
                mov     eax, ds
                mov     ss, eax
                mov     esp, offset stack_for_lzma

/*
; =============
; ============= DECOMPRESSION
; =============
*/

#include "arch/i386/nrv2b_d32.S"
#include "arch/i386/nrv2d_d32.S"
#include "arch/i386/nrv2e_d32.S"
#include "arch/i386/lzma_d.S"

section LZMA_DONE_STACK
                mov     ss, [eax]               // eax is always 0 here
                mov     esp, [eax + 4]

section DJ2BSS00
                mov     ecx, offset length_of_bss
                rep
                stosd

section DJCALLT2
                pop     edi

                cjt32   0

section         ctok32.00
                mov     ecx, offset filter_length
                ctok32  0, (offset filter_cto)

section DJRETURN
                jmp     original_entry

/*
; because of a feature of the djgpp loader, the size of this stub must be
; a multiple of 4 and as the upx decompressor depends on the fact that
; the compressed data stream begins just after the header,
; so the header section should be 4 byte aligned
*/

#include        "include/header.S"

// vi:ts=8:et:nowrap
