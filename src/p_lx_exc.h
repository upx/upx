/* p_lx_exc.h --

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


#ifndef __UPX_P_LX_EXC_H
#define __UPX_P_LX_EXC_H


/*************************************************************************
// linux/386 (generic "execve" format)
**************************************************************************/

class PackLinuxI386 : public PackUnixLe32
{
    typedef PackUnixLe32 super;
public:
    PackLinuxI386(InputFile *f) : super(f) { }
    virtual void generateElfHdr(
        OutputFile *,
        void const *proto,
        Elf_LE32_Phdr const *const phdr0,
        unsigned e_phnum,
        unsigned brka
    );
    virtual int getFormat() const { return UPX_F_LINUX_i386; }
    virtual const char *getName() const { return "linux/386"; }
    virtual const int *getCompressionMethods(int method, int level) const;
    virtual const int *getFilters() const;
    virtual int buildLoader(const Filter *);

    virtual bool canPack();

protected:
    virtual void pack1(OutputFile *, Filter &);  // generate executable header
    // virtual void pack2(OutputFile *, Filter &);  // append compressed data
    // virtual void pack3(OutputFile *, Filter &);  // append loader
    virtual void pack4(OutputFile *, Filter &);  // append PackHeader

    unsigned find_file_offset(
        unsigned vaddr,
        Elf_LE32_Phdr const *phdr,
        unsigned e_phnum
    );
    Elf_LE32_Phdr const *find_DYNAMIC(Elf_LE32_Phdr const *phdr, unsigned n);

    // loader util
    virtual int getLoaderPrefixSize() const;
    virtual int buildLinuxLoader(
        upx_byte const *const proto,  // assembly-only sections
        unsigned const szproto,
        upx_byte const *const fold,  // linked assembly + C section
        unsigned const szfold,
        Filter const *ft
    );

    // patch util
    virtual void patchLoader();
    virtual void patchLoaderChecksum();
    virtual void updateLoader(OutputFile *);

    // ELF util
    virtual int checkEhdr(const Elf_LE32_Ehdr *ehdr) const;
    virtual off_t getbrk(const Elf_LE32_Phdr *phdr, int e_phnum) const;

    enum {
        UPX_ELF_MAGIC = 0x5850557f          // "\x7fUPX"
    };

    unsigned n_mru;

    struct cprElfHdr1 {
        struct Elf_LE32_Ehdr ehdr;
        struct Elf_LE32_Phdr phdr[1];
        struct PackUnix::l_info linfo;
    };
    struct cprElfHdr2 {
        struct Elf_LE32_Ehdr ehdr;
        struct Elf_LE32_Phdr phdr[2];
        struct PackUnix::l_info linfo;
    };
    struct cprElfHdr3 {
        struct Elf_LE32_Ehdr ehdr;
        struct Elf_LE32_Phdr phdr[3];
        struct PackUnix::l_info linfo;
    };

    cprElfHdr3 elfout;

};


#endif /* already included */


/*
vi:ts=4:et
*/

