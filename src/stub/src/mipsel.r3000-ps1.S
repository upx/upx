/*
;  mipsel.r3000-ps1.asm -- ps1/exe program entry & decompressor
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2008 Laszlo Molnar
;  Copyright (C) 2002-2008 Jens Medoch
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
;  Jens Medoch
;  <jssg@users.sourceforge.net>
;
*/

            .set    mips1
            .set    noreorder
            .set    noat
            .altmacro


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

.macro  regs    _w, sz, reg
             \_w     pc,SZ_REG*0(\reg)
             \_w     src,SZ_REG*1(\reg)
             \_w     cnt,SZ_REG*2(\reg)
             \_w     a3,SZ_REG*3(\reg)
             \_w     ra,SZ_REG*4(\reg)
            REG_SZ = (5*SZ_REG)
    .if (\sz == 1)
             \_w     tmp,SZ_REG*5(\reg)
            REG_SZ = (6*SZ_REG)
    .endif
.endm

.macro  push    sz = 0, reg = sp
    .if (PS1)
             regs    sw,\sz,\reg
    .else
             regs    sd,\sz,\reg
    .endif
.endm

.macro  pop     ok = 0, reg = sp
    .if (PS1)
             regs    lw,\ok,\reg
    .else
             regs    ld,\ok,\reg
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

/*
.macro  EnterCriticalSection
            li      a0, 1
            syscall
.endm


.macro  ExitCriticalSection
            li      a0, 2
            syscall
.endm
*/

#define CLzmaDecoderState   a0   /* CLzmaDecoderState */
#define inStream            a1
#define inSize              a2
#define pinSizeprocessed    a3  /*  *inSizeprocessed */

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
            ori     tmp,zero,%lo(ldr_sz+REG_SZ)  // size of decomp. routine
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
            li      tmp,%lo(ldr_sz+REG_SZ)  // size of decomp. routine
            subu    sp,tmp          // adjust the stack with this size
            push    1               // push used regs
            addiu   pc,sp,REG_SZ    // get offset for decomp. routine
            move    dst,pc
            la      src,DCRT        // load decompression routine's offset

section     con.mcpy
            ori     cnt,zero,%lo(ldr_sz)    // amount of removed zero's at eof
1:          lw      var,0(src)      // memcpy
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


/*
=============
============= ENTRY POINT bss
=============
*/
            mCDBOOT 0

section     bss.cdb.start.lzma
            la      t0,PSVR          // prepare to compute value
            subu    t0,s0,t0         // get stored header offset in mem
            la      var,wrkmem-REG_SZ
            jr      t0
            move    tmp,sp

section     bss.cdb.entry.lzma
            push    1,var            // push used regs
            move    sp,var
            addiu   src,t0,lzma_cpr  // compressed lzma decoder offset
            addiu   dst,sp,REG_SZ

section     bss.con.start
            la      var,wrkmem-REG_SZ
            move    tmp,sp
            push    1,var            // push used regs
            move    sp,var
            addiu   pc,sp,REG_SZ     // get offset for decomp. routine
            move    dst,pc
            la      src,DCRT         // load decompression routine's offset

section     bss.exit
            SysFlushCache
            pop     1                // pop used regs with marker for entry
            j       entry
            move    sp,tmp


// =============

section     memset.short
            ori     cnt,zero,%lo(SC) // amount of removed zero's at eof
1:          sw      zero,0(dst)
            subiu   cnt,1
            bnez    cnt,1b
            addiu   dst,4

section     memset.long
            ori     cnt,zero,%lo(SC) // amount of removed zero's at eof
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
section     8bit.sub
            build sub_only, 0, gb8_e

            UCL_init    32,1,0
section     32bit.sub
            build sub_only, 0, gb32_e

section     nrv2b.small
            build without_sub, nrv2b
section     nrv2d.small
            build without_sub, nrv2d
section     nrv2e.small
            build without_sub, nrv2e

section     nrv.done
decomp_done:

section     decompressor.start
decompressor:

section     lzma.init
            li      tmp,%lo(lzma_stack_adjust-lzma_args_sz)
            addu    sp,tmp
/*
            move    var,sp
1:
            sw      zero,0(var)
            addiu   tmp,4
            bnez    tmp,1b
            addiu   var,4
*/
            addiu   src,2
            la      inSize,lzma_c_len
            la      outSize,lzma_u_len

            addu    poutSizeProcessed,sp,outSizeProcessed
            addiu   pinSizeprocessed,sp,inSizeProcessed

            la      tmp,lzma_properties
            sw      dst,dst_save(sp)
            addiu   CLzmaDecoderState,sp,lzma_args_sz
            bal     decompressor
            sw      tmp,0(CLzmaDecoderState)
            lw      dst,dst_save(sp)
            lw      outSize,outSizeProcessed(sp)
            li      tmp,%lo(lzma_stack_adjust-lzma_args_sz)
            addu    dst,outSize
            subu    sp,tmp

#include    "arch/mips/mipsel.r3000/lzma_d.S"

#include    "include/header.S"


// vi:ts=8:et:nowrap
