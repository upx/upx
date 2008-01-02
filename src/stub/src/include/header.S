/*
;  header.S --
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


section         UPX1HEAD

                .byte   85,80,88,33     //  0  UPX_MAGIC_LE32
                .byte   161,216,208,213 //     UPX_MAGIC2_LE32
                .long   0               //  8  uncompressed adler32
                .long   0               // 12  compressed adler32

#ifdef COM
                .short  0               // 16  uncompressed len
                .short  0               // 18  compressed len
                .byte   0               // 20  filter
                .byte   45              // 21  header checksum
#elif defined(EXE)
                .byte   0,0,0           // 16  uncompressed len
                .byte   0,0,0           // 19  compressed len
                .byte   0,0,0           // 22  original file size
                .byte   0               // 25  filter
                .byte   45              // 26  header checksum
#else
                .long   0               // 16  uncompressed len
                .long   0               // 20  compressed len
                .long   0               // 24  original file size
                .byte   0               // 28  filter id
                .byte   0               // 29  filter cto
                .byte   0               //  unused
                .byte   45              // 31  header checksum
#endif


// vi:ts=8:et:nowrap
