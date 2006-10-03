/* linker.h --

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


#ifndef __UPX_LINKER_H
#define __UPX_LINKER_H


/*************************************************************************
// ElfLinker
**************************************************************************/

class ElfLinker : private nocopy
{
protected:
    struct      Section;
    struct      Symbol;
    struct      Relocation;

    bool        frozen;         // FIXME: can we remove this ?

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

    void preprocessSections(char *start, const char *end);
    void preprocessSymbols(char *start, const char *end);
    void preprocessRelocations(char *start, const char *end);
    Section *findSection(const char *name) const;
    Symbol *findSymbol(const char *name) /*const*/;

    Symbol *addSymbol(const char *name, const char *section, unsigned offset);
    Relocation *addRelocation(const char *section, unsigned off, const char *type,
                              const char *symbol, unsigned add);

public:
    ElfLinker();
    virtual ~ElfLinker();

    virtual void init(const void *pdata, int plen);
    virtual void setLoaderAlignOffset(int phase);
    virtual int addLoader(const char *sname);
    virtual void addSection(const char *sname, const void *sdata, int slen, int align);
    virtual void freeze();
    virtual int getSection(const char *sname, int *slen=NULL);
    virtual upx_byte *getLoader(int *llen=NULL);
    virtual void relocate();
    virtual void defineSymbol(const char *name, unsigned value);
    virtual unsigned getSymbolOffset(const char *) const;
    virtual void dumpSymbols(FILE *fp=NULL) const;

    void alignWithByte(unsigned len, upx_byte b);
    virtual void alignCode(unsigned len) { alignWithByte(len, 0); }
    virtual void alignData(unsigned len) { alignWithByte(len, 0); }

    virtual void relocate1(const Relocation *, upx_byte *location,
                           unsigned value, const char *type);
};


struct ElfLinker::Section : private nocopy
{
    char *name;
    void *input;
    upx_byte *output;
    unsigned size;
    unsigned offset;
    unsigned char align;  // log2
    Section *next;

    Section(const char *n, const void *i, unsigned s, unsigned a=0);
    ~Section();
};


struct ElfLinker::Symbol : private nocopy
{
    char *name;
    Section *section;
    unsigned offset;

    Symbol(const char *n, Section *s, unsigned o);
    ~Symbol();
};


struct ElfLinker::Relocation : private nocopy
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
protected:
    virtual void alignCode(unsigned len);
    virtual void relocate1(const Relocation *, upx_byte *location,
                           unsigned value, const char *type);
};


class ElfLinkerPpc32 : public ElfLinker
{
    typedef ElfLinker super;
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

