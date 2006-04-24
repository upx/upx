/*
 *  l_mac_ppc32.S -- program entry point & decompressor (PowerPC32 Mach-o)
 *
 *  This file is part of the UPX executable compressor.
 *
 *  Copyright (C) 2005 John F. Reiser
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
 *  John F. Reiser
 *  <jreiser@users.sourceforge.net>
 *
 */

#include "ppc_regs.h"

/*__MACOS000__*/
_start: .globl _start
        call main  # must be exactly 1 instruction; link_register= &decompress
#include "ppc_d_nrv2e.S"

/* Temporary until we get the buildLoader stuff working ... */
        .ascii "\n$Id: UPX (C) 1996-2006 the UPX Team. "
        .asciz "All Rights Reserved. http://upx.sf.net $\n"
        .p2align 2  # (1<<2)

sz_b_info= 12
  sz_unc= 0
  sz_cpr= 4

/* Decompress the rest of this loader, and jump to it. */
unfold:
        mflr t0  # -4+ &{ b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...}

        lwz lsrc,4+sz_cpr(t0); mtctr lsrc  # length to copy (and decompress)
        lwz ldst,4+sz_unc(t0)
        add  dst,lsrc,t0; addi dst,dst,4+sz_b_info
        add  src,ldst,t0; addi src,src,GAP+64  # defend against prefetch and overlap
movup:  # descending copy moves folded_loader to higher address
        lbzu r0,-1(dst)
        stbu r0,-1(src)
        bdnz+ movup  # typical count is about 0x4cb(1227) bytes

	mtctr r31  # &decompress
        addi dst,t0,GAP  # &unfolded result
	la  ldst,-4(sp)  # &sz_result [will be ignored]
	bctr  # call decompress: branch to counter register, return to link register

main:
	mflr r31  # r31= &decompress
        call unfold
L100: GAP= 128  # > farthest_prefetch; must match ../p_mach.cpp
        b GAP+L100  # 'isync' has trouble on Macintosh G4?
	/* { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...} */

eof:
/*__XTHEENDX__*/

/*
vi:ts=8:et:nowrap
*/

