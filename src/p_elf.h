/* p_elf.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2001 Laszlo Molnar
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

   Markus F.X.J. Oberhumer   Laszlo Molnar
   markus@oberhumer.com      ml1050@cdata.tvnet.hu
 */


#ifndef __UPX_P_ELF_H
#define __UPX_P_ELF_H


/*************************************************************************
// Some ELF type definitinons
**************************************************************************/

// The ELF file header. This appears at the start of every ELF file.
struct Elf_LE32_Ehdr
{
    unsigned char e_ident[16];  /* Magic number and other info */
    LE16 e_type;                /* Object file type */
    LE16 e_machine;             /* Architecture */
    LE32 e_version;             /* Object file version */
    LE32 e_entry;               /* Entry point virtual address */
    LE32 e_phoff;               /* Program header table file offset */
    LE32 e_shoff;               /* Section header table file offset */
    LE32 e_flags;               /* Processor-specific flags */
    LE16 e_ehsize;              /* ELF header size in bytes */
    LE16 e_phentsize;           /* Program header table entry size */
    LE16 e_phnum;               /* Program header table entry count */
    LE16 e_shentsize;           /* Section header table entry size */
    LE16 e_shnum;               /* Section header table entry count */
    LE16 e_shstrndx;            /* Section header string table index */
}
__attribute_packed;


// Program segment header.
struct Elf_LE32_Phdr
{
    LE32 p_type;                /* Segment type */
    LE32 p_offset;              /* Segment file offset */
    LE32 p_vaddr;               /* Segment virtual address */
    LE32 p_paddr;               /* Segment physical address */
    LE32 p_filesz;              /* Segment size in file */
    LE32 p_memsz;               /* Segment size in memory */
    LE32 p_flags;               /* Segment flags */
    LE32 p_align;               /* Segment alignment */

    // Values for p_type
    enum {
        PT_LOAD     = 1,    /* Loadable program segment */
        PT_DYNAMIC  = 2,    /* Dynamic linking information */
        PT_PHDR     = 6     /* Entry for header table itself */
    };

    // Values for p_flags
    enum { PF_X = (1 << 0) };   /* Segment is executable */
    enum { PF_W = (1 << 1) };   /* Segment is writable */
    enum { PF_R = (1 << 2) };   /* Segment is readable */
}
__attribute_packed;


struct Elf_LE32_Dyn
{
    LE32 d_tag;
    LE32 d_val;

    enum {  // tags
        DT_NULL     =  0,   /* End flag */
        DT_NEEDED   =  1,   /* Name of needed library */
        DT_STRTAB   =  5,   /* String table */
        DT_STRSZ    = 10    /* Sizeof string table */
    };
}
__attribute_packed;

#endif /* already included */


/*
vi:ts=4:et
*/

