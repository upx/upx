/*
;  i386-darwin.dylib-entry.S -- program entry point & decompressor (i386 Mach-o)
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
// We have been CALLed as a subroutine from dyld; C-language rules apply.
// -4*4+_start: .long offset(user_init_function)
// -3*4+_start: .long offset(&b_info of compressed Mach_headers)
// -2*4+_start: .long length(compressed __TEXT)
// -1*4+_start: .long total_length  # of preceding bytes in file
**************************************************************************/

section LEXEC000
_start: .globl _start
////    int3  # for debug only
        push eax  # space for &user_init_function
        pusha
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
//;;              align   8

#include "arch/i386/nrv2b_d32.S"
#include "arch/i386/nrv2d_d32.S"
#include "arch/i386/nrv2e_d32.S"
#include "arch/i386/lzma_d.S"

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
                cit32   edi
section LEXEC017
                popa
                ret

section LEXEC020

#define PAGE_SIZE ( 1<<12)

sz_Mach_header= 7*4
mh_sizeofcmds=5*4

seg_vmaddr=2*4+16
seg_vmsize=4+seg_vmaddr
seg_filesize=2*4+seg_vmsize

sz_l_info=3*4
sz_p_info=3*4
sz_b_info=3*4
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

#define MAP_FIXED     0x10
#define MAP_PRIVATE   0x02
#define MAP_ANON    0x1000
#define PROT_READ      1
#define PROT_WRITE     2
#define PROT_EXEC      4


main:
        pop ebp  # &decompress
        lea ebx,[-4+ _start - decompress + ebp]  # &total_length
        mov eax,[-1*4 + ebx]  # length(compressed __TEXT)
        add eax,offset(dy_top)
        sub eax,offset(decompress)

        push eax  # length for eventual munmap

        push 0  # hi32(offset)
        push 0  # lo32(offset)
        push -1  # fd
        push MAP_ANON|MAP_PRIVATE
        push PROT_READ|PROT_WRITE
        push eax  # length
        push 0  # addr
        call mmap
        add esp,7*4

        push eax  # addr for eventual munmap

        // Copy interval [decompress, dy_top).
        mov esi,ebp  # decompressor
        mov ebp,eax  # new location
        mov edi,eax  # dst for decompressor
        mov ecx,offset(dy_top)
        sub ecx,offset(decompress)
        cld; rep movsb

        // Goto the copied dy_reloc.
        lea eax,[-offset(dy_top - dy_reloc) + edi]
        jmp %eax
dy_reloc:

        // Copy compressed __TEXT.
        push edi  # remember start of compressed __TEXT
        mov edx,ebx  # &total_length
          mov eax,[-3*4+ebx]  # offset(user_init_function)
        sub edx,[ebx]  # runtime base address
          add eax,edx; mov [(1+2+8)*4 + esp],eax  # relocate &user_init_function
        mov esi,[-2*4 + ebx]; add esi,edx
        mov ecx,[-1*4 + ebx]
        rep movsb
        pop esi  # &b_info for Mach_header
        mov edi,edx  # runtime base address

        // Decompress __TEXT, but do not overwrite Mach_headers
        // in order to maintain consistency with dyld partial caching of them.
        // So, skip the first compressed block.
        lodsd; add edi,eax  # sz_unc
        lodsd; add esi,eax  # sz_cpr
        lodsd  # b_method
dy_uncpr:
        push esi; push edi  # save in case unfilter

        lodsd; test eax,eax; jz dy_done
        push eax  // sz_uncompressed  (maximum dstlen for lzma)
        mov ecx,esp  // save &dstlen
        push eax  // space for 5th param b_info.misc
        push ecx  // &dstlen
        push edi  // dst
        add edi,eax  // next dst
        lodsd; push eax  // sz_compressed  (srclen)
        mov ecx,eax
        lodsd; mov [3*4 + esp],eax // last 4 bytes of b_info
        push esi  // &compressed __TEXT
        add esi,ecx  // next src
        call ebp  // decompress(src, srclen, dst, &dstlen, b_info.misc)
        add esp, (5+1)*4  // (5+1) args to decompress

        pop edx; pop eax  # edx= old dst; eax= old &b_info
        movzx ecx, byte ptr [1+ b_method + eax]; jecxz dy_uncpr; push ecx  # ftid
        movzx ecx, byte ptr [2+ b_method + eax];                 push ecx  # cto8
        push [sz_unc + eax]
        push edx    # dst
        lea eax,[2+ ebp]; call eax  # f_unfilter(dst, dstlen, cto8, ftid)
        add esp,4*4
        jmp dy_uncpr

SYS_mmap    =197
mmap:
        mov eax,SYS_mmap
        call sysgo; jncs 0f; or eax,~0
0:
        ret

SYS_munmap=73
dy_done:
        pop eax  # discard, leaving 1 junk word below the regs for POPA
        lea edx,[-5+ edi]  # steal some space at high end of __TEXT
        mov  byte ptr [   edx],      0x58  # pop eax
        mov dword ptr [1+ edx],0xc3615858  # pop eax; pop eax; popa; ret
        mov eax,SYS_munmap
        push edx  # retaddr
sysgo:
        pop edx  # return address for sysenter
        mov ecx,esp  # &{user_ret, arg1, arg2, ...}
        .byte 0x0f, 0x34  # sysenter

dy_top:

// vi:ts=8:et:nowrap
