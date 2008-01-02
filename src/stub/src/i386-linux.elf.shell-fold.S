/*
;  i386-linux.elf.shell-fold.S -- Linux program entry point & decompressor (shell script)
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2008 Laszlo Molnar
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
#define e_entry  (16 + 2*2 + 4)
#define szl_info 12
#define szp_info 12
#define a_type 0
#define a_val  4
#define sz_auxv 8

fold_begin:     // In: %ebx= uncDst; edx= &b_info cprSrc; ebp = &decompress

// Move argc,argv,envp down to make room for complete Elf_auxv table.
// Linux kernel 2.4.2 and earlier give only AT_HWCAP and AT_PLATFORM
// because we have no PT_INTERP.  Linux kernel 2.4.5 (and later?)
// give not quite everything.  It is simpler and smaller code for us
// to generate a "complete" table where Elf_auxv[k -1].a_type = k.
// ld-linux.so.2 depends on AT_PHDR and AT_ENTRY, for instance

#define AT_NULL   0
#define AT_IGNORE 1
#define AT_PHDR   3
#define AT_NUMBER (5+ 37)
// 2002-11-09  glibc-2.2.90  AT_IGNOREPPC==22  plus 5 for future growth
// 2006-05-15  glibc-2.4-4   AT_L3_CACHESHAPE==37

        mov esi, esp
        sub esp, sz_auxv * AT_NUMBER  // more than 128 bytes
        mov edi, esp
do_auxv:  // entry: %esi=src = &argc; %edi=dst.  exit: %edi= &AT_NULL
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

// complete Elf_auxv table full of AT_IGNORE
        push edi  // save base of resulting table
        inc eax  // convert 0 to AT_IGNORE
        push 2 * (AT_NUMBER -1)  // less than 128
        pop ecx
        rep stosd
        dec eax  // convert AT_IGNORE into AT_NULL
        stosd  // terminate Elf_auxv
        stosd
        pop edi  // base of resulting table

L30:  // distribute existing Elf32_auxv into new table
        lodsd
        test eax,eax  // AT_NULL ?
        xchg eax,ecx  // edx is busy, do not use
        lodsd
        je L40
        cmp ecx, AT_NUMBER
        jae L30
        mov [a_type - sz_auxv + sz_auxv*ecx + edi], ecx
        mov [a_val  - sz_auxv + sz_auxv*ecx + edi], eax
        jmp L30
L40:

#define OVERHEAD 2048
#define MAX_ELF_HDR 512

        sub esp, MAX_ELF_HDR + OVERHEAD

        xchg eax, ebx  // eax= uncDst
        mov ecx, [   edx]  // sz_unc
        mov ebx, [4+ edx]  // sz_cpr
        mov esi, eax  // extra copy of uncDst
        pusha  // (AT_table,uncDst,f_decpr,&ehdr,{sz_cpr,cprSrc},{sz_unc,uncDst})
.extern upx_main
        call upx_main  // entry = upx_main(...)
        pop ecx  // junk
        push eax  // save entry address
        popa  // edi= entry address; esi= uncDst
        add esp, MAX_ELF_HDR + OVERHEAD  // remove temp space

        pop ecx  // argc
        pop edx  // $0 filename, to become argv[0]
        push edx  // restore $0 filename

        inc ecx
        push esi  // &uncompressed shell script
        sub esi, 3

        mov word ptr [esi], 0x632d  // "-c"
        inc ecx
        push esi  // &"-c"

        inc ecx
        push edx  // argv[0] is duplicate of $0

        push ecx  // new argc
        push edi  // save entry address

// _dl_start and company (ld-linux.so.2) assumes that it has virgin stack,
// and does not initialize all its stack local variables to zero.
// Ulrich Drepper (drepper@cyngus.com) has refused to fix the bugs.
// See GNU wwwgnats libc/1165 .

#define  N_STKCLR (0x100 + MAX_ELF_HDR + OVERHEAD)/4
        lea edi, [esp - 4*N_STKCLR]
        pusha  // values will be zeroed
        mov ebx,esp  // save
        mov esp,edi  // Linux does not grow stack below esp
        mov ecx, N_STKCLR
        xor eax,eax
        rep stosd
        mov esp,ebx  // restore

// Because the decompressed shell script occupies low memory anyway,
// there isn't much payback to unmapping the compressed script and
// ourselves the stub.  We would need a place to put the escape hatch
// "int $0x80; popa; ret", and some kernels do not allow execution
// on the stack.  So, we would have to dirty a page of the shell
// or of /lib/ld-linux.so.  It's simpler just to omit the unmapping.
        popa
        ret

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

