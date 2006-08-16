/*
;  mipsel.r3000-ps1.S -- ps1/exe program entry & decompressor
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2006 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2006 Laszlo Molnar
;  Copyright (C) 2002-2006 Jens Medoch
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
;  Jens Medoch
;  <jssg@users.sourceforge.net>
;
*/

            .altmacro
            .set    mips1
            .set    noreorder
            .set    noat


#include "arch/mips/mipsel.r3000/macros.ash"
#include "arch/mips/mipsel.r3000/bits.ash"

/*
=============
============= none
=============
*/

.if (PS1)
    SZ_REG = 4
.else
    SZ_REG = 8
.endif

.macro mCDBOOT s
    .if (\s == 1)
        REG_SZ = (5*SZ_REG)
        CDBOOT = 1
    .else
        REG_SZ = (6*SZ_REG)
        CDBOOT = 0
    .endif
.endm

.macro  regs    _w, ok
             \_w     pc,SZ_REG*0(sp)
             \_w     src,SZ_REG*1(sp)
             \_w     cnt,SZ_REG*2(sp)
             \_w     a3,SZ_REG*3(sp)
             \_w     ra,SZ_REG*4(sp)
            REG_SZ = (5*SZ_REG)
    .if (\ok == 1)
             \_w     tmp,SZ_REG*5(sp)
            REG_SZ = (6*SZ_REG)
    .endif
.endm

.macro  push    ok = 0
    .if (PS1)
             regs    sw,\ok
    .else
             regs    sd,\ok
    .endif
.endm

.macro  pop     ok = 0
    .if (PS1)
             regs    lw,\ok
    .else
             regs    ld,\ok
    .endif
.endm

.macro  SysFlushCache
    .if (PS1)
            PRINT ("SYSCALL PS1")
            li      t2,160
            jalr    ra,t2
            li      t1,68
    .else
            PRINT ("SYSCALL PS2")
            move    a0, zero
            li      v1, 100
            syscall
    .endif
.endm


#define CLzmaDecoderState   a0   /* CLzmaDecoderState */
#define inStream            a1
#define inSize              a2
#define pinSizeprocessed    a3  /*   inSizeprocessed */

#define outStream           t0
#define outSize             t1
#define poutSizeProcessed   t2

#define dst_save            0*SZ_REG
#define outSizeProcessed    1*SZ_REG
#define inSizeProcessed     2*SZ_REG

            lzma_args_sz =  4*SZ_REG

/*
=============
============= ENTRY POINT cd-boot
=============
*/
            mCDBOOT 1

section     cdb.start
            la      t0,PSVR         // prepare to compute value
            subu    t0,s0,t0        // get stored header offset in mem
            jr      t0
            subiu   sp,REG_SZ       // prep to adjust stack

section     cdb.entry
            push                    // push used regs
            la      src,CPDO        // load compressed data offset

section     cdb.start.lzma
            la      t0,PSVR         // prepare to compute value
            subu    t0,s0,t0        // get stored header offset in mem
            li      tmp,%lo(LS+REG_SZ)      // size of decomp. routine
            jr      t0
            subu    sp,tmp          // adjust the stack with this size

section     cdb.entry.lzma
            push    1               // push used regs
            addiu   src,t0,lzma_cpr // load compressed data offset
            addiu   dst,sp,REG_SZ

section     cdb.lzma.cpr
            la      src,CPDO        // load compressed data offset

section     cdb.dec.ptr
            la      dst,DECO        // load decompress data offset

section     cdb.dec.ptr.hi
            lui     dst,%hi(DECO)

section     cdb.exit
            SysFlushCache
            pop                     // pop used regs with marker for entry
            j       entry
            addiu   sp,REG_SZ


/*
=============
============= ENTRY POINT console
=============
*/

            mCDBOOT 0

section     con.start
            li      tmp,%lo(LS+REG_SZ)      // size of decomp. routine
            subu    sp,tmp                  // adjust the stack with this size
            push    1                // push used regs
            subiu   cnt,tmp,REG_SZ   // cnt = counter copyloop
            addiu   pc,sp,REG_SZ     // get offset for decomp. routine
            move    dst,pc
            la      src,DCRT         // load decompression routine's offset

section     con.mcpy
1:          lw      var,0(src)       // memcpy
            subiu   cnt,4
            sw      var,0(dst)
            addiu   src,4
            bnez    cnt,1b
            addiu   dst,4

section     lzma.prep
            addiu   pc,dst,%lo(lzma_init_off)

section     con.padcd
            addiu   src,%lo(PAD)     // pointer compressed data

section     dec.ptr
            lui     dst,%hi(DECO)    // load decompress data offset
            jr      pc
            addiu   dst,%lo(DECO)

section     dec.ptr.hi
            jr      pc
            lui     dst,%hi(DECO)

section     con.entry

section     con.exit
            SysFlushCache
            pop     1                // pop used regs with marker for entry
            j       entry
            addu    sp,tmp


// =============

section     memset.short
            li      cnt,%lo(SC)      // amount of removed zero's at eof
1:          sw      zero,0(dst)
            subiu   cnt,1
            bnez    cnt,1b
            addiu   dst,4

section     memset.long
            li      cnt,%lo(SC)      // amount of removed zero's at eof
            sll     cnt,3            // (cd mode 2 data sector alignment)
1:          sw      zero,0(dst)
            subiu   cnt,1
            bnez    cnt,1b
            addiu   dst,4


/*
=============
============= DECOMPRESSION
=============
*/

#include "arch/mips/mipsel.r3000/nrv2b_d.ash"
#include "arch/mips/mipsel.r3000/nrv2d_d.ash"
#include "arch/mips/mipsel.r3000/nrv2e_d.ash"


// ========== cd-boot

            UCL_init    8,0,1
section     nrv2b.8bit
            build full, nrv2b
section     nrv2d.8bit
            build full, nrv2d
section     nrv2e.8bit
            build full, nrv2e

            UCL_init    32,0,1
section     nrv2b.32bit
            build full, nrv2b
section     nrv2d.32bit
            build full, nrv2d
section     nrv2e.32bit
            build full, nrv2e

// ========== console-run

            UCL_init    8,1,0
section     gb.8bit.sub
            build sub_only

            UCL_init    32,1,0
section     gb.32bit.sub
            build sub_only

section     nrv2b.small
            build without_sub, nrv2b
section     nrv2d.small
            build without_sub, nrv2d
section     nrv2e.small
            build without_sub, nrv2e

section     nrv.small.done
decomp_done:

#include    "arch/mips/mipsel.r3000/lzma_d.S"

#include    "include/header.S"


// vi:ts=8:et:nowrap
