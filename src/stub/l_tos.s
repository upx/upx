;  l_tos.s -- loader & decompressor for the atari/tos format
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2002 Laszlo Molnar
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


#define NRV_BB  8
#include "../version.h"


;
; see also:
;   freemint/sys/mint/basepage.h
;   freemint/sys/mint/mem.h         (FILEHEAD)
;   freemint/sys/memory.c           (load_region, load_and_reloc)
;   freemint/sys/arch/cpu.S         (cpush)
;

;
; This file is first preprocessed by cpp, then the a68k assembler
; is run and finally the generated object file is translated to a .h file
; by a simple perl script. We also maintain compatiblity with the pasm
; assembler (which must be started in the emulator window).
;


#if defined(__A68K__)
#  define align4        align   0,4
#  define L(label)      \/**/label
#  define macro(name)   name    macro
#  define text          section code
#elif defined(__ASL__)
#  define align4        align   4
#  define L(label)      $$/**/label
#  define macro(name)   name    macro
#  define text          section code
#else
#  define align4        align   4
#  define L(label)      ./**/label
#  define macro(name)   macro   name
#endif

; defines needed for including ident_[ns].ash
#define db      dc.b
#define dw      dc.w
#define dd      dc.l


; basepage offsets
p_lowtpa        equ     $0      ; .l    pointer to self (bottom of TPA)
p_hitpa         equ     $4      ; .l    pointer to top of TPA + 1
p_tbase         equ     $8      ; .l    base of text segment
p_tlen          equ     $c      ; .l    length of text segment
p_dbase         equ     $10     ; .l    base of data segment
p_dlen          equ     $14     ; .l    length of data segment
p_bbase         equ     $18     ; .l    base of BSS segment
p_blen          equ     $1c     ; .l    length of BSS segment
p_dta           equ     $20     ; .l    pointer to current DTA
p_parent        equ     $24     ; .l    pointer to parent's basepage
p_flags         equ     $28     ; .l    memory usage flags
p_env           equ     $2c     ; .l    pointer to environment string

;
; long living registers:
;   d4  p_tbase - start of text segment
;   a6  p_bbase - start of decompressed bss segment, this also is the
;                     - end of decompressed text+data
;                     - start of decompressed relocations
;                     - start of dirty bss
;   ASTACK (a7) - final startup code copied below stack
;


; /*************************************************************************
; // flush cache macros
; **************************************************************************/

; note:
;   GEMDOS/XBIOS trashes d0, d1, d2, a0, a1, a2


; Ssystem(S_FLUSHCACHE, base, length) - inside the kernel this
; is called `cpush(base, length)'.
;   returns: d0.l should be either 0 or -32 (== ENOSYS == EINVFN)
; Available since FreeMiNT 1.15.1 (1999-04-13).
;
; Note that on a 68060 FreeMiNT just uses `cpusha bc' in all cases,
; so we don't bother passing base and length. (info: base would be d4)

macro(MINT_FLUSH_CACHE)
                pea     -1              ; length
                clr.l   -(sp)           ; base
#if 0
                move.w  #$0016,-(sp)    ; S_FLUSHCACHE (22)
                move.w  #$0154,-(sp)    ; Ssystem (340)
#else
                move.l  #$01540016,-(sp)
#endif
                trap    #1              ; GEMDOS
                lea     12(sp),sp
        endm


; First try `cpusha bc' (68040/68060). If that fails try temporary changing
; the cache control register (68030).

macro(SUPEXEC_FLUSH_CACHE)
                pea     \@super(pc)
                move.w  #38,-(sp)       ; Supexec
                trap    #14             ; XBIOS
                addq.l  #6,sp
                bra     \@done


; exception handler
\@exception:    move.l  a1,sp           ; restore stack (SSP)
                jmp     (a0)            ; and continue


\@super:        move.l  ($10),-(sp)
                move.l  ($2c),-(sp)
                move.l  ($f4),-(sp)
                move.l  sp,a1           ; save stack pointer (SSP)

        ; set exception vectors
                lea     \@exception(pc),a0
                move.l  a0,($10)
                move.l  a0,($2c)
                move.l  a0,($f4)
                nop                     ; flush write pipeline

        ; try 68040 / 68060
                lea     \@1(pc),a0
                dc.w    $f4f8           ; cpusha bc
                bra     \@ret
\@1:
        ; try 68030
                lea     \@2(pc),a0
                dc.l    $4e7a0002       ; movec.l cacr,d0
                move.l  d0,d1
                or.w    #$0808,d1
                dc.l    $4e7b1002       ; movec.l d1,cacr
                dc.l    $4e7b0002       ; movec.l d0,cacr
;;;                bra     \@ret
\@2:

\@ret:          move.l  (sp)+,($f4)
                move.l  (sp)+,($2c)
                move.l  (sp)+,($10)
                nop                     ; flush write pipeline
                rts

\@done:
        endm



macro(BOTH_FLUSH_CACHE)
                MINT_FLUSH_CACHE
                tst.w   d0
                beq     \@done
                SUPEXEC_FLUSH_CACHE
\@done:
        endm



#define ASTACK          a7

#if 1
#  define FLUSH_CACHE   BOTH_FLUSH_CACHE
#elif 0
#  define FLUSH_CACHE   MINT_FLUSH_CACHE
#else
#  undef FLUSH_CACHE
#endif


; /*************************************************************************
; // entry - the text segment of a compressed executable
; //
; // note: compressed programs never have the F_SHTEXT flag set,
; //       so we can assume that the text, data & bss segments
; //       are contiguous in memory
; **************************************************************************/

#if defined(__ASL__)
                padding off
#endif
                text
                dc.b    'UPX1'          ; marker for o2bin.pl

start:
                move.l  a0,d0           ; a0 is basepage if accessory
                beq     L(l_app)
                move.l  4(a0),sp        ; accessory - get stack
                bra     L(start)

L(l_app):       move.l  4(sp),d0        ; application - get basepage
L(start):       movem.l d1-d7/a0-a6,-(sp)


; ------------- restore original basepage

        ; we also setup d4 and a6 here, and we prepare a4

                move.l  d0,a2           ; a2 = basepage
                addq.l  #p_tbase,a2
                move.l  (a2)+,a6
                move.l  a6,d4                   ; d4 = p_tbase
                move.l  #'up11',(a2)    ; p_tlen
                add.l   (a2)+,a6
                move.l  a6,(a2)+        ; p_dbase
                move.l  #'up12',(a2)    ; p_dlen
                add.l   (a2)+,a6                ; a6 = decompressed p_bbase
                move.l  (a2),a4                 ; a4 = compressed p_bbase
                move.l  a6,(a2)+        ; p_bbase
                move.l  #'up13',(a2)    ; p_blen


; ------------- copy data segment (from a4 to a3, downwards)

                ; a4 (top of compressed data) already initialized above

                move.l  d4,a3
                add.l   #'up21',a3      ; top of data segment + offset

#if defined(SMALL)

                move.l  #'up22',d0      ; (len / 4)

        ; copy 4 bytes per loop
L(loop):        move.l  -(a4),-(a3)
                ;;subq.l  #1,d0
                dc.b    'u1'            ; subq.l #1,d0 / subq.w #1,d0
                bne     L(loop)

#else

                move.l  #'up22',d0      ; (len / 160)

        ; loop1 - use 10 registers to copy 4*10*4 = 160 bytes per loop
L(loop1):
                lea.l   -160(a4),a4
                movem.l 120(a4),d1-d3/d5-d7/a0-a2/a5
                movem.l d1-d3/d5-d7/a0-a2/a5,-(a3)
                movem.l 80(a4),d1-d3/d5-d7/a0-a2/a5
                movem.l d1-d3/d5-d7/a0-a2/a5,-(a3)
                movem.l 40(a4),d1-d3/d5-d7/a0-a2/a5
                movem.l d1-d3/d5-d7/a0-a2/a5,-(a3)
                movem.l (a4),d1-d3/d5-d7/a0-a2/a5
                movem.l d1-d3/d5-d7/a0-a2/a5,-(a3)
                ;;subq.l  #1,d0
                dc.b    'u1'            ; subq.l #1,d0 / subq.w #1,d0
                bne     L(loop1)

        ; loop2 - copy the remaining 4..160 bytes
                ;;moveq.l #xx,d0          ; ((len % 160) / 4) - 1
                dc.b    'u2'            ; moveq.l #xx,d0

L(loop2):       move.l  -(a4),-(a3)
                dbra    d0,L(loop2)

#endif

        ; a3 now points to the start of the compressed block


; ------------- copy code to stack and setup ASTACK

; Copy the final startup code below the stack. This will get
; called via "jmp (ASTACK)" after decompression and relocation.

copy_to_stack:

                lea.l   clear_bss_end(pc),a2
                move.l  d4,-(ASTACK)    ; entry point for final jmp

                moveq.l #((clear_bss_end-clear_bss)/2-1),d5
L(loop):        move.w  -(a2),-(ASTACK)
                subq.l  #1,d5
                bcc     L(loop)

#ifdef FLUSH_CACHE
                ; patch code: on the stack, the `rts' becomes a `nop'
                move.w #$4e71,flush_cache_rts-clear_bss(ASTACK)
#endif

        ; note: d5.l is now -1 (needed for decompressor)


; -------------

#ifdef FLUSH_CACHE
                bsr     flush_cache
#endif


; ------------- prepare decompressor

        ; a3 still points to the start of the compressed block
                move.l  d4,a4           ; dest. for decompressing

#define NRV_NO_INIT

                ;;moveq.l #-1,d5        ; last_off = -1
                moveq.l #-128,d0        ; d0.b = $80
#if defined(NRV2B)
                moveq.l #-1,d7
                moveq.l #-$68,d6        ; 0xffffff98
                lsl.w   #5,d6           ; 0xfffff300 == -0xd00
#elif defined(NRV2D)
                moveq.l #-1,d7
                moveq.l #-$50,d6        ; 0xffffffb0
                lsl.w   #4,d6           ; 0xfffffb00 == -0x500
#elif defined(NRV2E)
                moveq.l #0,d7
                moveq.l #-$50,d6        ; 0xffffffb0
                lsl.w   #4,d6           ; 0xfffffb00 == -0x500
#endif


; ------------- jump to copied decompressor

                move.l  d4,a2
                add.l   #'up31',a2
                jmp     (a2)            ; jmp decompr_start


; /*************************************************************************
; // this is the final part of the startup code which runs in the stack
; **************************************************************************/

; ------------- clear dirty bss

clear_bss:

        ; on entry:
        ;   ASTACK      == pc == clear_bss (on stack)
        ;   a6          start of dirty bss [long living register]
        ;   d6.l        number of clr loops
        ;   d3.l        0


#if defined(SMALL)
L(loop):        move.l  d3,(a6)+
                ;;subq.l  #1,d6
                dc.b    'u4'            ; subq.l #1,d6 / subq.w #1,d6
                bne     L(loop)
#else
        ; the dirty bss is usually not too large, so we don't
        ; bother making movem optimizations here
L(loop):        move.l  d3,(a6)+
                move.l  d3,(a6)+
                move.l  d3,(a6)+
                move.l  d3,(a6)+
                ;;subq.l  #1,d6
                dc.b    'u4'            ; subq.l #1,d6 / subq.w #1,d6
                bne     L(loop)
#endif


; ------------- flush the cache

#ifdef FLUSH_CACHE

; info:
;  This is also called as a subroutine (before decompression, NOT running
;  in the stack). When running in the stack the `rts' is replaced by a `nop'.
;
flush_cache:
                FLUSH_CACHE
flush_cache_rts:
                rts

#endif


; ------------- restore ASTACK

                lea     clear_bss_end-clear_bss+4(ASTACK),sp

        ;; assert sp == clear_bss_end(pc)+4


; ------------- clear the dirty stack

#if 0
;;; /* 0 || defined(FLUSH_CACHE) */

; better don't do this - we are currently running in the stack
; and don't want to make yet another instruction-cache-line dirty

clear_dirty_stack:

                ; clear down to clear_bss(pc) + 32 extra longs
                moveq.l #((L(loop)-clear_bss+3)/4+32-1),d0
                lea     L(loop)(pc),a0
L(loop):        move.l  d3,-(a0)
                dbra    d0,L(loop)

#endif


; ------------- start program

                movem.l (sp)+,d1-d7/a0-a6
                move.l  a0,d0
                beq     L(l_app)
                sub.l   sp,sp           ; accessory: no stack
L(l_app):       dc.w    $4ef9           ; jmp $xxxxxxxx - jmp to text segment

clear_bss_end:


; /*************************************************************************
; // UPX ident & packheader
; **************************************************************************/

#if defined(SMALL)
#  include "ident_s.ash"
#else
#  include "ident_n.ash"
#endif

                align4

                ; 32 bytes - #include "header.ash"
                dc.b    85,80,88,33     ;     UPX_MAGIC_LE32
                dc.b    161,216,208,213 ;     UPX_MAGIC2_LE32
                dc.l    0,0,0,0,0
                dc.b    0,0,0,45


        ; end of text segment - size is a multiple of 4


; /*************************************************************************
; // This part is appended after the compressed data.
; // It runs in the last part of the dirty bss (after the
; // relocations and the original fileheader).
; **************************************************************************/

cutpoint:

; ------------- decompress (from a3 to a4)

#define a0 A3
#define a1 A4
#define a3 A2
#define d2 D3

#if defined(NRV2B)
#  include <m68k/n2b_d.ash>
#elif defined(NRV2D)
#  include <m68k/n2d_d.ash>
#elif defined(NRV2E)
#  include <m68k/n2e_d.ash>
#else
#  error
#endif

#undef a0
#undef a1
#undef a3
#undef d2

        ; note: d3.l is 0 from decompressor above


; ------------- prepare d6 for clearing the dirty bss

#if defined(SMALL)
                move.l  #'up41',d6      ; dirty_bss / 4
#else
                move.l  #'up41',d6      ; dirty_bss / 16
#endif


; ------------- test if we need to reloc

                dc.b    'u3'            ; moveq.l #1,d5 / jmp (ASTACK)


; ------------- reloc

reloc:

; The decompressed relocations now are just after the decompressed
; data segment, i.e. at the beginning of the (dirty) bss.

        ; note: d3.l is still 0

                move.l  a6,a0           ; a0 = start of relocations

                move.l  d4,a1
                add.l   (a0)+,a1        ; get initial fixup

L(loop1):       add.l   d3,a1           ; increase fixup
                add.l   d4,(a1)         ; reloc one address
L(loop2):       move.b  (a0)+,d3
                beq     reloc_end
                cmp.b   d5,d3           ; note: d5.b is #1 from above
                bne     L(loop1)
                lea     254(a1),a1      ; d3 == 1 -> add 254, don't reloc
                bra     L(loop2)

reloc_end:


; ------------- clear dirty bss & start program

; We are currently running in the dirty bss.
; Jump to the code we copied below the stack.

        ; note: d3.l is still 0

                jmp     (ASTACK)        ; jmp clear_bss (on stack)



eof:
                dc.w    cutpoint-start  ; size of entry
                dc.w    eof-cutpoint    ; size of decompressor
                dc.w    decompr_start-cutpoint  ; offset of decompressor start
                dc.b    'UPX9'          ; marker for o2bin.pl

#if defined(__ASL__)
                endsection code
#endif
                end


; vi:ts=8:et:nowrap

