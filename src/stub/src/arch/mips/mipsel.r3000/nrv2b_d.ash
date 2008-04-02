/*
;  nrv2b_d.ash -- NRV2B decompressor in Mips R3000 assembly
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


/*
; ------------- DECOMPRESSION -------------
; On entry: (regs are defined)
;   src compressed data pointer
;   dst store uncompressed data pointer
*/

.macro  nrv2b done

            local   n2b_1, n2b_2, n2b_3
            local   n2b_4, n2b_5, n2b_6

            init gb_nrv2b
n2b_1:
            GETBIT
            li      m_off,1
            beqz    var,n2b_2
            lbu     var,0(src_ilen)
            addiu   src_ilen,1
            sb      var,0(dst)
            b       n2b_1
            addiu   dst,1
n2b_2:
            GETBIT
            sll     m_off,1
        .if (UCL_SMALL == 1)
            GETBIT
            addu    m_off,var
        .else
            addu    m_off,var
            GETBIT
        .endif
            beqz    var,n2b_2
            li      var,2
            bne     m_off,var,n2b_3
            addiu   m_off,-3
            b       n2b_4
            move    m_off,last_m_off
n2b_3:
            lbu     var,0(src_ilen)
            sll     m_off,8
            addu    m_off,var
            addiu   m_off,1
            beqz    m_off,\done
            addiu   src_ilen,1
            move    last_m_off,m_off
n2b_4:
            GETBIT  1
            move    m_len,var
            GETBIT
            sll     m_len,1
            addu    m_len,var
            bnez    m_len,n2b_6
            addiu   m_len,2-4
            li      m_len,1
n2b_5:
            GETBIT
            sll     m_len,1
        .if (UCL_SMALL == 1)
            GETBIT
            addu    m_len,var
        .else
            addu    m_len,var
            GETBIT
        .endif
            beqz    var,n2b_5
n2b_6:
            sltiu   var,m_off,0x0D01
            addiu   m_len,4
            subu    m_len,var
            uclmcpy n2b_1

.endm
