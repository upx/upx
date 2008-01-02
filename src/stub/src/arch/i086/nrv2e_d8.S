/*
;  n2e_d8e.ash -- ucl_nrv2e_decompress_8 in 16-bit assembly (dos/exe)
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
; ds:si - source
; es:di - dest
; dx = ds
; cx = 0
; bx = 0x800F
; bp = 1
*/

                CPU     8086

section         NRV2E16S
literal_n2e:
                movsb
decompr_start_n2e:
                add     bh, bh
                jnz     dec1_n2e
                call    reloadbh_n2e
dec1_n2e:
                jc      literal_n2e
                inc     cx
                mov     ax, es
section         N2E64K01
                add     di, di
                jnc     di_ok_n2e
                add     ah, 8
                mov     es, ax
di_ok_n2e:
                shr     di, 1
section         NRV2EEX1
offset_loop_n2e:
                call    getbit_cx_n2e
                jc      offset_loopend_n2e
                dec     cx
                call    getbit_n2e
                adc     cx, cx
                jns     offset_loop_n2e
                jmps    decomp_done_n2e

offset_loopend_n2e:
                dec     cx
                dec     cx
                jzs     offset_ok_n2e
section         N2EX8601
                add     cx, cx
                add     cx, cx
                add     cx, cx
section         N2E28601
                CPU     286
                shl     cx, 3
                CPU     8086
section         NRV2EEX2
                mov     bp, cx
                mov     bl, [si]
                inc     si
                not     bl
                xor     cx, cx
                shr     bl, 1
                jnc     ook1_n2e
mlen1_n2e:
                call    getbit_n2e
                adc     cx, cx
                jmps    copy_match_n2e

offset_ok_n2e:
                call    getbit_n2e
                jc      mlen1_n2e
ook1_n2e:
                inc     cx
                call    getbit_n2e
                jc      mlen1_n2e
length_loop_n2e:
                call    getbit_cx_n2e
                jnc     length_loop_n2e
                inc     cx
                inc     cx
copy_match_n2e:
                cmp     bp, 0x51
                sbb     cx, -3

                sub     ax, bp
                jbs     handle_underflow_n2e
                mov     ds, ax
                lea     ax, [bx + di]
ds_ok_n2e:
                sub     ah, bh
                xchg    ax, si
                rep
                movsb
                xchg    ax, si
                mov     ds, dx
                jmps    decompr_start_n2e

handle_underflow_n2e:
section         N2EX8602
                shl     ax, 1
                shl     ax, 1
                shl     ax, 1
                shl     ax, 1
                push    ax
                xor     ax, ax
                mov     ds, ax
                pop     ax
section         N2E28602
                CPU     286
                shl     ax, 4
                push    0
                pop     ds
                CPU     8086
section         NRV2EEX3
                add     ax, bx
                add     ax, di
                jmps    ds_ok_n2e

getbit_cx_n2e:
                add     bh, bh
                jnz     gb2_n2e
                call    reloadbh_n2e
gb2_n2e:
                adc     cx, cx
getbit_n2e:
                add     bh, bh
                jnzs    f2_n2e
reloadbh_n2e:
                mov     bh, [si]
section         N2E64K02
                adc     si, si
                jnc     si_ok_n2e
                add     dh, 8
                mov     ds, dx
si_ok_n2e:
                shr     si, 1
section         NRV2EEX9
                inc     si
                adc     bh, bh
f2_n2e:
                ret
decomp_done_n2e:


/* vim:set ts=4 et: */
