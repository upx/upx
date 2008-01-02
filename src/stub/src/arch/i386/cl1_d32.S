/*
;  cl1_d32.ash -- cl1_decompress_le32 in 32-bit assembly
;  schema from ucl/nrv2b_d32.ash
;
;  Copyright (C) 2004-2008 John Reiser
;  Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
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
*/


// One of these two is instantiated many times by buildLoader
section CL1SMA1B
                call edx
section CL1FAS1B
                add ebx, ebx
                jnzs 1f
                call edx
1:
section CL1GET1B

section CL1ENTER
                nop             // 'int3' for debugging
                call    start_cl1       // pic push address of next instr
section CL1SMA10
getbit_cl1:  // appears only in small version
                add ebx, ebx
                jzs reload_cl1
                ret
section CL1RLOAD
reload_cl1:  // for both small and fast version
                mov ebx, [esi]
                sub esi, -4
                adc ebx, ebx
                ret
section CL1WID01
widelit_cl1:
                sub ecx,ecx             // back to 0
                // getbit
section CL1WID02
                adc ecx,ecx
                // getbit
section CL1WID03
                jcs lit89_cl1
                // getbit
section CL1WID04
                adc ecx,ecx
                // getbit
section CL1WID05
                jcs lit10_12_cl1
                // getbit
section CL1WID06
                adc ecx,ecx             // 0..7; -1+ (width/2) of length
litwidth_cl1:           // ss22 algorithm, counted width<=8 pairs; returns eax= 2..0x15555
                // getbit
section CL1WID07
                adc eax,eax
                dec eax
                // getbit
section CL1WID08
                adc eax,eax
                sub ecx, 1
                jncs litwidth_cl1
                lea ecx,[17 -2 + eax]   // 17: predecessors; ss22 returns 2..
                cmp eax,0xffff-(17 -2)
                jbs litgo_cl1           // not maximal range of search
                lea eax,[esi + ecx]     // esi after copy
                push eax                // "parameter" to maxlit_cl1
                jmp maxlit_cl1          // can have another literal afterwards
lit13_16_cl1:
                // getbit
section CL1WID09
                adc ecx,ecx
                // getbit
section CL1WID10
                adc ecx,ecx
                add ecx, 13
                jmps litmov_cl1
lit10_12_cl1:
                test ecx,ecx
                jzs lit13_16_cl1
                inc ecx         // 2,3,4
lit89_cl1:
                add ecx, 8
litgo_cl1:
                jmps litmov_cl1
section CL1START
start_cl1:
                sub ecx,ecx  // 0
                pop edx             // edx= getbit_cl1 or reload_cl1
                sub ebx, ebx        // cause reload on first bit

section CL1TOP00
top_cl1:                // In: 0==ecx
                lea eax,[1+ ecx]        // 1: the msb of offset or large width
                // getbit
section CL1TOP01
                jncs match_cl1
                // getbit
section CL1TOP02
                jcs lit1_cl1
                // getbit
section CL1TOP03
                jcs lit2_cl1
                // getbit
section CL1TOP04
                jcs lit3_cl1
                add ecx, 2
                // getbit
section CL1TOP05
                jcs lit45_cl1
                inc ecx
                // getbit
section CL1TOP06
                jcs lit67_cl1
                jmp widelit_cl1
lit67_cl1:
lit45_cl1:
                // getbit
section CL1TOP07
                adc ecx,ecx
litmov_cl1:
                .byte 0xD1,((3<<6)|(5<<3)|1)    //shr ecx,1
                jncs litmovb_cl1
                movsb
litmovb_cl1:
                .byte 0xD1,((3<<6)|(5<<3)|1)    //shr ecx,1
                jncs litmovw_cl1
                movsw
litmovw_cl1:
                rep
                movsd
                lea eax,[1+ ecx]  // 1: the msb
                jmps litdone_cl1
lit3_cl1:
                movsb
lit2_cl1:
                movsb
lit1_cl1:
                movsb
litdone_cl1:

match_cl1:              // In: 0==ecx; 1==eax

offset_cl1:             // ss11 algorithm
                // getbit
section CL1OFF01
                adc eax,eax
                // getbit
section CL1OFF02
                jncs offset_cl1
                sub eax, 3         // 2.. ==> -1[prev], (0,,<<8)|byte
                jcs prev_off_cl1
                shl eax,8
                lodsb
                xor eax, ~0
                jzs done_cl1             // EOF
                mov ebp,eax             // -offset
prev_off_cl1:           // 1st 2 bits encode (5<=len),2,3,4
                // getbit
section CL1OFF03
                adc ecx,ecx
                // getbit
section CL1OFF04
                adc ecx,ecx
                jnzs wrinkle_cl1
section CL1LEN00
                inc ecx         // 1: the msb
mlen_cl1:
                // getbit
section CL1LEN01
                adc ecx,ecx
                // getbit
section CL1LEN02
                jncs mlen_cl1
                add ecx, 2         // 2.. ==> 4..
section CL1COPY0
wrinkle_cl1:
                cmp ebp,-0xd00
                adc ecx, 1
copy_cl1:
                push esi
                lea esi,[edi + ebp]
                cmp ebp, -4
                ja ripple_cl1
maxlit_cl1: // literal copy cannot overlap; omit test for ripple
                .byte 0xD1,((3<<6)|(5<<3)|1)    //shr ecx,1
                jncs maxlitb_cl1
                movsb
maxlitb_cl1:
                .byte 0xD1,((3<<6)|(5<<3)|1)    //shr ecx,1
                jncs maxlitw_cl1
                movsw
maxlitw_cl1:
                rep
                movsd
popbot_cl1:
                pop esi
bottom_cl1:
                jmp top_cl1
ripple_cl1:
                cmp ebp, -1
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
section CL1END

// vi:ts=8:et

