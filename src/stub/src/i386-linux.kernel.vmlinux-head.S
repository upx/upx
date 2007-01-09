/*
;  i386-linux.kernel.vmlinux-head.S -- set up stack for vmlinux/i386 format
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 2006 John Reiser
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
;  John Reiser
;  <jreiser@users.sourceforge.net>
*/

#include "arch/i386/macros.S"
        .att_syntax prefix

// In: %esi=0x90000 setup data "real_mode pointer"
        #cli  # this must be true already
////    .byte 0xf1  # qemu debug only

        /* The only facts about segments here, that are true for all kernels:
         * %cs is a valid "flat" code segment; no other segment reg is valid;
         * the next segment after %cs is a valid "flat" data segment, but
         * no segment register designates it yet.
         */
        movl %cs,%eax; addl $1<<3,%eax  # the next segment after %cs
        movl %eax,%ds
        movl %eax,%es
        movl %eax,%fs
        movl %eax,%gs
        leal 0x9000(%esi),%ecx  # 0x99000 typical
        movl %ecx,-8(%ecx)  # 32-bit offset for stack pointer
        movl %eax,-4(%ecx)  # segment for stack pointer
        lss -8(%ecx),%esp  # %ss:%esp= %ds:0x99000
            /* Linux Documentation/i386/boot.txt "SAMPLE BOOT CONFIGURATION" says
                 0x8000-0x8FFF  Stack and heap  [inside the "real mode segment",
                 just below the command line at offset 0x9000].

                 arch/i386/boot/compressed/head.S "Do the decompression ..." says
                 %esi contains the "real mode pointer" [as a 32-bit addr].

                 In any case, avoid EBDA (Extended BIOS Data Area) below 0xA0000.
                 boot.txt says 0x9A000 is the limit.  LILO goes up to 0x9B000.
            */

        pushl $0; popf  # subsumes "cli; cld"; also clears NT for buggy BIOS

// PackVmlinuxI386::pack knows the format of the following instruction.
        call COMPRESSED_LENGTH
// Compressed data appears >here<, then decompressor.

// vi:ts=8:et:nowrap
