/*
;  arm-linux.kernel.vmlinux.S -- loader & decompressor for the vmlinux/arm format
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2006 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2006 Laszlo Molnar
;  Copyright (C) 2004-2006 John Reiser
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
;  Markus F.X.J. Oberhumer              Laszlo Molnar
;  <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>
;
;  John Reiser
;  <jreiser@users.sourceforge.net>
*/

#include "arch/arm/v5a/macros.S"

/*
; =============
; ============= ENTRY POINT
; =============

  How to debug: run under qemu (http://fabrice.bellard.free.fr/qemu/)
  after un-commenting the  bkpt  opcode below.  That opcode forces qemu
  to stop in gdb.  You'll have to "set $pc+=4" by hand.
*/
section         LINUX000
        // bkpt  // qemu DEBUG only
/*
  r0= inptr
  r1= insize
  r2= outptr
  r3= &outsize
  sp/ method
*/

section         LXCALLT1
        mvn ip,#1
        bkpt
section         LXCKLLT1
        mvn ip,#2
        bkpt
        .long filter_cto,filter_length
section         LXMOVEUP
        mvn ip,#3
        bkpt

// =============
// ============= DECOMPRESSION
// =============

section NRV2B
#include "arch/arm/v5a/nrv2b_d8.S"

section NRV2D
#include "arch/arm/v5a/nrv2d_d8.S"

section NRV2E
#include "arch/arm/v5a/nrv2e_d8.S"

#include "arch/arm/v5a/lzma_d.S"

// =============
// ============= UNFILTER
// =============

section         LXCKLLT9
        mvn ip,#4
        bkpt

section         LXCALLT9
        mvn ip,#5
        bkpt

section         LINUX990
        mvn ip,#6
        bkpt
        mov pc,lr

section         LINUX991
        mvn ip,#7
        bkpt
#if 0  /*{*/
                or  ebp, -1        // decompressor assumption
#endif  /*}*/

section         LINUX992
        mvn ip,#8
        bkpt

#include "include/header.S"

// vi:ts=8:et:nowrap
