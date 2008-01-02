/* nrv2e_d.S -- AMD64 decompressor for NRV2E

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

        cmpb $ M_NRV2E_LE32,methb; je top_n2e; jmp not_n2e

lit_n2e:
        incq %rsi; movb %dl,(%rdi)
        incq %rdi
top_n2e:
        movb (%rsi),%dl  # speculate: literal, or bottom 8 bits of offset
        jnextb1yp lit_n2e
        lea 1(lenq),off  # [len= 0] off= 1
        jmp getoff_n2e

off_n2e:
        dec off
        getnextbp(off)
getoff_n2e:
        getnextbp(off)
        jnextb0np off_n2e

        subl $ 3,off; jc offprev_n2e
        shll $ 8,off; movzbl %dl,%edx
        orl %edx,off; incq %rsi
        xorl $~0,off; jz eof
        sarl off  # Carry= original low bit
        movslq off,disp  # XXX: 2GB
        jc lenlast_n2e
        jmp lenmore_n2e

offprev_n2e:
        jnextb1y lenlast_n2e
lenmore_n2e:
        incl len  # len= 1
        jnextb1y lenlast_n2e
len_n2e:
        getnextb(len)
        jnextb0n len_n2e
        addl $6-2-2,len
        jmp gotlen_n2e

lenlast_n2e:
        getnextb(len)  # 0,1,2,3
gotlen_n2e:
        cmpq $-0x500,disp
        adcl $2,len  # len += 2+ (disp < -0x500);
        call copy
bot_n2e:  # In: 0==len
        jmp top_n2e

not_n2e:

/*
vi:ts=8:et:nowrap
*/

