;  macros.ash --
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2004 Laszlo Molnar
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


; =============
; ============= 16-BIT CALLTRICK & JUMPTRICK
; =============


%macro          cjt16   1
%ifdef  __CALLTR16__
                pop     si
                mov     cx, 'CT'
cjt16_L1:
                lodsb
                sub     al, 0xe8
                cmp     al, 1
                ja      cjt16_L1

%ifdef  __CT16I286__
                rol     word [si], 8
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
                jnz     %1              ; FIXME: this doesn't get relocated
%else;  __CT16JUL2__
                jnz     cjt16_L2
%endif; __CT16DUM2__

%ifdef  __CT16I287__
                rol     word [di], 8
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
%endmacro



;; =============
;; ============= 32-BIT CALLTRICK & JUMPTRICK
;; =============

;;  call & jump trick : 2 in 1
%macro          cjt32   1
%ifdef  __CALLTR00__
                mov     ecx, 'TEXL'
calltrickloop:
                mov     al, [edi]
                inc     edi
                sub     al, 0xE8
ct1:
                cmp     al, 1
                ja      calltrickloop
%ifdef  __CTCLEVE1__
                cmp     byte [edi], '?'
                jnz     calltrickloop
%endif; __CALLTR01__
                mov     eax, [edi]
                mov     bl, [edi + 4]
%ifdef  __CTDUMMY1__
%ifdef  __CTBSHR01__
                shr     ax, 8
%else;  __CTBROR01__
                xchg    al, ah
%endif; __CTBSWA01__
                rol     eax, 16
                xchg    al, ah
%endif; __CALLTR02__
                sub     eax, edi
                sub     bl, 0xE8
 %ifnidn %1,0
                add     eax, %1
 %endif
                mov     [edi], eax
                add     edi, byte 5
                mov     al, bl
                loop    ct1
%else;  __CALLTR10__
;; 32-bit call XOR jump trick
                mov     ecx, 'TEXL'
ctloop1:
%ifdef  __CALLTRE8__
                mov     al,0xE8
%else;  __CALLTRE9__
                mov     al,0xE9
%endif; __CALLTR11__
ctloop2:
                repnz
                scasb
                jnz     ctend
%ifdef  __CTCLEVE2__
                cmp     byte [edi], '?'
                jnz     ctloop2
%endif; __CALLTR12__
                mov     eax, [edi]
%ifdef  __CTDUMMY2__
%ifdef  __CTBSHR11__
                shr     ax, 8
%else;  __CTBROR11__
                xchg    al, ah
%endif; __CTBSWA11__
                rol     eax, 16
                xchg    al, ah
%endif; __CALLTR13__
                sub     eax, edi
 %ifnidn %1,0
                add     eax, %1
 %endif
                stosd
                jmps    ctloop1
ctend:
%endif; __CTTHEEND__
%endmacro


;;  call/jump/jcc trick; also used more than once (and/or optionally), so
;;  ecx has byte count (not count of applied instances), and
;;  edi points to buffer.
%macro          ckt32   2
; 1st param: effective addvalue (typically 0 or edi; any rvalue)
; 2nd param: where is cto8 (dl, bl, or literal)

;__CKLLTR00__
 %ifnidn %1,0
                mov     esi, %1
 %endif
                jmps    ckstart
ckloop3:
                mov     al, [edi]
                add     edi, byte 1
;__CKLLTR10__  Jcc only
                cmp     al, 0x80  ; lo of 6-byte Jcc
                jb      ckloop2
                cmp     al, 0x8f  ; hi of 6-byte Jcc
                ja      ckloop2
                cmp     byte [edi -2], 0x0F  ; prefix of 6-byte Jcc
                je      ckmark
ckloop2:
;__CKLLTR20__
                sub     al, 0xE8
                cmp     al, 0xE9 - 0xE8
                ja      ckcount
ckmark:
                cmp     byte [edi], %2  ; cto8
                jnz     ckcount
                mov     eax, [edi]

                shr     ax, 8
                rol     eax, 16
                xchg    al, ah
; above 3 instr are equivalent to the following 2 instr:
;               mov     al, 0   ; clear cto8  [setup partial-write stall]
;               bswap   eax     ; not on 386: need 486 and up

                sub     eax, edi
 %ifnidn %1,0
                add     eax, esi
 %endif
                mov     [edi], eax
                add     edi, byte 4
ckstart:
                sub     ecx, byte 4
;__CKLLTR30__  Jcc only
                mov     al, [edi]
                add     edi, byte 1
                loop    ckloop2  ; prefix cannot overlap previous displacement
;__CKLLTR40__
ckcount:
                sub     ecx, byte 1
                jg      ckloop3
ckend:
%endmacro

;; =============
;; ============= 32-BIT RELOCATIONS
;; =============

%macro          reloc32 3
;       __RELOC320__
reloc_main:
                xor     eax, eax
                mov     al, [%1]
                inc     %1
                or      eax, eax
                jz      reloc_endx
                cmp     al, 0xEF
                ja      reloc_fx
reloc_add:
                add     %2, eax
 %if 1
                mov     eax, [%2]
                xchg    al, ah
                rol     eax, 16
                xchg    al, ah
                add     eax, %3
                mov     [%2], eax
 %else
                add     [%2], %3
 %endif
                jmps    reloc_main
reloc_fx:
                and     al, 0x0F
                shl     eax, 16
                mov     ax, [%1]
                add     %1, byte 2
%ifdef  __REL32BIG__
                or      eax, eax
                jnz     reloc_add
                mov     eax, [%1]
                add     %1, byte 4
%endif; __RELOC32J__
                jmps    reloc_add
reloc_endx:
;       __REL32END__
%endmacro


;; =============
;; ============= 32-BIT CALL TRICK UNFILTER WITH MostRecentlyUsed BUFFER
;; =============

;;;; names of pseudo-sections for addLoader:
;; LXUNFnnn  Linux unfilter
;; LXMRUnnn  MostRecentlyUsed recoding of destinations
;; MRUARBnn  arbitrary number of entries in wheel
;; MRUBITSn  power of 2          entries in wheel (smaller code)
;; MRUBYTEn  256                 entries in wheel (smallest code)

%macro          ctojr32 0
%push ctojr32

;; I got confused by the syntactic sugar of the fake %ifdefs.
;; I can read the section name more easily when it is at the left margin.
;; Also, some of the logic to select the sections is not that simple,
;; and any mismatch between the logic and the %ifdefs is very confusing.
;; Instead, I use comments after the section name, and blank lines for grouping.

;__LXUNF000__  enter at +0 for decompression; +2 for unfiltering
        jmps decompr0
;__LXUNF002__
  ;; 2+ address of decompress subroutine
  ;; unfilter(upx_byte *, length, cto8)
lxunfilter:
        pop edx  ; return address
        pop eax  ; upx_byte *, same as addvalue
        pop ecx  ; length
        xchg eax, edi  ; edi= pointer; eax= saved_edi
        pusha  ; save C-convention ebx, ebp, esi, edi; also eax, edx

; at most one of the next 2
;__MRUBYTE0__  256==n_mru
        xor ebx, ebx  ; zero
;__LXMRU005__  0!=n_mru
        mov ebx, 'NMRU'  ; modified N_MRU or N_MRU -1

;__LXMRU006__ 0!=n_mru
        push byte 0x0f  ; prefix of 6-byte Jcc <d32>
        pop eax
        mov ah, [esp + 8*4]  ; cto8
;__LXMRU007__  0==n_mru
        push byte 0x0f  ; prefix of 6-byte Jcc <d32>
        pop ebx
        mov bh, [esp + 8*4]  ; cto8

;__LXUNF008__
        mov dl, [esp + 8*4]  ; cto8

;__LXUNF010__
        jmpn lxunf0
decompr0:

;; These %define are only if 0!=n_mru;
;; else 0x0F==bl && cto8==bh==dh && 0xE8==dl && addvalue==esi .
%define %$n_mru      [esi]
%define %$n_mru1     [esi]
%define %$tail       [esi + 4*1]
%define %$cto8_e8e9  [esi + 4*2]
%define %$cto8_0f    [esi + 4*3]
%define %$addvalue   [esi + 4*4]
%define %$tmp        ebp
%define %$hand       ebx
%define %$hand_l      bl
%define %$kh         edx
%define %$kh_l        dl

;__LXJCC010__
lxunf2:  ; have seen 0x80..0x8f of possible recoded 6-byte Jcc <d32>
        movzx ebp, word [edi]  ; 2 bytes, zero-extended

;__LXMRU045__  0!=n_mru
        sub ebp, %$cto8_0f
;__LXMRU046__  0==n_mru
        sub ebp, ebx

;__LXJCC020__  0==n_mru, or Jcc excluded ('sub' of equals clears Carry)
        jne unfcount
;__LXJCC021__  0!=n_mru and Jcc participates; must set Carry
        sub ebp, byte 1  ; set Carry iff in range
        jnb unfcount

;__LXJCC023__  found Jcc; re-swap 0x8Y opcode and 0x0f prefix
        mov byte [edi -1], bl  ; 0x0f prefix
        dec ecx  ; preserve Carry
        mov byte [edi], al  ; Jcc opcode
        inc edi  ; preserve Carry

;__LXUNF037__
%define %$jc     eax

lxunf:  ; in: Carry set iff we should apply mru and 0!=n_mru
        mov eax, [edi]  ; BE32 displacement with cto8 in low 8 bits

;__LXUNF386__  0!=n_mru && 386
        pushf
;__LXUNF387__  ==386
        shr ax, 8
        rol eax, 16
        xchg al, ah
;__LXUNF388__  0!=n_mru && 386
        popf
        jnc unf_store  ; do not apply mru

;__LXUNF486__  >=486
        mov al, byte 0
        CPU     486
        bswap eax  ; preserve Carry (2-byte instruction)
        CPU     386
;__LXUNF487__  0!=n_mru && >=486
        jnc unf_store  ; do not apply mru

;__LXMRU065__  0!=n_mru
        shr %$jc, 1  ; eax= jc, or mru index
        jnc mru4  ; not 1st time for this jc
;__MRUBYTE3__
        dec %$hand_l
;__MRUARB30__
        dec %$hand
;__MRUBITS3__
        and %$hand, %$n_mru1
;__MRUARB40__
        jge mru3
        add %$hand, %$n_mru
mru3:
;__LXMRU070__

        mov [esp + 4*%$hand], %$jc  ; 1st time: mru[hand] = jc
        jmps unf_store

mru4:  ; not 1st time for this jc
        lea %$kh, [%$jc + %$hand]  ; kh = jc + hand
;__MRUBYTE4__
        movzx %$kh, %$kh_l
;__MRUBITS4__
        and %$kh, %$n_mru1
;__MRUARB50__
        cmp %$kh, %$n_mru
        jb mru5
        sub %$kh, %$n_mru
mru5:
;__LXMRU080__
        mov %$jc, [esp + 4*%$kh]  ; jc = mru[kh]
;__MRUBYTE5__
        dec %$hand_l
;__MRUARB60__
        dec %$hand
;__MRUBITS5__
        and %$hand, %$n_mru1
;__MRUARB70__
        jge mru6
        add %$hand, %$n_mru
mru6:
;__LXMRU090__

        mov %$tmp, [esp + 4*%$hand]  ; tmp = mru[hand]
        test %$tmp,%$tmp
        jnz mru8

          push %$jc  ; ran out of registers
        mov eax, %$tail

;__MRUBYTE6__
        dec al
;__MRUARB80__
        dec eax
;__MRUBITS6__
        and eax, %$n_mru1
;__MRUARB90__
        jge mru7
        add eax, %$n_mru
mru7:
;__LXMRU100__

        xor %$tmp,%$tmp
        mov %$tail, eax
        xchg [4+ esp + 4*eax], %$tmp  ; tmp = mru[tail]; mru[tail] = 0
          pop %$jc
mru8:
        mov [esp + 4*%$kh  ], %$tmp  ; mru[kh] = tmp
        mov [esp + 4*%$hand], %$jc   ; mru[hand] = jc
;__LXUNF040__
unf_store:
        sub eax, edi
        sub ecx, byte 4

; one of the next2
;__LXMRU110__  0!=n_mru
        add eax, %$addvalue
;__LXMRU111__  0==n_mru
        add eax, esi  ; addvalue (same as initial pointer)

;__LXUNF041__
        mov [edi], eax
        add edi, byte 4
        jmps unfcount
;__LXUNF042__
lxunf0:           ;; continuation of entry prolog for unfilter
;__LEXEC016__  bug in APP: jmp and label must be in same .asx/.asy
        jmp lxunf0  ; this instr does not really go here!

;__LXMRU010__ 0!=n_mru
        push eax  ; cto8_0f
;__LXJMPA00__  only JMP, and not CALL, is filtered
        mov al, 0xE9
;__LXCALLB0__  only CALL, or both CALL and JMP are filtered
        mov al, 0xE8
;__LXUNF021__  common tail
        push eax  ; cto8_e8e9
        push byte 0  ; tail
        push ebx  ; n_mru or n_mru1
        mov esi, esp  ; flat model "[esi]" saves a byte over "[ebp]"

;__LXMRU022__  0==n_mru
        pop esi  ; addvalue
        mov edx, ebx  ; dh= cto8
;__LXJMPA01__  only JMP, and not CALL, is filtered
        mov dl, 0xE9
;__LXCALLB1__  only CALL, or both CALL and JMP are filtered
        mov dl, 0xE8


;__MRUBITS1__
        inc %$hand  ; n_mru1 ==> n_mru
;__LXMRU030__
lxunf1:  ; allocate and clear mru[]
        push byte 0

; one of the next 2, if n_mru
;__MRUBYTE1__
        dec %$hand_l
;__MRUARB10__
        dec %$hand

;__LXMRU040__  0!=n_mru
        jnz lxunf1  ; leaves 0=='hand'

;__LXUNF030__
lxctloop:
        movzx eax, word [edi]  ; 2 bytes, zero extended
        add edi, byte 1
;__LXJCC000__
        cmp al, 0x80  ; lo of Jcc <d32>
        jb lxct1
        cmp al, 0x8f  ; hi of Jcc <d32>
        jbe lxunf2
lxct1:

;__LXCJ0MRU__  0==n_mru
        sub eax, edx
;__LXCJ1MRU__  0!=n_mru
        sub eax, %$cto8_e8e9

; both CALL and JMP are filtered
;__LXCALJMP__
        sub eax, byte 1+ (0xE9 - 0xE8)  ; set Carry iff in range (result: -2, -1)

; only CALL, or only JMP, is filtered
;__LXCALL00__  0==n_mru
        je lxunf
;__LXCALL01__  0!=n_rmu
        sub eax, byte 1  ; set Carry iff in range

;__LXCJ2MRU__  0==n_mru, or apply mru to all that are filtered here
        jb lxunf  ; only Carry (Borrow) matters
;__LXCJ4MRU__  0!=n_mru, but apply mru only to subset of filtered here
        jnb unfcount  ; was not filtered anyway: do not unfilter

;we will unfilter, and 0!=n_mru, but should we apply mru?
;__LXCJ6MRU__  apply mru to JMP  only (0xFF==al)
        jpe lxct3  ; jump if even number of 1 bits in al
;__LXCJ7MRU__  apply mru to CALL only (0xFE==al)
        jpo lxct3  ; jump if odd  number of 1 bits in al
;__LXCJ8MRU__  do not apply mru to one or both
        clc
lxct3:
        jmps lxunf

;__LXUNF034__
unfcount:
        sub ecx, byte 1
        jg lxctloop

;__LXMRU055__
        mov edi, esp ; clear mru[] portion of stack
;__MRUBYTE2__
        mov ecx, 4+ 256  ; unused, tail, cto8_e8e9, cto8_0f
;__MRUBITS2__
        mov ecx, %$n_mru1
        add ecx, byte 1+ 4  ; n_mru1, tail, cto8_e8e9, cto8_0f
;__MRUARB20__
        mov ecx, %$n_mru
        add ecx, byte 4  ; n_mru, tail, cto8_e8e9, cto8_0f
;__LXMRU057__
        xor eax, eax
        rep
        stosd
        mov esp, edi

;__LXMRU058__  0==n_mru
        push esi
;__LXUNF035__
        popa
        xchg eax, edi
        push ecx
        push eax
        push edx
        ret
%pop
%endmacro

; vi:ts=8:et:nowrap
