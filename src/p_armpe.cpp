/* p_armpe.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2013 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2013 Laszlo Molnar
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

#if 0
static void xcheck(const void *p, size_t plen, const void *b, size_t blen)
{
    const char *pp = (const char *) p;
    const char *bb = (const char *) b;
    if (pp < bb || pp > bb + blen || pp + plen > bb + blen)
        throwCantUnpack("pointer out of range; take care!");
}

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

void PackArmPe::processImports(unsigned myimport, unsigned iat_off) // pass 2
{
    PeFile::processImports(myimport, iat_off);

__packed_struct(import_desc)
    LE32  oft;      // orig first thunk
    char  _[8];
    LE32  dllname;
    LE32  iat;      // import address table
__packed_struct_end()

    COMPILE_TIME_ASSERT(sizeof(import_desc) == 20);

    // adjust import data
    for (import_desc *im = (import_desc*) oimpdlls; im->dllname; im++)
    {
        im->oft = im->iat;
        im->iat = im == (import_desc*) oimpdlls ? iat_off : iat_off + 12;
    }
}

void PackArmPe::addKernelImports()
{
    addKernelImport("COREDLL.DLL", "LoadLibraryW");
    addKernelImport("COREDLL.DLL", "GetProcAddressA");
    addKernelImport("COREDLL.DLL", "CacheSync");
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

bool PackArmPe::handleForceOption()
{
    return (ih.cpu != 0x1c0 && ih.cpu != 0x1c2)
        || (ih.opthdrsize != 0xe0)
        || ((ih.flags & EXECUTABLE) == 0)
        || (ih.entry == 0 /*&& !isdll*/)
        || (ih.ddirsentries != 16)
//        || IDSIZE(PEDIR_EXCEPTION) // is this used on arm?
//        || IDSIZE(PEDIR_COPYRIGHT)
        ;
}

void PackArmPe::callCompressWithFilters(Filter &ft, int filter_strategy, unsigned ih_codebase)
{
    // limit stack size needed for runtime decompression
    upx_compress_config_t cconf; cconf.reset();
    cconf.conf_lzma.max_num_probs = 1846 + (768 << 4); // ushort: ~28 KiB stack
    compressWithFilters(&ft, 2048, &cconf, filter_strategy,
                        ih_codebase, rvamin, 0, NULL, 0);
}

void PackArmPe::addNewRelocations(Reloc &rel, unsigned upxsection)
{
    static const char* symbols_to_relocate[] = {
        "ONAM", "BIMP", "BREL", "FIBE", "FIBS", "ENTR", "DST0", "SRC0"
    };
    for (unsigned s2r = 0; s2r < TABLESIZE(symbols_to_relocate); s2r++)
    {
        unsigned off = linker->getSymbolOffset(symbols_to_relocate[s2r]);
        if (off != 0xdeaddead)
            rel.add(off + upxsection, 3);
    }
}

unsigned PackArmPe::getProcessImportParam(unsigned upxsection)
{
    return linker->getSymbolOffset("IATT") + upxsection;
}

void PackArmPe::defineSymbols(unsigned ncsection, unsigned, unsigned,
                              unsigned ic, Reloc &, unsigned s1addr)
{
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
    linker->defineSymbol("start_of_compressed", ih.imagebase + s1addr + ic);
    defineDecompressorSymbols();
}

void PackArmPe::setOhDataBase(const pe_section_t *osection)
{
    oh.database = osection[2].vaddr;
}

void PackArmPe::setOhHeaderSize(const pe_section_t *osection)
{
    oh.headersize = osection[1].rawdataptr;
}

void PackArmPe::pack(OutputFile *fo)
{
    super::pack0(fo, 1U << 9, 0x10000, true);
#if 0
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
                throwCantPack("writable shared sections not supported (try --force)");
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

    const unsigned dllstrings = PeFile32::processImports();
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
//    super::processTls(&rel,&tlsiv,ic);
    ODADDR(PEDIR_TLS) = sotls ? ic : 0;
    ODSIZE(PEDIR_TLS) = sotls ? 0x18 : 0;
    ic += sotls;

    // these are put into section 2

    ic = ncsection;

    // wince wants relocation data at the beginning of a section
    PeFile::processRelocs(&rel);
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
#endif
}

/*
vi:ts=4:et
*/

