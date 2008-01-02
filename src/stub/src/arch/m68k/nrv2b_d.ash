/*
;  n2b_d.ash -- NRV2B decompression in 68000 assembly
;
;  This file is part of the UCL data compression library.
;
;  Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
;  All Rights Reserved.
;
;  The UCL library is free software; you can redistribute it and/or
;  modify it under the terms of the GNU General Public License as
;  published by the Free Software Foundation; either version 2 of
;  the License, or (at your option) any later version.
;
;  The UCL library is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with the UCL library; see the file COPYING.
;  If not, write to the Free Software Foundation, Inc.,
;  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
;
;  Markus F.X.J. Oberhumer
;  <markus@oberhumer.com>
;  http://www.oberhumer.com/opensource/ucl/
;


; ------------- DECOMPRESSION -------------

; decompress from a0 to a1
;   note: must preserve d4 and a5-a7

;
; On entry:
;   a0  src pointer
;   a1  dest pointer
;
; On exit:
;   d1.l = 0
;   d2.l = 0
;
; Register usage:
;   a3  m_pos
;
;   d0  bit buffer
;   d1  m_off
;   d2  m_len
;   d5  last_m_off
;
;   d6  constant: -$d00
;   d7  constant: -1
;
;
; Notes:
;   we have max_match = 65535, so we can use word arithmetics on d2
;


; ------------- constants & macros -------------
*/

#if !defined(NRV_NO_INIT)

                //move.l  #-$d00,d6             // 0xfffff300
                moveq.l #-$68,d6                //   0xffffff98
                lsl.w   #5,d6                   //   << 5

                moveq.l #-1,d7
                moveq.l #-1,d5                  // last_off = -1

                // init d0 with high bit set
#if (NRV_BB == 8)
                //move.b  #$80,d0               // init d0.b for FILLBYTES
                moveq.l #-128,d0                // d0.b = $80
#elif (NRV_BB == 32)
                //move.l  #$80000000,d0         // init d0.l for FILLBYTES
                moveq.l #1,d0
                ror.l   #1,d0                   // d0.l = $80000000
#endif
                bra     decompr_start

#endif


#include "bits.ash"


#if defined(FILLBYTES_SR)
fillbytes_sr:   FILLBYTES_SR
                rts                                             // 16
#endif



// ------------- DECOMPRESSION -------------


decompr_literal:
                move.b  (a0)+,(a1)+

.globl decompr_start
decompr_start:
decompr_loop:
#ifdef SMALL
        //   cost literal:   4 + 10 + 10
        //   cost match:     4 + 10 +  8
        //   cost fillbits:  4 +  8
                GETBIT
                bcss    decompr_literal
#else
        // optimization: carry is clear -> we know that bits are available
        //   cost literal:   4 +  8 + 10
        //   cost match:     4 + 10
        //   cost fillbits:  4 +  8 +  8
                ADDBITS
                bccs    decompr_match
                bnes    decompr_literal
                FILLBITS
                bcss    decompr_literal
#endif



decompr_match:
                moveq.l #1,d1
                moveq.l #0,d2
decompr_l1:
                GETBIT
                addx.l  d1,d1
#ifdef SMALL
        //   cost loop continue:  4 + 10 + 10
        //   cost loop break:     4 + 10 +  8
        //   cost fillbits:       4 +  8
                GETBIT
                bccs    decompr_l1
#else
        // optimization: carry is clear -> we know that bits are available
        //   cost loop continue:  4 + 10
        //   cost loop break:     4 +  8 + 10
        //   cost fillbits:       4 +  8 +  8
                ADDBITS
                bccs    decompr_l1
                bnes    L(break)
                FILLBITS
                bccs    decompr_l1
L(break):
#endif
                subq.l  #3,d1
                bcss    decompr_get_mlen        // last m_off
                lsl.l   #8,d1
                move.b  (a0)+,d1
                not.l   d1
                beqs    decompr_end
                move.l  d1,d5



decompr_get_mlen:
                GETBIT
                addx.w  d2,d2
                GETBIT
                addx.w  d2,d2
                tst.w   d2              // addx doesn't set the Z flag...
                bnes    decompr_got_mlen
                addq.w  #1,d2

decompr_l2:     GETBIT
                addx.w  d2,d2
#ifdef SMALL
        //   cost loop continue:  4 + 10 + 10
        //   cost loop break:     4 + 10 +  8
        //   cost fillbits:       4 +  8
                GETBIT
                bccs    decompr_l2
#else
        // optimization: carry is clear -> we know that bits are available
        //   cost loop continue:  4 + 10
        //   cost loop break:     4 +  8 + 10
        //   cost fillbits:       4 +  8 +  8
                ADDBITS
                bccs    decompr_l2
                bnes    L(break2)
                FILLBITS
                bccs    decompr_l2
L(break2):
#endif
                addq.w  #2,d2



decompr_got_mlen:
                lea     0(a1,d5.l),a3

                // must use sub as cmp doesn't affect the X flag
                move.l  d5,d1
                sub.l   d6,d1
                addx.w  d7,d2

// TODO: partly unroll this loop; could use some magic with d7 for address
//       computations, then compute a nice `jmp yyy(pc,dx.w)'

#if 1
        //   cost for any m_len:   12 + 22 * (m_len - 1) + 4
        //     38, 60, 82, 104, 126, 148, 170, 192, 214, 236
                move.b  (a3)+,(a1)+                             // 12
L(copy):        move.b  (a3)+,(a1)+                             // 12
                dbra    d2,L(copy)                              // 10 / 14
#else
        //   cost for even m_len:  18 + 34 * (m_len / 2) + 4
        //   cost for odd m_len:   28 + 34 * (m_len / 2) + 4
        //     56, 66, 90, 100, 124, 134, 158, 168, 192, 202
                lsr.w   #1,d2                                   //  8
                bccs    L(copy)                                 // 10 /  8
                move.b  (a3)+,(a1)+                             // 12
L(copy):        move.b  (a3)+,(a1)+                             // 12
                move.b  (a3)+,(a1)+                             // 12
                dbra    d2,L(copy)                              // 10 / 14
#endif

                bras    decompr_loop



decompr_end:


// vi:ts=8:et

