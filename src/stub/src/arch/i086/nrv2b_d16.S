/*
;  n2b_d16.ash -- ucl_nrv2b_decompress_le16 in 16-bit assembly
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


; ------------- DECOMPRESSION -------------

; Input
; bx - 0x8000
; cx - 0
; si - source
; di - dest
; bp - -1
*/

#ifndef  COPY_MATCH
# define COPY_MATCH     rep movsb
#endif

section         NRV2B160
literal_n2b:
                movsb
decomp_start_n2b:
                call    getbit_n2b
                jcs     literal_n2b

                inc     cx
loop1_n2b:
                call    getbit_cx_n2b
section         NRVDDONE
                jcxz    decomp_done_n2b
section         NRVDRETU
                jcxz    decomp_ret_n2b
section         NRVDECO1
                jnbs    loop1_n2b
                sub     cx, 3
                jbs     axbp_n2b
                mov     ah, cl
                lodsb
                not     ax
                xchg    bp, ax
axbp_n2b:
                xor     cx, cx
                call    getbit_cx_n2b
                adc     cx, cx
                jnz     copy_match_n2b
                inc     cx
loop2_n2b:
                call    getbit_cx_n2b
                jnbs    loop2_n2b
                inc     cx
                inc     cx
copy_match_n2b:
section         NRVLED00
                inc     cx
section         NRVGTD00
                cmp     bp, -0xd00
                adc     cx, 1
section         NRVDECO2
                lea     ax, [bp+di]
                xchg    ax, si
                COPY_MATCH
                xchg    ax, si
                jmps    decomp_start_n2b
getbit_cx_n2b:
                call    getbit_n2b
                adc     cx, cx
getbit_n2b:
                add     bx, bx
                jnz     decomp_ret_n2b
                lodsw
                adc     ax, ax
                xchg    ax, bx
decomp_ret_n2b:
                ret
decomp_done_n2b:


/*
; =============
; ============= 16-BIT CALLTRICK & JUMPTRICK
; =============
*/

                cjt16   decomp_ret_n2b


/* vim:set ts=4 et: */
