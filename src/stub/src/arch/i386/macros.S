/*
;  macros.S --
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2008 Laszlo Molnar
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
;  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
;
*/

#undef amd64
#undef i386
#undef i486
#undef i586
#undef x86_64

                .altmacro
                .intel_syntax noprefix
                .arch   i386, jumps
                .code32

.macro          section name
                .section \name
                .code32
.endm

.macro          CPU     id
                .ifc    \id, 386
                .arch   i386, jumps
                .endif
                .ifc    \id, 486
                .arch   i486, jumps
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

#define         jbes    jnas
#define         jbs     jcs
#define         jes     jzs
#define         jnbs    jncs
#define         jnes    jnzs


/*
;; =============
;; ============= 32-BIT CALLTRICK & JUMPTRICK
;; =============

;;  call & jump trick : 2 in 1
*/

.macro          cjt32   addvalue
                local   calltrickloop
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

        .ifnc   \addvalue, 0
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

                .ifnc   \addvalue, 0
                add     eax, \addvalue
                .endif

                stosd
                jmps    ctloop1
ctend:
.endm

// cit32 is same as cjt32 except esi captures 'addvalue' at the beginning
.macro          cit32   addvalue
section         CALLTR00
                mov     ecx, offset filter_length
        .ifnc   \addvalue, 0
        .ifnc   \addvalue, esi
                mov     esi, \addvalue
        .endif
        .endif
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

        .ifnc   \addvalue, 0
                add     eax, esi
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

        .ifnc   \addvalue, 0
                add     eax, esi
        .endif

                stosd
                jmps    ctloop1
ctend:
.endm

/*
;;  call/jump/jcc trick; also used more than once (and/or optionally), so
;;  ecx has byte count (not count of applied instances), and
;;  edi points to buffer.
*/
.macro          ctok32  addvalue, cto8
                local   ckstart, ckmark, ckcount, ckend
                local   ckloop2, ckloop3
// 1st param: effective addvalue (typically 0 or edi; any rvalue)
// 2nd param: where is cto8 (dl, bl, or literal)

section         ctok32.00
        .ifnc   \addvalue, 0
        .ifc    \addvalue, edi
                mov     esi, edi
        .endif
        .endif
                jmps    ckstart
ckloop3:
                mov     al, [edi]
                add     edi, 1
section         ctok32.10                // Jcc only
                cmp     al, 0x80        // lo of 6-byte Jcc
                jb      ckloop2
                cmp     al, 0x8f        // hi of 6-byte Jcc
                ja      ckloop2
                cmpb    [edi -2], 0x0F  // prefix of 6-byte Jcc
                jes     ckmark
ckloop2:
section         ctok32.20
                sub     al, 0xE8
                cmp     al, 0xE9 - 0xE8
                jas     ckcount
ckmark:
                cmpb    [edi], \cto8
                jnzs    ckcount
                mov     eax, [edi]

                shr     ax, 8
                rol     eax, 16
                xchg    ah, al
// above 3 instr are equivalent to the following 2 instr:
//               mov     al, 0   ; clear cto8  [setup partial-write stall]
//               bswap   eax     ; not on 386: need 486 and up

                sub     eax, edi
        .ifnc   \addvalue, 0
        .ifc    \addvalue, edi
                add     eax, esi
        .else
                add     eax, \addvalue
        .endif
        .endif
                mov     [edi], eax
                add     edi, 4
ckstart:
                sub     ecx, 4
section         ctok32.30   // Jcc only
                mov     al, [edi]
                add     edi, 1
                loop    ckloop2  // prefix cannot overlap previous displacement
section         ctok32.40
ckcount:
                sub     ecx, 1
                jgs     ckloop3
ckend:
.endm

// =============
// ============= 32-BIT RELOCATIONS
// =============

.macro          reloc32 buffer, destination, addvalue
section         RELOC320
reloc_main:
                xor     eax, eax
                mov     al, [\buffer]
                inc     \buffer
                or      eax, eax
                jzs     reloc_endx
                cmp     al, 0xEF
                ja      reloc_fx
reloc_add:
                add     \destination, eax
#if 1
                mov     eax, [\destination]
                xchg    ah, al
                rol     eax, 16
                xchg    ah, al
                add     eax, \addvalue
                mov     [\destination], eax
#else
                add     [\destination], \addvalue
#endif
                jmps    reloc_main
reloc_fx:
                and     al, 0x0F
                shl     eax, 16
                mov     ax, [\buffer]
                add     \buffer, 2
section         REL32BIG
                or      eax, eax
                jnzs    reloc_add
                mov     eax, [\buffer]
                add     \buffer, 4
section         RELOC32J
                jmps    reloc_add
reloc_endx:
section         REL32END
.endm

/*
;; =============
;; ============= 32-BIT CALL TRICK UNFILTER WITH MostRecentlyUsed BUFFER
;; =============

;;;; names of pseudo-sections for addLoader:
;; LXUNFnnn  Linux unfilter
;; LXMRUnnn  MostRecentlyUsed recoding of destinations
;; MRUARBnn  arbitrary number of entries in wheel
;; MRUBITSn  power of 2          entries in wheel (smaller code)
;; MRUBYTEn  256                 entries in wheel (smallest code)
*/

/*
;; I got confused by the syntactic sugar of the fake %ifdefs.
;; I can read the section name more easily when it is at the left margin.
;; Also, some of the logic to select the sections is not that simple,
;; and any mismatch between the logic and the %ifdefs is very confusing.
;; Instead, I use comments after the section name, and blank lines for grouping.
*/

.macro  ctojr32

section LXUNF000        // enter at +0 for decompression; +2 for unfiltering
        jmps decompr0
section LXUNF002
  // 2+ address of decompress subroutine
  // unfilter(upx_byte *, length, cto8)
lxunfilter:
        pop edx  // return address
        pop eax  // upx_byte *, same as addvalue
        pop ecx  // length
        xchg eax, edi  // edi= pointer; eax= saved_edi
        pusha  // save C-convention ebx, ebp, esi, edi; also eax, edx

// at most one of the next 2
section MRUBYTE0 // 256==n_mru
        xor ebx, ebx  // zero
section LXMRU005 // 0!=n_mru
        mov ebx, offset NMRU  // modified N_MRU or N_MRU -1

section LXMRU006 // 0!=n_mru
        push 0x0f  // prefix of 6-byte Jcc <d32>
        pop eax
        mov ah, [esp + 8*4]  // cto8
section LXMRU007 //  0==n_mru
        push 0x0f  // prefix of 6-byte Jcc <d32>
        pop ebx
        mov bh, [esp + 8*4]  // cto8

section LXUNF008
        mov dl, [esp + 8*4]  // cto8

section LXUNF010
        jmp lxunf0
decompr0:

// These #define are only if 0!=n_mru;
// else 0x0F==bl && cto8==bh==dh && 0xE8==dl && addvalue==esi .
#define __n_mru      [esi]
#define __n_mru1     [esi]
#define __tail       [esi + 4*1]
#define __cto8_e8e9  [esi + 4*2]
#define __cto8_0f    [esi + 4*3]
#define __addvalue   [esi + 4*4]
#define __tmp        ebp
#define __hand       ebx
#define __hand_l      bl
#define __kh         edx
#define __kh_l        dl

section LXJCC010
lxunf2:  // have seen 0x80..0x8f of possible recoded 6-byte Jcc <d32>
        movzxw ebp, [edi]  // 2 bytes, zero-extended

section LXMRU045 // 0!=n_mru
        sub ebp, __cto8_0f
section LXMRU046 // 0==n_mru
        sub ebp, ebx

section LXJCC020 // 0==n_mru, or Jcc excluded ('sub' of equals clears Carry)
        jnes unfcount
section LXJCC021 // 0!=n_mru and Jcc participates// must set Carry
        sub ebp, 1  // set Carry iff in range
        jnbs unfcount

section LXJCC023 // found Jcc// re-swap 0x8Y opcode and 0x0f prefix
        mov [edi -1], bl  // 0x0f prefix
        dec ecx  // preserve Carry
        mov [edi], al  // Jcc opcode
        inc edi  // preserve Carry

section LXUNF037
#define __jc     eax

lxunf:  // in: Carry set iff we should apply mru and 0!=n_mru
        mov eax, [edi]  // BE32 displacement with cto8 in low 8 bits

section LXUNF386 // 0!=n_mru && 386
        pushf
section LXUNF387 // ==386
        shr ax, 8
        rol eax, 16
        xchg ah, al
section LXUNF388 // 0!=n_mru && 386
        popf
        jncs unf_store  // do not apply mru

section LXUNF486 // >=486
        mov al, 0
        CPU     486
        bswap eax  // preserve Carry (2-byte instruction)
        CPU     386
section LXUNF487 // 0!=n_mru && >=486
        jncs unf_store  // do not apply mru

section LXMRU065 // 0!=n_mru
    CPU     486         // (use faster but longer shr opcode sequence)
        shr __jc, 1  // eax= jc, or mru index
    CPU     386
        jncs mru4  // not 1st time for this jc
section MRUBYTE3
        dec __hand_l
section MRUARB30
        dec __hand
section MRUBITS3
        and __hand, __n_mru1
section MRUARB40
        jge mru3
        add __hand, __n_mru
mru3:
section LXMRU070

        mov [esp + 4*__hand], __jc  // 1st time: mru[hand] = jc
        jmps unf_store

mru4:  // not 1st time for this jc
        lea __kh, [__jc + __hand]  // kh = jc + hand
section MRUBYTE4
        movzx __kh, __kh_l
section MRUBITS4
        and __kh, __n_mru1
section MRUARB50
        cmp __kh, __n_mru
        jb mru5
        sub __kh, __n_mru
mru5:
section LXMRU080
        mov __jc, [esp + 4*__kh]  // jc = mru[kh]
section MRUBYTE5
        dec __hand_l
section MRUARB60
        dec __hand
section MRUBITS5
        and __hand, __n_mru1
section MRUARB70
        jge mru6
        add __hand, __n_mru
mru6:
section LXMRU090

        mov __tmp, [esp + 4*__hand]  // tmp = mru[hand]
        test __tmp,__tmp
        jnzs mru8

          push __jc  // ran out of registers
        mov eax, __tail

section MRUBYTE6
        dec al
section MRUARB80
        dec eax
section MRUBITS6
        and eax, __n_mru1
section MRUARB90
        jge mru7
        add eax, __n_mru
mru7:
section LXMRU100

        xor __tmp,__tmp
        mov __tail, eax
        xchg [4+ esp + 4*eax], __tmp  // tmp = mru[tail]; mru[tail] = 0
          pop __jc
mru8:
        mov [esp + 4*__kh  ], __tmp  // mru[kh] = tmp
        mov [esp + 4*__hand], __jc   // mru[hand] = jc
section LXUNF040
unf_store:
        sub eax, edi
        sub ecx, 4

// one of the next2
section LXMRU110 // 0!=n_mru
        add eax, __addvalue
section LXMRU111 // 0==n_mru
        add eax, esi  // addvalue (same as initial pointer)

section LXUNF041
        mov [edi], eax
        add edi, 4
        jmps unfcount
section LXUNF042
lxunf0:           // continuation of entry prolog for unfilter
section LEXEC016 // bug in APP: jmp and label must be in same .asx/.asy
        jmp lxunf0  // this instr does not really go here!

section LXMRU010 // 0!=n_mru
        push eax  // cto8_0f
section LXJMPA00 // only JMP, and not CALL, is filtered
        mov al, 0xE9
section LXCALLB0 // only CALL, or both CALL and JMP are filtered
        mov al, 0xE8
section LXUNF021 // common tail
        push eax  // cto8_e8e9
        push 0  // tail
        push ebx  // n_mru or n_mru1
        mov esi, esp  // flat model "[esi]" saves a byte over "[ebp]"

section LXMRU022 // 0==n_mru
        pop esi  // addvalue
        mov edx, ebx  // dh= cto8
section LXJMPA01 // only JMP, and not CALL, is filtered
        mov dl, 0xE9
section LXCALLB1 // only CALL, or both CALL and JMP are filtered
        mov dl, 0xE8


section MRUBITS1
        inc __hand  // n_mru1 ==> n_mru
section LXMRU030
lxunf1:  // allocate and clear mru[]
        push 0

// one of the next 2, if n_mru
section MRUBYTE1
        dec __hand_l
section MRUARB10
        dec __hand

section LXMRU040 // 0!=n_mru
        jnzs lxunf1  // leaves 0=='hand'

section LXUNF030
lxctloop:
        movzxw eax, [edi]  // 2 bytes, zero extended
        add edi, 1
section LXJCC000
        cmp al, 0x80  // lo of Jcc <d32>
        jb lxct1
        cmp al, 0x8f  // hi of Jcc <d32>
        jbes lxunf2
lxct1:

section LXCJ0MRU // 0==n_mru
        sub eax, edx
section LXCJ1MRU // 0!=n_mru
        sub eax, __cto8_e8e9

// both CALL and JMP are filtered
section LXCALJMP
        sub eax, 1+ (0xE9 - 0xE8)  // set Carry iff in range (result: -2, -1)

// only CALL, or only JMP, is filtered
section LXCALL00 // 0==n_mru
        jes lxunf
section LXCALL01 // 0!=n_rmu
        sub eax, 1  // set Carry iff in range

section LXCJ2MRU // 0==n_mru, or apply mru to all that are filtered here
        jbs lxunf  // only Carry (Borrow) matters
section LXCJ4MRU // 0!=n_mru, but apply mru only to subset of filtered here
        jnbs unfcount  // was not filtered anyway: do not unfilter

//we will unfilter, and 0!=n_mru, but should we apply mru?
section LXCJ6MRU // apply mru to JMP  only (0xFF==al)
        jps lxct3  // jump if even number of 1 bits in al
section LXCJ7MRU // apply mru to CALL only (0xFE==al)
        jnps lxct3  // jump if odd  number of 1 bits in al
section LXCJ8MRU // do not apply mru to one or both
        clc
lxct3:
        jmps lxunf

section LXUNF034
unfcount:
        sub ecx, 1
        jgs lxctloop

section LXMRU055
        mov edi, esp // clear mru[] portion of stack
section MRUBYTE2
        mov ecx, 4+ 256  // unused, tail, cto8_e8e9, cto8_0f
section MRUBITS2
        mov ecx, __n_mru1
        add ecx, 1+ 4  // n_mru1, tail, cto8_e8e9, cto8_0f
section MRUARB20
        mov ecx, __n_mru
        add ecx, 4  // n_mru, tail, cto8_e8e9, cto8_0f
section LXMRU057
        xor eax, eax
        rep
        stosd
        mov esp, edi

section LXMRU058 // 0==n_mru
        push esi
section LXUNF035
        popa
        xchg eax, edi
        push ecx
        push eax
        push edx
        ret
.endm

// vi:ts=8:et:nowrap
