/* arm-linux.elf-fold.lds --

   This file is part of the UPX executable compressor.

   Copyright (C) 2000-2008 John F. Reiser
   All Rights Reserved.

   UPX and the UCL library are free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer              Laszlo Molnar
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>

   John F. Reiser
   <jreiser@users.sourceforge.net>
 */


OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
/*ENTRY(_start)*/
PHDRS
{
    text PT_LOAD FILEHDR PHDRS ;
    data PT_LOAD ;  /* for setting brk(0) */
}
SECTIONS
{
  /*  0x00004000: below 0x00008000 usual, enough to avoid icache sync */
  . = 0x00008000 + SIZEOF_HEADERS + 12;  /* 12==sizeof(l_info) */
  .text : {
    *(.text)
    *(.data)
  } : text
  .data : {
  } : data
}
