;  l_armwce.asm -- loader & decompressor for the arm/wince/pe format
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


%define         jmps    jmp short
%define         jmpn    jmp near
%include        "macros.ash"

                BITS    32
                SECTION .text
                ORG     0
                CPU     386

; =============
; ============= ENTRY POINT
; =============

;       __ARMPEAXE__
%include        "l_armpe_axe.ah"
;       __ARMPEADE__
%include        "l_armpe_ade.ah"
;       __ARMPETXE__
%include        "l_armpe_txe.ah"
;       __ARMPETDE__
%include        "l_armpe_tde.ah"
;       __ARMPETXB__
%include        "l_armpe_txb.ah"
;       __ARMPETDB__
%include        "l_armpe_tdb.ah"
;       __ARMPEHEAD__
%include        "header.ash"
eof:
;       __ARMPEEOF__
                section .data
                dd      -1
                dw      eof


; vi:ts=8:et:nowrap
