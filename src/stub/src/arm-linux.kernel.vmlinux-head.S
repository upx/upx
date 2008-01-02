/*
;  arm-linux.kernel.vmlinux-head.S -- set up stack for vmlinux/arm format
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 2006-2008 John Reiser
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
;  John Reiser
;  <jreiser@users.sourceforge.net>
*/

        mov ip,lr  // ip= retaddr
        bl LINUX000  // lr= PIC location of compressed data
// Compressed data appears >here<, then decompressor.

// vi:ts=8:et:nowrap
