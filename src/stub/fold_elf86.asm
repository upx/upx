;  fold_elf86.asm -- linkage to C code to process Elf binary
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 2000-2001 John F. Reiser
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
;  Markus F.X.J. Oberhumer   Laszlo Molnar           John F. Reiser
;  markus@oberhumer.com      ml1050@cdata.tvnet.hu   jreiser@BitWagon.com
;

%define szElf32_Ehdr 0x34
%define szElf32_Phdr 8*4
%define e_entry  (16 + 2*2 + 4)
%define p_memsz  5*4
%define szl_info 12
%define szp_info 12
%define a_val  4

%define __NR_munmap   91

;; control just falls through, after this part and compiled C code
;; are uncompressed.

fold_begin:  ; enter: %ebx= &Elf32_Ehdr of this program
        ; patchLoader will modify to be
        ;   dword sz_uncompressed, sz_compressed
        ;   byte  compressed_data...

        pop eax  ; discard &sz_uncompressed
        pop eax  ; discard  sz_uncompressed

; Move argc,argv,envp down so that we can insert more Elf_auxv entries.
; ld-linux.so.2 depends on AT_PHDR and AT_ENTRY, for instance

%define PAGE_SIZE ( 1<<12)
%define OVERHEAD 2048
%define MAX_ELF_HDR 512

        mov esi, esp
        sub esp, byte 6*8  ; AT_PHENT, AT_PHNUM, AT_PAGESZ, AT_ENTRY, AT_PHDR, AT_NULL
        mov edi, esp
        call do_auxv

          mov eax, [p_memsz + 2*szElf32_Phdr + szElf32_Ehdr + ebx]  ; size of PT_DYNAMIC
        sub esp, dword MAX_ELF_HDR + OVERHEAD
          mov ecx, [e_entry + ebx]  ; beyond compressed data
        push esp  ; argument: temp space
          lea eax, [szElf32_Ehdr + 3*szElf32_Phdr + szl_info + szp_info + ebx + eax]  ; 1st &b_info
        push edi  ; argument: AT_next
          sub ecx, eax  ; length of compressed data
        push ebp  ; argument: &decompress
          push ecx  ; argument: sz_compressed
        push eax  ; argument: 1st &b_info
EXTERN upx_main
        call upx_main  ; entry = upx_main(b1st_info, sz_cpr, &decompress, AT_next, tmp_ehdr)
        add esp, dword 5*4 + MAX_ELF_HDR + OVERHEAD  ; remove 5 params, temp space
        push eax  ; save entry address

        mov edi, [a_val + edi]  ; AT_PHDR
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
        mov ecx, N_STKCLR
        xor eax,eax
        rep stosd

        xor ecx, ecx  ; 0
        mov ch, PAGE_SIZE>>8  ; 0x1000
        add ecx, [p_memsz + szElf32_Ehdr + ebx]  ; length to unmap
        mov bh, 0  ; from 0x401000 to 0x400000
        push byte __NR_munmap
        pop eax
        jmp edx  ; unmap ourselves via escape hatch, then goto entry

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

