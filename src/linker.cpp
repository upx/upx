/* linker.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2006 Laszlo Molnar
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

static int hex(char c)
{
    return (c & 0xf) + (c > '9' ? 9 : 0);
}


/*************************************************************************
//
**************************************************************************/

#define NJUMPS      200
#define NSECTIONS   550

class DefaultLinker::Label
{
    char label[31 + 1];
public:
    unsigned set(const char *s)
    {
        size_t len = strlen(s);
        assert(len > 0); assert(len <= 31);
        strcpy(label, s);
        return len + 1;
    }
    unsigned set(const unsigned char *s) { return set((const char *)s); }
    operator const char *() const { return label; }
};


struct DefaultLinker::Jump
{
    int         pos;
    int         len;
    int         toffs;
    DefaultLinker::Label tsect;
};

struct DefaultLinker::Section
{
    int         istart;
    int         ostart;
    int         len;
    DefaultLinker::Label name;
};


DefaultLinker::DefaultLinker() :
    iloader(NULL), oloader(NULL), jumps(NULL), sections(NULL)
{
}


void DefaultLinker::init(const void *pdata, int plen, int pinfo)
{
    assert(!frozen);
    ilen = plen;
    iloader = new unsigned char[plen + 8192];
    memcpy(iloader, pdata, plen);
    oloader = new unsigned char[plen];
    olen = 0;
    align_hack = 0;
    align_offset = 0;
    info = pinfo;
    njumps = nsections = frozen = 0;
    jumps = new Jump[NJUMPS];
    sections = new Section[NSECTIONS];

    unsigned char *p = iloader + info;
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
            assert(njumps < NJUMPS);
        }
    }

    int ic;
    for (ic = 0; ic < nsections - 1; ic++)
        sections[ic].len = sections[ic+1].istart - sections[ic].istart;
    sections[ic].len = 0;
}


DefaultLinker::~DefaultLinker()
{
    delete [] iloader;
    delete [] oloader;
    delete [] jumps;
    delete [] sections;
}


void DefaultLinker::setLoaderAlignOffset(int offset)
{
    assert(!frozen);
    align_offset = offset;
}


int DefaultLinker::addSection(const char *sname)
{
    assert(!frozen);
    if (sname[0] == 0)
        return olen;
    char *begin = strdup(sname);
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
            if (ic == nsections) {
                printf("%s", sect);
                assert(ic != nsections);
            }
        }
        sect += strlen(sect) + 1;
    }
    free(begin);
    return olen;
}


void DefaultLinker::addSection(const char *sname, const void *sdata, int slen)
{
    assert(!frozen);
    // add a new section - can be used for adding stuff like ident or header
    sections[nsections].name.set(sname);
    sections[nsections].istart = ilen;
    sections[nsections].len = slen;
    sections[nsections++].ostart = olen;
    assert(nsections < NSECTIONS);
    memcpy(iloader+ilen, sdata, slen);
    ilen += slen;
}


void DefaultLinker::freeze()
{
    if (frozen)
        return;

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

    frozen = true;
}


int DefaultLinker::getSection(const char *sname, int *slen)
{
    assert(frozen);

    for (int ic = 0; ic < nsections; ic++)
        if (strcmp(sname, sections[ic].name) == 0)
        {
            if (slen)
                *slen = sections[ic].len;
            return sections[ic].ostart;
        }
    return -1;
}


unsigned char *DefaultLinker::getLoader(int *llen)
{
    assert(frozen);

    if (llen)
        *llen = olen;
    return oloader;
}


/*************************************************************************
//
**************************************************************************/

SimpleLinker::SimpleLinker() :
    oloader(NULL)
{
}


void SimpleLinker::init(const void *pdata, int plen, int pinfo)
{
    assert(!frozen);
    UNUSED(pinfo);
    oloader = new unsigned char[plen];
    olen = plen;
    memcpy(oloader, pdata, plen);
}


SimpleLinker::~SimpleLinker()
{
    delete [] oloader;
}


void SimpleLinker::setLoaderAlignOffset(int offset)
{
    assert(!frozen);
    UNUSED(offset);
    assert(0);
}


int SimpleLinker::addSection(const char *sname)
{
    assert(!frozen);
    UNUSED(sname);
    assert(0);
    return -1;
}


void SimpleLinker::addSection(const char *sname, const void *sdata, int slen)
{
    assert(!frozen);
    UNUSED(sname); UNUSED(sdata); UNUSED(slen);
    assert(0);
}


void SimpleLinker::freeze()
{
    frozen = true;
}


int SimpleLinker::getSection(const char *sname, int *slen)
{
    assert(frozen);
    UNUSED(sname); UNUSED(slen);
    assert(0);
    return -1;
}


unsigned char *SimpleLinker::getLoader(int *llen)
{
    assert(frozen);
    if (llen)
        *llen = olen;
    return oloader;
}


void ElfLinker::preprocessSections(char *start, const char *end)
{
    nsections = 0;
    while (start < end)
    {
        char name[1024];
        unsigned offset, size;

        char *nextl = strchr(start, '\n');
        assert(nextl != NULL);

        if (sscanf(start, "%*d %1023s %x %*d %*d %x",
                   name, &size, &offset) == 3)
        {
            char *n = strstr(start, name);
            n[strlen(name)] = 0;
            addSection(n, input + offset, size);

            //printf("section %s preprocessed\n", n);
        }
        start = nextl + 1;
    }
    addSection("*ABS*", NULL, 0);
    addSection("*UND*", NULL, 0);
}

void ElfLinker::preprocessSymbols(char *start, const char *end)
{
    nsymbols = 0;
    while (start < end)
    {
        char section[1024];
        char symbol[1024];
        unsigned offset;

        char *nextl = strchr(start, '\n');
        assert(nextl != NULL);

        if (sscanf(start, "%x%*8c %1024s %*x %1023s",
                   &offset, section, symbol) == 3)
        {
            char *s = strstr(start, symbol);
            s[strlen(symbol)] = 0;

            assert(nsymbols < TABLESIZE(symbols));
            if (strcmp(section, "*UND*") == 0)
                offset = 0xdeaddead;
            symbols[nsymbols++] = Symbol(s, findSection(section), offset);

            //printf("symbol %s preprocessed o=%x\n", s, offset);
        }

        start = nextl + 1;
    }
}

void ElfLinker::preprocessRelocations(char *start, const char *end)
{
    char sect[1024];
    Section *section = NULL;

    nrelocations = 0;
    while (start < end)
    {
        if (sscanf(start, "RELOCATION RECORDS FOR [%[^]]", sect) == 1)
            section = findSection(sect);

        unsigned offset;
        char type[100];
        char symbol[1024];

        char *nextl = strchr(start, '\n');
        assert(nextl != NULL);

        if (sscanf(start, "%x %99s %1023s",
                   &offset, type, symbol) == 3)
        {
            char *t = strstr(start, type);
            t[strlen(type)] = 0;

            unsigned add = 0;
            if (char *p = strstr(symbol, "+0x"))
            {
                *p = 0;
                sscanf(p + 3, "%x", &add);
            }

            assert(nrelocations < TABLESIZE(relocations));
            relocations[nrelocations++] = Relocation(section, offset, t,
                                                     findSymbol(symbol), add);

            //printf("relocation %s %x preprocessed\n", section->name, offset);
        }

        start = nextl + 1;
    }
}

ElfLinker::Section *ElfLinker::findSection(const char *name)
{
    for (unsigned ic = 0; ic < nsections; ic++)
        if (strcmp(sections[ic].name, name) == 0)
            return sections + ic;

    printf("unknown section %s\n", name);
    abort();
    return NULL;
}

ElfLinker::Symbol *ElfLinker::findSymbol(const char *name)
{
    for (unsigned ic = 0; ic < nsymbols; ic++)
        if (strcmp(symbols[ic].name, name) == 0)
            return symbols + ic;

    printf("unknown symbol %s\n", name);
    abort();
    return NULL;
}

ElfLinker::ElfLinker() : input(NULL), output(NULL), head(NULL), tail(NULL)
{}

ElfLinker::~ElfLinker()
{
    delete [] input;
    delete [] output;
}

void ElfLinker::init(const void *pdata, int plen, int)
{
    upx_byte *i = new upx_byte[plen + 1];
    memcpy(i, pdata, plen);
    input = i;
    inputlen = plen;

    output = new upx_byte[plen];
    outputlen = 0;

    int pos = find(input, plen, "Sections:", 9);
    assert(pos != -1);
    char *psections = pos + (char *) input;

    char *psymbols = strstr(psections, "SYMBOL TABLE:");
    assert(psymbols != NULL);

    char *prelocs = strstr(psymbols, "RELOCATION RECORDS FOR");
    assert(prelocs != NULL);
    input[plen] = 0;

    preprocessSections(psections, psymbols);
    preprocessSymbols(psymbols, prelocs);
    preprocessRelocations(prelocs, (char*) input + inputlen);
    addSection("*UND*");
}

void ElfLinker::setLoaderAlignOffset(int phase)
{
    // FIXME: do not use this yet
    assert(phase & 0);
}

int ElfLinker::addSection(const char *sname)
{
    assert(!frozen);
    if (sname[0] == 0)
        return outputlen;

    char *begin = strdup(sname);
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
            assert(tail);
            if (unsigned l = (hex(sect[2]) - tail->offset - tail->size)
                % hex(sect[1]))
            {
                align(l);
                tail->size += l;
                outputlen += l;
            }
        }
        else
        {
            Section *section = findSection(sect);
            memcpy(output + outputlen, section->input, section->size);
            section->output = output + outputlen;
            outputlen += section->size;
            //printf("section added: %s\n", sect);

            if (head)
            {
                tail->next = section;
                section->offset = tail->offset + tail->size;
            }
            else
                head = section;
            tail = section;
        }
        sect += strlen(sect) + 1;
    }
    free(begin);
    return outputlen;
}

void ElfLinker::addSection(const char *sname, const void *sdata, int slen)
{
    assert(!frozen);
    assert(nsections < TABLESIZE(sections));
    sections[nsections++] = Section(sname, sdata, slen);
}

void ElfLinker::freeze()
{
    if (frozen)
        return;

    frozen = true;
}

int ElfLinker::getSection(const char *sname, int *slen)
{
    assert(frozen);
    Section *section = findSection(sname);
    if (slen)
        *slen = section->size;
    return section->output - output;
}

upx_byte *ElfLinker::getLoader(int *llen)
{
    assert(frozen);

    if (llen)
        *llen = outputlen;
    return output;
}

void ElfLinker::relocate()
{
    assert(frozen);

    for (unsigned ic = 0; ic < nrelocations; ic++)
    {
        Relocation *rel = relocations + ic;
        if (rel->section->output == NULL)
            continue;
        if (rel->value->section->output == NULL)
        {
            printf("can not apply reloc '%s:%x' without section '%s'\n",
                   rel->section->name, rel->offset,
                   rel->value->section->name);
            abort();
        }

        if (strcmp(rel->value->section->name, "*UND*") == 0 &&
            rel->value->offset == 0xdeaddead)
        {
            printf("undefined symbol '%s' referenced\n", rel->value->name);
            abort();
        }
        unsigned value = rel->value->section->offset +
                         rel->value->offset + rel->add;
        upx_byte *location = rel->section->output + rel->offset;

        relocate1(rel, location, value, rel->type);
    }
}

void ElfLinker::defineSymbol(const char *name, unsigned value)
{
    Symbol *symbol = findSymbol(name);
    if (strcmp(symbol->section->name, "*UND*") == 0) // for undefined symbols
        symbol->offset = value;
    else if (strcmp(symbol->section->name, name) == 0) // for sections
    {
        for (Section *section = symbol->section; section; section = section->next)
        {
            assert(section->offset < value);
            section->offset = value;
            value += section->size;
        }
    }
    else
    {
        printf("symbol '%s' already defined\n", name);
        abort();
    }
}

unsigned ElfLinker::getSymbolOffset(const char *name) const
{
    assert(frozen);
    Symbol *symbol = const_cast<ElfLinker *>(this)->findSymbol(name);
    if (symbol->section->output == NULL)
        return 0xdeaddead;
    return symbol->section->offset + symbol->offset;
}

void ElfLinker::alignWithByte(unsigned len, upx_byte b)
{
    memset(output + outputlen, b, len);
}

void ElfLinker::align(unsigned len)
{
    alignWithByte(len, 0);
}

void ElfLinker::relocate1(Relocation *rel, upx_byte *,
                          unsigned, const char *)
{
    printf("unknown relocation type '%s\n", rel->type);
    abort();
}

void ElfLinkerX86::align(unsigned len)
{
    alignWithByte(len, 0x90);
}

void ElfLinkerX86::relocate1(Relocation *rel, upx_byte *location,
                             unsigned value, const char *type)
{
    if (strncmp(type, "R_386_", 6))
        return super::relocate1(rel, location, value, type);
    type += 6;

    if (strncmp(type, "PC", 2) == 0)
    {
        value -= rel->section->offset + rel->offset;
        type += 2;
    }

    if (strcmp(type, "8") == 0)
        *location += value;
    else if (strcmp(type, "16") == 0)
        set_le16(location, get_le16(location) + value);
    else if (strcmp(type, "32") == 0)
        set_le32(location, get_le32(location) + value);
    else
        super::relocate1(rel, location, value, type);
}

void ElfLinkerArmLE::relocate1(Relocation *rel, upx_byte *location,
                               unsigned value, const char *type)
{
    if (strcmp(type, "R_ARM_PC24") == 0)
    {
        value -= rel->section->offset + rel->offset;
        set_le24(location, get_le24(location) + value / 4);
    }
    else if (strcmp(type, "R_ARM_ABS32") == 0)
    {
        set_le32(location, get_le32(location) + value);
    }
    else if (strcmp(type, "R_ARM_THM_CALL") == 0)
    {
        value -= rel->section->offset + rel->offset;
        value += ((get_le16(location) & 0x7ff) << 12);
        value += (get_le16(location + 2) & 0x7ff) << 1;

        set_le16(location, 0xf000 + ((value >> 12) & 0x7ff));
        set_le16(location + 2, 0xf800 + ((value >> 1)  & 0x7ff));

        //(b, 0xF000 + ((v - 1) / 2) * 0x10000);
        //set_le32(location, get_le32(location) + value / 4);
    }
    else
        super::relocate1(rel, location, value, type);
}


/*
vi:ts=4:et
*/

