/* p_lx_exc.h --

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
        unsigned const brka
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
    virtual int checkEhdr(const Elf32_Ehdr *ehdr) const;
    virtual off_t getbrk(const Elf32_Phdr *phdr, int e_phnum) const;
    virtual off_t getbase(const Elf32_Phdr *phdr, int e_phnum) const;

    enum {
        UPX_ELF_MAGIC = 0x5850557f          // "\x7fUPX"
    };

    unsigned n_mru;

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


#endif /* already included */


/*
vi:ts=4:et
*/

