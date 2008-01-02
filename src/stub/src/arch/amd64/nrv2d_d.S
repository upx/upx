/* nrv2d_d.S -- AMD64 decompressor for NRV2D

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
   Copyright (C) 2000-2008 John F. Reiser
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

   Markus F.X.J. Oberhumer              Laszlo Molnar
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>

   John F. Reiser
   <jreiser@users.sourceforge.net>
*/

        cmpb $ M_NRV2D_LE32,methb; je top_n2d; jmp not_n2d

lit_n2d:
        incq %rsi; movb %dl,(%rdi)
        incq %rdi
top_n2d:
        movb (%rsi),%dl  // speculate: literal, or bottom 8 bits of offset
        jnextb1yp lit_n2d
        lea 1(lenq),off  // [len= 0] off= 1
        jmp getoff_n2d

off_n2d:
        dec off
        getnextbp(off)
getoff_n2d:
        getnextbp(off)
        jnextb0np off_n2d

        subl $ 3,off; jc offprev_n2d
        shll $ 8,off; movzbl %dl,%edx
        orl %edx,off; incq %rsi
        xorl $~0,off; jz eof
        sarl     off  // Carry= original low bit
        movslq   off,disp  // XXX: 2GB
        jmp len_n2d-2  // CHEAT: assume tail of getnextb is "adcl len,len"
offprev_n2d:
        getnextb(len)
len_n2d:
        getnextb(len); jne gotlen_n2d
        inc len  // len= 1
lenmore_n2d:
        getnextb(len)
        jnextb0n lenmore_n2d
        addl $2,len
gotlen_n2d:
        cmpq $-0x500,disp
        adcl $1,len  // len += 1+ (disp < -0x500);
        call copy
bot_n2d:  // In: 0==len
        jmp top_n2d

not_n2d:

/*
vi:ts=8:et:nowrap
*/

