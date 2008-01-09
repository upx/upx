/* p_elf.h --

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


#ifndef __UPX_P_ELF_H
#define __UPX_P_ELF_H


/*************************************************************************
// N_Elf
**************************************************************************/

namespace N_Elf {

// integral types
template <class THalf, class TWord, class TXword, class TAddr, class TOff>
struct ElfITypes
{
    typedef THalf   Half;
    typedef TWord   Word;
    typedef TXword  Xword;
    typedef TAddr   Addr;
    typedef TOff    Off;
    typedef THalf   Section;
    typedef THalf   Versym;
};


// The ELF file header. This appears at the start of every ELF file.
template <class TElfITypes>
struct Ehdr
{
    typedef typename TElfITypes::Half    Half;
    typedef typename TElfITypes::Word    Word;
    typedef typename TElfITypes::Addr    Addr;
    typedef typename TElfITypes::Off     Off;

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

#   define WANT_EHDR_ENUM 1
#   include "p_elf_enum.h"
}
__attribute_packed;


template <class TElfITypes>
struct Dyn
{
    typedef typename TElfITypes::Xword   Xword;
    typedef typename TElfITypes::Addr    Addr;

    Xword d_tag;
    Addr d_val;

#   define WANT_DYN_ENUM 1
#   include "p_elf_enum.h"
}
__attribute_packed;


template <class TElfITypes>
struct External_Note
{
    typedef typename TElfITypes::Word   Word;

    Word xn_namesz;  // includes terminating '\0'
    Word xn_datasz;
    Word xn_type;
    //char xn_name[N];  // terminate with '\0'
    //char xn_data[M];  // aligned to 0 mod 4
}
__attribute_packed;


} // namespace N_Elf


/*************************************************************************
// N_Elf32
**************************************************************************/

namespace N_Elf32 {

template <class TElfITypes>
struct Phdr
{
    typedef typename TElfITypes::Word    Word;
    typedef typename TElfITypes::Addr    Addr;
    typedef typename TElfITypes::Off     Off;

    Word p_type;                /* Segment type */
    Off  p_offset;              /* Segment file offset */
    Addr p_vaddr;               /* Segment virtual address */
    Addr p_paddr;               /* Segment physical address */
    Word p_filesz;              /* Segment size in file */
    Word p_memsz;               /* Segment size in memory */
    Word p_flags;               /* Segment flags */
    Word p_align;               /* Segment alignment */

#   define WANT_PHDR_ENUM 1
#   include "p_elf_enum.h"
}
__attribute_packed;


template <class TElfITypes>
struct Shdr
{
    typedef typename TElfITypes::Word    Word;
    typedef typename TElfITypes::Addr    Addr;
    typedef typename TElfITypes::Off     Off;

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

#   define WANT_SHDR_ENUM 1
#   include "p_elf_enum.h"
}
__attribute_packed;


template <class TElfITypes>
struct Sym
{
    typedef typename TElfITypes::Word    Word;
    typedef typename TElfITypes::Addr    Addr;
    typedef typename TElfITypes::Section Section;

    Word st_name;               /* symbol name (index into string table) */
    Addr st_value;              /* symbol value */
    Word st_size;               /* symbol size */
    unsigned char st_info;      /* symbol type and binding */
    unsigned char st_other;     /* symbol visibility */
    Section st_shndx;           /* section index */

#   define WANT_SYM_ENUM 1
#   include "p_elf_enum.h"

    static unsigned int  get_st_bind(unsigned x) { return 0xf & (x>>4); }
    static unsigned int  get_st_type(unsigned x) { return 0xf &  x    ; }
    static unsigned char make_st_info(unsigned bind, unsigned type) { return (bind<<4) + (0xf & type); }
}
__attribute_packed;


} // namespace N_Elf32


/*************************************************************************
// N_Elf64
**************************************************************************/

namespace N_Elf64 {

template <class TElfITypes>
struct Phdr
{
    typedef typename TElfITypes::Word    Word;
    typedef typename TElfITypes::Xword   Xword;
    typedef typename TElfITypes::Addr    Addr;
    typedef typename TElfITypes::Off     Off;

    Word  p_type;               /* Segment type */
    Word  p_flags;              /* Segment flags */
    Off   p_offset;             /* Segment file offset */
    Addr  p_vaddr;              /* Segment virtual address */
    Addr  p_paddr;              /* Segment physical address */
    Xword p_filesz;             /* Segment size in file */
    Xword p_memsz;              /* Segment size in memory */
    Xword p_align;              /* Segment alignment */

#   define WANT_PHDR_ENUM 1
#   include "p_elf_enum.h"
}
__attribute_packed;


template <class TElfITypes>
struct Shdr
{
    typedef typename TElfITypes::Word    Word;
    typedef typename TElfITypes::Xword   Xword;
    typedef typename TElfITypes::Addr    Addr;
    typedef typename TElfITypes::Off     Off;

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

#   define WANT_SHDR_ENUM 1
#   include "p_elf_enum.h"
}
__attribute_packed;


template <class TElfITypes>
struct Sym
{
    typedef typename TElfITypes::Word    Word;
    typedef typename TElfITypes::Xword   Xword;
    typedef typename TElfITypes::Addr    Addr;
    typedef typename TElfITypes::Section Section;

    Word st_name;               /* symbol name (index into string table) */
    unsigned char st_info;      /* symbol type and binding */
    unsigned char st_other;     /* symbol visibility */
    Section st_shndx;           /* section index */
    Addr st_value;              /* symbol value */
    Xword st_size;              /* symbol size */

#   define WANT_SYM_ENUM 1
#   include "p_elf_enum.h"

    static unsigned int  get_st_bind(unsigned x) { return 0xf & (x>>4); }
    static unsigned int  get_st_type(unsigned x) { return 0xf &  x    ; }
    static unsigned char make_st_info(unsigned bind, unsigned type) { return (bind<<4) + (0xf & type); }
}
__attribute_packed;


} // namespace N_Elf64


/*************************************************************************
// aggregate types into an ElfClass
**************************************************************************/

namespace N_Elf {

template <class TP>
struct ElfClass_32
{
    typedef TP BeLePolicy;

    // integral types (target endianness)
    typedef typename TP::U16 TE16;
    typedef typename TP::U32 TE32;
    typedef typename TP::U64 TE64;
    typedef N_Elf::ElfITypes<TE16, TE32, TE32, TE32, TE32> ElfITypes;

    // ELF types
    typedef N_Elf  ::Ehdr<ElfITypes> Ehdr;
    typedef N_Elf32::Phdr<ElfITypes> Phdr;
    typedef N_Elf32::Shdr<ElfITypes> Shdr;
    typedef N_Elf  ::Dyn <ElfITypes> Dyn;
    typedef N_Elf32::Sym <ElfITypes> Sym;
    typedef N_Elf  ::External_Note<ElfITypes> External_Note;

    static void compileTimeAssertions() {
        BeLePolicy::compileTimeAssertions();
        COMPILE_TIME_ASSERT(sizeof(Ehdr) == 52)
        COMPILE_TIME_ASSERT(sizeof(Phdr) == 32)
        COMPILE_TIME_ASSERT(sizeof(Shdr) == 40)
        COMPILE_TIME_ASSERT(sizeof(Dyn)  ==  8)
        COMPILE_TIME_ASSERT(sizeof(Sym)  == 16)
        COMPILE_TIME_ASSERT_ALIGNED1(Ehdr)
        COMPILE_TIME_ASSERT_ALIGNED1(Phdr)
        COMPILE_TIME_ASSERT_ALIGNED1(Shdr)
        COMPILE_TIME_ASSERT_ALIGNED1(Dyn)
        COMPILE_TIME_ASSERT_ALIGNED1(Sym)
    }
};


template <class TP>
struct ElfClass_64
{
    typedef TP BeLePolicy;

    // integral types (target endianness)
    typedef typename TP::U16 TE16;
    typedef typename TP::U32 TE32;
    typedef typename TP::U64 TE64;
    typedef N_Elf::ElfITypes<TE16, TE32, TE64, TE64, TE64> ElfITypes;

    // ELF types
    typedef N_Elf  ::Ehdr<ElfITypes> Ehdr;
    typedef N_Elf64::Phdr<ElfITypes> Phdr;
    typedef N_Elf64::Shdr<ElfITypes> Shdr;
    typedef N_Elf  ::Dyn <ElfITypes> Dyn;
    typedef N_Elf64::Sym <ElfITypes> Sym;
    typedef N_Elf  ::External_Note<ElfITypes> External_Note;

    static void compileTimeAssertions() {
        BeLePolicy::compileTimeAssertions();
        COMPILE_TIME_ASSERT(sizeof(Ehdr) == 64)
        COMPILE_TIME_ASSERT(sizeof(Phdr) == 56)
        COMPILE_TIME_ASSERT(sizeof(Shdr) == 64)
        COMPILE_TIME_ASSERT(sizeof(Dyn)  == 16)
        COMPILE_TIME_ASSERT(sizeof(Sym)  == 24)
        COMPILE_TIME_ASSERT_ALIGNED1(Ehdr)
        COMPILE_TIME_ASSERT_ALIGNED1(Phdr)
        COMPILE_TIME_ASSERT_ALIGNED1(Shdr)
        COMPILE_TIME_ASSERT_ALIGNED1(Dyn)
        COMPILE_TIME_ASSERT_ALIGNED1(Sym)
    }
};


} // namespace N_Elf


typedef N_Elf::ElfClass_32<N_BELE_CTP::HostPolicy> ElfClass_Host32;
typedef N_Elf::ElfClass_64<N_BELE_CTP::HostPolicy> ElfClass_Host64;
typedef N_Elf::ElfClass_32<N_BELE_CTP::BEPolicy>   ElfClass_BE32;
typedef N_Elf::ElfClass_64<N_BELE_CTP::BEPolicy>   ElfClass_BE64;
typedef N_Elf::ElfClass_32<N_BELE_CTP::LEPolicy>   ElfClass_LE32;
typedef N_Elf::ElfClass_64<N_BELE_CTP::LEPolicy>   ElfClass_LE64;


/*************************************************************************
// shortcuts
**************************************************************************/

typedef ElfClass_Host32::Ehdr Elf32_Ehdr;
typedef ElfClass_Host32::Phdr Elf32_Phdr;
typedef ElfClass_Host32::Shdr Elf32_Shdr;
typedef ElfClass_Host32::Dyn  Elf32_Dyn;
typedef ElfClass_Host32::Sym  Elf32_Sym;
typedef ElfClass_Host32::External_Note  Elf32_External_Note;

typedef ElfClass_Host64::Ehdr Elf64_Ehdr;
typedef ElfClass_Host64::Phdr Elf64_Phdr;
typedef ElfClass_Host64::Shdr Elf64_Shdr;
typedef ElfClass_Host64::Dyn  Elf64_Dyn;
typedef ElfClass_Host64::Sym  Elf64_Sym;
typedef ElfClass_Host64::External_Note  Elf64_External_Note;

typedef ElfClass_BE32::Ehdr   Elf_BE32_Ehdr;
typedef ElfClass_BE32::Phdr   Elf_BE32_Phdr;
typedef ElfClass_BE32::Shdr   Elf_BE32_Shdr;
typedef ElfClass_BE32::Dyn    Elf_BE32_Dyn;
typedef ElfClass_BE32::Sym    Elf_BE32_Sym;
typedef ElfClass_BE32::External_Note    Elf_BE32_External_Note;

typedef ElfClass_BE64::Ehdr   Elf_BE64_Ehdr;
typedef ElfClass_BE64::Phdr   Elf_BE64_Phdr;
typedef ElfClass_BE64::Shdr   Elf_BE64_Shdr;
typedef ElfClass_BE64::Dyn    Elf_BE64_Dyn;
typedef ElfClass_BE64::Sym    Elf_BE64_Sym;
typedef ElfClass_BE64::External_Note    Elf_BE64_External_Note;

typedef ElfClass_LE32::Ehdr   Elf_LE32_Ehdr;
typedef ElfClass_LE32::Phdr   Elf_LE32_Phdr;
typedef ElfClass_LE32::Shdr   Elf_LE32_Shdr;
typedef ElfClass_LE32::Dyn    Elf_LE32_Dyn;
typedef ElfClass_LE32::Sym    Elf_LE32_Sym;
typedef ElfClass_LE32::External_Note    Elf_LE32_External_Note;

typedef ElfClass_LE64::Ehdr   Elf_LE64_Ehdr;
typedef ElfClass_LE64::Phdr   Elf_LE64_Phdr;
typedef ElfClass_LE64::Shdr   Elf_LE64_Shdr;
typedef ElfClass_LE64::Dyn    Elf_LE64_Dyn;
typedef ElfClass_LE64::Sym    Elf_LE64_Sym;
typedef ElfClass_LE64::External_Note    Elf_LE64_External_Note;


#endif /* already included */


/*
vi:ts=4:et
*/

