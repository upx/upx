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

%define PAGE_SIZE ( 1<<12)
%define szElf32_Ehdr 0x34
%define szElf32_Phdr 8*4
%define e_entry  (16 + 2*2 + 4)
%define p_memsz  5*4
%define szb_info 12
%define szl_info 12
%define szp_info 12
%define a_type 0
%define a_val  4
%define sz_auxv 8

%define __NR_munmap   91

;; control just falls through, after this part and compiled C code
;; are uncompressed.

fold_begin:  ; enter: %ebx= &Elf32_Ehdr of this program
        ; patchLoader will modify to be
        ;   dword sz_uncompressed, sz_compressed
        ;   byte  compressed_data...

        pop eax  ; discard &sz_uncompressed
        pop eax  ; discard  sz_uncompressed

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

        push ebx  ; save &Elf32_Ehdr of this stub
        sub esp, dword MAX_ELF_HDR + OVERHEAD
        lea eax, [szElf32_Ehdr + 2*szElf32_Phdr + szl_info + szp_info + ebx]  ; 1st &b_info
        mov esi, [e_entry + ebx]  ; beyond compressed data
        sub esi, eax  ; length of compressed data
        mov ebx, [   eax]  ; length of uncompressed ELF headers
        mov edx, esp  ;
        mov ecx, [4+ eax]  ; length of   compressed ELF headers
        add ecx, byte szb_info
        pusha  ; (AT_table, sz_cpr, f_expand, &tmp_ehdr, {sz_unc, &tmp}, {sz_cpr, &b1st_info} )
EXTERN upx_main
        call upx_main  ; returns entry address
        add esp, dword 8*4 + MAX_ELF_HDR + OVERHEAD  ; remove 8 params, temp space
        pop ebx  ; &Elf32_Ehdr of this stub
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
        mov ecx, N_STKCLR
        xor eax,eax
        rep stosd

        xor ecx, ecx  ; 0
        mov ch, PAGE_SIZE>>8  ; 0x1000
        add ecx, [p_memsz + szElf32_Ehdr + ebx]  ; length to unmap
        mov bh, 0  ; from 0x401000 to 0x400000
        mov eax, __NR_munmap  ; do not dirty the stack with push byte + pop
        jmp edx  ; unmap ourselves via escape hatch, then goto entry

; vi:ts=8:et:nowrap

