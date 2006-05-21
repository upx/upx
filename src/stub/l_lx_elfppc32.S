/*  l_lx_elfppc.S -- Linux program entry point & decompressor (Elf binary)
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

#include "ppc_regs.h"

/*__MACOS000__*/
_start: .globl _start
        call main  # must be exactly 1 instruction; link_register= &decompress
#include "ppc_d_nrv2e.S"

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

__NR_mmap= 90

PAGE_SHIFT= 12
PAGE_SIZE = -(~0<<PAGE_SHIFT)

/* Temporary until we get the buildLoader stuff working ... */
        .ascii "\n$Id: UPX (C) 1996-2006 the UPX Team. "
        .asciz "All Rights Reserved. http://upx.sf.net $\n"
        .p2align 2  # (1<<2)

/* /usr/include/asm-ppc/unistd.h */
__NR_write = 4
__NR_exit  = 1

msg_SELinux:
        call L71
L70:
        .asciz "PROT_EXEC|PROT_WRITE failed.\n"
        .p2align 2  # (1<<2)
L71:
        li   a2,L71 - L70  # length
        mflr a1    # message text
        li   a0,2  # fd stderr
        li 0,__NR_write; sc
die:
        li a0,127
        li 0,__NR_exit; sc

/* Decompress the rest of this loader, and jump to it. */
unfold:
        mflr r30  # &{ b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...}

        li a5,0  # off_t
        li a4,-1  # fd; cater to *BSD for MAP_ANON
          lwz  a0,sz_cpr(r30)
        li a3,MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS
        li a2,PROT_READ | PROT_WRITE | PROT_EXEC
        li a1,PAGE_SIZE
          add  a0,a0,r30
        li 0,__NR_mmap
          addi a0,a0,sz_b_info+PAGE_SIZE-1
        rlwinm a0,a0,0,0,31-PAGE_SHIFT  # next page boundary after fold
        sc; bso- msg_SELinux  # Branch if SummaryOverflow (failure)
0:
        mtctr r31
        lbz meth,b_method(r30)
        la ldst,31*4(sp)  # &do_not_care
        mr dst,a0
        mtlr   a0  # &continuation
        lwz lsrc,sz_cpr(r30)
        addi src,r30,sz_b_info
        bctr  # goto decomrpess; return to link register (mmap'ed page)

main:
        stwu r1,-32*4(sp)  # allocate space (keeping 0 mod 16), save r1
        stmw r2,4(sp) # save registers r2 thru r31
        mflr r31  # &decompress
        call unfold
        /* { b_info={sz_unc, sz_cpr, {4 char}}, folded_loader...} */

eof:
/*__XTHEENDX__*/

/*
vi:ts=8:et:nowrap
*/

