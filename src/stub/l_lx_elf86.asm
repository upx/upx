;  l_lx_elf86.asm -- Linux program entry point & decompressor (Elf binary)
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2004 Laszlo Molnar
;  Copyright (C) 2000-2004 John F. Reiser
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
;  John F. Reiser
;  <jreiser@users.sourceforge.net>
;


                BITS    32
                SECTION .text

%define         jmps    jmp short
%define         jmpn    jmp near

; /*************************************************************************
; // program entry point
; // see glibc/sysdeps/i386/elf/start.S
; **************************************************************************/

GLOBAL _start
;__LEXEC000__
_start:
;;;;    int3
;; How to debug this code:  Uncomment the 'int3' breakpoint instruction above.
;; Build the stubs and upx.  Compress a testcase, such as a copy of /bin/date.
;; Invoke gdb, and give a 'run' command.  Define a single-step macro such as
;;      define g
;;      stepi
;;      x/i $pc
;;      end
;; and a step-over macro such as
;;      define h
;;      x/2i $pc
;;      tbreak *$_
;;      continue
;;      x/i $pc
;;      end
;; Step through the code; remember that <Enter> repeats the previous command.
;;
        call main  ; push address of decompress subroutine
decompress:

; /*************************************************************************
; // C callable decompressor
; **************************************************************************/

%define         INP     dword [esp+8*4+8]
%define         INS     dword [esp+8*4+12]
%define         OUTP    dword [esp+8*4+16]
%define         OUTS    dword [esp+8*4+20]

;__LEXEC009__
        ;;  empty section for commonality with l_lx_exec86.asm
;__LEXEC010__
                pusha
                push    byte '?'  ; cto8 (sign extension does not matter)
                ; cld

                mov     esi, INP
                mov     edi, OUTP

                or      ebp, byte -1
;;;             align   8

%include      "n2b_d32.ash"
%include      "n2d_d32.ash"
%include      "n2e_d32.ash"
%include      "macros.ash"
                cjt32 0

;__LEXEC015__
                ; eax is 0 from decompressor code
                ;xor     eax, eax               ; return code

; check compressed size
                mov     edx, INP
                add     edx, INS
                cmp     esi, edx
                jz      .ok
                dec     eax
.ok:

; write back the uncompressed size
                sub     edi, OUTP
                mov     edx, OUTS
                mov     [edx], edi

                pop edx  ; cto8

                mov [7*4 + esp], eax
                popa
                ret

                ctojr32
                ckt32   edi, dl
;__LEXEC017__
                popa
                ret

;__LEXEC020__

%define PAGE_SIZE ( 1<<12)

%define MAP_FIXED     0x10
%define MAP_PRIVATE   0x02
%define MAP_ANONYMOUS 0x20
%define PROT_READ      1
%define PROT_WRITE     2
%define PROT_EXEC      4
%define __NR_mmap     90

; Decompress the rest of this loader, and jump to it
unfold:
        pop esi  ; &{ sz_uncompressed, sz_compressed, compressed_data...}
        cld
        lodsd
        push eax  ; sz_uncompressed  (junk, actually)
        push esp  ; &sz_uncompressed
        mov eax, 0x400000
        push eax  ; &destination

                ; mmap a page to hold the decompressed program
        xor ecx, ecx
        push ecx
        push ecx
        mov ch, PAGE_SIZE >> 8
        push byte MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS
        push byte PROT_READ | PROT_WRITE | PROT_EXEC
        push ecx  ; length
        push eax  ; destination
        mov ebx, esp  ; address of parameter vector for __NR_mmap
        push byte __NR_mmap
        pop eax
        int 0x80
        xchg eax, ebx
        mov bh, PAGE_SIZE>>8  ; ebx= 0x401000
        add esp, byte 6*4  ; discard args to mmap

        lodsd
        push eax  ; sz_compressed
        lodsd  ; junk cto8, algo, unused[2]
        push esi  ; &compressed_data
        call ebp  ; decompress(&src, srclen, &dst, &dstlen)
        pop eax  ; discard &compressed_data
        pop eax  ; discard sz_compressed
        ret      ; &destination
main:
        pop ebp  ; &decompress
        call unfold

eof:
;       __XTHEENDX__
        section .data
        dd      -1
        dw      eof

; vi:ts=8:et:nowrap

