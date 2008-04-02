/*
;  nrv2e_d.ash -- NRV2E decompressor in Mips R3000 assembly
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
; On entry$: (regs are defined)
;   src compressed data pointer
;   dst store uncompressed data pointer
*/

.macro  nrv2e done

            local   n2e_1, n2e_2, n2e_3, n2e_4
            local   n2e_5, n2e_6, n2e_7

            init gb_nrv2e
n2e_1:
            GETBIT
            li      m_off,1
            beqz    var,n2e_2
            lbu     var,0(src_ilen)
            addiu   src_ilen,1
            sb      var,0(dst)
            b       n2e_1
            addiu   dst,1
n2e_2:
            GETBIT
            sll     m_off,1
        .if (UCL_SMALL == 1)
            GETBIT
            addu    m_off,var
        .else
            addu    m_off,var
            GETBIT
        .endif
            bnez    var,n2e_3
            addiu   var,m_off,-2
        .if (UCL_SMALL == 1)
            GETBIT
            addu    m_off,var
        .else
            addu    m_off,var
            GETBIT
        .endif
            b       n2e_2
            addu    m_off,var
n2e_3:
            bnez    var,n2e_4
            addiu   m_off,-3
            GETBIT
            move    m_off,last_m_off
            b       n2e_5
            andi    m_len,var,0x0001
n2e_4:
            lbu     var,0(src_ilen)
            sll     m_off,8
            addu    m_off,var
            addiu   var,m_off,1
            beqz    var,\done
            addiu   src_ilen,1
            srl     m_off,1
            addiu   m_off,1
            move    last_m_off,m_off
            andi    m_len,var,0x0001
n2e_5:
            GETBIT  0
            bnez    m_len,n2e_7
            addiu   m_len,var,3-5
            beqz    var,n2e_6
            li      m_len,1
            GETBIT  0
            b       n2e_7
            move    m_len,var
n2e_6:
            GETBIT
            sll     m_len,1
        .if (UCL_SMALL == 1)
            GETBIT
            addu    m_len,var
        .else
            addu    m_len,var
            GETBIT
        .endif
            beqz    var,n2e_6
n2e_7:
            sltiu   var,m_off,0x0501
            addiu   m_len,5
            subu    m_len,var
            uclmcpy n2e_1

.endm
