/* p_w32pe.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2003 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2003 Laszlo Molnar
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
   markus@oberhumer.com      ml1050@cdata.tvnet.hu
 */


#include "conf.h"
#include "file.h"
#include "filter.h"
#include "packer.h"
#include "p_w32pe.h"

static const
#include "stub/l_w32pe.h"

#define IDSIZE(x)       ih.ddirs[x].size
#define IDADDR(x)       ih.ddirs[x].vaddr
#define ODSIZE(x)       oh.ddirs[x].size
#define ODADDR(x)       oh.ddirs[x].vaddr

#define isdll           ((ih.flags & DLL_FLAG) != 0)

#define FILLVAL         0


// Some defines to keep the namespace clean.
// It would be better to use inner classes except for Interval, which
// could be used elsewhere too.

#define Interval        PackW32Pe_Interval
#define Reloc           PackW32Pe_Reloc
#define Resource        PackW32Pe_Resource
#define import_desc     PackW32Pe_import_desc
#define Export          PackW32Pe_Export
#define tls             PackW32Pe_tls


/*************************************************************************
//
**************************************************************************/

#if defined(__BORLANDC__)
#  undef strcpy
#  define strcpy(a,b)   strcpy((char *)(a),(const char *)(b))
#endif


// Unicode string compare
static bool ustrsame(const void *s1, const void *s2)
{
    unsigned len1 = get_le16(s1);
    unsigned len2 = get_le16(s2);
    if (len1 != len2)
        return false;
    return memcmp(s1, s2, 2 + 2*len1) == 0;
}


/*************************************************************************
//
**************************************************************************/

PackW32Pe::PackW32Pe(InputFile *f) : super(f)
{
    //printf("pe_header_t %d\n", (int) sizeof(pe_header_t));
    //printf("pe_section_t %d\n", (int) sizeof(pe_section_t));
    COMPILE_TIME_ASSERT(sizeof(pe_header_t) == 248);
    COMPILE_TIME_ASSERT(sizeof(pe_section_t) == 40);
    COMPILE_TIME_ASSERT(RT_LAST == TABLESIZE(opt->w32pe.compress_rt));

    isection = NULL;
    oimport = NULL;
    oimpdlls = NULL;
    orelocs = NULL;
    oexport = NULL;
    otls = NULL;
    oresources = NULL;
    oxrelocs = NULL;
    icondir_offset = 0;
    icondir_count = 0;
    importbyordinal = false;
    kernel32ordinal = false;
    tlsindex = 0;
    big_relocs = 0;
}


PackW32Pe::~PackW32Pe()
{
    delete [] isection;
    delete [] orelocs;
    delete [] oimport;
    delete [] oimpdlls;
    delete [] oexport;
    delete [] otls;
    delete [] oresources;
    delete [] oxrelocs;
    //delete res;
}


const int *PackW32Pe::getCompressionMethods(int method, int level) const
{
    bool small = ih.codesize + ih.datasize <= 256*1024;
    return Packer::getDefaultCompressionMethods_le32(method, level, small);
}


const int *PackW32Pe::getFilters() const
{
    static const int filters[] = {
        0x26, 0x24, 0x16, 0x13, 0x14, 0x11, 0x25, 0x15, 0x12,
    -1 };
    return filters;
}


bool PackW32Pe::testUnpackVersion(int version) const
{
    if (version != ph_version && ph_version != -1)
        throwCantUnpack("program has been modified; run a virus checker!");
    if (!canUnpackVersion(version))
        throwCantUnpack("this program is packed with an obsolete version and cannot be unpacked");
    return true;
}


/*************************************************************************
// util
**************************************************************************/

int PackW32Pe::readFileHeader()
{
    struct exe_header_t
    {
        LE16 mz;
        LE16 m512;
        LE16 p512;
        char _[18];
        LE16 relocoffs;
        char __[34];
        LE32 nexepos;
    }
    __attribute_packed;
    COMPILE_TIME_ASSERT(sizeof(exe_header_t) == 64);

    exe_header_t h;
    int ic;
    pe_offset = 0;

    for (ic = 0; ic < 20; ic++)
    {
        fi->seek(pe_offset,SEEK_SET);
        fi->readx(&h,sizeof(h));

        if (h.mz == 'M' + 'Z'*256) // dos exe
        {
            if (h.relocoffs >= 0x40)      // new format exe
                pe_offset += h.nexepos;
            else
                pe_offset += h.p512*512+h.m512 - h.m512 ? 512 : 0;
        }
        else if (get_le32(&h) == 'P' + 'E'*256)
            break;
        else
            return 0;
    }
    if (ic == 20)
        return 0;
    fi->seek(pe_offset,SEEK_SET);
    fi->readx(&ih,sizeof(ih));
    fi->seek(0x200,SEEK_SET);
    fi->readx(&h,6);
    isrtm = memcmp(&h,"32STUB",6) == 0;
    return UPX_F_WIN32_PE;
}


/*************************************************************************
// interval handling
**************************************************************************/

class Interval
{
    unsigned capacity;
    void *base;
public:
    struct interval
    {
        unsigned start, len;
    } *ivarr;

    unsigned ivnum;

    Interval(void *b) : capacity(0),base(b),ivarr(NULL),ivnum(0) {}
    ~Interval() {free(ivarr);}

    void add(unsigned start,unsigned len);
    void add(const void *start,unsigned len) {add(ptr_diff(start,base),len);}
    void add(const void *start,const void *end) {add(ptr_diff(start,base),ptr_diff(end,start));}
    void add(const Interval *iv);
    void flatten();

    void clear();
    void dump() const;

private:
    static int __acc_cdecl_qsort compare(const void *p1,const void *p2)
    {
        const interval *i1 = (const interval*) p1;
        const interval *i2 = (const interval*) p2;
        if (i1->start < i2->start) return -1;
        if (i1->start > i2->start) return 1;
        if (i1->len < i2->len) return 1;
        if (i1->len > i2->len) return -1;
        return 0;
    }
};

void Interval::add(unsigned start,unsigned len)
{
    if (ivnum == capacity)
        ivarr = (interval*) realloc(ivarr,(capacity += 15) * sizeof (interval));
    ivarr[ivnum].start = start;
    ivarr[ivnum++].len = len;
}

void Interval::add(const Interval *iv)
{
    for (unsigned ic = 0; ic < iv->ivnum; ic++)
        add(iv->ivarr[ic].start,iv->ivarr[ic].len);
}

void Interval::flatten()
{
    if (!ivnum)
        return;
    qsort(ivarr,ivnum,sizeof (interval),Interval::compare);
    for (unsigned ic = 0; ic < ivnum - 1; ic++)
    {
        unsigned jc;
        for (jc = ic + 1; jc < ivnum && ivarr[ic].start + ivarr[ic].len >= ivarr[jc].start; jc++)
            if (ivarr[ic].start + ivarr[ic].len < ivarr[jc].start + ivarr[jc].len)
                ivarr[ic].len = ivarr[jc].start + ivarr[jc].len - ivarr[ic].start;
        if (jc > ic + 1)
        {
            memmove(ivarr + ic + 1, ivarr + jc,sizeof(interval) * (ivnum - jc));
            ivnum -= jc - ic - 1;
        }
    }
}

void Interval::clear()
{
    for (unsigned ic = 0; ic < ivnum; ic++)
        memset((char*) base + ivarr[ic].start,0,ivarr[ic].len);
}

void Interval::dump() const
{
    printf("%d intervals:\n",ivnum);
    for (unsigned ic = 0; ic < ivnum; ic++)
        printf("%x %x\n",ivarr[ic].start,ivarr[ic].len);
}


/*************************************************************************
// relocation handling
**************************************************************************/

class Reloc
{
    upx_byte *start;
    unsigned size;

    struct reloc
    {
        LE32  pagestart;
        LE32  size;
    }
    __attribute_packed;

    void newRelocPos(void *p)
    {
        rel = (reloc*) p;
        rel1 = (LE16*) ((char*) p + sizeof (reloc));
    }

    reloc *rel;
    LE16 *rel1;
    unsigned counts[16];

public:
    Reloc(upx_byte *,unsigned);
    Reloc(unsigned rnum);
    //
    bool next(unsigned &pos,unsigned &type);
    const unsigned *getcounts() const { return counts; }
    //
    void add(unsigned pos,unsigned type);
    void finish(upx_byte *&p,unsigned &size);
};

Reloc::Reloc(upx_byte *s,unsigned si) :
    start(s), size(si), rel(NULL), rel1(NULL)
{
    COMPILE_TIME_ASSERT(sizeof(reloc) == 8);
    memset(counts,0,sizeof(counts));
    unsigned pos,type;
    while (next(pos,type))
        counts[type]++;
}

Reloc::Reloc(unsigned rnum) :
    start(NULL), size(0), rel(NULL), rel1(NULL)
{
    start = new upx_byte[rnum * 4 + 8192];
    counts[0] = 0;
}

bool Reloc::next(unsigned &pos,unsigned &type)
{
    if (!rel)
        newRelocPos(start);
    if (ptr_diff(rel, start) >= (int) size || rel->pagestart == 0)
        return rel = 0,false; // rewind

    pos = rel->pagestart + (*rel1 & 0xfff);
    type = *rel1++ >> 12;
    //printf("%x %d\n",pos,type);
    if (ptr_diff(rel1,rel) >= (int) rel->size)
        newRelocPos(rel1);
    return type == 0 ? next(pos,type) : true;
}

void Reloc::add(unsigned pos,unsigned type)
{
    set_le32(start + 1024 + 4 * counts[0]++,(pos << 4) + type);
}

void Reloc::finish(upx_byte *&p,unsigned &siz)
{
    unsigned prev = 0xffffffff;
    set_le32(start + 1024 + 4 * counts[0]++,0xf0000000);
    qsort(start + 1024,counts[0],4,le32_compare);

    rel = (reloc*) start;
    rel1 = (LE16*) rel;
    for (unsigned ic = 0; ic < counts[0]; ic++)
    {
        unsigned pos = get_le32(start + 1024 + 4 * ic);
        if ((pos ^ prev) >= 0x10000)
        {
            prev = pos;
            *rel1 = 0;
            rel->size = ALIGN_UP(ptr_diff(rel1,rel),4);
            newRelocPos((char *)rel + rel->size);
            rel->pagestart = (pos >> 4) &~ 0xfff;
        }
        *rel1++ = (pos << 12) + ((pos >> 4) & 0xfff);
    }
    p = start;
    siz = ptr_diff(rel1,start) &~ 3;
    siz -= 8;
    assert(siz > 0);
    start = 0; // safety
}

void PackW32Pe::processRelocs(Reloc *rel) // pass2
{
    rel->finish(oxrelocs,soxrelocs);
    if (opt->w32pe.strip_relocs && !isdll)
        soxrelocs = 0;
}

void PackW32Pe::processRelocs() // pass1
{
    big_relocs = 0;

    Reloc rel(ibuf + IDADDR(PEDIR_RELOC),IDSIZE(PEDIR_RELOC));
    const unsigned *counts = rel.getcounts();
    const unsigned rnum = counts[1] + counts[2] + counts[3];

    if ((opt->w32pe.strip_relocs && !isdll) || rnum == 0)
    {
        if (IDSIZE(PEDIR_RELOC))
            ibuf.fill(IDADDR(PEDIR_RELOC), IDSIZE(PEDIR_RELOC), FILLVAL);
        orelocs = new upx_byte [1];
        sorelocs = 0;
        return;
    }

    unsigned ic;
    for (ic = 15; ic > 3; ic--)
        if (counts[ic])
            infoWarning("skipping unsupported relocation type %d (%d)",ic,counts[ic]);

    LE32 *fix[4];
    for (; ic; ic--)
        fix[ic] = new LE32 [counts[ic]];

    // prepare sorting
    unsigned pos,type;
    while (rel.next(pos,type))
    {
        if (type == 3)
            set_le32(ibuf + pos,get_le32(ibuf + pos) - ih.imagebase - rvamin);
        if (type < 4)
            *fix[type]++ = pos - rvamin;
    }
    fix[3] -= counts[3];

    ibuf.fill(IDADDR(PEDIR_RELOC), IDSIZE(PEDIR_RELOC), FILLVAL);
    orelocs = new upx_byte [rnum * 4 + 1024];  // 1024 - safety
    sorelocs = ptr_diff(optimizeReloc32((upx_byte*) fix[3],counts[3],orelocs,ibuf + rvamin,1,&big_relocs),orelocs);

    // append relocs type "LOW" then "HIGH"
    for (ic = 2; ic ; ic--)
    {
        fix[ic] -= counts[ic];
        memcpy(orelocs + sorelocs,fix[ic],4 * counts[ic]);
        sorelocs += 4 * counts[ic];
        delete [] fix[ic];

        set_le32(orelocs + sorelocs,0);
        if (counts[ic])
        {
            sorelocs += 4;
            big_relocs |= 2 * ic;
        }
    }
    delete [] fix[3];
    info("Relocations: original size: %u bytes, preprocessed size: %u bytes",(unsigned) IDSIZE(PEDIR_RELOC),sorelocs);
}


/*************************************************************************
// import handling
**************************************************************************/

struct import_desc
{
    LE32  oft;      // orig first thunk
    char  _[8];
    LE32  dllname;
    LE32  iat;      // import address table
}
__attribute_packed;

void PackW32Pe::processImports(unsigned myimport) // pass 2
{
    COMPILE_TIME_ASSERT(sizeof(import_desc) == 20);

    // adjust import data
    for (import_desc *im = (import_desc*) oimpdlls; im->dllname; im++)
    {
        if (im->dllname < myimport)
            im->dllname += myimport;
        LE32 *p = (LE32*) (oimpdlls + im->iat);
        im->iat += myimport;

        while (*p)
            if ((*p++ & 0x80000000) == 0)  // import by name?
                p[-1] += myimport;
    }
}

unsigned PackW32Pe::processImports() // pass 1
{
    static const upx_byte kernel32dll[] = "KERNEL32.DLL";
    static const char llgpa[] = "\x0\x0""LoadLibraryA\x0\x0""GetProcAddress\x0\x0";
    static const char exitp[] = "ExitProcess\x0\x0\x0";

    unsigned dllnum = 0;
    import_desc *im = (import_desc*) (ibuf + IDADDR(PEDIR_IMPORT));
    import_desc * const im_save = im;
    if (IDADDR(PEDIR_IMPORT))
    {
        while (im->dllname)
            dllnum++, im++;
        im = im_save;
    }

    struct udll
    {
        const upx_byte *name;
        const upx_byte *shname;
        unsigned   ordinal;
        unsigned   iat;
        LE32       *lookupt;
        unsigned   npos;
        bool       isk32;
        unsigned   _;           // padding to 32

        static int __acc_cdecl_qsort compare(const void *p1, const void *p2)
        {
            const udll *u1 = * (const udll * const *) p1;
            const udll *u2 = * (const udll * const *) p2;
            if (u1->isk32) return -1;
            if (u2->isk32) return 1;
            int rc = strcasecmp(u1->name,u2->name);
            if (rc) return rc;
            if (u1->ordinal) return -1;
            if (u2->ordinal) return 1;
            if (!u1->shname) return 1;
            if (!u2->shname) return -1;
            return strlen(u1->shname) - strlen(u2->shname);
        }
    }
    __attribute_packed;

    // +1 for dllnum=0
    Array(struct udll, dlls, dllnum+1);
    Array(struct udll *, idlls, dllnum+1);

    soimport = 1024; // safety

    unsigned ic,k32o;
    for (ic = k32o = 0; dllnum && im->dllname; ic++, im++)
    {
        idlls[ic] = dlls + ic;
        dlls[ic].name = ibuf + im->dllname;
        dlls[ic].shname = NULL;
        dlls[ic].ordinal = 0;
        dlls[ic].iat = im->iat;
        dlls[ic].lookupt = (LE32*) (ibuf + (im->oft ? im->oft : im->iat));
        dlls[ic].npos = 0;
        dlls[ic].isk32 = strcasecmp(kernel32dll,dlls[ic].name) == 0;

        soimport += strlen(dlls[ic].name) + 1 + 4;

        for (LE32 *tarr = dlls[ic].lookupt; *tarr; tarr++)
        {
            if (*tarr & 0x80000000)
            {
                importbyordinal = true;
                soimport += 2; // ordinal num: 2 bytes
                dlls[ic].ordinal = *tarr & 0xffff;
                if (dlls[ic].isk32)
                    kernel32ordinal = true,k32o++;
            }
            else
            {
                unsigned len = strlen(ibuf + *tarr + 2);
                soimport += len + 1;
                if (dlls[ic].shname == NULL || len < strlen (dlls[ic].shname))
                    dlls[ic].shname = ibuf + *tarr + 2;
            }
            soimport++; // separator
        }
    }
    oimport = new upx_byte[soimport];
    memset(oimport,0,soimport);
    oimpdlls = new upx_byte[soimport];
    memset(oimpdlls,0,soimport);

    qsort(idlls,dllnum,sizeof (udll*),udll::compare);

    unsigned dllnamelen = sizeof (kernel32dll);
    unsigned dllnum2 = 1;
    for (ic = 0; ic < dllnum; ic++)
        if (!idlls[ic]->isk32 && (ic == 0 || strcasecmp(idlls[ic - 1]->name,idlls[ic]->name)))
        {
            dllnum2++;
            dllnamelen += strlen(idlls[ic]->name) + 1;
        }
    //fprintf(stderr,"dllnum=%d dllnum2=%d soimport=%d\n",dllnum,dllnum2,soimport); //

    info("Processing imports: %d DLLs", dllnum);

    // create the new import table
    im = (import_desc*) oimpdlls;

    LE32 *ordinals = (LE32*) (oimpdlls + (dllnum2 + 1) * sizeof(import_desc));
    LE32 *lookuptable = ordinals + 3 + k32o + (isdll ? 0 : 1);
    upx_byte *dllnames = ((upx_byte*) lookuptable) + (dllnum2 - 1) * 8;
    upx_byte *importednames = dllnames + (dllnamelen &~ 1);

    unsigned k32namepos = ptr_diff(dllnames,oimpdlls);

    memcpy(importednames,llgpa,sizeof(llgpa));
    if (!isdll)
        memcpy(importednames + sizeof(llgpa) - 1,exitp,sizeof(exitp));
    strcpy(dllnames,kernel32dll);
    im->dllname = k32namepos;
    im->iat = ptr_diff(ordinals,oimpdlls);
    *ordinals++ = ptr_diff(importednames,oimpdlls);
    *ordinals++ = ptr_diff(importednames,oimpdlls) + 14;
    if (!isdll)
        *ordinals++ = ptr_diff(importednames,oimpdlls) + sizeof(llgpa) - 3;
    dllnames += sizeof(kernel32dll);
    importednames += sizeof(llgpa) - 2 + (isdll ? 0 : sizeof(exitp) - 1);

    im++;
    for (ic = 0; ic < dllnum; ic++)
        if (idlls[ic]->isk32)
        {
            idlls[ic]->npos = k32namepos;
            if (idlls[ic]->ordinal)
                for (LE32 *tarr = idlls[ic]->lookupt; *tarr; tarr++)
                    if (*tarr & 0x80000000)
                        *ordinals++ = *tarr;
        }
        else if (ic && strcasecmp(idlls[ic-1]->name,idlls[ic]->name) == 0)
            idlls[ic]->npos = idlls[ic-1]->npos;
        else
        {
            im->dllname = idlls[ic]->npos = ptr_diff(dllnames,oimpdlls);
            im->iat = ptr_diff(lookuptable,oimpdlls);

            strcpy(dllnames,idlls[ic]->name);
            dllnames += strlen(idlls[ic]->name)+1;
            if (idlls[ic]->ordinal)
                *lookuptable = idlls[ic]->ordinal + 0x80000000;
            else if (idlls[ic]->shname)
            {
                if (ptr_diff(importednames,oimpdlls) & 1)
                    importednames--;
                *lookuptable = ptr_diff(importednames,oimpdlls);
                importednames += 2;
                strcpy(importednames,idlls[ic]->shname);
                importednames += strlen(idlls[ic]->shname) + 1;
            }
            lookuptable += 2;
            im++;
        }
    soimpdlls = ALIGN_UP(ptr_diff(importednames,oimpdlls),4);

    Interval names(ibuf),iats(ibuf),lookups(ibuf);
    // create the preprocessed data
    ordinals -= k32o;
    upx_byte *ppi = oimport;  // preprocessed imports
    for (ic = 0; ic < dllnum; ic++)
    {
        LE32 *tarr = idlls[ic]->lookupt;
        if (!*tarr)  // no imports from this dll
            continue;
        set_le32(ppi,idlls[ic]->npos);
        set_le32(ppi+4,idlls[ic]->iat - rvamin);
        ppi += 8;
        for (; *tarr; tarr++)
            if (*tarr & 0x80000000)
            {
                if (idlls[ic]->isk32)
                {
                    *ppi++ = 0xfe; // signed + odd parity
                    set_le32(ppi,ptr_diff(ordinals,oimpdlls));
                    ordinals++;
                    ppi += 4;
                }
                else
                {
                    *ppi++ = 0xff;
                    set_le16(ppi,*tarr & 0xffff);
                    ppi += 2;
                }
            }
            else
            {
                *ppi++ = 1;
                unsigned len = strlen(ibuf + *tarr + 2) + 1;
                memcpy(ppi,ibuf + *tarr + 2,len);
                ppi += len;
                names.add(*tarr,len + 2 + 1);
            }
        ppi++;

        unsigned esize = ptr_diff(tarr,idlls[ic]->lookupt);
        lookups.add(idlls[ic]->lookupt,esize);
        if (ptr_diff(ibuf + idlls[ic]->iat,idlls[ic]->lookupt))
        {
            memcpy(ibuf + idlls[ic]->iat,idlls[ic]->lookupt,esize);
            iats.add(idlls[ic]->iat,esize);
        }
        names.add(idlls[ic]->name,strlen(idlls[ic]->name) + 1 + 1);
    }
    ppi += 4;
    assert(ppi < oimport+soimport);
    soimport = ptr_diff(ppi,oimport);

    if (soimport == 4)
        soimport = 0;

    //OutputFile::dump("x0.imp", oimport, soimport);
    //OutputFile::dump("x1.imp", oimpdlls, soimpdlss);

    unsigned ilen = 0;
    names.flatten();
    if (names.ivnum > 1)
    {
        // The area occupied by the dll and imported names is not continuous
        // so to still support uncompression, I can't zero the iat area.
        // This decreases compression ratio, so FIXME somehow.
        infoWarning("can't remove unneeded imports");
        ilen += sizeof(import_desc) * dllnum;
#if defined(DEBUG)
        if (opt->verbose > 3)
            names.dump();
#endif
        // do some work for the unpacker
        im = im_save;
        for (ic = 0; ic < dllnum; ic++, im++)
        {
            memset(im,FILLVAL,sizeof(*im));
            im->dllname = ptr_diff(idlls[ic]->name,ibuf); // I only need this info
        }
    }
    else
    {
        iats.add(im_save,sizeof(import_desc) * dllnum);
        // zero unneeded data
        iats.clear();
        lookups.clear();
    }
    names.clear();

    iats.add(&names);
    iats.add(&lookups);
    iats.flatten();
    for (ic = 0; ic < iats.ivnum; ic++)
        ilen += iats.ivarr[ic].len;

    info("Imports: original size: %u bytes, preprocessed size: %u bytes",ilen,soimport);
    return names.ivnum == 1 ? names.ivarr[0].start : 0;
}


/*************************************************************************
// export handling
**************************************************************************/

class Export
{
    struct export_dir_t
    {
        char  _[12]; // flags, timedate, version
        LE32  name;
        char  __[4]; // ordinal base
        LE32  functions;
        LE32  names;
        LE32  addrtable;
        LE32  nameptrtable;
        LE32  ordinaltable;
    }
    __attribute_packed;

    export_dir_t edir;
    char  *ename;
    char  *functionptrs;
    char  *ordinals;
    char  **names;

    char  *base;
    unsigned size;
    Interval iv;

public:
    Export(char *base);
    ~Export();

    void convert(unsigned eoffs,unsigned esize);
    void build(char *base,unsigned newoffs);
    unsigned getsize() const { return size; }

private:
    Export(const Export&);
    Export& operator=(const Export&);
};

Export::Export(char *_base) : base(_base), iv(_base)
{
    COMPILE_TIME_ASSERT(sizeof(export_dir_t) == 40);
    ename = functionptrs = ordinals = NULL;
    names = NULL;
    memset(&edir,0,sizeof(edir));
    size = 0;
}

Export::~Export()
{
    free(ename);
    delete [] functionptrs;
    delete [] ordinals;
    for (unsigned ic = 0; ic < edir.names + edir.functions; ic++)
        free(names[ic]);
    delete [] names;
}

void Export::convert(unsigned eoffs,unsigned esize)
{
    memcpy(&edir,base + eoffs,sizeof(export_dir_t));
    size = sizeof(export_dir_t);
    iv.add(eoffs,size);

    unsigned len = strlen(base + edir.name) + 1;
    ename = strdup(base + edir.name);
    size += len;
    iv.add(edir.name,len);

    len = 4 * edir.functions;
    functionptrs = new char[len + 1];
    memcpy(functionptrs,base + edir.addrtable,len);
    size += len;
    iv.add(edir.addrtable,len);

    unsigned ic;
    names = new char* [edir.names + edir.functions + 1];
    for (ic = 0; ic < edir.names; ic++)
    {
        char *n = base + get_le32(base + edir.nameptrtable + ic * 4);
        len = strlen(n) + 1;
        names[ic] = strdup(n);
        size += len;
        iv.add(get_le32(base + edir.nameptrtable + ic * 4),len);
    }
    iv.add(edir.nameptrtable,4 * edir.names);
    size += 4 * edir.names;

    LE32 *fp = (LE32*) functionptrs;
    // export forwarders
    for (ic = 0; ic < edir.functions; ic++)
        if (fp[ic] >= eoffs && fp[ic] < eoffs + esize)
        {
            char *forw = base + fp[ic];
            len = strlen(forw) + 1;
            iv.add(forw,len);
            size += len;
            names[ic + edir.names] = strdup(forw);
        }
        else
            names[ic + edir.names] = 0;

    len = 2 * edir.names;
    ordinals = new char[len + 1];
    memcpy(ordinals,base + edir.ordinaltable,len);
    size += len;
    iv.add(edir.ordinaltable,len);
    iv.flatten();
    if (iv.ivnum == 1)
        iv.clear();
#if defined(DEBUG)
    else
        iv.dump();
#endif
}

void Export::build(char *newbase,unsigned newoffs)
{
    char *functionp = newbase + sizeof(edir);
    char *namep = functionp + 4 * edir.functions;
    char *ordinalp = namep + 4 * edir.names;
    char *enamep = ordinalp + 2 * edir.names;
    char *exports = enamep + strlen(ename) + 1;

    edir.addrtable = newoffs + functionp - newbase;
    edir.ordinaltable = newoffs + ordinalp - newbase;
    memcpy(ordinalp,ordinals,2 * edir.names);

    edir.name = newoffs + enamep - newbase;
    strcpy(enamep,ename);
    edir.nameptrtable = newoffs + namep - newbase;
    unsigned ic;
    for (ic = 0; ic < edir.names; ic++)
    {
        strcpy(exports,names[ic]);
        set_le32(namep + 4 * ic,newoffs + exports - newbase);
        exports += strlen(exports) + 1;
    }

    memcpy(functionp,functionptrs,4 * edir.functions);
    for (ic = 0; ic < edir.functions; ic++)
        if (names[edir.names + ic])
        {
            strcpy(exports,names[edir.names + ic]);
            set_le32(functionp + 4 * ic,newoffs + exports - newbase);
            exports += strlen(exports) + 1;
        }

    memcpy(newbase,&edir,sizeof(edir));
    assert(exports - newbase == (int) size);
}

void PackW32Pe::processExports(Export *xport) // pass1
{
    soexport = ALIGN_UP(IDSIZE(PEDIR_EXPORT),4);
    if (soexport == 0)
        return;
    if (!isdll && opt->w32pe.compress_exports)
    {
        infoWarning("exports compressed, --compress-exports=0 might be needed");
        soexport = 0;
        return;
    }
    xport->convert(IDADDR(PEDIR_EXPORT),IDSIZE(PEDIR_EXPORT));
    soexport = ALIGN_UP(xport->getsize(),4);
    oexport = new upx_byte[soexport];
    memset(oexport, 0, soexport);
}

void PackW32Pe::processExports(Export *xport,unsigned newoffs) // pass2
{
    if (soexport)
        xport->build((char*) oexport,newoffs);
}


/*************************************************************************
// TLS handling
**************************************************************************/

// thanks for theowl for providing me some docs, so that now I understand
// what I'm doing here :)

// 1999-10-17: this was tricky to find:
// when the fixup records and the tls area are on the same page, then
// the tls area is not relocated, because the relocation is done by
// the virtual memory manager only for pages which are not yet loaded.
// of course it was impossible to debug this ;-)

struct tls
{
    LE32 datastart; // VA tls init data start
    LE32 dataend;   // VA tls init data end
    LE32 tlsindex;  // VA tls index
    LE32 callbacks; // VA tls callbacks
    char _[8];      // zero init, characteristics
}
__attribute_packed;

void PackW32Pe::processTls(Interval *iv) // pass 1
{
    COMPILE_TIME_ASSERT(sizeof(tls) == 24);

    if ((sotls = ALIGN_UP(IDSIZE(PEDIR_TLS),4)) == 0)
        return;

    const tls * const tlsp = (const tls*) (ibuf + IDADDR(PEDIR_TLS));
    // note: TLS callbacks are not implemented in Windows 95/98/ME
    if (tlsp->callbacks)
    {
        if (tlsp->callbacks < ih.imagebase)
            throwCantPack("invalid TLS callback");
        else if (tlsp->callbacks - ih.imagebase + 4 >= ih.imagesize)
            throwCantPack("invalid TLS callback");
        unsigned v = get_le32(ibuf + tlsp->callbacks - ih.imagebase);
        if (v != 0)
        {
            //fprintf(stderr, "TLS callbacks: 0x%0x -> 0x%0x\n", (int)tlsp->callbacks, v);
            throwCantPack("TLS callbacks are not supported");
        }
    }

    const unsigned tlsdatastart = tlsp->datastart - ih.imagebase;
    const unsigned tlsdataend = tlsp->dataend - ih.imagebase;

    // now some ugly stuff: find the relocation entries in the tls data area
    unsigned pos,type;
    Reloc rel(ibuf + IDADDR(PEDIR_RELOC),IDSIZE(PEDIR_RELOC));
    while (rel.next(pos,type))
        if (pos >= tlsdatastart && pos < tlsdataend)
            iv->add(pos,type);

    sotls = sizeof(tls) + tlsdataend - tlsdatastart;

    // the PE loader wants this stuff uncompressed
    otls = new upx_byte[sotls];
    memset(otls,0,sotls);
    memcpy(otls,ibuf + IDADDR(PEDIR_TLS),0x18);
    // WARNING: this can acces data in BSS
    memcpy(otls + sizeof(tls),ibuf + tlsdatastart,sotls - sizeof(tls));
    tlsindex = tlsp->tlsindex - ih.imagebase;
    info("TLS: %u bytes tls data and %u relocations added",sotls - (unsigned) sizeof(tls),iv->ivnum);
}

void PackW32Pe::processTls(Reloc *rel,const Interval *iv,unsigned newaddr) // pass 2
{
    if (sotls == 0)
        return;
    // add new relocation entries
    unsigned ic;
    for (ic = 0; ic < 12; ic += 4)
        rel->add(newaddr + ic,3);

    tls * const tlsp = (tls*) otls;
    // now the relocation entries in the tls data area
    for (ic = 0; ic < iv->ivnum; ic += 4)
    {
        void *p = otls + iv->ivarr[ic].start - (tlsp->datastart - ih.imagebase) + sizeof(tls);
        unsigned kc = get_le32(p);
        if (kc < tlsp->dataend && kc >= tlsp->datastart)
        {
            kc +=  newaddr + sizeof(tls) - tlsp->datastart;
            set_le32(p,kc + ih.imagebase);
            rel->add(kc,iv->ivarr[ic].len);
        }
        else
            rel->add(kc - ih.imagebase,iv->ivarr[ic].len);
    }

    tlsp->datastart = newaddr + sizeof(tls) + ih.imagebase;
    tlsp->dataend = newaddr + sotls + ih.imagebase;
    tlsp->callbacks = 0; // note: TLS callbacks are not implemented in Windows 95/98/ME
}


/*************************************************************************
// resource handling
**************************************************************************/

class Resource
{
    struct res_dir_entry
    {
        LE32  tnl; // Type | Name | Language id - depending on level
        LE32  child;
    }
    __attribute_packed;
    struct res_dir
    {
        char  _[12]; // flags, timedate, version
        LE16  namedentr;
        LE16  identr;

        unsigned Sizeof() const { return 16 + sizeof(res_dir_entry)*(namedentr + identr); }
        res_dir_entry entries[1];
        // it's usually safe to assume that every res_dir contains
        // at least one res_dir_entry - check() complains otherwise
    }
    __attribute_packed;
    struct res_data
    {
        LE32  offset;
        LE32  size;
        char  _[8]; // codepage, reserved
    }
    __attribute_packed;
    //
    struct upx_rnode
    {
        unsigned        id;
        upx_byte        *name;
        upx_rnode       *parent;
    };
    struct upx_rbranch : public upx_rnode
    {
        unsigned        nc;
        upx_rnode       **children;
        res_dir         data;
    };
    struct upx_rleaf : public upx_rnode
    {
        upx_rleaf       *next;
        unsigned        newoffset;
        res_data        data;
    };

    const upx_byte *start;
    upx_byte   *newstart;
    upx_rnode  *root;
    upx_rleaf  *head;
    upx_rleaf  *current;
    unsigned   dsize;
    unsigned   ssize;

    void check(const res_dir*,unsigned);
    upx_rnode *convert(const void *,upx_rnode *,unsigned);
    void build(const upx_rnode *,unsigned &,unsigned &,unsigned);
    void clear(upx_byte *,unsigned,Interval *);
    void dump(const upx_rnode *,unsigned) const;
    void destroy(upx_rnode *urd,unsigned level);

public:
    Resource() : root(NULL) {}
    Resource(const upx_byte *p) {init(p);}
    ~Resource() {if (root) destroy (root,0);}
    void init(const upx_byte *);

    unsigned dirsize() const {return ALIGN_UP(dsize + ssize,4);}
    bool next() {return (current = current ? current->next : head) != 0;} // wow, builtin autorewind... :-)

    unsigned itype() const {return current->parent->parent->id;}
    const upx_byte *ntype() const {return current->parent->parent->name;}
    unsigned size() const {return ALIGN_UP(current->data.size,4);}
    unsigned offs() const {return current->data.offset;}
    unsigned &newoffs() {return current->newoffset;}

    upx_byte *build();
    bool clear();

    void dump() const { dump(root,0); }
/*
    unsigned iname()  const {return current->parent->id;}
    const upx_byte *nname() const {return current->parent->name;}

    unsigned ilang()  const {return current->id;}
    const upx_byte *nlang() const {return current->name;}
*/
};

void Resource::init(const upx_byte *res)
{
    COMPILE_TIME_ASSERT(sizeof(res_dir_entry) == 8);
    COMPILE_TIME_ASSERT(sizeof(res_dir) == 16 + sizeof(res_dir_entry));
    COMPILE_TIME_ASSERT(sizeof(res_data) == 16);

    start = res;
    root = head = current = 0;
    dsize = ssize = 0;
    check((const res_dir*) start,0);
    root = convert(start,0,0);
}

void Resource::check(const res_dir *node,unsigned level)
{
    int ic = node->identr + node->namedentr;
    if (ic == 0)
    {
        //throwCantPack("unsupported resource structure");
        throwCantPack("empty resource sections are not supported");
    }
    for (const res_dir_entry *rde = node->entries; --ic >= 0; rde++)
        if (((rde->child & 0x80000000) == 0) ^ (level == 2))
            throwCantPack("unsupported resource structure");
        else if (level != 2)
            check((const res_dir*) (start + (rde->child & 0x7fffffff)),level + 1);
}

Resource::upx_rnode *Resource::convert(const void *rnode,upx_rnode *parent,unsigned level)
{
    if (level == 3)
    {
        const res_data *node = (const res_data *) rnode;
        upx_rleaf *leaf = new upx_rleaf;
        leaf->name = 0;
        leaf->parent = parent;
        leaf->next = head;
        leaf->newoffset = 0;
        leaf->data = *node;

        head = leaf; // append node to a linked list for traversal
        dsize += sizeof(res_data);
        return leaf;
    }

    const res_dir *node = (const res_dir *) rnode;
    upx_rbranch *branch = new upx_rbranch;
    branch->name = 0;
    branch->parent = parent;
    int ic = branch->nc = node->identr + node->namedentr;
    branch->children = new upx_rnode*[ic];
    branch->data = *node;

    for (const res_dir_entry *rde = node->entries + ic - 1; --ic >= 0; rde--)
    {
        upx_rnode *child = convert(start + (rde->child & 0x7fffffff),branch,level + 1);
        branch->children[ic] = child;
        child->id = rde->tnl;
        if (child->id & 0x80000000)
        {
            const upx_byte *p = start + (child->id & 0x7fffffff);
            const unsigned len = 2 + 2 * get_le16(p);
            child->name = new upx_byte[len];
            memcpy(child->name,p,len); // copy unicode string
            ssize += len; // size of unicode strings
        }
    }
    dsize += node->Sizeof();
    return branch;
}

void Resource::build(const upx_rnode *node,unsigned &bpos,unsigned &spos,unsigned level)
{
    if (level == 3)
    {
        res_data *l = (res_data*) (newstart + bpos);
        const upx_rleaf *leaf = (const upx_rleaf*) node;
        *l = leaf->data;
        if (leaf->newoffset)
            l->offset = leaf->newoffset;
        bpos += sizeof(*l);
        return;
    }
    res_dir * const b = (res_dir*) (newstart + bpos);
    const upx_rbranch *branch = (const upx_rbranch*) node;
    *b = branch->data;
    bpos += b->Sizeof();
    res_dir_entry *be = b->entries;
    for (unsigned ic = 0; ic < branch->nc; ic++, be++)
    {
        be->tnl = branch->children[ic]->id;
        be->child = bpos + ((level < 2) ? 0x80000000 : 0);

        const upx_byte *p;
        if ((p = branch->children[ic]->name) != 0)
        {
            be->tnl = spos + 0x80000000;
            memcpy(newstart + spos,p,get_le16(p) * 2 + 2);
            spos += get_le16(p) * 2 + 2;
        }

        build(branch->children[ic],bpos,spos,level + 1);
    }
}

upx_byte *Resource::build()
{
    newstart = new upx_byte [dirsize()];
    unsigned bpos = 0,spos = dsize;
    build(root,bpos,spos,0);
    return newstart;
}

void Resource::destroy(upx_rnode *node,unsigned level)
{
    delete [] node->name; node->name = NULL;
    if (level == 3)
        return;
    upx_rbranch * const branch = (upx_rbranch *) node;
    for (int ic = branch->nc; --ic >= 0; )
        destroy(branch->children[ic],level + 1);
    delete [] branch->children; branch->children = NULL;
}

static void lame_print_unicode(const upx_byte *p)
{
    for (unsigned ic = 0; ic < get_le16(p); ic++)
        printf("%c",(char)p[ic * 2 + 2]);
}

void Resource::dump(const upx_rnode *node,unsigned level) const
{
    if (level)
    {
        for (unsigned ic = 1; ic < level; ic++)
            printf("\t\t");
        if (node->name)
            lame_print_unicode(node->name);
        else
            printf("0x%x",node->id);
        printf("\n");
    }
    if (level == 3)
        return;
    const upx_rbranch * const branch = (const upx_rbranch *) node;
    for (unsigned ic = 0; ic < branch->nc; ic++)
        dump(branch->children[ic],level + 1);
}

void Resource::clear(upx_byte *node,unsigned level,Interval *iv)
{
    if (level == 3)
        iv->add(node,sizeof (res_data));
    else
    {
        const res_dir * const rd = (res_dir*) node;
        const unsigned n = rd->identr + rd->namedentr;
        const res_dir_entry *rde = rd->entries;
        for (unsigned ic = 0; ic < n; ic++, rde++)
            clear(newstart + (rde->child & 0x7fffffff),level + 1,iv);
        iv->add(rd,rd->Sizeof());
    }
}

bool Resource::clear()
{
    newstart = const_cast<upx_byte*> (start);
    Interval iv(newstart);
    clear(newstart,0,&iv);
    iv.flatten();
    if (iv.ivnum == 1)
        iv.clear();
#if defined(DEBUG)
    if (opt->verbose > 3)
        iv.dump();
#endif
    return iv.ivnum == 1;
}

void PackW32Pe::processResources(Resource *res,unsigned newaddr)
{
    if (IDSIZE(PEDIR_RESOURCE) == 0)
        return;
    while (res->next())
        if (res->newoffs())
            res->newoffs() += newaddr;
    upx_byte *p = res->build();
    memcpy(oresources,p,res->dirsize());
    delete [] p;
}

void PackW32Pe::processResources(Resource *res)
{
    const unsigned vaddr = IDADDR(PEDIR_RESOURCE);
    if ((soresources = IDSIZE(PEDIR_RESOURCE)) == 0)
        return;

    // setup default options for resource compression
    if (opt->w32pe.compress_resources < 0)
        opt->w32pe.compress_resources = true;
    if (!opt->w32pe.compress_resources)
    {
        opt->w32pe.compress_icons = false;
        for (int i = 0; i < RT_LAST; i++)
            opt->w32pe.compress_rt[i] = false;
    }
    if (opt->w32pe.compress_rt[RT_STRING] < 0)
    {
        // by default, don't compress RT_STRINGs of screensavers (".scr")
        opt->w32pe.compress_rt[RT_STRING] = true;
        if (fn_has_ext(fi->getName(),"scr"))
            opt->w32pe.compress_rt[RT_STRING] = false;
    }

    res->init(ibuf + vaddr);

    for (soresources = res->dirsize(); res->next(); soresources += 4 + res->size())
        ;
    oresources = new upx_byte[soresources];
    upx_byte *ores = oresources + res->dirsize();

    unsigned iconsin1stdir = 0;
    if (opt->w32pe.compress_icons == 2)
        while (res->next()) // there is no rewind() in Resource
            if (res->itype() == RT_GROUP_ICON && iconsin1stdir == 0)
                iconsin1stdir = get_le16(ibuf + res->offs() + 4);

    bool compress_icon = false, compress_idir = false;
    unsigned iconcnt = 0;

    // some statistics
    unsigned usize = 0;
    unsigned csize = 0;
    unsigned unum = 0;
    unsigned cnum = 0;

    while (res->next())
    {
        const unsigned rtype = res->itype();
        bool do_compress = true;
        if (!opt->w32pe.compress_resources)
            do_compress = false;
        else if (rtype == RT_VERSION)       // version info
            do_compress = false;
        else if (rtype == RT_ICON)          // icon
            do_compress = compress_icon && opt->w32pe.compress_icons;
        else if (rtype == RT_GROUP_ICON)    // icon directory
            do_compress = compress_idir && opt->w32pe.compress_icons;
        else if (rtype > 0 && rtype < RT_LAST)
            do_compress = opt->w32pe.compress_rt[rtype] ? true : false;
        else if (res->ntype())              // named resource type
        {
            const upx_byte * const t = res->ntype();
            if (ustrsame(t, "\x7\x0T\x0Y\x0P\x0""E\x0L\x0I\x0""B\x0"))
                do_compress = false;        // u"TYPELIB"
            else if (ustrsame(t, "\x8\x0R\x0""E\x0G\x0I\x0S\x0T\x0R\x0Y\x0"))
                do_compress = false;        // u"REGISTRY"
        }

        if (do_compress)
        {
            csize += res->size();
            cnum++;
            continue;
        }

        usize += res->size();
        unum++;

        set_le32(ores,res->offs()); // save original offset
        ores += 4;
        memcpy(ores, ibuf + res->offs(), res->size());
        ibuf.fill(res->offs(), res->size(), FILLVAL);
        res->newoffs() = ptr_diff(ores,oresources);
        if (rtype == RT_ICON)
            compress_icon = (++iconcnt >= iconsin1stdir || opt->w32pe.compress_icons == 1);
        else if (rtype == RT_GROUP_ICON)
        {
            if (opt->w32pe.compress_icons == 1)
            {
                icondir_offset = 4 + ptr_diff(ores,oresources);
                icondir_count = get_le16(oresources + icondir_offset);
                set_le16(oresources + icondir_offset,1);
            }
            compress_idir = true;
        }
        ores += res->size();
    }
    soresources = ptr_diff(ores,oresources);

    if (!res->clear())
    {
        // The area occupied by the resource directory is not continuous
        // so to still support uncompression, I can't zero this area.
        // This decreases compression ratio, so FIXME somehow.
        infoWarning("can't remove unneeded resource directory");
    }
    info("Resources: compressed %u (%u bytes), not compressed %u (%u bytes)",cnum,csize,unum,usize);
}


unsigned PackW32Pe::virta2objnum(unsigned addr,pe_section_t *sect,unsigned objs)
{
    unsigned ic;
    for (ic = 0; ic < objs; ic++)
    {
        if (sect->vaddr <= addr && sect->vaddr + sect->vsize > addr)
            return ic;
        sect++;
    }
    //throwCantPack("virta2objnum() failed");
    return ic;
}


unsigned PackW32Pe::tryremove (unsigned vaddr,unsigned objs)
{
    unsigned ic = virta2objnum(vaddr,isection,objs);
    if (ic && ic == objs - 1)
    {
        //fprintf(stderr,"removed section: %d size: %lx\n",ic,(long)isection[ic].size);
        info("removed section: %d size: 0x%lx",ic,(long)isection[ic].size);
        objs--;
    }
    return objs;
}


unsigned PackW32Pe::stripDebug(unsigned overlaystart)
{
    if (IDADDR(PEDIR_DEBUG) == 0)
        return overlaystart;

    struct debug_dir_t
    {
        char  _[16]; // flags, time/date, version, type
        LE32  size;
        char  __[4]; // rva
        LE32  fpos;
    }
    __attribute_packed;

    const debug_dir_t *dd = (const debug_dir_t*) (ibuf + IDADDR(PEDIR_DEBUG));
    for (unsigned ic = 0; ic < IDSIZE(PEDIR_DEBUG) / sizeof(debug_dir_t); ic++, dd++)
        if (overlaystart == dd->fpos)
            overlaystart += dd->size;
    ibuf.fill(IDADDR(PEDIR_DEBUG), IDSIZE(PEDIR_DEBUG), FILLVAL);
    return overlaystart;
}


/*************************************************************************
// pack
**************************************************************************/

bool PackW32Pe::canPack()
{
    if (!readFileHeader())
        return false;
    return true;
}


int PackW32Pe::buildLoader(const Filter *ft)
{
    // prepare loader
    initLoader(nrv_loader, sizeof(nrv_loader), -1, 2);
    addLoader(isdll ? "PEISDLL1" : "",
              "PEMAIN01",
              icondir_count > 1 ? (icondir_count == 2 ? "PEICONS1" : "PEICONS2") : "",
              tlsindex ? "PETLSHAK" : "",
              "PEMAIN02",
              getDecompressor(),
              /*multipass ? "PEMULTIP" :  */  "",
              "PEMAIN10",
              NULL
             );
    if (ft->id)
    {
        const unsigned texv = ih.codebase - rvamin;
        assert(ft->calls > 0);
        addLoader(texv ? "PECTTPOS" : "PECTTNUL",NULL);
        addFilter32(ft->id);
    }
    if (soimport)
        addLoader("PEIMPORT",
                  importbyordinal ? "PEIBYORD" : "",
                  kernel32ordinal ? "PEK32ORD" : "",
                  importbyordinal ? "PEIMORD1" : "",
                  "PEIMPOR2",
                  isdll ? "PEIERDLL" : "PEIEREXE",
                  "PEIMDONE",
                  NULL
                 );
    if (sorelocs)
    {
        addLoader(soimport == 0 || soimport + cimports != crelocs ? "PERELOC1" : "PERELOC2",
                  "PERELOC3""RELOC320",
                  big_relocs ? "REL32BIG" : "",
                  "RELOC32J",
                  NULL
                 );
        //FIXME: the following should be moved out of the above if
        addLoader(big_relocs&6 ? "PERLOHI0" : "",
                  big_relocs&4 ? "PERELLO0" : "",
                  big_relocs&2 ? "PERELHI0" : "",
                  NULL
                 );
    }
    addLoader("PEMAIN20",
              ih.entry ? "PEDOJUMP" : "PERETURN",
              "IDENTSTR""UPX1HEAD",
              NULL
             );
    return getLoaderSize();
}


void PackW32Pe::pack(OutputFile *fo)
{
    unsigned objs = ih.objects;
    isection = new pe_section_t[objs];
    fi->seek(pe_offset+sizeof(ih),SEEK_SET);
    fi->readx(isection,sizeof(pe_section_t)*objs);

    rvamin = isection[0].vaddr;

    infoHeader("[Processing %s, format %s, %d sections]", fn_basename(fi->getName()), getName(), objs);

    // check the PE header
    // FIXME: add more checks
    if (!opt->force
        && (ih.cpu < 0x14c || ih.cpu > 0x150
        || ih.opthdrsize != 0xE0
        || (ih.flags & EXECUTABLE) == 0
        || (ih.subsystem != 2 && ih.subsystem != 3 && ih.subsystem != 1)
        || (ih.entry == 0 && !isdll)
        || ih.ddirsentries != 16
        || IDSIZE(PEDIR_EXCEPTION) // is this used on i386?
//        || IDSIZE(PEDIR_COPYRIGHT)
        || IDSIZE(PEDIR_LOADCONF)
        || IDSIZE(PEDIR_COMRT)
       ))
        throwCantPack("unexpected value in PE header (try --force)");

    if (IDSIZE(PEDIR_SEC))
        throwCantPack("compressing certificate info is not supported");
    //if (IDSIZE(PEDIR_DELAYIMP))
    //   throwCantPack("delay load imports are not supported");
    if (isdll)
        opt->w32pe.strip_relocs = 0;
    else if (opt->w32pe.strip_relocs < 0)
        opt->w32pe.strip_relocs = (ih.imagebase >= 0x400000);
    if (opt->w32pe.strip_relocs)
        if (ih.imagebase < 0x400000)
            throwCantPack("--strip-relocs is not allowed when imagebase < 0x400000");
        else
            ih.flags |= RELOCS_STRIPPED;

    if (memcmp(isection[0].name,"UPX",3) == 0)
        throwAlreadyPackedByUPX();
    if (!opt->force && (IDSIZE(15) || ih.entry > isection[1].vaddr))
        throwCantPack("file is possibly packed/protected (try --force)");
    if (ih.entry && ih.entry < rvamin)
        throwCantPack("run a virus scanner on this file!");
    if (!opt->force && ih.subsystem == 1)
        throwCantPack("subsystem `native' is not supported (try --force)");
    if (ih.filealign < 0x200)
        throwCantPack("filealign < 0x200 is not yet supported");

    handleStub(fi,fo,pe_offset);
    const unsigned usize = ih.imagesize;
    const unsigned xtrasize = 65536+IDSIZE(PEDIR_IMPORT)+IDSIZE(PEDIR_BOUNDIM)+IDSIZE(PEDIR_IAT)+IDSIZE(PEDIR_DELAYIMP)+IDSIZE(PEDIR_RELOC);
    ibuf.alloc(usize + xtrasize);

    // BOUND IMPORT support. FIXME: is this ok?
    fi->seek(0,SEEK_SET);
    fi->readx(ibuf,isection[0].rawdataptr);

    Interval holes(ibuf);

    unsigned ic,jc,overlaystart = 0;
    ibuf.clear(0, usize);
    for (ic = jc = 0; ic < objs; ic++)
    {
        if (isection[ic].rawdataptr && overlaystart < isection[ic].rawdataptr + isection[ic].size)
            overlaystart = ALIGN_UP(isection[ic].rawdataptr + isection[ic].size,ih.filealign);
        if (isection[ic].vsize == 0)
            isection[ic].vsize = isection[ic].size;
        if ((isection[ic].flags & PEFL_BSS) || isection[ic].rawdataptr == 0
            || (isection[ic].flags & PEFL_INFO))
        {
            holes.add(isection[ic].vaddr,isection[ic].vsize);
            continue;
        }
        if (isection[ic].vaddr + isection[ic].size > usize)
            throwCantPack("section size problem");
        if (!isrtm && ((isection[ic].flags & (PEFL_WRITE|PEFL_SHARED))
            == (PEFL_WRITE|PEFL_SHARED)))
            if (!opt->force)
                throwCantPack("writeable shared sections not supported (try --force)");
        if (jc && isection[ic].rawdataptr - jc > ih.filealign)
            throwCantPack("superfluous data between sections");
        fi->seek(isection[ic].rawdataptr,SEEK_SET);
        jc = isection[ic].size;
        if (jc > isection[ic].vsize)
            jc = isection[ic].vsize;
        if (isection[ic].vsize == 0) // hack for some tricky programs - may this break other progs?
            jc = isection[ic].vsize = isection[ic].size;
        fi->readx(ibuf + isection[ic].vaddr,jc);
        jc += isection[ic].rawdataptr;
    }

    // check for NeoLite
    if (find(ibuf + ih.entry, 64+7, "NeoLite", 7) >= 0)
        throwCantPack("file is already compressed with another packer");

    unsigned overlay = file_size - stripDebug(overlaystart);
    if (overlay >= (unsigned) file_size)
    {
#if 0
        if (overlay < file_size + ih.filealign)
            overlay = 0;
        else if (!opt->force)
            throwNotCompressible("overlay problem (try --force)");
#endif
        overlay = 0;
    }
    checkOverlay(overlay);

    Resource res;
    Interval tlsiv(ibuf);
    Export xport((char*)(unsigned char*)ibuf);

    const unsigned dllstrings = processImports();
    processTls(&tlsiv); // call before processRelocs!!
    processResources(&res);
    processExports(&xport);
    processRelocs();

    //OutputFile::dump("x1", ibuf, usize);

    // some checks for broken linkers - disable filter if neccessary
    bool allow_filter = true;
    if (ih.codebase == ih.database
        || ih.codebase + ih.codesize > ih.imagesize
        || (isection[virta2objnum(ih.codebase,isection,objs)].flags & PEFL_CODE) == 0)
        allow_filter = false;

    const unsigned oam1 = ih.objectalign-1;

    // FIXME: disabled: the uncompressor would not allocate enough memory
    //objs = tryremove(IDADDR(PEDIR_RELOC),objs);

    // FIXME: if the last object has a bss then this won't work
    // newvsize = (isection[objs-1].vaddr + isection[objs-1].size + oam1) &~ oam1;
    // temporary solution:
    unsigned newvsize = (isection[objs-1].vaddr + isection[objs-1].vsize + oam1) &~ oam1;

    //fprintf(stderr,"newvsize=%x objs=%d\n",newvsize,objs);
    memcpy(ibuf+newvsize,oimport,soimport);
    memcpy(ibuf+newvsize+soimport,orelocs,sorelocs);

    cimports = newvsize - rvamin;   // rva of preprocessed imports
    crelocs = cimports + soimport;  // rva of preprocessed fixups

    ph.u_len = newvsize + soimport + sorelocs;

    // some extra data for uncompression support
    unsigned s = 0;
    upx_byte * const p1 = ibuf + ph.u_len;
    memcpy(p1 + s,&ih,sizeof (ih));
    s += sizeof (ih);
    memcpy(p1 + s,isection,ih.objects * sizeof(*isection));
    s += ih.objects * sizeof(*isection);
    if (soimport)
    {
        set_le32(p1 + s,cimports);
        set_le32(p1 + s + 4,dllstrings);
        s += 8;
    }
    if (sorelocs)
    {
        set_le32(p1 + s,crelocs);
        p1[s + 4] = (unsigned char) (big_relocs & 6);
        s += 5;
    }
    if (soresources)
    {
        set_le16(p1 + s,icondir_count);
        s += 2;
    }
    // end of extra data
    set_le32(p1 + s,ptr_diff(p1,ibuf) - rvamin);
    s += 4;
    ph.u_len += s;
    obuf.allocForCompression(ph.u_len);

    // prepare packheader
    ph.u_len -= rvamin;
    // prepare filter
    Filter ft(ph.level);
    ft.buf_len = ih.codesize;
    ft.addvalue = ih.codebase - rvamin;
    // compress
    int strategy = allow_filter ? 0 : -3;
    compressWithFilters(&ft, 2048, strategy,
                        NULL, 0, 0, ih.codebase, rvamin);

    newvsize = (ph.u_len + rvamin + ph.overlap_overhead + oam1) &~ oam1;
    if (tlsindex && ((newvsize - ph.c_len - 1024 + oam1) &~ oam1) > tlsindex + 4)
        tlsindex = 0;

    const unsigned lsize = getLoaderSize();
    MemBuffer loader(lsize);
    memcpy(loader,getLoader(),lsize);
    patchPackHeader(loader, lsize);

    int identsize = 0;
    const unsigned codesize = getLoaderSection("IDENTSTR",&identsize);
    assert(identsize > 0);
    getLoaderSection("UPX1HEAD",(int*)&ic);
    identsize += ic;

    pe_section_t osection[3];
    // section 0 : bss
    //         1 : [ident + header] + packed_data + unpacker + tls
    //         2 : not compressed data

    // section 2 should start with the resource data, because lots of lame
    // windoze codes assume that resources starts on the beginning of a section

    // identsplit - number of ident + (upx header) bytes to put into the PE header
    int identsplit = pe_offset + sizeof(osection) + sizeof(oh);
    if ((identsplit & 0x1ff) == 0)
        identsplit = 0;
    else if (((identsplit + identsize) ^ identsplit) < 0x200)
        identsplit = identsize;
    else
        identsplit = ALIGN_GAP(identsplit, 0x200);
    ic = identsize - identsplit;

    const unsigned clen = ((ph.c_len + ic) & 15) == 0 ? ph.c_len : ph.c_len + 16 - ((ph.c_len + ic) & 15);
    obuf.clear(ph.c_len, clen - ph.c_len);

    const unsigned s1size = ALIGN_UP(ic + clen + codesize,4) + sotls;
    const unsigned s1addr = (newvsize - (ic + clen) + oam1) &~ oam1;

    const unsigned ncsection = (s1addr + s1size + oam1) &~ oam1;
    const unsigned upxsection = s1addr + ic + clen;
    const unsigned myimport = ncsection + soresources - rvamin;

    // patch loader
    if (ih.entry)
    {
        unsigned jmp_pos = find_le32(loader,codesize + 4,get_le32("JMPO"));
        patch_le32(loader,codesize + 4,"JMPO",ih.entry - upxsection - jmp_pos - 4);
    }
    if (big_relocs & 6)
        patch_le32(loader,codesize,"DELT", 0u -ih.imagebase - rvamin);
    if (sorelocs && (soimport == 0 || soimport + cimports != crelocs))
        patch_le32(loader,codesize,"BREL",crelocs);
    if (soimport)
    {
        if (!isdll)
            patch_le32(loader,codesize,"EXIT",myimport + get_le32(oimpdlls + 16) + 8);
        patch_le32(loader,codesize,"GETP",myimport + get_le32(oimpdlls + 16) + 4);
        if (kernel32ordinal)
            patch_le32(loader,codesize,"K32O",myimport);
        patch_le32(loader,codesize,"LOAD",myimport + get_le32(oimpdlls + 16));
        patch_le32(loader,codesize,"IMPS",myimport);
        patch_le32(loader,codesize,"BIMP",cimports);
    }

    if (patchFilter32(loader, codesize, &ft))
    {
        const unsigned texv = ih.codebase - rvamin;
        if (texv)
            patch_le32(loader, codesize, "TEXV", texv);
    }
    if (tlsindex)
    {
        // in case of overlapping decompression, this hack is needed,
        // because windoze zeroes the word pointed by tlsindex before
        // it starts programs
        if (tlsindex + 4 > s1addr)
            patch_le32(loader,codesize,"TLSV",get_le32(obuf + tlsindex - s1addr - ic));
        else
            patch_le32(loader,codesize,"TLSV",0); // bad guess
        patch_le32(loader,codesize,"TLSA",tlsindex - rvamin);
    }
    if (icondir_count > 1)
    {
        if (icondir_count > 2)
            patch_le16(loader,codesize,"DR",icondir_count - 1);
        patch_le32(loader,codesize,"ICON",ncsection + icondir_offset - rvamin);
    }

    const unsigned esi0 = s1addr + ic;
    patch_le32(loader,codesize,"EDI0", rvamin - esi0);
    patch_le32(loader,codesize,"ESI0",esi0  + ih.imagebase);
    ic = getLoaderSection("PEMAIN01") + 2 + upxsection;

    Reloc rel(1024); // new relocations are put here
    rel.add(ic,3);

    // new PE header
    memcpy(&oh,&ih,sizeof(oh));
    oh.filealign = 0x200; // identsplit depends on this
    memset(osection,0,sizeof(osection));

    oh.entry = upxsection;
    oh.objects = 3;
    oh.chksum = 0;

    // fill the data directory
    ODADDR(PEDIR_DEBUG) = 0;
    ODSIZE(PEDIR_DEBUG) = 0;
    ODADDR(PEDIR_IAT) = 0;
    ODSIZE(PEDIR_IAT) = 0;
    ODADDR(PEDIR_BOUNDIM) = 0;
    ODSIZE(PEDIR_BOUNDIM) = 0;

    // tls is put into section 1

    ic = s1addr + s1size - sotls;
    processTls(&rel,&tlsiv,ic);
    ODADDR(PEDIR_TLS) = sotls ? ic : 0;
    ODSIZE(PEDIR_TLS) = sotls ? 0x18 : 0;
    ic += sotls;

    // these are put into section 2

    ic = ncsection;
    if (soresources)
        processResources(&res,ic);
    ODADDR(PEDIR_RESOURCE) = soresources ? ic : 0;
    ODSIZE(PEDIR_RESOURCE) = soresources;
    ic += soresources;
    processImports(ic);
    ODADDR(PEDIR_IMPORT) = ic;
    ODSIZE(PEDIR_IMPORT) = soimpdlls;
    ic += soimpdlls;
    processExports(&xport,ic);
    ODADDR(PEDIR_EXPORT) = soexport ? ic : 0;
    ODSIZE(PEDIR_EXPORT) = soexport;
    if (!isdll && opt->w32pe.compress_exports)
    {
        ODADDR(PEDIR_EXPORT) = IDADDR(PEDIR_EXPORT);
        ODSIZE(PEDIR_EXPORT) = IDSIZE(PEDIR_EXPORT);
    }
    ic += soexport;
    processRelocs(&rel);
    ODADDR(PEDIR_RELOC) = soxrelocs ? ic : 0;
    ODSIZE(PEDIR_RELOC) = soxrelocs;
    ic += soxrelocs;

    // this is computed here, because soxrelocs changes some lines above
    const unsigned ncsize = soresources + soimpdlls + soexport + soxrelocs;
    ic = oh.filealign - 1;

    // this one is tricky: it seems windoze touches 4 bytes after
    // the end of the relocation data - so we have to increase
    // the virtual size of this section
    const unsigned ncsize_virt_increase = (ncsize & oam1) == 0 ? 8 : 0;

    // fill the sections
    strcpy(osection[0].name,"UPX0");
    strcpy(osection[1].name,"UPX1");
    // after some windoze debugging I found that the name of the sections
    // DOES matter :( .rsrc is used by oleaut32.dll (TYPELIBS)
    // and because of this lame dll, the resource stuff must be the
    // first in the 3rd section - the author of this dll seems to be
    // too idiot to use the data directories... M$ suxx 4 ever!
    // ... even worse: exploder.exe in NiceTry also depends on this to
    // locate version info

    strcpy(osection[2].name,soresources ? ".rsrc" : "UPX2");

    osection[0].vaddr = rvamin;
    osection[1].vaddr = s1addr;
    osection[2].vaddr = ncsection;

    osection[0].size = 0;
    osection[1].size = (s1size + ic) &~ ic;
    osection[2].size = (ncsize + ic) &~ ic;

    osection[0].vsize = osection[1].vaddr - osection[0].vaddr;
    osection[1].vsize = (osection[1].size + oam1) &~ oam1;
    osection[2].vsize = (osection[2].size + ncsize_virt_increase + oam1) &~ oam1;

    osection[0].rawdataptr = (pe_offset + sizeof(oh) + sizeof(osection) + ic) &~ ic;
    osection[1].rawdataptr = osection[0].rawdataptr;
    osection[2].rawdataptr = osection[1].rawdataptr + osection[1].size;

    osection[0].flags = (unsigned) (PEFL_BSS|PEFL_EXEC|PEFL_WRITE|PEFL_READ);
    osection[1].flags = (unsigned) (PEFL_DATA|PEFL_EXEC|PEFL_WRITE|PEFL_READ);
    osection[2].flags = (unsigned) (PEFL_DATA|PEFL_WRITE|PEFL_READ);

    oh.imagesize = osection[2].vaddr + osection[2].vsize;
    oh.bsssize  = osection[0].vsize;
    oh.datasize = osection[2].vsize;
    oh.database = osection[2].vaddr;
    oh.codesize = osection[1].vsize;
    oh.codebase = osection[1].vaddr;
    // oh.headersize = osection[0].rawdataptr;
    oh.headersize = rvamin;

    if (opt->w32pe.strip_relocs && !isdll)
        oh.flags |= RELOCS_STRIPPED;

    //for (ic = 0; ic < oh.filealign; ic += 4)
    //    set_le32(ibuf + ic,get_le32("UPX "));
    ibuf.clear(0, oh.filealign);

    infoHeader("[Writing compressed file]");

    // write loader + compressed file
    fo->write(&oh,sizeof(oh));
    fo->write(osection,sizeof(osection));
    // some alignment
    if (identsplit == identsize)
    {
        unsigned n = osection[0].rawdataptr - fo->getBytesWritten() - identsize;
        assert(n <= oh.filealign);
        fo->write(ibuf, n);
    }
    fo->write(loader + codesize,identsize);
    infoWriting("loader", fo->getBytesWritten());
    fo->write(obuf,clen);
    infoWriting("compressed data", clen);
    fo->write(loader,codesize);
    if ((ic = fo->getBytesWritten() & 3) != 0)
        fo->write(ibuf,4 - ic);
    fo->write(otls,sotls);
    if ((ic = fo->getBytesWritten() & (oh.filealign-1)) != 0)
        fo->write(ibuf,oh.filealign - ic);
    fo->write(oresources,soresources);
    fo->write(oimpdlls,soimpdlls);
    fo->write(oexport,soexport);
    fo->write(oxrelocs,soxrelocs);

    if ((ic = fo->getBytesWritten() & (oh.filealign-1)) != 0)
        fo->write(ibuf,oh.filealign - ic);

#if 0
    printf("%-13s: program hdr  : %8ld bytes\n", getName(), (long) sizeof(oh));
    printf("%-13s: sections     : %8ld bytes\n", getName(), (long) sizeof(osection));
    printf("%-13s: ident        : %8ld bytes\n", getName(), (long) identsize);
    printf("%-13s: compressed   : %8ld bytes\n", getName(), (long) clen);
    printf("%-13s: decompressor : %8ld bytes\n", getName(), (long) codesize);
    printf("%-13s: tls          : %8ld bytes\n", getName(), (long) sotls);
    printf("%-13s: resources    : %8ld bytes\n", getName(), (long) soresources);
    printf("%-13s: imports      : %8ld bytes\n", getName(), (long) soimpdlls);
    printf("%-13s: exports      : %8ld bytes\n", getName(), (long) soexport);
    printf("%-13s: relocs       : %8ld bytes\n", getName(), (long) soxrelocs);
#endif

    // verify
    verifyOverlappingDecompression();

    // copy the overlay
    copyOverlay(fo, overlay, &obuf);

    // finally check the compression ratio
    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();
}


/*************************************************************************
// unpack
**************************************************************************/

int PackW32Pe::canUnpack()
{
    if (!readFileHeader())
        return false;

    unsigned objs = ih.objects;
    isection = new pe_section_t[objs];
    fi->seek(pe_offset+sizeof(ih),SEEK_SET);
    fi->readx(isection,sizeof(pe_section_t)*objs);
    if (ih.objects < 3)
        return -1;
    bool is_packed = (ih.objects == 3 &&
                      (IDSIZE(15) || ih.entry > isection[1].vaddr));
    bool found_ph = false;
    if (memcmp(isection[0].name,"UPX",3) == 0)
    {
        // current version
        fi->seek(isection[1].rawdataptr - 64, SEEK_SET);
        found_ph = readPackHeader(1024);
        if (!found_ph)
        {
            // old versions
            fi->seek(isection[2].rawdataptr, SEEK_SET);
            found_ph = readPackHeader(1024);
        }
    }
    if (is_packed && found_ph)
        return true;
    if (!is_packed && !found_ph)
        return -1;
    if (is_packed && ih.entry < isection[2].vaddr)
    {
        unsigned char buf[256];
        memset(buf, 0, sizeof(buf));
        bool x = false;

        try {
            fi->seek(ih.entry - isection[1].vaddr + isection[1].rawdataptr, SEEK_SET);
            fi->read(buf, sizeof(buf));

            static const char magic[] = "\x8b\x1e\x83\xee\xfc\x11\xdb";
            // mov ebx, [esi];    sub esi, -4;    adc ebx,ebx

            int offset = find(buf, sizeof(buf), magic, 7);
            if (offset >= 0 && find(buf + offset + 1, sizeof(buf) - offset - 1, magic, 7) >= 0)
                x = true;
        } catch (...) {
            //x = true;
        }
        if (x)
            throwCantUnpack("file is modified/hacked/protected; take care!!!");
        else
            throwCantUnpack("file is possibly modified/hacked/protected; take care!");
        return false;   // not reached
    }

    // FIXME: what should we say here ?
    //throwCantUnpack("file is possibly modified/hacked/protected; take care!");
    return false;
}


void PackW32Pe::rebuildImports(upx_byte *& extrainfo)
{
    if (ODADDR(PEDIR_IMPORT) == 0)
        return;

    const upx_byte * const idata = obuf + get_le32(extrainfo);
    const unsigned inamespos = get_le32(extrainfo + 4);
    extrainfo += 8;

    unsigned sdllnames = 0;

    const upx_byte *import = ibuf + IDADDR(PEDIR_IMPORT) - isection[2].vaddr;
    const upx_byte *p = idata;

    while (get_le32(p) != 0)
    {
        sdllnames += strlen(get_le32(p) + import) + 1;
        for (p += 8; *p;)
            if (*p == 1)
                p += strlen(++p) + 1;
            else if (*p == 0xff)
                p += 3; // ordinal
            else
                p += 5;

        p++;
    }
    sdllnames = ALIGN_UP(sdllnames,2);

    upx_byte * const Obuf = obuf - rvamin;
    import_desc * const im0 = (import_desc*) (Obuf + ODADDR(PEDIR_IMPORT));
    import_desc *im = im0;
    upx_byte *dllnames = Obuf + inamespos;
    upx_byte *importednames = dllnames + sdllnames;

    for (p = idata; get_le32(p) != 0; p++)
    {
        // restore the name of the dll
        const unsigned iatoffs = get_le32(p + 4) + rvamin;
        if (inamespos)
        {
            // now I rebuild the dll names
            im->dllname = ptr_diff(dllnames,Obuf);
            strcpy(dllnames,get_le32(p) + import);
            //;;;printf("\ndll: %s:",dllnames);
            dllnames += strlen(dllnames) + 1;
        }
        else
            strcpy(Obuf + im->dllname,get_le32(p) + import);
        im->iat = iatoffs;
        LE32 *newiat = (LE32 *) (Obuf + iatoffs);

        // restore the imported names+ordinals
        for (p += 8; *p; newiat++)
            if (*p == 1)
            {
                unsigned len = strlen(++p) + 1;
                if (inamespos)
                {
                    if (ptr_diff(importednames,oimpdlls) & 1)
                        importednames--;
                    memcpy(importednames + 2,p,len);
                    //;;;printf(" %s",importednames+2);
                    *newiat = ptr_diff(importednames,Obuf);
                    importednames += 2 + len;
                }
                else
                    strcpy(Obuf + *newiat + 2,p);
                p += len;
            }
            else if (*p == 0xff)
            {
                *newiat = get_le16(p + 1) + 0x80000000;
                //;;;printf(" %x",(unsigned)*newiat);
                p += 3;
            }
            else
            {
                *newiat = get_le32(get_le32(p + 1) + import);
                assert(*newiat & 0x80000000);
                p += 5;
            }
        *newiat = 0;
        im++;
    }
    //memset(idata,0,p - idata);
}

void PackW32Pe::rebuildRelocs(upx_byte *& extrainfo)
{
    if (!ODADDR(PEDIR_RELOC) || !ODSIZE(PEDIR_RELOC) || (oh.flags & RELOCS_STRIPPED))
        return;

    if (ODSIZE(PEDIR_RELOC) == 8) // some tricky dlls use this
    {
        memcpy(obuf + ODADDR(PEDIR_RELOC) - rvamin, "\x0\x0\x0\x0\x8\x0\x0\x0", 8);
        return;
    }

    upx_byte *rdata = obuf + get_le32(extrainfo);
    const upx_byte big = extrainfo[4];
    extrainfo += 5;

    upx_byte *p = rdata;
    MemBuffer wrkmem;
    unsigned relocn = unoptimizeReloc32(&rdata,obuf,&wrkmem,1);
    unsigned r16 = 0;
    if (big & 6)                // 16 bit relocations
    {
        LE32 *q = (LE32*) rdata;
        while (*q++)
            r16++;
        if ((big & 6) == 6)
            while (*++q)
                r16++;
    }
    Reloc rel(relocn + r16);

    if (big & 6)
    {
        LE32 *q = (LE32*) rdata;
        while (*q)
            rel.add(*q++ + rvamin,(big & 4) ? 2 : 1);
        if ((big & 6) == 6)
            while (*++q)
                rel.add(*q + rvamin,1);
        rdata = (upx_byte*) q;
    }

    //memset(p,0,rdata - p);

    for (unsigned ic = 0; ic < relocn; ic++)
    {
        p = obuf + get_le32(wrkmem + 4 * ic);
        set_le32(p,get_le32(p) + oh.imagebase + rvamin);
        rel.add(rvamin + get_le32(wrkmem + 4 * ic),3);
    }
    rel.finish (oxrelocs,soxrelocs);

    if (opt->w32pe.strip_relocs && !isdll)
    {
        obuf.clear(ODADDR(PEDIR_RELOC) - rvamin, ODSIZE(PEDIR_RELOC));
        ODADDR(PEDIR_RELOC) = 0;
        soxrelocs = 0;
        // FIXME: try to remove the original relocation section somehow
    }
    else
        memcpy (obuf + ODADDR(PEDIR_RELOC) - rvamin,oxrelocs,soxrelocs);
    delete [] oxrelocs; oxrelocs = NULL;
    wrkmem.dealloc();

    ODSIZE(PEDIR_RELOC) = soxrelocs;
}

void PackW32Pe::rebuildExports()
{
    if (ODSIZE(PEDIR_EXPORT) == 0 || ODADDR(PEDIR_EXPORT) == IDADDR(PEDIR_EXPORT))
        return; // nothing to do

    opt->w32pe.compress_exports = 0;
    Export xport((char*)(unsigned char*) ibuf - isection[2].vaddr);
    processExports(&xport);
    processExports(&xport,ODADDR(PEDIR_EXPORT));
    memcpy(obuf + ODADDR(PEDIR_EXPORT) - rvamin,oexport,soexport);
}

void PackW32Pe::rebuildTls()
{
    // this is an easy one : just do nothing ;-)
}

void PackW32Pe::rebuildResources(upx_byte *& extrainfo)
{
    if (ODSIZE(PEDIR_RESOURCE) == 0)
        return;

    icondir_count = get_le16(extrainfo);
    extrainfo += 2;

    const unsigned vaddr = IDADDR(PEDIR_RESOURCE);
    const upx_byte *r = ibuf - isection[2].vaddr;
    Resource res(r + vaddr);
    while (res.next())
        if (res.offs() > vaddr)
        {
            unsigned origoffs = get_le32(r + res.offs() - 4);
            res.newoffs() = origoffs;
            memcpy(obuf + origoffs - rvamin,r + res.offs(),res.size());
            if (icondir_count && res.itype() == RT_GROUP_ICON)
            {
                set_le16(obuf + origoffs - rvamin + 4,icondir_count);
                icondir_count = 0;
            }
        }
    upx_byte *p = res.build();
    // write back when the original is zeroed
    if (get_le32(obuf + ODADDR(PEDIR_RESOURCE) - rvamin + 12) == 0)
        memcpy(obuf + ODADDR(PEDIR_RESOURCE) - rvamin,p,res.dirsize());
    delete [] p;
}

void PackW32Pe::unpack(OutputFile *fo)
{
    //infoHeader("[Processing %s, format %s, %d sections]", fn_basename(fi->getName()), getName(), objs);

    handleStub(fi,fo,pe_offset);

    const unsigned overlay = file_size - ALIGN_UP(isection[2].rawdataptr + isection[2].size,ih.filealign);
    checkOverlay(overlay);

    ibuf.alloc(ph.c_len);
    obuf.allocForUncompression(ph.u_len);
    fi->seek(isection[1].rawdataptr - 64 + ph.buf_offset + ph.getPackHeaderSize(),SEEK_SET);
    fi->readx(ibuf,ph.c_len);

    // decompress
    decompress(ibuf,obuf);
    upx_byte *extrainfo = obuf + get_le32(obuf + ph.u_len - 4);
    //upx_byte * const eistart = extrainfo;

    memcpy(&oh, extrainfo, sizeof (oh));
    extrainfo += sizeof (oh);
    unsigned objs = oh.objects;

    Array(pe_section_t, osection, objs);
    memcpy(osection,extrainfo,sizeof(pe_section_t) * objs);
    rvamin = osection[0].vaddr;
    extrainfo += sizeof(pe_section_t) * objs;

    // read the noncompressed section
    ibuf.dealloc();
    ibuf.alloc(isection[2].size);
    fi->seek(isection[2].rawdataptr,SEEK_SET);
    fi->readx(ibuf,isection[2].size);

    // unfilter
    if (ph.filter)
    {
        Filter ft(ph.level);
        ft.init(ph.filter,oh.codebase - rvamin);
        ft.cto = (unsigned char) ph.filter_cto;
        ft.unfilter(obuf + oh.codebase - rvamin, oh.codesize);
    }

    rebuildImports(extrainfo);
    rebuildRelocs(extrainfo);
    rebuildTls();
    rebuildExports();
    rebuildResources(extrainfo);

    //FIXME: this does bad things if the relocation section got removed
    // during compression ...
    //memset(eistart,0,extrainfo - eistart + 4);

    // fill the data directory
    ODADDR(PEDIR_DEBUG) = 0;
    ODSIZE(PEDIR_DEBUG) = 0;
    ODADDR(PEDIR_IAT) = 0;
    ODSIZE(PEDIR_IAT) = 0;
    ODADDR(PEDIR_BOUNDIM) = 0;
    ODSIZE(PEDIR_BOUNDIM) = 0;

    // oh.headersize = osection[0].rawdataptr;
    // oh.headersize = ALIGN_UP(pe_offset + sizeof(oh) + sizeof(pe_section_t) * objs, oh.filealign);
    oh.headersize = rvamin;
    oh.chksum = 0;

    // FIXME: ih.flags is checked here because of a bug in UPX 0.92
    if ((opt->w32pe.strip_relocs && !isdll) || (ih.flags & RELOCS_STRIPPED))
    {
        oh.flags |= RELOCS_STRIPPED;
        ODADDR(PEDIR_RELOC) = 0;
        ODSIZE(PEDIR_RELOC) = 0;
    }

    // write decompressed file
    if (fo)
    {
        ibuf.dealloc();
        ibuf.alloc(osection[0].rawdataptr);
        ibuf.clear();
        infoHeader("[Writing uncompressed file]");

        // write loader + compressed file
        fo->write(&oh,sizeof(oh));
        fo->write(osection,objs * sizeof(pe_section_t));
        fo->write(ibuf,osection[0].rawdataptr - fo->getBytesWritten());
        for(unsigned ic = 0; ic < objs; ic++)
            if (osection[ic].rawdataptr)
                fo->write(obuf + osection[ic].vaddr - rvamin,ALIGN_UP(osection[ic].size,oh.filealign));
        copyOverlay(fo, overlay, &obuf);
    }
    ibuf.dealloc();
}

/*
 extra info added to help uncompression:

 <ih sizeof(pe_head)>
 <pe_section_t objs*sizeof(pe_section_t)>
 <start of compressed imports 4> - optional           \
 <start of the names from uncompressed imports> - opt /
 <start of compressed relocs 4> - optional   \
 <relocation type indicator 1> - optional    /
 <icondir_count 2> - optional
 <offset of extra info 4>
*/


/*
vi:ts=4:et
*/

