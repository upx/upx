/* linker.cpp --

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


#include "conf.h"
#include "linker.h"


struct Linker::section
{
    int  istart;
    int  ostart;
    int  len;
    char name[8];
};

struct Linker::jump
{
    int  pos;
    int  len;
    char tsect[8];
    int  toffs;
};


Linker::Linker(const void *pdata, int plen, int pinfo)
{
    iloader = new char[(ilen = plen) + 4096];
    memcpy(iloader,pdata,plen);
    oloader = new char[plen];
    olen = 0;
    align_hack = 0;
    info = pinfo;
    njumps = nsections = frozen = 0;
    jumps = new jump[200];
    sections = new section[200];

    char *p = iloader + info;
    while (get_le32(p) != (unsigned)(-1))
    {
        if (get_le32(p))
        {
            memcpy(sections[nsections].name,p,8);
            sections[nsections].istart = get_le32(p+8);
            sections[nsections++].ostart = -1;
            p += 12;
            assert(nsections < 200);
        }
        else
        {
            int l;
            for (l = get_le32(p+4) - 1; iloader[l] == 0; l--)
                ;

            jumps[njumps].pos = l+1;
            jumps[njumps].len = get_le32(p+4)-jumps[njumps].pos;
            memcpy(jumps[njumps].tsect,p+8,8);
            jumps[njumps++].toffs = get_le32(p+16);
            p += 20;
            assert(njumps < 200);
        }
    }

    int ic;
    for (ic = 0; ic < nsections - 1; ic++)
        sections[ic].len = sections[ic+1].istart - sections[ic].istart;
    sections[ic].len = 0;
}


Linker::~Linker()
{
    delete [] iloader;
    delete [] oloader;
    delete [] jumps;
    delete [] sections;
}


void Linker::addSection(const char *sect)
{
    int ic;
    while (*sect)
    {
        if (*sect == '+') // alignment
        {
            if (sect[1] == '0')
                align_hack = olen;
            else
            {
                ic = (sect[1] & 0xf) + (sect[1] > '9' ? 9 : 0);
                ic = (ic + (sect[2] & 0xf) + (sect[2] > '9' ? 9 : 0)
                      - (olen - align_hack) % ic) % ic;
                memset(oloader+olen,sect[3] == 'C' ? 0x90 : 0,ic);
                olen += ic;
            }
        }
        else
        {
            for (ic = 0; ic < nsections; ic++)
                if (memcmp(sect,sections[ic].name,8) == 0)
                {
                    memcpy(oloader+olen,iloader+sections[ic].istart,sections[ic].len);
                    sections[ic].ostart = olen;
                    olen += sections[ic].len;
                    break;
                }
            //printf("%8.8s",section);
            assert(ic!=nsections);
        }
        sect += 8;
    }
}


void Linker::addSection(const char *sname, const void *sdata, unsigned len)
{
    // add a new section - can be used for adding stuff like ident or header
    memcpy(sections[nsections].name,sname,8);
    sections[nsections].istart = ilen;
    sections[nsections].len = len;
    sections[nsections++].ostart = olen;
    assert(nsections < 200);
    memcpy(iloader+ilen,sdata,len);
    ilen += len;
}


const char *Linker::getLoader(int *llen)
{
    if (!frozen)
    {
        int ic,jc,kc;
        for (ic = 0; ic < njumps; ic++)
        {
            for (jc = 0; jc < nsections-1; jc++)
                if (jumps[ic].pos >= sections[jc].istart
                    && jumps[ic].pos < sections[jc+1].istart)
                    break;
            assert(jc!=nsections-1);
            if (sections[jc].ostart < 0)
                continue;

            for (kc = 0; kc < nsections-1; kc++)
                if (memcmp(jumps[ic].tsect,sections[kc].name,8) == 0)
                    break;
            assert(kc!=nsections-1);

            int offs = sections[kc].ostart+jumps[ic].toffs -
                (jumps[ic].pos+jumps[ic].len -
                 sections[jc].istart+sections[jc].ostart);

            if (jumps[ic].len == 1)
                assert(-128 <= offs && offs <= 127);

            set_le32(&offs,offs);
            memcpy(oloader+sections[jc].ostart+jumps[ic].pos-sections[jc].istart,&offs,jumps[ic].len);
        }
        frozen=1;
    }
    if (llen) *llen = olen;
    return oloader;
}


int Linker::getSection(const char *name, int *slen) const
{
    if (!frozen)
        return -1;
    for (int ic = 0; ic < nsections; ic++)
        if (memcmp(name,sections[ic].name,8) == 0)
        {
            if (slen)
                *slen = sections[ic].len;
            return sections[ic].ostart;
        }
    return -1;
}


/*
vi:ts=4:et
*/

