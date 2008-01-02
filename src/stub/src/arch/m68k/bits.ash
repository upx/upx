/*
;  bits.ash -- bit access for decompression
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
*/


// ------------- ADDBITS -------------

macro(ADDBITS)
#if (NRV_BB == 8)
                add.b   d0,d0           // sets Z, C and X      //  4
#elif (NRV_BB == 32)
                add.l   d0,d0           // sets Z, C and X      //  6
#endif
        endm


#if 0
macro(ADDXBITS)
#if (NRV_BB == 8)
                addx.b   d0,d0          // sets C and X         //  4
#elif (NRV_BB == 32)
                addx.l   d0,d0          // sets C and X         //  8
#endif
        endm
#endif


// ------------- FILLBYTES_xx -------------

#undef FILLBYTES_SR

// get 1 byte// then get 1 bit into both C and X
macro(FILLBYTES_8)
        // note: we shift the X flag through -> must init d0.b with $80
                move.b  (a0)+,d0                                //  8
                addx.b  d0,d0           // sets C and X         //  4
        endm


// get 32 bits in little endian format// then get 1 bit into both C and X
macro(FILLBYTES_LE32)
#if 0
                move.b  (a0)+,d0                                //  8
                ror.l   #8,d0                                   // 24
                move.b  (a0)+,d0                                //  8
                ror.l   #8,d0                                   // 24
                move.b  (a0)+,d0                                //  8
                ror.l   #8,d0                                   // 24
                move.b  (a0)+,d0                                //  8
                ror.l   #8,d0                                   // 24
                add.l   d0,d0           // sets C and X         //  6
                bset    #0,d0           // only changes Z       // 12
                                                           //    -----
                                                           //     146
#elif 1
                move.b  3(a0),d0                                // 12
                lsl.w   #8,d0                                   // 22
                move.b  2(a0),d0                                // 12
                swap    d0                                      //  4
                move.b  1(a0),d0                                // 12
                lsl.w   #8,d0                                   // 22
                move.b  (a0),d0                                 //  8
                addq.l  #4,a0           // does not affect flags//  8
                add.l   d0,d0           // sets C and X         //  6
                bset    #0,d0           // only changes Z       // 12
                                                           //    -----
                                                           //     118
#elif 1
        // note: we shift the X flag through -> must init d0.l with $80000000
        // note: rol/ror do not change X flag (but asl/asr/lsl/lsr do)
                move.b  3(a0),d0                                // 12
                ror.w   #8,d0                                   // 22
                move.b  2(a0),d0                                // 12
                swap    d0                                      //  4
                move.b  1(a0),d0                                // 12
                ror.w   #8,d0                                   // 22
                move.b  (a0),d0                                 //  8
                addq.l  #4,a0           // does not affect flags//  8
                addx.l  d0,d0           // sets C and X         //  8
                                                           //    -----
                                                           //     108
#else
        // IMPORTANT: movep is not implemented on the 68060
#  error "do not use movep"
        // note: we shift the X flag through -> must init d0.l with $80000000
        // note: must use dc.l because of a bug in the pasm assembler
        // note: may access past the end of the input// this is ok for UPX
                dc.l    $01080003       // movep.w 3(a0),d0     // 16
                move.b  2(a0),d0                                // 12
                swap    d0                                      //  4
                dc.l    $01080001       // movep.w 1(a0),d0     // 16
                move.b  (a0),d0                                 //  8
                addq.l  #4,a0           // does not affect flags//  8
                addx.l  d0,d0           // sets C and X         //  8
                                                           //    -----
                                                           //      72
#endif
        endm


// ------------- FILLBITS -------------

macro(FILLBITS)
#if (NRV_BB == 8)
                // no need for a subroutine
                FILLBYTES_8
#elif (NRV_BB == 32)
# ifdef SMALL
#  define FILLBYTES_SR FILLBYTES_LE32
                bsr     fillbytes_sr                            // 18
# else
                FILLBYTES_LE32
# endif
#endif
        endm


// ------------- GETBIT -------------

// get one bit into both the Carry and eXtended flag
macro(GETBIT)
#if defined(__A68K__)
                ADDBITS                                         //  4 / 6
                bnes    \@                                      // 10 (if jump)
                FILLBITS
\@:
#elif defined(__ASL__)
                ADDBITS                                         //  4 / 6
                bnes    done                                    // 10 (if jump)
                FILLBITS
done:
#else
LOCAL done
                ADDBITS                                         //  4 / 6
                bnes    done                                    // 10 (if jump)
                FILLBITS
done:
#endif
        endm



// vi:ts=8:et

