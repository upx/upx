/* i386_bxx.S -- i386 Call/Branch Trick unfilter

   This file is part of the UPX executable compressor.

   Copyright (C) 2005-2023 John F. Reiser
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

//#include "regs.h"

i386bxx:  # (*f_unf)(xo->buf, out_len, h.b_cto8, h.b_ftid);

#ifndef NO_METHOD_CHECK
        cmpl $0x49,4*NBPW(%esp); jne ckend0  # filter: JMP, CALL, 6-byte Jxx
#endif
        push %ebp; movl %esp,%ebp
        push %ebx  # C-language saved regs
        push %esi
        push %edi

        movl 2*NBPW(%ebp),%esi  // src
        movl 3*NBPW(%ebp),%ecx  // len
        movl 4*NBPW(%ebp),%edx  // b_cto8 (%dl)
        lea (1- 4)(%esi,%ecx),%ecx  # beyond last possible displacement
        movl %esi,%ebx  // start of buffer
        jmp ckstart
ckloop4:
        cmpl %ecx,%esi; jae ckend
        push %esi  # tail merge
ckloop3:
        pop %esi; lodsb  # next main opcode
        cmpb $0x80,%al; jb ckloop2  # lo of 6-byte Jcc
        cmpb $0x8F,%al; ja ckloop2  # hi of 6-byte Jcc
        cmpb $0x0F,-2(%esi); je ckmark  # prefix of 6-byte Jcc
ckloop2:
        subb $     0xE8,%al
        cmpb $0xE9-0xE8,%al; ja ckloop4  # not JMP, not CALL
ckmark:
        cmpl %ecx,%esi; jae ckend  # peek only; not marked ==> do not consume
        push %esi; lodsl  # (assume) marked, bswapped 32-bit displacement
        subb %dl,%al; jne ckloop3  # not marked with cto8
        pop %edi
        bswap %eax  # (0<<24) | d24
        subl %edi,%eax  # hardware: %esi;  software: %edi  [ 4==delta ]
        addl %ebx,%eax
        stosl
ckstart:
        cmpl %ecx,%esi; jae ckend
        lodsb; jmp ckloop2  # 0x0F prefix would overlap previous displacement
ckend:
        pop %edi
        pop %esi
        pop %ebx
        pop %ebp
ckend0:
#ifndef NO_METHOD_CHECK
        ret
#endif
