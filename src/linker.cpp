/* linker.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2016 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2016 Laszlo Molnar
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
   <markus@oberhumer.com>               <ezerotven+github@gmail.com>
 */


#include "conf.h"
#include "linker.h"

static unsigned hex(unsigned char c)
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

static void __acc_cdecl_va internal_error(const char *format, ...)
{
    static char buf[1024];
    va_list ap;

    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);

    throwInternalError(buf);
}


/*************************************************************************
// Section
**************************************************************************/

ElfLinker::Section::Section(const char *n, const void *i, unsigned s, unsigned a) :
    name(NULL), output(NULL), size(s), offset(0), p2align(a), next(NULL)
{
    name = strdup(n);
    assert(name != NULL);
    input = malloc(s + 1);
    assert(input != NULL);
    if (s != 0)
        memcpy(input, i, s);
    ((char *)input)[s] = 0;
}

ElfLinker::Section::~Section()
{
    free(name);
    free(input);
}


/*************************************************************************
// Symbol
**************************************************************************/

ElfLinker::Symbol::Symbol(const char *n, Section *s, upx_uint64_t o) :
    name(NULL), section(s), offset(o)
{
    name = strdup(n);
    assert(name != NULL);
    assert(section != NULL);
}

ElfLinker::Symbol::~Symbol()
{
    free(name);
}


/*************************************************************************
// Relocation
**************************************************************************/

ElfLinker::Relocation::Relocation(const Section *s, unsigned o, const char *t,
                                  const Symbol *v, upx_uint64_t a) :
    section(s), offset(o), type(t), value(v), add(a)
{
    assert(section != NULL);
}


/*************************************************************************
// ElfLinker
**************************************************************************/

ElfLinker::ElfLinker() :
    bele(&N_BELE_RTP::le_policy),
    input(NULL), output(NULL), head(NULL), tail(NULL),
    sections(NULL), symbols(NULL), relocations(NULL),
    nsections(0), nsections_capacity(0),
    nsymbols(0), nsymbols_capacity(0),
    nrelocations(0), nrelocations_capacity(0),
    reloc_done(false)
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

void ElfLinker::init(const void *pdata_v, int plen)
{
    const upx_byte *pdata = (const upx_byte *) pdata_v;
    if (plen >= 16 && memcmp(pdata, "UPX#", 4) == 0)
    {
        // decompress pre-compressed stub-loader
        int method;
        unsigned u_len, c_len;
        if (pdata[4])
        {
            method = pdata[4];
            u_len = get_le16(pdata + 5);
            c_len = get_le16(pdata + 7);
            pdata += 9;
            assert(9 + c_len == (unsigned) plen);
        }
        else
        {
            method = pdata[5];
            u_len = get_le32(pdata + 6);
            c_len = get_le32(pdata + 10);
            pdata += 14;
            assert(14 + c_len == (unsigned) plen);
        }
        assert((unsigned) plen < u_len);
        inputlen = u_len;
        input = new upx_byte[inputlen + 1];
        unsigned new_len = u_len;
        int r = upx_decompress(pdata, c_len, input, &new_len, method, NULL);
        if (r == UPX_E_OUT_OF_MEMORY)
            throwOutOfMemoryException();
        if (r != UPX_E_OK || new_len != u_len)
            throwBadLoader();
    }
    else
    {
        inputlen = plen;
        input = new upx_byte[inputlen + 1];
        if (inputlen)
            memcpy(input, pdata, inputlen);
    }
    input[inputlen] = 0; // NUL terminate

    output = new upx_byte[inputlen ? inputlen : 0x4000];
    outputlen = 0;

    if ((int)strlen("Sections:\n" "SYMBOL TABLE:\n" "RELOCATION RECORDS FOR ") < inputlen) {
        int pos = find(input, inputlen, "Sections:\n", 10);
        assert(pos != -1);
        char *psections = (char *) input + pos;

        char *psymbols = strstr(psections, "SYMBOL TABLE:\n");
        assert(psymbols != NULL);

        char *prelocs = strstr(psymbols, "RELOCATION RECORDS FOR ");
        assert(prelocs != NULL);

        preprocessSections(psections, psymbols);
        preprocessSymbols(psymbols, prelocs);
        preprocessRelocations(prelocs, (char*) input + inputlen);
        addLoader("*UND*");
    }
}

void ElfLinker::preprocessSections(char *start, char *end)
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

void ElfLinker::preprocessSymbols(char *start, char *end)
{
    nsymbols = 0;
    while (start < end)
    {
        char section[1024];
        char symbol[1024];
        unsigned value, offset;

        char *nextl = strchr(start, '\n');
        assert(nextl != NULL);

        if (sscanf(start, "%x g *ABS* %x %1023s",
                   &value, &offset, symbol) == 3)
        {
            char *s = strstr(start, symbol);
            s[strlen(symbol)] = 0;
            addSymbol(s, "*ABS*", value);
            assert(offset == 0);
        }
#if 0
        else if (sscanf(start, "%x%*8c %1023s %*x %1023s",
#else
        // work around broken scanf() implementations
        // http://bugs.winehq.org/show_bug.cgi?id=10401 (fixed in Wine 0.9.58)
        else if (sscanf(start, "%x%*c%*c%*c%*c%*c%*c%*c%*c %1023s %*x %1023s",
#endif
                        &offset, section, symbol) == 3)
        {
            char *s = strstr(start, symbol);
            s[strlen(symbol)] = 0;
            if (strcmp(section, "*UND*") == 0)
                offset = 0xdeaddead;
            assert(strcmp(section, "*ABS*") != 0);
            addSymbol(s, section, offset);
        }

        start = nextl + 1;
    }
}

void ElfLinker::preprocessRelocations(char *start, char *end)
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

            upx_uint64_t add = 0;
            char *p = strstr(symbol, "+0x");
            if (p == NULL)
                p = strstr(symbol, "-0x");
            if (p)
            {
                char sign = *p;
                *p = 0;  // terminate the symbol name
                p += 3;
                assert(strlen(p) == 8 || strlen(p) == 16);
#if (ACC_CC_MSC && (_MSC_VER < 1800))
                unsigned a = 0, b = 0;
                if (sscanf(p, "%08x%08x", &a, &b) == 2)
                    add = ((upx_uint64_t)a << 32) | b;
                else
                    add = a;
#else
                char *endptr = NULL;
                add = strtoull(p, &endptr, 16);
                assert(endptr && *endptr == '\0');
#endif
                if (sign == '-')
                    add = 0 - add;
            }

            addRelocation(section->name, offset, t, symbol, add);
            //printf("relocation %s %s %x %llu preprocessed\n", section->name, symbol, offset, (unsigned long long) add);
        }

        start = nextl + 1;
    }
}

ElfLinker::Section *ElfLinker::findSection(const char *name, bool fatal) const
{
    for (unsigned ic = 0; ic < nsections; ic++)
        if (strcmp(sections[ic]->name, name) == 0)
            return sections[ic];
    if (fatal)
        internal_error("unknown section %s\n", name);
    return NULL;
}

ElfLinker::Symbol *ElfLinker::findSymbol(const char *name, bool fatal) const
{
    for (unsigned ic = 0; ic < nsymbols; ic++)
        if (strcmp(symbols[ic]->name, name) == 0)
            return symbols[ic];
    if (fatal)
        internal_error("unknown symbol %s\n", name);
    return NULL;
}

ElfLinker::Section *ElfLinker::addSection(const char *sname, const void *sdata, int slen, unsigned p2align)
{
    //printf("addSection: %s len=%d align=%d\n", sname, slen, p2align);
    if (update_capacity(nsections, &nsections_capacity))
        sections = static_cast<Section **>(realloc(sections, nsections_capacity * sizeof(Section *)));
    assert(sections);
    assert(sname); assert(sname[0]); assert(sname[strlen(sname)-1] != ':');
    assert(findSection(sname, false) == NULL);
    Section *sec = new Section(sname, sdata, slen, p2align);
    sections[nsections++] = sec;
    return sec;
}

ElfLinker::Symbol *ElfLinker::addSymbol(const char *name, const char *section,
                                        upx_uint64_t offset)
{
    //printf("addSymbol: %s %s 0x%x\n", name, section, offset);
    if (update_capacity(nsymbols, &nsymbols_capacity))
        symbols = static_cast<Symbol **>(realloc(symbols, nsymbols_capacity * sizeof(Symbol *)));
    assert(symbols != NULL);
    assert(name); assert(name[0]); assert(name[strlen(name)-1] != ':');
    assert(findSymbol(name, false) == NULL);
    Symbol *sym = new Symbol(name, findSection(section), offset);
    symbols[nsymbols++] = sym;
    return sym;
}

ElfLinker::Relocation *ElfLinker::addRelocation(const char *section, unsigned off,
                                                const char *type, const char *symbol,
                                                upx_uint64_t add)
{
    if (update_capacity(nrelocations, &nrelocations_capacity))
        relocations = static_cast<Relocation **>(realloc(relocations, (nrelocations_capacity) * sizeof(Relocation *)));
    assert(relocations != NULL);
    Relocation *rel = new Relocation(findSection(section), off,
                                     type, findSymbol(symbol), add);
    relocations[nrelocations++] = rel;
    return rel;
}

#if 0
void ElfLinker::setLoaderAlignOffset(int phase)
{
    //assert(phase & 0);
    printf("\nFIXME: ElfLinker::setLoaderAlignOffset %d\n", phase);
}
#endif

int ElfLinker::addLoader(const char *sname)
{
    assert(sname != NULL);
    if (!sname[0])
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

        if (sect[0] == '+') // alignment
        {
            assert(tail);
            unsigned l = (hex(sect[2]) - tail->offset - tail->size) % hex(sect[1]);
            if (l)
            {
                if (sect[3] == 'D')
                    alignData(l);
                else
                    alignCode(l);
                tail->size += l;
            }
        }
        else
        {
            Section *section = findSection(sect);
            if (section->p2align) {
                assert(tail);
                assert(tail != section);
                unsigned const v = ~0u << section->p2align;
                if (unsigned const l = ~v & (0u-(tail->offset + tail->size))) {
                    alignCode(l);
                    tail->size += l;
                }
            }
            memcpy(output + outputlen, section->input, section->size);
            section->output = output + outputlen;
            //printf("section added: 0x%04x %3d %s\n", outputlen, section->size, section->name);
            outputlen += section->size;

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

void ElfLinker::addLoader(const char *s, va_list ap)
{
    while (s != NULL)
    {
        addLoader(s);
        s = va_arg(ap, const char *);
    }
}

void __acc_cdecl_va ElfLinker::addLoaderVA(const char *s, ...)
{
    va_list ap;
    va_start(ap, s);
    addLoader(s, ap);
    va_end(ap);
}

int ElfLinker::getSection(const char *sname, int *slen) const
{
    const Section *section = findSection(sname);
    if (slen)
        *slen = section->size;
    return (int) (section->output - output);
}

int ElfLinker::getSectionSize(const char *sname) const
{
    const Section *section = findSection(sname);
    return section->size;
}

upx_byte *ElfLinker::getLoader(int *llen) const
{
    if (llen)
        *llen = outputlen;
    return output;
}

void ElfLinker::relocate()
{
    assert(!reloc_done);
    reloc_done = true;
    for (unsigned ic = 0; ic < nrelocations; ic++)
    {
        const Relocation *rel = relocations[ic];
        upx_uint64_t value = 0;

        if (rel->section->output == NULL)
            continue;
        if (strcmp(rel->value->section->name, "*ABS*") == 0)
        {
            value = rel->value->offset;
        }
        else if (strcmp(rel->value->section->name, "*UND*") == 0 &&
                 rel->value->offset == 0xdeaddead)
            internal_error("undefined symbol '%s' referenced\n", rel->value->name);
        else if (rel->value->section->output == NULL)
            internal_error("can not apply reloc '%s:%x' without section '%s'\n",
                           rel->section->name, rel->offset,
                           rel->value->section->name);
        else
        {
            value = rel->value->section->offset +
                    rel->value->offset + rel->add;
        }
        upx_byte *location = rel->section->output + rel->offset;
        //printf("%-28s %-28s %-10s 0x%16llx 0x%16llx\n", rel->section->name, rel->value->name, rel->type, (long long) value, (long long) value - rel->section->offset - rel->offset);
        //printf("  %llx %d %llx %d %llx : %d\n", (long long) value, rel->value->section->offset, rel->value->offset, rel->offset, (long long) rel->add, *location);
        relocate1(rel, location, value, rel->type);
    }
}

void ElfLinker::defineSymbol(const char *name, upx_uint64_t value)
{
    Symbol *symbol = findSymbol(name);
    if (strcmp(symbol->section->name, "*ABS*") == 0)
        internal_error("defineSymbol: symbol '%s' is *ABS*\n", name);
    else if (strcmp(symbol->section->name, "*UND*") == 0) // for undefined symbols
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
        internal_error("defineSymbol: symbol '%s' already defined\n", name);
}

// debugging support
void ElfLinker::dumpSymbol(const Symbol *symbol, unsigned flags, FILE *fp) const
{
    if ((flags & 1) && symbol->section->output == NULL)
        return;
    char d0[16+1], d1[16+1];
    upx_snprintf(d0, sizeof(d0), "%016llx", (upx_uint64_t) symbol->offset);
    upx_snprintf(d1, sizeof(d1), "%016llx", (upx_uint64_t) symbol->section->offset);
    fprintf(fp, "%-28s 0x%-16s | %-28s 0x%-16s\n",
            symbol->name, d0, symbol->section->name, d1);
}
void ElfLinker::dumpSymbols(unsigned flags, FILE *fp) const
{
    if (fp == NULL)
        fp = stdout;
    if ((flags & 2) == 0)
    {
        // default: dump symbols in used section order
        for (const Section *section = head; section; section = section->next)
        {
            char d1[16+1];
            upx_snprintf(d1, sizeof(d1), "%016llx", (upx_uint64_t) section->offset);
            fprintf(fp, "%-42s%-28s 0x%-16s\n", "", section->name, d1);
            for (unsigned ic = 0; ic < nsymbols; ic++)
            {
                const Symbol *symbol = symbols[ic];
                if (symbol->section == section && strcmp(symbol->name, section->name) != 0)
                    dumpSymbol(symbol, flags, fp);
            }
        }
    }
    else
    {
        // dump all symbols
        for (unsigned ic = 0; ic < nsymbols; ic++)
            dumpSymbol(symbols[ic], flags, fp);
    }
}

upx_uint64_t ElfLinker::getSymbolOffset(const char *name) const
{
    const Symbol *symbol = findSymbol(name);
    if (symbol->section->output == NULL)
        return 0xdeaddead;
    return symbol->section->offset + symbol->offset;
}

void ElfLinker::alignWithByte(unsigned len, unsigned char b)
{
    memset(output + outputlen, b, len);
    outputlen += len;
}

void ElfLinker::relocate1(const Relocation *rel, upx_byte *,
                          upx_uint64_t, const char *)
{
    internal_error("unknown relocation type '%s\n", rel->type);
}


/*************************************************************************
// ElfLinker arch subclasses
// FIXME: add more displacment overflow checks
// FIXME: add support for our special "ignore_reloc_overflow" section
**************************************************************************/

#if 0 // FIXME
static void check8(const Relocation *rel, const upx_byte *location, int v, int d)
{
    if (v < -128 || v > 127)
        internal_error("value out of range (%d) in reloc %s:%x\n",
                       v, rel->section->name, rel->offset);
    if (d < -128 || d > 127)
        internal_error("displacement target out of range (%d) in reloc %s:%x\n",
                       v, rel->section->name, rel->offset);
}
#endif


void ElfLinkerAMD64::relocate1(const Relocation *rel, upx_byte *location,
                               upx_uint64_t value, const char *type)
{
    if (strncmp(type, "R_X86_64_", 9))
        return super::relocate1(rel, location, value, type);
    type += 9;

    bool range_check = false;
    if (strncmp(type, "PC", 2) == 0)
    {
        value -= rel->section->offset + rel->offset;
        type += 2;
        range_check = true;
    }

    if (strcmp(type, "8") == 0)
    {
        int displ = (signed char) *location + (int) value;
        if (range_check && (displ < -128 || displ > 127))
            internal_error("target out of range (%d) in reloc %s:%x\n",
                           displ, rel->section->name, rel->offset);
        *location += value;
    }
    else if (strcmp(type, "16") == 0)
        set_le16(location, get_le16(location) + value);
    else if (strncmp(type, "32", 2) == 0)  // for "32" and "32S"
        set_le32(location, get_le32(location) + value);
    else if (strcmp(type, "64") == 0)
        set_le64(location, get_le64(location) + value);
    else
        super::relocate1(rel, location, value, type);
}


void ElfLinkerArmBE::relocate1(const Relocation *rel, upx_byte *location,
                               upx_uint64_t value, const char *type)
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
    else if (strcmp(type, "R_ARM_THM_CALL") == 0
         ||  strcmp(type, "R_ARM_THM_XPC22") == 0
         ||  strcmp(type, "R_ARM_THM_PC22") == 0
    )
    {
        value -= rel->section->offset + rel->offset;
        value += ((get_be16(location) & 0x7ff) << 12);
        value += (get_be16(location + 2) & 0x7ff) << 1;

        set_be16(location, 0xf000 + ((value >> 12) & 0x7ff));
        set_be16(location + 2, 0xf800 + ((value >> 1)  & 0x7ff));

        //(b, 0xF000 + ((v - 1) / 2) * 0x10000);
        //set_be32(location, get_be32(location) + value / 4);
    }
    else if (0==strcmp("R_ARM_ABS8", type)) {
        *location += value;
    }
    else
        super::relocate1(rel, location, value, type);
}


void ElfLinkerArmLE::relocate1(const Relocation *rel, upx_byte *location,
                               upx_uint64_t value, const char *type)
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
    else if (strcmp(type, "R_ARM_THM_CALL") == 0
         ||  strcmp(type, "R_ARM_THM_XPC22") == 0
         ||  strcmp(type, "R_ARM_THM_PC22") == 0
    )
    {
        value -= rel->section->offset + rel->offset;
        value += ((get_le16(location) & 0x7ff) << 12);
        value += (get_le16(location + 2) & 0x7ff) << 1;

        set_le16(location, 0xf000 + ((value >> 12) & 0x7ff));
        set_le16(location + 2, 0xf800 + ((value >> 1)  & 0x7ff));

        //(b, 0xF000 + ((v - 1) / 2) * 0x10000);
        //set_le32(location, get_le32(location) + value / 4);
    }
    else if (0==strcmp("R_ARM_ABS8", type)) {
        *location += value;
    }
    else
        super::relocate1(rel, location, value, type);
}

void ElfLinkerArm64LE::relocate1(const Relocation *rel, upx_byte *location,
                               upx_uint64_t value, const char *type)
{
    if (strcmp(type, "R_AARCH64_CALL26") == 0)
    {
        value -= rel->section->offset + rel->offset;
        set_le24(location, get_le24(location) + value / 4);  // FIXME set_le26
    }
    else if (strcmp(type, "R_AARCH64_ABS32") == 0)
    {
        set_le32(location, get_le32(location) + value);
    }
    else
        super::relocate1(rel, location, value, type);
}


void ElfLinkerM68k::alignCode(unsigned len)
{
    assert((len & 1) == 0);
    assert((outputlen & 1) == 0);
    for (unsigned i = 0; i < len; i += 2)
        set_be16(output + outputlen + i, 0x4e71); // "nop"
    outputlen += len;
}

void ElfLinkerM68k::relocate1(const Relocation *rel, upx_byte *location,
                              upx_uint64_t value, const char *type)
{
    if (strncmp(type, "R_68K_", 6))
        return super::relocate1(rel, location, value, type);
    type += 6;

    if (strncmp(type, "PC", 2) == 0)
    {
        value -= rel->section->offset + rel->offset;
        type += 2;
    }
    if (strcmp(type, "8") == 0) {
        *location += value;
    }
    else if (strcmp(type, "16") == 0) {
        set_be16(location, get_be16(location) + value);
    }
    else if (strcmp(type, "32") == 0) {
        set_be32(location, get_be32(location) + value);
    }
    else
        super::relocate1(rel, location, value, type);
}


void ElfLinkerMipsBE::relocate1(const Relocation *rel, upx_byte *location,
                                upx_uint64_t value, const char *type)
{
#define MIPS_HI(a)      (((a) >> 16) + (((a) & 0x8000) >> 15))
#define MIPS_LO(a)      ((a) & 0xffff)
#define MIPS_PC16(a)    ((a) >> 2)
#define MIPS_PC26(a)    (((a) & 0x0fffffff) >> 2)

    if (strcmp(type, "R_MIPS_HI16") == 0)
        set_be16(2+location, get_be16(2+location) + MIPS_HI(value));
    else if (strcmp(type, "R_MIPS_LO16") == 0)
        set_be16(2+location, get_be16(2+location) + MIPS_LO(value));
    else if (strcmp(type, "R_MIPS_PC16") == 0)
    {
        value -= rel->section->offset + rel->offset;
        set_be16(2+location, get_be16(2+location) + MIPS_PC16(value));
    }
    else if (strcmp(type, "R_MIPS_26") == 0)
        set_be32(location, get_be32(location) + MIPS_PC26(value));
    else if (strcmp(type, "R_MIPS_32") == 0)
        set_be32(location, get_be32(location) + value);
    else
        super::relocate1(rel, location, value, type);

#undef MIPS_HI
#undef MIPS_LO
#undef MIPS_PC16
#undef MIPS_PC26
}


void ElfLinkerMipsLE::relocate1(const Relocation *rel, upx_byte *location,
                                upx_uint64_t value, const char *type)
{
#define MIPS_HI(a)      (((a) >> 16) + (((a) & 0x8000) >> 15))
#define MIPS_LO(a)      ((a) & 0xffff)
#define MIPS_PC16(a)    ((a) >> 2)
#define MIPS_PC26(a)    (((a) & 0x0fffffff) >> 2)

    if (strcmp(type, "R_MIPS_HI16") == 0)
        set_le16(location, get_le16(location) + MIPS_HI(value));
    else if (strcmp(type, "R_MIPS_LO16") == 0)
        set_le16(location, get_le16(location) + MIPS_LO(value));
    else if (strcmp(type, "R_MIPS_PC16") == 0)
    {
        value -= rel->section->offset + rel->offset;
        set_le16(location, get_le16(location) + MIPS_PC16(value));
    }
    else if (strcmp(type, "R_MIPS_26") == 0)
        set_le32(location, get_le32(location) + MIPS_PC26(value));
    else if (strcmp(type, "R_MIPS_32") == 0)
        set_le32(location, get_le32(location) + value);
    else
        super::relocate1(rel, location, value, type);

#undef MIPS_HI
#undef MIPS_LO
#undef MIPS_PC16
#undef MIPS_PC26
}


void ElfLinkerPpc32::relocate1(const Relocation *rel, upx_byte *location,
                               upx_uint64_t value, const char *type)
{
    if (strncmp(type, "R_PPC_", 6))
        return super::relocate1(rel, location, value, type);
    type += 6;

    if (strcmp(type, "ADDR32") == 0)
    {
        set_be32(location, get_be32(location) + value);
        return;
    }

    if (strncmp(type, "REL", 3) == 0)
    {
        value -= rel->section->offset + rel->offset;
        type += 3;
    }

    // FIXME: more relocs

    // Note that original (*location).displ is ignored.
    if (strcmp(type, "24") == 0) {
        if (3& value)
            internal_error("unaligned word diplacement");
        // FIXME: displacment overflow?
        set_be32(location, (0xfc000003 & get_be32(location)) +
            (0x03fffffc & value));
    }
    else if (strcmp(type, "14") == 0) {
        if (3& value)
            internal_error("unaligned word diplacement");
        // FIXME: displacment overflow?
        set_be32(location, (0xffff0003 & get_be32(location)) +
            (0x0000fffc & value));
    }
    else
        super::relocate1(rel, location, value, type);
}

void ElfLinkerPpc64le::relocate1(const Relocation *rel, upx_byte *location,
                                upx_uint64_t value, const char *type)
{
    if (strncmp(type, "R_PPC64_REL", 11))
        return super::relocate1(rel, location, value, type);
    type += 11;

    bool range_check = false;
    if (strncmp(type, "PC", 2) == 0)
    {
        type += 2;
        range_check = true;
    }

    /* value will hold relative displacement */
    value -= rel->section->offset + rel->offset;

    if (strcmp(type, "8") == 0)
    {
        int displ = (signed char) *location + (int) value;
        if (range_check && (displ < -128 || displ > 127))
            internal_error("target out of range (%d) in reloc %s:%x\n",
                           displ, rel->section->name, rel->offset);
        *location += value;
    }
    else if (strncmp(type, "14", 2) == 0)  // for "32" and "32S"
        set_le16(location, get_le16(location) + value);
    else if (strcmp(type, "16") == 0)
        set_le16(location, get_le16(location) + value);
    else if (strncmp(type, "24", 2) == 0)  // for "32" and "32S"
        set_le24(location, get_le24(location) + value);
    else if (strncmp(type, "32", 2) == 0)  // for "32" and "32S"
        set_le32(location, get_le32(location) + value);
    else if (strcmp(type, "64") == 0)
        set_le64(location, get_le64(location) + value);
    else
        super::relocate1(rel, location, value, type);
}


void ElfLinkerX86::relocate1(const Relocation *rel, upx_byte *location,
                             upx_uint64_t value, const char *type)
{
    if (strncmp(type, "R_386_", 6))
        return super::relocate1(rel, location, value, type);
    type += 6;

    bool range_check = false;
    if (strncmp(type, "PC", 2) == 0)
    {
        value -= rel->section->offset + rel->offset;
        type += 2;
        range_check = true;
    }

    if (strcmp(type, "8") == 0)
    {
        int displ = (signed char) *location + (int) value;
        if (range_check && (displ < -128 || displ > 127))
            internal_error("target out of range (%d,%d,%d) in reloc %s:%x\n",
                           displ, *location, value, rel->section->name, rel->offset);
        *location += value;
    }
    else if (strcmp(type, "16") == 0)
        set_le16(location, get_le16(location) + value);
    else if (strcmp(type, "32") == 0)
        set_le32(location, get_le32(location) + value);
    else
        super::relocate1(rel, location, value, type);
}

/* vim:set ts=4 sw=4 et: */
