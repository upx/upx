;  l_tos.s -- loader & decompressor for the atari/tos format
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2000 Laszlo Molnar
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
;  Markus F.X.J. Oberhumer                   Laszlo Molnar
;  markus.oberhumer@jk.uni-linz.ac.at        ml1050@cdata.tvnet.hu
;


#define NRV_BB  8


;
; see also:
;   mint/src/basepage.h
;   mint/src/mem.h       (FILEHEAD)
;   mint/src/mem.c       (load_region, load_and_reloc)
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

#if 0
; file header offsets (NOT USED)
fh_magic        equ     $0      ; .w    $601a
fh_text         equ     $2      ; .l
fh_data         equ     $6      ; .l
fh_bss          equ     $a      ; .l
fh_sym          equ     $e      ; .l
fh_reserved     equ     $12     ; .l
fh_flag         equ     $16     ; .l
fh_reloc        equ     $1a     ; .w
FH_SIZE         equ     $1c     ;       28 bytes
#endif

;
; long living registers:
;   d4  p_tbase - start of text segment
;   a6  p_bbase - start of decompressed bss segment, this also is the
;                     - end of decompressed text+data
;                     - start of decompressed relocations
;                     - start of dirty bss
;   a5  final startup code copied below stack
;


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

        ; we also setup d4, a6 and a1 here

                move.l  d0,a2           ; a2 = basepage
                addq.l  #p_tbase,a2
                move.l  (a2)+,a6
                move.l  a6,d4                   ; d4 = p_tbase
                move.l  #'up11',(a2)    ; p_tlen
                add.l   (a2)+,a6
                move.l  a6,(a2)+        ; p_dbase
                move.l  #'up12',(a2)    ; p_dlen
                add.l   (a2)+,a6                ; a6 = decompressed p_bbase
                move.l  (a2),a1                 ; a1 = compressed p_bbase
                move.l  a6,(a2)+        ; p_bbase
                move.l  #'up13',(a2)    ; p_blen


; ------------- copy data segment (from a1 to a0, downwards)

                ; a1 (top of compressed data) already initialized above

                move.l  d4,a0
                add.l   #'up21',a0      ; top of data segment + offset

#if defined(SMALL)

                move.l  #'up22',d0      ; (len / 4)

        ; copy 4 bytes per loop
L(loop):        move.l  -(a1),-(a0)
                ;;subq.l  #1,d0
                dc.b    'u1'            ; subq.l #1,d0 / subq.w #1,d0
                bne     L(loop)

#else

                move.l  #'up22',d0      ; (len / 160)

        ; loop1 - use 10 registers to copy 4*10*4 = 160 bytes per loop
L(loop1):
                lea.l   -160(a1),a1
                movem.l 120(a1),d1-d3/d5-d7/a2-a5
                movem.l d1-d3/d5-d7/a2-a5,-(a0)
                movem.l 80(a1),d1-d3/d5-d7/a2-a5
                movem.l d1-d3/d5-d7/a2-a5,-(a0)
                movem.l 40(a1),d1-d3/d5-d7/a2-a5
                movem.l d1-d3/d5-d7/a2-a5,-(a0)
                movem.l (a1),d1-d3/d5-d7/a2-a5
                movem.l d1-d3/d5-d7/a2-a5,-(a0)
                ;;subq.l  #1,d0
                dc.b    'u1'            ; subq.l #1,d0 / subq.w #1,d0
                bne     L(loop1)

        ; loop2 - copy the remaining 4..160 bytes
                ;;moveq.l #xx,d0          ; ((len % 160) / 4) - 1
                dc.b    'u2'            ; moveq.l #xx,d0

L(loop2):       move.l  -(a1),-(a0)
                dbra    d0,L(loop2)

#endif

        ; a0 now points to the start of the compressed block


; ------------- copy code to stack and setup a5

; Copy the final startup code below the stack. This will get
; called via "jmp (a5)" after decompression and relocation.

copy_to_stack:
                lea.l   clear_bss_end(pc),a2
                move.l  sp,a5
                moveq.l #((clear_bss_end-clear_bss)/2-1),d5

                move.l  d4,-(a5)        ; entry point for final jmp
L(loop):        move.w  -(a2),-(a5)
                subq.l  #1,d5
                bcc     L(loop)

        ; note: d5.l is now -1 (needed for decompressor)


; ------------- prepare decompressor

        ; a0 still points to the start of the compressed block
                move.l  d4,a1           ; dest. for decompressing

#define NRV_NO_INIT
                ;;moveq.l #-1,d5        ; last_off = -1
                moveq.l #-1,d7
                moveq.l #-128,d0        ; d0.b = $80
#if defined(NRV2B)
                moveq.l #-$68,d6        ; 0xffffff98
                lsl.w   #5,d6           ; 0xfffff300 == -0xd00
#elif defined(NRV2D)
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

        ; on entry d2 is 0

#if defined(SMALL)
L(loop):        move.l  d2,(a6)+
                ;;subq.l  #1,d0
                dc.b    'u4'            ; subq.l #1,d0 / subq.w #1,d0
                bne     L(loop)
#else
        ; the dirty bss is usually not too large, so we don't
        ; bother making movem optimizations here
L(loop):        move.l  d2,(a6)+
                move.l  d2,(a6)+
                move.l  d2,(a6)+
                move.l  d2,(a6)+
                ;;subq.l  #1,d0
                dc.b    'u4'            ; subq.l #1,d0 / subq.w #1,d0
                bne     L(loop)
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

                dc.b    'UPX!'          ; magic
                dc.l    0,0,0,0,0,0,0   ; 28 bytes - #include "header.ash"


        ; end of text segment - size is a multiple of 4


; /*************************************************************************
; // This part is appended after the compressed data.
; // It runs in the last part of the dirty bss (after the
; // relocations and the original fileheader).
; **************************************************************************/

cutpoint:

; ------------- decompress (from a0 to a1)

#if defined(NRV2B)
#  include "m68k/n2b_d.ash"
#elif defined(NRV2D)
#  include "m68k/n2d_d.ash"
#else
#  error
#endif

        ; note: d2 is 0 from decompressor above


; ------------- prepare d0 for clearing the dirty bss

#if defined(SMALL)
                move.l  #'up41',d0      ; dirty_bss / 4
#else
                move.l  #'up41',d0      ; dirty_bss / 16
#endif


; ------------- test if we need to reloc

                dc.b    'u3'            ; moveq.l #1,d3 / jmp (a5)


; ------------- reloc

reloc:

; The decompressed relocations now are just after the decompressed
; data segment, i.e. at the beginning of the (dirty) bss.

        ; note: d2 is still 0

                move.l  a6,a0           ; a0 = start of relocations

                move.l  d4,a1
                add.l   (a0)+,a1        ; get initial fixup

L(loop1):       add.l   d2,a1           ; increase fixup
                add.l   d4,(a1)         ; reloc one address
L(loop2):       move.b  (a0)+,d2
                beq     reloc_end
                cmp.b   d3,d2           ; note: d3.b is #1
                bne     L(loop1)
                lea     254(a1),a1      ; d2 == 1 -> add 254, don't reloc
                bra     L(loop2)

reloc_end:


; ------------- clear dirty bss & start program

; We are currently running in the dirty bss.
; Jump to the code we copied below the stack.

        ; note: d2 is still 0

                jmp     (a5)            ; jmp clear_bss (on stack)



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

