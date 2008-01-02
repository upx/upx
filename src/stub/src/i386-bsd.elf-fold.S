/*
;  i386-bsd.elf-fold.asm -- linkage to C code to process Elf binary
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
#define szElf32_Ehdr 0x34
#define szElf32_Phdr 8*4
#define e_type    16
#define e_entry  (16 + 2*2 + 4)
#define p_memsz  5*4
#define szb_info 12
#define szl_info 12
#define szp_info 12
#define a_type 0
#define a_val  4
#define sz_auxv 8

#define __NR_munmap   73

// control just falls through, after this part and compiled C code
// are uncompressed.

fold_begin:  // enter: %ebx= &Elf32_Ehdr of this program
        // patchLoader will modify to be
        //   dword sz_uncompressed, sz_compressed
        //   byte  compressed_data...

// ld-linux.so.2 depends on AT_PHDR and AT_ENTRY, for instance.
// Move argc,argv,envp down to make room for Elf_auxv table.
// Linux kernel 2.4.2 and earlier give only AT_HWCAP and AT_PLATFORM
// because we have no PT_INTERP.  Linux kernel 2.4.5 (and later?)
// give not quite everything.  It is simpler and smaller code for us
// to generate a "complete" table where Elf_auxv[k -1].a_type = k.
// On second thought, that wastes a lot of stack space (the entire kernel
// auxv, plus those slots that remain empty anyway).  So try for minimal
// space on stack, without too much code, by doing it serially.

#define AT_NULL   0
#define AT_IGNORE 1
#define AT_PHDR   3
#define AT_PHENT  4
#define AT_PHNUM  5
#define AT_PAGESZ 6
#define AT_BASE   7
#define AT_ENTRY  9

#define ET_DYN    3

        sub ecx, ecx
        mov edx, (1<<AT_PHDR) | (1<<AT_PHENT) | (1<<AT_PHNUM) | (1<<AT_PAGESZ) | (1<<AT_BASE) | (1<<AT_ENTRY)
        mov esi, esp
        mov edi, esp
        call do_auxv  // clear bits in edx according to existing auxv slots

        mov esi, esp
L50:
        shr edx, 1  // Carry = bottom bit
        sbb eax, eax  // -1 or 0
        sub ecx, eax  // count of 1 bits that remained in edx
        lea esp, [esp + sz_auxv * eax]  // allocate one auxv slot, if needed
        test edx,edx
        jne L50

        mov edi, esp
        call do_auxv  // move; fill new auxv slots with AT_IGNORE

#define OVERHEAD 2048
#define MAX_ELF_HDR 512

        sub esp, MAX_ELF_HDR + OVERHEAD  // alloca
        push ebx  // start of unmap region (&Elf32_Ehdr of this stub)

// Cannot pre-round .p_memsz because kernel requires PF_W to setup .bss,
// but strict SELinux (or PaX, grsecurity) prohibits PF_W with PF_X.
        mov edx, [p_memsz + szElf32_Ehdr + ebx]  // phdr[0].p_memsz
        lea edx, [-1 + 2*PAGE_SIZE + edx + ebx]  // 1 page for round, 1 for unfold
        and edx, 0-PAGE_SIZE

        push edx  // end of unmap region
        sub eax, eax  // 0
        cmp word ptr [e_type + ebx], ET_DYN
        jne L53
        xchg eax, edx  // dynbase for ET_DYN; assumes mmap(0, ...) is placed after us!
L53:
        push eax  // dynbase

        mov esi, [e_entry + ebx]  // end of compressed data
        lea eax, [szElf32_Ehdr + 2*szElf32_Phdr + szl_info + szp_info + ebx]  // 1st &b_info
        sub esi, eax  // length of compressed data
        mov ebx, [   eax]  // length of uncompressed ELF headers
        mov ecx, [4+ eax]  // length of   compressed ELF headers
        add ecx, szb_info
        lea edx, [3*4 + esp]  // &tmp
        pusha  // (AT_table, sz_cpr, f_expand, &tmp_ehdr, {sz_unc, &tmp}, {sz_cpr, &b1st_info} )
        inc edi  // swap with above 'pusha' to inhibit auxv_up for PT_INTERP
.extern upx_main
        call upx_main  // returns entry address
        add esp, (8 +1)*4  // remove 8 params from pusha, also dynbase
        pop ecx  // end of unmap region
        pop ebx  // start of unmap region (&Elf32_Ehdr of this stub)
        add esp, MAX_ELF_HDR + OVERHEAD  // un-alloca

        push eax  // save entry address as ret.addr
        push 0  // 'leave' uses this to clear ebp
        mov ebp,esp  // frame

        sub ecx, ebx
        sub eax,eax  // 0, also AT_NULL
        push ecx  // length to unmap
        push ebx  // start of unmap region (&Elf32_Ehdr of this stub)
        push eax  // fake ret.addr

        dec edi  // auxv table
        .byte 0x3c  // "cmpb al, ..." like "jmp 1+L60" but 1 byte shorter
L60:
        scasd  // a_un.a_val etc.
        scasd  // a_type
        jne L60  // not AT_NULL
// edi now points at [AT_NULL]a_un.a_ptr which contains result of make_hatch()
        push dword ptr [edi]  // &escape hatch

        xor edi,edi
        xor esi,esi
        xor edx,edx
        xor ecx,ecx
        xor ebx,ebx
        mov al, __NR_munmap  // eax was 0 from L60
        ret  // goto escape hatch: int 0x80; leave; ret

// called twice:
//  1st with esi==edi, ecx=0, edx= bitmap of slots needed: just update edx.
//  2nd with esi!=edi, ecx= slot_count: move, then append AT_IGNORE slots
// entry: esi= src = &argc; edi= dst; ecx= # slots wanted; edx= bits wanted
// exit:  edi= &auxtab; edx= bits still needed
do_auxv:
        // cld

L10:  // move argc+argv
        lodsd
        stosd
        test eax,eax
        jne L10

L20:  // move envp
        lodsd
        stosd
        test eax,eax
        jne L20

        push edi  // return value
L30:  // process auxv
        lodsd  // a_type
        stosd
        cmp eax, 32
        jae L32  // prevent aliasing by 'btr' when 32<=a_type
        btr edx, eax  // no longer need a slot of type eax  [Carry only]
L32:
        test eax, eax  // AT_NULL ?
        lodsd
        stosd
        jnz L30  // a_type != AT_NULL

        sub edi, 8  // backup to AT_NULL
        add ecx, ecx  // two words per auxv
        inc eax  // convert 0 to AT_IGNORE
        rep stosd  // allocate and fill
        dec eax  // convert AT_IGNORE to AT_NULL
        stosd  // re-terminate with AT_NULL
        stosd

        pop edi  // &auxtab
        ret

// vi:ts=8:et:nowrap

