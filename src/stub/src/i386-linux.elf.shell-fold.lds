/* i386-linux.elf.shell-fold.lds --

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


OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
/*ENTRY(_start)*/
PHDRS
{
  phdr0 PT_LOAD FILEHDR PHDRS FLAGS(5);  /* no PF_W: strict SELinux, PaX, grSecurity */
  phdr1 PT_LOAD;  /* kernel-2.6.12-2.3.legacy_FC3 bug: SIGSEGV if no PF_W for ".bss" */
}

SECTIONS
{
  /*  0x01400000: avoid 0x01000000 for shell itself being compressed */
  . = 0x01400000 + SIZEOF_HEADERS;
  . = ALIGN(0x80);
  .data : { /* put everything together in one Phdr */
    *(.text)
    *(.rodata)
    *(.data)
    *(.bss)
    *(COMMON)
  } : phdr0
}
