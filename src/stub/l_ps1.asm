;  l_ps1.asm -- ps1/exe program entry & decompressor
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2003 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2003 Laszlo Molnar
;  Copyright (C) 2002-2003 Jens Medoch
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


#include <mr3k/macros.ash>

#define  SZ_REG  4

do_regs         MACRO   _w
                _w      at,SZ_REG*0(sp)
                _w      a0,SZ_REG*1(sp)
                _w      a1,SZ_REG*2(sp)
                _w      a2,SZ_REG*3(sp)
                _w      a3,SZ_REG*4(sp)
                _w      v0,SZ_REG*5(sp)
                _w      v1,SZ_REG*6(sp)
                _w      ra,SZ_REG*7(sp)
do_regs         ENDM

#define  REG_SZ (8*SZ_REG)

#define HI(a)   (a >> 16)
#define LO(a)   (a & 0xffff)

                ORG      0

; =============
; ============= ENTRY POINT
; =============

entry:
;       __PS1MAIN0__
                addiu   at,zero,'LS'    ; size of decomp. routine
                subu    sp,at           ; adjust the stack with this size
                do_regs sw              ; push used regs
                subiu   a0,at,REG_SZ    ; a0 = counter copyloop
                addiu   a3,sp,REG_SZ    ; get offset for decomp. routine
                move    a1,a3
                li      a2,'DCRT'       ; load decompression routine's offset
copyloop:
                addi    a0,-4
                lw      at,0(a2)        ; memcpy *a2 -> at -> *a1
                addiu   a2,4
                sw      at,0(a1)
                bnez    a0,copyloop
                addiu   a1,4

                li      a0,'COMP'       ; load COMPDATA HI offset
;               li      a1,'CDSZ'       ; compressed data size !disabled
;       __PS1MAINZ__

; =============

;       __PS1JSTA0__
                lui     a2,HI('DECO')   ; load DECOMPDATA HI offset
                jr      a3
                ori     a2,LO('DECO')   ; load DECOMPDATA LO offset
;       __PS1JSTAZ__
;       __PS1JSTH0__
                jr      a3
                lui     a2,HI('DECO')   ; same for HI only !(offset&0xffff)
;       __PS1JSTHZ__

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

;       __PS1DECO0__
;       __PS1DECOZ__
;       __PS1N2BD0__
#include <mr3k/n2b_d.ash>
;       __PS1N2BDZ__
;       __PS1N2DD0__
#include <mr3k/n2d_d.ash>
;       __PS1N2DDZ__
;       __PS1N2ED0__
#include <mr3k/n2e_d.ash>
;       __PS1N2EDZ__

; =============

;       __MSETBIG0__
                ori     a0,zero,'SC'    ; amount of removed zero's at eof
                sll     a0,3            ; (cd mode 2 data sector alignment)
;       __MSETBIGZ__
;       __MSETSML0__
                ori     a0,zero,'SC'    ; amount of removed zero's at eof
;       __MSETSMLZ__

; =============

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

; =============

;       __PS1EXIT0__
                li      t2,160          ; flushes
                jalr    ra,t2           ; instruction
                li      t1,68           ; cache
                do_regs lw              ; pop used regs
                DW      'JPEP'          ; marker for the entry jump
                addu    sp,at
;       __PS1EXITZ__

; =============

;       __PS1PHDR0__
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
;       __PS1PHDRZ__

; =============

;       __PS1RGSZ0__
                DW      REG_SZ
;       __PS1RGSZZ__
eof:

;                section .data
                DW      -1
                DH      eof

; vi:ts=8:et:nowrap
