/*
;  bits.ash -- bit access for decompression
;
;  This file is part of the UCL data compression library.
;
;  Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
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
;  <markus@oberhumer.com>
;  http://www.oberhumer.com/opensource/ucl/
;
;  Jens Medoch
;  <jssg@users.sourceforge.net>
;
 */


#ifndef _MR3K_STD_CONF_
#define _MR3K_STD_CONF_

#define  JOHN       1
#define  ALT_SMALL  1


;//////////////////////////////////////
;// register defines
;//////////////////////////////////////

#if defined(PS1)  /*{*/
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

#elif defined(_TARGET_LINUX_)  /*}{*/
#define tmp         at

/* decompress(src, src_len, dst, &dst_len, method); */
#define src         a0
#define dst         a2

#define src_ilen    src
#define bb          t1
#define ilen        t2
#define last_m_off  t3
#define m_len       t4
#define bc          t5

#define var         t6
#define m_off       t7
#define m_pos       t7

#else  /*}{*/
.print "\nwarning redefined src / dst\n"

#define tmp         v1

#define src         a0
#define dst         a1

#define pc          v0
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

#endif  /*}*/


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

.macro  init gb_subr

init_sz = .

.if (JOHN == 0)
            move    bc,zero

.else   //  John's method

    .if (UCL_SMALL == 1)
        .if (ALT_SMALL == 1)
            lui     bc,1 << (31 - 16)
            move    bb,bc
        .else
            move    bc,zero
        .endif
    .else
            lui     bc,1 << (31 - 16)
            move    bb,bc
    .endif
.endif
    .if (src != src_ilen)
            move    src_ilen,src
    .endif
.ifnb gb_subr
    .if (BIG_ENDIAN == 1)
      .if (JOHN != 1) || (UCL_SMALL != 1) || (ALT_SMALL != 1)
         .error "assume 1==JOHN==UCL_SMALL==ALT_SMALL"
      .endif
            local around, refill
        gb_e = gb_subr
            b around
              li      last_m_off,1
        refill:
            // "lw bb,0(src_ilen)" of little-endian, unaligned data
            lbu     bb,3(src_ilen)
            lbu     var,2(src_ilen)
            sll     bb,bb,8
            or      bb,bb,var
            lbu     var,1(src_ilen)
            sll     bb,bb,8
            or      bb,bb,var
            lbu     var,0(src_ilen)
            sll     bb,bb,8
            or      bb,bb,var
            addiu   src_ilen,4

            srl     var,bb,31  # var= most significant bit of bb
            sll     bb,1
            jr      ra
              addiu   bb,1  # the flag bit
        gb_subr:
            beq     bc,bb,refill   # detect empty
              srl     var,bb,31  # var= most significant bit of bb
            jr      ra
              sll     bb,1
        around:
    .else
            li      last_m_off,1
    .endif
.else
            li      last_m_off,1
.endif

init_sz = . - init_sz

.endm


;//////////////////////////////////////
;// getbit macro
;//////////////////////////////////////

.macro  ADDBITS

.if (JOHN == 0)
    .if (UCL_SMALL == 1)
            addiu   bc, -1
            bltz    bc, 2b
    .else
            bgtz    bc, 2f
            addiu   bc, -1
    .endif

.else   //  John's method

    .if (UCL_SMALL == 1)
        .if (ALT_SMALL == 1)
            beq     bc,bb,2b  # detect flag bit [empty]
        .else
            addiu   bc,-1
            bltz    bc,2b
        .endif
    .else
        srl     var,bb,31  # var= most significant bit of bb
            bne     bc,bb,2f  # detect flag bit [empty]
        sll     bb,1
    .endif
.endif

.endm

.macro  ADDBITS_DONE

.if (JOHN == 0)
            srlv    var,bb,bc
    .if (UCL_SMALL == 1)
            jr      ra
    .endif
            andi    var,0x0001

.else   //  John's method

    .if (UCL_SMALL == 1)
        .if (ALT_SMALL == 1)
        srl     var,bb,31  # var= most significant bit of bb
            jr      ra
        sll     bb,1
        .else
            srlv    var,bb,bc
            jr      ra
            andi    var,0x0001
        .endif
    .else
        .if (UCL_NRV_BB == 8)
            sll     bb,1
            addiu   bb,1  # the flag bit
        srl     var,bb,8  # var= most significant bit of bb
            sll     bb,24  # left-justify in register
        .else
        srl     var,bb,31  # var= most significant bit of bb
        sll     bb,1
            addiu   bb,1
        .endif
    .endif

.endif

.endm

.macro  FILLBYTES_8

.if (JOHN == 0)
            li      bc,7
            lbu     bb,0(src_ilen)
            addiu   src_ilen,1

.else   //  John's method

    .if (ALT_SMALL == 0)
       .if (UCL_SMALL == 1)
            li      bc,8
        .endif
    .endif
            lbu     bb,0(src_ilen)
            addiu   src_ilen,1
    .if ((ALT_SMALL == 1) && (UCL_SMALL == 1))
            sll     bb,1
            addiu   bb,1  # the flag bit
            sll     bb,24-1 # left-justify in register
    .endif

.endif

.endm


#ifndef BIG_ENDIAN
#define BIG_ENDIAN 0
#endif

.macro  FILLBYTES_32

.if (JOHN == 0)
  .if (BIG_ENDIAN == 1)
            // "lw bb,0(src_ilen)" of little-endian, unaligned data
            lbu     bb,3(src_ilen)
            lbu     tmp,2(src_ilen)
            sll     bb,bb,8
            or      bb,bb,tmp
            lbu     tmp,1(src_ilen)
            sll     bb,bb,8
            or      bb,bb,tmp
            lbu     tmp,0(src_ilen)
            sll     bb,bb,8
            or      bb,bb,tmp
  .else  // LITTLE_ENDIAN
            lwr     bb,0(src_ilen)
            lwl     bb,3(src_ilen)
  .endif
            addiu   src_ilen,4
  .if (UCL_SMALL == 1)
            li      bc,32
  .else
            li      bc,31
  .endif
.else   //  John's method
    .if (ALT_SMALL == 0)
        .if (UCL_SMALL == 1)
            li      bc,32
        .endif
    .endif
    .if (BIG_ENDIAN == 1)
            // "lw bb,0(src_ilen)" of little-endian, unaligned data
            lbu     bb,3(src_ilen)
            lbu     tmp,2(src_ilen)
            sll     bb,bb,8
            or      bb,bb,tmp
            lbu     tmp,1(src_ilen)
            sll     bb,bb,8
            or      bb,bb,tmp
            lbu     tmp,0(src_ilen)
            sll     bb,bb,8
            or      bb,bb,tmp
    .else  // LITTLE_ENDIAN
            lwr     bb,0(src_ilen)
            lwl     bb,3(src_ilen)
    .endif
            addiu   src_ilen,4
    .if ((ALT_SMALL == 1) && (UCL_SMALL == 1))
        srl     var,bb,31  # var= most significant bit of bb
        sll     bb,1
            jr      ra
            addiu   bb,1
    .endif
.endif

.endm

.macro  FILLBITS

    .if (UCL_NRV_BB == 8)
            FILLBYTES_8
    .else // (UCL_NRV_BB == 32)
            FILLBYTES_32
    .endif

.endm

.macro  GBIT

    .if (UCL_SMALL == 1)
2:
            FILLBITS
1:
            ADDBITS
            ADDBITS_DONE
    .else
            ADDBITS
            FILLBITS
.if (JOHN == 1)
            ADDBITS_DONE
2:
.else
2:
            ADDBITS_DONE
.endif
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

.if (JOHN == 0)
            bal     t+4    // gb_sub+4
            addiu   bc,-1

.else   //  John's method

    .if (UCL_SMALL == 1)
        .if (ALT_SMALL == 0)
            bal     t+4    // gb_sub+4
            addiu   bc,-1
        .else
            bal     t     // gb_sub+4
            nop
        .endif
    .endif
.endif
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
            GBIT
    .endif
.else
.ifc "\option", "sub_only"

2:
            FILLBITS
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

            local   wordchk, prepbcpy
            local   bcopy, skip

.if (UCL_FAST == 1)
            slti    var,m_off,4
            bnez    var,prepbcpy
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

    .if (UCL_SMALL == 1)
        .if (WITHOUT_SUB == 1)
            t = gb_e
        .else
            t = 1f
        .endif
        .if (1 == 1)
            b       \ret
            nop
        .else
            bal     t
            addiu   ra, (\ret + 4) - (. + 4)
        .endif
    .else
        .if (JOHN == 0)
            b       \ret
            nop
        .else
            b       \ret + 4
            srl     var,bb,31  # var= most significant bit of bb
        .endif
    .endif
prepbcpy:
.else
            subu    m_pos,dst,m_off
.endif
bcopy:
            lbu     var,0(m_pos)
skip:
            addiu   m_len,-1
            addiu   m_pos,1
            addiu   dst,1
            bnez    m_len,bcopy
            sb      var,-1(dst)

    .if (UCL_SMALL == 1)
        .if (WITHOUT_SUB == 1)
            t = gb_e
        .else
            t = 1f
        .endif
        .if (1 == 1)
            b       \ret
            nop
        .else
            bal     t
            addiu   ra, (\ret + 4) - (. + 4)
        .endif
    .else
        .if (JOHN == 0)
            b       \ret
            nop
        .else
            b       \ret + 4
            srl     var,bb,31  # var= most significant bit of bb
        .endif
    .endif

.endm


#endif  //_MR3K_STD_CONF_
