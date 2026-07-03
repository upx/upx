/* arm64-win64.pe.S --
*
*  This file is part of the UPX executable compressor.
*
*  Copyright (C) Markus Franz Xaver Johannes Oberhumer
*  Copyright (C) Laszlo Molnar
*  Copyright (C) John F. Reiser
*  All Rights Reserved.
*
*  UPX and the UCL library are free software; you can redistribute them
*  and/or modify them under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; see the file COPYING.
*  If not, write to the Free Software Foundation, Inc.,
*  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
*  Markus F.X.J. Oberhumer              Laszlo Molnar
*  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
*
*  John F. Reiser
*  <jreiser@users.sourceforge.net>
*/

#include "arch/arm64/v8/macros.S"

// AArch64 win64/pe loader.  Faithful transliteration of amd64-win64.pe.S
// (same UPX linker symbols, same arithmetic) with three structural changes
// required by the architecture:
//   1) RIP-relative addressing -> PC-relative literal pools (no MOVW/ADRP in
//      ElfLinkerArm64LE; only ADR_PREL_LO21 / ABS32/64 / PREL32 / CALL26 / JUMP26).
//   2) The decompressor is a callable subroutine (bl), not inline fall-through.
//   3) An explicit instruction-cache flush (sync_cache) before entering the
//      decompressed original code -- x86 never needs this, AArch64 does.
//
// Register convention across the loader (callee-saved so they survive the
// decompressor bl): x19 = xdst (VA of decompressed region = imagebase+rvamin,
// the "rsi" of the amd64 stub after PEMAIN10), x20 = xsrc (VA of compressed data).

// LDADDR reg, label  -- reg = runtime VA of UPX-linker symbol `label`
//   `label` is a PREL32 literal (.word label - .); adr gives the literal's VA,
//   ldrsw loads (symval_rva - lit_rva), add yields imagebase + symval_rva.
.macro LDADDR reg, lbl
        adr     x17, 8f
        ldrsw   x16, [x17]
        add     \reg, x17, x16
        b       9f
8:      .word   \lbl - .
9:
.endm

// LDVAL reg, val  -- reg = raw 32-bit (sign-extended) value of linker symbol
.macro LDVAL reg, val
        adr     x17, 8f
        ldr     w16, [x17]
        sxtw    \reg, w16
        b       9f
8:      .word   \val
9:
.endm

.arch armv8-a

// =============
// ============= ENTRY POINT (EXE)
// =============

section START
section PEMAIN01
        stp     x29, x30, [sp, #-32]!
        stp     x19, x20, [sp, #16]

        LDADDR  x20, start_of_compressed        // x20 = xsrc (VA compressed)
        LDVAL   x9,  start_of_uncompressed
        add     x19, x20, x9                    // x19 = xdst (VA decompressed region)

section PEMAIN02
        // In-place (overlapping) decompression: the packer's
        // testOverlappingDecompression has already verified the source/dest
        // separation, and the AArch64 NRV decompressors are overlap-safe.
        // decompressor ABI: f(src=x0, c_len=w1, dst=x2, &u_len=x3, method=w4)
        mov     x0, x20                         // src = xsrc (compressed, in image)
        LDVAL   x1, comp_len
        mov     x2, x19                         // dst = xdst
        sub     sp, sp, #16
        mov     x3, sp                          // &u_len scratch slot
        mov     w4, wzr                         // NO_METHOD_CHECK: method unused
        // fall through into the one decompressor-call section the linker adds

// One of these single-instruction call sections is emitted by buildLoader,
// selected by ph.method.  Each branches-and-links to its decompressor body
// (placed after PEDOJUMP, reached only via bl) then falls through to PEMAIN10.
section PECALL2B
        bl      ucl_nrv2b_decompress_32
section PECALL2D
        bl      ucl_nrv2d_decompress_32
section PECALL2E
        bl      ucl_nrv2e_decompress_32

section PEMAIN10
        add     sp, sp, #16                     // discard &u_len slot

// =============
// ============= IMPORTS  (x19 = xdst base, "rsi")
// =============

section PEIMPORT
        LDVAL   x9, compressed_imports
        add     x21, x19, x9                    // x21 = rdi: import descriptor cursor
.Lnext_dll:
        ldr     w9, [x21]                       // dll name RVA (0 => done)
        cbz     w9, .Limports_done
        ldr     w10, [x21, #4]                  // iat RVA
        LDVAL   x11, start_of_imports
        add     x0, x19, x9
        add     x0, x0, x11                     // x0 = dll name VA
        add     x22, x19, x10                   // x22 = rbx: iat write cursor
        add     x21, x21, #8
        LDADDR  x12, LoadLibraryA
        ldr     x12, [x12]
        blr     x12                             // LoadLibraryA(name)
        mov     x23, x0                         // x23 = rbp: hmodule
.Lnext_func:
        // per-function flag byte: 0 => end of dll; 1 => by name (NUL-terminated
        // name follows); 0xff => by ordinal (le16 ordinal follows). The 0xfe
        // kernel32-ordinal optimization is not emitted unless the source imports
        // kernel32 by ordinal (rare); treated like 0xff here.
        ldrb    w9, [x21], #1
        cbz     w9, .Lnext_dll
        cmp     w9, #1
        b.ne    .Lby_ord
        mov     x1, x21                         // by name: lpProcName = name ptr
.Lscan_name:
        ldrb    w10, [x21], #1
        cbnz    w10, .Lscan_name
        b       .Lcall_gpa
.Lby_ord:
        ldrh    w1, [x21], #2                   // ordinal (< 0x10000 => by ordinal)
.Lcall_gpa:
        mov     x0, x23                         // hmodule
        LDADDR  x12, GetProcAddress
        ldr     x12, [x12]
        blr     x12                             // GetProcAddress(hmodule, name|ord)
        str     x0, [x22], #8
        b       .Lnext_func
.Limports_done:

// =============
// ============= RELOCATION  (x19 = base added to each DIR64 slot)
// =============

section PERELOC1
        LDVAL   x9, start_of_relocs
        add     x21, x19, x9                    // x21 = rdi: reloc stream cursor
        sub     x22, x19, #4                    // x22 = rbx: position (base-4)
.Lreloc_main:
        ldrb    w9, [x21], #1
        cbz     w9, .Lreloc_end
        cmp     w9, #0xEF
        b.hi    .Lreloc_fx
.Lreloc_add:
        add     x22, x22, x9                    // advance position by delta
        ldr     x10, [x22]
        rev     x10, x10                        // stored big-endian -> native
        add     x10, x10, x19                   // + base
        str     x10, [x22]
        b       .Lreloc_main
.Lreloc_fx:
        and     w9, w9, #0x0F
        lsl     w9, w9, #16
        ldrb    w10, [x21], #1                  // low8
        ldrb    w11, [x21], #1                  // mid8
        orr     w9, w9, w10
        orr     w9, w9, w11, lsl #8
        cbnz    w9, .Lreloc_add
        ldr     w9, [x21], #4                   // big: 32-bit LE delta
        b       .Lreloc_add
.Lreloc_end:

// =============
// ============= INSTRUCTION-CACHE SYNC  (AArch64-specific, new vs amd64)
// =============

section PECACHE
        // FlushInstructionCache(GetCurrentProcess()=-1, xdst, sizeof_image).
        // Windows ARM64 traps EL0 cache maintenance (mrs ctr_el0 / dc cvau /
        // ic ivau) as STATUS_ILLEGAL_INSTRUCTION, so we cannot use inline cache
        // ops the way the Linux stub does; the documented kernel32 API is required.
        mov     x0, #-1
        mov     x1, x19
        LDVAL   x2, sizeof_image
        LDADDR  x9, FlushInstructionCache
        ldr     x9, [x9]
        blr     x9

// =============
// ============= TLS callback support, part 1 (installer)
// =============
// Emitted only when the input has TLS callbacks.  The packed PE registers
// PETLSC2 (below) as its sole TLS callback; PETLSC2 starts inert (`ret`) so
// Windows' pre-entry callback invocation does nothing before decompression.
// Here, after the image is decompressed and cache-synced, we activate PETLSC2
// (patch its first instruction ret->nop) and invoke the original chain once
// for DLL_PROCESS_ATTACH.

section PETLSC
        LDADDR  x20, PETLSC2                     // x20 (callee-saved) = addr to patch
        movz    w10, #0x201f                     // NOP = 0xD503201F
        movk    w10, #0xd503, lsl #16
        str     w10, [x20]
        mov     x0, #-1                          // FlushInstructionCache(-1, &PETLSC2, 4)
        mov     x1, x20
        mov     x2, #4
        LDADDR  x9, FlushInstructionCache
        ldr     x9, [x9]
        blr     x9
        mov     x0, x19                          // tls_module_base (image base region)
        mov     w1, #1                           // DLL_PROCESS_ATTACH
        mov     x2, xzr                          // reserved
        bl      PETLSC2

// =============
// ============= JUMP TO ORIGINAL ENTRY POINT
// =============

section PEDOJUMP
        ldp     x19, x20, [sp, #16]
        ldp     x29, x30, [sp], #32
        b       original_entry

// =============
// ============= DECOMPRESSION  (callable; reached only via bl above)
// =============

section NRV_HEAD
#define M_NRV2B_LE32    2
#define M_NRV2D_LE32    5
#define M_NRV2E_LE32    8
#define M_CL1B_LE32     11
#define M_LZMA          14

#define xsrc    x0
#define xsrclen w1
#define xdst    x2
#define xdstlen x3
#define methb   w4

#define NO_METHOD_CHECK 1
// The decompressors' built-in sync_cache uses EL0 cache maintenance (mrs
// ctr_el0 / dc cvau / ic ivau), which Windows ARM64 traps as an illegal
// instruction. Suppress it; PECACHE flushes via kernel32 FlushInstructionCache.
#define NO_SYNC_CACHE 1

off     .req w5
section NRV2B
#include "arch/arm64/v8/nrv2b_d32.S"

section NRV2D
#include "arch/arm64/v8/nrv2d_d32.S"

section NRV2E
#include "arch/arm64/v8/nrv2e_d32.S"

// =============
// ============= TLS callback support, part 2 (the handler)
// =============
// Registered as the packed PE's TLS callback (tls_handler_offset =
// getSymbolOffset("PETLSC2")).  Inert until PETLSC patches the leading `ret`
// to a nop; thereafter walks the original callback chain.
//   TLS_CALLBACK(void *DllHandle x0, DWORD Reason w1, void *Reserved x2)

section PETLSC2
        ret                                     // patched to NOP by PETLSC
        stp     x29, x30, [sp, #-48]!
        stp     x19, x20, [sp, #16]
        stp     x21, x22, [sp, #32]
        mov     x19, x0
        mov     x20, x1
        mov     x21, x2
        LDADDR  x22, tls_callbacks_ptr
.Lwalk_tlsc:
        ldr     x9, [x22], #8
        cbz     x9, .Ltlsc_done
        mov     x0, x19
        mov     w1, w20
        mov     x2, x21
        blr     x9
        b       .Lwalk_tlsc
.Ltlsc_done:
        ldp     x21, x22, [sp, #32]
        ldp     x19, x20, [sp, #16]
        ldp     x29, x30, [sp], #48
        ret

// Data-only, never executed (placed after PEDOJUMP).  The base packer calls
// defineFilterSymbols() unconditionally; with no filter active it defines
// filter_length/filter_cto, so those external symbols must exist somewhere in
// the linked image even though this loader applies no filter yet.
section PEFILTSYM
        .word   filter_length
        .word   filter_cto

#include "include/header.S"
