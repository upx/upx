;  header.ash --
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
;  Copyright (C) 1996-2004 Laszlo Molnar
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


; ------------- HEADER -------------    ;     __UPX1HEAD__

                db      85,80,88,33     ;  0  UPX_MAGIC_LE32
 %if 0
                db      0               ;  4  version
                db      0               ;  5  format
                db      0               ;  6  compression method
                db      0               ;  7  compression level
 %else
        ; another magic for PackHeader::putPackHeader
                db      161,216,208,213 ;     UPX_MAGIC2_LE32
 %endif
                dd      0               ;  8  uncompressed adler32
                dd      0               ; 12  compressed adler32

 %ifdef COM
                dw      0               ; 16  uncompressed len
                dw      0               ; 18  compressed len
                db      0               ; 20  filter
                db      45              ; 21  header checksum
 %elifdef EXE
                db      0,0,0           ; 16  uncompressed len
                db      0,0,0           ; 19  compressed len
                db      0,0,0           ; 22  original file size
                db      0               ; 25  filter
                db      45              ; 26  header checksum
 %else
                dd      0               ; 16  uncompressed len
                dd      0               ; 20  compressed len
                dd      0               ; 24  original file size
                db      0               ; 28  filter id
                db      0               ; 29  filter cto
                db      0               ;  unused
                db      45              ; 31  header checksum
 %endif


; vi:ts=8:et:nowrap
