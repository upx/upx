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
   markus@oberhumer.com      ml1050@users.sourceforge.net
 */


#ifndef __UPX_P_ELF_H
#define __UPX_P_ELF_H


/*************************************************************************
// Some ELF type definitinons
**************************************************************************/

namespace TT_Elf {

// The ELF file header. This appears at the start of every ELF file.
template <class Word, class Addr, class Off, class Half>
struct Ehdr
{
    unsigned char e_ident[16];  /* Magic number and other info */
    Half e_type;                /* Object file type */
    Half e_machine;             /* Architecture */
    Word e_version;             /* Object file version */
    Addr e_entry;               /* Entry point virtual address */
    Off  e_phoff;               /* Program header table file offset */
    Off  e_shoff;               /* Section header table file offset */
    Word e_flags;               /* Processor-specific flags */
    Half e_ehsize;              /* ELF header size in bytes */
    Half e_phentsize;           /* Program header table entry size */
    Half e_phnum;               /* Program header table entry count */
    Half e_shentsize;           /* Section header table entry size */
    Half e_shnum;               /* Section header table entry count */
    Half e_shstrndx;            /* Section header string table index */

    enum { // e_ident[]
        EI_CLASS      = 4,
        EI_DATA       = 5,      /* Data encoding */
        EI_VERSION    = 6,
        EI_OSABI      = 7,
        EI_ABIVERSION = 8,
    };
    enum { // e_ident[EI_CLASS]
        ELFCLASS32 = 1,         /* 32-bit objects */
        ELFCLASS64 = 2,         /* 64-bit objects */
    };
    enum { // e_ident[EI_DATA]
        ELFDATA2LSB = 1,        /* 2's complement, little endian */
        ELFDATA2MSB = 2         /* 2's complement, big endian */
    };
    enum { // e_ident[EI_OSABI]
        ELFOSABI_LINUX = 3
    };
    enum { // e_type
        ET_NONE = 0,            /* No file type */
        ET_REL  = 1,            /* Relocatable file */
        ET_EXEC = 2,            /* Executable file */
        ET_DYN  = 3,            /* Shared object file */
        ET_CORE = 4,            /* Core file */
    };
    enum { // e_machine
        EM_386    = 3,
        EM_PPC    = 20,
        EM_PPC64  = 21,
        EM_ARM    = 40,
        EM_X86_64 = 62,
    };
    enum { // e_version
        EV_CURRENT = 1,
    };
}
__attribute_packed;


template <class Word, class Addr>
struct Dyn
{
    Word d_tag;
    Addr d_val;

    enum { // d_tag
        DT_NULL     =  0,       /* End flag */
        DT_NEEDED   =  1,       /* Name of needed library */
        DT_HASH     =  4,       /* Hash table of symbol names */
        DT_STRTAB   =  5,       /* String table */
        DT_SYMTAB   =  6,       /* Symbol table */
        DT_STRSZ    = 10,       /* Sizeof string table */
    };
}
__attribute_packed;

} // namespace TT_Elf


namespace TT_Elf32 {

// Program segment header.
template <class Word, class Addr, class Off>
struct Phdr
{
    Word p_type;                /* Segment type */
    Off  p_offset;              /* Segment file offset */
    Addr p_vaddr;               /* Segment virtual address */
    Addr p_paddr;               /* Segment physical address */
    Word p_filesz;              /* Segment size in file */
    Word p_memsz;               /* Segment size in memory */
    Word p_flags;               /* Segment flags */
    Word p_align;               /* Segment alignment */

    enum { // p_type
        PT_LOAD    = 1,         /* Loadable program segment */
        PT_DYNAMIC = 2,         /* Dynamic linking information */
        PT_INTERP  = 3,         /* Name of program interpreter */
        PT_PHDR    = 6,         /* Entry for header table itself */
    };

    enum { // p_flags
        PF_X = 1,               /* Segment is executable */
        PF_W = 2,               /* Segment is writable */
        PF_R = 4,               /* Segment is readable */
    };
}
__attribute_packed;

template <class Word, class Addr, class Off>
struct Shdr
{
    Word sh_name;               /* Section name (string tbl index) */
    Word sh_type;               /* Section type */
    Word sh_flags;              /* Section flags */
    Addr sh_addr;               /* Section virtual addr at execution */
    Off  sh_offset;             /* Section file offset */
    Word sh_size;               /* Section size in bytes */
    Word sh_link;               /* Link to another section */
    Word sh_info;               /* Additional section information */
    Word sh_addralign;          /* Section alignment */
    Word sh_entsize;            /* Entry size if section holds table */

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
        SHT_GNU_LIBLIST = 0x6ffffff7,   /* Prelink library list */
    };

    enum { // sh_flags
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

template <class TT16, class TT32, class TT64>
struct Sym
{
    TT32 st_name;               /* symbol name (index into string table) */
    TT32 st_value;              /* symbol value */
    TT32 st_size;               /* symbol size */
    unsigned char st_info;      /* symbol type and binding */
    unsigned char st_other;     /* symbol visibility */
    TT16 st_shndx;              /* section index */

    unsigned int st_bind(unsigned int x) const { return 0xf & (x>>4); }
    unsigned int st_type(unsigned int x) const { return 0xf &  x    ; }
    unsigned char St_info(unsigned bind, unsigned type) const { return (bind<<4) + (0xf & type); }

    enum { // st_bind (high 4 bits of st_info)
        STB_LOCAL   =   0,      /* Local symbol */
        STB_GLOBAL  =   1,      /* Global symbol */
        STB_WEAK    =   2,      /* Weak symbol */
    };

    enum { // st_type (low 4 bits of st_info)
        STT_NOTYPE  =   0,      /* Symbol type is unspecified */
        STT_OBJECT  =   1,      /* Symbol is a data object */
        STT_FUNC    =   2,      /* Symbol is a code object */
        STT_SECTION =   3,      /* Symbol associated with a section */
        STT_FILE    =   4,      /* Symbol's name is file name */
        STT_COMMON  =   5,      /* Symbol is a common data object */
        STT_TLS     =   6,      /* Symbol is thread-local data object*/
    };

    enum { // st_other (visibility)
        STV_DEFAULT  =  0,      /* Default symbol visibility rules */
        STV_INTERNAL =  1,      /* Processor specific hidden class */
        STV_HIDDEN   =  2,      /* Sym unavailable in other modules */
        STV_PROTECTED=  3,      /* Not preemptible, not exported */
    };

    enum { // st_shndx
        SHN_UNDEF   =   0,      /* Undefined section */
        SHN_ABS     =   0xfff1, /* Associated symbol is absolute */
        SHN_COMMON  =   0xfff2, /* Associated symbol is common */
    };
}
__attribute_packed;

} // namespace TT_Elf32

namespace TT_Elf64 {

// Program segment header.
template <class Word, class Xword, class Addr, class Off>
struct Phdr
{
    Word  p_type;               /* Segment type */
    Word  p_flags;              /* Segment flags */
    Off   p_offset;             /* Segment file offset */
    Addr  p_vaddr;              /* Segment virtual address */
    Addr  p_paddr;              /* Segment physical address */
    Xword p_filesz;             /* Segment size in file */
    Xword p_memsz;              /* Segment size in memory */
    Xword p_align;              /* Segment alignment */

    enum { // p_type
        PT_LOAD    = 1,         /* Loadable program segment */
        PT_DYNAMIC = 2,         /* Dynamic linking information */
        PT_INTERP  = 3,         /* Name of program interpreter */
        PT_PHDR    = 6,         /* Entry for header table itself */
    };

    enum { // p_flags
        PF_X = 1,               /* Segment is executable */
        PF_W = 2,               /* Segment is writable */
        PF_R = 4,               /* Segment is readable */
    };
}
__attribute_packed;

template <class Word, class Xword, class Addr, class Off>
struct Shdr
{
    Word  sh_name;              /* Section name (string tbl index) */
    Word  sh_type;              /* Section type */
    Xword sh_flags;             /* Section flags */
    Addr  sh_addr;              /* Section virtual addr at execution */
    Off   sh_offset;            /* Section file offset */
    Xword sh_size;              /* Section size in bytes */
    Word  sh_link;              /* Link to another section */
    Word  sh_info;              /* Additional section information */
    Xword sh_addralign;         /* Section alignment */
    Xword sh_entsize;           /* Entry size if section holds table */

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
        SHT_GNU_LIBLIST = 0x6ffffff7,   /* Prelink library list */
    };
}
__attribute_packed;

} // namespace TT_Elf64

/*************************************************************************
// now for the actual types
**************************************************************************/

typedef TT_Elf  ::Ehdr<LE32,LE32,LE32,LE16> Elf_LE32_Ehdr;
typedef TT_Elf32::Phdr<LE32,LE32,LE32>      Elf_LE32_Phdr;
typedef TT_Elf32::Shdr<LE32,LE32,LE32>      Elf_LE32_Shdr;
typedef TT_Elf  ::Dyn <LE32,LE32>           Elf_LE32_Dyn;
typedef TT_Elf32::Sym <LE16,LE32,void> Elf_LE32_Sym;

typedef TT_Elf  ::Ehdr<unsigned int,unsigned int,unsigned int,unsigned short> Elf32_Ehdr;
typedef TT_Elf32::Phdr<unsigned int,unsigned int,unsigned int> Elf32_Phdr;
typedef TT_Elf32::Shdr<unsigned int,unsigned int,unsigned int> Elf32_Shdr;
typedef TT_Elf  ::Dyn <unsigned int,unsigned int>              Elf32_Dyn;
typedef TT_Elf32::Sym <unsigned short,unsigned int,void> Elf32_Sym;

ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(Elf_LE32_Ehdr) == 52)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(Elf_LE32_Phdr) == 32)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(Elf_LE32_Shdr) == 40)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(Elf_LE32_Dyn)  ==  8)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(Elf_LE32_Sym)  == 16)

ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(Elf32_Ehdr) == 52)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(Elf32_Phdr) == 32)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(Elf32_Shdr) == 40)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(Elf32_Dyn)  ==  8)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(Elf32_Sym)  == 16)


typedef TT_Elf  ::Ehdr<LE32,LE64,LE64,LE16> Elf_LE64_Ehdr;
typedef TT_Elf64::Phdr<LE32,LE64,LE64,LE64> Elf_LE64_Phdr;
typedef TT_Elf64::Shdr<LE32,LE64,LE64,LE64> Elf_LE64_Shdr;
typedef TT_Elf  ::Dyn <LE64,LE64>           Elf_LE64_Dyn;

typedef TT_Elf  ::Ehdr<unsigned int,unsigned long long,
                        unsigned long long,unsigned short>     Elf64_Ehdr;
typedef TT_Elf64::Phdr<unsigned int,unsigned long long,
                        unsigned long long,unsigned long long> Elf64_Phdr;
typedef TT_Elf64::Shdr<unsigned int,unsigned long long,
                        unsigned long long,unsigned long long> Elf64_Shdr;
typedef TT_Elf  ::Dyn <unsigned long long,unsigned long long>  Elf64_Dyn;


ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(Elf_LE64_Ehdr) == 64)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(Elf_LE64_Phdr) == 56)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(Elf_LE64_Shdr) == 64)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(Elf_LE64_Dyn)  == 16)

ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(Elf64_Ehdr) == 64)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(Elf64_Phdr) == 56)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(Elf64_Shdr) == 64)
ACC_COMPILE_TIME_ASSERT_HEADER(sizeof(Elf64_Dyn)  == 16)


#endif /* already included */


/*
vi:ts=4:et
*/

