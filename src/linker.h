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

#pragma once

/*************************************************************************
// ElfLinker
**************************************************************************/

class ElfLinker : private noncopyable {
    friend class Packer;

public:
    const N_BELE_RTP::AbstractPolicy *const bele; // target endianness
protected:
    struct Section;
    struct Symbol;
    struct Relocation;

    byte *input = nullptr;
    int inputlen = 0;
    byte *output = nullptr;
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
    explicit ElfLinker(const N_BELE_RTP::AbstractPolicy *b = &N_BELE_RTP::le_policy) noexcept;
    virtual ~ElfLinker() noexcept;

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
    byte *getLoader(int *llen = nullptr) const;
    void defineSymbol(const char *name, upx_uint64_t value);
    upx_uint64_t getSymbolOffset(const char *) const;

    void dumpSymbol(const Symbol *, unsigned flags, FILE *fp) const;
    void dumpSymbols(unsigned flags = 0, FILE *fp = nullptr) const;

    void alignWithByte(unsigned len, byte b);
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
    virtual void relocate1(const Relocation *, byte *location, upx_uint64_t value,
                           const char *type);
};

struct ElfLinker::Section : private noncopyable {
    char *name = nullptr;
    void *input = nullptr;
    byte *output = nullptr;
    unsigned size = 0;
    unsigned sort_id = 0; // for qsort()
    upx_uint64_t offset = 0;
    unsigned p2align = 0; // log2
    Section *next = nullptr;

    explicit Section(const char *n, const void *i, unsigned s, unsigned a = 0);
    ~Section() noexcept;
};

struct ElfLinker::Symbol : private noncopyable {
    char *name = nullptr;
    Section *section = nullptr;
    upx_uint64_t offset = 0;

    explicit Symbol(const char *n, Section *s, upx_uint64_t o);
    ~Symbol() noexcept;
};

struct ElfLinker::Relocation : private noncopyable {
    const Section *section = nullptr;
    unsigned offset = 0;
    const char *type = nullptr;
    const Symbol *value = nullptr;
    upx_uint64_t add; // used in .rela relocations

    explicit Relocation(const Section *s, unsigned o, const char *t, const Symbol *v,
                        upx_uint64_t a);
    ~Relocation() noexcept {}
};

/*************************************************************************
// ElfLinker arch subclasses
**************************************************************************/

class ElfLinkerAMD64 /*not_final*/ : public ElfLinker {
    typedef ElfLinker super;
protected:
    virtual void alignCode(unsigned len) override { alignWithByte(len, 0x90); }
    virtual void relocate1(const Relocation *, byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerArm64LE final : public ElfLinker {
    typedef ElfLinker super;
protected:
    virtual void relocate1(const Relocation *, byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerArmBE final : public ElfLinker {
    typedef ElfLinker super;
public:
    explicit ElfLinkerArmBE() noexcept : super(&N_BELE_RTP::be_policy) {}
protected:
    virtual void relocate1(const Relocation *, byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerArmLE final : public ElfLinker {
    typedef ElfLinker super;
protected:
    virtual void relocate1(const Relocation *, byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerM68k final : public ElfLinker {
    typedef ElfLinker super;
public:
    explicit ElfLinkerM68k() noexcept : super(&N_BELE_RTP::be_policy) {}
protected:
    virtual void alignCode(unsigned len) override;
    virtual void relocate1(const Relocation *, byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerMipsBE final : public ElfLinker {
    typedef ElfLinker super;
public:
    explicit ElfLinkerMipsBE() noexcept : super(&N_BELE_RTP::be_policy) {}
protected:
    virtual void relocate1(const Relocation *, byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerMipsLE final : public ElfLinker {
    typedef ElfLinker super;
protected:
    virtual void relocate1(const Relocation *, byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerPpc32 final : public ElfLinker {
    typedef ElfLinker super;
public:
    explicit ElfLinkerPpc32() noexcept : super(&N_BELE_RTP::be_policy) {}
protected:
    virtual void relocate1(const Relocation *, byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerPpc64 final : public ElfLinker {
    typedef ElfLinker super;
public:
    explicit ElfLinkerPpc64() noexcept : super(&N_BELE_RTP::be_policy) {}
protected:
    virtual void relocate1(const Relocation *, byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerPpc64le final : public ElfLinker {
    typedef ElfLinker super;
protected:
    virtual void relocate1(const Relocation *, byte *location, upx_uint64_t value,
                           const char *type) override;
};

class ElfLinkerX86 final : public ElfLinker {
    typedef ElfLinker super;
protected:
    virtual void alignCode(unsigned len) override { alignWithByte(len, 0x90); }
    virtual void relocate1(const Relocation *, byte *location, upx_uint64_t value,
                           const char *type) override;
};

/* vim:set ts=4 sw=4 et: */
