/*  l_lx_elf64amd.S -- Linux program entry point & decompressor (Elf binary)
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) 1996-2004 Laszlo Molnar
*  Copyright (C) 2000-2005 John F. Reiser
*  All Rights Reserved.
*
*  UPX and the UCL library are free software; you can redistribute them
*  and/or modify them under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; see the file COPYING.
*  If not, write to the Free Software Foundation, Inc.,
*  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
*  Markus F.X.J. Oberhumer              Laszlo Molnar
*  <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>
*
*  John F. Reiser
*  <jreiser@users.sourceforge.net>
*/

sz_l_info= 12
  l_lsize= 8

sz_p_info= 12

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4
  b_method= 8

PROT_READ=  1
PROT_WRITE= 2
PROT_EXEC=  4

MAP_PRIVATE= 2
MAP_FIXED=     0x10
MAP_ANONYMOUS= 0x20

SYS_mmap= 9  # 64-bit mode only!

PAGE_SHIFT= 12
PAGE_MASK= (~0<<PAGE_SHIFT)
PAGE_SIZE= -PAGE_MASK

/*__LEXEC000__*/
_start: .globl _start
/* The following 'call' must be at _start; fold_begin knows this,
   and so does PackLinuxElf64amd::pack3() .
*/
        call main  # push &decompress
#include "amd_d_nrv2e.S"

#include "amd_regs.h"

/* Decompress the rest of this loader, and jump to it.
   Map a page to hold the decompressed bytes.  Logically this could
   be done by setting .p_memsz for our first PT_LOAD.  But as of 2005-11-09,
   linux 2.6.14 only does ".bss expansion" on the PT_LOAD that describes the
   highest address.  [I regard this as a bug, and it makes the kernel's
   fs/binfmt_elf.c complicated, buggy, and insecure.]  For us, that is the 2nd
   PT_LOAD, which is the only way that linux allows to set the brk() for the
   uncompressed program.  [This is a significant kernel misfeature.]
*/
unfold:
        pop %rbx  # &b_info

        .byte 7+0xB8; .ascii "ADRM"  # movl $'ADRM',%edi
        push $ PROT_READ | PROT_WRITE | PROT_EXEC; pop %arg3
        .byte 6+0xB8; .ascii "LENM"  # movl $'LENM',%esi
        push $ MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS; pop %sys4
        subl %arg5l,%arg5l  #; subl %arg6l,%arg6l  # MAP_ANON ==> ignore offset
        push $ SYS_mmap; pop %rax
        syscall  # %rax= result; trashes %rcx,%r11 only
        cmpl %eax,%edi; je 0f; hlt; 0:  # XXX: 4GB

        .byte   0x68; .ascii "JMPU"  # push $'JMPU'  # for unmap in fold
        .byte   0x68; .ascii "ADRU"  # push $'ADRU'  # for unmap in fold
        .byte 6+0xB8; .ascii "ADRC"  # movl $'ADRC',%esi
        .byte   0x68; .ascii "LENU"  # push $'LENU'  # for unmap in fold
        .byte 1+0xB8; .ascii "CNTC"  # movl $'CNTC',%ecx
        .byte   0x68; .ascii "ADRX"  # push $'ADRX'  # for upx_main
        .byte   0x68; .ascii "LENX"  # push $'LENX'  # for upx_main

        movl %edi,%edx
        subl %esi,%edx  # relocation amount
        addl      %edx,%ebp  # update &decompress
        addl      %edx,%ebx  # update &b_info

        rep; movsq
        xchgl %eax,%edi

        movl %ebx,%esi  # %arg2l= &b_info (relocated)
        push %rax  # ret_addr after decompression
               xchgl %eax,%arg3l  # %arg3= dst for unfolding  XXX: 4GB
        lodsl; movl  %esi,%arg4l  # &len_dst ==> &do_not_care  XXX: 4GB
        lodsl; xchgl %eax,%arg1l  # sz_cpr  XXX: 4GB
        lodsl; movzbl %al,%arg5l  # b_method
              xchg %arg1l,%arg2l  # XXX: 4GB
        jmp *%rbp  # goto decompress; return to unfolded loader
main:
        # int3  # uncomment for debugging
        pop %rbp  # &decompress
        call unfold  # push &b_info
        /* { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...} */

eof:
/*__XTHEENDX__*/

/*
vi:ts=8:et:nowrap
*/

