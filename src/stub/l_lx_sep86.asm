;  l_lx_sep86.asm -- Linux program entry point & decompressor (separate script)
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
;; Invoke gdb on the separate stub (such as "gdb upxb"), and give the command
;; "run date".  Define a single-step macro such as
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
%define a_type 0
%define a_val  4
%define sz_auxv 8

%define szElf32_Phdr 8*4
%define a_val  4
%define __NR_munmap   91

main:
        pop ebp  ; &decompress
        cld

; Move argc,argv,envp down to make room for complete Elf_auxv table.
; Linux kernel 2.4.2 and earlier give only AT_HWCAP and AT_PLATFORM
; because we have no PT_INTERP.  Linux kernel 2.4.5 (and later?)
; give not quite everything.  It is simpler and smaller code for us
; to generate a "complete" table where Elf_auxv[k -1].a_type = k.
; ld-linux.so.2 depends on AT_PHDR and AT_ENTRY, for instance

%define AT_NULL   0
%define AT_IGNORE 1
%define AT_PHDR   3
%define AT_NUMBER 20

        mov esi, esp
        sub esp, sz_auxv * AT_NUMBER  ; more than 128 bytes
        mov edi, esp
do_auxv:  ; entry: %esi=src = &argc; %edi=dst.  exit: %edi= &AT_NULL
        ; cld

L10:  ; move argc+argv
        lodsd
        stosd
        test eax,eax
        jne L10

L20:  ; move envp
        lodsd
        stosd
        test eax,eax
        jne L20

; complete Elf_auxv table full of AT_IGNORE
        push edi  ; save base of resulting table
        inc eax  ; convert 0 to AT_IGNORE
        mov ecx, 2 * (AT_NUMBER -1)
        rep stosd
        dec eax  ; convert AT_IGNORE into AT_NULL
        stosd  ; terminate Elf_auxv
        stosd
        pop edi  ; base of resulting table

L30:  ; distribute existing Elf32_auxv into new table
        lodsd
        test eax,eax  ; AT_NULL ?
        xchg eax,edx
        lodsd
        je L40
        mov [a_type + sz_auxv*(edx -1) + edi], edx
        mov [a_val  + sz_auxv*(edx -1) + edi], eax
        jmp L30
L40:

%define OVERHEAD 2048
%define MAX_ELF_HDR 512

        lea ecx, [4+esp]  ; argv
        sub esp, dword MAX_ELF_HDR + OVERHEAD

        push esp  ; argument: temp space
        push edi  ; argument: AT_table
        push ebp  ; argument: &decompress
        push ecx  ; argument: argv
EXTERN upx_main
        call upx_main  ; entry = upx_main(argv, &decompress, AT_table, tmp_ehdr)
        add esp, dword 4*4 + MAX_ELF_HDR + OVERHEAD  ; remove temp space, args

        pop ecx  ; argc
        pop edx  ; ++argv  discard argv[0] == pathname of stub
        dec ecx  ; --argc
        push ecx
        push eax  ; save entry address

        mov edi, [a_val + sz_auxv * (AT_PHDR -1) + edi]
find_hatch:
        push edi
EXTERN make_hatch
        call make_hatch  ; find hatch = make_hatch(phdr)
        pop ecx  ; junk the parameter
        add edi, byte szElf32_Phdr  ; prepare to try next Elf32_Phdr
        test eax,eax
        jz find_hatch
        xchg eax,edx  ; edx= &hatch

; _dl_start and company (ld-linux.so.2) assumes that it has virgin stack,
; and does not initialize all its stack local variables to zero.
; Ulrich Drepper (drepper@cyngus.com) has refused to fix the bugs.
; See GNU wwwgnats libc/1165 .

%define  N_STKCLR (0x100 + MAX_ELF_HDR + OVERHEAD)/4
        lea edi, [esp - 4*N_STKCLR]
        pusha  ; values will be zeroed
        mov ebx,esp  ; save
        mov esp,edi  ; Linux does not grow stack below esp
        mov ecx, N_STKCLR
        xor eax,eax
        rep stosd
        mov esp,ebx  ; restore

        mov ecx, dword -PAGE_SIZE
        mov ebx, ebp
        and ebx, ecx  ; round down to page boundary
        neg ecx  ; PAGE_SIZE  (this stub fits in it)
        mov eax, __NR_munmap  ; do not dirty the stack with push byte + pop
        jmp edx  ; unmap ourselves, then goto entry


; vi:ts=8:et:nowrap

