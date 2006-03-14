;  l_lx_pti86.asm -- Linux separate ELF PT_INTERP
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
                CPU     386

%define         jmps    jmp short
%define         jmpn    jmp near

; /*************************************************************************
; // program entry point
; // see glibc/sysdeps/i386/elf/start.S
; **************************************************************************/

GLOBAL _start
;__LXPTI000__
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
        call L200  ; push address of get_funf
get_funf:
        cmp eax, byte 0x46
        mov ecx, unf46
        je L110
        cmp eax, byte 0x49
        mov ecx, unf49
        je L110
L120:
        mov ecx, none
L110:
        mov eax, ecx
none:
        ret

%define M_NRV2B_LE32    2
%define M_NRV2D_LE32    5
%define M_NRV2E_LE32    8
%define M_CL1B_LE32     11

L200:
        call L300  ; push address of get_fexp
get_fexp:
        cmp eax, byte M_NRV2B_LE32
        mov ecx, nrv2b
        je L110
        cmp eax, byte M_NRV2D_LE32
        mov ecx, nrv2d
        je L110
        cmp eax, byte M_NRV2E_LE32
        mov ecx, nrv2e
        je L110
        cmp eax, byte M_CL1B_LE32
        mov ecx, cl1b
        je L110
        jmpn L120

; /*************************************************************************
; // C callable decompressor
; **************************************************************************/
;__LXPTI040__
nrv2b:
;__LXPTI041__
nrv2d:
;__LXPTI042__
nrv2e:
;__LXPTI043__
cl1b:

%define         INP     dword [esp+8*4+1*4]
%define         INS     dword [esp+8*4+2*4]
%define         OUTP    dword [esp+8*4+3*4]
%define         OUTS    dword [esp+8*4+4*4]

;__LXPTI050__
                pusha
                ; cld
                or      ebp, byte -1
                mov     esi, INP
                mov     edi, OUTP
;;;             align   8

%include      "n2b_d32.ash"
%include      "n2d_d32.ash"
%include      "n2e_d32.ash"
%include      "cl1_d32.ash"
;__LXPTI090__
                jmpn exp_done
;__LXPTI091__
                ; eax is 0 from decompressor code
                ;xor     eax, eax               ; return code
exp_done:
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

                mov [7*4 + esp], eax
                popa
                ret

%include      "macros.ash"
                cjt32 0
                ctojr32

;__LXPTI140__
unf46:
;__LXPTI141__
unf49:

%define         CTO8    dword [esp+8*4+3*4]

;__LXPTI150__
                pusha
                mov edi,INP
                mov ecx,INS
                mov edx,CTO8

                ckt32   edi, dl

;__LXPTI160__
                popa
                ret

;__LXPTI200__
L300:

eof:
;       __XTHEENDX__
        section .data
        dd      -1
        dw      eof

; vi:ts=8:et:nowrap

