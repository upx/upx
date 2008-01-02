/*
;  i386-linux.kernel.vmlinux.S -- loader & decompressor for the vmlinux/i386 format
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2008 Laszlo Molnar
;  Copyright (C) 2004-2008 John Reiser
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
;  John Reiser
;  <jreiser@users.sourceforge.net>
*/

#include "arch/i386/macros.S"


/*
; =============
; ============= ENTRY POINT
; =============

;  In:
;       #eax= &uncompressed [and final entry]; #ds= #es= __BOOT_DS
;       #esp: &compressed; __BOOT_CS

  How to debug: run under qemu (http://fabrice.bellard.free.fr/qemu/)
  after un-commenting the 0xf1 opcode below.  That opcode forces qemu
  to stop in gdb.  You'll have to "set $pc+=1" by hand.
*/
section         LINUX000
////    .byte 0xf1  // qemu In-Circuit-Emulator breakpoint

#if 0  /*{*/
// VGA debugging; In: edi= &putstr (linux/arch/*/boot/compressed/misc.c) */
        jmp .L10
print_hex:  // %eax in " %.8x"; ebx= &putstr;  clobbers edi,edx,ecx
        sub esp,12
        mov edi,esp
        mov edx,eax
        cld

        mov al,' '
        stosb
        mov ecx,8
.L30:
        rol edx,4
        mov al,dl
        and al,0xf
        cmp al,10
        sbb al,0x69
        das
        stosb
        loop .L30
        mov al,0
        stosb

        push esp
        call ebx
        add esp,4+ 3*4
        ret

print_stk8:  // eax= &putstr
        push ebp
        mov ebp,esp
        push ebx
        mov ebx,eax  // &putstr
        push esi
        push edi

        mov esi,-8
.L20:
        mov eax,[ebp + 4*esi + 8*4 + 8]
        call print_hex
        inc esi
        jnz .L20

        sub esp,4
        mov edi,esp
        push edi
        mov al,'\n'
        stosb
        mov al,0
        stosb
        call ebx  // NL
        add esp,4+ 4

        pop edi
        pop esi
        pop ebx
        pop ebp
        ret
.L10:
        mov ebx,edi  // &putstr in better register
#endif  /*}*/

                pop     edx     // &compressed; length at -4(#edx)
                push %cs
                push offset ENTRY_POINT  // MATCH00
                mov eax, offset PHYSICAL_START  // destination of uncompression

                push    edi     // MATCH01  save
                push    esi     // MATCH02  save

section         LXCALLT1
                push    eax     // MATCH03  src unfilter
section         LXCKLLT1
                push    eax     // MATCH03  src unfilter
                //push   offset filter_cto // MATCH04  cto unfilter
                .byte   0x6a, filter_cto   // MATCH04  cto unfilter
section         LXMOVEUP
                push    offset filter_length // MATCH05  uncompressed length
                call move_up    // MATCH06

// =============
// ============= DECOMPRESSION
// =============

#include "arch/i386/nrv2b_d32.S"
#include "arch/i386/nrv2d_d32.S"
#include "arch/i386/nrv2e_d32.S"
#include "arch/i386/lzma_d.S"

// =============
// ============= UNFILTER
// =============

section         LXCKLLT9
                pop     ecx     // MATCH05  len
                pop     edx     // MATCH04  cto
                pop     edi     // MATCH03  src

                ctok32  edi, dl // dl has cto8
/*
        ;edi: adjust for the difference between 0 origin of buffer at filter,
        ;and actual origin of destination at unfilter.
        ;Filter.addvalue is 0: destination origin is unknown at filter time.
        ;The input data is still relocatable, and address is assigned later
        ;[as of 2004-12-15 it is 'always' 0x100000].
*/

section         LXCALLT9
                pop     ecx     // MATCH05  len
                pop     edi     // MATCH03  src
                cjt32   0
section         LINUX990
                pop     esi     // MATCH02  restore
                pop     edi     // MATCH01  restore
                xor     ebx, ebx        // booting the 1st cpu
                lret    // MATCH00  set cs

#define UNLAP 0x10
#define ALIGN (~0<<4)
        // must have 0==(UNLAP &~ ALIGN)

move_up:
                pop esi           // MATCH06  &decompressor
                mov ecx,[-4+ esi] // length of decompressor+unfilter
                mov ebp,eax       // &uncompressed
                add eax,[esp]     // MATCH05  ULEN + base; entry to decompressor
                add eax, ~ALIGN + UNLAP
                and eax, ALIGN

#if 0  /*{*/
  pusha
  mov eax,ebx  // &putstr
  call print_stk8
  popa
.L5:
        pause
        jmp .L5
#endif  /*}*/
                std
        // copy decompressor
                lea esi,[-1+ ecx + esi]  // unmoved top -1 of decompressor
                lea edi,[-1+ ecx + eax]  //   moved top -1 of decompressor
                rep
                movsb

                mov ecx,[-4+ edx]  // length of compressed data
                add ecx, 3
                shr ecx,2          // count of .long
        // copy compressed data
                lea esi,[-4+ 4*ecx + edx] // unmoved top -4 of compressed data
                lea edi,[-4+         eax] //   moved top -4 of compressed data
                rep
                movsd

                cld
                lea esi,[4+ edi]   //   &compressed [after move]
                mov edi,ebp        // &uncompressed

section         LINUX991
                or  ebp, -1        // decompressor assumption
section         LINUX992
                jmp eax            // enter moved decompressor

#include "include/header.S"

// vi:ts=8:et:nowrap
