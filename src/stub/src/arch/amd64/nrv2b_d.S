/* amd_d_nrv2b.S -- AMD64 decompressor for NRV2B

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

        cmpb $ M_NRV2B_LE32,methb; je top_n2b; jmp not_n2b

lit_n2b:
        incq %rsi; movb %dl,(%rdi)
        incq %rdi
top_n2b:
        movb (%rsi),%dl  # speculate: literal, or bottom 8 bits of offset
        jnextb1yp lit_n2b
        lea 1(lenq),off  # [len= 0] off= 1
offmore_n2b:
        getnextbp(off)
        jnextb0np offmore_n2b

        subl $ 3,off; jc len_n2b  # use previous offset
        shll $ 8,off; movzbl %dl,%edx
        orl %edx,off; incq %rsi
        xorl $~0,off; jz eof
        movslq off,disp  # XXX: 2GB
len_n2b:
        lea 1(lenq),off  # [len= 0] off= 1
        getnextb(len); getnextb(len)  # two bits; cc set on result
        jnz gotlen_n2b  # raw 1,2,3 ==> 2,3,4
        movl off,len  # len= 1, the msb
        addl $3-1,off  # raw 2.. ==> 5..
lenmore_n2b:
        getnextb(len)
        jnextb0n lenmore_n2b
gotlen_n2b:
        cmpq $-0xd00,disp
        adcl off,len  # len += off + (disp < -0xd00)
        call copy
bot_n2b:  # In: 0==len
        jmp top_n2b

not_n2b:
/*
vi:ts=8:et:nowrap
*/

