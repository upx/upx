/* p_vmlinx.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2023 Laszlo Molnar
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
   <markus@oberhumer.com>               <ezerotven+github@gmail.com>
 */


#ifndef __UPX_P_VMLINX_H
#define __UPX_P_VMLINX_H 1

#include "p_elf.h"


/*************************************************************************
// vmlinx/i386 (bare binary Linux kernel image)
**************************************************************************/

template <class TElfClass>
class PackVmlinuxBase : public Packer
{
    typedef Packer super;
protected:
    typedef TElfClass ElfClass;
    typedef typename ElfClass::BeLePolicy BeLePolicy;
    typedef typename ElfClass::ElfITypes  ElfITypes;
    // integral types
    typedef typename ElfClass::TE16  TE16;
    typedef typename ElfClass::TE32  TE32;
    typedef typename ElfClass::TE64  TE64;
    typedef typename ElfITypes::Addr Addr;
    // ELF types
    typedef typename ElfClass::Ehdr Ehdr;
    typedef typename ElfClass::Shdr Shdr;
    typedef typename ElfClass::Phdr Phdr;
    typedef typename ElfClass::Dyn  Dyn;
    typedef typename ElfClass::Sym  Sym;

public:
    PackVmlinuxBase(InputFile *, unsigned, unsigned, unsigned, char const *);
    virtual ~PackVmlinuxBase();
    virtual int getVersion() const override { return 13; }

protected:
    unsigned int const my_e_machine;
    unsigned char const my_elfclass;
    unsigned char const my_elfdata;
    char const *const my_boot_label;

    int n_ptload;
    unsigned sz_ptload;
    unsigned paddr_min;
    Phdr *phdri; // from input file
    Shdr *shdri; // from input file
    char *shstrtab; // from input file
    Shdr *p_text;
    Shdr *p_note0;
    Shdr *p_note1;
    Ehdr ehdri; // from input file

    virtual Shdr const *getElfSections();
    virtual int getStrategy(Filter &/*ft*/);
    virtual bool is_valid_e_entry(Addr) = 0;
    virtual bool has_valid_vmlinux_head() = 0;
    virtual bool canPack() override;
    virtual void pack(OutputFile *fo) override;
    virtual int canUnpack() override;  // bool, except -1: format known, but not packed
    virtual void unpack(OutputFile *fo) override;
    virtual unsigned write_vmlinux_head(
        OutputFile *fo,
        Shdr *stxt
    ) = 0;
    static int __acc_cdecl_qsort compare_Phdr(void const *aa, void const *bb);
};


class PackVmlinuxI386 : public PackVmlinuxBase<ElfClass_LE32>
{
    typedef PackVmlinuxBase<ElfClass_LE32> super;
public:
    PackVmlinuxI386(InputFile *f) : super(f, Ehdr::EM_386,
        Ehdr::ELFCLASS32, Ehdr::ELFDATA2LSB, "startup_32") { }
    virtual int getFormat() const override { return UPX_F_VMLINUX_i386; }
    virtual const char *getName() const override { return "vmlinux/i386"; }
    virtual const char *getFullName(const options_t *) const override { return "i386-linux.kernel.vmlinux"; }
    virtual const int *getCompressionMethods(int method, int level) const override;
    virtual const int *getFilters() const override;

protected:
    virtual void buildLoader(const Filter *ft) override;
    virtual void defineDecompressorSymbols() override;
    virtual Linker* newLinker() const override;
    virtual bool is_valid_e_entry(Addr) override;
    virtual bool has_valid_vmlinux_head() override;
    virtual unsigned write_vmlinux_head(
        OutputFile *fo,
        Shdr *stxt
    ) override;
};


class PackVmlinuxARMEL : public PackVmlinuxBase<ElfClass_LE32>
{
    typedef PackVmlinuxBase<ElfClass_LE32> super;
public:
    PackVmlinuxARMEL(InputFile *f) : super(f, Ehdr::EM_ARM,
        Ehdr::ELFCLASS32, Ehdr::ELFDATA2LSB, "decompress_kernel") { }
    virtual int getFormat() const override { return UPX_F_VMLINUX_ARMEL; }
    virtual const char *getName() const override { return "vmlinux/arm"; }
    virtual const char *getFullName(const options_t *) const override { return "arm-linux.kernel.vmlinux"; }
    virtual const int *getCompressionMethods(int method, int level) const override;
    virtual const int *getFilters() const override;

protected:
    virtual void buildLoader(const Filter *ft) override;
    virtual void defineDecompressorSymbols() override;
    virtual Linker* newLinker() const override;
    virtual bool is_valid_e_entry(Addr) override;
    virtual bool has_valid_vmlinux_head() override;
    virtual unsigned write_vmlinux_head(
        OutputFile *fo,
        Shdr *stxt
    ) override;
};

class PackVmlinuxARMEB : public PackVmlinuxBase<ElfClass_BE32>
{
    typedef PackVmlinuxBase<ElfClass_BE32> super;
public:
    PackVmlinuxARMEB(InputFile *f) : super(f, Ehdr::EM_ARM,
        Ehdr::ELFCLASS32, Ehdr::ELFDATA2MSB, "decompress_kernel") { }
    virtual int getFormat() const override { return UPX_F_VMLINUX_ARMEB; }
    virtual const char *getName() const override { return "vmlinux/armeb"; }
    virtual const char *getFullName(const options_t *) const override { return "armeb-linux.kernel.vmlinux"; }
    virtual const int *getCompressionMethods(int method, int level) const override;
    virtual const int *getFilters() const override;

protected:
    virtual void buildLoader(const Filter *ft) override;
    virtual void defineDecompressorSymbols() override;
    virtual Linker* newLinker() const override;
    virtual bool is_valid_e_entry(Addr) override;
    virtual bool has_valid_vmlinux_head() override;
    virtual unsigned write_vmlinux_head(
        OutputFile *fo,
        Shdr *stxt
    ) override;
};

class PackVmlinuxPPC32 : public PackVmlinuxBase<ElfClass_BE32>
{
    typedef PackVmlinuxBase<ElfClass_BE32> super;
public:
    PackVmlinuxPPC32(InputFile *f) : super(f, Ehdr::EM_PPC,
        Ehdr::ELFCLASS32, Ehdr::ELFDATA2MSB, "_vmlinux_start") { }
    virtual int getFormat() const override { return UPX_F_VMLINUX_PPC32; }
    virtual const char *getName() const override { return "vmlinux/ppc32"; }
    virtual const char *getFullName(const options_t *) const override { return "powerpc-linux.kernel.vmlinux"; }
    virtual const int *getCompressionMethods(int method, int level) const override;
    virtual const int *getFilters() const override;

protected:
    virtual void buildLoader(const Filter *ft) override;
    virtual void defineDecompressorSymbols() override;
    virtual Linker* newLinker() const override;
    virtual bool is_valid_e_entry(Addr) override;
    virtual bool has_valid_vmlinux_head() override;
    virtual unsigned write_vmlinux_head(
        OutputFile *fo,
        Shdr *stxt
    ) override;
};

class PackVmlinuxPPC64LE : public PackVmlinuxBase<ElfClass_LE64>
{
    typedef PackVmlinuxBase<ElfClass_LE64> super;
public:
    PackVmlinuxPPC64LE(InputFile *f) : super(f, Ehdr::EM_PPC64,
        Ehdr::ELFCLASS64, Ehdr::ELFDATA2LSB, "_vmlinux_start") { }
    virtual int getFormat() const override { return UPX_F_VMLINUX_PPC64LE; }
    virtual const char *getName() const override { return "vmlinux/ppc64le"; }
    virtual const char *getFullName(const options_t *) const override { return "powerpc64le-linux.kernel.vmlinux"; }
    virtual const int *getCompressionMethods(int method, int level) const override;
    virtual const int *getFilters() const override;

protected:
    virtual void buildLoader(const Filter *ft) override;
    virtual void defineDecompressorSymbols() override;
    virtual Linker* newLinker() const override;
    virtual bool is_valid_e_entry(Addr) override;
    virtual bool has_valid_vmlinux_head() override;
    virtual unsigned write_vmlinux_head(
        OutputFile *fo,
        Shdr *stxt
    ) override;
};


class PackVmlinuxAMD64 : public PackVmlinuxBase<ElfClass_LE64>
{
    typedef PackVmlinuxBase<ElfClass_LE64> super;
public:
    PackVmlinuxAMD64(InputFile *f) : super(f, Ehdr::EM_X86_64,
        Ehdr::ELFCLASS64, Ehdr::ELFDATA2LSB, "startup_32") { }
    virtual int getFormat() const override { return UPX_F_VMLINUX_AMD64; }
    virtual const char *getName() const override { return "vmlinux/amd64"; }
    virtual const char *getFullName(const options_t *) const override { return "amd64-linux.kernel.vmlinux"; }
    virtual const int *getCompressionMethods(int method, int level) const override;
    virtual const int *getFilters() const override;

protected:
    virtual void buildLoader(const Filter *ft) override;
    virtual void defineDecompressorSymbols() override;
    virtual Linker* newLinker() const override;
    virtual bool is_valid_e_entry(Addr) override;
    virtual bool has_valid_vmlinux_head() override;
    virtual unsigned write_vmlinux_head(
        OutputFile *fo,
        Shdr *stxt
    ) override;
};


#endif /* already included */

/* vim:set ts=4 sw=4 et: */
