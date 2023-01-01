/*
;  i386-linux.elf-entry.S -- Linux program entry point & decompressor (Elf binary)
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2023 Laszlo Molnar
;  Copyright (C) 2000-2023 John F. Reiser
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
;  <markus@oberhumer.com>               <ezerotven+github@gmail.com>
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
sz_pack2 = -4+ _start
_start: .globl _start
////    nop; int3  // DEBUG
        push eax  // space for %entry address
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
f_exp:  // synonym

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

PAGE_SHIFT= 12
PAGE_SIZE= ( 1 << PAGE_SHIFT)
PAGE_MASK= (~0 << PAGE_SHIFT)

MAP_FIXED=     0x10
MAP_PRIVATE=   0x02
MAP_ANONYMOUS= 0x20

PROT_READ=      1
PROT_WRITE=     2
PROT_EXEC=      4

__NR_exit=  1
__NR_write= 4
__NR_open=  5
__NR_mmap=      90
__NR_mprotect= 125

szElf32_Ehdr= 0x34
p_memsz=  5*4

sz_unc= 0
sz_cpr= 4

EINVAL= 22

#define pushsbli .byte 0x6a,  /* push sign-extended byte to long immediate*/

msg_SELinux:
        pushsbli L71 - L70
        pop edx  // length
        call L71
L70:
        .ascii "mmap failed.\n"
L71:
        pop ecx  // message text
        push 2; pop ebx  // fd stderr
        push __NR_write; pop eax; int 0x80
die:
        mov bl, 127  // only low 7 bits matter!
        push __NR_exit; pop eax; int 0x80

// Decompress the rest of this loader, and jump to it
unfold:  // IN: ebp= f_exp;  esp/ &O_BINFO
        pop esi; lodsd  // skip O_BINFO; esi= &compressed fold_elf86
        lea edi,[sz_pack2 - f_exp + ebp]
        sub edi,[edi]  // edi= &Elf32_Ehdr of this program

// Open /proc/self/exe
        call 0f; .asciz "/proc/self/exe"; 0: pop ebx  // path
        sub ecx,ecx  // O_RDONLY
        push __NR_open; pop eax; int 0x80; push eax  // P_31  fd

// Reserve space for unfolded stub
        push 0  // offset
        push -1  // fd
        push MAP_PRIVATE|MAP_ANONYMOUS
          mov edx,[esi]  // .sz_unc
        push PROT_READ|PROT_WRITE
        push edx  // len
        push 0 // kernel chooses addr
        call mmap
        push edx  // P_23  LENU
        push eax  // P_24  ADRU
        xchg eax,edx  // edx= ADRU

// Unfold
        lodsd; push eax; mov ebx,esp  // sz_unc
        lodsd; xchg ecx,eax  // sz_cpr
        lodsd; push eax  // b_method
        push ebx  // &sz_unc
        push edx  // dst
        push ecx  // sz_cpr
        push esi  // src
        call ebp  // decompress
        add esp,6*4

// PROT_EXEC
        pop %ebx  // P_24 ADRU
        pop %ecx  // P_23 LENU
        push PROT_EXEC|PROT_READ; pop edx
        push %ecx  // P_23 LENU
        push %ebx  // P_24 ADRU
        push __NR_mprotect; pop eax; int 0x80

// Jump to unfolded code
        mov eax,[-4*4 + esi]  // O_BINFO
        add eax,edi  // ADRX
        mov ecx,[sz_pack2 - f_exp + ebp]  // LENX(==sz_pack2)
// eax:ADRX; ebx:free; ecx:LENX
// ebp:f_exp; esi:&b_info(fold); edi:dynbase
        pop %edx; push %edx
        jmp edx  // esp/ ADRU,LENU,fd,%entry,argc,argv,0,...

mmap:  // oldmmap: ebx -> 6 arguments; remove arguments on return
        lea ebx,[4+esp]
        push __NR_mmap; pop eax; int 0x80
        cmp eax,PAGE_MASK; jb 0f; hlt; 0:
        ret 6*4

main:
        pop ebp  // &decompress
        call unfold
        .long O_BINFO
            // compressed fold_elf86 follows
eof:

/* vim:set ts=8 sw=8 et: */
