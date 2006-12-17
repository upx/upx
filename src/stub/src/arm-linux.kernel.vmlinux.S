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
/* Calling sequence of equivalent code in arch/arm/boot/compressed/misc.c:
decompress_kernel:  # (char *out, char *tmp, char *tmp_end, int arch_id)
        lr= &indata; ip= retaddr  # from arm-linux.kernel.vmlinux-head.S
*/
        mov r1,r2  // tmp_end
        mov r2,r0  // &outdata
        mov r0,lr  // &indata
        str ip,[r1,#-4]!  // push retaddr on new stack

// Value stored from r1 to memory will be overwritten by outsize.
        stmdb r1!,{r0,r1,r2,r3,sp}  // &indata, space, &outdata, arch_id, sp_in
        ldr r3,2*4+.L10  // method
        mov sp,r1  // switch stacks to tmp_end area (64KB)
        ldr r1,0*4+.L10  // insize
        str r3,[sp,#-4]!  // method
        ldr r3,1*4+.L10  // outsize
        str r3,[sp,#2*4]  // outsize
        add r3,sp, #2*4  // &outsize
        bl .L20  // (&indata, insize, &outdata, &outsize, method)
spin:
        cmp r0,#0  // check for success
        bne spin

        ldmia sp,{r0,r1,r2,r3,ip,sp,lr}  // method, &indata, outsize, &outdata, arch_id, sp_in, retaddr
        add r0,r2,r3  // rv= &outdata[outsize]
        mov pc,lr  // return
.L10:
        .long   COMPRESSED_LENGTH
        .long UNCOMPRESSED_LENGTH
        .long METHOD
.L20:
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

#if 0  /*{ remind me about parameters for unfilter? */
section         LXCKLLT9
                pop     ecx     // MATCH05  len
                pop     edx     // MATCH04  cto
                pop     edi     // MATCH03  src

                ckt32   edi, dl // dl has cto8
/*
        ;edi: adjust for the difference between 0 origin of buffer at filter,
        ;and actual origin of destination at unfilter.
        ;Filter.addvalue is 0: destination origin is unknown at filter time.
        ;The input data is still relocatable, and address is assigned later
        ;[as of 2004-12-15 it is 'always' 0x100000].
*/

section         LXCALLT9
                pop     ecx     // MATCH05  len
                pop     edi     // MATCH03  src
                cjt32   0
section         LINUX990
                pop     esi     // MATCH02  restore
                pop     edi     // MATCH01  restore
                xor     ebx, ebx        // booting the 1st cpu
                lret    // MATCH00  set cs
#endif  /*}*/

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
