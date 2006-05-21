/* p_lx_elf.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2004 Laszlo Molnar
   Copyright (C) 2000-2004 John F. Reiser
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
   <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>

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
    /*virtual int buildLoader(const Filter *);*/
    virtual bool canUnpackVersion(int version) const { return (version >= 11); }

protected:
    virtual const int *getCompressionMethods(int method, int level) const;

    // All other virtual functions in this class must be pure virtual
    // because they depend on Elf32 or Elf64 data structures, which differ.

    virtual void pack1(OutputFile *, Filter &) = 0;  // generate executable header
    virtual void pack2(OutputFile *, Filter &) = 0;  // append compressed data
    //virtual void pack3(OutputFile *, Filter &) = 0;  // append loader
    virtual void pack4(OutputFile *, Filter &) = 0;  // append pack header

    virtual void generateElfHdr(
        OutputFile *,
        void const *proto,
        unsigned const brka
    ) = 0;
    virtual void unpack(OutputFile *fo) = 0;

protected:
    unsigned sz_phdrs;  // sizeof Phdr[]
    unsigned sz_elf_hdrs;  // all Elf headers
};

class PackLinuxElf32 : public PackLinuxElf
{
    typedef PackLinuxElf super;
public:
    PackLinuxElf32(InputFile *f);
    virtual ~PackLinuxElf32();
protected:
    virtual int checkEhdr(
        Elf32_Ehdr const *ehdr,
        unsigned char e_machine,
        unsigned char ei_class,
        unsigned char ei_data) const;

    virtual void pack1(OutputFile *, Filter &);  // generate executable header
    virtual void pack2(OutputFile *, Filter &);  // append compressed data
    //virtual void pack3(OutputFile *, Filter &);  // append loader
    virtual void pack4(OutputFile *, Filter &);  // append pack header
    virtual void unpack(OutputFile *fo);

    virtual void generateElfHdr(
        OutputFile *,
        void const *proto,
        unsigned const brka
    );
    virtual off_t getbrk(const Elf32_Phdr *phdr, int e_phnum) const;
    virtual void patchLoader();
    virtual void updateLoader(OutputFile *fo);
    virtual unsigned find_LOAD_gap(Elf32_Phdr const *const phdri, unsigned const k,
        unsigned const e_phnum);
    virtual off_t getbase(const Elf32_Phdr *phdr, int e_phnum) const;

protected:
    Elf32_Ehdr  ehdri; // from input file
    Elf32_Phdr *phdri; // for  input file

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
};


class PackLinuxElf64 : public PackLinuxElf
{
    typedef PackLinuxElf super;
public:
    PackLinuxElf64(InputFile *f);
    virtual ~PackLinuxElf64();
    /*virtual int buildLoader(const Filter *);*/

protected:
    virtual int checkEhdr(
        Elf64_Ehdr const *ehdr,
        unsigned char e_machine,
        unsigned char ei_class,
        unsigned char ei_data) const;

    virtual void pack1(OutputFile *, Filter &);  // generate executable header
    virtual void pack2(OutputFile *, Filter &);  // append compressed data
    //virtual void pack3(OutputFile *, Filter &);  // append loader
    virtual void pack4(OutputFile *, Filter &);  // append pack header
    virtual void unpack(OutputFile *fo);

    virtual void generateElfHdr(
        OutputFile *,
        void const *proto,
        unsigned const brka
    );
    virtual off_t getbrk(const Elf64_Phdr *phdr, int e_phnum) const;
    virtual void patchLoader();
    virtual void updateLoader(OutputFile *fo);
    virtual unsigned find_LOAD_gap(Elf64_Phdr const *const phdri, unsigned const k,
        unsigned const e_phnum);

protected:
    Elf64_Ehdr  ehdri; // from input file
    Elf64_Phdr *phdri; // for  input file

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
};

class PackLinuxElf32Be : public PackLinuxElf32
{
    typedef PackLinuxElf32 super;
protected:
    PackLinuxElf32Be(InputFile *f) : super(f) { }

    virtual acc_uint64l_t get_native64(const void *b) const { return get_be64(b); }
    virtual unsigned get_native32(const void *b) const { return get_be32(b); }
    virtual unsigned get_native16(const void *b) const { return get_be16(b); }
    virtual void set_native64(void *b, acc_uint64l_t v) const { set_be64(b, v); }
    virtual void set_native32(void *b, unsigned v) const { set_be32(b, v); }
    virtual void set_native16(void *b, unsigned v) const { set_be16(b, v); }
};

class PackLinuxElf32Le : public PackLinuxElf32
{
    typedef PackLinuxElf32 super;
protected:
    PackLinuxElf32Le(InputFile *f) : super(f) { }

    virtual acc_uint64l_t get_native64(const void *b) const { return get_le64(b); }
    virtual unsigned get_native32(const void *b) const { return get_le32(b); }
    virtual unsigned get_native16(const void *b) const { return get_le16(b); }
    virtual void set_native64(void *b, acc_uint64l_t v) const { set_le64(b, v); }
    virtual void set_native32(void *b, unsigned v) const { set_le32(b, v); }
    virtual void set_native16(void *b, unsigned v) const { set_le16(b, v); }
};

class PackLinuxElf64Le : public PackLinuxElf64
{
    typedef PackLinuxElf64 super;
protected:
    PackLinuxElf64Le(InputFile *f) : super(f) { }

    virtual acc_uint64l_t get_native64(const void *b) const { return get_le64(b); }
    virtual unsigned get_native32(const void *b) const { return get_le32(b); }
    virtual unsigned get_native16(const void *b) const { return get_le16(b); }
    virtual void set_native64(void *b, acc_uint64l_t v) const { set_le64(b, v); }
    virtual void set_native32(void *b, unsigned v) const { set_le32(b, v); }
    virtual void set_native16(void *b, unsigned v) const { set_le16(b, v); }
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
    virtual const int *getFilters() const;
    virtual bool canPack();
protected:
    virtual void pack3(OutputFile *, Filter &);  // append loader
    virtual const int *getCompressionMethods(int method, int level) const;
    virtual int buildLoader(const Filter *);
    virtual int buildLinuxLoader(
        upx_byte const *const proto,  // assembly-only sections
        unsigned const szproto,
        upx_byte const *const fold,  // linked assembly + C section
        unsigned const szfold,
        Filter const *ft
    );
    virtual void generateElfHdr(
        OutputFile *,
        void const *proto,
        unsigned const brka
    );
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
    virtual const int *getFilters() const;
    virtual bool canPack();
protected:
    virtual void pack3(OutputFile *, Filter &);  // append loader
    virtual const int *getCompressionMethods(int method, int level) const;
    virtual int buildLoader(const Filter *);
    virtual int buildLinuxLoader(
        upx_byte const *const proto,  // assembly-only sections
        unsigned const szproto,
        upx_byte const *const fold,  // linked assembly + C section
        unsigned const szfold,
        Filter const *ft
    );
    virtual void generateElfHdr(
        OutputFile *,
        void const *proto,
        unsigned const brka
    );
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
    virtual int getVersion() const { return 13; }
    virtual int getFormat() const { return UPX_F_LINUX_ELF_i386; }
    virtual const char *getName() const { return "linux/elf386"; }
    virtual const int *getFilters() const;

    virtual void unpack(OutputFile *fo);

    virtual bool canPack();
    virtual bool canUnpackVersion(int version) const
        { return (version >= 11); }

protected:
    virtual void pack1(OutputFile *, Filter &);  // generate executable header
    virtual void pack2(OutputFile *, Filter &);  // append compressed data
    virtual void pack3(OutputFile *, Filter &);  // append loader
    virtual void pack4(OutputFile *, Filter &);  // append pack header

    virtual void patchLoader();
    virtual int buildLoader(const Filter *);
    virtual int buildLinuxLoader(
        upx_byte const *const proto,  // assembly-only sections
        unsigned const szproto,
        upx_byte const *const fold,  // linked assembly + C section
        unsigned const szfold,
        Filter const *ft
    );
    virtual void generateElfHdr(
        OutputFile *,
        void const *proto,
        unsigned const brka
    );

    Elf32_Ehdr  ehdri; // from input file
    Elf32_Phdr *phdri; // for  input file
    unsigned sz_phdrs;  // sizeof Phdr[]

            char       *file_image;       // if ET_DYN investigation
    Elf32_Dyn    const *dynseg;   // from PT_DYNAMIC
    unsigned int const *hashtab;  // from DT_HASH
            char const *dynstr;   // from DT_STRTAB
    Elf32_Sym    const *dynsym;   // from DT_SYMTAB

    static unsigned elf_hash(char const *) /*const*/;
    virtual void const *elf_find_dynamic(unsigned) const;
    virtual Elf32_Sym const *elf_lookup(char const *) const;
    virtual unsigned elf_get_offset_from_address(unsigned) const;
    virtual unsigned find_LOAD_gap(Elf32_Phdr const *const phdri, unsigned const k,
        unsigned const e_phnum);
};


#endif /*} already included */


/*
vi:ts=4:et
*/

