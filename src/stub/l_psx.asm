;  l_psx.asm -- psx/exe program entry & decompressor
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2001 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2001 Laszlo Molnar
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
;  Markus F.X.J. Oberhumer   Laszlo Molnar
;  markus@oberhumer.com      ml1050@cdata.tvnet.hu
;
;  psOne r3k v1.2 by ssg

INCLUDE "mr3k/macros.asm"

                ORG      0

; =============
; ============= ENTRY POINT
; =============

;       __PSXMAIN0__
entry:
                addiu   at,zero,'LS'    ; size of decomp. routine
                sub     sp,at           ; adjust the stack with this size
                addi    sp,-(8*4)
                sw      at,0(sp)
                sw      a0,4(sp)
                sw      a1,8(sp)
                sw      a2,12(sp)
                sw      a3,16(sp)
                sw      v0,20(sp)
                sw      v1,24(sp)
                sw      ra,28(sp)
                move    a0,at
                lui     a2,'DH'         ; load DECOMPRESSION HI offset
                ori     a2,'DL'         ; and the LO part
                addiu   a3,sp,(8*4)
                move    a1,a3
copyloop:
                lw      at,0(a2)        ; memcpy *a2 -> at -> *a1
                addiu   a2,4
                addi    a0,-4
                sw      at,0(a1)
                bnez    a0,copyloop
                addiu   a1,4

                lui     a0,'CH'        ; load COMPDATA HI offset
                ori     a0,'CL'        ; and the LO part
;               lui     a1,'LH'        ; length of compressed data
;               ori     a1,'LL'        ; HI and LO, but disabled
                lui     a2,'OH'        ; load DECOMPDATA HI offset
                jr      a3
                ori     a2,'OL'        ; load DECOMPDATA LO offset
;       __PSXMAINZ__

; =============
; ============= DECOMPRESSION
; =============

;       __PSXDECO0__
;       __PSXDECOZ__
;       __PSXN2BD0__
                INCLUDE "mr3k/n2b_d32.asm"
;       __PSXN2BDZ__
;       __PSXN2DD0__
                INCLUDE "mr3k/n2d_d32.asm"
;       __PSXN2DDZ__
;       ;_PSXN2ED0__
;               ;;;;;INCLUDE "mr3k/n2e_d32.asm"
;       ;_PSXN2EDZ__


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
                lw      a0,4(sp)
                lw      a1,8(sp)
                lw      a2,12(sp)
                lw      a3,16(sp)
                lw      v0,20(sp)
                lw      v1,24(sp)
                lw      ra,28(sp)
                lw      at,0(sp)
                addiu   sp,(8*4)
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

