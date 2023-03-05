/*
;  arm.v4a-expand.S -- decompressors for arm.v4a
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2021 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2021 Laszlo Molnar
;  Copyright (C) 2000-2021 John F. Reiser
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
;  <markus@oberhumer.com>               <ezerotven+github@gmail.com>
;
;  John F. Reiser
;  <jreiser@users.sourceforge.net>
;
*/

NBPW= 4
//    .arch armv6
//    .syntax unified
#include "arch/arm/v4a/macros.S"

    .arm

.macro push reg; str \reg,[sp,#-NBPW]!; .endm
.macro pop  reg; ldr \reg,[sp],#NBPW;   .endm
.macro call subr; bl \subr; .endm

  section EXP_HEAD

sz_unc= 0 *NBPW
sz_cpr= 1 *NBPW
method= 2 *NBPW
sz_binfo= 3 *NBPW

f_unfilter:  @ on stack: (char *ptr, uint len, uint cto, uint fid)  // not callable externally!
        ptr  .req r0
        len  .req r1
        cto  .req r2  @ unused
        fid  .req r3

        t1   .req r2
        t2   .req r3

#ifndef FILTER_ID  /*{*/
#define FILTER_ID 0x50  /* little-endian */
#endif  /*}*/
        and fid,fid,#0xff
        cmp fid,#FILTER_ID  @ last use of fid
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
        ret

    .unreq ptr
    .unreq len
    .unreq cto
    .unreq fid

    .unreq t1
    .unreq t2

get4:  // un-aligned fetch (little endian)
    mov r1,r0
    ldrb r0,[r1],#1
    ldrb r2,[r1],#1; orr r0,r0,r2,lsl #8
    ldrb r2,[r1],#1; orr r0,r0,r2,lsl #16
    ldrb r2,[r1],#1; orr r0,r0,r2,lsl #24
    ret

// int f_expand(nrv_byte const *src, nrv_byte *dst, size_t *dstlen)
// Includes unfilter and cache flush.
// Returns 0 on success.
//  *dstlen returns actual length
//  *src includes 3-word b_info (sz_unc, sz_cpr, {method, filter, cto8, extra})
//  src might not be 4-byte aligned.
// C-callable, so must save+restore r4,r5,r6,r7
// Calling sequence registers
#define xsrc    r0
#define xsrclen r1
#define xdst    r2
#define xdstlen r3

#define arg1 r0
#define arg2 r1
#define arg3 r2
#define arg4 r3
#define arg5 r4

f_expand: .globl f_expand   // start of code for actual de-compressor
    stmdb sp!,{r0,r1,r2,r3,r4,r5,r6,r7,lr}  // all registers

    ldrb arg5,[xsrc,#method]
    mov r5,r0  // copy
    ldr xdstlen,[sp,#2*NBPW]  // arg4
    add r0,r5,#sz_cpr; call get4; mov xsrclen,r0  // arg2
    ldr xdst,[sp,#1*NBPW]  // arg3
    add xsrc,r5,#sz_binfo  // arg1
    call decompress

    ldr arg4,[sp]  // src
    ldrb arg3,[arg4,#2+ method]  // cto8
    ldrb arg4,[arg4,#1+ method]  // ftid
    tst arg4,arg4; beq no_filt
    ldr arg1,[sp,#1*NBPW]  // dst
    ldr arg2,[sp,#2*NBPW]  // &dstlen
    ldr arg2,[arg2]  // dstlen
    call f_unfilter
no_filt:

    ldr arg2,[sp,#2*NBPW]  // &dstlen
    ldr arg1,[sp,#1*NBPW]  // dst
    ldr arg2,[arg2]  // dstlen
    add arg2,arg2,arg1  // &dst[dstlen]
    call __clear_cache

    mov r0,#0; str r0,[sp]  // success return from f_expand  FIXME
    ldmia sp!,{r0,r1,r2,r3,r4,r5,r6,r7,pc}  // end f_expand

decompress:  // sections NRV2B, etc, follow

  section EXP_TAIL
        src .req r0
        dst .req r2
        tmp .req r3
  .type eof,function
  .globl eof
eof:  // end of a compressed extent
        POP {tmp}  // &input_eof
        mov r0,src; SUB2 r0,tmp  // src -= eof;  // return 0: good; else: bad
        POP {tmp}  // original dst
        POP {r1}; SUB2 dst,tmp  // dst -= original dst
        POP {tmp}; str dst,[tmp]  // actual length used at dst  XXX: 4GB
        ret

    .unreq src
    .unreq dst
    .unreq tmp

#define M_NRV2B_LE32    2
#define M_NRV2B_8    3
#define M_NRV2D_LE32    5
#define M_NRV2D_8    6
#define M_NRV2E_LE32    8
#define M_NRV2E_8    9
#define M_CL1B_LE32     11
#define M_LZMA          14

  section NRV2E
#include "arch/arm/v4a/nrv2e_d8.S"

  section NRV2D
#include "arch/arm/v4a/nrv2d_d8.S"

  section NRV2B
#include "arch/arm/v4a/nrv2b_d8.S"

/* lzma has its own 'section's */
#include "arch/arm/v4a/lzma_d.S"

