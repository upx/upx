/* p_armpe.cpp --

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
#include "p_armpe.h"
#include "linker.h"

static const
#include "stub/arm.v4a-wince.pe.h"
static const
#include "stub/arm.v4t-wince.pe.h"

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

PackArmPe::PackArmPe(InputFile *f) : super(f)
{
    use_thumb_stub = false;
}


PackArmPe::~PackArmPe()
{
}


const int *PackArmPe::getCompressionMethods(int method, int level) const
{
    static const int m_all[]   = { M_NRV2B_8, M_NRV2E_8, M_LZMA, M_END };
    static const int m_lzma[]  = { M_LZMA, M_END };
    static const int m_nrv2b[] = { M_NRV2B_8, M_END };
    static const int m_nrv2e[] = { M_NRV2E_8, M_END };

    if (!use_thumb_stub)
        return getDefaultCompressionMethods_8(method, level);

    if (method == M_ALL)    return m_all;
    if (M_IS_LZMA(method))  return m_lzma;
    if (M_IS_NRV2B(method)) return m_nrv2b;
    if (M_IS_NRV2E(method)) return m_nrv2e;
    return m_nrv2e;
}


const int *PackArmPe::getFilters() const
{
    static const int filters[] = { 0x50, FT_END };
    return filters;
}


Linker* PackArmPe::newLinker() const
{
    return new ElfLinkerArmLE;
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

void PackArmPe::processImports(unsigned myimport, unsigned iat_off) // pass 2
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

unsigned PackArmPe::processImports() // pass 1
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


void PackArmPe::processTls(Interval *) // pass 1
{
    if ((sotls = ALIGN_UP(IDSIZE(PEDIR_TLS),4u)) == 0)
        return;

    // never should happen on wince
    throwCantPack("Static TLS entries found. Send a report please.");
}


/*************************************************************************
// pack
**************************************************************************/

bool PackArmPe::canPack()
{
    if (!readFileHeader() || (ih.cpu != 0x1c0 && ih.cpu != 0x1c2))
        return false;
    use_thumb_stub |= ih.cpu == 0x1c2 || (ih.entry & 1) == 1;
    use_thumb_stub |= (opt->cpu == opt->CPU_8086); // FIXME
    return true;
}


void PackArmPe::buildLoader(const Filter *ft)
{
    const unsigned char *loader = use_thumb_stub ? stub_arm_v4t_wince_pe : stub_arm_v4a_wince_pe;
    unsigned size = use_thumb_stub ? sizeof(stub_arm_v4t_wince_pe) : sizeof(stub_arm_v4a_wince_pe);

    // prepare loader
    initLoader(loader, size);

    if (isdll)
        addLoader("DllStart", NULL);
    addLoader("ExeStart", NULL);

    if (ph.method == M_NRV2E_8)
        addLoader("Call2E", NULL);
    else if (ph.method == M_NRV2B_8)
        addLoader("Call2B", NULL);
    else if (ph.method == M_NRV2D_8)
        addLoader("Call2D", NULL);
    else if (M_IS_LZMA(ph.method))
        addLoader("+40C,CallLZMA", NULL);


    if (ft->id == 0x50)
        addLoader("+40C,Unfilter_0x50", NULL);

    if (sorelocs)
        addLoader("+40C,Relocs", NULL);

    addLoader("+40C,Imports", NULL);
    addLoader("ProcessEnd", NULL);

    if (ph.method == M_NRV2E_8)
        addLoader(".ucl_nrv2e_decompress_8", NULL);
    else if (ph.method == M_NRV2B_8)
        addLoader(".ucl_nrv2b_decompress_8", NULL);
    else if (ph.method == M_NRV2D_8)
        addLoader(".ucl_nrv2d_decompress_8", NULL);
    else if (M_IS_LZMA(ph.method))
        addLoader("+40C,LZMA_DECODE,LZMA_DEC10", NULL);

    addLoader("IDENTSTR,UPX1HEAD", NULL);
}


void PackArmPe::pack(OutputFile *fo)
{
    // FIXME: we need to think about better support for --exact
    if (opt->exact)
        throwCantPackExact();

    const unsigned objs = ih.objects;
    isection = new pe_section_t[objs];
    fi->seek(pe_offset+sizeof(ih),SEEK_SET);
    fi->readx(isection,sizeof(pe_section_t)*objs);

    rvamin = isection[0].vaddr;

    infoHeader("[Processing %s, format %s, %d sections]", fn_basename(fi->getName()), getName(), objs);

    // check the PE header
    // FIXME: add more checks
    if (!opt->force && (
           (ih.cpu != 0x1c0 && ih.cpu != 0x1c2)
        || (ih.opthdrsize != 0xe0)
        || ((ih.flags & EXECUTABLE) == 0)
        || (ih.subsystem != 9)
        || (ih.entry == 0 /*&& !isdll*/)
        || (ih.ddirsentries != 16)
//        || IDSIZE(PEDIR_EXCEPTION) // is this used on arm?
//        || IDSIZE(PEDIR_COPYRIGHT)
       ))
        throwCantPack("unexpected value in PE header (try --force)");

    if (IDSIZE(PEDIR_SEC))
        IDSIZE(PEDIR_SEC) = IDADDR(PEDIR_SEC) = 0;
    //    throwCantPack("compressing certificate info is not supported");

    if (IDSIZE(PEDIR_COMRT))
        throwCantPack(".NET files (win32/net) are not yet supported");

    if (isdll)
        opt->win32_pe.strip_relocs = false;
    else if (opt->win32_pe.strip_relocs < 0)
        opt->win32_pe.strip_relocs = (ih.imagebase >= 0x10000);
    if (opt->win32_pe.strip_relocs)
    {
        if (ih.imagebase < 0x10000)
            throwCantPack("--strip-relocs is not allowed when imagebase < 0x10000");
        else
            ih.flags |= RELOCS_STRIPPED;
    }

    if (memcmp(isection[0].name,"UPX",3) == 0)
        throwAlreadyPackedByUPX();
    if (!opt->force && IDSIZE(15))
        throwCantPack("file is possibly packed/protected (try --force)");
    if (ih.entry && ih.entry < rvamin)
        throwCantPack("run a virus scanner on this file!");
    if (!opt->force && ih.subsystem == 1)
        throwCantPack("subsystem 'native' is not supported (try --force)");
    if (ih.filealign < 0x200)
        throwCantPack("filealign < 0x200 is not yet supported");

    handleStub(fi,fo,pe_offset);
    const unsigned usize = ih.imagesize;
    const unsigned xtrasize = UPX_MAX(ih.datasize, 65536u) + IDSIZE(PEDIR_IMPORT) + IDSIZE(PEDIR_BOUNDIM) + IDSIZE(PEDIR_IAT) + IDSIZE(PEDIR_DELAYIMP) + IDSIZE(PEDIR_RELOC);
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
        if (((isection[ic].flags & (PEFL_WRITE|PEFL_SHARED))
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
        if (isection[ic].vaddr + jc > ibuf.getSize())
            throwInternalError("buffer too small 1");
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

    // some checks for broken linkers - disable filter if necessary
    bool allow_filter = true;
    if (ih.codebase == ih.database
        || ih.codebase + ih.codesize > ih.imagesize
        || (isection[virta2objnum(ih.codebase,isection,objs)].flags & PEFL_CODE) == 0)
        allow_filter = false;

    const unsigned oam1 = ih.objectalign - 1;

    // FIXME: disabled: the uncompressor would not allocate enough memory
    //objs = tryremove(IDADDR(PEDIR_RELOC),objs);

    // FIXME: if the last object has a bss then this won't work
    // newvsize = (isection[objs-1].vaddr + isection[objs-1].size + oam1) &~ oam1;
    // temporary solution:
    unsigned newvsize = (isection[objs-1].vaddr + isection[objs-1].vsize + oam1) &~ oam1;

    //fprintf(stderr,"newvsize=%x objs=%d\n",newvsize,objs);
    if (newvsize + soimport + sorelocs > ibuf.getSize())
         throwInternalError("buffer too small 2");
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
    int filter_strategy = allow_filter ? 0 : -3;

    // disable filters for files with broken headers
    if (ih.codebase + ih.codesize > ph.u_len)
    {
        ft.buf_len = 1;
        filter_strategy = -3;
    }

    // limit stack size needed for runtime decompression
    upx_compress_config_t cconf; cconf.reset();
    cconf.conf_lzma.max_num_probs = 1846 + (768 << 4); // ushort: ~28 KiB stack
    compressWithFilters(&ft, 2048, &cconf, filter_strategy,
                        ih.codebase, rvamin, 0, NULL, 0);
// info: see buildLoader()
    newvsize = (ph.u_len + rvamin + ph.overlap_overhead + oam1) &~ oam1;
    /*
    if (tlsindex && ((newvsize - ph.c_len - 1024 + oam1) &~ oam1) > tlsindex + 4)
    tlsindex = 0;
    */

    const unsigned lsize = getLoaderSize();

    int identsize = 0;
    const unsigned codesize = getLoaderSection("IDENTSTR",&identsize);
    assert(identsize > 0);
    getLoaderSection("UPX1HEAD",(int*)&ic);
    identsize += ic;

    pe_section_t osection[4];
    // section 0 : bss
    //         1 : [ident + header] + packed_data + unpacker + tls
    //         2 : not compressed data
    //         3 : resource data -- wince 5 needs a new section for this

    // identsplit - number of ident + (upx header) bytes to put into the PE header
    int identsplit = pe_offset + sizeof(osection) + sizeof(oh);
    if ((identsplit & 0x1ff) == 0)
        identsplit = 0;
    else if (((identsplit + identsize) ^ identsplit) < 0x200)
        identsplit = identsize;
    else
        identsplit = ALIGN_GAP(identsplit, 0x200);
    ic = identsize - identsplit;

    const unsigned c_len = ((ph.c_len + ic) & 15) == 0 ? ph.c_len : ph.c_len + 16 - ((ph.c_len + ic) & 15);
    obuf.clear(ph.c_len, c_len - ph.c_len);

    const unsigned s1size = ALIGN_UP(ic + c_len + codesize,4u) + sotls;
    const unsigned s1addr = (newvsize - (ic + c_len) + oam1) &~ oam1;

    const unsigned ncsection = (s1addr + s1size + oam1) &~ oam1;
    const unsigned upxsection = s1addr + ic + c_len;

    Reloc rel(1024); // new relocations are put here
    static const char* symbols_to_relocate[] = {
        "ONAM", "BIMP", "BREL", "FIBE", "FIBS", "ENTR", "DST0", "SRC0"
    };
    for (unsigned s2r = 0; s2r < TABLESIZE(symbols_to_relocate); s2r++)
    {
        unsigned off = linker->getSymbolOffset(symbols_to_relocate[s2r]);
        if (off != 0xdeaddead)
            rel.add(off + upxsection, 3);
    }

    // new PE header
    memcpy(&oh,&ih,sizeof(oh));
    oh.filealign = 0x200; // identsplit depends on this
    memset(osection,0,sizeof(osection));

    oh.entry = upxsection;
    oh.objects = 4;
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
    super::processTls(&rel,&tlsiv,ic);
    ODADDR(PEDIR_TLS) = sotls ? ic : 0;
    ODSIZE(PEDIR_TLS) = sotls ? 0x18 : 0;
    ic += sotls;

    // these are put into section 2

    ic = ncsection;

    // wince wants relocation data at the beginning of a section
    processRelocs(&rel);
    ODADDR(PEDIR_RELOC) = soxrelocs ? ic : 0;
    ODSIZE(PEDIR_RELOC) = soxrelocs;
    ic += soxrelocs;

    processImports(ic, linker->getSymbolOffset("IATT") + upxsection);
    ODADDR(PEDIR_IMPORT) = ic;
    ODSIZE(PEDIR_IMPORT) = soimpdlls;
    ic += soimpdlls;

    processExports(&xport,ic);
    ODADDR(PEDIR_EXPORT) = soexport ? ic : 0;
    ODSIZE(PEDIR_EXPORT) = soexport;
    if (!isdll && opt->win32_pe.compress_exports)
    {
        ODADDR(PEDIR_EXPORT) = IDADDR(PEDIR_EXPORT);
        ODSIZE(PEDIR_EXPORT) = IDSIZE(PEDIR_EXPORT);
    }
    ic += soexport;

    ic = (ic + oam1) &~ oam1;
    const unsigned res_start = ic;
    if (soresources)
        processResources(&res,ic);
    ODADDR(PEDIR_RESOURCE) = soresources ? ic : 0;
    ODSIZE(PEDIR_RESOURCE) = soresources;
    ic += soresources;

    const unsigned onam = ncsection + soxrelocs + ih.imagebase;
    linker->defineSymbol("start_of_dll_names", onam);
    linker->defineSymbol("start_of_imports", ih.imagebase + rvamin + cimports);
    linker->defineSymbol("start_of_relocs", crelocs + rvamin + ih.imagebase);
    linker->defineSymbol("filter_buffer_end", ih.imagebase + ih.codebase + ih.codesize);
    linker->defineSymbol("filter_buffer_start", ih.imagebase + ih.codebase);
    linker->defineSymbol("original_entry", ih.entry + ih.imagebase);
    linker->defineSymbol("uncompressed_length", ph.u_len);
    linker->defineSymbol("start_of_uncompressed", ih.imagebase + rvamin);
    linker->defineSymbol("compressed_length", ph.c_len);
    linker->defineSymbol("start_of_compressed", ih.imagebase + s1addr + identsize - identsplit);
    defineDecompressorSymbols();
    relocateLoader();

    MemBuffer loader(lsize);
    memcpy(loader, getLoader(), lsize);
    patchPackHeader(loader, lsize);

    // this is computed here, because soxrelocs changes some lines above
    const unsigned ncsize = soxrelocs + soimpdlls + soexport;
    const unsigned fam1 = oh.filealign - 1;

    // fill the sections
    strcpy(osection[0].name,"UPX0");
    strcpy(osection[1].name,"UPX1");
    strcpy(osection[2].name, "UPX2");
    strcpy(osection[3].name, ".rsrc");

    osection[0].vaddr = rvamin;
    osection[1].vaddr = s1addr;
    osection[2].vaddr = ncsection;
    osection[3].vaddr = res_start;

    osection[0].size = 0;
    osection[1].size = (s1size + fam1) &~ fam1;
    osection[2].size = (ncsize + fam1) &~ fam1;
    osection[3].size = (soresources + fam1) &~ fam1;

    osection[0].vsize = osection[1].vaddr - osection[0].vaddr;
    //osection[1].vsize = (osection[1].size + oam1) &~ oam1;
    //osection[2].vsize = (osection[2].size + oam1) &~ oam1;
    osection[1].vsize = osection[1].size;
    osection[2].vsize = osection[2].size;
    osection[3].vsize = osection[3].size;

    osection[0].rawdataptr = 0;
    osection[1].rawdataptr = (pe_offset + sizeof(oh) + sizeof(osection) + fam1) &~ fam1;
    osection[2].rawdataptr = osection[1].rawdataptr + osection[1].size;
    osection[3].rawdataptr = osection[2].rawdataptr + osection[2].size;

    osection[0].flags = (unsigned) (PEFL_BSS|PEFL_EXEC|PEFL_WRITE|PEFL_READ);
    osection[1].flags = (unsigned) (PEFL_DATA|PEFL_EXEC|PEFL_WRITE|PEFL_READ);
    osection[2].flags = (unsigned) (PEFL_DATA|PEFL_READ);
    osection[3].flags = (unsigned) (PEFL_DATA|PEFL_READ);

    oh.imagesize = (osection[3].vaddr + osection[3].vsize + oam1) &~ oam1;
    oh.bsssize  = osection[0].vsize;
    oh.datasize = osection[2].vsize + osection[3].vsize;
    oh.database = osection[2].vaddr;
    oh.codesize = osection[1].vsize;
    oh.codebase = osection[1].vaddr;
    oh.headersize = osection[1].rawdataptr;
    if (rvamin < osection[0].rawdataptr)
        throwCantPack("object alignment too small");

    if (opt->win32_pe.strip_relocs && !isdll)
        oh.flags |= RELOCS_STRIPPED;

    //for (ic = 0; ic < oh.filealign; ic += 4)
    //    set_le32(ibuf + ic,get_le32("UPX "));
    ibuf.clear(0, oh.filealign);

    info("Image size change: %u -> %u KiB",
         ih.imagesize / 1024, oh.imagesize / 1024);

    infoHeader("[Writing compressed file]");

    if (soresources == 0)
    {
        oh.objects = 3;
        memset(&osection[3], 0, sizeof(osection[3]));
    }
    // write loader + compressed file
    fo->write(&oh,sizeof(oh));
    fo->write(osection,sizeof(osection));
    // some alignment
    if (identsplit == identsize)
    {
        unsigned n = osection[1].rawdataptr - fo->getBytesWritten() - identsize;
        assert(n <= oh.filealign);
        fo->write(ibuf, n);
    }
    fo->write(loader + codesize,identsize);
    infoWriting("loader", fo->getBytesWritten());
    fo->write(obuf,c_len);
    infoWriting("compressed data", c_len);
    fo->write(loader,codesize);
    if (opt->debug.dump_stub_loader)
        OutputFile::dump(opt->debug.dump_stub_loader, loader, codesize);
    if ((ic = fo->getBytesWritten() & 3) != 0)
        fo->write(ibuf,4 - ic);
    fo->write(otls,sotls);
    if ((ic = fo->getBytesWritten() & fam1) != 0)
        fo->write(ibuf,oh.filealign - ic);
    fo->write(oxrelocs,soxrelocs);
    fo->write(oimpdlls,soimpdlls);
    fo->write(oexport,soexport);

    if ((ic = fo->getBytesWritten() & fam1) != 0)
        fo->write(ibuf,oh.filealign - ic);

    fo->write(oresources,soresources);
    if ((ic = fo->getBytesWritten() & fam1) != 0)
        fo->write(ibuf,oh.filealign - ic);

#if 0
    printf("%-13s: program hdr  : %8ld bytes\n", getName(), (long) sizeof(oh));
    printf("%-13s: sections     : %8ld bytes\n", getName(), (long) sizeof(osection));
    printf("%-13s: ident        : %8ld bytes\n", getName(), (long) identsize);
    printf("%-13s: compressed   : %8ld bytes\n", getName(), (long) c_len);
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

int PackArmPe::canUnpack()
{
    if (!readFileHeader() || (ih.cpu != 0x1c0 && ih.cpu != 0x1c2))
        return false;

    unsigned objs = ih.objects;
    isection = new pe_section_t[objs];
    fi->seek(pe_offset+sizeof(ih),SEEK_SET);
    fi->readx(isection,sizeof(pe_section_t)*objs);
    if (ih.objects < 3)
        return -1;
    bool is_packed = ((ih.objects == 3 || ih.objects == 4) &&
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
        bool x = false;

        memset(buf, 0, sizeof(buf));
        try {
            fi->seek(ih.entry - isection[1].vaddr + isection[1].rawdataptr, SEEK_SET);
            fi->read(buf, sizeof(buf));

            // FIXME this is for x86
            static const unsigned char magic[] = "\x8b\x1e\x83\xee\xfc\x11\xdb";
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


void PackArmPe::rebuildImports(upx_byte *& extrainfo)
{
    if (ODADDR(PEDIR_IMPORT) == 0
        || ODSIZE(PEDIR_IMPORT) <= sizeof(import_desc))
        return;

//    const upx_byte * const idata = obuf + get_le32(extrainfo);
    OPTR_C(const upx_byte, idata, obuf + get_le32(extrainfo));
    const unsigned inamespos = get_le32(extrainfo + 4);
    extrainfo += 8;

    unsigned sdllnames = 0;

//    const upx_byte *import = ibuf + IDADDR(PEDIR_IMPORT) - isection[2].vaddr;
//    const upx_byte *p;
    IPTR_I(const upx_byte, import, ibuf + IDADDR(PEDIR_IMPORT) - isection[2].vaddr);
    OPTR(const upx_byte, p);

    for (p = idata; get_le32(p) != 0; ++p)
    {
        const upx_byte *dname = get_le32(p) + import;
        ICHECK(dname, 1);
        const unsigned dlen = strlen(dname);
        ICHECK(dname, dlen + 1);

        sdllnames += dlen + 1;
        for (p += 8; *p;)
            if (*p == 1)
                p += strlen(++p) + 1;
            else if (*p == 0xff)
                p += 3; // ordinal
            else
                p += 5;
    }
    sdllnames = ALIGN_UP(sdllnames,2u);

    upx_byte * const Obuf = obuf - rvamin;
    import_desc * const im0 = (import_desc*) (Obuf + ODADDR(PEDIR_IMPORT));
    import_desc *im = im0;
    upx_byte *dllnames = Obuf + inamespos;
    upx_byte *importednames = dllnames + sdllnames;
    upx_byte * const importednames_start = importednames;

    for (p = idata; get_le32(p) != 0; ++p)
    {
        // restore the name of the dll
        const upx_byte *dname = get_le32(p) + import;
        ICHECK(dname, 1);
        const unsigned dlen = strlen(dname);
        ICHECK(dname, dlen + 1);

        const unsigned iatoffs = get_le32(p + 4) + rvamin;
        if (inamespos)
        {
            // now I rebuild the dll names
            OCHECK(dllnames, dlen + 1);
            strcpy(dllnames, dname);
            im->dllname = ptr_diff(dllnames,Obuf);
            //;;;printf("\ndll: %s:",dllnames);
            dllnames += dlen + 1;
        }
        else
        {
            OCHECK(Obuf + im->dllname, dlen + 1);
            strcpy(Obuf + im->dllname, dname);
        }
        im->oft = im->iat = iatoffs;

//        LE32 *newiat = (LE32 *) (Obuf + iatoffs);
        OPTR_I(LE32, newiat, (LE32 *) (Obuf + iatoffs));

        // restore the imported names+ordinals
        for (p += 8; *p; ++newiat)
            if (*p == 1)
            {
                const unsigned ilen = strlen(++p) + 1;
                if (inamespos)
                {
                    if (ptr_diff(importednames, importednames_start) & 1)
                        importednames -= 1;
                    omemcpy(importednames + 2, p, ilen);
                    //;;;printf(" %s",importednames+2);
                    *newiat = ptr_diff(importednames, Obuf);
                    importednames += 2 + ilen;
                }
                else
                {
                    OCHECK(Obuf + *newiat + 2, ilen + 1);
                    strcpy(Obuf + *newiat + 2, p);
                }
                p += ilen;
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

/*
vi:ts=4:et
*/

