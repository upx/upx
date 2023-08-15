/* p_w64pe_amd64.h --

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
// win64/pe (amd64)
**************************************************************************/

class PackW64PeAmd64 final : public PeFile64 {
    typedef PeFile64 super;

public:
    explicit PackW64PeAmd64(InputFile *f);
    virtual ~PackW64PeAmd64() noexcept;
    virtual int getFormat() const override { return UPX_F_W64PE_AMD64; }
    virtual const char *getName() const override { return "win64/pe"; }
    virtual const char *getFullName(const Options *) const override { return "amd64-win64.pe"; }
    virtual const int *getCompressionMethods(int method, int level) const override;
    virtual const int *getFilters() const override;

    virtual bool needForceOption() const override;
    virtual void defineSymbols(unsigned ncsection, unsigned upxsection, unsigned sizeof_oh,
                               unsigned isize_isplit, unsigned s1addr) override;
    virtual void setOhDataBase(const pe_section_t *) override {}
    virtual void setOhHeaderSize(const pe_section_t *osection) override;
    virtual void pack(OutputFile *fo) override;

    virtual tribool canPack() override;

protected:
    virtual void buildLoader(const Filter *ft) override;
    virtual Linker *newLinker() const override;
};

/* vim:set ts=4 sw=4 et: */
