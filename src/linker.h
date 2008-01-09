/* linker.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
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
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
 */


#ifndef __UPX_LINKER_H
#define __UPX_LINKER_H


/*************************************************************************
// ElfLinker
**************************************************************************/

class ElfLinker : private noncopyable
{
    friend class Packer;
public:
    const N_BELE_RTP::AbstractPolicy *bele; // target endianness
protected:
    struct      Section;
    struct      Symbol;
    struct      Relocation;

    upx_byte    *input;
    int         inputlen;
    upx_byte    *output;
    int         outputlen;

    Section     *head;
    Section     *tail;

    Section     **sections;
    Symbol      **symbols;
    Relocation  **relocations;

    unsigned    nsections;
    unsigned    nsections_capacity;
    unsigned    nsymbols;
    unsigned    nsymbols_capacity;
    unsigned    nrelocations;
    unsigned    nrelocations_capacity;

    bool        reloc_done;

protected:
    void preprocessSections(char *start, char *end);
    void preprocessSymbols(char *start, char *end);
    void preprocessRelocations(char *start, char *end);
    Section *findSection(const char *name, bool fatal=true) const;
    Symbol *findSymbol(const char *name, bool fatal=true) const;

    Symbol *addSymbol(const char *name, const char *section, unsigned offset);
    Relocation *addRelocation(const char *section, unsigned off, const char *type,
                              const char *symbol, unsigned add);

public:
    ElfLinker();
    virtual ~ElfLinker();

    virtual void init(const void *pdata, int plen);
    //virtual void setLoaderAlignOffset(int phase);
    virtual int addLoader(const char *sname);
    void addLoader(const char *s, va_list ap);
#if 1 && (ACC_CC_GNUC >= 0x040100)
    void __acc_cdecl_va addLoaderVA(const char *s, ...) __attribute__((__sentinel__));
#else
    void __acc_cdecl_va addLoaderVA(const char *s, ...);
#endif
    virtual Section *addSection(const char *sname, const void *sdata, int slen, unsigned p2align);
    virtual int getSection(const char *sname, int *slen=NULL) const;
    virtual int getSectionSize(const char *sname) const;
    virtual upx_byte *getLoader(int *llen=NULL) const;
    virtual void defineSymbol(const char *name, unsigned value);
    virtual unsigned getSymbolOffset(const char *) const;

    virtual void dumpSymbol(const Symbol *, unsigned flags, FILE *fp) const;
    virtual void dumpSymbols(unsigned flags=0, FILE *fp=NULL) const;

    void alignWithByte(unsigned len, unsigned char b);
    virtual void alignCode(unsigned len) { alignWithByte(len, 0); }
    virtual void alignData(unsigned len) { alignWithByte(len, 0); }

protected:
    virtual void relocate();
    virtual void relocate1(const Relocation *, upx_byte *location,
                           unsigned value, const char *type);

    // target endianness abstraction
    unsigned get_te16(const void *p)        const { return bele->get16(p); }
    unsigned get_te32(const void *p)        const { return bele->get32(p); }
    acc_uint64l_t get_te64(const void *p)   const { return bele->get64(p); }
    void set_te16(void *p, unsigned v)      const { bele->set16(p, v); }
    void set_te32(void *p, unsigned v)      const { bele->set32(p, v); }
    void set_te64(void *p, acc_uint64l_t v) const { bele->set64(p, v); }
};


struct ElfLinker::Section : private noncopyable
{
    char *name;
    void *input;
    upx_byte *output;
    unsigned size;
    unsigned offset;
    unsigned p2align;   // log2
    Section *next;

    Section(const char *n, const void *i, unsigned s, unsigned a=0);
    ~Section();
};


struct ElfLinker::Symbol : private noncopyable
{
    char *name;
    Section *section;
    unsigned offset;

    Symbol(const char *n, Section *s, unsigned o);
    ~Symbol();
};


struct ElfLinker::Relocation : private noncopyable
{
    const Section *section;
    unsigned offset;
    const char *type;
    const Symbol *value;
    unsigned add;           // used in .rela relocations

    Relocation(const Section *s, unsigned o, const char *t,
               const Symbol *v, unsigned a);
};


/*************************************************************************
// ElfLinker arch subclasses
**************************************************************************/

class ElfLinkerAMD64 : public ElfLinker
{
    typedef ElfLinker super;
protected:
    virtual void alignCode(unsigned len) { alignWithByte(len, 0x90); }
    virtual void relocate1(const Relocation *, upx_byte *location,
                           unsigned value, const char *type);
};


class ElfLinkerArmBE : public ElfLinker
{
    typedef ElfLinker super;
public:
    ElfLinkerArmBE() { bele = &N_BELE_RTP::be_policy; }
protected:
    virtual void relocate1(const Relocation *, upx_byte *location,
                           unsigned value, const char *type);
};


class ElfLinkerArmLE : public ElfLinker
{
    typedef ElfLinker super;
protected:
    virtual void relocate1(const Relocation *, upx_byte *location,
                           unsigned value, const char *type);
};


class ElfLinkerM68k : public ElfLinker
{
    typedef ElfLinker super;
public:
    ElfLinkerM68k() { bele = &N_BELE_RTP::be_policy; }
protected:
    virtual void alignCode(unsigned len);
    virtual void relocate1(const Relocation *, upx_byte *location,
                           unsigned value, const char *type);
};


class ElfLinkerMipsBE : public ElfLinker
{
    typedef ElfLinker super;
public:
    ElfLinkerMipsBE() { bele = &N_BELE_RTP::be_policy; }
protected:
    virtual void relocate1(const Relocation *, upx_byte *location,
                           unsigned value, const char *type);
};


class ElfLinkerMipsLE : public ElfLinker
{
    typedef ElfLinker super;
protected:
    virtual void relocate1(const Relocation *, upx_byte *location,
                           unsigned value, const char *type);
};


class ElfLinkerPpc32 : public ElfLinker
{
    typedef ElfLinker super;
public:
    ElfLinkerPpc32() { bele = &N_BELE_RTP::be_policy; }
protected:
    virtual void relocate1(const Relocation *, upx_byte *location,
                           unsigned value, const char *type);
};


class ElfLinkerX86 : public ElfLinker
{
    typedef ElfLinker super;
protected:
    virtual void alignCode(unsigned len) { alignWithByte(len, 0x90); }
    virtual void relocate1(const Relocation *, upx_byte *location,
                           unsigned value, const char *type);
};



#endif /* already included */


/*
vi:ts=4:et
*/

