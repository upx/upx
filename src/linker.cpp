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

static int hex(unsigned char c)
{
    return (c & 0xf) + (c > '9' ? 9 : 0);
}

static bool update_capacity(unsigned size, unsigned *capacity)
{
    if (size < *capacity)
        return false;
    if (*capacity == 0)
        *capacity = 16;
    while (size >= *capacity)
        *capacity *= 2;
    return true;
}

/*************************************************************************
//
**************************************************************************/

ElfLinker::Section::Section(const char *n, const void *i, unsigned s, unsigned a) :
    name(strdup(n)), output(NULL), size(s), offset(0), align(a), next(NULL)
{
    assert(name);
    input = malloc(s + 1);
    assert(input);
    memcpy(input, i, s);
}

ElfLinker::Section::~Section()
{
    free(name);
    free(input);
}

ElfLinker::Symbol::Symbol(const char *n, Section *s, unsigned o) :
    name(strdup(n)), section(s), offset(o)
{
    assert(name);
}

ElfLinker::Symbol::~Symbol()
{
    free(name);
}

ElfLinker::Relocation::Relocation(const Section *s, unsigned o, const char *t,
                                  const Symbol *v, unsigned a) :
    section(s), offset(o), type(t), value(v), add(a)
{}


void ElfLinker::preprocessSections(char *start, const char *end)
{
    nsections = 0;
    while (start < end)
    {
        char name[1024];
        unsigned offset, size, align;

        char *nextl = strchr(start, '\n');
        assert(nextl != NULL);

        if (sscanf(start, "%*d %1023s %x %*d %*d %x 2**%d",
                   name, &size, &offset, &align) == 4)
        {
            char *n = strstr(start, name);
            n[strlen(name)] = 0;
            addSection(n, input + offset, size, align);

            //printf("section %s preprocessed\n", n);
        }
        start = nextl + 1;
    }
    addSection("*ABS*", NULL, 0, 0);
    addSection("*UND*", NULL, 0, 0);
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

            if (strcmp(section, "*UND*") == 0)
                offset = 0xdeaddead;
            addSymbol(s, section, offset);

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
// Beware: sscanf("0xfffffffffffffffc", "%x", &add) ==> -1 == add
// because conversion stops after 32 bits for a non-long.
// Also, glibc-2.3.5 has a bug: even using "%lx" still gives -1 instead of -4.
                long addend;
                *p = 0;  // terminate the symbol name
                p+=3;
                if ('f'==p[0] && 0==strncmp(p, "ffffffff", 8)) {
                    p+=8;  // workaround a bug in glibc-2.3.5
                }
                sscanf(p, "%lx", &addend);
                add = (unsigned)addend;
            }

            addRelocation(section->name, offset, t, symbol, add);

            //printf("relocation %s %x preprocessed\n", section->name, offset);
        }

        start = nextl + 1;
    }
}

ElfLinker::Section *ElfLinker::findSection(const char *name) const
{
    for (unsigned ic = 0; ic < nsections; ic++)
        if (strcmp(sections[ic]->name, name) == 0)
            return sections[ic];

    printf("unknown section %s\n", name);
    abort();
    return NULL;
}

ElfLinker::Symbol *ElfLinker::findSymbol(const char *name) /*const*/
{
    for (unsigned ic = 0; ic < nsymbols; ic++)
        if (strcmp(symbols[ic]->name, name) == 0)
            return symbols[ic];

    // FIXME: this should be a const method !
    if ('*' == name[0]) // *ABS*
        return addSymbol(name, name, 0);

    printf("unknown symbol %s\n", name);
    abort();
    return NULL;
}

ElfLinker::Symbol *ElfLinker::addSymbol(const char *name, const char *section,
                          unsigned offset)
{
    if (update_capacity(nsymbols, &nsymbols_capacity))
        symbols = static_cast<Symbol **>(realloc(symbols, nsymbols_capacity * sizeof(Symbol *)));
    assert(symbols != NULL);
    Symbol *sym = new Symbol(name, findSection(section), offset);
    symbols[nsymbols++] = sym;
    return sym;
}

ElfLinker::Relocation *ElfLinker::addRelocation(const char *section, unsigned off,
                                                const char *type, const char *symbol,
                                                unsigned add)
{
    if (update_capacity(nrelocations, &nrelocations_capacity))
        relocations = static_cast<Relocation **>(realloc(relocations, (nrelocations_capacity) * sizeof(Relocation *)));
    assert(relocations != NULL);
    Relocation *rel = new Relocation(findSection(section), off,
                                     type, findSymbol(symbol), add);
    relocations[nrelocations++] = rel;
    return rel;
}

ElfLinker::ElfLinker() :
    frozen(false), input(NULL), output(NULL), head(NULL), tail(NULL),
    sections(NULL), symbols(NULL), relocations(NULL),
    nsections(0), nsections_capacity(0),
    nsymbols(0), nsymbols_capacity(0),
    nrelocations(0), nrelocations_capacity(0)
{
}

ElfLinker::~ElfLinker()
{
    delete [] input;
    delete [] output;

    unsigned ic;
    for (ic = 0; ic < nsections; ic++)
        delete sections[ic];
    free(sections);
    for (ic = 0; ic < nsymbols; ic++)
        delete symbols[ic];
    free(symbols);
    for (ic = 0; ic < nrelocations; ic++)
        delete relocations[ic];
    free(relocations);
}

void ElfLinker::init(const void *pdata, int plen)
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
    addLoader("*UND*");
}

void ElfLinker::setLoaderAlignOffset(int phase)
{
    //assert(phase & 0);
    printf("\nFIXME: ElfLinker::setLoaderAlignOffset %d\n", phase);
}

int ElfLinker::addLoader(const char *sname)
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
            if (section->align) {
                unsigned const v = ~0u << section->align;
                assert(tail);
                if (unsigned const l = ~v & -(tail->offset + tail->size)) {
                    align(l);
                    tail->size += l;
                    outputlen += l;
                }
            }
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

void ElfLinker::addSection(const char *sname, const void *sdata, int slen, int align)
{
    assert(!frozen);
    if (update_capacity(nsections, &nsections_capacity))
        sections = static_cast<Section **>(realloc(sections, nsections_capacity * sizeof(Section *)));
    assert(sections);
    Section *sec = new Section(sname, sdata, slen, align);
    sections[nsections++] = sec;
    //return sec;
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
        Relocation *rel = relocations[ic];
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

void ElfLinker::relocate1(const Relocation *rel, upx_byte *,
                          unsigned, const char *)
{
    printf("unknown relocation type '%s\n", rel->type);
    abort();
}


/*************************************************************************
// ElfLinker arch subclasses
**************************************************************************/

void ElfLinkerAMD64::relocate1(const Relocation *rel, upx_byte *location,
                               unsigned value, const char *type)
{
    if (strncmp(type, "R_X86_64_", 9))
        return super::relocate1(rel, location, value, type);
    type += 9;

    if (strncmp(type, "PC", 2) == 0)
    {
        value -= rel->section->offset + rel->offset;
        type += 2;
    }

    if (strcmp(type, "8") == 0)
    {
        int displ = (signed char) *location + (int) value;
        if (displ < -127 || displ > 128)
        {
            printf("target out of range (%d) in reloc %s:%x\n",
                   displ, rel->section->name, rel->offset);
            abort();
        }
        *location += value;
    }
    else if (strcmp(type, "16") == 0)
        set_le16(location, get_le16(location) + value);
    else if (strcmp(type, "32") == 0)
        set_le32(location, get_le32(location) + value);
    else
        super::relocate1(rel, location, value, type);
}


void ElfLinkerArmBE::relocate1(const Relocation *rel, upx_byte *location,
                               unsigned value, const char *type)
{
    if (strcmp(type, "R_ARM_PC24") == 0)
    {
        value -= rel->section->offset + rel->offset;
        set_be24(1+ location, get_be24(1+ location) + value / 4);
    }
    else if (strcmp(type, "R_ARM_ABS32") == 0)
    {
        set_be32(location, get_be32(location) + value);
    }
    else if (strcmp(type, "R_ARM_THM_CALL") == 0)
    {
        value -= rel->section->offset + rel->offset;
        value += ((get_be16(location) & 0x7ff) << 12);
        value += (get_be16(location + 2) & 0x7ff) << 1;

        set_be16(location, 0xf000 + ((value >> 12) & 0x7ff));
        set_be16(location + 2, 0xf800 + ((value >> 1)  & 0x7ff));

        //(b, 0xF000 + ((v - 1) / 2) * 0x10000);
        //set_be32(location, get_be32(location) + value / 4);
    }
    else
        super::relocate1(rel, location, value, type);
}


void ElfLinkerArmLE::relocate1(const Relocation *rel, upx_byte *location,
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


void ElfLinkerM68k::relocate1(const Relocation *rel, upx_byte *location,
                              unsigned value, const char *type)
{
    if (strncmp(type, "R_68K_", 6))
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
        set_be16(location, get_be16(location) + value);
    else if (strcmp(type, "32") == 0)
        set_be32(location, get_be32(location) + value);
    else
        super::relocate1(rel, location, value, type);
}


void ElfLinkerPpc32::relocate1(const Relocation *rel, upx_byte *location,
                               unsigned value, const char *type)
{
    if (strncmp(type, "R_PPC_", 6))
        return super::relocate1(rel, location, value, type);
    type += 6;

    if (strncmp(type, "REL", 3) == 0)
    {
        value -= rel->section->offset + rel->offset;
        type += 3;
    }

    // FIXME: more relocs

    // Note that original (*location).displ is ignored.
    if (strcmp(type, "24") == 0) {
        if (3& value) {
            printf("unaligned word diplacement");
            abort();
        }
        // FIXME: displacment overflow?
        set_be32(location, (0xfc000003 & get_be32(location)) +
            (0x03fffffc & value));
    }
    else if (strcmp(type, "14") == 0) {
        if (3& value) {
            printf("unaligned word diplacement");
            abort();
        }
        // FIXME: displacment overflow?
        set_be32(location, (0xffff0003 & get_be32(location)) +
            (0x0000fffc & value));
    }
    else
        super::relocate1(rel, location, value, type);
}


void ElfLinkerX86::relocate1(const Relocation *rel, upx_byte *location,
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
    {
        int displ = (signed char) *location + (int) value;
        if (displ < -127 || displ > 128)
        {
            printf("target out of range (%d) in reloc %s:%x\n",
                   displ, rel->section->name, rel->offset);
            abort();
        }
        *location += value;
    }
    else if (strcmp(type, "16") == 0)
        set_le16(location, get_le16(location) + value);
    else if (strcmp(type, "32") == 0)
        set_le32(location, get_le32(location) + value);
    else
        super::relocate1(rel, location, value, type);
}


/*
vi:ts=4:et
*/

