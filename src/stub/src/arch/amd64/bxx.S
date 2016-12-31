/* amd_bxx.S -- AMD x86_64 Call/Branch Trick unfilter

   This file is part of the UPX executable compressor.

   Copyright (C) 2005-2017 John F. Reiser
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

#ifndef NO_METHOD_CHECK
        cmpl $0x49,ftid; jne ckend  # filter: JMP, CALL, 6-byte Jxx
        movq len,%rcx  # byte count
#endif
        movq ptr,%rsi  # remember start of buffer
        jmp ckstart
ckloop3:
        movb (%rdi),%al; incq %rdi
        cmpb $0x80,%al; jb ckloop2  # lo of 6-byte Jcc
        cmpb $0x8F,%al; ja ckloop2  # hi of 6-byte Jcc
        cmpb $0x0F,-2(%rdi); je ckmark  # prefix of 6-byte Jcc
ckloop2:
        subb $     0xE8,%al
        cmpb $0xE9-0xE8,%al; ja ckcount  # not JMP, not CALL
ckmark:
        cmpq $4,%rcx; jb ckend  # peek only; not marked ==> do not consume
        movl (%rdi),%eax  # (assume) marked, bswapped 32-bit displacement
        subb %dl,%al; jne ckcount  # not marked with cto8
        bswap %eax  # (0<<24) | d24
        subl %edi,%eax
        addl %esi,%eax
        subq $4,%rcx  # consume; length was checked before the fetch
        stosl  # *%rdi++ = %eax;

/* We might do "extra" work checking opcodes that are too close to the end.
   But not having 4 bytes for displacement is caught by ckmark,
   and it is simpler to count exactly the bytes that are consumed.
*/
ckstart:  # %rcx might be 0 on fall-through from above
        subq $1,%rcx; jb ckend  # 'dec' does not set Carry
        movb (%rdi),%al; incq %rdi
        jmp ckloop2  # 0x0F prefix must not overlap previous displacement
ckcount:
        subq $1,%rcx; jnb ckloop3  # 0x0F prefix is allowed
ckend:
#ifndef NO_METHOD_CHECK
        ret
#endif

#undef ptr
#undef len
#undef cto8
#undef ftid
