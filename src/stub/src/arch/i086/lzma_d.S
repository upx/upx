/*
;  lzma_d.ash -- 16-bit assembly
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 2006-2007 Markus Franz Xaver Johannes Oberhumer
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
;  Markus F.X.J. Oberhumer
;  <markus@oberhumer.com>
;  http://www.oberhumer.com/opensource/upx/
;


; ------------- DECOMPRESSION -------------

; Input:
;   si - source
;   di - dest
;   cld

; Output:
*/

// init
section LZMA_DEC00


section LZMA_DEC10
.arch   i8086, nojumps
#include "lzma_d_cs.S"
section LZMA_DEC20
.arch   i8086, nojumps
#include "lzma_d_cf.S"

.arch   i8086, jumps

// cleanup
section LZMA_DEC30


// vi:ts=8:et

