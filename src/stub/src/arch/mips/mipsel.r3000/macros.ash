;  macros.ash -- macros
;
;  This file is part of the UCL data compression library.
;
;  Copyright (C) 1996-2006 Markus Franz Xaver Johannes Oberhumer
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
;  Markus F.X.J. Oberhumer              Jens Medoch
;  <markus@oberhumer.com>               <jssg@users.sourceforge.net>
;  http://www.oberhumer.com/opensource/ucl/
;

; Load Address macro

la MACRO reg,addr
        lui     reg,(addr >> 16)+((addr >> 15)&1)
        addiu   reg,reg,((addr & 0FFFFh))
la ENDM

; Load Immidiate macro

li MACRO reg,imm
IF (imm == -1)
        addiu    reg, zero, -1
ELSE
        IF ((imm>>16)!=0)
                lui     reg,imm >> 16
                ori     reg,reg,(imm & 0FFFFh)
        ELSE
                IF ((imm&0xffff)>0x8000)
                        ori     reg,zero,(imm & 0FFFFh)
                ELSE
                        addiu   reg,zero,(imm & 0FFFFh)
                ENDIF
        ENDIF
ENDIF
li ENDM
