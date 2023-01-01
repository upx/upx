/* p_djgpp2.h --

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

#pragma once
#ifndef UPX_P_DJGPP2_H__
#define UPX_P_DJGPP2_H__ 1

/*************************************************************************
// djgpp2/coff
**************************************************************************/

class PackDjgpp2 final : public Packer {
    typedef Packer super;

public:
    PackDjgpp2(InputFile *f);
    virtual int getVersion() const override { return 14; }
    virtual int getFormat() const override { return UPX_F_DJGPP2_COFF; }
    virtual const char *getName() const override { return "djgpp2/coff"; }
    virtual const char *getFullName(const options_t *) const override {
        return "i386-dos32.djgpp2.coff";
    }
    virtual const int *getCompressionMethods(int method, int level) const override;
    virtual const int *getFilters() const override;

    virtual void pack(OutputFile *fo) override;
    virtual void unpack(OutputFile *fo) override;

    virtual bool canPack() override;
    virtual int canUnpack() override;

protected:
    void handleStub(OutputFile *fo);
    int readFileHeader();

    virtual unsigned findOverlapOverhead(const upx_bytep buf, const upx_bytep tbuf,
                                         unsigned range = 0,
                                         unsigned upper_limit = ~0u) const override;
    virtual void buildLoader(const Filter *ft) override;
    virtual Linker *newLinker() const override;

    unsigned coff_offset;

    struct alignas(1) external_scnhdr_t {
        char _[12]; // name, paddr
        LE32 vaddr;
        LE32 size;
        LE32 scnptr;
        char misc[12]; // relptr, lnnoptr, nreloc, nlnno
        char __[4];    // flags
    };

    struct alignas(1) coff_header_t {
        // ext_file_hdr
        LE16 f_magic;
        LE16 f_nscns;
        char _[4]; // f_timdat
        LE32 f_symptr;
        LE32 f_nsyms;
        char __[2]; // f_opthdr
        LE16 f_flags;

        // aout_hdr
        LE16 a_magic;
        char ___[2]; // a_vstamp
        LE32 a_tsize;
        LE32 a_dsize;
        char ____[4]; //  a_bsize
        LE32 a_entry;
        char _____[8]; // a_text_start a_data_start

        // section headers
        external_scnhdr_t sh[3];
    };

    coff_header_t coff_hdr;
    external_scnhdr_t *text, *data, *bss;

    void stripDebug();
};

#endif /* already included */

/* vim:set ts=4 sw=4 et: */
