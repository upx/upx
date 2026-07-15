/* linker.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) Markus Franz Xaver Johannes Oberhumer
   Copyright (C) Laszlo Molnar
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
#define WANT_EHDR_ENUM 1
#include "p_elf_enum.h"
#include "linker.h"

static inline unsigned hex(uchar c) noexcept { return (c & 0xf) + (c > '9' ? 9 : 0); }

static bool grow_capacity(unsigned size, unsigned *capacity) noexcept {
    if (size < *capacity)
        return false;
    if (*capacity == 0)
        *capacity = 16;
    while (size >= *capacity)
        *capacity *= 2;
    return true;
}

template <class T>
static noinline T **realloc_array(T **array, size_t capacity) may_throw {
    assert_noexcept(capacity > 0);
    // NOLINTNEXTLINE(bugprone-multi-level-implicit-pointer-conversion)
    void *p = ::realloc(array, mem_size(sizeof(T *), capacity));
    assert_noexcept(p != nullptr);
    return static_cast<T **>(p);
}

template <class T>
static noinline void free_array(T **array, size_t count) noexcept {
    for (size_t i = 0; i < count; i++) {
        T *item = upx::atomic_exchange(&array[i], (T *) nullptr);
        delete item;
    }
    ::free(array); // NOLINT(bugprone-multi-level-implicit-pointer-conversion)
}

/*************************************************************************
// Section
**************************************************************************/

ElfLinker::Section::Section(const char *n, const void *i, unsigned s, unsigned a)
    : name(nullptr), output(nullptr), size(s), offset(0), p2align(a), next(nullptr) {
    name = ::strdup(n);
    assert_noexcept(name != nullptr);
    input = ::malloc(s + 1);
    assert_noexcept(input != nullptr);
    if (s != 0) {
        assert_noexcept(i != nullptr);
        memcpy(input, i, s);
    }
    ((char *) input)[s] = 0;
}

ElfLinker::Section::~Section() noexcept {
    ::free(name);
    ::free(input);
}

/*************************************************************************
// Symbol
**************************************************************************/

ElfLinker::Symbol::Symbol(const char *n, Section *s, upx_uint64_t o)
    : name(nullptr), section(s), offset(o) {
    name = ::strdup(n);
    assert_noexcept(name != nullptr);
    assert_noexcept(section != nullptr);
}

ElfLinker::Symbol::~Symbol() noexcept { ::free(name); }

/*************************************************************************
// Relocation
**************************************************************************/

ElfLinker::Relocation::Relocation(const Section *s, unsigned o, const char *t, const Symbol *v,
                                  upx_uint64_t a)
    : section(s), offset(o), type(t), value(v), add(a) {
    assert_noexcept(section != nullptr);
}

/*************************************************************************
// ElfLinker
**************************************************************************/

ElfLinker::ElfLinker(const N_BELE_RTP::AbstractPolicy *b) noexcept : bele(b) {}

ElfLinker::~ElfLinker() noexcept {
    delete[] input;
    delete[] output;
    free_array(sections, nsections);
    free_array(symbols, nsymbols);
    free_array(relocations, nrelocations);
}

void ElfLinker::init(unsigned arch, const void *pdata_v, int plen, unsigned pxtra) {
    const byte *pdata = (const byte *) pdata_v;
    this->e_machine = arch;
    if (plen >= 16 && memcmp(pdata, "UPX#", 4) == 0) {
        // decompress pre-compressed stub-loader
        int method;
        unsigned u_len, c_len;
        if (pdata[4]) {
            method = pdata[4];
            u_len = get_le16(pdata + 5);
            c_len = get_le16(pdata + 7);
            pdata += 9;
            assert(9 + c_len == (unsigned) plen);
        } else {
            method = pdata[5];
            u_len = get_le32(pdata + 6);
            c_len = get_le32(pdata + 10);
            pdata += 14;
            assert(14 + c_len == (unsigned) plen);
        }
        assert((unsigned) plen < u_len);
        inputlen = u_len;
        input = New(byte, inputlen + 1);
        unsigned new_len = u_len;
        int r = upx_decompress(pdata, c_len, input, &new_len, method, nullptr);
        if (r == UPX_E_OUT_OF_MEMORY)
            throwOutOfMemoryException();
        if (r != UPX_E_OK || new_len != u_len)
            throwBadLoader();
    } else {
        inputlen = plen;
        input = New(byte, inputlen + 1);
        if (inputlen)
            memcpy(input, pdata, inputlen);
    }
    input[inputlen] = 0; // NUL terminate

    output_capacity = (inputlen ? (inputlen + pxtra) : 0x4000);
    assert(output_capacity < (1 << 16)); // LE16 l_info.l_size
    output = New(byte, output_capacity);
    outputlen = 0;
    NO_printf("\nElfLinker::init %d @%p\n", output_capacity, output);

    // FIXME: bad compare when either symbols or relocs are absent
    if ((int) strlen("Sections:\n"
                     "SYMBOL TABLE:\n"
                     "RELOCATION RECORDS FOR ") < inputlen) {
        const char *const eof = (const char *) &input[inputlen];
        int pos = find(input, inputlen, "Sections:\n", 10);
        assert(pos != -1);
        char *const psections = (char *) input + pos;

        char *const psymbols = strstr(psections, "SYMBOL TABLE:\n");
        // assert(psymbols != nullptr);

        char *const prelocs = strstr((psymbols ? psymbols : psections), "RELOCATION RECORDS FOR ");
        // assert(prelocs != nullptr);

        preprocessSections(psections, (psymbols ? psymbols : (prelocs ? prelocs : eof)));
        if (psymbols)
            preprocessSymbols(psymbols, (prelocs ? prelocs : eof));
        if (prelocs)
            preprocessRelocations(prelocs, eof);
        addLoader("*UND*");
    }
}

void ElfLinker::preprocessSections(char *start, const char *end) {
    assert_noexcept(nsections == 0);
    char *nextl;
    for (; start < end; start = 1 + nextl) {
        nextl = strchr(start, '\n');
        assert(nextl != nullptr);
        *nextl = '\0'; // a record is a line

        unsigned offset, size, align;
        char name[1024];

        if (sscanf(start, "%*d %1023s %x %*x %*x %x 2**%d", name, &size, &offset, &align) == 4) {
            char *n = strstr(start, name);
            n[strlen(name)] = 0;
            addSection(n, input + offset, size, align);
            NO_printf("section %s preprocessed\n", n);
        }
    }
    addSection("*ABS*", nullptr, 0, 0);
    addSection("*UND*", nullptr, 0, 0);
}

void ElfLinker::preprocessSymbols(char *start, const char *end) {
    assert_noexcept(nsymbols <= 1);
    char *nextl;
    for (; start < end; start = 1 + nextl) {
        nextl = strchr(start, '\n');
        assert(nextl != nullptr);
        *nextl = '\0'; // a record is a line

        unsigned value, offset;
        char section[1024];
        char symbol[1024];

        if (sscanf(start, "%x g *ABS* %x %1023s", &value, &offset, symbol) == 3) {
            char *s = strstr(start, symbol);
            s[strlen(symbol)] = 0;
            addSymbol(s, "*ABS*", value);
            assert(offset == 0);
        }
#if 0
        else if (sscanf(start, "%x%*8c %1023s %*x %1023s", &offset, section, symbol) == 3)
#else
        // work around broken scanf() implementations
        // https://bugs.winehq.org/show_bug.cgi?id=10401 (fixed in Wine 0.9.58)
        else if (sscanf(start, "%x%*c%*c%*c%*c%*c%*c%*c%*c %1023s %*x %1023s", &offset, section,
                        symbol) == 3)
#endif
        {
            char *s = strstr(start, symbol);
            s[strlen(symbol)] = 0;
            if (strcmp(section, "*UND*") == 0)
                offset = 0xdeaddead;
            assert(strcmp(section, "*ABS*") != 0);
            addSymbol(s, section, offset);
        }
    }
}

void ElfLinker::preprocessRelocations(char *start, const char *end) {
    assert_noexcept(nrelocations == 0);
    Section *section = nullptr;
    char *nextl;
    for (; start < end; start = 1 + nextl) {
        nextl = strchr(start, '\n');
        assert(nextl != nullptr);
        *nextl = '\0'; // a record is a line

        {
            char sect[1024];
            if (sscanf(start, "RELOCATION RECORDS FOR [%[^]]", sect) == 1)
                section = findSection(sect);
        }

        unsigned offset;
        char type[100];
        char symbol[1024];

        if (sscanf(start, "%x %99s %1023s", &offset, type, symbol) == 3) {
            char *t = strstr(start, type);
            t[strlen(type)] = 0;

            upx_uint64_t add = 0;
            char *p = strstr(symbol, "+0x");
            if (p == nullptr)
                p = strstr(symbol, "-0x");
            if (p) {
                char sign = *p;
                *p = 0; // terminate the symbol name
                p += 3;
                assert(strlen(p) == 8 || strlen(p) == 16);
#if (ACC_CC_MSC && (_MSC_VER < 1800))
                unsigned a = 0, b = 0;
                if (sscanf(p, "%08x%08x", &a, &b) == 2)
                    add = ((upx_uint64_t) a << 32) | b;
                else
                    add = a;
#else
                char *endptr = nullptr;
                add = strtoull(p, &endptr, 16);
                assert(endptr && *endptr == '\0');
#endif
                if (sign == '-')
                    add = 0 - add;
            }

            if (section) {
                addRelocation(section->name, offset, t, symbol, add);
                NO_printf("relocation %s %s %x %llu preprocessed\n", section->name, symbol, offset,
                          (unsigned long long) add);
            }
        }
    }
}

ElfLinker::Section *ElfLinker::findSection(const char *name, bool fatal) const {
    for (unsigned ic = 0; ic < nsections; ic++)
        if (strcmp(sections[ic]->name, name) == 0)
            return sections[ic];
    if (fatal)
        throwInternalError("unknown section %s\n", name);
    return nullptr;
}

ElfLinker::Symbol *ElfLinker::findSymbol(const char *name, bool fatal) const {
    for (unsigned ic = 0; ic < nsymbols; ic++)
        if (strcmp(symbols[ic]->name, name) == 0)
            return symbols[ic];
    if (fatal)
        throwInternalError("unknown symbol %s\n", name);
    return nullptr;
}

ElfLinker::Section *ElfLinker::addSection(const char *sname, const void *sdata, int slen,
                                          unsigned p2align) {
    NO_printf("addSection: %s len=%d align=%d\n", sname, slen, p2align);
    if (!sdata && (!strcmp("ABS*", sname) || !strcmp("UND*", sname)))
        return nullptr;
    assert(sname && sname[0]);
    assert(sname[strlen(sname) - 1] != ':');
    assert(findSection(sname, false) == nullptr);
    if (grow_capacity(nsections, &nsections_capacity))
        sections = realloc_array(sections, nsections_capacity);
    Section *sec = new Section(sname, sdata, slen, p2align);
    sec->sort_id = nsections;
    sections[nsections++] = sec;
    if ('*' == sname[0]) {
        if (!strcmp("*ABS*", sname)) {
            addSymbol("*ABS*", "*ABS*", 0);
        }
    }
    return sec;
}

ElfLinker::Symbol *ElfLinker::addSymbol(const char *name, const char *section,
                                        upx_uint64_t offset) {
    NO_printf("addSymbol: %s %s 0x%llx\n", name, section, offset);
    assert(name && name[0]);
    assert(name[strlen(name) - 1] != ':');
    assert(findSymbol(name, false) == nullptr);
    if (grow_capacity(nsymbols, &nsymbols_capacity))
        symbols = realloc_array(symbols, nsymbols_capacity);
    Symbol *sym = new Symbol(name, findSection(section), offset);
    symbols[nsymbols++] = sym;
    return sym;
}

ElfLinker::Relocation *ElfLinker::addRelocation(const char *section, unsigned off, const char *type,
                                                const char *symbol, upx_uint64_t add) {
    if (EM_RISCV == this->e_machine) {
        if ((symbol && '.' == symbol[0])        // compiler-generated; or 0f, 0b (etc.)
            && (!strcmp(type, "R_RISCV_BRANCH") // known types
                || !strcmp(type, "R_RISCV_RVC_BRANCH") || !strcmp(type, "R_RISCV_JAL") ||
                !strcmp(type, "R_RISCV_JUMP") || !strcmp(type, "R_RISCV_RVC_JUMP"))) {
            return nullptr; // ASSUME already relocated: source and target in same section
        }
    }
    if (grow_capacity(nrelocations, &nrelocations_capacity))
        relocations = realloc_array(relocations, nrelocations_capacity);
    NO_printf("addRelocation section=(%s +%#x)  symbol=(%s +%#x)\n", section, off, symbol,
              (unsigned) add);
    Relocation *rel = new Relocation(findSection(section), off, type, findSymbol(symbol), add);
    relocations[nrelocations++] = rel;
    return rel;
}

#if 0
void ElfLinker::setLoaderAlignOffset(int phase) {
    // assert(phase & 0);
    NO_printf("\nFIXME: ElfLinker::setLoaderAlignOffset %d\n", phase);
}
#endif

int ElfLinker::addLoader(const char *sname) {
    assert(sname != nullptr);
    if (!sname[0])
        return outputlen;

    char *begin = ::strdup(sname);
    assert_noexcept(begin != nullptr);
    const auto begin_deleter = upx::MallocDeleter(&begin, 1); // don't leak memory
    char *end = begin + strlen(begin);
    for (char *sect = begin; sect < end;) {
        for (char *tokend = sect; *tokend; tokend++)
            if (*tokend == ' ' || *tokend == ',') {
                *tokend = 0;
                break;
            }

        if (sect[0] == '+') { // alignment
            assert(tail);
            unsigned l = hex(sect[2]) - tail->offset - tail->size;
            unsigned m = hex(sect[1]);
            if (m) {
                l %= hex(sect[1]);
            }
            if (l) {
                if (sect[3] == 'D')
                    alignData(l);
                else
                    alignCode(l);
                tail->size += l;
            }
        } else {
            Section *section = findSection(sect);
            if (section->p2align) {
                assert(tail);
                assert(tail != section);
                // .p2align must be < 32
                const unsigned v = ~0u << section->p2align;
                if (const unsigned l = ~v & (0u - (unsigned) (tail->offset + tail->size))) {
                    alignCode(l);
                    tail->size += l;
                }
            }
            // Section->output is not relocatable, and C++ has no realloc() anyway.
            assert((section->size + outputlen) <= output_capacity);
            memcpy(output + outputlen, section->input, section->size);
            section->output = output + outputlen; // FIXME: INVALIDATED by realloc()
            NO_printf("section added: 0x%04x %3d %s\n", outputlen, section->size, section->name);
            outputlen += section->size;

            if (head) {
                tail->next = section;
                section->offset = tail->offset + tail->size;
            } else
                head = section;
            tail = section;
        }
        sect += strlen(sect) + 1;
    }
    return outputlen;
}

void ElfLinker::addLoader(const char *s, va_list ap) {
    while (s != nullptr) {
        addLoader(s);
        s = va_arg(ap, const char *);
    }
}

void ElfLinker::addLoaderVA(const char *s, ...) {
    va_list ap;
    va_start(ap, s);
    addLoader(s, ap);
    va_end(ap);
}

int ElfLinker::getSection(const char *sname, int *slen) const {
    const Section *section = findSection(sname);
    if (slen)
        *slen = section->size;
    return (int) (section->output - output);
}

int ElfLinker::getSectionSize(const char *sname) const {
    const Section *section = findSection(sname);
    return section->size;
}

byte *ElfLinker::getLoader(int *llen) const {
    if (llen)
        *llen = outputlen;
    return output;
}

void ElfLinker::relocate() {
    assert(!reloc_done);
    reloc_done = true;
    for (unsigned ic = 0; ic < nrelocations; ic++) {
        const Relocation *rel = relocations[ic];
        upx_uint64_t value = 0;

        if (rel->section->output == nullptr)
            continue;
        if (strcmp(rel->value->section->name, "*ABS*") == 0) {
            value = rel->value->offset;
        } else if (strcmp(rel->value->section->name, "*UND*") == 0 &&
                   rel->value->offset == 0xdeaddead)
            throwInternalError("undefined symbol '%s' referenced\n", rel->value->name);
        else if (rel->value->section->output == nullptr)
            throwInternalError("cannot apply reloc '%s:%x' without section '%s'\n",
                               rel->section->name, rel->offset, rel->value->section->name);
        else {
            value = rel->value->section->offset + rel->value->offset + rel->add;
        }
        byte *location = rel->section->output + rel->offset;
        NO_printf("%-28s %-28s %-10s %#16llx %#16llx\n", rel->section->name, rel->value->name,
                  rel->type, (long long) value,
                  (long long) value - rel->section->offset - rel->offset);
        NO_printf("  %llx %d %llx %d %llx :%d\n", (long long) value,
                  (int) rel->value->section->offset, rel->value->offset, rel->offset,
                  (long long) rel->add, *location);
        relocate1(rel, location, value, rel->type);
    }
}

void ElfLinker::defineSymbol(const char *name, upx_uint64_t value) {
    Symbol *symbol = findSymbol(name);
    if (strcmp(symbol->section->name, "*ABS*") == 0)
        throwInternalError("defineSymbol: symbol '%s' is *ABS*\n", name);
    else if (strcmp(symbol->section->name, "*UND*") == 0) // for undefined symbols
        symbol->offset = value;
    else if (strcmp(symbol->section->name, name) == 0) // for sections
    {
        for (Section *section = symbol->section; section; section = section->next) {
            assert(section->offset < value);
            section->offset = value;
            value += section->size;
        }
    } else
        throwInternalError("defineSymbol: symbol '%s' already defined\n", name);
}

// debugging support
void ElfLinker::dumpSymbol(const Symbol *symbol, unsigned flags, FILE *fp) const {
    if ((flags & 1) && symbol->section->output == nullptr)
        return;
    char d0[16 + 1], d1[16 + 1];
    upx_safe_snprintf(d0, sizeof(d0), "%016llx", (upx_uint64_t) symbol->offset);
    upx_safe_snprintf(d1, sizeof(d1), "%016llx", (upx_uint64_t) symbol->section->offset);
    fprintf(fp, "%-28s 0x%-16s | %-28s 0x%-16s\n", symbol->name, d0, symbol->section->name, d1);
}
void ElfLinker::dumpSymbols(unsigned flags, FILE *fp) const {
    if (fp == nullptr)
        fp = stdout;
    if ((flags & 2) == 0) {
        // default: dump symbols in used section order
        for (const Section *section = head; section; section = section->next) {
            char d1[16 + 1];
            upx_safe_snprintf(d1, sizeof(d1), "%016llx", (upx_uint64_t) section->offset);
            fprintf(fp, "%-42s%-28s 0x%-16s\n", "", section->name, d1);
            for (unsigned ic = 0; ic < nsymbols; ic++) {
                const Symbol *symbol = symbols[ic];
                if (symbol->section == section && strcmp(symbol->name, section->name) != 0)
                    dumpSymbol(symbol, flags, fp);
            }
        }
    } else {
        // dump all symbols
        for (unsigned ic = 0; ic < nsymbols; ic++)
            dumpSymbol(symbols[ic], flags, fp);
    }
}

upx_uint64_t ElfLinker::getSymbolOffset(const char *name) const {
    const Symbol *symbol = findSymbol(name);
    if (symbol->section->output == nullptr)
        return 0xdeaddead;
    return symbol->section->offset + symbol->offset;
}

void ElfLinker::alignWithByte(unsigned len, byte b) {
    memset(output + outputlen, b, len);
    outputlen += len;
}

void ElfLinker::relocate1(const Relocation *rel, byte *, upx_uint64_t, const char *) {
    throwInternalError("unknown relocation type '%s\n'", rel->type);
}

/*************************************************************************
// ElfLinker arch subclasses
// FIXME: add more displacement overflow checks
// FIXME: add support for our special "ignore_reloc_overflow" section
**************************************************************************/

#if 0 // FIXME
static void check8(const Relocation *rel, const byte *location, int v, int d) {
    if (v < -128 || v > 127)
        throwInternalError("value out of range (%d) in reloc %s:%x\n", v, rel->section->name,
                           rel->offset);
    if (d < -128 || d > 127)
        throwInternalError("displacement target out of range (%d) in reloc %s:%x\n", v,
                           rel->section->name, rel->offset);
}
#endif

void ElfLinkerAMD64::relocate1(const Relocation *rel, byte *location, upx_uint64_t value,
                               const char *type) {
    if (strncmp(type, "R_X86_64_", 9))
        return super::relocate1(rel, location, value, type);
    type += 9;

    bool range_check = false;
    if (strncmp(type, "PC", 2) == 0) {
        type += 2;
        value -= rel->section->offset + rel->offset;
        range_check = true;
    } else if (strncmp(type, "PLT", 3) == 0) {
        type += 3;
        value -= rel->section->offset + rel->offset;
        range_check = true;
    }

    if (strcmp(type, "8") == 0) {
        int displ = (upx_int8_t) *location + (int) value;
        if (range_check && (displ < -128 || displ > 127))
            throwInternalError("target out of range (%d) in reloc %s:%x\n", displ,
                               rel->section->name, rel->offset);
        *location += value;
    } else if (strcmp(type, "16") == 0)
        set_le16(location, get_le16(location) + value);
    else if (strncmp(type, "32", 2) == 0) // for "32" and "32S"
        set_le32(location, get_le32(location) + value);
    else if (strcmp(type, "64") == 0)
        set_le64(location, get_le64(location) + value);
    else
        super::relocate1(rel, location, value, type);
}

void ElfLinkerArmBE::relocate1(const Relocation *rel, byte *location, upx_uint64_t value,
                               const char *type) {
    if (!strcmp(type, "R_ARM_PC24") || !strcmp(type, "R_ARM_CALL") ||
        !strcmp(type, "R_ARM_JUMP24")) {
        value -= rel->section->offset + rel->offset;
        set_be24(1 + location, get_be24(1 + location) + value / 4);
    } else if (strcmp(type, "R_ARM_ABS32") == 0) {
        set_be32(location, get_be32(location) + value);
    } else if (strcmp(type, "R_ARM_THM_CALL") == 0 || strcmp(type, "R_ARM_THM_XPC22") == 0 ||
               strcmp(type, "R_ARM_THM_PC22") == 0) {
        value -= rel->section->offset + rel->offset;
        value += ((get_be16(location) & 0x7ff) << 12);
        value += (get_be16(location + 2) & 0x7ff) << 1;

        set_be16(location, 0xf000 + ((value >> 12) & 0x7ff));
        set_be16(location + 2, 0xf800 + ((value >> 1) & 0x7ff));

        //(b, 0xF000 + ((v - 1) / 2) * 0x10000);
        // set_be32(location, get_be32(location) + value / 4);
    } else if (0 == strcmp("R_ARM_ABS8", type)) {
        *location += value;
    } else
        super::relocate1(rel, location, value, type);
}

void ElfLinkerArmLE::relocate1(const Relocation *rel, byte *location, upx_uint64_t value,
                               const char *type) {
    if (!strcmp(type, "R_ARM_PC24") || !strcmp(type, "R_ARM_CALL") ||
        !strcmp(type, "R_ARM_JUMP24")) {
        value -= rel->section->offset + rel->offset;
        set_le24(location, get_le24(location) + value / 4);
    } else if (strcmp(type, "R_ARM_ABS32") == 0) {
        set_le32(location, get_le32(location) + value);
    } else if (strcmp(type, "R_ARM_THM_CALL") == 0 || strcmp(type, "R_ARM_THM_XPC22") == 0 ||
               strcmp(type, "R_ARM_THM_PC22") == 0) {
        value -= rel->section->offset + rel->offset;
        value += ((get_le16(location) & 0x7ff) << 12);
        value += (get_le16(location + 2) & 0x7ff) << 1;

        set_le16(location, 0xf000 + ((value >> 12) & 0x7ff));
        set_le16(location + 2, 0xf800 + ((value >> 1) & 0x7ff));

        //(b, 0xF000 + ((v - 1) / 2) * 0x10000);
        // set_le32(location, get_le32(location) + value / 4);
    } else if (0 == strcmp("R_ARM_ABS8", type)) {
        *location += value;
    } else
        super::relocate1(rel, location, value, type);
}

void ElfLinkerArm64LE::relocate1(const Relocation *rel, byte *location, upx_uint64_t value,
                                 const char *type) {
    if (strncmp(type, "R_AARCH64_", 10))
        return super::relocate1(rel, location, value, type);
    type += 10;

    if (!strncmp(type, "PREL", 4)) {
        value -= rel->section->offset + rel->offset;
        type += 4;

        if (!strcmp(type, "16"))
            set_le16(location, get_le16(location) + value);
        else if (!strncmp(type, "32", 2)) // for "32" and "32S"
            set_le32(location, get_le32(location) + value);
        else if (!strcmp(type, "64"))
            set_le64(location, get_le64(location) + value);
    } else if (!strcmp(type, "ADR_PREL_LO21")) {
        value -= rel->section->offset + rel->offset;
        const upx_uint32_t m19 = ~(~0u << 19);
        upx_uint32_t w = get_le32(location);
        set_le32(location, (w & ~((3u << 29) | (m19 << 5))) | ((3u & value) << 29) |
                               ((m19 & (value >> 2)) << 5));
    } else if (!strcmp(type, "ABS32")) {
        set_le32(location, get_le32(location) + value);
    } else if (!strcmp(type, "ABS64")) {
        set_le64(location, get_le64(location) + value);
    } else if (!strcmp(type, "CONDBR19")) {
        value -= rel->section->offset + rel->offset;
        const upx_uint32_t m19 = ~(~0u << 19);
        upx_uint32_t w = get_le32(location);
        set_le32(location, (w & ~(m19 << 5)) | ((((w >> 5) + (value >> 2)) & m19) << 5));
    } else if (!strcmp(type, "CALL26") || !strcmp(type, "JUMP26")) {
        value -= rel->section->offset + rel->offset;
        const upx_uint32_t m26 = ~(~0u << 26);
        upx_uint32_t w = get_le32(location);
        set_le32(location, (w & ~m26) | (m26 & (value >> 2)));
    } else
        super::relocate1(rel, location, value, type);
}

// clang-format off
unsigned extr_imm_RV_JAL(unsigned w);
unsigned extr_imm_RV_JAL(unsigned w)
{
    return ((0x3ff & (w >> 21)) << 1)
        | ((     1 & (w >> 20)) << 11)
        | ((  0xff & (w >> 12)) << 12)
        | ((     1 & (w >> 31)) << 20);
}
unsigned ins_imm_RV_JAL(unsigned w);
unsigned ins_imm_RV_JAL(unsigned w)
{
    return 0
        | (( 0x3ff & (w >>  1)) << 21)
        | ((     1 & (w >> 11)) << 20)
        | ((  0xff & (w >> 12)) << 12)
        | ((     1 & (w >> 20)) << 31);
}

unsigned extr_imm_RV_Bxx(unsigned w);
unsigned extr_imm_RV_Bxx(unsigned w)
{
    return 0
        | ((0xf & (w >>  8)) << 1)
        | ((077 & (w >> 25)) << 5)
        | ((  1 & (w >>  7)) << 11)
        | ((  1 & (w >> 31)) << 12);
}
unsigned ins_imm_RV_Bxx(unsigned w);
unsigned ins_imm_RV_Bxx(unsigned w)
{
    return 0
        | ((0xf & (w >>  1)) <<  8)
        | ((077 & (w >>  5)) << 25)
        | ((  1 & (w >> 11)) <<  7)
        | ((  1 & (w >> 12)) << 31);
}

unsigned extr_imm_RVC_Beq(unsigned w);
unsigned extr_imm_RVC_Beq(unsigned w)
{
    return ((3 & (w >>  3)) << 1)
        | (( 3 & (w >> 10)) << 3)
        | (( 1 & (w >>  2)) << 5)
        | (( 3 & (w >>  5)) << 6)
        | (( 1 & (w >> 12)) << 8);
}
unsigned ins_imm_RVC_Beq(unsigned w);
unsigned ins_imm_RVC_Beq(unsigned w)
{
    return 0
        | ((1 & (w >> 5)) <<  2)
        | ((3 & (w >> 1)) <<  3)
        | ((3 & (w >> 6)) <<  5)
        | ((3 & (w >> 3)) << 10)
        | ((1 & (w >> 8)) << 12);
}

unsigned extr_imm_RVC_Jmp(unsigned w);
unsigned extr_imm_RVC_Jmp(unsigned w)
{
    return ((7 & (w >>  3)) << 1)
        | (( 1 & (w >> 11)) << 4)
        | (( 1 & (w >>  2)) << 5)
        | (( 1 & (w >>  7)) << 6)
        | (( 1 & (w >>  6)) << 7)
        | (( 3 & (w >>  9)) << 8)
        | (( 1 & (w >>  8)) << 10)
        | (( 1 & (w >> 12)) << 11);
}
unsigned ins_imm_RVC_Jmp(unsigned w);
unsigned ins_imm_RVC_Jmp(unsigned w)
{
    return 0
        | ((7 & (w >>  1)) <<  3)
        | ((1 & (w >>  4)) << 11)
        | ((1 & (w >>  5)) <<  2)
        | ((1 & (w >>  6)) <<  7)
        | ((1 & (w >>  7)) <<  6)
        | ((3 & (w >>  8)) <<  9)
        | ((1 & (w >> 10)) <<  8)
        | ((1 & (w >> 11)) << 12);
}

#if 0  //{  RV32 only!
unsigned extr_imm_RVC_JAL(unsigned w);
unsigned extr_imm_RVC_JAL(unsigned w)  // RV32 only!
{
    return ((0x7 & (w >>  3)) <<  1)
        | ((   1 & (w >> 11)) <<  4)
        | ((   1 & (w >>  2)) <<  5)
        | ((   1 & (w >>  7)) <<  6)
        | ((   1 & (w >>  6)) <<  7)
        | ((   3 & (w >>  9)) <<  8)
        | ((   1 & (w >>  8)) << 10)
        | ((   1 & (w >> 11)) << 11) ;
}
unsigned ins_imm_RVC_JAL(unsigned w);
unsigned ins_imm_RVC_JAL(unsigned w)  // RV32 only!
{
    return ((0x7 & (w >>  1)) <<  3)
        | ((   1 & (w >>  4)) << 11)
        | ((   1 & (w >>  5)) <<  2)
        | ((   1 & (w >>  6)) <<  7)
        | ((   1 & (w >>  7)) <<  6)
        | ((   3 & (w >>  8)) <<  9)
        | ((   1 & (w >> 10)) <<  0)
        | ((   1 & (w >> 11)) << 11) ;
}
#endif  //}
// clang-format on

void ElfLinkerRiscv64LE::relocate1(const Relocation *rel, byte *location, upx_uint64_t value,
                                   const char *type) {
    if (strncmp(type, "R_RISCV_", 8)) // not us
        return super::relocate1(rel, location, value, type);
    type += 8;

    if (!strncmp(type, "BRANCH", 6)) {
        value -= rel->section->offset + rel->offset;
        unsigned instr = get_le32(location);
        set_le32(location,
                 (((037 << 20) | (037 << 15) | (7 << 12) | 0x7f) & instr) | ins_imm_RV_Bxx(value));
    } else if (!strncmp(type, "JAL", 3)) {
        value -= rel->section->offset + rel->offset;
        unsigned instr = get_le32(location);
        set_le32(location, (0xfff & instr) | ins_imm_RV_JAL(value));
    } else if (!strncmp(type, "RVC_BRANCH", 10)) {
        value -= rel->section->offset + rel->offset;
        unsigned instr = get_le16(location);
        set_le16(location, (((7 << 13) | (7 << 7) | 3) & instr) | ins_imm_RVC_Beq(value));
    } else if (!strncmp(type, "RVC_JUMP", 8)) {
        value -= rel->section->offset + rel->offset;
        unsigned instr = get_le16(location);
        set_le16(location, (((7 << 13) | 3) & instr) | ins_imm_RVC_Jmp(value));
    } else if (!strncmp(type, "32", 2)) {
        set_le32(location, value);
    } else
        super::relocate1(rel, location, value, type);
}

void ElfLinkerM68k::alignCode(unsigned len) {
    assert((len & 1) == 0);
    assert((outputlen & 1) == 0);
    for (unsigned i = 0; i < len; i += 2)
        set_be16(output + outputlen + i, 0x4e71); // "nop"
    outputlen += len;
}

void ElfLinkerM68k::relocate1(const Relocation *rel, byte *location, upx_uint64_t value,
                              const char *type) {
    if (strncmp(type, "R_68K_", 6))
        return super::relocate1(rel, location, value, type);
    type += 6;

    if (strncmp(type, "PC", 2) == 0) {
        value -= rel->section->offset + rel->offset;
        type += 2;
    }
    if (strcmp(type, "8") == 0) {
        *location += value;
    } else if (strcmp(type, "16") == 0) {
        set_be16(location, get_be16(location) + value);
    } else if (strcmp(type, "32") == 0) {
        set_be32(location, get_be32(location) + value);
    } else
        super::relocate1(rel, location, value, type);
}

void ElfLinkerMipsBE::relocate1(const Relocation *rel, byte *location, upx_uint64_t value,
                                const char *type) {
#define MIPS_HI(a)   (((a) >> 16) + (((a) &0x8000) >> 15))
#define MIPS_LO(a)   ((a) &0xffff)
#define MIPS_PC16(a) ((a) >> 2)
#define MIPS_PC26(a) (((a) &0x0fffffff) >> 2)

    if (strcmp(type, "R_MIPS_HI16") == 0)
        set_be16(2 + location, get_be16(2 + location) + MIPS_HI(value));
    else if (strcmp(type, "R_MIPS_LO16") == 0)
        set_be16(2 + location, get_be16(2 + location) + MIPS_LO(value));
    else if (strcmp(type, "R_MIPS_PC16") == 0) {
        value -= rel->section->offset + rel->offset;
        set_be16(2 + location, get_be16(2 + location) + MIPS_PC16(value));
    } else if (strcmp(type, "R_MIPS_26") == 0)
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

void ElfLinkerMipsLE::relocate1(const Relocation *rel, byte *location, upx_uint64_t value,
                                const char *type) {
#define MIPS_HI(a)   (((a) >> 16) + (((a) &0x8000) >> 15))
#define MIPS_LO(a)   ((a) &0xffff)
#define MIPS_PC16(a) ((a) >> 2)
#define MIPS_PC26(a) (((a) &0x0fffffff) >> 2)

    if (strcmp(type, "R_MIPS_HI16") == 0)
        set_le16(location, get_le16(location) + MIPS_HI(value));
    else if (strcmp(type, "R_MIPS_LO16") == 0)
        set_le16(location, get_le16(location) + MIPS_LO(value));
    else if (strcmp(type, "R_MIPS_PC16") == 0) {
        value -= rel->section->offset + rel->offset;
        set_le16(location, get_le16(location) + MIPS_PC16(value));
    } else if (strcmp(type, "R_MIPS_26") == 0)
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

void ElfLinkerPpc32::relocate1(const Relocation *rel, byte *location, upx_uint64_t value,
                               const char *type) {
    if (strncmp(type, "R_PPC_", 6))
        return super::relocate1(rel, location, value, type);
    type += 6;

    if (strcmp(type, "ADDR32") == 0) {
        set_be32(location, get_be32(location) + value);
        return;
    }

    if (strncmp(type, "REL", 3) == 0) {
        value -= rel->section->offset + rel->offset;
        type += 3;
    }

    // FIXME: more relocs

    // Note that original (*location).displ is ignored.
    if (strcmp(type, "24") == 0) {
        if (3 & value)
            throwInternalError("unaligned word displacement");
        // FIXME: displacement overflow?
        set_be32(location, (0xfc000003 & get_be32(location)) + (0x03fffffc & value));
    } else if (strcmp(type, "14") == 0) {
        if (3 & value)
            throwInternalError("unaligned word displacement");
        // FIXME: displacement overflow?
        set_be32(location, (0xffff0003 & get_be32(location)) + (0x0000fffc & value));
    } else
        super::relocate1(rel, location, value, type);
}

void ElfLinkerPpc64le::relocate1(const Relocation *rel, byte *location, upx_uint64_t value,
                                 const char *type) {
    if (!strcmp(type, "R_PPC64_ADDR64")) {
        set_le64(location, get_le64(location) + value);
        return;
    }
    if (!strcmp(type, "R_PPC64_ADDR32")) {
        set_le32(location, get_le32(location) + value);
        return;
    }
    if (strncmp(type, "R_PPC64_REL", 11))
        return super::relocate1(rel, location, value, type);
    type += 11;

    bool range_check = false;
    if (strncmp(type, "PC", 2) == 0) {
        type += 2;
        range_check = true;
    }

    /* value will hold relative displacement */
    value -= rel->section->offset + rel->offset;

    if (strncmp(type, "14", 2) == 0) { // for "14" and "14S"
        if (3 & value)
            throwInternalError("unaligned word displacement");
        // FIXME: displacement overflow?
        set_le32(location, (0xffff0003 & get_le32(location)) + (0x0000fffc & value));
    } else if (strncmp(type, "24", 2) == 0) { // for "24" and "24S"
        if (3 & value)
            throwInternalError("unaligned word displacement");
        // FIXME: displacement overflow?
        set_le32(location, (0xfc000003 & get_le32(location)) + (0x03fffffc & value));
    } else if (strcmp(type, "8") == 0) {
        int displ = (upx_int8_t) *location + (int) value;
        if (range_check && (displ < -128 || displ > 127))
            throwInternalError("target out of range (%d) in reloc %s:%x\n", displ,
                               rel->section->name, rel->offset);
        *location += value;
    } else if (strcmp(type, "16") == 0)
        set_le16(location, get_le16(location) + value);
    else if (strncmp(type, "32", 2) == 0) // for "32" and "32S"
        set_le32(location, get_le32(location) + value);
    else if (strcmp(type, "64") == 0)
        set_le64(location, get_le64(location) + value);
    else
        super::relocate1(rel, location, value, type);
}

void ElfLinkerPpc64::relocate1(const Relocation *rel, byte *location, upx_uint64_t value,
                               const char *type) {
    if (!strcmp(type, "R_PPC64_ADDR32")) {
        set_be32(location, get_be32(location) + value);
        return;
    }
    if (!strcmp(type, "R_PPC64_ADDR64")) {
        set_be64(location, get_be64(location) + value);
        return;
    }
    if (strncmp(type, "R_PPC64_REL", 11))
        return super::relocate1(rel, location, value, type);
    type += 11;

    bool range_check = false;
    if (strncmp(type, "PC", 2) == 0) {
        type += 2;
        range_check = true;
    }

    // We have "R_PPC64_REL" or "R_PPC64_RELPC" as a prefix.
    /* value will hold relative displacement */
    value -= rel->section->offset + rel->offset;

    if (strncmp(type, "14", 2) == 0) { // for "14" and "14S"
        if (3 & value)
            throwInternalError("unaligned word displacement");
        // FIXME: displacement overflow?
        set_be32(location, (0xffff0003 & get_be32(location)) + (0x0000fffc & value));
    } else if (strncmp(type, "24", 2) == 0) { // for "24" and "24S"
        if (3 & value)
            throwInternalError("unaligned word displacement");
        // FIXME: displacement overflow?
        set_be32(location, (0xfc000003 & get_be32(location)) + (0x03fffffc & value));
    } else if (strcmp(type, "8") == 0) {
        int displ = (upx_int8_t) *location + (int) value;
        if (range_check && (displ < -128 || displ > 127))
            throwInternalError("target out of range (%d) in reloc %s:%x\n", displ,
                               rel->section->name, rel->offset);
        *location += value;
    } else if (strcmp(type, "16") == 0)
        set_be16(location, get_be16(location) + value);
    else if (strncmp(type, "32", 2) == 0) // for "32" and "32S"
        set_be32(location, get_be32(location) + value);
    else if (strcmp(type, "64") == 0)
        set_be64(location, get_be64(location) + value);
    else
        super::relocate1(rel, location, value, type);
}

void ElfLinkerX86::relocate1(const Relocation *rel, byte *location, upx_uint64_t value,
                             const char *type) {
    if (strncmp(type, "R_386_", 6))
        return super::relocate1(rel, location, value, type);
    type += 6;

    bool range_check = false;
    if (strncmp(type, "PC", 2) == 0) {
        value -= rel->section->offset + rel->offset;
        type += 2;
        range_check = true;
    }

    if (strcmp(type, "8") == 0) {
        int displ = (upx_int8_t) *location + (int) value;
        if (range_check && (displ < -128 || displ > 127))
            throwInternalError("target out of range (%d,%d,%llu) in reloc %s:%x\n", displ,
                               *location, value, rel->section->name, rel->offset);
        *location += value;
    } else if (strcmp(type, "16") == 0)
        set_le16(location, get_le16(location) + value);
    else if (strcmp(type, "32") == 0)
        set_le32(location, get_le32(location) + value);
    else
        super::relocate1(rel, location, value, type);
}

/* vim:set ts=4 sw=4 et: */
