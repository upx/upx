/* p_tmt.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
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


#ifndef __UPX_P_TMT_H
#define __UPX_P_TMT_H


/*************************************************************************
// tmt/adam
**************************************************************************/

class PackTmt : public Packer
{
    typedef Packer super;
public:
    PackTmt(InputFile *f);
    virtual int getVersion() const { return 13; }
    virtual int getFormat() const { return UPX_F_TMT_ADAM; }
    virtual const char *getName() const { return "tmt/adam"; }
    virtual const char *getFullName(const options_t *) const { return "i386-dos32.tmt.adam"; }
    virtual const int *getCompressionMethods(int method, int level) const;
    virtual const int *getFilters() const;

    virtual void pack(OutputFile *fo);
    virtual void unpack(OutputFile *fo);

    virtual bool canPack();
    virtual int canUnpack();

protected:
    virtual int readFileHeader();

    virtual unsigned findOverlapOverhead(const upx_bytep buf,
                                         const upx_bytep tbuf,
                                         unsigned range = 0,
                                         unsigned upper_limit = ~0u) const;
    virtual void buildLoader(const Filter *ft);
    virtual Linker* newLinker() const;

    unsigned adam_offset;
    int big_relocs;

    struct tmt_header_t
    {
        char _[16];     // signature,linkerversion,minversion,exesize,imagestart
        LE32 imagesize;
        char __[4];     // initial memory
        LE32 entry;
        char ___[12];   // esp,numfixups,flags
        LE32 relocsize;
    } ih, oh;
};


#endif /* already included */


/*
vi:ts=4:et
*/

