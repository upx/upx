/* p_w64pep.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2016 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2016 Laszlo Molnar
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


#ifndef __UPX_P_W64PEP_H
#define __UPX_P_W64PEP_H 1

/*************************************************************************
// w64/pep
**************************************************************************/

class PackW64Pep : public PeFile64
{
    typedef PeFile64 super;

public:
    PackW64Pep(InputFile *f);
    virtual ~PackW64Pep();
    virtual int getFormat() const { return UPX_F_WIN64_PEP; }
    virtual const char *getName() const { return "win64/pe"; }
    virtual const char *getFullName(const options_t *) const { return "amd64-win64.pe"; }
    virtual const int *getCompressionMethods(int method, int level) const;
    virtual const int *getFilters() const;

    virtual bool handleForceOption();
    virtual void defineSymbols(unsigned ncsection, unsigned upxsection,
                               unsigned sizeof_oh, unsigned isize_isplit,
                               Reloc &rel, unsigned s1addr);
    virtual void setOhDataBase(const pe_section_t *) {}
    virtual void setOhHeaderSize(const pe_section_t *osection);
    virtual void pack(OutputFile *fo);

    virtual bool canPack();

protected:
    virtual void buildLoader(const Filter *ft);
    virtual Linker* newLinker() const;
};


#endif /* already included */

/* vim:set ts=4 sw=4 et: */
