;  l_lx_exec86.asm -- Linux program entry point & decompressor (kernel exec)
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2000 Laszlo Molnar
;  All Rights Reserved.
;
;  Integration of virtual exec() with decompression is
;  Copyright (C) 2000 John F. Reiser.  All rights reserved.
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
;  John F. Reiser
;  jreiser@BitWagon.com


                BITS    32
                SECTION .text

%define         jmps    jmp short
%define         jmpn    jmp near

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



%include "ident.ash"

; /*************************************************************************
; // program entry point
; // see glibc/sysdeps/i386/elf/start.S
; **************************************************************************/

GLOBAL _start

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
%if 0
                ; personality(PER_LINUX)
                mov     eax, 136            ; syscall_personality
                xor     ebx, ebx            ; PER_LINUX
                int     0x80
%endif
        call main  ; push address of decompress subroutine

; /*************************************************************************
; // C callable decompressor
; **************************************************************************/

%define         INP     dword [esp+8*4+4]
%define         INS     dword [esp+8*4+8]
%define         OUTP    dword [esp+8*4+12]
%define         OUTS    dword [esp+8*4+16]

decompress:
                pusha
                ; cld

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

                mov [7*4 + esp], eax
                popa
                ret


%define PAGE_MASK (~0<<12)
%define PAGE_SIZE ( 1<<12)

%define szElf32_Ehdr 0x34
%define szElf32_Phdr 8*4
%define p_filesz 4*4
%define p_memsz  5*4
%define a_val  4

%define MAP_FIXED     0x10
%define MAP_PRIVATE   0x02
%define MAP_ANONYMOUS 0x20
%define PROT_READ      1
%define PROT_WRITE     2
%define PROT_EXEC      4
%define __NR_mmap     90
%define __NR_munmap   91

; Decompress the rest of this loader, and jump to it
unfold:
        pop esi  ; &fold_begin = src
          push esi  ; &dst
        mov ecx, ebp  ; &decompress
        and ecx, dword PAGE_MASK  ; &my_elfhdr
        mov ebx, ecx  ; save &my_elfhdr for later

;; Compressed code now begins at fold_begin.
;; We want decompressed code to begin at fold_begin, too.
;; Move the compressed code to the high end of the page.
;; Assume non-overlapping so that forward movsb is OK.
        lea edi, [-PAGE_MASK + ecx]  ; high end of page
        add ecx, [p_filesz + szElf32_Ehdr + ecx]  ; beyond src
        sub ecx, esi  ; srclen
          push ecx  ; srclen
        sub edi, ecx
          push edi  ; &src
        cld
        rep movsb

          call ebp  ; decompress(&src, srclen, &dst, &dstlen)
        pop eax  ; discard &src
        pop eax  ; discard srclen
        pop eax  ; &dst == fold_begin

;; icache lookahead of compressed code after "call unfold" is still there.
;; Synchronize with dcache of decompressed code.
        pushf
        push cs
        push eax
        iret  ; back to fold_begin!

main:
        pop ebp  ; &decompress
          push eax  ; place to store dstlen
          push esp  ; &dstlen
        call unfold
fold_begin:  ;; this label is known to the Makefile
        pop eax  ; discard &dstlen
        pop eax  ; discard  dstlen

                pop     eax                 ; Pop the argument count
                mov     ecx, esp            ; argv starts just at the current stack top
                lea     edx, [ecx+eax*4+4]  ; envp = &argv[argc + 1]
                push    eax                 ; Restore the stack
                push    ebp  ; argument: &decompress
                push    ebx  ; argument: &my_elfhdr
                push    edx  ; argument: envp
                push    ecx  ; argument: argv
EXTERN upx_main
                call    upx_main            ; Call the UPX main function
                hlt                         ; Crash if somehow upx_main does return

; vi:ts=8:et:nowrap
