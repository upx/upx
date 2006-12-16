/*
;  arm-linux.kernel.vmlinux-head.S -- set up stack for vmlinux/arm format
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 2006 John Reiser
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
;  John Reiser
;  <jreiser@users.sourceforge.net>
*/

/* Calling sequence of equivalent code in arch/arm/boot/compressed/misc.c:
decompress_kernel:  # (char *out, char *tmp, char *end_tmp, int arch_id)
*/
        bkpt
        stmdb r2!,{r0,sp,lr}
        mov sp,r2
// PackVmlinuxARM::pack knows the format of the next instruction.
        bl COMPRESSED_LENGTH
        ldmia sp,{r1,sp,pc}

// Compressed data appears >here<, then decompressor.

// vi:ts=8:et:nowrap
