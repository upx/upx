/* linker.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2005 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2005 Laszlo Molnar
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


#include "conf.h"
#include "linker.h"

class LinkerLabel
{
    enum { LINKER_MAX_LABEL_LEN = 32 };
    char label[LINKER_MAX_LABEL_LEN + 1];

public:
    unsigned set(const char *l)
    {
        strncpy(label, l, sizeof(label));
        return strlen(label) + 1;
    }
    operator const char *() const { return label; }
};


struct Linker::section
{
    int         istart;
    int         ostart;
    int         len;
    LinkerLabel name;
};

struct Linker::jump
{
    int         pos;
    int         len;
    int         toffs;
    LinkerLabel tsect;
};

Linker::Linker(const void *pdata, int plen, int pinfo)
{
    iloader = new char[(ilen = plen) + 8192];
    memcpy(iloader,pdata,plen);
    oloader = new char[plen];
    olen = 0;
    align_hack = 0;
    align_offset = 0;
    info = pinfo;
    njumps = nsections = frozen = 0;
    jumps = new jump[200];
#define NSECTIONS 550
    sections = new section[NSECTIONS];

    char *p = iloader + info;
    while (get32(p) != (unsigned)(-1))
    {
        if (get32(p))
        {
            p += sections[nsections].name.set(p);
            sections[nsections].istart = get32(p);
            sections[nsections++].ostart = -1;
            p += 4;
            assert(nsections < NSECTIONS);
        }
        else
        {
            int l;
            for (l = get32(p+4) - 1; iloader[l] == 0; l--)
                ;

            jumps[njumps].pos = l+1;
            jumps[njumps].len = get32(p+4)-jumps[njumps].pos;
            p += 8 + jumps[njumps].tsect.set(p + 8);
            jumps[njumps++].toffs = get32(p);
            p += 4;
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


void Linker::setLoaderAlignOffset(int offset)
{
    align_offset = offset;
}

static int hex(char c)
{
    return (c & 0xf) + (c > '9' ? 9 : 0);
}

int Linker::addSection(const char *psect)
{
    if (psect[0] == 0)
        return olen;
    char *begin = strdup(psect);
    char *end = begin + strlen(begin);
    for (char *sect = begin; sect < end; )
    {
        for (char *tokend = sect; *tokend; tokend++)
            if (*tokend == ' ' || *tokend == ',')
            {
                *tokend = 0;
                break;
            }

        if (*sect == '+') // alignment
        {
            if (sect[1] == '0')
                align_hack = olen + align_offset;
            else
            {
                unsigned j =  hex(sect[1]);
                j = (hex(sect[2]) - ((olen + align_offset) - align_hack) ) % j;
                memset(oloader+olen, (sect[3] == 'C' ? 0x90 : 0), j);
                olen += j;
            }
        }
        else
        {
            int ic;
            for (ic = 0; ic < nsections; ic++)
                if (strcmp(sect, sections[ic].name) == 0)
                {
                    memcpy(oloader+olen,iloader+sections[ic].istart,sections[ic].len);
                    sections[ic].ostart = olen;
                    olen += sections[ic].len;
                    break;
                }
            if (ic == nsections)
                printf("%s", sect);
            assert(ic != nsections);
        }
        sect += strlen(sect) + 1;
    }
    free(begin);
    return olen;
}


void Linker::addSection(const char *sname, const void *sdata, unsigned len)
{
    // add a new section - can be used for adding stuff like ident or header
    sections[nsections].name.set(sname);
    sections[nsections].istart = ilen;
    sections[nsections].len = len;
    sections[nsections++].ostart = olen;
    assert(nsections < NSECTIONS);
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
                if (strcmp(jumps[ic].tsect,sections[kc].name) == 0)
                    break;
            assert(kc!=nsections-1);

            int offs = sections[kc].ostart+jumps[ic].toffs -
                (jumps[ic].pos+jumps[ic].len -
                 sections[jc].istart+sections[jc].ostart);

            if (jumps[ic].len == 1)
                assert(-128 <= offs && offs <= 127);

            set32(&offs,offs);
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
        if (strcmp(name, sections[ic].name) == 0)
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

