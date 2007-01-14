/*
;  bits.ash -- bit access for decompression
;
;  This file is part of the UCL data compression library.
;
;  Copyright (C) 1996-2007 Markus Franz Xaver Johannes Oberhumer
;  All Rights Reserved.
;
;  The UCL library is free software; you can redistribute it and/or
;  modify it under the terms of the GNU General Public License as
;  published by the Free Software Foundation; either version 2 of
;  the License, or (at your option) any later version.
;
;  The UCL library is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with the UCL library; see the file COPYING.
;  If not, write to the Free Software Foundation, Inc.,
;  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
;
;  Markus F.X.J. Oberhumer
;  <mfx@users.sourceforge.net>
;  http://www.oberhumer.com/opensource/ucl/
;
;  Jens Medoch
;  <jssg@users.sourceforge.net>
;
 */


#ifndef _MR3K_STD_CONF_
#define _MR3K_STD_CONF_


;//////////////////////////////////////
;// register defines
;//////////////////////////////////////

#define tmp         at

#define dst         t0
#define src         a1

#define pc          a0
#define cnt         a2

#define src_ilen    src
#define bb          t1
#define ilen        t2
#define last_m_off  t3
#define m_len       t4
#define bc          t5

#define var         t6
#define m_off       t7
#define m_pos       t7


;//////////////////////////////////////
;// init bitaccess
;//////////////////////////////////////

.macro  UCL_init    bsz,opt,fmcpy

            UCL_NRV_BB = \bsz
            UCL_SMALL = \opt
            UCL_FAST = \fmcpy

            .if ((\bsz != 32) && (\bsz != 8))
                .error "UCL_NRV_BB must be 8 or 32 and not \bsz")
            .else
                PRINT ("\bsz bit, small = \opt, fast memcpy = \fmcpy")
            .endif
            .if (PS1)
                 PRINT ("R3000 code")
            .else
                 PRINT ("R5900 code")
            .endif

.endm


;//////////////////////////////////////
;// init decompressor
;//////////////////////////////////////

.macro  init

            move    bc,zero
            li      last_m_off,1
    .if (src != src_ilen)
            move    src_ilen,src
    .endif

.endm


;//////////////////////////////////////
;// getbit macro
;//////////////////////////////////////

.macro  ADDBITS

    .if (UCL_SMALL == 1)
            addiu   bc, -1
            bltz    bc, 2b
    .else
            bgtz    bc, 2f
            addiu   bc, -1
    .endif

.endm

.macro  ADDBITS_DONE

            srlv    var,bb,bc
    .if (UCL_SMALL == 1)
            jr      ra
    .endif
            andi    var,0x0001

.endm

.macro  FILLBYTES_8

    .if (UCL_SMALL == 1)
            li      bc,8
    .else
            li      bc,7
    .endif
            lbu     bb,0(src_ilen)
            addiu   src_ilen,1

.endm

.macro  FILLBYTES_32

    .if (UCL_SMALL == 1)
            li      bc,32
    .else
            li      bc,31
    .endif
            lwr     bb,0(src_ilen)
            lwl     bb,3(src_ilen)
            addiu   src_ilen,4

.endm

.macro  FILLBYTES

    .if (UCL_NRV_BB == 8)
            FILLBYTES_8
    .else // (UCL_NRV_BB == 32)
            FILLBYTES_32
    .endif

.endm

// This alternate for UCL_SMALL is 1 cycle faster for most getbit
// at a cost of 1 cycle (32 bits) or 3 cycles (8 bits) during refill.
// [Refill could save 1 cycle if MIPS had "set CarryIn" for ADD or SHIFT.]
// Call 'getbit'; it returns the next bit (0 or 1), in register 'var'.
// Register 'bb' is the bit buffer; register 'bc' contains the 'empty' value.
// The cases for widths 8 and 32 are not as similar as before,
// and 64-bit registers would require other code.
// But it is 1 cycle faster 31/32 of the time.
// [Indentation after control transfer emphasizes delay slot.]
//
//init:
//	lui bc,1<<(31 - 16)  # 1<<31  the flag bit
//	lui bb,1<<(31 - 16)  # 1<<31  empty
//--
//  .if (8==UCL_NRV_BB)
//refill:
//	lbu bb,0(src_ilen)
//	addiu src_len,1
//	sll bb,1
//	ori bb,1  # the flag bit
//	sll bb,24-1  # left-justify in register
//	// falling through the 'beq' below saves 3 words of space
//  .endif
//getbit:
//	beq bc,bb,refill  # detect flag bit [empty]
//	  srl var,bb,31  # var= most significant bit of bb
//	jr ra
//	  sll bb,1
//  .if (32==UCL_NRV_BB)
//refill:
//	lwr bb,0(src_ilen)
//	lwl bb,3(src_ilen)
//	addiu src_ilen,4
//	srl var,bb,31  # var= most significant bit of bb
//	sll bb,1
//	jr ra
//	  ori bb,1  # the flag bit
//  .endif

// 2006-09-06  Faster by 3 cycles for inline expansion:
//	beq bc,bb,7f  # detect flag bit [empty]
//	  srl var,bb,31  # var= most significant bit of bb
//	sll bb,1
//6:
//	.subsection 1  # somewhere out-of-line
//7:
//	b refill
//	  addi ra,r_bd_base,6b - bd_base  # return past the 'sll' above
//	.subsection 0  # return to main in-line code
//
// which is 3 cycles usually, +2 cycles for entering refill,
// +8 bytes (2 instructions) per getbit [7 in nrv2b, 9 in nrv2e]
// and requires another register r_bd_base which holds a handy
// address within +/- 32KB of the returns from refill.
//

.macro  GBIT

    .if (UCL_SMALL == 1)
2:
            FILLBYTES
1:
            ADDBITS
            ADDBITS_DONE
    .else
            ADDBITS
            FILLBYTES
2:
            ADDBITS_DONE
    .endif

.endm

;//////////////////////////////////////
;// getbit call macro for SMALL version
;//////////////////////////////////////

.macro      GETBIT  p1

    .if (UCL_SMALL == 1)
        .if (WITHOUT_SUB == 1)
            t = gb_e
        .else
            t = 1f
        .endif
        .ifb   p1
            bal     t      // gb_sub
        .else
            bal     t+4    // gb_sub+4
            addiu   bc,-1
        .endif
    .else
            GBIT
    .endif

.endm


;//////////////////////////////////////
;// getbit call macro for SMALL version
;//////////////////////////////////////

.macro  build   option, type, label

            local   done

.ifc "\option", "full"
    .ifnb label
\label:
    .endif
            \type   decomp_done
    .if (UCL_SMALL == 1)
            WITHOUT_SUB = 0
            GBIT
    .endif
.else
.ifc "\option", "sub_only"

2:
            FILLBYTES
    .ifnb label
            .global \label
\label:
    .endif
            ADDBITS
            ADDBITS_DONE
.else
.ifc "\option", "without_sub"
    .if (UCL_SMALL == 1)
        PRINT ("[WARNING] building \type with UCL_SMALL = 1 without subroutine")
            WITHOUT_SUB = 1
            \type   decomp_done
            WITHOUT_SUB = 0
    .else
        .error "cannot build \"without_sub\" if UCL_SMALL = 0"
    .endif
.else
    .error "use \"full\", \"sub\" or \"without_sub\" for build"
.endif
.endif
.endif
.endm


;//////////////////////////////////////
;// ucl memcpy
;//////////////////////////////////////

.macro   uclmcpy     ret

            local   wordchk, bcpy
            local   bcpy_chk, skip

    .if (UCL_FAST == 1)
            slti    var,m_off,4
            bnez    var,bcpy
            subu    m_pos,dst,m_off
wordchk:
            slti    var,m_len,4
            bnez    var,skip
            lwr     var,0(m_pos)
            lwl     var,3(m_pos)
            addiu   m_len,-4
            swr     var,0(dst)
            swl     var,3(dst)
            addiu   m_pos,4
            bnez    m_len,wordchk
            addiu   dst,4
            b       \ret
            nop
    .else
            subu    m_pos,dst,m_off
    .endif
bcpy_chk:
            beqz    m_len,\ret
bcpy:
            lbu     var,0(m_pos)
skip:
            addiu   m_len,-1
            sb      var,0(dst)
            addiu   m_pos,1
            b       bcpy_chk
            addiu   dst,1

.endm

#endif  //_MR3K_STD_CONF_
