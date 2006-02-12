;  l_ps1.asm -- ps1/exe program entry & decompressor
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2004 Laszlo Molnar
;  Copyright (C) 2002-2004 Jens Medoch
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

#define HI(a)   (a >> 16)
#define LO(a)   (a & 0xffff)

#define  SZ_REG  4

#if CDBOOT
regs            MACRO   _w, marker
                _w      a0,SZ_REG*0(sp)
                _w      a1,SZ_REG*1(sp)
                _w      a2,SZ_REG*2(sp)
                _w      a3,SZ_REG*3(sp)
                _w      v0,SZ_REG*4(sp)
                _w      v1,SZ_REG*5(sp)
IF (marker != 0)
                DW      marker
ENDIF
                _w      ra,SZ_REG*6(sp)
regs            ENDM

#define  REG_SZ (7*SZ_REG)

#else //console

regs            MACRO   _w, marker
                _w      at,SZ_REG*0(sp)
                _w      a0,SZ_REG*1(sp)
                _w      a1,SZ_REG*2(sp)
                _w      a2,SZ_REG*3(sp)
                _w      a3,SZ_REG*4(sp)
                _w      v0,SZ_REG*5(sp)
                _w      v1,SZ_REG*6(sp)
                _w      ra,SZ_REG*7(sp)
IF (marker != 0)
                DW      marker
                addu    sp,at
ENDIF
regs            ENDM

#define  REG_SZ (8*SZ_REG)

#endif //CDBOOT

                ORG      0
start:

#if CDBOOT
; =============
; ============= ENTRY POINT
; =============
; for cd-boot only

;       __PS1START__
                li      t0,'PSVR'       ; prepare to compute value
                subu    t0,s0,t0        ; get stored header offset in mem
                jr      t0
                subiu   sp,REG_SZ       ; adjust stack
cutpoint:
;       __PS1ENTRY__
                regs    sw,0            ; push used regs
                li      a0,'CPDO'       ; load COMPDATA offset
;               li      a1,'CDSZ'       ; compressed data size - disabled!
;       __PS1CONHL__
                li      a2,'DECO'
;       __PS1CONHI__
                lui     a2,HI('DECO')


#else //CONSOLE
; =============
; ============= ENTRY POINT
; =============
; for console- / cd-boot

;       __PS1START__
                addiu   at,zero,'LS'    ; size of decomp. routine
                subu    sp,at           ; adjust the stack with this size
                regs    sw,0            ; push used regs
                subiu   a2,at,REG_SZ    ; a2 = counter copyloop
                addiu   a3,sp,REG_SZ    ; get offset for decomp. routine
                move    a1,a3
                li      a0,'DCRT'       ; load decompression routine's offset
copyloop:
                lw      at,0(a0)        ; memcpy *a0 -> at -> *a1
                addiu   a2,-4
                sw      at,0(a1)
                addiu   a0,4
                bnez    a2,copyloop
                addiu   a1,4

;       __PS1PADCD__
                addiu   a0,'PC'         ; a0 = pointer compressed data
;       __PS1CONHL__
                lui     a2,HI('DECO')   ; load DECOMPDATA HI offset
                jr      a3
                ori     a2,LO('DECO')   ; load DECOMPDATA LO offset
;       __PS1CONHI__
                jr      a3
                lui     a2,HI('DECO')   ; same for HI only !(offset&0xffff)
cutpoint:
;       __PS1ENTRY__

#endif //CDBOOT
; =============
; ============= DECOMPRESSION
; =============

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

;       __PS1N2B08__
#include <mr3k/n2b_d.ash>
;       __PS1N2D08__
#include <mr3k/n2d_d.ash>
;       __PS1N2E08__
#include <mr3k/n2e_d.ash>

#ifdef NRV_BB
#   undef NRV_BB
#endif
#define NRV_BB 32

;       __PS1N2B32__
#include <mr3k/n2b_d.ash>
;       __PS1N2D32__
#include <mr3k/n2d_d.ash>
;       __PS1N2E32__
#include <mr3k/n2e_d.ash>


; =============

;       __PS1MSETS__
                ori     a0,zero,'SC'    ; amount of removed zeros at eof
;       __PS1MSETB__
                ori     a0,zero,'SC'    ; amount of removed zeros at eof
                sll     a0,3            ; (cd mode 2 data sector alignment)
;       __PS1MSETA__
memset_aligned:
                sw      zero,0(a2)
                addiu   a0,-1
                bnez    a0,memset_aligned
                addiu   a2,4
;       __PS1MSETU__
memset_unaligned:
                swl     zero,3(a2)
                swr     zero,0(a2)
                addiu   a0,-1
                bnez    a0,memset_unaligned
                addiu   a2,4

; =============

;       __PS1EXITC__
                li      t2,160          ; flushes
                jalr    ra,t2           ; instruction
                li      t1,68           ; cache
                regs    lw, 'JPEP'      ; marker for the entry jump

; =============

;       __PS1PAHDR__
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

; =============

#if !CDBOOT
;       __PS1SREGS__
                DW      REG_SZ
#endif //CDBOOT

;       __PS1EOASM__
eof:
;               section .data
                DW_UNALIGNED    -1
                DH_UNALIGNED    eof

; vi:ts=8:et:nowrap
