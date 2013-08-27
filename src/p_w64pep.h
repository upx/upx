/* p_w64pep.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2013 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2013 Laszlo Molnar
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
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
 */


#ifndef __UPX_P_W64PEP_H
#define __UPX_P_W64PEP_H 1

/*************************************************************************
// w64/pep
**************************************************************************/

class PackW64Pep : public PepFile
{
    typedef PepFile super;

public:
    PackW64Pep(InputFile *f);
    virtual ~PackW64Pep();
    virtual int getFormat() const { return UPX_F_WIN64_PEP; }
    virtual const char *getName() const { return "win64/pe"; }
    virtual const char *getFullName(const options_t *) const { return "amd64-win64.pe"; }
    virtual const int *getCompressionMethods(int method, int level) const;
    virtual const int *getFilters() const;

    virtual void pack(OutputFile *fo);

    virtual bool canPack();
    virtual int canUnpack();

protected:
    virtual int readFileHeader();

    virtual void buildLoader(const Filter *ft);
    virtual Linker* newLinker() const;

    virtual unsigned processImports();
    virtual void processImports(unsigned, unsigned);
    virtual void rebuildImports(upx_byte *&);

    virtual void processTls(Interval *); //NEW: TLS callback handling - Stefan Widmann
    void processTls(Reloc *, const Interval *, unsigned); //NEW: TLS callback handling - Stefan Widmann

    void processLoadConf(Reloc *, const Interval *, unsigned);
    void processLoadConf(Interval *);
    upx_byte *oloadconf;
    unsigned soloadconf;

    unsigned tlscb_ptr; //NEW: TLS callback handling - Stefan Widmann
    unsigned tls_handler_offset;

    bool isrtm;
    bool use_dep_hack;
    bool use_clear_dirty_stack;
    bool use_tls_callbacks;  //NEW: TLS callback handling - Stefan Widmann
};


#endif /* already included */


/*
vi:ts=4:et
*/
