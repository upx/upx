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
NBPW= 4

O_RDWR= 2
O_CREAT= 0100
O_DIRECTORY= 0200000   // 0x10000
O_TMPFILE= 020000000  // 0x400000

__NR_exit=  1
__NR_write= 4
__NR_open=  5
__NR_close= 6
__NR_mmap=      90
__NR_memfd_create= 356
__NR_mprotect= 125

/*************************************************************************
// program entry point
// see glibc/sysdeps/i386/elf/start.S
**************************************************************************/

section LEXEC000
sz_pack2 = -4+ _start
_start: .globl _start
    nop; int3  // DEBUG
        call here; jmps L20  // esi= L05
L05:  Dsz05 = sz_pack2 - L05
        .asciz "/dev/shm"
L07:  D0705 = L07 - L05
        .asciz "/proc/self/exe"
L20:
// memfd_create() gets ENOSYS on early Android.  There are 32-bit x86 Android
// such as Zenfone 2 (discontinued 2018?), x86 Chromebooks (2019 and later),
// FydeOS, Windows subsystem for Android.  But the main use is for developing,
// to make Android emulator running on x86_64 (Linux or Windows) run faster
// by emulating x86 instead of ARM.
//
// So fall back to an unnamed temporary file in /dev/shm.
// (Won't work if /dev/shm is mounted 'noexec'; too bad.)
        mov edx,0700  // S_IRWXU  permissions
        mov ecx,O_RDWR | O_DIRECTORY | O_TMPFILE // flags
        mov ebx,esi  // L05
        push __NR_open; pop eax; call sys_check
        push eax  // mfd in %entry

        sub ecx,ecx  // 0; O_RDONLY
        lea ebx,[D0705 + esi] // "/proc/self/exe"
        push __NR_open; pop eax; call sys_check
        pop ebx; push ebx  // mfd
        push eax  // xfd

        jmp unfold
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
decompress:
f_exp:  // synonym
Dfx05= f_exp - L05

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
MAP_SHARED=    0x01
MAP_PRIVATE=   0x02
MAP_ANONYMOUS= 0x20

PROT_READ=      1
PROT_WRITE=     2
PROT_EXEC=      4

szElf32_Ehdr= 0x34
p_memsz=  5*4

sz_unc= 0
sz_cpr= 4

EINVAL= 22

// Decompress the rest of this loader, and jump to it
unfold:
        lea ebp,[Dfx05 + esi]  // f_exp
        lea edi,[Dsz05 + esi]  // &sz_pack2
        mov edx,[edi]  // sz_pack2
        sub edi,[edi]
        push edi  // P_40   &Elf32_Ehdr,xfd,mfd; argc...
        call fold_data; pop esi  // %esi/ O_BINFO,b_info
        lodsd; sub edx,eax; push edx // P_41  LENX
        add eax,edi; push eax  // P_42  ADRX,LENX,&Elf32_Ehdr,xfd,mfd; argc...

// alloca() for unfolded stub onto stack (at most 64K: l_info.l_lsize)
        mov eax,esp
        sub eax,[esi]  // - .sz_unc
        and eax,-2*NBPW  // aligned
        mov edi,eax  // dst for unfold
        xchg eax,esp  // installed
        push eax  // P_52 old sp in ADRU

#define pushsbli .byte 0x6a,

// Unfold
        push esi  // P_59 &b_info
        lodsd; push eax; mov edx,esp  // P_53 LENU sz_unc
        lodsd; xchg ecx,eax  // sz_cpr
        lodsd; push eax  // P_54  b_method
        push edx  // P_55  &sz_unc
        push edi  // P_56  dst
        push ecx  // P_57  sz_cpr
        push esi  // P_58  src
        call ebp  // decompress
        pop ecx  // P_58 src
        pop ecx  // P_57 sz_cpr

        pop ecx  // P_56  dst
        pop edx; mov edx,[edx]  // P_55  sz_unc
        //mov ebx,ebx  // mfd
        pop eax  // P_54  b_method
        pushsbli __NR_write; pop eax; call sys_check

        pushsbli 0  // P_60  offset
        push ebx  // P_61  mfd
        pushsbli MAP_SHARED  // P_62
        pushsbli PROT_EXEC | PROT_READ  // P_63
        push [-3*NBPW + esi]  // P_64  sz_unc len LENU
        pushsbli 0  // P_65
        call mmap  // P_65 P_64 P_63 P_62 P_61 P_60
        pop ecx  // P_53  LENU
        pop esi  // P_59 &b_info
        pop esp  // P_52  ADRX,LENX,&Elf32_Ehdr,xfd,mfd, argc...

        pop edx  // P_42  ADRX
        pop ecx  // P_41  LENX
        pop edi  // P_40  &Elf32_Ehdr
        push [esi]  // P_70  LENU
        push eax  // P_71  ADRU
        push ecx  // P_72  LENX
        push edx  // P_73  ADRX

        mov ebx,[4*NBPW + esp]  // mfd
        pushsbli __NR_close; pop eax; call sys_check
        
// Jump to unfolded code
        pop eax  // P_73  ADRX
        pop ecx  // P_72  LENX
// eax:ADRX; ebx:free; ecx:LENX
// ebp:f_exp; esi:&b_info(fold); edi:dynbase
        pop %edx; push %edx
        jmp edx  // esp/ ADRU,LENU,xfd,%entry,argc,argv,0,...

mmap:  // oldmmap: ebx -> 6 arguments; remove arguments on return
        lea ebx,[4+esp]
        push __NR_mmap; pop eax; call sys_check
        ret 6*4

sys_check:
        push eax
        int 0x80
        pop edx  // recover __NR_ for debug
        cmp eax,PAGE_MASK; jae 0f; ret; 0:
        hlt
here:
        mov esi,[esp]
        lodsw  // esi += 2
        ret

fold_data:
        pop eax; call eax
        .long O_BINFO
            // compressed fold_elf86 follows
eof:

/* vim:set ts=8 sw=8 et: */
