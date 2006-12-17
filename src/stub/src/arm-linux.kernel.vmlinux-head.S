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
        stmdb r2!,{r0,r3,sp,lr}  // &outdata, arch_id, sp_in, retaddr
        mov sp,r2  // switch stacks to end_tmp area (64KB)
        mov r2,r0  // &outdata
        adr r0,.L10
        ldmia r0!,{r1,r3,ip}  // r0=&indata; r1=insize; r3=outsize; ip=method
        str r3,[sp,#-4]!  // outsize
        mov r3,sp  // &outsize
        str ip,[sp,#-4]!  // method
// PackVmlinuxARM::pack knows the layout of the following words.
        bl LINUX000  // (&indata, insize, &outdata, &outsize, method)
spin:
        cmp r0,#0  // check for success
        bne spin

        ldmia sp,{r1,r2,r3,ip,sp,lr}  // method, outsize, &outdata, arch_id, sp_in, retaddr
        add r0,r2,r3  // rv= &outdata[outsize]
        mov pc,lr  // return
.L10:
        .long   COMPRESSED_LENGTH
        .long UNCOMPRESSED_LENGTH
        .long METHOD

// Compressed data appears >here<, then decompressor.

// vi:ts=8:et:nowrap
