/*
;  i386-linux.elf.interp-fold.S -- linkage to C code to act as ELF PT_INTERP
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 2000-2008 John F. Reiser
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
;  John F. Reiser
;  <jreiser@users.sourceforge.net>
;
*/

#include "arch/i386/macros.S"


#define PAGE_SIZE ( 1<<12)

#define AT_NULL        0
#define AT_PHDR        3

#define szElf32_Ehdr 0x34
#define szElf32_Phdr 8*4
#define e_entry  (16 + 2*2 + 4)
#define p_vaddr  2*4
#define p_memsz  5*4
#define szb_info 12
#define szl_info 12
#define szp_info 12
#define a_type 0
#define a_val  4
#define sz_auxv 8

#define MAP_FIXED     0x10
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20
#define PROT_READ      1
#define PROT_WRITE     2
#define PROT_EXEC      4
#define __NR_mmap     90
#define __NR_munmap   91

#define OVERHEAD 2048
#define MAX_ELF_HDR 512

        pop ebp  // get_fexp
        pop ecx  // get_funf
        pop eax  // argc
        lea edi, [4+ 4*eax + esp]  // &environ
        push eax  // argc

        sub eax,eax  // 0
L310:
        scasd
        jne L310
        scasd  // edi= &Elf32_auxv_t

        mov esi,edi
L320:
        mov eax,[esi]  // a_type
        cmp eax, AT_PHDR
        je L330
        add esi, sz_auxv
        cmp eax, AT_NULL
        jne L320
L330:
        mov ebx,[a_val + esi]
        push ebx  // save &Elf32_Phdr of compressed data

        sub esp, MAX_ELF_HDR + OVERHEAD  // working storage
        mov edx, esp
        push ecx  // get_funf  9th param to pti_main
        lea eax, [2*szElf32_Phdr + szl_info + szp_info + ebx]  // 1st &b_info
        mov esi, [e_entry + ebx]  // beyond compressed data
        sub esi, eax  // length of compressed data
        mov ebx, [   eax]  // length of uncompressed ELF headers
        mov ecx, [4+ eax]  // length of   compressed ELF headers
        add ecx, szb_info
        pusha  // (AT_table, sz_cpr, get_fexp, &tmp_ehdr, {sz_unc, &tmp}, {sz_cpr, &b1st_info} )
.extern pti_main
        call pti_main  // returns entry address
        add esp, 9*4 + MAX_ELF_HDR + OVERHEAD  // remove 9 params, temp space
        pop ebx  // &Elf32_Phdr
        push eax  // save entry address
        mov ecx,[p_memsz + ebx]
        mov ebx,[p_vaddr + ebx]
        mov eax,__NR_munmap
        int 0x80  // unmap compressed data

        sub eax,eax
        sub ecx,ecx
        sub edx,edx
        sub ebx,ebx
        sub ebp,ebp
        sub esi,esi
        sub edi,edi
        ret  // goto entry point

#define __NR_mmap 90

mmap: .globl mmap
        push ebx
        lea ebx, [2*4 + esp]
        push __NR_mmap
        pop eax
        int 0x80
        pop ebx
        ret

// vi:ts=8:et:nowrap

