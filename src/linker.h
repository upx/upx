/* linker.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2001 Laszlo Molnar
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

   Markus F.X.J. Oberhumer                   Laszlo Molnar
   markus.oberhumer@jk.uni-linz.ac.at        ml1050@cdata.tvnet.hu
 */


#ifndef __UPX_LINKER_H
#define __UPX_LINKER_H


class Linker
{
public:
    Linker(const void *pdata, int plen, int pinfo);
    ~Linker();
    void addSection(const char *sect);
    void addSection(const char *sname, const void *sdata, unsigned len);
    const char *getLoader(int *llen);
    int getSection(const char *name, int *slen) const;

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

private:
    // disable copy and assignment
    Linker(Linker const &); // {}
    Linker& operator= (Linker const &); // { return *this; }
};



#endif /* already included */


/*
vi:ts=4:et
*/

