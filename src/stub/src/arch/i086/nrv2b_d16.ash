;  n2b_d16.ash -- ucl_nrv2b_decompress_le16 in 16-bit assembly
;
;  This file is part of the UCL data compression library.
;
;  Copyright (C) 1996-2005 Markus Franz Xaver Johannes Oberhumer
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


    %ifndef jmps
    %define jmps    jmp short
    %endif
    %ifndef jmpn
    %define jmpn    jmp near
    %endif

    CPU 8086


;       __NRV2B160__
literal_n2b:
                movsb
decomp_start_n2b:
                call    getbit_n2b
                jc      literal_n2b

                inc     cx
loop1_n2b:
                call    getbit_cx_n2b
%ifdef  __NRVDDONE__
                jcxz    decomp_done_n2b
%else;  __NRVDRETU__
                jcxz    decomp_ret_n2b
%endif; __NRVDECO1__
                jnb     loop1_n2b
                sub     cx, byte 3
                jb      axbp_n2b
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
                jnb     loop2_n2b
                inc     cx
                inc     cx
copy_match_n2b:
%ifdef  __NRVLED00__
                inc     cx
%else;  __NRVGTD00__
                cmp     bp, -0xd00
                adc     cx, byte 1
%endif; __NRVDECO2__
                lea     ax, [di+bp]
                xchg    ax, si
                rep
                movsb
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
;       __NRV2B169__



; =============
; ============= 16-BIT CALLTRICK & JUMPTRICK
; =============


    %ifdef CJT16

%ifdef  __CALLTR16__
                pop     si
                mov     cx, 'CT'
cjt16_L1:
                lodsb
                sub     al, 0xe8
                cmp     al, 1
                ja      cjt16_L1

%ifdef  __CT16I286__
    CPU 286
                rol     word [si], 8
    CPU 8086
;       __CT16SUB0__
                sub     [si], si
%else;  __CT16I086__
                mov     bx, [si]
                xchg    bl, bh
                sub     bx, si
                mov     [si], bx
%endif; __CALLTRI2__
                lodsw
                loop    cjt16_L1
%endif; __CT16DUM1__

; =============

%ifdef  __CT16E800__
                mov     al, 0xe8
%else;  __CT16E900__
                mov     al, 0xe9
%endif; __CALLTRI5__
                pop     di
                mov     cx, 'CT'
cjt16_L11:
                repne
                scasb
%ifdef  __CT16JEND__
                jnz     decomp_ret_n2b
%else;  __CT16JUL2__
                jnz     cjt16_L2
%endif; __CT16DUM2__

%ifdef  __CT16I287__
    CPU 286
                rol     word [di], 8
    CPU 8086
;       __CT16SUB1__
                sub     [di], di
%else;  __CT16I087__
                mov     bx, [di]
                xchg    bl, bh
                sub     bx, di
                mov     [di], bx
%endif; __CALLTRI6__
                scasw
                jmps    cjt16_L11
cjt16_L2:
;       __CT16DUMM3__

    %endif ; CJT16


    CPU 8086

; vi:ts=8:et

