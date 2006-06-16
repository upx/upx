;  n2d_d.ash -- NRV2D decompressor in Mips R3000 assembly
;
;  This file is part of the UCL data compression library.
;
;  Copyright (C) 1996-2006 Markus Franz Xaver Johannes Oberhumer
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
;  Markus F.X.J. Oberhumer              Jens Medoch
;  <markus@oberhumer.com>               <jssg@users.sourceforge.net>
;  http://www.oberhumer.com/opensource/ucl/
;

#if (NRV_BB==8)
#   ifdef SMALL
        IF (!small)
            DEFINE  small
        ENDIF
#       define  UCL_DECOMPRESSOR    ucl_nrv2d_decompress_8_small
#       define  GETBIT              gbit_call gbit_sub,NRV_BB
#   else
#       define  UCL_DECOMPRESSOR    ucl_nrv2d_decompress_8
#       define  GETBIT              gbit_8
#   endif
#elif (NRV_BB==32)
#   ifdef SMALL
#       define  UCL_DECOMPRESSOR    ucl_nrv2d_decompress_32_small
#       define  GETBIT              gbit_call gbit_sub,NRV_BB
#   else
        IF (small)
            UNDEF  small
        ENDIF
#       define  UCL_DECOMPRESSOR    ucl_nrv2d_decompress_32
#       define  GETBIT              gbit_le32
#   endif
#else
#   error "define NRV_BB first!"
#endif


#include "bits.ash"

; ------------- DECOMPRESSION -------------
; On entry:
;   a0  src pointer
;   a2  dest pointer


UCL_DECOMPRESSOR    PROC
        init    NRV_BB
n2d_18:
        GETBIT
        beqz    var,n2d_74
        li      m_off,1
        lbu     var,0(src_ilen)
        addiu   src_ilen,1
        sb      var,0(dst)
        b       n2d_18
        addiu   dst,1
n2d_74:
        GETBIT
        sll     m_off,1
        addu    m_off,var
        GETBIT
        bnez    var,n2d_124
        addiu   var,m_off,-1
        sll     m_off,var,1
        GETBIT
        b       n2d_74
        addu    m_off,var
n2d_124:
        li      var,2
        bne     m_off,var,n2d_168
        addiu   m_off,-3
        GETBIT
        move    m_off,last_m_off
        b       n2d_198
        andi    m_len,var,0x0001
n2d_168:
        lbu     var,0(src_ilen)
        addiu   src_ilen,1
        sll     m_off,8
        addu    m_off,var
        li      var,-1
        beq     m_off,var,n2d_decomp_done
        nor     var,zero,m_off
        andi    m_len,var,0x0001
        srl     m_off,1
        addiu   m_off,1
        move    last_m_off,m_off
n2d_198:
        GETBIT
        sll     m_len,1
        addu    m_len,var
        bnez    m_len,n2d_25C
        sltiu   var,m_off,0x0501
        li      m_len,1
n2d_1E4:
        GETBIT
        sll     m_len,1
        addu    m_len,var
        GETBIT
        beqz2gb var,n2d_1E4,NRV_BB
        addiu   m_len,2
        sltiu   var,m_off,0x0501
n2d_25C:
        xori    var,0x0001
        addu    m_len,var
        uclmcpy n2d_18,NRV_BB

#ifdef SMALL
gbit_sub:
#   if (NRV_BB == 8)
        gbit_8
#   elif (NRV_BB == 32)
        gbit_le32
#   else
#       error "define NRV_BB first!"
#   endif
#endif

n2d_decomp_done:
UCL_DECOMPRESSOR    ENDP

#undef  UCL_DECOMPRESSOR
#undef  GETBIT
