/*
;  upx_itoa.S -- decimal print; smaller than gcc, and no relocations
;
;  This file is part of the UPX executable compressor.
;
;  Copyright (C) 2002-2008 John F. Reiser
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
;  John F. Reiser
;  <jreiser@users.sourceforge.net>
;
*/


        .globl upx_itoa

upx_itoa:  // char *upx_itoa(eax= unsigned v, edx= char *buf)  /* 0<=(int)v */
        push edi  // save register
        mov edi,edx  // output ptr
        push 10
        cld
        pop ecx  // radix
        call recur
        mov [edi],ah  // NUL terminate
        xchg eax,edi  // eax= continuation point
        pop edi  // restore register
        ret
recur:
        cdq  // zero extend eax into edx [use "sub edx,edx" if eax < 0 ]
        div ecx  // eax=quo, edx=rem;  flags are undefined
        push edx
        test eax,eax
        je quo0
        call recur
quo0:
        pop eax  // remainder
        add al, '0'
        stosb
        ret


// vi:ts=8:et:nowrap
