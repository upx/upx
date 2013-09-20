/*
;  i386-linux.shlib-init.S -- Linux shared library init & decompressor (Elf binary)
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
//  .long .  // compress-time virtual address (detect runtime relocation)
//  .long user DT_INIT
//  .long &escape_hatch
//  .long &{p_info; b_info; compressed data}
_start: .globl _start
  ////    int3
        push eax; pusha
        mov ebp,esp
o_uinit= 8*4  // beyond saved registers
//o_reloc= 6*4  // saved ecx

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

#define PAGE_SIZE ( 1<<12)
PAGE_MASK= -PAGE_SIZE  // AND clears the offset within page

#define MAP_FIXED     0x10
#define MAP_PRIVATE   0x02
#define MAP_ANONYMOUS 0x20
#define PROT_READ      1
#define PROT_WRITE     2
#define PROT_EXEC      4
#define __NR_mmap     90
#define __NR_munmap   91
#define __NR_mprotect 125
#define szElf32_Ehdr 0x34
#define p_memsz  5*4
sz_p_info = 3*4
sz_l_info = 3*4
sz_b_info=3*4
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

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
        push __NR_write; pop eax; int 0x80
die:
        mov bl, 127  // only low 7 bits matter!
        push __NR_exit; pop eax; int 0x80

main:
//  1. allocate temporary pages
//  2. copy to temporary pages:
//       fragment of page below dst; compressed src;
//       decompress+unfilter; supervise
//  3. mmap destination pages for decompressed data
//  4. create escape hatch
//  5. jump to temporary pages
//  6. uncompress
//  7. unfilter
//  8. mprotect decompressed pages
//  9  setup args for unmap of temp pages
// 10. jump to escape hatch
// 11. unmap temporary pages
// 12. goto user DT_INIT

        pop edx  // &decompress

        lea esi,[edx + _start - decompress - 4*4]
               mov ecx,esi
        lodsd; sub ecx,eax; //mov [ebp+o_reloc],ecx
        lodsd; add eax,ecx; mov [ebp+o_uinit],eax  // reloc DT_INIT  for step 12
        lodsd; add eax,ecx; push eax  // reloc &hatch   for step 10
p_hatch= -1*4
        lodsd; add eax,ecx; xchg eax,edi  // &l_info; also destination for decompress
        lea esi,[edi + sz_l_info + sz_p_info]  // &b_info

        push eax; push eax  // param space: munmap temp pages  step 9
p_unmap= -3*4

        lodsd; lodsd; add esi,eax; lodsd  // skip unpack helper block

        lodsd  // eax=dstlen
        mov ebx,edi
        and ebx,~PAGE_MASK  // ebx= fragment
        add eax,ebx  // extend to page-aligned
        sub edi,ebx
        push eax; push edi  // params: mprotect restored pages  step 8
p_mprot= -5*4
        sub eax,ebx  // restore
        add edi,ebx

        movzx ecx, byte ptr [esi+b_method-4+1]; push ecx  // ftid
        movzx ecx, byte ptr [esi+b_method-4+2]; push ecx  // cto8
        push eax; mov ecx,esp  // dstlen  also for unfilter  step 7
        push edi  // dst                 param for unfilter  step 7
p_unflt= -9*4
        push edx  // &decompress
o_uncpr= -10*4

        lodsd; xchg eax,edx  // edx= srclen
        lodsd; push eax  // method,filter,cto,junk
        push ecx  // &dstlen
        push edi  // dst
        push edx  // srclen
        push esi  // src;  arglist ready for decompress  step 6
p_uncpr= -15*4

        mov eax,[ebp+o_uncpr]  // &decompress
        add edx,[eax-4]  // l_d_cpr + l_f_unc

        call L220
supervise:
        // Allocate pages for result of decompressing.
        // These replace the compressed source and the following hole.
        push 0; push 0
        push MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED
        push PROT_READ|PROT_WRITE
        push [ebp+p_mprot+4]  // dstlen
        push [ebp+p_mprot]  // dst
        mov ecx,ebx  // save fragment
        mov ebx,esp
        push __NR_mmap; pop eax; int 0x80
                cmp eax,[ebx]; je 0f; hlt; 0:
        add esp,6*4

        // Restore fragment of page below dst
        xchg eax,edi
        mov esi,[ebp+p_unmap]
        add ecx,3; shr ecx,2  // FIXME: is this safe?
        rep movsd

        call [ebp+o_uncpr]  // decompress
// decompression can overrun dst by 3 bytes on i386; construct hatch now
        pop eax; pop eax  // discard src, srclen
        pop eax  // dst
        pop ecx  // &dstlen
        pop edx  // discard method,filter,cto,junk
        add eax,[ecx]  // dst += dstlen
        mov dword ptr [eax],0xc36180cd  // "int 0x80; popa; ret"
        mov [esp + p_hatch - o_uncpr],eax  // hatch at end of .text
//o_uncpr
        pop eax  // &decompress
//p_unflt
        cmp word ptr [esp+3*4],0; je 0f  // 0==ftid ==> no filter
        add eax,2; call eax  // unfilter {i386 f_unf==(2+f_unc)}
0:
        add esp,4*4
//p_mprot
        pop ebx  // dst including fragment
        pop ecx  // dstlen
        push PROT_READ|PROT_EXEC; pop edx
        push __NR_mprotect; pop eax; int 0x80
//p_unmap
        pop ebx  // &temp pages
        pop ecx  // length
        push __NR_munmap; pop eax
//p_hatch
        ret  // goto escape hatch
//hatch:
        int 0x80  // munmap temporary pages
        popa
        ret  // goto user DT_INIT

L220:
        pop esi  // &supervise
        add edx,[esi-4]  // total length to allocate
        add edx,ebx  // include fragment

        // Allocate pages to hold temporary copy.
        push 0; push 0
        push MAP_PRIVATE|MAP_ANONYMOUS
        push PROT_READ|PROT_WRITE|PROT_EXEC
        push edx  // length with fragment
        push 0  // addr
        mov ecx,ebx  // save fragment
        mov ebx,esp  // & vec
        push __NR_mmap; pop eax; int 0x80
                cmp eax,PAGE_MASK; jb 0f; hlt; 0:
        add esp,6*4
        mov ebx,ecx  // save fragment

        mov [ebp+p_unmap  ],eax  // addr
        mov [ebp+p_unmap+4],edx  // length with fragment
        xchg eax,edi  // edi= dst
        xchg eax,esi  // eax= &supervise
//p_uncpr
        mov esi,[ebp+p_mprot]
        add ecx,3; shr ecx,2  // FIXME: is this safe?
        rep movsd  // copy the fragment

        pop esi  // &src data (after fragment)
                pop ecx; push ecx  // length
        push edi  // &copied data (after fragment)
        add ecx,3; shr ecx,2
        rep movsd  // copy compressed data

        mov esi,[ebp+o_uncpr]
        mov     [ebp+o_uncpr],edi
        mov ecx,[esi-4]
        rep movsb

//o_super
        xchg eax,esi  // esi= &supervise
        push edi  // &copied
        mov ecx,[esi-4]
        rep movsb

        ret  // goto copied supervise:

// empty sections for commonality with non-shlib
section LUNMP000
section LUNMP001
section LEXEC025
section LEXECDYN

// vi:ts=8:et:nowrap

