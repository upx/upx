;  l_lx_exec86.asm -- Linux program entry point & decompressor (execve)
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2000 Laszlo Molnar
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
;  Markus F.X.J. Oberhumer                   Laszlo Molnar
;  markus.oberhumer@jk.uni-linz.ac.at        ml1050@cdata.tvnet.hu
;


                BITS    32
                SECTION .text

%define         jmps    jmp short

; defines for ident.ash and n2b_d32.ash
%ifdef SMALL
  %define       __IDENTSMA__
  %define       __N2BSMA10__
  %define       __N2BSMA20__
  %define       __N2BSMA30__
  %define       __N2BSMA40__
  %define       __N2BSMA50__
  %define       __N2BSMA60__
  %define       __N2DSMA10__
  %define       __N2DSMA20__
  %define       __N2DSMA30__
  %define       __N2DSMA40__
  %define       __N2DSMA50__
  %define       __N2DSMA60__
%endif


; /*************************************************************************
; // program entry point
; // see glibc/sysdeps/i386/elf/start.S
; **************************************************************************/

GLOBAL _start
EXTERN upx_main

_start:
                xor     ebp, ebp            ; Clear the frame pointer
%if 0
                ; personality(PER_LINUX)
                mov     eax, 136            ; syscall_personality
                xor     ebx, ebx            ; PER_LINUX
                int     0x80
%endif
                pop     eax                 ; Pop the argument count
                mov     ecx, esp            ; argv starts just at the current stack top
                lea     edx, [ecx+eax*4+4]  ; envp = &argv[argc + 1]
                push    eax                 ; Restore the stack
                and     esp, byte -8        ; Align the stack
                push    edx                 ; Push third argument: envp
                push    ecx                 ; Push second argument: argv
;;;             push    eax                 ; Push first argument: argc
                call    upx_main            ; Call the UPX main function
                hlt                         ; Crash if somehow upx_main does return

%include        "ident.ash"


; /*************************************************************************
; // C callable decompressor
; **************************************************************************/

%ifdef NRV2B
  %define decompress    nrv2b_decompress_asm_fast
%elifdef NRV2D
  %define decompress    nrv2d_decompress_asm_fast
%else
  %error
%endif

GLOBAL decompress

%define         INP     dword [esp+24+4]
%define         INS     dword [esp+24+8]
%define         OUTP    dword [esp+24+12]
%define         OUTS    dword [esp+24+16]

decompress:
                push    ebp
                push    edi
                push    esi
                push    ebx
                push    ecx
                push    edx
                cld

                mov     esi, INP
                mov     edi, OUTP

                or      ebp, byte -1
;;;             align   8
%ifdef NRV2B
  %include      "n2b_d32.ash"
%elifdef NRV2D
  %include      "n2d_d32.ash"
%else
  %error
%endif


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

                pop     edx
                pop     ecx
                pop     ebx
                pop     esi
                pop     edi
                pop     ebp
                ret


; vi:ts=8:et:nowrap
