/* p_tmt.h --

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

/*************************************************************************
// tmt/adam
**************************************************************************/

class PackTmt final : public Packer {
    typedef Packer super;

public:
    explicit PackTmt(InputFile *f);
    virtual int getVersion() const override { return 13; }
    virtual int getFormat() const override { return UPX_F_TMT_ADAM; }
    virtual const char *getName() const override { return "tmt/adam"; }
    virtual const char *getFullName(const Options *) const override {
        return "i386-dos32.tmt.adam";
    }
    virtual const int *getCompressionMethods(int method, int level) const override;
    virtual const int *getFilters() const override;

    virtual void pack(OutputFile *fo) override;
    virtual void unpack(OutputFile *fo) override;

    virtual tribool canPack() override;
    virtual tribool canUnpack() override;

protected:
    int readFileHeader();

    virtual unsigned findOverlapOverhead(const byte *buf, const byte *tbuf, unsigned range = 0,
                                         unsigned upper_limit = ~0u) const override;
    virtual void buildLoader(const Filter *ft) override;
    virtual Linker *newLinker() const override;

    unsigned adam_offset = 0;
    int big_relocs = 0;

    struct alignas(1) tmt_header_t {
        byte _[16]; // signature,linkerversion,minversion,exesize,imagestart
        LE32 imagesize;
        byte __[4]; // initial memory
        LE32 entry;
        byte ___[12]; // esp,numfixups,flags
        LE32 relocsize;
    };
    tmt_header_t ih, oh;
};

/* vim:set ts=4 sw=4 et: */
