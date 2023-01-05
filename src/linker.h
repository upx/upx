/* linker.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2023 Laszlo Molnar
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

#ifndef __UPX_LINKER_H
#define __UPX_LINKER_H 1

/*************************************************************************
// ElfLinker
**************************************************************************/

class ElfLinker : private noncopyable {
    friend class Packer;

public:
    const N_BELE_RTP::AbstractPolicy *bele = nullptr; // target endianness
protected:
    struct Section;
    struct Symbol;
    struct Relocation;

    upx_byte *input = nullptr;
    int inputlen = 0;
    upx_byte *output = nullptr;
    int outputlen = 0;
    unsigned output_capacity = 0;

    Section *head = nullptr;
    Section *tail = nullptr;

    Section **sections = nullptr;
    Symbol **symbols = nullptr;
    Relocation **relocations = nullptr;

    unsigned nsections = 0;
    unsigned nsections_capacity = 0;
    unsigned nsymbols = 0;
    unsigned nsymbols_capacity = 0;
    unsigned nrelocations = 0;
    unsigned nrelocations_capacity = 0;

    bool reloc_done = false;

protected:
    void preprocessSections(char *start, char const *end);
    void preprocessSymbols(char *start, char const *end);
    void preprocessRelocations(char *start, char const *end);
    Section *findSection(const char *name, bool fatal = true) const;
    Symbol *findSymbol(const char *name, bool fatal = true) const;

    Symbol *addSymbol(const char *name, const char *section, upx_uint64_t offset);
    Relocation *addRelocation(const char *section, unsigned off, const char *type,
                              const char *symbol, upx_uint64_t add);

public:
    ElfLinker();
    virtual ~ElfLinker();

    void init(const void *pdata, int plen, unsigned pxtra = 0);
    // virtual void setLoaderAlignOffset(int phase);
    int addLoader(const char *sname);
    void addLoader(const char *s, va_list ap);
#if (ACC_CC_CLANG || ACC_CC_GNUC)
    void addLoaderVA(const char *s, ...) __attribute__((__sentinel__));
#else
    void addLoaderVA(const char *s, ...);
#endif
    Section *addSection(const char *sname, const void *sdata, int slen, unsigned p2align);
    int getSection(const char *sname, int *slen = nullptr) const;
    int getSectionSize(const char *sname) const;
    upx_byte *getLoader(int *llen = nullptr) const;
    void defineSymbol(const char *name, upx_uint64_t value);
    upx_uint64_t getSymbolOffset(const char *) const;

    void dumpSymbol(const Symbol *, unsigned flags, FILE *fp) const;
    void dumpSymbols(unsigned flags = 0, FILE *fp = nullptr) const;

    void alignWithByte(unsigned len, unsigned char b);
    virtual void alignCode(unsigned len) { alignWithByte(len, 0); }
    virtual void alignData(unsigned len) { alignWithByte(len, 0); }

    // provide overloads to pacify GitHub CodeQL
    void defineSymbol(const char *name, int value) { defineSymbol(name, upx_uint64_t(value)); }
    void defineSymbol(const char *name, unsigned value) { defineSymbol(name, upx_uint64_t(value)); }
    void defineSymbol(const char *name, unsigned long value) {
        defineSymbol(name, upx_uint64_t(value));
    }

protected:
    void relocate();
    virtual void relocate1(const Relocation *, upx_byte *location, upx_uint64_t value,
                           const char *type);

    // target endianness abstraction
    unsigned get_te16(const void *p) const { return bele->get16(p); }
    unsigned get_te32(const void *p) const { return bele->get32(p); }
    upx_uint64_t get_te64(const void *p) const { return bele->get64(p); }
    void set_te16(void *p, unsigned v) const { bele->set16(p, v); }
    void set_te32(void *p, unsigned v) const { bele->set32(p, v); }
    void set_te64(void *p, upx_uint64_t v) const { bele->set64(p, v); }
};

struct ElfLinker::Section : private noncopyable {
    char *name = nullptr;
    void *input = nullptr;
    upx_byte *output = nullptr;
    unsigned size = 0;
    upx_uint64_t offset = 0;
    unsigned p2align = 0; // log2
    Section *next = nullptr;

    Section(const char *n, const void *i, unsigned s, unsigned a = 0);
    ~Section();
};

struct ElfLinker::Symbol : private noncopyable {
    char *name = nullptr;
    Section *section = nullptr;
    upx_uint64_t offset = 0;

    Symbol(const char *n, Section *s, upx_uint64_t o);
    ~Symbol();
};

struct ElfLinker::Relocation : private noncopyable {
    const Section *section = nullptr;
    unsigned offset = 0;
    const char *type = nullptr;
    const Symbol *value = nullptr;
    upx_uint64_t add; // used in .rela relocations

    Relocation(const Section *s, unsigned o, const char *t, const Symbol *v, upx_uint64_t a);
};

/*************************************************************************
// ElfLinker arch subclasses
**************************************************************************/

class ElfLinkerAMD64 : public ElfLinker {
    typedef ElfLinker super;

protected:
    virtual void alignCode(unsigned len) override { alignWithByte(len, 0x90); }
    virtual void relocate1(const Relocation *, upx_byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerARM64 final : public ElfLinker {
    typedef ElfLinker super;

protected:
    virtual void alignCode(unsigned len) override { alignWithByte(len, 0x90); }
    virtual void relocate1(const Relocation *, upx_byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerArmBE final : public ElfLinker {
    typedef ElfLinker super;

public:
    ElfLinkerArmBE() { bele = &N_BELE_RTP::be_policy; }

protected:
    virtual void relocate1(const Relocation *, upx_byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerArmLE final : public ElfLinker {
    typedef ElfLinker super;

protected:
    virtual void relocate1(const Relocation *, upx_byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerArm64LE final : public ElfLinker {
    typedef ElfLinker super;

protected:
    virtual void relocate1(const Relocation *, upx_byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerM68k final : public ElfLinker {
    typedef ElfLinker super;

public:
    ElfLinkerM68k() { bele = &N_BELE_RTP::be_policy; }

protected:
    virtual void alignCode(unsigned len) override;
    virtual void relocate1(const Relocation *, upx_byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerMipsBE final : public ElfLinker {
    typedef ElfLinker super;

public:
    ElfLinkerMipsBE() { bele = &N_BELE_RTP::be_policy; }

protected:
    virtual void relocate1(const Relocation *, upx_byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerMipsLE final : public ElfLinker {
    typedef ElfLinker super;

protected:
    virtual void relocate1(const Relocation *, upx_byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerPpc32 final : public ElfLinker {
    typedef ElfLinker super;

public:
    ElfLinkerPpc32() { bele = &N_BELE_RTP::be_policy; }

protected:
    virtual void relocate1(const Relocation *, upx_byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerPpc64le final : public ElfLinker {
    typedef ElfLinker super;

protected:
    virtual void relocate1(const Relocation *, upx_byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerPpc64 final : public ElfLinker {
    typedef ElfLinker super;

public:
    ElfLinkerPpc64() { bele = &N_BELE_RTP::be_policy; }

protected:
    virtual void relocate1(const Relocation *, upx_byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerX86 final : public ElfLinker {
    typedef ElfLinker super;

protected:
    virtual void alignCode(unsigned len) override { alignWithByte(len, 0x90); }
    virtual void relocate1(const Relocation *, upx_byte *location, upx_uint64_t value,
                           const char *type) override;
};

#endif /* already included */

/* vim:set ts=4 sw=4 et: */
