;  l_ps1.asm -- ps1/exe program entry & decompressor
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2001 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2001 Laszlo Molnar
;  Copyright (C) 2002      Jens Medoch
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


#include "mr3k/macros.ash"

do_regs         MACRO   _w
                _w      at,0(sp)
                _w      a0,4(sp)
                _w      a1,8(sp)
                _w      a2,12(sp)
                _w      a3,16(sp)
                _w      v0,20(sp)
                _w      v1,24(sp)
                _w      ra,28(sp)
do_regs         ENDM

DEFINE  REG_SZ = (8*4)

                ORG      0

; =============
; ============= ENTRY POINT
; =============

entry:
;       __PSXPREP0__                    ; needed by packer to calc the LS value
                addiu   at,zero,'LS'    ; size of decomp. routine
                sub     sp,at           ; adjust the stack with this size
;       __PSXPREPZ__                    ; needed by packer to calc the LS value
;       __PSXSTSZ0__                    ; needed by packer to calc the LS value
                do_regs sw              ; push used regs
;       __PSXSTSZZ__
;       __PSXMAIN0__
                subiu   a0,at,REG_SZ    ; a0 = counter copyloop
                addiu   a3,sp,REG_SZ    ; get offset for decomp. routine
                move    a1,a3
                lui     a2,'DH'         ; load decomp routine HI offset
                ori     a2,'DL'         ; and the LO offset
copyloop:
                addi    a0,-4
                lw      at,0(a2)        ; memcpy *a2 -> at -> *a1
                addiu   a2,4
                sw      at,0(a1)
                bnez    a0,copyloop
                addiu   a1,4

                lui     a0,'CH'        ; load COMPDATA HI offset
                ori     a0,'CL'        ; and the LO part
;               lui     a1,'LH'        ; compressed data length
;               ori     a1,'LL'        ; HI and LO !disabled
;       __PSXMAINZ__
;       __PSXJSTA0__
                lui     a2,'OH'        ; load DECOMPDATA HI offset
                jr      a3
                ori     a2,'OL'        ; load DECOMPDATA LO offset
;       __PSXJSTAZ__
;       __PSXJSTH0__
                jr      a3             ; 
                lui     a2,'OH'        ; same for HI only !(offset&0xffff) 
;       __PSXJSTHZ__

; =============
; ============= DECOMPRESSION
; =============
#if 1
#   define NRV_BB 8
#else
#   define NRV_BB 32
#endif

#if 1
#   define SMALL
#endif

;       __PSXDECO0__
;       __PSXDECOZ__
;       __PSXN2BD0__
#include "mr3k/n2b_d.ash"
;       __PSXN2BDZ__
;       __PSXN2DD0__
#include "mr3k/n2d_d.ash"
;       __PSXN2DDZ__
;       __PSXN2ED0__
#include "mr3k/n2e_d.ash"
;       __PSXN2EDZ__


;       __MSETBIG0__
                ori     a0,zero,'SC'    ; amount of removed zero's at eof
                sll     a0,3            ; (cd mode 2 data sector alignment)
;       __MSETBIGZ__
;       __MSETSML0__
                ori     a0,zero,'SC'    ; amount of removed zero's at eof
;       __MSETSMLZ__
;       __MSETALG0__
memset_aligned:
                addi    a0,-4
                sw      zero,0(a2)
                bnez    a0,memset_aligned
                addiu   a2,4
;       __MSETALGZ__
;       __MSETUAL0__
memset_unaligned:
                addi    a0,-4
                swl     zero,3(a2)
                swr     zero,0(a2)
                bnez    a0,memset_unaligned
                addiu   a2,4
;       __MSETUALZ__
;       __PSXEXIT0__
                do_regs lw              ; pop used regs
                DW      'JPEP'          ; marker for the entry jump
                addu    sp,at
;       __PSXEXITZ__
;       __PSXPHDR0__
                DB      85,80,88,33     ;  0  UPX_MAGIC_LE32
        ; another magic for PackHeader::putPackHeader
                DB      161,216,208,213 ;     UPX_MAGIC2_LE32
                DW      0               ;  8  uncompressed adler32
                DW      0               ; 12  compressed adler32
                DW      0               ; 16  uncompressed len
                DW      0               ; 20  compressed len
                DW      0               ; 24  original file size
                DB      0               ; 28  filter id
                DB      0               ; 29  filter cto
                DB      0               ;  unsused
                DB      45              ; 31  header checksum
;       __PSXPHDRZ__
eof:

;                section .data
                DW      -1
                DH      eof

