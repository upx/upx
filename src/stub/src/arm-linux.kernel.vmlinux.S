/*
;  arm-linux.kernel.vmlinux.S -- loader & decompressor for the vmlinux/arm format
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2008 Laszlo Molnar
;  Copyright (C) 2004-2008 John Reiser
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
;  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
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
        loadcon8 3,METHOD  // mov r3,#METHOD
        mov sp,r1  // switch stacks to tmp_end area (64 KiB)
        ldr r1,1f  // insize
        str r3,[sp,#-4]!  // method
        ldr r3,2f  // outsize
        str r3,[sp,#2*4]  // outsize
        add r3,sp, #2*4  // &outsize
        bl decompressor  // (&indata, insize, &outdata, &outsize, method)
        b 3f
1:
        .long   COMPRESSED_LENGTH
2:
        .long UNCOMPRESSED_LENGTH
3:
spin:
        cmp r0,#0  // check for success
        bne spin

section         LINUX010
        ldr r0,[sp,#3*4]  // &outdata
        ldr r1,[sp,#2*4]  // outsize
        loadcon8 2,filter_cto   // mov r2,#filter_cto
        loadcon8 3,filter_id    // mov r3,#filter_id
        bl unfilter  // unfilter(&outdata, outsize, cto, fid)

section         LINUX020
        ldmia sp,{r0,r1,r2,r3,ip,sp,lr}  // method, &indata, outsize, &outdata, arch_id, sp_in, retaddr
        add r0,r2,r3  // rv= &outdata[outsize]
        mov pc,lr  // return

// =============
// ============= UNFILTER
// =============
section         ctok32.00
//f_unfilter:  @ (char *ptr, uint len, uint cto, uint fid)
        ptr  .req r0
        len  .req r1
        cto  .req r2  @ unused
        fid  .req r3

        t1   .req r2
        t2   .req r3
unfilter:
        and fid,fid,#0xff
section         ctok32.50
        cmp fid,#0x50  @ last use of fid
section         ctok32.51
        cmp fid,#0x51  @ last use of fid

section         ctok32.10
        movne pc,lr  @ no-op if not filter 0x50

        movs  len,len,lsr #2  @ word count
        cmpne ptr,#0
        moveq pc,lr  @ no-op if either len or ptr is 0

top_unf:
        sub len,len,#1
        ldr t1,[ptr,len,lsl #2]
        and t2,t1,#0x0f<<24
        cmp t2,   #0x0b<<24; bne tst_unf  @ not 'bl' subroutine call
        and t2,t1,#0xff<<24  @ all the non-displacement bits
        sub t1,t1,len  @ convert to word-relative displacement
        bic t1,t1,#0xff<<24  @ restrict to displacement field
        orr t1,t1,t2  @ re-combine
        str t1,[ptr,len,lsl #2]
tst_unf:
        cmp len,#0
        bne top_unf
        mov pc,lr

        .unreq ptr
        .unreq len
        .unreq cto
        .unreq fid

section         LINUX030
decompressor:
/*
  r0= inptr
  r1= insize
  r2= outptr
  r3= &outsize
  sp/ method
*/

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

#include "include/header.S"

// vi:ts=8:et:nowrap
