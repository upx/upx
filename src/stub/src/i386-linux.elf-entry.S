/*
;  i386-linux.elf-entry.S -- Linux program entry point & decompressor (Elf binary)
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2013 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2013 Laszlo Molnar
;  Copyright (C) 2000-2013 John F. Reiser
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
////    nop; int3
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

// /*************************************************************************
// // C callable decompressor
// **************************************************************************/

// /* Offsets to parameters, allowing for {pusha + call} */
#define         O_INP   (8*4 +1*4)
#define         O_INS   (8*4 +2*4)
#define         O_OUTP  (8*4 +3*4)
#define         O_OUTS  (8*4 +4*4)
#define         O_PARAM (8*4 +5*4)

#define         INP     dword ptr [esp+O_INP]
#define         INS     dword ptr [esp+O_INS]
#define         OUTP    dword ptr [esp+O_OUTP]
#define         OUTS    dword ptr [esp+O_OUTS]
#define         PARM    dword ptr [esp+O_PARAM]

section LEXEC009
        //;  empty section for commonality with l_lx_exec86.asm
section LEXEC010
                pusha
                // cld

                mov     esi, INP
                mov     edi, OUTP

                or      ebp, -1
//;;             align   8

#include "arch/i386/nrv2b_d32.S"
#include "arch/i386/nrv2d_d32.S"
#include "arch/i386/nrv2e_d32.S"
#include "arch/i386/lzma_d.S"
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
                popa
                ret

                ctojr32
                ctok32  edi, dl
section LEXEC017
                popa
                ret

section LEXEC020

#define PAGE_SHIFT 12
#define PAGE_SIZE ( 1 << PAGE_SHIFT)

#define MAP_FIXED     0x10
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20
#define PROT_READ      1
#define PROT_WRITE     2
#define PROT_EXEC      4
#define __NR_mmap     90
#define szElf32_Ehdr 0x34
#define p_memsz  5*4

#define __NR_write 4
#define __NR_exit  1

#define pushsbli .byte 0x6a,  /* push sign-extended byte to long immediate*/

msg_SELinux:
        pushsbli L71 - L70
        pop edx  // length
        call L71
L70:
        .ascii "PROT_EXEC|PROT_WRITE failed.\n"
L71:
        pop ecx  // message text
        push 2  // fd stderr
        pop ebx
        push __NR_write
        pop eax
        int 0x80
die:
        mov bl, 127  // only low 7 bits matter!
        push __NR_exit
        pop eax  // write to stderr could fail, leaving eax as -EBADF etc.
        int 0x80

// Decompress the rest of this loader, and jump to it
unfold:
        pop esi  // &{ b_info:{sz_unc, sz_cpr, 4{byte}}, compressed_data...}

        lea eax, [ebp - (4+ decompress - _start)]  // 4: sizeof(int)
        mov edi, [eax]  // total length of compressed data
        sub eax, edi    // %eax= &Elf32_Ehdr of this program
        mov edx, eax    // %edx= &Elf32_Ehdr of this program

// Linux requires PF_W in order to create .bss (implied by .p_filesz!=.p_memsz),
// but strict SELinux (or PaX, grSecurity) forbids PF_W with PF_X.
// So first PT_LOAD must be PF_R|PF_X only, and .p_memsz==.p_filesz.
// So we must round up here, instead of pre-rounding .p_memsz.
        add eax, [p_memsz + szElf32_Ehdr + eax]  // address after .text
        add eax,   PAGE_SIZE -1
        and eax, 0-PAGE_SIZE

section LEXECDYN
// VDSO might be at [eax]; if so, then erase AT_SYSINFO and AT_SYSINFO_EHDR.
// This happens with some kernels (Linux-2.6.34) for ET_DYN when stack is
// unlimited (ulimit -s unlimited), because the only PT_LOAD of compressed
// ET_DYN is placed at TASK_UNMAPPED_BASE (TASK_SIZE/3, typically 0x40000000)
// and VDSO follows.  When stack is limited, then allocation is from high
// (just below stack) to low: VDSO will be below our PT_LOAD, and mmap()
// below will steal the lowest page of stack.
        pop ecx; push ecx  // argc
        push esi
        push eax
        lea esi,[esp + 4*ecx + (3+1)*4]  // &env
1:
        lodsd; test eax,eax; jne 1b  // skip env
2:
        lodsd; test eax,eax; je 5f  // done with auxv
AT_IGNORE=  1
AT_SYSINFO= 32
AT_SYSINFO_EHDR=33
        subb al,AT_SYSINFO
        cmpb al,AT_SYSINFO_EHDR - AT_SYSINFO
        lodsd
        ja 2b  // not AT_SYSINFO*
        xor eax,[esp]
        shr eax,PAGE_SHIFT
        jne 2b  // no conflict with decompressed fold_elf86
        movb [esi - 2*4],AT_IGNORE
        jmp 2b
5:
        pop eax
        pop esi

section LUNMP000
        push 0  // 0-page bread crumb
section LUNMP001
        push 1  // 1-page bread crumb
section LEXEC025
        push eax  // destination for 'ret'

                // mmap a page to hold the decompressed fold_elf86
        xor ecx, ecx  // %ecx= 0
        // MAP_ANONYMOUS ==>offset is ignored, so do not push!
        // push ecx  ; offset
        push -1  // *BSD demands -1==fd for mmap(,,,MAP_ANON,,)
        push MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS
        mov ch, PAGE_SIZE >> 8  // %ecx= PAGE_SIZE
        push PROT_READ | PROT_WRITE | PROT_EXEC
        push ecx  // length
        push eax  // destination
        mov ebx, esp  // address of parameter vector for __NR_mmap
        push __NR_mmap
        pop eax
        int 0x80  // changes only %eax; %edx is live
        cmp eax,[ebx]
        jne msg_SELinux  // not the expected address (includes error)
        xchg eax, edx  // %edx= page after .text; %eax= &Elf32_Ehdr of this program
        xchg eax, ebx  // %ebx= &Elf32_Ehdr of this program

        cld
        lodsd
        push eax  // sz_uncompressed  (maximum dstlen for lzma)
        mov ecx,esp  // save &dstlen
        push eax  // space for 5th param
        push ecx  // &dstlen
        push edx  // &dst
        lodsd
        push eax  // sz_compressed  (srclen)
        lodsd     // last 4 bytes of b_info
        mov [4*3 + esp],eax
        push esi  // &compressed_data
        call ebp  // decompress(&src, srclen, &dst, &dstlen, b_info.misc)
        add esp, 0+(5+1 + 6-1)*4  // (5+1) args to decompress, (6-1) args to mmap
        ret      // &destination
main:
        pop ebp  // &decompress
        call unfold
            // compressed fold_elf86 follows
eof:

// vi:ts=8:et:nowrap

