;  l_lx_sh86.asm -- Linux program entry point & decompressor (shell script)
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2000 Laszlo Molnar
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
%define         jmpl    jmp dword

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
        pop esi  ; &{ sz_uncompressed, sz_compressed, compressed_data...}
        cld
        lodsd
        push eax  ; sz_uncompressed  (junk, actually)
        push esp  ; &sz_uncompressed
        mov eax, ebp  ; &decompress
        and eax, dword PAGE_MASK  ; &my_elfhdr
        mov edx, eax  ; need my_elfhdr later
        add eax, [p_memsz + szElf32_Ehdr + eax]
        push eax  ; &destination

                ; mmap a page to hold the decompressed program
        xor ecx,ecx
        push ecx
        push ecx
        mov ch, PAGE_SIZE >> 8
        push byte MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS
        push byte PROT_READ | PROT_WRITE | PROT_EXEC
        push ecx
        push eax  ; destination
        push byte __NR_mmap
        pop eax
        mov ebx, esp
        int 0x80
        add esp, byte 6*4  ; discard args to mmap

        lodsd
        push eax  ; sz_compressed
        push esi  ; &compressed_data
        call ebp  ; decompress(&src, srclen, &dst, &dstlen)
        pop ecx  ; discard &compressed_data
        pop ecx  ; discard sz_compressed
        pop ecx  ; &destination
        jmp ecx  ; goto fold_begin at p_vaddr + p_memsz
main:
        pop ebp  ; &decompress
        call unfold
fold_begin:
        ; patchLoader will modify to be
        ;   dword sz_uncompressed, sz_compressed
        ;   byte  compressed_data...

        pop eax  ; discard &sz_uncompressed
        pop eax  ; discard  sz_uncompressed

; Move argc,argv,envp down so that we can insert more Elf_auxv entries.
; ld-linux.so.2 depends on AT_PHDR and AT_ENTRY, for instance

%define OVERHEAD 2048
%define MAX_ELF_HDR 512

        mov esi, esp
        sub esp, byte 6*8  ; AT_PHENT, AT_PHNUM, AT_PAGESZ, AT_ENTRY, AT_PHDR, AT_NULL
        mov edi, esp
        call do_auxv

        sub esp, dword MAX_ELF_HDR + OVERHEAD
        push esp  ; argument: temp space
        push edi  ; argument: AT_next
        push ebp  ; argument: &decompress
        push edx  ; argument: my_elfhdr
        add ecx, PAGE_SIZE  ; uncompressed stub fits in this
        push ecx  ; argument: uncbuf
EXTERN upx_main
        call upx_main  ; entry = upx_main(uncbuf, my_elfhdr, &decompress, AT_next, tmp_ehdr)
        pop esi  ; decompression buffer
        pop ebx  ; my_elfhdr
        add esp, dword 3*4 + MAX_ELF_HDR + OVERHEAD  ; remove 3 params, temp space

        pop ecx  ; argc
        pop edx  ; $0 filename, to become argv[0]
        push edx  ; restore $0 filename

        add esi, byte 3
        inc ecx
        push esi  ; &uncompressed shell script
        sub esi, byte 3

        mov [esi], word 0x632d  ; "-c"
        inc ecx
        push esi  ; "-c"

        inc ecx
        push edx  ; argv[0] is duplicate of $0

        push ecx  ; new argc
        push eax  ; save entry address

; _dl_start and company (ld-linux.so.2) assumes that it has virgin stack,
; and does not initialize all its stack local variables to zero.
; Ulrich Drepper (drepper@cyngus.com) has refused to fix the bugs.
; See GNU wwwgnats libc/1165 .

%define  N_STKCLR (0x100 + MAX_ELF_HDR + OVERHEAD)/4
        lea edi, [esp - 4*N_STKCLR]
        pusha  ; values will be zeroed
        mov ecx, N_STKCLR
        xor eax,eax
        rep stosd

; Because the decompressed shell script occupies low memory anyway,
; there isn't much payback to unmapping the compressed script and
; ourselves the stub.  We would need a place to put the escape hatch
; "int $0x80; popa; ret", and some kernels do not allow execution
; on the stack.  So, we would have to dirty a page of the shell
; or of /lib/ld-linux.so.  It's simpler just to omit the unapping.
        popa
        ret

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

L30:  ; move existing Elf32_auxv
        lodsd
        stosd
        test eax,eax  ; AT_NULL ?
        lodsd
        stosd
        jne L30

        sub edi, byte 8  ; point to AT_NULL
        ret


; vi:ts=8:et:nowrap

