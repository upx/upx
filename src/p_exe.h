/* p_exe.h --

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


#ifndef __UPX_P_EXE_H
#define __UPX_P_EXE_H


/*************************************************************************
// dos/exe
**************************************************************************/

class PackExe : public Packer
{
    typedef Packer super;
public:
    PackExe(InputFile *f);
    virtual int getVersion() const { return 13; }
    virtual int getFormat() const { return UPX_F_DOS_EXE; }
    virtual const char *getName() const { return "dos/exe"; }
    virtual const int *getCompressionMethods(int method, int level) const;
    virtual const int *getFilters() const;

    virtual void pack(OutputFile *fo);
    virtual void unpack(OutputFile *fo);

    virtual bool canPack();
    virtual int canUnpack();

    // unpacker capabilities
    virtual bool canUnpackVersion(int version) const
    {
        // NOTE: could adapt p_exe.cpp to support (version >= 8)
        return (version >= 10);
    }
    virtual bool canUnpackFormat(int format) const
    {
        return (format == UPX_F_DOS_EXE || format == UPX_F_DOS_EXEH);
    }

protected:
    struct exe_header_t;

    virtual int readFileHeader(void);

    virtual int fillExeHeader(struct exe_header_t *) const;
    virtual int buildLoader(const Filter *ft);

    struct exe_header_t
    {
        LE16 ident;
        LE16 m512;
        LE16 p512;
        LE16 relocs;
        LE16 headsize16;
        LE16 min;
        LE16 max;
        LE16 ss;
        LE16 sp;
        char _[2];              // checksum
        LE16 ip;
        LE16 cs;
        LE16 relocoffs;
        char __[2];             // overlnum
        LE32 firstreloc;
    }
    __attribute_packed;

    exe_header_t ih, oh;

    unsigned ih_exesize;
    unsigned ih_imagesize;
    unsigned ih_overlay;
    unsigned relocsize;

    bool has_9a;
    bool device_driver;

    enum {
        NORELOC = 1,
        USEJUMP = 2,
        SS = 4,
        SP = 8,
        MINMEM = 16,
        MAXMEM = 32
    };
};


#endif /* already included */


/*
vi:ts=4:et
*/

