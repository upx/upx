;  l_lx_elf86.asm -- Linux program entry point & decompressor (Elf binary)
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

;;;; names of pseudo-sections for addLoader:
;; LXUNFnnn  Linux unfilter
;; LXNJMPnn  omit filtering of 6-byte Jxx (0x0f 0x80..0x8f)
;; LXMRUnnn  MostRecentlyUsed recoding of destinations
;; MRUARBnn  arbitrary number of entries in wheel
;; MRUBITSn  power of 2          entries in wheel (smaller code)
;; MRUBYTEn  256                 entries in wheel (smallest code)

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
decompress:
;__LXUNF000__
        jmps decompr0
  ;; 2+ address of decompress subroutine
  ;; unfilter(upx_byte *, length)
        pop edx  ; return address
        pop eax  ; upx_byte *, same as addvalue
        pop ecx  ; length
        pusha  ; save C-convention ebx, ebp, esi, edi; also eax, edx
        xchg eax, edi  ; edi= pointer

        push dword ('?'<<8) | 0x0f  ; cto8_0f  (cto8 byte is modified)
%ifdef   __MRUBYTE0__
        xor ebx, ebx  ; zero
%else   ;__MRUARB00__  (also __MRUBITS0__)
        mov ebx, 'NMRU'  ; modified N_MRU or N_MRU -1
%endif  ;__LXMRU010__

        xor edx, edx  ; zero
;__LXUNF010__
        jmpn unf0
;__LXELF010__

; /*************************************************************************
; // C callable decompressor
; **************************************************************************/

%define         INP     dword [esp+8*4+4]
%define         INS     dword [esp+8*4+8]
%define         OUTP    dword [esp+8*4+12]
%define         OUTS    dword [esp+8*4+16]

decompr0:
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

;__LXUNF020__
;; continuation of entry prolog for unfilter
unf0:
;__LXMRU020__
        push edx  ; tail
        push ebx  ; n_mru or n_mru1
;__LXUNF025__
        mov esi, esp

%define n_mru    [esi]
%define n_mru1   [esi]
%define tail     [esi + 4*1]
%define cto8_0f  [esi + 4*2]
%define cto8     [esi + 4*2 +1]
%define addvalue [esi + 4*3 + 7*4]

%ifdef  __MRUBITS1__
        inc ebx  ; n_mru1 ==> n_mru
%endif ;__LXMRU030__
unf1:  ; allocate and clear mru[]
        push edx  ; zero
%ifdef __MRUBYTE1__
        dec bl
%else ;__MRUARB10__
        dec ebx
%endif ;__LXMRU040__
        jnz unf1  ; leaves 0=='hand'

%define tmp ebp

%define jc     eax
%define hand   ebx
%define hand_l  al
%define kh     edx
%define kh_l    dl

;__LXUNF030__
calltrickloop:
        mov al, [edi]
        inc edi
%ifndef __LXNJMP00__
        sub al, 0x80        ; base of Jcc <d32>
        cmp al, 0x8f - 0x80 ; span of Jcc <d32>
        ja ct2             ; not Jcc <d32>
        mov edx, [edi]  ; often unaligned
        cmp dx, cto8_0f
        jne unfcount
        mov byte [edi -1], dl  ; 0x0f prefix
        add al, 0x80  ; reconstitute Jcc
        dec ecx
        mov byte [edi], al  ; Jcc opcode
        inc edi
        jmps lxunf
ct2:
        sub al, 0xE8 - 0x80  ; base of JMP/CALL <d32>
        cmp al, 0xE9 - 0xE8  ; span of JMP/CALL <d32>
%else  ;__LXNJMP10__
        sub al, 0xE8         ; base of JMP/CALL <d32>
        cmp al, 0xE9 - 0xE8  ; span of JMP/CALL <d32>
%endif ;__LXMRU050__
        ja unfcount
        mov al, [edi]
        cmp al, cto8
        je lxunf
unfcount:
        dec ecx
        jg calltrickloop

        mov edi,esp ; clear mru[] portion of stack
%ifdef __MRUBYTE2__
        mov ecx, 3+ 256  ; unused, tail, ct8_0f
%elifdef __MRUBITS2__
        mov ecx, n_mru1
        add ecx, byte 1+ 3  ; n_mru1, tail, ct8_0f
%else ;__MRUARB20__
        mov ecx, n_mru
        add ecx, byte 3  ; n_mru, tail, ct8_0f
%endif ;__LXMRU060__
        xor eax,eax
        rep
        stosd
        mov esp,edi
        popa
        push ecx
        push eax
        push edx
        ret

lxunf:
        mov eax, [edi]
        shr ax, 8
        rol eax, 16
        xchg al, ah
;__LXMRU065__
        shr jc, 1  ; eax= jc, or mru index
        jnc mru4  ; not 1st time for this jc
%ifdef __MRUBYTE3__
        dec hand_l
%else ;__MRUARB30__
        dec hand
%ifdef   __MRUBITS3__
        and hand, n_mru1
%else ;__MRUARB40__
        jge mru3
        add hand, n_mru
mru3:
%endif
%endif ;__LXMRU070__

        mov [esp + 4*hand], jc  ; 1st time: mru[hand] = jc
        jmps unf_store

mru4:  ; not 1st time for this jc
        lea kh, [jc + hand]  ; kh = jc + hand
%ifdef __MRUBYTE4__
        movzbl kh, kh_l
%elifdef __MRUBITS4__
        and kh, n_mru1
%else ;__MRUARB50__
        cmp kh, n_mru
        jb mru5
        sub kh, n_mru
mru5:
%endif ;__LXMRU080__
        mov jc, [esp + 4*kh]  ; jc = mru[kh]
%ifdef __MRUBYTE5__
        dec hand_l
%else ;__MRUARB60__
        dec hand
%ifdef   __MRUBITS5__
        and hand, n_mru1
%else ;__MRUARB70__
        jge mru6
        add hand, n_mru
mru6:
%endif
%endif ;__LXMRU090__

        mov tmp, [esp + 4*hand]  ; tmp = mru[hand]
        test tmp,tmp
        jnz mru8

          push jc  ; ran out of registers
        mov eax, tail

%ifdef  __MRUBYTE6__
        dec al
%else  ;__MRUARB80__
        dec eax
%ifdef  __MRUBITS6__
        and eax, n_mru1
%else  ;__MRUARB90__
        jge mru7
        add eax, n_mru
mru7:
%endif
%endif ;__LXMRU100__

        xor tmp,tmp
        mov tail, eax
        xchg [4+ esp + 4*eax], tmp  ; tmp = mru[tail]; mru[tail] = 0
          pop jc
mru8:
        mov [esp + 4*kh  ], tmp  ; mru[kh] = tmp
        mov [esp + 4*hand], jc   ; mru[hand] = jc
;__LXUNF040__
unf_store:
        sub eax, edi
        sub ecx, byte 4
        add eax, addvalue
        mov [edi], eax
        add edi, byte 4
        jmps unfcount

;__LXELF020__

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
        mov ah,0  ; round down to 64KB boundary
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
        pop eax  ; discard &compressed_data
        pop eax  ; discard sz_compressed
        ret      ; &destination
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
        add edx, [p_memsz + szElf32_Ehdr + edx]
        push edx  ; argument: uncbuf
EXTERN upx_main
        call upx_main  ; entry = upx_main(uncbuf, my_elfhdr, &decompress, AT_next, tmp_ehdr)
        pop esi  ; decompression buffer == (p_vaddr + p_memsz) of stub
        pop ebx  ; my_elfhdr
        add esp, dword 3*4 + MAX_ELF_HDR + OVERHEAD  ; remove 3 params, temp space
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

        mov ecx,esi  ; my p_vaddr + p_memsz
        mov bh,0  ; round down to 64KB boundary
        sub ecx,ebx  ; length to unmap
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

