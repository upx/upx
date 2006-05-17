/* linker.h --

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


#ifndef __UPX_LINKER_H
#define __UPX_LINKER_H


class Linker
{
public:
    Linker(const void *pdata, int plen, int pinfo);
    virtual ~Linker();
    int addSection(const char *sect);
    void addSection(const char *sname, const void *sdata, unsigned len);
    const char *getLoader(int *llen);
    int getSection(const char *name, int *slen) const;
    int getLoaderSize() const { return olen; }
    void setLoaderAlignOffset(int phase);

protected:
    // little endian
    virtual unsigned get32(const void *b) const { return get_le32(b); }
    virtual void set32(void *b, unsigned v) const { set_le32(b, v); }

private:
    struct section;
    struct jump;

    char     *iloader, *oloader;
    int      ilen, olen;
    int      info;
    jump     *jumps;
    int      njumps;
    section  *sections;
    int      nsections;
    int      frozen;
    int      align_hack;
    int     align_offset;

private:
    // disable copy and assignment
    Linker(const Linker &); // {}
    Linker& operator= (const Linker &); // { return *this; }
};


class BeLinker : public Linker
{
    typedef Linker super;
public:
    BeLinker(const void *pdata, int plen, int pinfo) :
        super(pdata, plen, pinfo) { }

protected:
    // big endian
    virtual unsigned get32(const void *b) const { return get_be32(b); }
    virtual void set32(void *b, unsigned v) const { set_be32(b, v); }

private:
    // disable copy and assignment
    BeLinker(const BeLinker &); // {}
    BeLinker& operator= (const BeLinker &); // { return *this; }
};


#endif /* already included */


/*
vi:ts=4:et
*/

