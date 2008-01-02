/* pefile.cpp --

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


#include "conf.h"
#include "file.h"
#include "filter.h"
#include "packer.h"
#include "pefile.h"

#define IDSIZE(x)       ih.ddirs[x].size
#define IDADDR(x)       ih.ddirs[x].vaddr
#define ODSIZE(x)       oh.ddirs[x].size
#define ODADDR(x)       oh.ddirs[x].vaddr

#define isdll           ((ih.flags & DLL_FLAG) != 0)

#define FILLVAL         0

/*************************************************************************
//
**************************************************************************/

#if defined(__BORLANDC__)
#  undef strcpy
#  define strcpy(a,b)   strcpy((char *)(a),(const char *)(b))
#endif

#if (__ACC_CXX_HAVE_PLACEMENT_DELETE) || defined(__DJGPP__)
#include "bptr.h"
#define IPTR(type, var)         BoundedPtr<type> var(ibuf, ibuf.getSize())
#define OPTR(type, var)         BoundedPtr<type> var(obuf, obuf.getSize())
#define IPTR_I(type, var, v)    BoundedPtr<type> var(ibuf, ibuf.getSize(), v)
#define OPTR_I(type, var, v)    BoundedPtr<type> var(obuf, obuf.getSize(), v)
#define IPTR_C(type, var, v)    const BoundedPtr<type> var(ibuf, ibuf.getSize(), v)
#define OPTR_C(type, var, v)    const BoundedPtr<type> var(obuf, obuf.getSize(), v)
#else
#define IPTR(type, var)         type* var = 0
#define OPTR(type, var)         type* var = 0
#define IPTR_I(type, var, v)    type* var = (v)
#define OPTR_I(type, var, v)    type* var = (v)
#define IPTR_C(type, var, v)    type* const var = (v)
#define OPTR_C(type, var, v)    type* const var = (v)
#endif

static void xcheck(const void *p, size_t plen, const void *b, size_t blen)
{
    const char *pp = (const char *) p;
    const char *bb = (const char *) b;
    if (pp < bb || pp > bb + blen || pp + plen > bb + blen)
        throwCantUnpack("pointer out of range; take care!");
}
#if 0
static void xcheck(size_t poff, size_t plen, const void *b, size_t blen)
{
    ACC_UNUSED(b);
    if (poff > blen || poff + plen > blen)
        throwCantUnpack("pointer out of range; take care!");
}
#endif
#define ICHECK(x, size)     xcheck(x, size, ibuf, ibuf.getSize())
#define OCHECK(x, size)     xcheck(x, size, obuf, obuf.getSize())

#define imemset(a,b,c)      ICHECK(a,c), memset(a,b,c)
#define omemset(a,b,c)      OCHECK(a,c), memset(a,b,c)
#define imemcpy(a,b,c)      ICHECK(a,c), memcpy(a,b,c)
#define omemcpy(a,b,c)      OCHECK(a,c), memcpy(a,b,c)


/*************************************************************************
//
**************************************************************************/

PeFile::PeFile(InputFile *f) : super(f)
{
    bele = &N_BELE_RTP::le_policy;
    //printf("pe_header_t %d\n", (int) sizeof(pe_header_t));
    //printf("pe_section_t %d\n", (int) sizeof(pe_section_t));
    COMPILE_TIME_ASSERT(sizeof(pe_header_t) == 248)
    COMPILE_TIME_ASSERT(sizeof(pe_header_t::ddirs_t) == 8)
    COMPILE_TIME_ASSERT(sizeof(pe_section_t) == 40)
    COMPILE_TIME_ASSERT_ALIGNED1(pe_header_t)
    COMPILE_TIME_ASSERT_ALIGNED1(pe_header_t::ddirs_t)
    COMPILE_TIME_ASSERT_ALIGNED1(pe_section_t)
    COMPILE_TIME_ASSERT(RT_LAST == TABLESIZE(opt->win32_pe.compress_rt));

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
    sorelocs = 0;
    soxrelocs = 0;
    sotls = 0;
}


PeFile::~PeFile()
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


bool PeFile::testUnpackVersion(int version) const
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

int PeFile::readFileHeader()
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
    return getFormat();
}


/*************************************************************************
// interval handling
**************************************************************************/

PeFile::Interval::Interval(void *b) : capacity(0),base(b),ivarr(NULL),ivnum(0)
{}

PeFile::Interval::~Interval()
{
    free(ivarr);
}

void PeFile::Interval::add(const void *start,unsigned len)
{
    add(ptr_diff(start,base),len);
}

void PeFile::Interval::add(const void *start,const void *end)
{
    add(ptr_diff(start,base),ptr_diff(end,start));
}

int __acc_cdecl_qsort PeFile::Interval::compare(const void *p1,const void *p2)
{
    const interval *i1 = (const interval*) p1;
    const interval *i2 = (const interval*) p2;
    if (i1->start < i2->start) return -1;
    if (i1->start > i2->start) return 1;
    if (i1->len < i2->len) return 1;
    if (i1->len > i2->len) return -1;
    return 0;
}

void PeFile::Interval::add(unsigned start,unsigned len)
{
    if (ivnum == capacity)
        ivarr = (interval*) realloc(ivarr,(capacity += 15) * sizeof (interval));
    ivarr[ivnum].start = start;
    ivarr[ivnum++].len = len;
}

void PeFile::Interval::add(const Interval *iv)
{
    for (unsigned ic = 0; ic < iv->ivnum; ic++)
        add(iv->ivarr[ic].start,iv->ivarr[ic].len);
}

void PeFile::Interval::flatten()
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

void PeFile::Interval::clear()
{
    for (unsigned ic = 0; ic < ivnum; ic++)
        memset((char*) base + ivarr[ic].start,0,ivarr[ic].len);
}

void PeFile::Interval::dump() const
{
    printf("%d intervals:\n",ivnum);
    for (unsigned ic = 0; ic < ivnum; ic++)
        printf("%x %x\n",ivarr[ic].start,ivarr[ic].len);
}


/*************************************************************************
// relocation handling
**************************************************************************/

struct PeFile::Reloc::reloc
{
    LE32  pagestart;
    LE32  size;
}
__attribute_packed;

void PeFile::Reloc::newRelocPos(void *p)
{
    rel = (reloc*) p;
    rel1 = (LE16*) ((char*) p + sizeof (reloc));
}

PeFile::Reloc::Reloc(upx_byte *s,unsigned si) :
    start(s), size(si), rel(NULL), rel1(NULL)
{
    COMPILE_TIME_ASSERT(sizeof(reloc) == 8);
    memset(counts,0,sizeof(counts));
    unsigned pos,type;
    while (next(pos,type))
        counts[type]++;
}

PeFile::Reloc::Reloc(unsigned rnum) :
    start(NULL), size(0), rel(NULL), rel1(NULL)
{
    start = new upx_byte[rnum * 4 + 8192];
    counts[0] = 0;
}

bool PeFile::Reloc::next(unsigned &pos,unsigned &type)
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

void PeFile::Reloc::add(unsigned pos,unsigned type)
{
    set_le32(start + 1024 + 4 * counts[0]++,(pos << 4) + type);
}

void PeFile::Reloc::finish(upx_byte *&p,unsigned &siz)
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

void PeFile::processRelocs(Reloc *rel) // pass2
{
    rel->finish(oxrelocs,soxrelocs);
    if (opt->win32_pe.strip_relocs && !isdll)
        soxrelocs = 0;
}

void PeFile::processRelocs() // pass1
{
    big_relocs = 0;

    Reloc rel(ibuf + IDADDR(PEDIR_RELOC),IDSIZE(PEDIR_RELOC));
    const unsigned *counts = rel.getcounts();
    const unsigned rnum = counts[1] + counts[2] + counts[3];

    if ((opt->win32_pe.strip_relocs && !isdll) || rnum == 0)
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

    unsigned xcounts[4];
    memset(xcounts, 0, sizeof(xcounts));

    // prepare sorting
    unsigned pos,type;
    while (rel.next(pos,type))
    {
        if (pos >= ih.imagesize)
            continue;           // skip out-of-bounds record
        if (type < 4)
            fix[type][xcounts[type]++] = pos - rvamin;
    }

    // remove duplicated records
    for (ic = 1; ic <= 3; ic++)
    {
        qsort(fix[ic], xcounts[ic], 4, le32_compare);
        unsigned prev = ~0;
        unsigned jc = 0;
        for (unsigned kc = 0; kc < xcounts[ic]; kc++)
            if (fix[ic][kc] != prev)
                prev = fix[ic][jc++] = fix[ic][kc];

        //printf("xcounts[%u] %u->%u\n", ic, xcounts[ic], jc);
        xcounts[ic] = jc;
    }

    // preprocess "type 3" relocation records
    for (ic = 0; ic < xcounts[3]; ic++)
    {
        pos = fix[3][ic] + rvamin;
        set_le32(ibuf + pos, get_le32(ibuf + pos) - ih.imagebase - rvamin);
    }

    ibuf.fill(IDADDR(PEDIR_RELOC), IDSIZE(PEDIR_RELOC), FILLVAL);
    orelocs = new upx_byte [rnum * 4 + 1024];  // 1024 - safety
    sorelocs = ptr_diff(optimizeReloc32((upx_byte*) fix[3], xcounts[3],
                                        orelocs, ibuf + rvamin,1, &big_relocs),
                        orelocs);
    delete [] fix[3];

    // append relocs type "LOW" then "HIGH"
    for (ic = 2; ic ; ic--)
    {
        memcpy(orelocs + sorelocs,fix[ic],4 * xcounts[ic]);
        sorelocs += 4 * xcounts[ic];
        delete [] fix[ic];

        set_le32(orelocs + sorelocs,0);
        if (xcounts[ic])
        {
            sorelocs += 4;
            big_relocs |= 2 * ic;
        }
    }
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

void PeFile::processImports(unsigned myimport, unsigned iat_off) // pass 2
{
    COMPILE_TIME_ASSERT(sizeof(import_desc) == 20);

    // adjust import data
    for (import_desc *im = (import_desc*) oimpdlls; im->dllname; im++)
    {
        if (im->dllname < myimport)
            im->dllname += myimport;
        LE32 *p = (LE32*) (oimpdlls + im->iat);
        im->iat += myimport;
        im->oft = im->iat;

        while (*p)
            if ((*p++ & 0x80000000) == 0)  // import by name?
                p[-1] += myimport;

        im->iat = im == (import_desc*) oimpdlls ? iat_off : iat_off + 12;
    }
}

unsigned PeFile::processImports() // pass 1
{
    static const unsigned char kernel32dll[] = "COREDLL.dll";
    static const char llgpa[] = "\x0\x0""LoadLibraryW\x0\x0"
                                "GetProcAddressA\x0\x0\x0"
                                "CacheSync";
    //static const char exitp[] = "ExitProcess\x0\x0\x0";

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
    };

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
                //if (dlls[ic].isk32)
                //    kernel32ordinal = true,k32o++;
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
    LE32 *lookuptable = ordinals + 4;// + k32o + (isdll ? 0 : 1);
    upx_byte *dllnames = ((upx_byte*) lookuptable) + (dllnum2 - 1) * 8;
    upx_byte *importednames = dllnames + (dllnamelen &~ 1);

    unsigned k32namepos = ptr_diff(dllnames,oimpdlls);

    memcpy(importednames, llgpa, ALIGN_UP((unsigned) sizeof(llgpa), 2u));
    strcpy(dllnames,kernel32dll);
    im->dllname = k32namepos;
    im->iat = ptr_diff(ordinals,oimpdlls);
    *ordinals++ = ptr_diff(importednames,oimpdlls);             // LoadLibraryW
    *ordinals++ = ptr_diff(importednames,oimpdlls) + 14;        // GetProcAddressA
    *ordinals++ = ptr_diff(importednames,oimpdlls) + 14 + 18;   // CacheSync
    dllnames += sizeof(kernel32dll);
    importednames += sizeof(llgpa);

    im++;
    for (ic = 0; ic < dllnum; ic++)
        if (idlls[ic]->isk32)
        {
            idlls[ic]->npos = k32namepos;
            /*
            if (idlls[ic]->ordinal)
                for (LE32 *tarr = idlls[ic]->lookupt; *tarr; tarr++)
                    if (*tarr & 0x80000000)
                        *ordinals++ = *tarr;
            */
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
    //ordinals -= k32o;
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
                /*if (idlls[ic]->isk32)
                {
                    *ppi++ = 0xfe; // signed + odd parity
                    set_le32(ppi,ptr_diff(ordinals,oimpdlls));
                    ordinals++;
                    ppi += 4;
                }
                else*/
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

        unsigned esize = ptr_diff((char *)tarr, (char *)idlls[ic]->lookupt);
        lookups.add(idlls[ic]->lookupt,esize);
        if (ptr_diff(ibuf + idlls[ic]->iat, (char *)idlls[ic]->lookupt))
        {
            memcpy(ibuf + idlls[ic]->iat, idlls[ic]->lookupt, esize);
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
    //OutputFile::dump("x1.imp", oimpdlls, soimpdlls);

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

PeFile::Export::Export(char *_base) : base(_base), iv(_base)
{
    COMPILE_TIME_ASSERT(sizeof(export_dir_t) == 40);
    ename = functionptrs = ordinals = NULL;
    names = NULL;
    memset(&edir,0,sizeof(edir));
    size = 0;
}

PeFile::Export::~Export()
{
    free(ename);
    delete [] functionptrs;
    delete [] ordinals;
    for (unsigned ic = 0; ic < edir.names + edir.functions; ic++)
        free(names[ic]);
    delete [] names;
}

void PeFile::Export::convert(unsigned eoffs,unsigned esize)
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
            names[ic + edir.names] = NULL;

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

void PeFile::Export::build(char *newbase, unsigned newoffs)
{
    char * const functionp = newbase + sizeof(edir);
    char * const namep = functionp + 4 * edir.functions;
    char * const ordinalp = namep + 4 * edir.names;
    char * const enamep = ordinalp + 2 * edir.names;
    char * exports = enamep + strlen(ename) + 1;

    edir.addrtable = newoffs + ptr_diff(functionp, newbase);
    edir.ordinaltable = newoffs + ptr_diff(ordinalp, newbase);
    memcpy(ordinalp,ordinals,2 * edir.names);

    edir.name = newoffs + ptr_diff(enamep, newbase);
    strcpy(enamep,ename);
    edir.nameptrtable = newoffs + ptr_diff(namep, newbase);
    unsigned ic;
    for (ic = 0; ic < edir.names; ic++)
    {
        strcpy(exports,names[ic]);
        set_le32(namep + 4 * ic,newoffs + ptr_diff(exports, newbase));
        exports += strlen(exports) + 1;
    }

    memcpy(functionp,functionptrs,4 * edir.functions);
    for (ic = 0; ic < edir.functions; ic++)
        if (names[edir.names + ic])
        {
            strcpy(exports,names[edir.names + ic]);
            set_le32(functionp + 4 * ic,newoffs + ptr_diff(exports, newbase));
            exports += strlen(exports) + 1;
        }

    memcpy(newbase,&edir,sizeof(edir));
    assert(exports - newbase == (int) size);
}

void PeFile::processExports(Export *xport) // pass1
{
    soexport = ALIGN_UP(IDSIZE(PEDIR_EXPORT),4);
    if (soexport == 0)
        return;
    if (!isdll && opt->win32_pe.compress_exports)
    {
        infoWarning("exports compressed, --compress-exports=0 might be needed");
        soexport = 0;
        return;
    }
    xport->convert(IDADDR(PEDIR_EXPORT),IDSIZE(PEDIR_EXPORT));
    soexport = ALIGN_UP(xport->getsize(), 4u);
    oexport = new upx_byte[soexport];
    memset(oexport, 0, soexport);
}

void PeFile::processExports(Export *xport,unsigned newoffs) // pass2
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

void PeFile::processTls(Interval *iv) // pass 1
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

    // makes sure tls index is zero after decompression
    if (tlsindex && tlsindex < ih.imagesize)
        set_le32(ibuf + tlsindex, 0);
}

void PeFile::processTls(Reloc *rel,const Interval *iv,unsigned newaddr) // pass 2
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

struct PeFile::Resource::res_dir_entry
{
    LE32  tnl; // Type | Name | Language id - depending on level
    LE32  child;
}
__attribute_packed;

struct PeFile::Resource::res_dir
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

struct PeFile::Resource::res_data
{
    LE32  offset;
    LE32  size;
    char  _[8]; // codepage, reserved
}
__attribute_packed;

struct PeFile::Resource::upx_rnode
{
    unsigned        id;
    upx_byte        *name;
    upx_rnode       *parent;
};

struct PeFile::Resource::upx_rbranch : public upx_rnode
{
    unsigned        nc;
    upx_rnode       **children;
    res_dir         data;
};

struct PeFile::Resource::upx_rleaf : public upx_rnode
{
    upx_rleaf       *next;
    unsigned        newoffset;
    res_data        data;
};

PeFile::Resource::Resource() : root(NULL)
{}

PeFile::Resource::Resource(const upx_byte *p)
{
    init(p);

}

PeFile::Resource::~Resource()
{
    if (root) destroy (root,0);
}

unsigned PeFile::Resource::dirsize() const
{
    return ALIGN_UP(dsize + ssize, 4u);
}

bool PeFile::Resource::next()
{
    // wow, builtin autorewind... :-)
    return (current = current ? current->next : head) != NULL;
}

unsigned PeFile::Resource::itype() const
{
    return current->parent->parent->id;
}

const upx_byte *PeFile::Resource::ntype() const
{
    return current->parent->parent->name;
}

unsigned PeFile::Resource::size() const
{
    return ALIGN_UP(current->data.size,4);
}

unsigned PeFile::Resource::offs() const
{
    return current->data.offset;
}

unsigned &PeFile::Resource::newoffs()
{
    return current->newoffset;
}

void PeFile::Resource::dump() const
{
    dump(root,0);
}

unsigned PeFile::Resource::iname() const
{
    return current->parent->id;
}

const upx_byte *PeFile::Resource::nname() const
{
    return current->parent->name;
}

/*
    unsigned ilang() const {return current->id;}
    const upx_byte *nlang() const {return current->name;}
*/

void PeFile::Resource::init(const upx_byte *res)
{
    COMPILE_TIME_ASSERT(sizeof(res_dir_entry) == 8)
    COMPILE_TIME_ASSERT(sizeof(res_dir) == 16 + 8)
    COMPILE_TIME_ASSERT(sizeof(res_data) == 16)
    COMPILE_TIME_ASSERT_ALIGNED1(res_dir_entry)
    COMPILE_TIME_ASSERT_ALIGNED1(res_dir)
    COMPILE_TIME_ASSERT_ALIGNED1(res_data)

    start = res;
    root = head = current = NULL;
    dsize = ssize = 0;
    check((const res_dir*) start,0);
    root = convert(start,NULL,0);
}

void PeFile::Resource::check(const res_dir *node,unsigned level)
{
    int ic = node->identr + node->namedentr;
    if (ic == 0)
        return;
    for (const res_dir_entry *rde = node->entries; --ic >= 0; rde++)
        if (((rde->child & 0x80000000) == 0) ^ (level == 2))
            throwCantPack("unsupported resource structure");
        else if (level != 2)
            check((const res_dir*) (start + (rde->child & 0x7fffffff)),level + 1);
}

PeFile::Resource::upx_rnode *PeFile::Resource::convert(const void *rnode,
                                                       upx_rnode *parent,
                                                       unsigned level)
{
    if (level == 3)
    {
        const res_data *node = (const res_data *) rnode;
        upx_rleaf *leaf = new upx_rleaf;
        leaf->name = NULL;
        leaf->parent = parent;
        leaf->next = head;
        leaf->newoffset = 0;
        leaf->data = *node;

        head = leaf; // append node to a linked list for traversal
        dsize += sizeof(res_data);
        return leaf;
    }

    const res_dir *node = (const res_dir *) rnode;
    int ic = node->identr + node->namedentr;
    if (ic == 0)
        return NULL;

    upx_rbranch *branch = new upx_rbranch;
    branch->name = NULL;
    branch->parent = parent;
    branch->nc = ic;
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

void PeFile::Resource::build(const upx_rnode *node, unsigned &bpos,
                             unsigned &spos, unsigned level)
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

upx_byte *PeFile::Resource::build()
{
    newstart = new upx_byte [dirsize()];
    unsigned bpos = 0,spos = dsize;
    build(root,bpos,spos,0);

    // dirsize() is 4 bytes aligned, so we may need to zero
    // up to 2 bytes to make valgrind happy
    while (spos < dirsize())
        newstart[spos++] = 0;

    return newstart;
}

void PeFile::Resource::destroy(upx_rnode *node,unsigned level)
{
    delete [] node->name; node->name = NULL;
    if (level != 3)
    {
        upx_rbranch * const branch = (upx_rbranch *) node;
        for (int ic = branch->nc; --ic >= 0; )
            destroy(branch->children[ic],level + 1);
        delete [] branch->children; branch->children = NULL;
    }
    delete node;
}

static void lame_print_unicode(const upx_byte *p)
{
    for (unsigned ic = 0; ic < get_le16(p); ic++)
        printf("%c",(char)p[ic * 2 + 2]);
}

void PeFile::Resource::dump(const upx_rnode *node,unsigned level) const
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

void PeFile::Resource::clear(upx_byte *node,unsigned level,Interval *iv)
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

bool PeFile::Resource::clear()
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

void PeFile::processResources(Resource *res,unsigned newaddr)
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

static bool match(unsigned itype, const unsigned char *ntype,
                  unsigned iname, const unsigned char *nname,
                  const char *keep)
{
    // format of string keep: type1[/name1],type2[/name2], ....
    // typex and namex can be string or number
    // hopefully resource names do not have '/' or ',' characters inside

    struct helper
    {
        static bool match(unsigned num, const unsigned char *unistr,
                          const char *mkeep)
        {
            if (!unistr)
                return (unsigned) atoi(mkeep) == num;

            unsigned ic;
            for (ic = 0; ic < get_le16(unistr); ic++)
                if (unistr[2 + ic * 2] != (unsigned char) mkeep[ic])
                    return false;
            return mkeep[ic] == 0 || mkeep[ic] == ',' || mkeep[ic] == '/';
        }
    };

    // FIXME this comparison is not too exact
    while (1)
    {
        char *delim1 = strchr(keep, '/');
        char *delim2 = strchr(keep, ',');
        if (helper::match(itype, ntype, keep))
        {
            if (!delim1)
                return true;
            if (delim2 && delim2 < delim1)
                return true;
            if (helper::match(iname, nname, delim1 + 1))
                return true;
        }

        if (delim2 == NULL)
            break;
        keep = delim2 + 1;
    }
    return false;
}

void PeFile::processResources(Resource *res)
{
    const unsigned vaddr = IDADDR(PEDIR_RESOURCE);
    if ((soresources = IDSIZE(PEDIR_RESOURCE)) == 0)
        return;

    // setup default options for resource compression
    if (opt->win32_pe.compress_resources < 0)
        opt->win32_pe.compress_resources = true;
    if (!opt->win32_pe.compress_resources)
    {
        opt->win32_pe.compress_icons = false;
        for (int i = 0; i < RT_LAST; i++)
            opt->win32_pe.compress_rt[i] = false;
    }
    if (opt->win32_pe.compress_rt[RT_STRING] < 0)
    {
        // by default, don't compress RT_STRINGs of screensavers (".scr")
        opt->win32_pe.compress_rt[RT_STRING] = true;
        if (fn_has_ext(fi->getName(),"scr"))
            opt->win32_pe.compress_rt[RT_STRING] = false;
    }

    res->init(ibuf + vaddr);

    for (soresources = res->dirsize(); res->next(); soresources += 4 + res->size())
        ;
    oresources = new upx_byte[soresources];
    upx_byte *ores = oresources + res->dirsize();

    char *keep_icons = NULL; // icon ids in the first icon group
    unsigned iconsin1stdir = 0;
    if (opt->win32_pe.compress_icons == 2)
        while (res->next()) // there is no rewind() in Resource
            if (res->itype() == RT_GROUP_ICON && iconsin1stdir == 0)
            {
                iconsin1stdir = get_le16(ibuf + res->offs() + 4);
                keep_icons = new char[1 + iconsin1stdir * 9];
                *keep_icons = 0;
                for (unsigned ic = 0; ic < iconsin1stdir; ic++)
                    snprintf(keep_icons + strlen(keep_icons), 9, "3/%u,",
                             get_le16(ibuf + res->offs() + 6 + ic * 14 + 12));
                if (*keep_icons)
                    keep_icons[strlen(keep_icons) - 1] = 0;
            }

    // the icon id which should not be compressed when compress_icons == 1
    unsigned first_icon_id = (unsigned) -1;
    if (opt->win32_pe.compress_icons == 1)
        while (res->next())
            if (res->itype() == RT_GROUP_ICON && first_icon_id == (unsigned) -1)
                first_icon_id = get_le16(ibuf + res->offs() + 6 + 12);

    bool compress_icon = opt->win32_pe.compress_icons > 1;
    bool compress_idir = opt->win32_pe.compress_icons == 3;

    // some statistics
    unsigned usize = 0;
    unsigned csize = 0;
    unsigned unum = 0;
    unsigned cnum = 0;

    while (res->next())
    {
        const unsigned rtype = res->itype();
        bool do_compress = true;
        if (!opt->win32_pe.compress_resources)
            do_compress = false;
        else if (rtype == RT_ICON)          // icon
        {
            if (opt->win32_pe.compress_icons == 0)
                do_compress = false;
            else if (opt->win32_pe.compress_icons == 1)
                if ((first_icon_id == (unsigned) -1
                     || first_icon_id == res->iname()))
                    do_compress = compress_icon;
        }
        else if (rtype == RT_GROUP_ICON)    // icon directory
            do_compress = compress_idir && opt->win32_pe.compress_icons;
        else if (rtype > 0 && rtype < RT_LAST)
            do_compress = opt->win32_pe.compress_rt[rtype] ? true : false;

        if (keep_icons)
            do_compress &= !match(res->itype(), res->ntype(), res->iname(),
                                  res->nname(), keep_icons);
        do_compress &= !match(res->itype(), res->ntype(), res->iname(),
                              res->nname(), "TYPELIB,REGISTRY,16");
        do_compress &= !match(res->itype(), res->ntype(), res->iname(),
                              res->nname(), opt->win32_pe.keep_resource);

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
        ICHECK(ibuf + res->offs(), res->size());
        memcpy(ores, ibuf + res->offs(), res->size());
        ibuf.fill(res->offs(), res->size(), FILLVAL);
        res->newoffs() = ptr_diff(ores,oresources);
        if (rtype == RT_ICON && opt->win32_pe.compress_icons == 1)
            compress_icon = true;
        else if (rtype == RT_GROUP_ICON)
        {
            if (opt->win32_pe.compress_icons == 1)
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

    delete[] keep_icons;
    if (!res->clear())
    {
        // The area occupied by the resource directory is not continuous
        // so to still support uncompression, I can't zero this area.
        // This decreases compression ratio, so FIXME somehow.
        infoWarning("can't remove unneeded resource directory");
    }
    info("Resources: compressed %u (%u bytes), not compressed %u (%u bytes)",cnum,csize,unum,usize);
}


unsigned PeFile::virta2objnum(unsigned addr,pe_section_t *sect,unsigned objs)
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


unsigned PeFile::tryremove (unsigned vaddr,unsigned objs)
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


unsigned PeFile::stripDebug(unsigned overlaystart)
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


/*************************************************************************
// unpack
**************************************************************************/

void PeFile::rebuildRelocs(upx_byte *& extrainfo)
{
    if (!ODADDR(PEDIR_RELOC) || !ODSIZE(PEDIR_RELOC) || (oh.flags & RELOCS_STRIPPED))
        return;

    if (ODSIZE(PEDIR_RELOC) == 8) // some tricky dlls use this
    {
        omemcpy(obuf + ODADDR(PEDIR_RELOC) - rvamin, "\x0\x0\x0\x0\x8\x0\x0\x0", 8);
        return;
    }

    upx_byte *rdata = obuf + get_le32(extrainfo);
    const upx_byte big = extrainfo[4];
    extrainfo += 5;

//    upx_byte *p = rdata;
    OPTR_I(upx_byte, p, rdata);
    MemBuffer wrkmem;
    unsigned relocn = unoptimizeReloc32(&rdata,obuf,&wrkmem,1);
    unsigned r16 = 0;
    if (big & 6)                // 16 bit relocations
    {
        const LE32 *q = (LE32*) rdata;
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
        set_le32(p, get_le32((unsigned char *)p) + oh.imagebase + rvamin);
        rel.add(rvamin + get_le32(wrkmem + 4 * ic),3);
    }
    rel.finish (oxrelocs,soxrelocs);

    if (opt->win32_pe.strip_relocs && !isdll)
    {
        obuf.clear(ODADDR(PEDIR_RELOC) - rvamin, ODSIZE(PEDIR_RELOC));
        ODADDR(PEDIR_RELOC) = 0;
        soxrelocs = 0;
        // FIXME: try to remove the original relocation section somehow
    }
    else
        omemcpy(obuf + ODADDR(PEDIR_RELOC) - rvamin,oxrelocs,soxrelocs);
    delete [] oxrelocs; oxrelocs = NULL;
    wrkmem.dealloc();

    ODSIZE(PEDIR_RELOC) = soxrelocs;
}

void PeFile::rebuildExports()
{
    if (ODSIZE(PEDIR_EXPORT) == 0 || ODADDR(PEDIR_EXPORT) == IDADDR(PEDIR_EXPORT))
        return; // nothing to do

    opt->win32_pe.compress_exports = 0;
    Export xport((char*)(unsigned char*) ibuf - isection[2].vaddr);
    processExports(&xport);
    processExports(&xport,ODADDR(PEDIR_EXPORT));
    omemcpy(obuf + ODADDR(PEDIR_EXPORT) - rvamin,oexport,soexport);
}

void PeFile::rebuildTls()
{
    // this is an easy one : just do nothing ;-)
}

void PeFile::rebuildResources(upx_byte *& extrainfo)
{
    if (ODSIZE(PEDIR_RESOURCE) == 0 || IDSIZE(PEDIR_RESOURCE) == 0)
        return;

    icondir_count = get_le16(extrainfo);
    extrainfo += 2;

    const unsigned vaddr = IDADDR(PEDIR_RESOURCE);
    const upx_byte *r = ibuf - isection[ih.objects - 1].vaddr;
    Resource res(r + vaddr);
    while (res.next())
        if (res.offs() > vaddr)
        {
            unsigned origoffs = get_le32(r + res.offs() - 4);
            res.newoffs() = origoffs;
            omemcpy(obuf + origoffs - rvamin,r + res.offs(),res.size());
            if (icondir_count && res.itype() == RT_GROUP_ICON)
            {
                set_le16(obuf + origoffs - rvamin + 4,icondir_count);
                icondir_count = 0;
            }
        }
    upx_byte *p = res.build();
    // write back when the original is zeroed
    if (get_le32(obuf + ODADDR(PEDIR_RESOURCE) - rvamin + 12) == 0)
        omemcpy(obuf + ODADDR(PEDIR_RESOURCE) - rvamin,p,res.dirsize());
    delete [] p;
}

void PeFile::unpack(OutputFile *fo)
{
    //infoHeader("[Processing %s, format %s, %d sections]", fn_basename(fi->getName()), getName(), objs);

    handleStub(fi,fo,pe_offset);

    const unsigned iobjs = ih.objects;
    const unsigned overlay = file_size - ALIGN_UP(isection[iobjs - 1].rawdataptr
                                                  + isection[iobjs - 1].size,
                                                  ih.filealign);
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

    if (iobjs == 4)
    {
        // read the resource section if present
        ibuf.dealloc();
        ibuf.alloc(isection[3].size);
        fi->seek(isection[3].rawdataptr,SEEK_SET);
        fi->readx(ibuf,isection[3].size);
    }

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
    if ((opt->win32_pe.strip_relocs && !isdll) || (ih.flags & RELOCS_STRIPPED))
    {
        oh.flags |= RELOCS_STRIPPED;
        ODADDR(PEDIR_RELOC) = 0;
        ODSIZE(PEDIR_RELOC) = 0;
    }

    // write decompressed file
    if (fo)
    {
        unsigned ic;
        for (ic = 0; ic < objs && osection[ic].rawdataptr == 0; ic++)
            ;

        ibuf.dealloc();
        ibuf.alloc(osection[ic].rawdataptr);
        ibuf.clear();
        infoHeader("[Writing uncompressed file]");

        // write loader + compressed file
        fo->write(&oh,sizeof(oh));
        fo->write(osection,objs * sizeof(pe_section_t));
        fo->write(ibuf,osection[ic].rawdataptr - fo->getBytesWritten());
        for (ic = 0; ic < objs; ic++)
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

