/* p_w32pe_i386.h --

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
// win32/pe (i386)
**************************************************************************/

class PackW32PeI386 final : public PeFile32 {
    typedef PeFile32 super;

public:
    explicit PackW32PeI386(InputFile *f);
    virtual ~PackW32PeI386() noexcept;
    virtual int getFormat() const override { return UPX_F_W32PE_I386; }
    virtual const char *getName() const override { return isrtm ? "rtm32/pe" : "win32/pe"; }
    virtual const char *getFullName(const Options *) const override { return "i386-win32.pe"; }
    virtual const int *getCompressionMethods(int method, int level) const override;
    virtual const int *getFilters() const override;

    virtual bool needForceOption() const override;
    virtual void defineSymbols(unsigned ncsection, unsigned upxsection, unsigned sizeof_oh,
                               unsigned isize_isplit, unsigned s1addr) override;
    virtual void addNewRelocations(Reloc &, unsigned upxsection) override;
    virtual void setOhDataBase(const pe_section_t *osection) override;
    virtual void setOhHeaderSize(const pe_section_t *osection) override;
    virtual void pack(OutputFile *fo) override;

    virtual tribool canPack() override;

protected:
    virtual int readFileHeader() override;

    virtual void buildLoader(const Filter *ft) override;
    virtual Linker *newLinker() const override;
};

/* vim:set ts=4 sw=4 et: */
