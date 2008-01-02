/* amd_bxx.S -- AMD x86_64 Call/Branch Trick unfilter

   This file is part of the UPX executable compressor.

   Copyright (C) 2005-2008 John F. Reiser
   All Rights Reserved.

   UPX and the UCL library are free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   John F. Reiser
   <jreiser@users.sourceforge.net>
*/

#include "regs.h"

amdbxx:  # (*f_unf)(xo->buf, out_len, h.b_cto8, h.b_ftid);

#define ptr  %arg1  /* known to be %rdi */
#define len  %arg2
#define cto8 %arg3l /* known to be "%dl" */
#define ftid %arg4l

        cmpl $0x49,ftid; jne ckend  # filter: JMP, CALL, 6-byte Jxx
        movq len,%rcx  # byte count
        movq ptr,%rsi  # remember start of buffer
        jmp ckstart
ckloop3:
        movb (%rdi),%al; addq $1,%rdi
        cmpb $0x80,%al; jb ckloop2  # lo of 6-byte Jcc
        cmpb $0x8F,%al; ja ckloop2  # hi of 6-byte Jcc
        cmpb $0x0F,-2(%rdi); je ckmark  # prefix of 6-byte Jcc
ckloop2:
        subb $     0xE8,%al
        cmpb $0xE9-0xE8,%al; ja ckcount  # not JMP, not CALL
ckmark:
        cmpb %dl,(%rdi); jne ckcount  # not marked with cto8
        movl (%rdi),%eax  # the marked, bswapped 32-bit displacement
        andl $~0<<8,%eax  # clear the mark
        bswap %eax
        subl %edi,%eax
        addl %esi,%eax
        stosl  # *%rdi++ = %eax;
ckstart:
        subq $4,%rcx
        movb (%rdi),%al; addq $1,%rdi
        decq %rcx; jnz ckloop2  # prefix cannot overlap previous displacement
        jmp ckend
ckcount:
        decq %rcx; jnz ckloop3
ckend:
        ret
