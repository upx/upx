/* p_elf.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2004 Laszlo Molnar
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
template <class TT16, class TT32>
struct TT_Elf32_Ehdr
{
    unsigned char e_ident[16];  /* Magic number and other info */
    TT16 e_type;                /* Object file type */
    TT16 e_machine;             /* Architecture */
    TT32 e_version;             /* Object file version */
    TT32 e_entry;               /* Entry point virtual address */
    TT32 e_phoff;               /* Program header table file offset */
    TT32 e_shoff;               /* Section header table file offset */
    TT32 e_flags;               /* Processor-specific flags */
    TT16 e_ehsize;              /* ELF header size in bytes */
    TT16 e_phentsize;           /* Program header table entry size */
    TT16 e_phnum;               /* Program header table entry count */
    TT16 e_shentsize;           /* Section header table entry size */
    TT16 e_shnum;               /* Section header table entry count */
    TT16 e_shstrndx;            /* Section header string table index */

    enum {  // e_ident
        EI_CLASS   = 4,
        EI_DATA    = 5,
        EI_VERSION = 6,
        EI_OSABI   = 7
    };
    enum { // EI_CLASS
        ELFCLASS32 = 1,	/* 32-bit objects */
        ELFCLASS64 = 2  /* 64-bit objects */
    };
    enum { // EI_DATA
        ELFDATA2LSB	= 1, /* 2's complement, little endian */
        ELFDATA2MSB	= 2	 /* 2's complement, big endian */
    };
    enum {  // e_type
        ET_NONE = 0,    /* No file type */
        ET_REL  = 1,    /* Relocatable file */
        ET_EXEC = 2,    /* Executable file */
        ET_DYN  = 3,    /* Shared object file */
        ET_CORE = 4     /* Core file */
    };
    enum {  // e_version
        EV_CURRENT = 1
    };
    enum {  // e_machine
        EM_PPC      = 20
    };
}
__attribute_packed;


// Program segment header.
template <class TT16, class TT32>
struct TT_Elf32_Phdr
{
    TT32 p_type;                /* Segment type */
    TT32 p_offset;              /* Segment file offset */
    TT32 p_vaddr;               /* Segment virtual address */
    TT32 p_paddr;               /* Segment physical address */
    TT32 p_filesz;              /* Segment size in file */
    TT32 p_memsz;               /* Segment size in memory */
    TT32 p_flags;               /* Segment flags */
    TT32 p_align;               /* Segment alignment */

    // Values for p_type
    enum {
        PT_LOAD     = 1,    /* Loadable program segment */
        PT_DYNAMIC  = 2,    /* Dynamic linking information */
        PT_INTERP   = 3,    /* Name of program interpreter */
        PT_PHDR     = 6     /* Entry for header table itself */
    };

    // Values for p_flags
    enum { PF_X = (1 << 0) };   /* Segment is executable */
    enum { PF_W = (1 << 1) };   /* Segment is writable */
    enum { PF_R = (1 << 2) };   /* Segment is readable */
}
__attribute_packed;


template <class TT16, class TT32>
struct TT_Elf32_Shdr
{
    TT32 sh_name;        /* Section name (string tbl index) */
    TT32 sh_type;        /* Section type */
    TT32 sh_flags;       /* Section flags */
    TT32 sh_addr;        /* Section virtual addr at execution */
    TT32 sh_offset;      /* Section file offset */
    TT32 sh_size;        /* Section size in bytes */
    TT32 sh_link;        /* Link to another section */
    TT32 sh_info;        /* Additional section information */
    TT32 sh_addralign;   /* Section alignment */
    TT32 sh_entsize;     /* Entry size if section holds table */

    enum {  // values for sh_type
        SHT_NULL = 0,    /* Section header table entry unused */
        SHT_PROGBITS = 1,/* Program data */
        SHT_SYMTAB = 2,  /* Symbol table */
        SHT_STRTAB = 3,  /* String table */
        SHT_RELA = 4,    /* Relocation entries with addends */
        SHT_HASH = 5,    /* Symbol hash table */
        SHT_DYNAMIC = 6, /* Dynamic linking information */
        SHT_NOTE = 7,    /* Notes */
        SHT_NOBITS = 8,  /* Program space with no data (bss) */
        SHT_REL = 9,     /* Relocation entries, no addends */
        SHT_SHLIB = 10,  /* Reserved */
        SHT_DYNSYM = 11, /* Dynamic linker symbol table */
            /* 12, 13  hole */
        SHT_INIT_ARRAY = 14,    /* Array of constructors */
        SHT_FINI_ARRAY = 15,    /* Array of destructors */
        SHT_PREINIT_ARRAY = 16, /* Array of pre-constructors */
        SHT_GROUP = 17,  /* Section group */
        SHT_SYMTAB_SHNDX = 18,  /* Extended section indeces */
        SHT_NUM = 19     /* Number of defined types.  */
    };

    enum {  // values for sh_flags
        SHF_WRITE      = (1 << 0),  /* Writable */
        SHF_ALLOC      = (1 << 1),  /* Occupies memory during execution */
        SHF_EXECINSTR  = (1 << 2),  /* Executable */
        SHF_MERGE      = (1 << 4),  /* Might be merged */
        SHF_STRINGS    = (1 << 5),  /* Contains nul-terminated strings */
        SHF_INFO_LINK  = (1 << 6),  /* `sh_info' contains SHT index */
        SHF_LINK_ORDER = (1 << 7),  /* Preserve order after combining */
    };
}
__attribute_packed;


template <class TT16, class TT32>
struct TT_Elf32_Dyn
{
    TT32 d_tag;
    TT32 d_val;

    enum {  // tags
        DT_NULL     =  0,   /* End flag */
        DT_NEEDED   =  1,   /* Name of needed library */
        DT_STRTAB   =  5,   /* String table */
        DT_STRSZ    = 10    /* Sizeof string table */
    };
}
__attribute_packed;


/*************************************************************************
// now for the actual types
**************************************************************************/

typedef TT_Elf32_Ehdr<LE16,LE32> Elf_LE32_Ehdr;
typedef TT_Elf32_Phdr<LE16,LE32> Elf_LE32_Phdr;
typedef TT_Elf32_Shdr<LE16,LE32> Elf_LE32_Shdr;

typedef TT_Elf32_Ehdr<unsigned short,unsigned int> Elf32_Ehdr;
typedef TT_Elf32_Phdr<unsigned short,unsigned int> Elf32_Phdr;
typedef TT_Elf32_Shdr<unsigned short,unsigned int> Elf32_Shdr;


#endif /* already included */


/*
vi:ts=4:et
*/

