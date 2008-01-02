/*
;  i386-linux.elf.shell-entry.S -- Linux program entry point & decompressor (shell script)
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


/*************************************************************************
// program entry point
// see glibc/sysdeps/i386/elf/start.S
**************************************************************************/

section LEXEC000
_start: .globl _start
////    int3
/*
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
*/

        call main  // push address of decompress subroutine
decompress:

/*************************************************************************
// C callable decompressor
**************************************************************************/

#define         INP     dword ptr [esp+8*4+4]
#define         INS     dword ptr [esp+8*4+8]
#define         OUTP    dword ptr [esp+8*4+12]
#define         OUTS    dword ptr [esp+8*4+16]

section LEXEC010
                pusha
                ; cld

                mov     esi, INP
                mov     edi, OUTP

                or      ebp, -1
//              .balign   8

#include      "arch/i386/nrv2b_d32.S"
#include      "arch/i386/nrv2d_d32.S"
#include      "arch/i386/nrv2e_d32.S"
#include      "arch/i386/cl1_d32.S"
#include      "arch/i386/lzma_d.S"
                cjt32 0

section LEXEC015
                // eax is 0 from decompressor code
                //xor     eax, eax               ; return code

// check compressed size
                mov     edx, INP
                add     edx, INS
                cmp     esi, edx
                jz      .ok
                dec     eax
.ok:

// write back the uncompressed size
                sub     edi, OUTP
                mov     edx, OUTS
                mov     [edx], edi

                mov [7*4 + esp], eax
section LEXEC017
                popa
                ret

section LEXEC020

#define PAGE_SIZE ( 1<<12)

#define MAP_FIXED     0x10
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20
#define PROT_READ      1
#define PROT_WRITE     2
#define PROT_EXEC      4
#define __NR_mmap     90

#define szElf32_Ehdr 0x34
#define szElf32_Phdr 8*4
#define e_entry  (16 + 2*2 + 4)
#define p_memsz  5*4
#define szl_info 12
#define szp_info 12
#define p_filesize 4

// Decompress the rest of this loader, and jump to it
main:
        pop ebp  // &decompress
        mov eax,0x1400000  // &Elf32_Ehdr of this stub
        lea edx,[0x80 + szp_info + eax]  // &cprScript
        add eax,[p_memsz + szElf32_Ehdr + eax]  // after .text
        add eax,PAGE_SIZE -1
        and eax, 0-PAGE_SIZE  // round up to next page

        push 0
        push -1
        push MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS
        push PROT_READ | PROT_WRITE
        push dword ptr [edx]  // sz_unc  length
        push eax  // address
        mov ebx,esp
        push __NR_mmap
        pop eax
        int 0x80
        add esp, 6*4  // remove arguments

        lea ebx,[3+ eax]  // space for "-c"
// fall into fold [not compressed!]

eof:

// vi:ts=8:et:nowrap

