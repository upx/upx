/* ppc_bxx.S -- PowerPC Branch Trick unfilter

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

#include "ppc_regs.h"

ppcbxx:  # (*f_unf)(xo->buf, out_len, h.b_cto8, h.b_ftid);
#define W_CTO 4  /* must match filteri/ppcbxx.h */

#define ptr  a0
#define len  a1
#define cto8 a2
#define ftid a3

#define ptr0 a4

        cmpli cr0,ftid,0xd0;       bnelr- cr0  # if (0xd0!=ftid)   return;
        rlwinm. len,len,32-2,2,31; beqlr- cr0  # if (0==(len>>=2)) return;
        lis r0,-(~0<<(32-16- (2+6+ W_CTO)))  # limit in 32-bit words
        cmpl cr0,len,r0
        blt  cr0,L5
        mr       len,r0
L5:
        addi cto8,cto8,18<<W_CTO  # cat(bxx_opcode, cto8)
        movr ptr0,ptr  # save base address
        addi ptr,ptr,-4  # prepare for 'lwzu'
        mtctr len  # count of words
        b L20
L10:
        rlwinm t1,t0,0,6+W_CTO,31-2  # the displacement field in position
        subf   t1,ptr,t1   # raw distance
        add    t1,t1,ptr0  # relative to virtual address of base
        rlwimi t0,t1,0,6      ,31-2  # unfiltered instruction
        stw    t0,0(ptr)   # replace in memory
        bdzlr-  # if (0==--ctr) return;  // extremely unlikely
L20:
        lwzu t0,4(ptr)  # t0= *++ptr;
        rlwinm t1,t0,6+W_CTO,32-(6+W_CTO),31  # t1= top (6+W_CTO) bits of t0
        cmpl cr0,t1,cto8; beq- cr0,L10  # unconditional branch marked with cto8; unlikely
        bdnz+ L20  # if (0!=--ctr) goto L20;  // likely
        ret

#undef ptr0
#undef ftid
#undef cto8
#undef len
#undef ptr
