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

#include "arch/mips/mipsel.r3000/macros.ash"

#define  SZ_REG  4

#if CDBOOT
.macro          regs    _w, marker
                \_w     a0,SZ_REG*0(sp)
                \_w     a1,SZ_REG*1(sp)
                \_w     a2,SZ_REG*2(sp)
                \_w     a3,SZ_REG*3(sp)
                \_w     v0,SZ_REG*4(sp)
                \_w     v1,SZ_REG*5(sp)
IF (\marker != 0)
                DW      \marker
ENDIF
                \_w     ra,SZ_REG*6(sp)
.endm

#define  REG_SZ (7*SZ_REG)

#else //console

.macro          regs    _w, marker
                \_w     at,SZ_REG*0(sp)
                \_w     a0,SZ_REG*1(sp)
                \_w     a1,SZ_REG*2(sp)
                \_w     a2,SZ_REG*3(sp)
                \_w     a3,SZ_REG*4(sp)
                \_w     v0,SZ_REG*5(sp)
                \_w     v1,SZ_REG*6(sp)
                \_w     ra,SZ_REG*7(sp)
IF (\marker != 0)
                DW      \marker
                addu    sp,at
ENDIF
.endm

#define  REG_SZ (8*SZ_REG)

#endif //CDBOOT

#if CDBOOT
// =============
// ============= ENTRY POINT
// =============
// for cd-boot only

section         PS1START
                la      t0, PSVR        // prepare to compute value
                subu    t0,s0,t0        // get stored header offset in mem
                jr      t0
                subiu   sp, REG_SZ      // adjust stack
section         PS1ENTRY
                regs    sw,0            // push used regs
                la      a0, CPDO        // load COMPDATA offset
//               li      a1,'CDSZ'      // compressed data size - disabled!
section         PS1CONHL
                la      a2, DECO
section         PS1CONHI
                lui     a2, %hi(DECO)


#else //CONSOLE
// =============
// ============= ENTRY POINT
// =============
// for console- / cd-boot

section         PS1START
                addiu   at,zero, LS     // size of decomp. routine
                subu    sp,at           // adjust the stack with this size
                regs    sw,0            // push used regs
                subiu   a2,at,REG_SZ    // a2 = counter copyloop
                addiu   a3,sp,REG_SZ    // get offset for decomp. routine
                move    a1,a3
                la      a0, DCRT        // load decompression routine's offset
copyloop:
                lw      at,0(a0)        // memcpy *a0 -> at -> *a1
                addiu   a2,-4
                sw      at,0(a1)
                addiu   a0,4
                bnez    a2,copyloop
                addiu   a1,4

section         PS1PADCD
                addiu   a0, PC          // a0 = pointer compressed data
section         PS1CONHL
                la      a2, DECO        // load DECOMPDATA HI offset
                jr      a3
section         PS1CONHI
                jr      a3
                lui     a2, %hi(DECO)   // same for HI only !(offset&0xffff)
section         PS1ENTRY

#endif //CDBOOT
// =============
// ============= DECOMPRESSION
// =============

#ifndef FAST
#   define FAST
#endif

#if CDBOOT
#   ifdef SMALL
#       undef   SMALL
#   endif
#else //CONSOLE
#   ifndef SMALL
#       define  SMALL
#   endif
#endif //CDBOOT

#ifdef NRV_BB
#   undef NRV_BB
#endif
#define NRV_BB 8

section         PS1N2B08
//#include "arch/mips/mipsel.r3000/nrv2b_d.ash"
section         PS1N2D08
//#include "arch/mips/mipsel.r3000/nrv2d_d.ash"
section         PS1N2E08
#include "arch/mips/mipsel.r3000/nrv2e_d.ash"

#ifdef NRV_BB
#   undef NRV_BB
#endif
#define NRV_BB 32

section         PS1N2B32
//#include "arch/mips/mipsel.r3000/nrv2b_d.ash"
section         PS1N2D32
//#include "arch/mips/mipsel.r3000/nrv2d_d.ash"
section         PS1N2E32
#include "arch/mips/mipsel.r3000/nrv2e_d.ash"


// =============

section         PS1MSETS
                ori     a0,zero, SC     // amount of removed zeros at eof
section         PS1MSETB
                ori     a0,zero, SC     // amount of removed zeros at eof
                sll     a0,3            // (cd mode 2 data sector alignment)
section         PS1MSETA
memset_aligned:
                sw      zero,0(a2)
                addiu   a0,-1
                bnez    a0,memset_aligned
                addiu   a2,4
section         PS1MSETU
memset_unaligned:
                swl     zero,3(a2)
                swr     zero,0(a2)
                addiu   a0,-1
                bnez    a0,memset_unaligned
                addiu   a2,4

// =============

section         PS1EXITC
                li      t2,160          // flushes
                jalr    ra,t2           // instruction
                li      t1,68           // cache
                regs    lw, JPEP        // marker for the entry jump

// =============

#include        "include/header2.ash"

// =============

#if !CDBOOT
section         PS1SREGS
                DW      REG_SZ
#endif //CDBOOT

// vi:ts=8:et:nowrap
