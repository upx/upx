;  fold_sh86.asm -- Linux program entry point & decompressor (shell script)
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2001 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2001 Laszlo Molnar
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


                BITS    32
                SECTION .text

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
%define PAGE_SIZE ( 1<<12)

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

