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


#ifdef __A68K__
#  define align4        align   0,4
#  define L(label)      \/**/label
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
p_lowtpa        equ     $0      ; .l
p_hitpa         equ     $4      ; .l
p_tbase         equ     $8      ; .l
p_tlen          equ     $c      ; .l
p_dbase         equ     $10     ; .l
p_dlen          equ     $14     ; .l
p_bbase         equ     $18     ; .l
p_blen          equ     $1c     ; .l

#if 0
; file header offsets (NOT USED)
fh_branch       equ     $0      ; .w   $601a
fh_tlen         equ     $2      ; .l
fh_dlen         equ     $6      ; .l
fh_blen         equ     $a      ; .l
fh_slen         equ     $e      ; .l
fh_res1         equ     $12     ; .l
fh_res2         equ     $16     ; .l
fh_flag         equ     $1a     ; .w

fh_size         equ     $1c     ; 28 bytes
#endif

;
; long living registers:
;   d4  p_tbase - start of text segment
;   a6  p_bbase - start of uncompressed bss segment, this also is the
;                     - end of decompressed text+data
;                     - beginning of decompressed relocations
;                     - beginning of dirty bss
;   a5  final startup code copied below stack
;


; /*************************************************************************
; // entry - the text segment of a compressed executable
; //
; // note: compressed programs never have the F_SHTEXT flag set,
; //       so we can assume that the text, data & bss segments
; //       are contiguous in memory
; **************************************************************************/

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
                add.l   (a2)+,a6                ; a6 = uncompressed p_bbase
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


; ------------- copy code to stack

; Copy the final startup code below the stack. This will get
; called via "jmp (a5)" after decompression and relocation.

copy_to_stack:
                lea.l   clear_bss_end(pc),a2
                move.l  sp,a5
                moveq.l #((clear_bss_end-clear_bss)/2),d0

                move.l  d4,-(a5)        ; entry point for final jmp
L(loop):        move.w  -(a2),-(a5)
                subq.w  #1,d0
                bne     L(loop)

        ; note: now d0 is 0


; ------------- prepare decompressor

        ; a0 now points to the start of the compressed block
                ; note: the next statement can be moved below cutpoint
                ;   if it helps for the align4
                ;;move.l  d4,a1           ; dest. for uncompressing
                move.l  d4,a1           ; dest. for uncompressing


; ------------- jump to copied decompressor

                move.l  d4,a2
                add.l   #'up31',a2
                jmp     (a2)            ; jmp cutpoint


; /*************************************************************************
; // this is the final part of the startup code which runs in the stack
; **************************************************************************/

        ; on entry d1 and d2 are 0

; ------------- clear dirty bss

clear_bss:

#if defined(SMALL)
L(loop):        move.l  d1,(a6)+
                ;;subq.l  #1,d0
                dc.b    'u4'            ; subq.l #1,d0 / subq.w #1,d0
                bne     L(loop)
#else
        ; the dirty bss is usually not too large, so we don't
        ; bother making movem optimizations here
L(loop):        move.l  d1,(a6)+
                move.l  d1,(a6)+
                move.l  d1,(a6)+
                move.l  d1,(a6)+
                ;;subq.l  #1,d0
                dc.b    'u4'            ; subq.l #1,d0 / subq.w #1,d0
                bne     L(loop)
#endif


; ------------- start program

        ; note: d0.l is now 0

                movem.l (sp)+,d1-d7/a0-a6
                cmp.l   d0,a0
                beq     L(l_app)
                ;;suba.l  sp,sp           ; accessory: no stack
                move.l  d0,sp           ; accessory: no stack
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
                even

                align4

                dc.b    'UPX!'          ; magic
                ds.b    28              ; #include "header.ash"


        ; end of text segment - size is a multiple of 4


; /*************************************************************************
; // This part is appended after the compressed data.
; // It runs in the last part of the dirty bss (after the relocations).
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

        ; note: d1 and d2 are 0 from decompressor above


; ------------- prepare d0 for clearing the dirty bss

#if defined(SMALL)
                move.l  #'up41',d0      ; dirty_bss / 4
#else
                move.l  #'up41',d0      ; dirty_bss / 16
#endif


; ------------- test if we need to reloc

                dc.b    'u3'            ; jmp (a5) / moveq.l #1,d3


; ------------- reloc

; The decompressed relocations now are just after the decompressed
; data segment, i.e. at the beginning of the (dirty) bss.

        ; note: d1 and d2 are still 0

                move.l  a6,a0           ; a0 = start of relocations

                move.l  d4,a1
                add.l   (a0)+,a1        ; get initial fixup

L(loop1):       add.l   d1,a1           ; increase fixup
                add.l   d4,(a1)         ; reloc one address
L(loop2):       move.b  (a0)+,d1
                beq     reloc_end
                cmp.b   d3,d1           ; note: d3.b is #1
                bne     L(loop1)
                lea     254(a1),a1      ; d1 == 1 -> add 254, don't reloc
                bra     L(loop2)

reloc_end:


; ------------- clear dirty bss & start program

; We are currently running in the dirty bss.
; Jump to the code we copied below the stack.

        ; note: d1 and d2 are still 0

                jmp     (a5)            ; jmp clear_bss (on stack)



eof:
                dc.w    cutpoint-start  ; size of entry
                dc.w    eof-cutpoint    ; size of decompressor
                dc.b    'UPX9'          ; marker for o2bin.pl

                end


; vi:ts=8:et:nowrap

