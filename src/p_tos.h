/* p_tos.h --

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

   Markus F.X.J. Oberhumer              Laszlo Molnar
   <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>
 */


#ifndef __UPX_P_TOS_H
#define __UPX_P_TOS_H


/*************************************************************************
// atari/tos
**************************************************************************/

class PackTos : public Packer
{
    typedef Packer super;
public:
    PackTos(InputFile *f);
    virtual int getVersion() const { return 13; }
    virtual int getFormat() const { return UPX_F_ATARI_TOS; }
    virtual const char *getName() const { return "atari/tos"; }
    virtual const int *getCompressionMethods(int method, int level) const;
    virtual const int *getFilters() const;

    virtual void pack(OutputFile *fo);
    virtual void unpack(OutputFile *fo);

    virtual bool canPack();
    virtual int canUnpack();

    virtual void fileInfo();

protected:
    virtual const upx_byte *getLoader() const;
    virtual int getLoaderSize() const;

    virtual int readFileHeader();
    virtual bool checkFileHeader();

    struct tos_header_t
    {
        BE16 fh_magic;
        BE32 fh_text;
        BE32 fh_data;
        BE32 fh_bss;
        BE32 fh_sym;
        BE32 fh_reserved;
        BE32 fh_flag;
        BE16 fh_reloc;
    }
    __attribute_packed;

    tos_header_t ih, oh;

protected:
    unsigned patch_d_subq(void *l, int llen, int, unsigned, const char*);
    unsigned patch_d_loop(void *l, int llen, int, unsigned, const char*, const char*);
};


#endif /* already included */


/*
vi:ts=4:et
*/

