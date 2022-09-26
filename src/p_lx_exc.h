/* p_lx_exc.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2022 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2022 Laszlo Molnar
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

   John F. Reiser
   <jreiser@users.sourceforge.net>
 */


#ifndef __UPX_P_LX_EXC_H
#define __UPX_P_LX_EXC_H 1


/*************************************************************************
// linux/386 (generic "execve" format)
**************************************************************************/

class PackLinuxI386 : public PackUnixLe32
{
    typedef PackUnixLe32 super;
public:
    PackLinuxI386(InputFile *f);
    virtual void generateElfHdr(
        OutputFile *,
        void const *proto,
        unsigned const brka
    );
    virtual int getFormat() const override { return UPX_F_LINUX_i386; }
    virtual const char *getName() const override { return "linux.exec/i386"; }
    virtual const char *getFullName(const options_t *) const override { return "i386-linux.elf.execve"; }
    virtual const int *getCompressionMethods(int method, int level) const override;
    virtual const int *getFilters() const override;
    virtual void buildLoader(const Filter *) override;

    virtual bool canPack() override;

protected:
    virtual void pack1(OutputFile *, Filter &) override;  // generate executable header
    // virtual void pack2(OutputFile *, Filter &) override;  // append compressed data
    // virtual void pack3(OutputFile *, Filter &) override;  // append loader
    virtual void pack4(OutputFile *, Filter &) override;  // append PackHeader

    // loader util
    virtual Linker* newLinker() const override;
    virtual int getLoaderPrefixSize() const;
    virtual void buildLinuxLoader(
        upx_byte const *const proto,  // assembly-only sections
        unsigned const szproto,
        upx_byte const *const fold,  // linked assembly + C section
        unsigned const szfold,
        Filter const *ft
    );

    // patch util
    virtual void patchLoader() override;
    virtual void patchLoaderChecksum() override;
    virtual void updateLoader(OutputFile *) override;

    // ELF util
    virtual int checkEhdr(const Elf_LE32_Ehdr *ehdr) const;

    enum {
        UPX_ELF_MAGIC = 0x5850557f          // "\x7fUPX"
    };

    unsigned n_mru;

    __packed_struct(cprElfHdr1)
        Elf_LE32_Ehdr ehdr;
        Elf_LE32_Phdr phdr[1];
        l_info linfo;
    __packed_struct_end()

    __packed_struct(cprElfHdr2)
        Elf_LE32_Ehdr ehdr;
        Elf_LE32_Phdr phdr[2];
        l_info linfo;
    __packed_struct_end()

    __packed_struct(cprElfHdr3)
        Elf_LE32_Ehdr ehdr;
        Elf_LE32_Phdr phdr[3];
        l_info linfo;
    __packed_struct_end()

    cprElfHdr3 elfout;

    struct Elf32_Note {
        unsigned namesz;  // 8
        unsigned descsz;  // 4
        unsigned type;    // 1
        char text[0x18 - 4*4];  // "OpenBSD"
        unsigned end;     // 0
    } elfnote;

    unsigned char ei_osabi;
    char const *osabi_note;

    static void compileTimeAssertions() {
        COMPILE_TIME_ASSERT(sizeof(cprElfHdr1) == 52 + 1*32 + 12)
        COMPILE_TIME_ASSERT(sizeof(cprElfHdr2) == 52 + 2*32 + 12)
        COMPILE_TIME_ASSERT(sizeof(cprElfHdr3) == 52 + 3*32 + 12)
        COMPILE_TIME_ASSERT_ALIGNED1(cprElfHdr1)
        COMPILE_TIME_ASSERT_ALIGNED1(cprElfHdr2)
        COMPILE_TIME_ASSERT_ALIGNED1(cprElfHdr3)
    }
};


class PackBSDI386 : public PackLinuxI386
{
    typedef PackLinuxI386 super;
public:
    PackBSDI386(InputFile *f);
    virtual int getFormat() const override { return UPX_F_BSD_i386; }
    virtual const char *getName() const override { return "bsd.exec/i386"; }
    virtual const char *getFullName(const options_t *) const override { return "i386-bsd.elf.execve"; }

protected:
    virtual void pack1(OutputFile *, Filter &) override;  // generate executable header

    virtual void buildLoader(const Filter *) override;
};
#endif /* already included */

/* vim:set ts=4 sw=4 et: */
