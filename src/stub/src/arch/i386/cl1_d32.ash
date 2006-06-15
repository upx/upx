;  cl1_d32.ash -- cl1_decompress_le32 in 32-bit assembly
;  schema from ucl/nrv2b_d32.ash
;
;  Copyright (C) 2004 John Reiser
;  Copyright (C) 1996-2003 Markus Franz Xaver Johannes Oberhumer
;  All Rights Reserved.
;
;  This file is free software; you can redistribute it and/or
;  modify it under the terms of the GNU General Public License as
;  published by the Free Software Foundation; either version 2 of
;  the License, or (at your option) any later version.
;
;  This file is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with the UCL library; see the file COPYING.
;  If not, write to the Free Software Foundation, Inc.,
;  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
;
;  Markus F.X.J. Oberhumer              John Reiser
;  <markus@oberhumer.com>               <jreiser@BitWagon.com>
;


; ------------- DECOMPRESSION -------------

; Input:
;   esi - source
;   edi - dest
;   ebp - -1
;   cld

; Output:
;   eax - 0
;   ecx - 0


    %ifndef jmps
    %define jmps    jmp short
    %endif
    %ifndef jmpn
    %define jmpn    jmp near
    %endif


;; One of these two is instantiated many times by buildLoader
%ifdef  __CL1SMA1B__
                call edx
%else;  __CL1FAS1B__
                add ebx, ebx
                db 0x75,2       ;jnz cl1_no_reload
                call edx
        ;cl1_no_reload:
%endif; __CL1GET1B__

; __CL1ENTER__
                nop             ; 'int3' for debugging
                call    start_cl1       ; pic push address of next instr
; __CL1SMA10__
getbit_cl1:  ; appears only in small version
                add ebx, ebx
                jz reload_cl1
                ret
; __CL1RLOAD__
reload_cl1:  ; for both small and fast version
                mov ebx, [esi]
                sub esi, byte -4
                adc ebx, ebx
                ret
; __CL1WID01__
widelit_cl1:
                sub ecx,ecx             ; back to 0
                ; getbit
; __CL1WID02__
                adc ecx,ecx
                ; getbit
; __CL1WID03__
                jc lit89_cl1
                ; getbit
; __CL1WID04__
                adc ecx,ecx
                ; getbit
; __CL1WID05__
                jc lit10_12_cl1
                ; getbit
; __CL1WID06__
                adc ecx,ecx             ; 0..7; -1+ (width/2) of length
litwidth_cl1:           ; ss22 algorithm, counted width<=8 pairs; returns eax= 2..0x15555
                ; getbit
; __CL1WID07__
                adc eax,eax
                dec eax
                ; getbit
; __CL1WID08__
                adc eax,eax
                sub ecx, byte 1
                jnc litwidth_cl1
                lea ecx,[17 -2 + eax]   ; 17: predecessors; ss22 returns 2..
                cmp eax,0xffff-(17 -2)
                jb litgo_cl1            ; not maximal range of search
                lea eax,[esi + ecx]     ; esi after copy
                push eax                ; "parameter" to maxlit_cl1
                jmpn maxlit_cl1         ; can have another literal afterwards
lit13_16_cl1:
                ; getbit
; __CL1WID09__
                adc ecx,ecx
                ; getbit
; __CL1WID10__
                adc ecx,ecx
                add ecx, byte 13
                jmps litmov_cl1
lit10_12_cl1:
                test ecx,ecx
                jz lit13_16_cl1
                inc ecx         ; 2,3,4
lit89_cl1:
                add ecx, byte 8
litgo_cl1:
                jmps litmov_cl1
; __CL1START__
start_cl1:
                sub ecx,ecx  ; 0
                pop edx             ; edx= getbit_cl1 or reload_cl1
                sub ebx, ebx        ; cause reload on first bit

; __CL1TOP00__
top_cl1:                ; In: 0==ecx
                lea eax,[1+ ecx]        ; 1: the msb of offset or large width
                ; getbit
; __CL1TOP01__
                jnc match_cl1
                ; getbit
; __CL1TOP02__
                jc lit1_cl1
                ; getbit
; __CL1TOP03__
                jc lit2_cl1
                ; getbit
; __CL1TOP04__
                jc lit3_cl1
                add ecx, byte 2
                ; getbit
; __CL1TOP05__
                jc lit45_cl1
                inc ecx
                ; getbit
; __CL1TOP06__
                jc lit67_cl1
                jmpn widelit_cl1
lit67_cl1:
lit45_cl1:
                ; getbit
; __CL1TOP07__
                adc ecx,ecx
litmov_cl1:
                db 0xD1,((3<<6)|(5<<3)|1)    ;shr ecx,1
                jnc litmovb_cl1
                movsb
litmovb_cl1:
                db 0xD1,((3<<6)|(5<<3)|1)    ;shr ecx,1
                jnc litmovw_cl1
                movsw
litmovw_cl1:
                rep
                movsd
                lea eax,[1+ ecx]  ; 1: the msb
                jmps litdone_cl1
lit3_cl1:
                movsb
lit2_cl1:
                movsb
lit1_cl1:
                movsb
litdone_cl1:

match_cl1:              ; In: 0==ecx; 1==eax

offset_cl1:             ; ss11 algorithm
                ; getbit
; __CL1OFF01__
                adc eax,eax
                ; getbit
; __CL1OFF02__
                jnc offset_cl1
                sub eax, byte 3         ; 2.. ==> -1[prev], (0,,<<8)|byte
                jc prev_off_cl1
                shl eax,8
                lodsb
                xor eax, byte ~0
                jz done_cl1             ; EOF
                mov ebp,eax             ; -offset
prev_off_cl1:           ; 1st 2 bits encode (5<=len),2,3,4
                ; getbit
; __CL1OFF03__
                adc ecx,ecx
                ; getbit
; __CL1OFF04__
                adc ecx,ecx
                jnz wrinkle_cl1
; __CL1LEN00__
                inc ecx         ; 1: the msb
mlen_cl1:
                ; getbit
; __CL1LEN01__
                adc ecx,ecx
                ; getbit
; __CL1LEN02__
                jnc mlen_cl1
                add ecx, byte 2         ; 2.. ==> 4..
; __CL1COPY0__
wrinkle_cl1:
                cmp ebp,-0xd00
                adc ecx, byte 1
copy_cl1:
                push esi
                lea esi,[edi + ebp]
                cmp ebp, byte -4
                ja ripple_cl1
maxlit_cl1: ; literal copy cannot overlap; omit test for ripple
                db 0xD1,((3<<6)|(5<<3)|1)    ;shr ecx,1
                jnc maxlitb_cl1
                movsb
maxlitb_cl1:
                db 0xD1,((3<<6)|(5<<3)|1)    ;shr ecx,1
                jnc maxlitw_cl1
                movsw
maxlitw_cl1:
                rep
                movsd
popbot_cl1:
                pop esi
bottom_cl1:
                jmpn top_cl1
ripple_cl1:
                cmp ebp, byte -1
                jne ripmov_cl1
                lodsb
                rep
                stosb
                jmps popbot_cl1
ripmov_cl1:
                rep
                movsb
                jmps popbot_cl1
done_cl1:
; __CL1END__

; vi:ts=8:et

