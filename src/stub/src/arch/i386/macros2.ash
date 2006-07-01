/*
;  macros.ash --
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2006 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2006 Laszlo Molnar
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
*/

                .intel_syntax noprefix

.macro          section name
                .section \name
                .code32
.endm

.macro          CPU     id
                .ifc    \id, 386
                //.arch   i386, nojumps
                .endif
                .ifc    \id, 486
                .arch   i486, nojumps
                .endif
.endm

.macro          jmps    target
                .byte   0xeb, \target - . - 1
.endm

.macro          jos     target
                .byte   0x70, \target - . - 1
.endm

.macro          jnos    target
                .byte   0x71, \target - . - 1
.endm

.macro          jcs     target
                .byte   0x72, \target - . - 1
.endm

.macro          jncs    target
                .byte   0x73, \target - . - 1
.endm

.macro          jzs     target
                .byte   0x74, \target - . - 1
.endm

.macro          jnzs    target
                .byte   0x75, \target - . - 1
.endm

.macro          jnas    target
                .byte   0x76, \target - . - 1
.endm

.macro          jas     target
                .byte   0x77, \target - . - 1
.endm

.macro          jss     target
                .byte   0x78, \target - . - 1
.endm

.macro          jnss    target
                .byte   0x79, \target - . - 1
.endm

.macro          jps     target
                .byte   0x7a, \target - . - 1
.endm

.macro          jnps    target
                .byte   0x7b, \target - . - 1
.endm

.macro          jls     target
                .byte   0x7c, \target - . - 1
.endm

.macro          jnls    target
                .byte   0x7d, \target - . - 1
.endm

.macro          jngs    target
                .byte   0x7e, \target - . - 1
.endm

.macro          jgs     target
                .byte   0x7f, \target - . - 1
.endm

#define         jes     jzs
#define         jnes    jnzs
#define         jbes    jnas

/*
;; =============
;; ============= 32-BIT CALLTRICK & JUMPTRICK
;; =============

;;  call & jump trick : 2 in 1
*/

.macro          cjt32   addvalue
section         CALLTR00
                mov     ecx, offset filter_length
calltrickloop:
                mov     al, [edi]
                inc     edi
                sub     al, 0xE8
ct1:
                cmp     al, 1
                ja      calltrickloop
section         CTCLEVE1
                cmpb    [edi], offset filter_cto
                jnzs    calltrickloop
section         CALLTR01
                mov     eax, [edi]
                mov     bl, [edi + 4]
section         CTBSHR01
                shr     ax, 8
section         CTBROR01
                xchg    ah, al
section         CTBSWA01
                rol     eax, 16
                xchg    ah, al
section         CALLTR02
                sub     eax, edi
                sub     bl, 0xE8

                .if     \addvalue
                add     eax, \addvalue
                .endif

                mov     [edi], eax
                add     edi, 5
                mov     al, bl
                loop    ct1

section         CALLTR10
//;; 32-bit call XOR jump trick
                mov     ecx, offset filter_length
ctloop1:
section         CALLTRE8
                mov     al,0xE8
section         CALLTRE9
                mov     al,0xE9
section         CALLTR11
ctloop2:
                repnz
                scasb
                jnzs    ctend
section         CTCLEVE2
                cmpb    [edi], offset filter_cto
                jnzs    ctloop2
section         CALLTR12
                mov     eax, [edi]
section         CTBSHR11
                shr     ax, 8
section         CTBROR11
                xchg    ah, al
section         CTBSWA11
                rol     eax, 16
                xchg    ah, al
section         CALLTR13
                sub     eax, edi

                .if     \addvalue
                add     eax, \addvalue
                .endif

                stosd
                jmps    ctloop1
ctend:
section CTTHEEND
.endm

#if 0

;;  call/jump/jcc trick; also used more than once (and/or optionally), so
;;  ecx has byte count (not count of applied instances), and
;;  edi points to buffer.
%macro          ckt32   2
; 1st param: effective addvalue (typically 0 or edi; any rvalue)
; 2nd param: where is cto8 (dl, bl, or literal)

section CKLLTR00
 %ifnidn %1,0
                mov     esi, %1
 %endif
                jmps    ckstart
ckloop3:
                mov     al, [edi]
                add     edi, byte 1
section CKLLTR10  Jcc only
                cmp     al, 0x80  ; lo of 6-byte Jcc
                jb      ckloop2
                cmp     al, 0x8f  ; hi of 6-byte Jcc
                ja      ckloop2
                cmp     byte [edi -2], 0x0F  ; prefix of 6-byte Jcc
                je      ckmark
ckloop2:
section CKLLTR20
                sub     al, 0xE8
                cmp     al, 0xE9 - 0xE8
                ja      ckcount
ckmark:
                cmp     byte [edi], %2  ; cto8
                jnz     ckcount
                mov     eax, [edi]

                shr     ax, 8
                rol     eax, 16
                xchg    ah, al
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
section CKLLTR30  Jcc only
                mov     al, [edi]
                add     edi, byte 1
                loop    ckloop2  ; prefix cannot overlap previous displacement
section CKLLTR40
ckcount:
                sub     ecx, byte 1
                jg      ckloop3
ckend:
%endmacro

;; =============
;; ============= 32-BIT RELOCATIONS
;; =============

%macro          reloc32 3
section RELOC320
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
                xchg    ah, al
                rol     eax, 16
                xchg    ah, al
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
section REL32BIG
                or      eax, eax
                jnz     reloc_add
                mov     eax, [%1]
                add     %1, byte 4
section RELOC32J
                jmps    reloc_add
reloc_endx:
section REL32END
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

section LXUNF000  enter at +0 for decompression; +2 for unfiltering
        jmps decompr0
section LXUNF002
  ;; 2+ address of decompress subroutine
  ;; unfilter(upx_byte *, length, cto8)
lxunfilter:
        pop edx  ; return address
        pop eax  ; upx_byte *, same as addvalue
        pop ecx  ; length
        xchg eax, edi  ; edi= pointer; eax= saved_edi
        pusha  ; save C-convention ebx, ebp, esi, edi; also eax, edx

; at most one of the next 2
section MRUBYTE0  256==n_mru
        xor ebx, ebx  ; zero
section LXMRU005  0!=n_mru
        mov ebx, 'NMRU'  ; modified N_MRU or N_MRU -1

section LXMRU006 0!=n_mru
        push byte 0x0f  ; prefix of 6-byte Jcc <d32>
        pop eax
        mov ah, [esp + 8*4]  ; cto8
section LXMRU007  0==n_mru
        push byte 0x0f  ; prefix of 6-byte Jcc <d32>
        pop ebx
        mov bh, [esp + 8*4]  ; cto8

section LXUNF008
        mov dl, [esp + 8*4]  ; cto8

section LXUNF010
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

section LXJCC010
lxunf2:  ; have seen 0x80..0x8f of possible recoded 6-byte Jcc <d32>
        movzx ebp, word [edi]  ; 2 bytes, zero-extended

section LXMRU045  0!=n_mru
        sub ebp, %$cto8_0f
section LXMRU046  0==n_mru
        sub ebp, ebx

section LXJCC020  0==n_mru, or Jcc excluded ('sub' of equals clears Carry)
        jne unfcount
section LXJCC021  0!=n_mru and Jcc participates; must set Carry
        sub ebp, byte 1  ; set Carry iff in range
        jnb unfcount

section LXJCC023  found Jcc; re-swap 0x8Y opcode and 0x0f prefix
        mov byte [edi -1], bl  ; 0x0f prefix
        dec ecx  ; preserve Carry
        mov byte [edi], al  ; Jcc opcode
        inc edi  ; preserve Carry

section LXUNF037
%define %$jc     eax

lxunf:  ; in: Carry set iff we should apply mru and 0!=n_mru
        mov eax, [edi]  ; BE32 displacement with cto8 in low 8 bits

section LXUNF386  0!=n_mru && 386
        pushf
section LXUNF387  ==386
        shr ax, 8
        rol eax, 16
        xchg ah, al
section LXUNF388  0!=n_mru && 386
        popf
        jnc unf_store  ; do not apply mru

section LXUNF486  >=486
        mov al, byte 0
        CPU     486
        bswap eax  ; preserve Carry (2-byte instruction)
        CPU     386
section LXUNF487  0!=n_mru && >=486
        jnc unf_store  ; do not apply mru

section LXMRU065  0!=n_mru
        shr %$jc, 1  ; eax= jc, or mru index
        jnc mru4  ; not 1st time for this jc
section MRUBYTE3
        dec %$hand_l
section MRUARB30
        dec %$hand
section MRUBITS3
        and %$hand, %$n_mru1
section MRUARB40
        jge mru3
        add %$hand, %$n_mru
mru3:
section LXMRU070

        mov [esp + 4*%$hand], %$jc  ; 1st time: mru[hand] = jc
        jmps unf_store

mru4:  ; not 1st time for this jc
        lea %$kh, [%$jc + %$hand]  ; kh = jc + hand
section MRUBYTE4
        movzx %$kh, %$kh_l
section MRUBITS4
        and %$kh, %$n_mru1
section MRUARB50
        cmp %$kh, %$n_mru
        jb mru5
        sub %$kh, %$n_mru
mru5:
section LXMRU080
        mov %$jc, [esp + 4*%$kh]  ; jc = mru[kh]
section MRUBYTE5
        dec %$hand_l
section MRUARB60
        dec %$hand
section MRUBITS5
        and %$hand, %$n_mru1
section MRUARB70
        jge mru6
        add %$hand, %$n_mru
mru6:
section LXMRU090

        mov %$tmp, [esp + 4*%$hand]  ; tmp = mru[hand]
        test %$tmp,%$tmp
        jnz mru8

          push %$jc  ; ran out of registers
        mov eax, %$tail

section MRUBYTE6
        dec al
section MRUARB80
        dec eax
section MRUBITS6
        and eax, %$n_mru1
section MRUARB90
        jge mru7
        add eax, %$n_mru
mru7:
section LXMRU100

        xor %$tmp,%$tmp
        mov %$tail, eax
        xchg [4+ esp + 4*eax], %$tmp  ; tmp = mru[tail]; mru[tail] = 0
          pop %$jc
mru8:
        mov [esp + 4*%$kh  ], %$tmp  ; mru[kh] = tmp
        mov [esp + 4*%$hand], %$jc   ; mru[hand] = jc
section LXUNF040
unf_store:
        sub eax, edi
        sub ecx, byte 4

; one of the next2
section LXMRU110  0!=n_mru
        add eax, %$addvalue
section LXMRU111  0==n_mru
        add eax, esi  ; addvalue (same as initial pointer)

section LXUNF041
        mov [edi], eax
        add edi, byte 4
        jmps unfcount
section LXUNF042
lxunf0:           ;; continuation of entry prolog for unfilter
section LEXEC016  bug in APP: jmp and label must be in same .asx/.asy
        jmp lxunf0  ; this instr does not really go here!

section LXMRU010 0!=n_mru
        push eax  ; cto8_0f
section LXJMPA00  only JMP, and not CALL, is filtered
        mov al, 0xE9
section LXCALLB0  only CALL, or both CALL and JMP are filtered
        mov al, 0xE8
section LXUNF021  common tail
        push eax  ; cto8_e8e9
        push byte 0  ; tail
        push ebx  ; n_mru or n_mru1
        mov esi, esp  ; flat model "[esi]" saves a byte over "[ebp]"

section LXMRU022  0==n_mru
        pop esi  ; addvalue
        mov edx, ebx  ; dh= cto8
section LXJMPA01  only JMP, and not CALL, is filtered
        mov dl, 0xE9
section LXCALLB1  only CALL, or both CALL and JMP are filtered
        mov dl, 0xE8


section MRUBITS1
        inc %$hand  ; n_mru1 ==> n_mru
section LXMRU030
lxunf1:  ; allocate and clear mru[]
        push byte 0

; one of the next 2, if n_mru
section MRUBYTE1
        dec %$hand_l
section MRUARB10
        dec %$hand

section LXMRU040  0!=n_mru
        jnz lxunf1  ; leaves 0=='hand'

section LXUNF030
lxctloop:
        movzx eax, word [edi]  ; 2 bytes, zero extended
        add edi, byte 1
section LXJCC000
        cmp al, 0x80  ; lo of Jcc <d32>
        jb lxct1
        cmp al, 0x8f  ; hi of Jcc <d32>
        jbe lxunf2
lxct1:

section LXCJ0MRU  0==n_mru
        sub eax, edx
section LXCJ1MRU  0!=n_mru
        sub eax, %$cto8_e8e9

; both CALL and JMP are filtered
section LXCALJMP
        sub eax, byte 1+ (0xE9 - 0xE8)  ; set Carry iff in range (result: -2, -1)

; only CALL, or only JMP, is filtered
section LXCALL00  0==n_mru
        je lxunf
section LXCALL01  0!=n_rmu
        sub eax, byte 1  ; set Carry iff in range

section LXCJ2MRU  0==n_mru, or apply mru to all that are filtered here
        jb lxunf  ; only Carry (Borrow) matters
section LXCJ4MRU  0!=n_mru, but apply mru only to subset of filtered here
        jnb unfcount  ; was not filtered anyway: do not unfilter

;we will unfilter, and 0!=n_mru, but should we apply mru?
section LXCJ6MRU  apply mru to JMP  only (0xFF==al)
        jpe lxct3  ; jump if even number of 1 bits in al
section LXCJ7MRU  apply mru to CALL only (0xFE==al)
        jpo lxct3  ; jump if odd  number of 1 bits in al
section LXCJ8MRU  do not apply mru to one or both
        clc
lxct3:
        jmps lxunf

section LXUNF034
unfcount:
        sub ecx, byte 1
        jg lxctloop

section LXMRU055
        mov edi, esp ; clear mru[] portion of stack
section MRUBYTE2
        mov ecx, 4+ 256  ; unused, tail, cto8_e8e9, cto8_0f
section MRUBITS2
        mov ecx, %$n_mru1
        add ecx, byte 1+ 4  ; n_mru1, tail, cto8_e8e9, cto8_0f
section MRUARB20
        mov ecx, %$n_mru
        add ecx, byte 4  ; n_mru, tail, cto8_e8e9, cto8_0f
section LXMRU057
        xor eax, eax
        rep
        stosd
        mov esp, edi

section LXMRU058  0==n_mru
        push esi
section LXUNF035
        popa
        xchg eax, edi
        push ecx
        push eax
        push edx
        ret
%pop
%endmacro
#endif

// vi:ts=8:et:nowrap
