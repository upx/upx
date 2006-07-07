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
//
**************************************************************************/

class Linker
{
protected:
    Linker() : frozen(false) { }
public:
    virtual ~Linker() { }
    // endian abstraction
    virtual unsigned get32(const void *b) const = 0;
    virtual void set32(void *b, unsigned v) const = 0;
    //
    virtual bool isFrozen() const { return frozen; }
    //
    virtual void init(const void *pdata, int plen, int pinfo) = 0;
    virtual void setLoaderAlignOffset(int phase) = 0;
    virtual int addSection(const char *sname) = 0;
    virtual void addSection(const char *sname, const void *sdata, int slen) = 0;
    virtual void freeze() = 0;
    virtual int getSection(const char *sname, int *slen=NULL) = 0;
    virtual unsigned char *getLoader(int *llen=NULL) = 0;

    // for ElfLinker descendants
    virtual void defineSymbol(const char *, unsigned) {}
    virtual void relocate() {}
    virtual unsigned getSymbolOffset(const char *) const { return 0; }

protected:
    bool     frozen;

private:
    // disable copy and assignment
    Linker(const Linker &); // {}
    Linker& operator= (const Linker &); // { return *this; }
};


/*************************************************************************
// DefaultLinker
**************************************************************************/

class DefaultLinker : public Linker
{
protected:
    DefaultLinker();
public:
    virtual ~DefaultLinker();
    //
    virtual void init(const void *pdata, int plen, int pinfo);
    virtual void setLoaderAlignOffset(int phase);
    virtual int addSection(const char *sname);
    virtual void addSection(const char *sname, const void *sdata, int slen);
    virtual void freeze();
    virtual int getSection(const char *sname, int *slen=NULL);
    virtual unsigned char *getLoader(int *llen=NULL);

private:
    struct Label;
    struct Jump;
    struct Section;

    unsigned char *iloader, *oloader;
    int      ilen, olen;
    int      info;
    Jump     *jumps;
    int      njumps;
    Section  *sections;
    int      nsections;
    int      align_hack;
    int      align_offset;
};


template <class T>
struct TDefaultLinker : public DefaultLinker
{
    virtual unsigned get32(const void *b) const { return T::get32(b); }
    virtual void set32(void *b, unsigned v) const { T::set32(b, v); }
};

typedef TDefaultLinker<NBELE::BEPolicy> DefaultBELinker;
typedef TDefaultLinker<NBELE::LEPolicy> DefaultLELinker;


/*************************************************************************
// SimpleLinker
**************************************************************************/

class SimpleLinker : public Linker
{
protected:
    SimpleLinker();
public:
    virtual ~SimpleLinker();
    //
    virtual void init(const void *pdata, int plen, int pinfo);
    virtual void setLoaderAlignOffset(int phase);
    virtual int addSection(const char *sname);
    virtual void addSection(const char *sname, const void *sdata, int slen);
    virtual void freeze();
    virtual int getSection(const char *sname, int *slen=NULL);
    virtual unsigned char *getLoader(int *llen=NULL);
private:
    unsigned char *oloader;
    int      olen;
};


template <class T>
struct TSimpleLinker : public SimpleLinker
{
    virtual unsigned get32(const void *b) const { return T::get32(b); }
    virtual void set32(void *b, unsigned v) const { T::set32(b, v); }
};

typedef TSimpleLinker<NBELE::BEPolicy> SimpleBELinker;
typedef TSimpleLinker<NBELE::LEPolicy> SimpleLELinker;


class ElfLinker : public Linker
{
    typedef Linker super;

protected:
    struct Section
    {
        const char *name;
        const void *input;
        upx_byte   *output;
        unsigned size;
        unsigned offset;
        Section *next;

        Section(){}
        Section(const char *n, const void *i, unsigned s) :
            name(n), input(i), output(NULL), size(s), offset(0), next(NULL)
        {}
    };

    struct Symbol
    {
        const char *name;
        Section *section;
        unsigned offset;

        Symbol(){}
        Symbol(const char *n, Section *s, unsigned o) :
            name(n), section(s), offset(o)
        {}
    };

    struct Relocation
    {
        Section *section;
        unsigned offset;
        const char *type;
        Symbol *value;
        unsigned add;           // used in .rela relocations

        Relocation(){}
        Relocation(Section *s, unsigned o, const char *t,
                   Symbol *v, unsigned a) :
            section(s), offset(o), type(t), value(v), add(a)
        {}
    };

    upx_byte    *input;
    int         inputlen;
    upx_byte    *output;
    int         outputlen;

    Section     *head;
    Section     *tail;

    Section     sections[550];
    Symbol      symbols[1000];
    Relocation  relocations[2000];

    unsigned    nsections;
    unsigned    nsymbols;
    unsigned    nrelocations;

    void preprocessSections(char *start, const char *end);
    void preprocessSymbols(char *start, const char *end);
    void preprocessRelocations(char *start, const char *end);
    Section *findSection(const char *name);
    Symbol *findSymbol(const char *name);

public:
    ElfLinker();

protected:
    virtual ~ElfLinker();

    virtual void init(const void *pdata, int plen, int);
    virtual void setLoaderAlignOffset(int phase);
    virtual int addSection(const char *sname);
    virtual void addSection(const char *sname, const void *sdata, int slen);
    virtual void freeze();
    virtual int getSection(const char *sname, int *slen=NULL);
    virtual upx_byte *getLoader(int *llen=NULL);
    virtual void relocate();
    virtual void defineSymbol(const char *name, unsigned value);
    virtual unsigned getSymbolOffset(const char *) const;

    virtual unsigned get32(const void *) const { return 0; }
    virtual void set32(void *, unsigned) const {}

    //

    void alignWithByte(unsigned len, upx_byte b);
    virtual void align(unsigned len);
    virtual void relocate1(Relocation *, upx_byte *location,
                           unsigned value, const char *type);
};

class ElfLinkerX86 : public ElfLinker
{
    typedef ElfLinker super;

protected:
    virtual void align(unsigned len);
    virtual void relocate1(Relocation *, upx_byte *location,
                           unsigned value, const char *type);
};

class ElfLinkerArmLE : public ElfLinker
{
    typedef ElfLinker super;

protected:
    virtual void relocate1(Relocation *, upx_byte *location,
                           unsigned value, const char *type);
};


#endif /* already included */


/*
vi:ts=4:et
*/

