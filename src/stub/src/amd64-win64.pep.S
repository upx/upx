/*
;  AMD64-win64.pep.S -- loader & decompressor for the w64/pe32+ format
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2009 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2009 Laszlo Molnar
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
;---------------------------------------------------------------------
;
; 64 bit modifications (C) 2010 Stefan Widmann
;
; Major changes when porting 32 bit code to 64 bit:
;  - there are no pusha/popa instructions
;  - since we cannot use pusha/popa, we save rbx, rsi, rdi and rbp instead
;  - inc <reg> are now being encoded in 2 byte instructions
;  - functions use fast call calling convention (parameters are passed in registers)
;    rcx, rdx,  r8, r9
;  - caller is responsible for stack allocation for callee
;  - caller cleans up stack
;  - caller must keep the stack 16 byte aligned, 32 bytes shadow space on stack
; For more information about the 64 bit calling convention see http://blogs.msdn.com/oldnewthing/archive/2004/01/14/58579.aspx
*/

#include "arch/amd64/macros.S"

#define         IMM8(value) byte ptr 0; \
                .reloc . - 1, R_X86_64_8, value

#define         IMM16(value) word ptr (1<<15); \
                .reloc . - 2, R_X86_64_16, (1<<15) + value

#define         IMM32(value) dword ptr (1<<31); \
                .reloc . - 4, R_X86_64_32, (1<<31) + value

#define         IMM64(value) qword ptr (1<<63); \
                .reloc . - 8, R_X86_64_64, (1<<63) + value

#define         SHORT(label) . + 1; .reloc . - 1, R_X86_64_PC8, label

//; debugging is not too user friendly under wine:
//; by adding the "DEBUG" macro into the code
//; an exception will be raised, and a register dump is printed
//; WINEDEBUG=trace+seh wine x.exe &> x.debug
#define         DEBUG           movb [0], 0

.intel_syntax noprefix

// =============
// ============= ENTRY POINT
// =============

section         START
section         PEISDLL0
                mov     [rsp + 8], rcx
                mov     [rsp + 0x10], rdx
                mov     [rsp + 0x18], r8

section         PEISDLL1
                cmp     dl, 1
                jnz     reloc_end_jmp
section         PEMAIN01
                //; remember to keep stack aligned!
                push    rbx
                push    rsi
                push    rdi
                push    rbp
                lea     rsi, [rip + start_of_compressed]
                lea     rdi, [rsi + start_of_uncompressed]

section         PEICONS1
                incw    [rdi + icon_offset]
section         PEICONS2
                add     [rdi + icon_offset], IMM16(icon_delta)
section         PETLSHAK
                lea     rax, [rdi + tls_address]
                push    [rax]   // save the TLS index
                mov     [rax],  IMM32(tls_value) // restore compressed data overwritten by the TLS index
                push    rax

section         PEMAIN02
                push    rdi
section         PEMAIN03

// =============
// ============= DECOMPRESSION
// =============

.att_syntax
section         NRV_HEAD
/* Working registers */
#define off  %eax  /* XXX: 2GB */
#define len  %ecx  /* XXX: 2GB */
#define lenq %rcx
#define bits %ebx
#define disp %rbp

        xor bits,bits  // empty; force refill
        xor len,len  // create loop invariant
        orq $(~0),disp  // -1: initial displacement
        call setup  // push &getbit [TUNED]
ra_setup:

#define jnextb0np jnextb0yp
#define jnextb0yp GETBITp; jnc
#define jnextb1np jnextb1yp
#define jnextb1yp GETBITp; jc
#define GETBITp \
        addl bits,bits; jnz 0f; \
        movl (%rsi),bits; subq $-4,%rsi; \
        adcl bits,bits; movb (%rsi),%dl; \
0:
/* Same, but without prefetch (not useful for length of match.) */
#define jnextb0n jnextb0y
#define jnextb0y GETBIT; jnc
#define jnextb1n jnextb1y
#define jnextb1y GETBIT; jc
#define GETBIT \
        addl bits,bits; jnz 0f; \
        movl (%rsi),bits; subq $-4,%rsi; \
        adcl bits,bits; \
0:

/* rotate next bit into bottom bit of reg */
#define getnextbp(reg) call *%r11; adcl reg,reg
#define getnextb(reg)  getnextbp(reg)


getbit:
        addl bits,bits; jz refill  // Carry= next bit
        rep; ret
refill:
        movl (%rsi),bits; subq $-4,%rsi  // next 32 bits; set Carry
        adcl bits,bits  // LSB= 1 (CarryIn); CarryOut= next bit
        movb (%rsi),%dl  // speculate: literal, or bottom 8 bits of offset
        rep; ret

copy:  // In: len, %rdi, disp;  Out: 0==len, %rdi, disp;  trashes %rax, %rdx
        leaq (%rdi,disp),%rax; cmpl $5,len  // <=3 is forced
        movb (%rax),%dl; jbe copy1  // <=5 for better branch predict
        cmpq $-4,disp;   ja  copy1  // 4-byte chunks would overlap
        subl $4,len  // adjust for termination cases
copy4:
        movl (%rax),%edx; addq $4,      %rax; subl $4,len
        movl %edx,(%rdi); leaq  4(%rdi),%rdi; jnc copy4
        addl $4,len; movb (%rax),%dl; jz copy0
copy1:
        incq %rax; movb %dl,(%rdi); subl $1,len
                   movb (%rax),%dl
        leaq 1(%rdi),%rdi;          jnz copy1
copy0:
        rep; ret

setup:
        cld
        pop %r11  // addq $ getbit - ra_setup,%r11  # &getbit

#define NO_METHOD_CHECK

section         NRV2B
#include "arch/amd64/nrv2b_d.S"
section         NRV2D
#include "arch/amd64/nrv2d_d.S"
section         NRV2E
#include "arch/amd64/nrv2e_d.S"
#undef len

.intel_syntax noprefix
section         LZMA_HEAD
                mov     eax, IMM32(lzma_u_len)
                push    rax
                mov     rcx, rsp
                mov     rdx, rdi
                mov     rdi, rsi
                mov     esi, IMM32(lzma_c_len)

.att_syntax
#include "arch/amd64/regs.h"
#include "arch/amd64/lzma_d.S"

.intel_syntax noprefix
section         LZMA_TAIL
                leave
                pop     rax
// =============
section         PEMAIN10
eof:
                pop     rsi             // load vaddr

section         PETLSHAK2               // restore the TLS index
                pop     rdi
                pop     rax
                mov     [rdi], eax

// =============
// ============= FILTERS
// =============

section         PECTTPOS
                lea     rdi, [rsi + filter_buffer_start]
section         PECTTNUL
                mov     rdi, rsi

section         PEFILTER49
                mov     ecx, offset filter_length
                mov     dl, IMM8(filter_cto)
.att_syntax
#define esi ebx
#define rsi rbx
#include "arch/amd64/bxx.S"
#undef esi
#undef rsi
.intel_syntax noprefix

// =============
// ============= IMPORTS
// =============

section PEIMPORT
                sub     rsp, 0x28
                lea     rdi, [rsi + compressed_imports]
next_dll:
                mov     eax, [rdi]
                or      eax, eax
                jz      SHORT(imports_done)
                mov     ebx, [rdi + 4]    // iat
                lea     rcx, [rax + rsi + start_of_imports]
                add     rbx, rsi
                add     rdi, 8

                call    [rsi + LoadLibraryA]

                xchg    rax, rbp
next_func:
                mov     al, [rdi]
                inc     rdi
                or      al, al
                jz      next_dll
section         PEIBYORD
                jns     SHORT(byname)
section         PEK32ORD
                jpe     not_kernel32
                mov     eax, [rdi]
                add     rdi, 4
                mov     rax, [rax + rsi + kernel32_ordinals]
                jmp     SHORT(next_imp)
not_kernel32:
section         PEIMORD1
                movzxw  rdx, word ptr [rdi]
                add     rdi, 2
                jmp     SHORT(first_imp)

byname:
section         PEIMPOR2
                mov     rcx, rdi        // something > 0
                mov     rdx, rdi
                dec     eax
                repne
                scasb
first_imp:
                mov     rcx, rbp

                call    [rsi + GetProcAddress]

#if 1
;// FIXME: is this error handling really needed?
                or      rax, rax
                jz      imp_failed
#endif
next_imp:
                mov     [rbx], rax
                add     rbx, 8
                jmp     SHORT(next_func)
imp_failed:
section         PEIERDLL
                add     rsp, 0x28
                pop     rbp
                pop     rdi
                pop     rsi
                pop     rbx
                xor     eax, eax
                ret

section         PEIEREXE
                //      rcx contains garbage -> garbage return code
                jmp     [rsi + ExitProcess]
section         PEIMDONE
imports_done:
                add     rsp, 0x28

// =============
// ============= RELOCATION
// =============

section         PERELOC1
                lea     rdi, [rsi + start_of_relocs]
section         PERELOC2
                add     rdi, 4
section         PERELOC3
                lea     rbx, [rsi - 4]
reloc_main:
                xor     eax, eax
                mov     al, [rdi]
                inc     rdi
                or      eax, eax
                jz      SHORT(reloc_endx)
                cmp     al, 0xEF
                ja      reloc_fx
reloc_add:
                add     rbx, rax
                mov     rax, [rbx]
                bswap   rax
                add     rax, rsi
                mov     [rbx], rax
                jmp     reloc_main
reloc_fx:
                and     al, 0x0F
                shl     eax, 16
                mov     ax, [rdi]
                add     rdi, 2
section         REL64BIG
                or      eax, eax
                jnz     SHORT(reloc_add)
                mov     eax, [rdi]
                add     rdi, 4
section         RELOC64J
                jmp     SHORT(reloc_add)
reloc_endx:


// =============

// FIXME: depends on that in PERELOC1 rdi is set!!
section         PERLOHI0
                xchg    rdi, rsi
                lea     rcx, [rdi + reloc_delt]

section         PERELLO0
                jmp     1f
rello0:
                add     [rdi + rax], cx
1:
                lodsd
                or      eax, eax
                jnz     rello0

// =============

section         PERELHI0
                shr     ecx, 16
                jmp     1f
relhi0:
                add     [rdi + rax], cx
1:
                lodsd
                or      eax, eax
                jnz     relhi0

// =============
section         PEDEPHAK
                mov     rbp, [rsi + VirtualProtect]
                lea     rdi, [rsi + vp_base]
                mov     ebx, IMM32(vp_size)     // 0x1000 or 0x2000

                push    rax                     // provide 8 bytes stack
                mov     r9, rsp
                mov     r8d, 4                  // PAGE_READWRITE
                mov     rdx, rbx                // size
                mov     rcx, rdi                // address

                sub     rsp, 0x20
                call    rbp                     // VirtualProtect

                lea     rax, [rdi + swri]
                andb    [rax], 0x7f             // marks UPX0 non writeable
                andb    [rax + 0x28], 0x7f      // marks UPX1 non writeable

                lea     r9, [rsp + 0x20]
                movq    r8, [r9]                // original protection
                mov     rdx, rbx
                mov     rcx, rdi

                call    rbp
                add     rsp, 0x28

// =============
// ============= TLS callback support part 1
// =============

section         PETLSC
                movb    [rip + PETLSC2], 0xfc   // "cld" instead of "ret"
                lea     rcx, [rsi + tls_module_base] // module base
                push    1                       // DLL_PROCESS_ATTACH
                pop     rdx
                xor     r8, r8                  // 0 - reserved

                push    rax                     // align stack
                call    PETLSC2
                pop     rax

// ============= Cleanup

section         PEMAIN20
                pop     rbp
                pop     rdi
                pop     rsi
                pop     rbx

// clear the dirty stack
.macro          clearstack128  tmp_reg
                .local   loop
                lea     \tmp_reg, [rsp - 128]
loop:
                push    0
                cmp     rsp, \tmp_reg
                jnz     loop
                sub     rsp, -128
.endm

section         CLEARSTACK
                clearstack128 rax

section         PEMAIN21
reloc_end_jmp:

section         PEISDLL9
                mov     r8, [rsp + 0x18]
                mov     rdx, [rsp + 0x10]
                mov     rcx, [rsp + 8]

section         PERETURN
                push    1
                pop     rax
                ret
section         PEDOJUMP
                jmp     original_entry

// =============
// ============= TLS callback support part 2
// =============

// this is the new TLS callback handler
// it calls the original callbacks ONLY after the compression is done

section         PETLSC2         // TLS_CALLBACK(hModule, reason, reserved)
                ret             // this ret gets overwritten with cld by PETLSC
                push    rsi
                lea     rsi, [rip + tls_callbacks_ptr]
walk_tlsc_chain2:
                lodsq
                test    rax, rax
                jz      done_callbacks

                push    rcx
                push    rdx
                push    r8

                sub     rsp, 0x28
                call    rax
                add     rsp, 0x28

                pop     r8
                pop     rdx
                pop     rcx

                jmp     walk_tlsc_chain2
done_callbacks:
                pop     rsi
                ret

// =============
// ============= CUT HERE
// =============

#include        "include/header.S"

// vi:ts=8:et:nowrap
