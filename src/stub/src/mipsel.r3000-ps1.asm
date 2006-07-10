/*
;  l_ps1.asm -- ps1/exe program entry & decompressor
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

            .set    mips1
            .altmacro
            .set    noreorder
            .set    noat


#include "arch/mips/mipsel.r3000/macros.ash"
#include "arch/mips/mipsel.r3000/bits.ash"

/*
=============
============= none
=============
*/

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

.macro  regs    _w, marker

.if (PS1)
    SZ_REG = 4
.else
    SZ_REG = 8
.endif

.if (CDBOOT == 1)
    REG_SZ = (4*SZ_REG)
.else
    REG_SZ = (5*SZ_REG)
.endif

             \_w     src,SZ_REG*0(sp)
             \_w     dst,SZ_REG*1(sp)
             \_w     pc,SZ_REG*2(sp)
             \_w     ra,SZ_REG*3(sp)
    .if (CDBOOT == 0)
             \_w     tmp,SZ_REG*4(sp)
    .endif
    .if (\marker != 0)
             DW      \marker
        .if (CDBOOT == 0)
             addu    sp,tmp
        .else
             addiu   sp,REG_SZ
        .endif
    .endif

.endm

.macro  push    jmp

    .if (PS1)
             regs    sw,\jmp
    .else
             regs    sd,\jmp
    .endif

.endm

.macro  pop     jmp

    .if (PS1)
             regs    lw,\jmp
    .else
             regs    ld,\jmp
    .endif

.endm


/*
=============
============= ENTRY POINT cd-boot
=============
*/
            CDBOOT = 1

section     ps1.cdb.start
            la      t0,PSVR         // prepare to compute value
            subu    t0,s0,t0        // get stored header offset in mem
            jr      t0
            subiu   sp,REG_SZ       // prep to adjust stack

section     ps1.cdb.entry
            push    0               // push used regs
            la      src,CPDO        // load compressed data offset

section     ps1.cdb.nrv.ptr
            la      dst,DECO        // load decompressed data offset

section     ps1.cdb.nrv.ptr.hi
            lui     dst,%hi(DECO)

section     ps1.cdb.exit
            SysFlushCache
            pop     JPEP             // pop used regs with marker for entry


/*
=============
============= ENTRY POINT console
=============
*/

            CDBOOT = 0

section     ps1.con.start
            li      tmp,%lo(LS)      // size of decomp. routine
            subu    sp,tmp           // adjust the stack with this size
            push    0                // push used regs
            subiu   cnt,tmp,REG_SZ   // cnt = counter copyloop
            addiu   pc,sp,REG_SZ     // get offset for decomp. routine
            move    dst,pc
            la      src,DCRT         // load decompression routine's offset
1:
            lw      tmp,0(src)       // memcpy *a0 -> tmp -> *a1
            subiu   cnt,4
            sw      tmp,0(dst)
            addiu   src,4
            bnez    cnt,1b
            addiu   dst,4

section     ps1.con.padcd
            addiu   src,PC           // a0 = pointer compressed data

section     ps1.con.nrv.ptr
            lui     dst,%hi(DECO)    // load DECOMPDATA HI offset
            jr      pc
            addiu   dst,%lo(DECO)    // load DECOMPDATA LO offset

section     ps1.con.nrv.ptr.hi
            jr      pc
            lui     dst,%hi(DECO)    // same for HI only !(offset&0xffff)

section     ps1.con.entry

section     ps1.con.exit
            SysFlushCache
            pop     JPEP             // pop used regs with marker for entry

section     ps1.con.regs
            DW      REG_SZ


// =============

section     ps1.mset.short
            li      cnt,%lo(SC)          // amount of removed zero's at eof

section     ps1.mset.long
            li      cnt,%lo(SC)          // amount of removed zero's at eof
            sll     cnt,3           // (cd mode 2 data sector alignment)

section     ps1.mset.aligned
1:
            sw      zero,0(dst)
            subiu   cnt,1
            bnez    cnt,1b
            addiu   dst,4

section     ps1.mset.unaligned
1:
            swl     zero,3(dst)
            swr     zero,0(dst)
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
section     ps1.cdb.nrv2b.8bit
            build full, nrv2b
section     ps1.cdb.nrv2d.8bit
            build full, nrv2d
section     ps1.cdb.nrv2e.8bit
            build full, nrv2e

            UCL_init    32,0,1
section     ps1.cdb.nrv2b.32bit
            build full, nrv2b
section     ps1.cdb.nrv2d.32bit
            build full, nrv2d
section     ps1.cdb.nrv2e.32bit
            build full, nrv2e

// ========== console-run

            UCL_init    8,1,0
section     ps1.getbit.8bit.sub
            build sub_only
section     ps1.getbit.8bit.size
            DW  sub_size

            UCL_init    32,1,0
section     ps1.getbit.32bit.sub
            build sub_only
section     ps1.getbit.32bit.size
            DW  sub_size

section     ps1.small.nrv2b
            build without_sub, nrv2b
section     ps1.small.nrv2d
            build without_sub, nrv2d
section     ps1.small.nrv2e
            build without_sub, nrv2e


#include    "include/header2.ash"


// vi:ts=8:et:nowrap
