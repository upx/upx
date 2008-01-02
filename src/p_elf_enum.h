/* p_elf_enum.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
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
 */


/*************************************************************************
// Use the preprocessor to work around
//   - that the types embedding these enums have to be PODs, and
//     deriving from an empty base class (which is the usual C++ way
//     of "importing" enums) does not yield a POD any more
//   - that older compilers do not correctly perform EBCO
**************************************************************************/

#ifdef WANT_EHDR_ENUM
#undef WANT_EHDR_ENUM
    enum { // e_ident[]
        EI_CLASS      = 4,
        EI_DATA       = 5,      /* Data encoding */
        EI_VERSION    = 6,
        EI_OSABI      = 7,
        EI_ABIVERSION = 8
    };
    enum { // e_ident[EI_CLASS]
        ELFCLASS32 = 1,         /* 32-bit objects */
        ELFCLASS64 = 2          /* 64-bit objects */
    };
    enum { // e_ident[EI_DATA]
        ELFDATA2LSB = 1,        /* 2's complement, little endian */
        ELFDATA2MSB = 2         /* 2's complement, big endian */
    };
    enum { // e_ident[EI_OSABI]
        ELFOSABI_NONE    = 0,      // == ELFOSABI_SYSV
        ELFOSABI_NETBSD  = 2,
        ELFOSABI_LINUX   = 3,
        ELFOSABI_FREEBSD = 9,
        ELFOSABI_OPENBSD = 12,
        ELFOSABI_ARM     = 97
    };
    enum { // e_type
        ET_NONE = 0,            /* No file type */
        ET_REL  = 1,            /* Relocatable file */
        ET_EXEC = 2,            /* Executable file */
        ET_DYN  = 3,            /* Shared object file */
        ET_CORE = 4             /* Core file */
    };
    enum { // e_machine
        EM_386    = 3,
        EM_MIPS   = 8,
        EM_MIPS_RS3_LE =  10,   /* MIPS R3000 little-endian */
        EM_PPC    = 20,
        EM_PPC64  = 21,
        EM_ARM    = 40,
        EM_X86_64 = 62

    };
    enum { // e_version
        EV_CURRENT = 1
    };
#endif


#ifdef WANT_PHDR_ENUM
#undef WANT_PHDR_ENUM
    enum { // p_type
        PT_LOAD    = 1,         /* Loadable program segment */
        PT_DYNAMIC = 2,         /* Dynamic linking information */
        PT_INTERP  = 3,         /* Name of program interpreter */
        PT_NOTE    = 4,         /* Auxiliary information (esp. OpenBSD) */
        PT_PHDR    = 6          /* Entry for header table itself */
    };

    enum { // p_flags
        PF_X = 1,               /* Segment is executable */
        PF_W = 2,               /* Segment is writable */
        PF_R = 4                /* Segment is readable */
    };
#endif


#ifdef WANT_SHDR_ENUM
#undef WANT_SHDR_ENUM
    enum { // sh_type
        SHT_NULL = 0,           /* Section header table entry unused */
        SHT_PROGBITS = 1,       /* Program data */
        SHT_SYMTAB = 2,         /* Symbol table */
        SHT_STRTAB = 3,         /* String table */
        SHT_RELA = 4,           /* Relocation entries with addends */
        SHT_HASH = 5,           /* Symbol hash table */
        SHT_DYNAMIC = 6,        /* Dynamic linking information */
        SHT_NOTE = 7,           /* Notes */
        SHT_NOBITS = 8,         /* Program space with no data (bss) */
        SHT_REL = 9,            /* Relocation entries, no addends */
        SHT_SHLIB = 10,         /* Reserved */
        SHT_DYNSYM = 11,        /* Dynamic linker symbol table */
            /* 12, 13  hole */
        SHT_INIT_ARRAY = 14,    /* Array of constructors */
        SHT_FINI_ARRAY = 15,    /* Array of destructors */
        SHT_PREINIT_ARRAY = 16, /* Array of pre-constructors */
        SHT_GROUP = 17,         /* Section group */
        SHT_SYMTAB_SHNDX = 18,  /* Extended section indeces */
        SHT_GNU_LIBLIST = 0x6ffffff7    /* Prelink library list */
    };

    enum { // sh_flags
        SHF_WRITE      = (1 << 0),  /* Writable */
        SHF_ALLOC      = (1 << 1),  /* Occupies memory during execution */
        SHF_EXECINSTR  = (1 << 2),  /* Executable */
        SHF_MERGE      = (1 << 4),  /* Might be merged */
        SHF_STRINGS    = (1 << 5),  /* Contains nul-terminated strings */
        SHF_INFO_LINK  = (1 << 6),  /* 'sh_info' contains SHT index */
        SHF_LINK_ORDER = (1 << 7)   /* Preserve order after combining */
    };
#endif


#ifdef WANT_DYN_ENUM
#undef WANT_DYN_ENUM
    enum { // d_tag
        DT_NULL     =  0,       /* End flag */
        DT_NEEDED   =  1,       /* Name of needed library */
        DT_PLTRELSZ =  2,       /* Size in bytes of PLT relocs */
        DT_HASH     =  4,       /* Hash table of symbol names */
        DT_STRTAB   =  5,       /* String table */
        DT_SYMTAB   =  6,       /* Symbol table */
        DT_STRSZ    = 10,       /* Sizeof string table */
        DT_PLTREL   = 20,       /* Type of reloc in PLT */
        DT_JMPREL   = 23,       /* Address of PLT relocs */
        DT_GNU_HASH = 0x6ffffef5        /* GNU-style hash table */
    };
#endif


#ifdef WANT_SYM_ENUM
#undef WANT_SYM_ENUM
    enum { // st_bind (high 4 bits of st_info)
        STB_LOCAL   =   0,      /* Local symbol */
        STB_GLOBAL  =   1,      /* Global symbol */
        STB_WEAK    =   2       /* Weak symbol */
    };

    enum { // st_type (low 4 bits of st_info)
        STT_NOTYPE  =   0,      /* Symbol type is unspecified */
        STT_OBJECT  =   1,      /* Symbol is a data object */
        STT_FUNC    =   2,      /* Symbol is a code object */
        STT_SECTION =   3,      /* Symbol associated with a section */
        STT_FILE    =   4,      /* Symbol's name is file name */
        STT_COMMON  =   5,      /* Symbol is a common data object */
        STT_TLS     =   6       /* Symbol is thread-local data object*/
    };

    enum { // st_other (visibility)
        STV_DEFAULT  =  0,      /* Default symbol visibility rules */
        STV_INTERNAL =  1,      /* Processor specific hidden class */
        STV_HIDDEN   =  2,      /* Sym unavailable in other modules */
        STV_PROTECTED=  3       /* Not preemptible, not exported */
    };

    enum { // st_shndx
        SHN_UNDEF   =   0,      /* Undefined section */
        SHN_ABS     =   0xfff1, /* Associated symbol is absolute */
        SHN_COMMON  =   0xfff2  /* Associated symbol is common */
    };
#endif


/*
vi:ts=4:et
*/

