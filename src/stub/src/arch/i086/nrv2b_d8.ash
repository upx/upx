;  n2b_d8e.ash -- ucl_nrv2b_decompress_8 in 16-bit assembly (dos/exe)
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


    %ifndef jmps
    %define jmps    jmp short
    %endif
    %ifndef jmpn
    %define jmpn    jmp near
    %endif

    CPU 8086


;       __NRV2B16S__
literal_n2b:
                movsb
decompr_start_n2b:
                add     bh, bh
                jnz     dec1_n2b
                call    reloadbh_n2b
dec1_n2b:
                jc      literal_n2b
                inc     cx
                mov     ax, es
%ifdef  __N2B64K01__
                add     di, di
                jnc     di_ok_n2b
                add     ah, 8
                mov     es, ax
di_ok_n2b:
                shr     di, 1
%endif; __NRV2BEX1__
offset_loop_n2b:
                call    getbit_cx_n2b
                jcxz    decomp_done_n2b
                jnc     offset_loop_n2b
                dec     cx
                dec     cx
                jz      offset_ok_n2b
%ifdef  __N2BX8601__
                add     cx, cx
                add     cx, cx
                add     cx, cx
                add     cx, cx
%else;  __N2B28601__
    CPU 286
                shl     cx, 4
    CPU 8086
%endif; __NRV2BEX2__
                mov     bp, cx
                mov     bl, [si]
                inc     si
                not     bl
                xor     cx, cx
offset_ok_n2b:
                call    getbit_cx_n2b
                adc     cx, cx
                jnz     copy_match_n2b
                inc     cx
length_loop_n2b:
                call    getbit_cx_n2b
                jnc     length_loop_n2b
                inc     cx
                inc     cx
copy_match_n2b:
                cmp     bp, 0xd1
                sbb     cx, byte -2

                sub     ax, bp
                jb      handle_underflow_n2b
                mov     ds, ax
                lea     ax, [bx + di]
ds_ok_n2b:
                sub     ah, bh
                xchg    ax, si
                rep
                movsb
                xchg    ax, si
                mov     ds, dx
                jmps    decompr_start_n2b
handle_underflow_n2b:
%ifdef  __N2BX8602__
                shl     ax, 1
                shl     ax, 1
                shl     ax, 1
                shl     ax, 1
                push    ax
                xor     ax, ax
                mov     ds, ax
                pop     ax
%else;  __N2B28602__
    CPU 286
                shl     ax, 4
                push    byte 0
                pop     ds
    CPU 8086
%endif; __NRV2BEX3__
                add     ax, bx
                add     ax, di
                jmps    ds_ok_n2b
getbit_cx_n2b:
                add     bh, bh
                jnz     gb2_n2b
                call    reloadbh_n2b
gb2_n2b:
                adc     cx, cx
getbit_n2b:
                add     bh, bh
                jnz     f2_n2b
reloadbh_n2b:
                mov     bh, [si]
%ifdef  __N2B64K02__
                adc     si, si
                jnc     si_ok_n2b
                add     dh, 8
                mov     ds, dx
si_ok_n2b:
                shr     si, 1
%endif; __NRV2BEX9__
                inc     si
                adc     bh, bh
f2_n2b:
                ret
decomp_done_n2b:
;       __NRV2B16E__


    CPU 8086

; vi:ts=8:et

