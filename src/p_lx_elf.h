/* p_lx_elf.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
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


#ifndef __UPX_P_LX_ELF_H  //{
#define __UPX_P_LX_ELF_H


class PackLinuxElf : public PackUnix
{
    typedef PackUnix super;
public:
    PackLinuxElf(InputFile *f);
    virtual ~PackLinuxElf();
    /*virtual void buildLoader(const Filter *);*/
    virtual bool canUnpackVersion(int version) const { return (version >= 11); }

protected:
    virtual const int *getCompressionMethods(int method, int level) const;

    // All other virtual functions in this class must be pure virtual
    // because they depend on Elf32 or Elf64 data structures, which differ.

    virtual void pack1(OutputFile *, Filter &) = 0;  // generate executable header
    virtual void pack2(OutputFile *, Filter &) = 0;  // append compressed data
    virtual void pack3(OutputFile *, Filter &) = 0;  // append loader
    //virtual void pack4(OutputFile *, Filter &) = 0;  // append pack header

    virtual void generateElfHdr(
        OutputFile *,
        void const *proto,
        unsigned const brka
    ) = 0;
    virtual void defineSymbols(Filter const *);
    virtual void addStubEntrySections(Filter const *);
    virtual void unpack(OutputFile *fo) = 0;

protected:
    char       *file_image;       // if ET_DYN investigation
    char const *dynstr;   // from DT_STRTAB

    unsigned sz_phdrs;  // sizeof Phdr[]
    unsigned sz_elf_hdrs;  // all Elf headers
    unsigned sz_pack2;  // after pack2(), before loader
    unsigned lg2_page;  // log2(PAGE_SIZE)
    unsigned page_size;  // 1u<<lg2_page

    unsigned short e_machine;
    unsigned char ei_class;
    unsigned char ei_data;
    unsigned char ei_osabi;
    char const *osabi_note;
};

class PackLinuxElf32 : public PackLinuxElf
{
    typedef PackLinuxElf super;
public:
    PackLinuxElf32(InputFile *f);
    virtual ~PackLinuxElf32();
protected:
    virtual int checkEhdr(Elf32_Ehdr const *ehdr) const;
    virtual bool canPack();

    // These ARM routines are essentially common to big/little endian,
    // but the class hierarchy splits after this class.
    virtual void ARM_defineSymbols(Filter const *ft);
    virtual void ARM_updateLoader(OutputFile *);

    virtual void pack1(OutputFile *, Filter &);  // generate executable header
    virtual void pack2(OutputFile *, Filter &);  // append compressed data
    virtual void pack3(OutputFile *, Filter &);  // append loader
    virtual void pack4(OutputFile *, Filter &);  // append pack header
    virtual void unpack(OutputFile *fo);

    virtual void generateElfHdr(
        OutputFile *,
        void const *proto,
        unsigned const brka
    );
    virtual void buildLinuxLoader(
        upx_byte const *const proto,  // assembly-only sections
        unsigned const szproto,
        upx_byte const *const fold,  // linked assembly + C section
        unsigned const szfold,
        Filter const *ft
    );
    virtual off_t getbrk(const Elf32_Phdr *phdr, int e_phnum) const;
    virtual void patchLoader();
    virtual void updateLoader(OutputFile *fo);
    virtual unsigned find_LOAD_gap(Elf32_Phdr const *const phdri, unsigned const k,
        unsigned const e_phnum);
    virtual off_t getbase(const Elf32_Phdr *phdr, int e_phnum) const;

    static unsigned elf_hash(char const *) /*const*/;
    static unsigned gnu_hash(char const *) /*const*/;
    virtual void const *elf_find_dynamic(unsigned) const;
    virtual Elf32_Sym const *elf_lookup(char const *) const;
    virtual unsigned elf_get_offset_from_address(unsigned) const;
    Elf32_Shdr const *elf_find_section_name(char const *) const;
    Elf32_Shdr const *elf_find_section_type(unsigned) const;

protected:
    Elf32_Ehdr  ehdri; // from input file
    Elf32_Phdr *phdri; // for  input file
    Elf32_Shdr const *shdri; // from input file
    unsigned page_mask;  // AND clears the offset-within-page

    Elf32_Dyn    const *dynseg;   // from PT_DYNAMIC
    unsigned int const *hashtab;  // from DT_HASH
    unsigned int const *gashtab;  // from DT_GNU_HASH
    Elf32_Sym    const *dynsym;   // from DT_SYMTAB
    char const *shstrtab;   // via Elf32_Shdr
    int n_elf_shnum;  // via e_shnum

    Elf32_Shdr const *sec_strndx;
    Elf32_Shdr const *sec_dynsym;
    Elf32_Shdr const *sec_dynstr;

    struct cprElfHdr1 {
        Elf32_Ehdr ehdr;
        Elf32_Phdr phdr[1];
        l_info linfo;
    }
    __attribute_packed;

    struct cprElfHdr2 {
        Elf32_Ehdr ehdr;
        Elf32_Phdr phdr[2];
        l_info linfo;
    }
    __attribute_packed;

    struct cprElfHdr3 {
        Elf32_Ehdr ehdr;
        Elf32_Phdr phdr[3];
        l_info linfo;
    }
    __attribute_packed;

    cprElfHdr3 elfout;

    struct Elf32_Note {
        unsigned namesz;  // 8
        unsigned descsz;  // 4
        unsigned type;    // 1
        char text[0x18 - 4*4];  // "OpenBSD"
        unsigned end;     // 0
    } elfnote;

    static void compileTimeAssertions() {
        COMPILE_TIME_ASSERT(sizeof(cprElfHdr1) == 52 + 1*32 + 12)
        COMPILE_TIME_ASSERT(sizeof(cprElfHdr2) == 52 + 2*32 + 12)
        COMPILE_TIME_ASSERT(sizeof(cprElfHdr3) == 52 + 3*32 + 12)
        COMPILE_TIME_ASSERT_ALIGNED1(cprElfHdr1)
        COMPILE_TIME_ASSERT_ALIGNED1(cprElfHdr2)
        COMPILE_TIME_ASSERT_ALIGNED1(cprElfHdr3)
    }
};


class PackLinuxElf64 : public PackLinuxElf
{
    typedef PackLinuxElf super;
public:
    PackLinuxElf64(InputFile *f);
    virtual ~PackLinuxElf64();
    /*virtual void buildLoader(const Filter *);*/

protected:
    virtual int checkEhdr(Elf64_Ehdr const *ehdr) const;

    virtual void pack1(OutputFile *, Filter &);  // generate executable header
    virtual void pack2(OutputFile *, Filter &);  // append compressed data
    virtual void pack3(OutputFile *, Filter &);  // append loader
    virtual void pack4(OutputFile *, Filter &);  // append pack header
    virtual void unpack(OutputFile *fo);

    virtual void generateElfHdr(
        OutputFile *,
        void const *proto,
        unsigned const brka
    );
    virtual void buildLinuxLoader(
        upx_byte const *const proto,  // assembly-only sections
        unsigned const szproto,
        upx_byte const *const fold,  // linked assembly + C section
        unsigned const szfold,
        Filter const *ft
    );
    virtual off_t getbrk(const Elf64_Phdr *phdr, int e_phnum) const;
    virtual void patchLoader();
    virtual void updateLoader(OutputFile *fo);
    virtual unsigned find_LOAD_gap(Elf64_Phdr const *const phdri, unsigned const k,
        unsigned const e_phnum);

protected:
    Elf64_Ehdr  ehdri; // from input file
    Elf64_Phdr *phdri; // for  input file
    acc_uint64l_t page_mask;  // AND clears the offset-within-page

    struct cprElfHdr1 {
        Elf64_Ehdr ehdr;
        Elf64_Phdr phdr[1];
        l_info linfo;
    }
    __attribute_packed;

    struct cprElfHdr2 {
        Elf64_Ehdr ehdr;
        Elf64_Phdr phdr[2];
        l_info linfo;
    }
    __attribute_packed;

    struct cprElfHdr3 {
        Elf64_Ehdr ehdr;
        Elf64_Phdr phdr[3];
        l_info linfo;
    }
    __attribute_packed;

    cprElfHdr3 elfout;

    static void compileTimeAssertions() {
        COMPILE_TIME_ASSERT(sizeof(cprElfHdr1) == 64 + 1*56 + 12)
        COMPILE_TIME_ASSERT(sizeof(cprElfHdr2) == 64 + 2*56 + 12)
        COMPILE_TIME_ASSERT(sizeof(cprElfHdr3) == 64 + 3*56 + 12)
        COMPILE_TIME_ASSERT_ALIGNED1(cprElfHdr1)
        COMPILE_TIME_ASSERT_ALIGNED1(cprElfHdr2)
        COMPILE_TIME_ASSERT_ALIGNED1(cprElfHdr3)
    }
};

class PackLinuxElf32Be : public PackLinuxElf32
{
    typedef PackLinuxElf32 super;
protected:
    PackLinuxElf32Be(InputFile *f) : super(f) { bele = &N_BELE_RTP::be_policy; }
};

class PackLinuxElf32Le : public PackLinuxElf32
{
    typedef PackLinuxElf32 super;
protected:
    PackLinuxElf32Le(InputFile *f) : super(f) { bele = &N_BELE_RTP::le_policy; }
};

class PackLinuxElf64Le : public PackLinuxElf64
{
    typedef PackLinuxElf64 super;
protected:
    PackLinuxElf64Le(InputFile *f) : super(f) { bele = &N_BELE_RTP::le_policy; }
};


/*************************************************************************
// linux/elf64amd
**************************************************************************/

class PackLinuxElf64amd : public PackLinuxElf64Le
{
    typedef PackLinuxElf64Le super;
public:
    PackLinuxElf64amd(InputFile *f);
    virtual ~PackLinuxElf64amd();
    virtual int getFormat() const { return UPX_F_LINUX_ELF64_AMD; }
    virtual const char *getName() const { return "linux/ElfAMD"; }
    virtual const char *getFullName(const options_t *) const { return "amd64-linux.elf"; }
    virtual const int *getFilters() const;
    virtual bool canPack();
protected:
    virtual void pack1(OutputFile *, Filter &);  // generate executable header
    //virtual void pack3(OutputFile *, Filter &);  // append loader
    virtual void buildLoader(const Filter *);
    virtual Linker* newLinker() const;
    virtual void defineSymbols(Filter const *);
};


/*************************************************************************
// linux/elf32ppc
**************************************************************************/

class PackLinuxElf32ppc : public PackLinuxElf32Be
{
    typedef PackLinuxElf32Be super;
public:
    PackLinuxElf32ppc(InputFile *f);
    virtual ~PackLinuxElf32ppc();
    virtual int getFormat() const { return UPX_F_LINUX_ELFPPC32; }
    virtual const char *getName() const { return "linux/ElfPPC"; }
    virtual const char *getFullName(const options_t *) const { return "powerpc-linux.elf"; }
    virtual const int *getFilters() const;
protected:
    virtual void pack1(OutputFile *, Filter &);  // generate executable header
    virtual void buildLoader(const Filter *);
    virtual Linker* newLinker() const;
};


/*************************************************************************
// linux/elf386
**************************************************************************/

class PackLinuxElf32x86 : public PackLinuxElf32Le
{
    typedef PackLinuxElf32Le super;
public:
    PackLinuxElf32x86(InputFile *f);
    virtual ~PackLinuxElf32x86();
    virtual int getFormat() const { return UPX_F_LINUX_ELF_i386; }
    virtual const char *getName() const { return "linux/elf386"; }
    virtual const char *getFullName(const options_t *) const { return "i386-linux.elf"; }
    virtual const int *getFilters() const;

    virtual void unpack(OutputFile *fo);

protected:
    virtual void pack1(OutputFile *, Filter &);  // generate executable header

    virtual void buildLoader(const Filter *);
    virtual void addStubEntrySections(Filter const *);
    virtual Linker* newLinker() const;
    virtual void defineSymbols(Filter const *);
};

class PackBSDElf32x86 : public PackLinuxElf32x86
{
    typedef PackLinuxElf32x86 super;
public:
    PackBSDElf32x86(InputFile *f);
    virtual ~PackBSDElf32x86();
    virtual int getFormat() const { return UPX_F_BSD_ELF_i386; }
    virtual const char *getName() const { return "BSD/elf386"; }

protected:
    virtual void pack1(OutputFile *, Filter &);  // generate executable header

    virtual void buildLoader(const Filter *);
};

class PackFreeBSDElf32x86 : public PackBSDElf32x86
{
    typedef PackBSDElf32x86 super;
public:
    PackFreeBSDElf32x86(InputFile *f);
    virtual ~PackFreeBSDElf32x86();
    virtual const char *getFullName(const options_t *) const { return "i386-freebsd.elf"; }
};

class PackNetBSDElf32x86 : public PackBSDElf32x86
{
    typedef PackBSDElf32x86 super;
public:
    PackNetBSDElf32x86(InputFile *f);
    virtual ~PackNetBSDElf32x86();
    virtual const char *getFullName(const options_t *) const { return "i386-netbsd.elf"; }
};

class PackOpenBSDElf32x86 : public PackBSDElf32x86
{
    typedef PackBSDElf32x86 super;
public:
    PackOpenBSDElf32x86(InputFile *f);
    virtual ~PackOpenBSDElf32x86();
    virtual const char *getFullName(const options_t *) const { return "i386-openbsd.elf"; }

protected:
    virtual void buildLoader(const Filter *ft);
    virtual void generateElfHdr(
        OutputFile *,
        void const *proto,
        unsigned const brka
    );
};


/*************************************************************************
// linux/elfarm
**************************************************************************/

class PackLinuxElf32armLe : public PackLinuxElf32Le
{
    typedef PackLinuxElf32Le super;
public:
    PackLinuxElf32armLe(InputFile *f);
    virtual ~PackLinuxElf32armLe();
    virtual int getFormat() const { return UPX_F_LINUX_ELF32_ARMEL; }
    virtual const char *getName() const { return "linux/armel"; }
    virtual const char *getFullName(const options_t *) const { return "arm-linux.elf"; }
    virtual const int *getFilters() const;

protected:
    virtual const int *getCompressionMethods(int method, int level) const;
    virtual Linker* newLinker() const;
    virtual void pack1(OutputFile *, Filter &);  // generate executable header
    virtual void buildLoader(const Filter *);
    virtual void updateLoader(OutputFile *);
    virtual void defineSymbols(Filter const *);
};

class PackLinuxElf32armBe : public PackLinuxElf32Be
{
    typedef PackLinuxElf32Be super;
public:
    PackLinuxElf32armBe(InputFile *f);
    virtual ~PackLinuxElf32armBe();
    virtual int getFormat() const { return UPX_F_LINUX_ELF32_ARMEB; }
    virtual const char *getName() const { return "linux/armeb"; }
    virtual const char *getFullName(const options_t *) const { return "armeb-linux.elf"; }
    virtual const int *getFilters() const;

protected:
    virtual const int *getCompressionMethods(int method, int level) const;
    virtual Linker* newLinker() const;
    virtual void pack1(OutputFile *, Filter &);  // generate executable header
    virtual void buildLoader(const Filter *);
    virtual void updateLoader(OutputFile *);
    virtual void defineSymbols(Filter const *);
};

class PackLinuxElf32mipseb : public PackLinuxElf32Be
{
    typedef PackLinuxElf32Be super;
public:
    PackLinuxElf32mipseb(InputFile *f);
    virtual ~PackLinuxElf32mipseb();
    virtual int getFormat() const { return UPX_F_LINUX_ELF32_MIPSEB; }
    virtual const char *getName() const { return "linux/mipseb"; }
    virtual const char *getFullName(const options_t *) const { return "mips-linux.elf"; }
    virtual const int *getFilters() const;

protected:
    virtual Linker* newLinker() const;
    virtual void pack1(OutputFile *, Filter &);  // generate executable header
    virtual void buildLoader(const Filter *);
    virtual void updateLoader(OutputFile *);
    virtual void defineSymbols(Filter const *);
};

class PackLinuxElf32mipsel : public PackLinuxElf32Le
{
    typedef PackLinuxElf32Le super;
public:
    PackLinuxElf32mipsel(InputFile *f);
    virtual ~PackLinuxElf32mipsel();
    virtual int getFormat() const { return UPX_F_LINUX_ELF32_MIPSEL; }
    virtual const char *getName() const { return "linux/mipsel"; }
    virtual const char *getFullName(const options_t *) const { return "mipsel-linux.elf"; }
    virtual const int *getFilters() const;

protected:
    virtual Linker* newLinker() const;
    virtual void pack1(OutputFile *, Filter &);  // generate executable header
    virtual void buildLoader(const Filter *);
    virtual void updateLoader(OutputFile *);
    virtual void defineSymbols(Filter const *);
};


#endif /*} already included */


/*
vi:ts=4:et
*/

