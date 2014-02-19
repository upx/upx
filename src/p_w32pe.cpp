/* p_w32pe.cpp --

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
#include "p_w32pe.h"
#include "linker.h"

static const
#include "stub/i386-win32.pe.h"

#define FILLVAL         0


/*************************************************************************
//
**************************************************************************/

#if defined(__BORLANDC__)
#  undef strcpy
#  define strcpy(a,b)   strcpy((char *)(a),(const char *)(b))
#endif

#if 0
//static
unsigned my_strlen(const char *s)
{
    size_t l = strlen((const char*)s); assert((unsigned) l == l); return (unsigned) l;
}
static unsigned my_strlen(const unsigned char *s)
{
    size_t l = strlen((const char*)s); assert((unsigned) l == l); return (unsigned) l;
}
#undef strlen
#define strlen my_strlen
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

PackW32Pe::PackW32Pe(InputFile *f) : super(f)
{
    isrtm = false;
    use_dep_hack = true;
    use_clear_dirty_stack = true;
}


PackW32Pe::~PackW32Pe()
{}


const int *PackW32Pe::getCompressionMethods(int method, int level) const
{
    bool small = ih.codesize + ih.datasize <= 256*1024;
    return Packer::getDefaultCompressionMethods_le32(method, level, small);
}


const int *PackW32Pe::getFilters() const
{
    static const int filters[] = {
        0x26, 0x24, 0x49, 0x46, 0x16, 0x13, 0x14, 0x11,
        FT_ULTRA_BRUTE, 0x25, 0x15, 0x12,
    FT_END };
    return filters;
}


Linker* PackW32Pe::newLinker() const
{
    return new ElfLinkerX86;
}


/*************************************************************************
// util
**************************************************************************/

int PackW32Pe::readFileHeader()
{
    char buf[6];
    fi->seek(0x200, SEEK_SET);
    fi->readx(buf, 6);
    isrtm = memcmp(buf, "32STUB" ,6) == 0;
    return super::readFileHeader();
}

/*************************************************************************
// pack
**************************************************************************/

bool PackW32Pe::canPack()
{
    if (!readFileHeader() || ih.cpu < 0x14c || ih.cpu > 0x150)
        return false;
    return true;
}


void PackW32Pe::buildLoader(const Filter *ft)
{
    // recompute tlsindex (see pack() below)
    unsigned tmp_tlsindex = tlsindex;
    const unsigned oam1 = ih.objectalign - 1;
    const unsigned newvsize = (ph.u_len + rvamin + ph.overlap_overhead + oam1) &~ oam1;
    if (tlsindex && ((newvsize - ph.c_len - 1024 + oam1) &~ oam1) > tlsindex + 4)
        tmp_tlsindex = 0;

    // prepare loader
    initLoader(stub_i386_win32_pe, sizeof(stub_i386_win32_pe), 2);
    addLoader(isdll ? "PEISDLL1" : "",
              "PEMAIN01",
              icondir_count > 1 ? (icondir_count == 2 ? "PEICONS1" : "PEICONS2") : "",
              tmp_tlsindex ? "PETLSHAK" : "",
              "PEMAIN02",
              ph.first_offset_found == 1 ? "PEMAIN03" : "",
              getDecompressorSections(),
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
                  "PERELOC3,RELOC320",
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
    if (use_dep_hack)
        addLoader("PEDEPHAK", NULL);

    //NEW: TLS callback support PART 1, the callback handler installation - Stefan Widmann
    if(use_tls_callbacks)
        addLoader("PETLSC", NULL);

    addLoader("PEMAIN20", NULL);
    if (use_clear_dirty_stack)
        addLoader("CLEARSTACK", NULL);
    addLoader("PEMAIN21", NULL);
#if 0
    addLoader(ih.entry ? "PEDOJUMP" : "PERETURN",
              "IDENTSTR,UPX1HEAD",
              NULL
             );
#endif
    //NEW: last loader sections split up to insert TLS callback handler - Stefan Widmann
    addLoader(ih.entry ? "PEDOJUMP" : "PERETURN", NULL);

    //NEW: TLS callback support PART 2, the callback handler - Stefan Widmann
    if(use_tls_callbacks)
        addLoader("PETLSC2", NULL);

    addLoader("IDENTSTR,UPX1HEAD", NULL);

}


void PackW32Pe::pack(OutputFile *fo)
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
    //NEW: subsystem check moved to switch ... case below, check of COFF magic, 32 bit machine flag check - Stefan Widmann
    if (!opt->force && (
           (ih.cpu < 0x14c || ih.cpu > 0x150)
        || (ih.opthdrsize != 0xe0)
        || ((ih.flags & EXECUTABLE) == 0)
        || ((ih.flags & BITS_32_MACHINE) == 0) //NEW: 32 bit machine flag must be set - Stefan Widmann
        || (ih.coffmagic != 0x10B) //COFF magic is 0x10B in PE files, 0x20B in PE32+ files - Stefan Widmann
        || (ih.entry == 0 && !isdll)
        || (ih.ddirsentries != 16)
        || IDSIZE(PEDIR_EXCEPTION) // is this used on i386?
//        || IDSIZE(PEDIR_COPYRIGHT)
       ))
        throwCantPack("unexpected value in PE header (try --force)");

    switch(ih.subsystem)  //let's take a closer look at the subsystem
      {
        case 1: //NATIVE
          {
            if(!opt->force)
              {
                throwCantPack("win32/native applications are not yet supported");
              }
            break;
          }
        case 2: //GUI
          {
            //fine, continue
            break;
          }
        case 3: //CONSOLE
          {
            //fine, continue
            break;
          }
        case 5: //OS2
          {
            throwCantPack("win32/os2 files are not yet supported");
            break;
          }
        case 7: //POSIX - do x32/posix files exist?
          {
            throwCantPack("win32/posix files are not yet supported");
            break;
          }
        case 9: //WINCE x86 files
          {
            throwCantPack("PE32/wince files are not yet supported");
            break;
          }
        case 10: //EFI APPLICATION
          {
            throwCantPack("PE32/EFIapplication files are not yet supported");
            break;
          }
        case 11: //EFI BOOT SERVICE DRIVER
          {
            throwCantPack("PE32/EFIbootservicedriver files are not yet supported");
            break;
          }
        case 12: //EFI RUNTIME DRIVER
          {
            throwCantPack("PE32/EFIruntimedriver files are not yet supported");
            break;
          }
        case 13: //EFI ROM
          {
            throwCantPack("PE32/EFIROM files are not yet supported");
            break;
          }
        case 14: //XBOX - will we ever see a XBOX file?
          {
            throwCantPack("PE32/xbox files are not yet supported");
            break;
          }
        case 16: //WINDOWS BOOT APPLICATION
          {
            throwCantPack("PE32/windowsbootapplication files are not yet supported");
            break;
          }
        default: //UNKNOWN SUBSYSTEM
          {
            throwCantPack("PE32/? unknown subsystem");
            break;
          }
      }

    //remove certificate directory entry
    if (IDSIZE(PEDIR_SEC))
        IDSIZE(PEDIR_SEC) = IDADDR(PEDIR_SEC) = 0;

    //check CLR Runtime Header directory entry
    if (IDSIZE(PEDIR_COMRT))
        throwCantPack(".NET files (win32/.net) are not yet supported");

    //NEW: disable reloc stripping if ASLR is enabled
    if(ih.dllflags & IMAGE_DLL_CHARACTERISTICS_DYNAMIC_BASE)
        opt->win32_pe.strip_relocs = false;
    else if (isdll) //do never strip relocations from DLLs
        opt->win32_pe.strip_relocs = false;
    else if (opt->win32_pe.strip_relocs < 0)
        opt->win32_pe.strip_relocs = (ih.imagebase >= 0x400000);
    if (opt->win32_pe.strip_relocs)
    {
        if (ih.imagebase < 0x400000)
            throwCantPack("--strip-relocs is not allowed when imagebase < 0x400000");
        else
            ih.flags |= RELOCS_STRIPPED;
    }

    if (memcmp(isection[0].name,"UPX",3) == 0)
        throwAlreadyPackedByUPX();
    if (!opt->force && IDSIZE(15))
        throwCantPack("file is possibly packed/protected (try --force)");
    if (ih.entry && ih.entry < rvamin)
        throwCantPack("run a virus scanner on this file!");
#if 0 //subsystem check moved to switch ... case above - Stefan Widmann
        if (!opt->force && ih.subsystem == 1)
        throwCantPack("subsystem 'native' is not supported (try --force)");
#endif
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
        if (!isrtm && ((isection[ic].flags & (PEFL_WRITE|PEFL_SHARED))
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
    Interval loadconfiv(ibuf);
    Export xport((char*)(unsigned char*)ibuf);

    const unsigned dllstrings = processImports();
    processTls(&tlsiv); // call before processRelocs!!
    processLoadConf(&loadconfiv);
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

    compressWithFilters(&ft, 2048, NULL_cconf, filter_strategy,
                        ih.codebase, rvamin, 0, NULL, 0);
// info: see buildLoader()
    newvsize = (ph.u_len + rvamin + ph.overlap_overhead + oam1) &~ oam1;
    if (tlsindex && ((newvsize - ph.c_len - 1024 + oam1) &~ oam1) > tlsindex + 4)
        tlsindex = 0;

    int identsize = 0;
    const unsigned codesize = getLoaderSection("IDENTSTR",&identsize);
    assert(identsize > 0);
    getLoaderSection("UPX1HEAD",(int*)&ic);
    identsize += ic;

    pe_section_t osection[3];
    // section 0 : bss
    //         1 : [ident + header] + packed_data + unpacker + tls + loadconf
    //         2 : not compressed data

    // section 2 should start with the resource data, because lots of lame
    // windoze codes assume that resources starts on the beginning of a section

    // note: there should be no data in section 2 which needs fixup

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

    const unsigned s1size = ALIGN_UP(ic + c_len + codesize,4u) + sotls + soloadconf;
    const unsigned s1addr = (newvsize - (ic + c_len) + oam1) &~ oam1;

    const unsigned ncsection = (s1addr + s1size + oam1) &~ oam1;
    const unsigned upxsection = s1addr + ic + c_len;
    const unsigned myimport = ncsection + soresources - rvamin;

    // patch loader
    linker->defineSymbol("original_entry", ih.entry);
    if (use_dep_hack)
    {
        // This works around a "protection" introduced in MSVCRT80, which
        // works like this:
        // When the compiler detects that it would link in some code from its
        // C runtime library which references some data in a read only
        // section then it compiles in a runtime check whether that data is
        // still in a read only section by looking at the pe header of the
        // file. If this check fails the runtime does "interesting" things
        // like not running the floating point initialization code - the result
        // is a R6002 runtime error.
        // These supposed to be read only addresses are covered by the sections
        // UPX0 & UPX1 in the compressed files, so we have to patch the PE header
        // in the memory. And the page on which the PE header is stored is read
        // only so we must make it rw, fix the flags (i.e. clear
        // PEFL_WRITE of osection[x].flags), and make it ro again.

        // rva of the most significant byte of member "flags" in section "UPX0"
        const unsigned swri = pe_offset + sizeof(oh) + sizeof(pe_section_t) - 1;
        // make sure we only touch the minimum number of pages
        const unsigned addr = 0u - rvamin + swri;
        linker->defineSymbol("swri", addr &  0xfff);    // page offset
        // check whether osection[0].flags and osection[1].flags
        // are on the same page
        linker->defineSymbol("vp_size", ((addr & 0xfff) + 0x28 >= 0x1000) ?
                             0x2000 : 0x1000);          // 2 pages or 1 page
        linker->defineSymbol("vp_base", addr &~ 0xfff); // page mask
        linker->defineSymbol("VirtualProtect", myimport + get_le32(oimpdlls + 16) + 8);
    }
    linker->defineSymbol("reloc_delt", 0u - (unsigned) ih.imagebase - rvamin);
    linker->defineSymbol("start_of_relocs", crelocs);
    linker->defineSymbol("ExitProcess", myimport + get_le32(oimpdlls + 16) + 20);
    linker->defineSymbol("GetProcAddress", myimport + get_le32(oimpdlls + 16) + 4);
    linker->defineSymbol("kernel32_ordinals", myimport);
    linker->defineSymbol("LoadLibraryA", myimport + get_le32(oimpdlls + 16));
    linker->defineSymbol("start_of_imports", myimport);
    linker->defineSymbol("compressed_imports", cimports);
#if 0
    linker->defineSymbol("VirtualAlloc", myimport + get_le32(oimpdlls + 16) + 12);
    linker->defineSymbol("VirtualFree", myimport + get_le32(oimpdlls + 16) + 16);
#endif

    defineDecompressorSymbols();
    defineFilterSymbols(&ft);
    linker->defineSymbol("filter_buffer_start", ih.codebase - rvamin);

    // in case of overlapping decompression, this hack is needed,
    // because windoze zeroes the word pointed by tlsindex before
    // it starts programs
    linker->defineSymbol("tls_value", (tlsindex + 4 > s1addr) ?
                         get_le32(obuf + tlsindex - s1addr - ic) : 0);
    linker->defineSymbol("tls_address", tlsindex - rvamin);

    linker->defineSymbol("icon_delta", icondir_count - 1);
    linker->defineSymbol("icon_offset", ncsection + icondir_offset - rvamin);

    const unsigned esi0 = s1addr + ic;
    linker->defineSymbol("start_of_uncompressed", 0u - esi0 + rvamin);
    linker->defineSymbol("start_of_compressed", esi0 + ih.imagebase);

    //NEW: TLS callback support - Stefan Widmann
    ic = s1addr + s1size - sotls - soloadconf; //moved here, we need the address of the new TLS!
    if(use_tls_callbacks)
      {
        //esi is ih.imagebase + rvamin
        linker->defineSymbol("tls_callbacks_ptr", tlscb_ptr);
        //linker->defineSymbol("tls_callbacks_off", ic + sotls - 8 - rvamin);
        linker->defineSymbol("tls_module_base", 0u - rvamin);
      }

    linker->defineSymbol(isdll ? "PEISDLL1" : "PEMAIN01", upxsection);
    //linker->dumpSymbols();
    relocateLoader();

    const unsigned lsize = getLoaderSize();
    MemBuffer loader(lsize);
    memcpy(loader,getLoader(),lsize);
    patchPackHeader(loader, lsize);

    Reloc rel(1024); // new relocations are put here
    rel.add(linker->getSymbolOffset("PEMAIN01") + 2, 3);

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

    // tls & loadconf are put into section 1

    //ic = s1addr + s1size - sotls - soloadconf;  //ATTENTION: moved upwards to TLS callback handling - Stefan Widmann
    //get address of TLS callback handler
    if (use_tls_callbacks)
    {
        tls_handler_offset = linker->getSymbolOffset("PETLSC2");
        //add relocation entry for TLS callback handler
        rel.add(tls_handler_offset + 4, 3);
    }

    processTls(&rel,&tlsiv,ic);
    ODADDR(PEDIR_TLS) = sotls ? ic : 0;
    ODSIZE(PEDIR_TLS) = sotls ? 0x18 : 0;
    ic += sotls;

    processLoadConf(&rel, &loadconfiv, ic);
    ODADDR(PEDIR_LOADCONF) = soloadconf ? ic : 0;
    ODSIZE(PEDIR_LOADCONF) = soloadconf;
    ic += soloadconf;

    // these are put into section 2

    ic = ncsection;
    if (soresources)
        processResources(&res,ic);
    ODADDR(PEDIR_RESOURCE) = soresources ? ic : 0;
    ODSIZE(PEDIR_RESOURCE) = soresources;
    ic += soresources;

    PeFile::processImports(ic, 0);
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

    PeFile::processRelocs(&rel);
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
    fo->write(obuf,c_len);
    infoWriting("compressed data", c_len);
    fo->write(loader,codesize);
    if (opt->debug.dump_stub_loader)
        OutputFile::dump(opt->debug.dump_stub_loader, loader, codesize);
    if ((ic = fo->getBytesWritten() & 3) != 0)
        fo->write(ibuf,4 - ic);
    fo->write(otls,sotls);
    fo->write(oloadconf, soloadconf);
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
    printf("%-13s: compressed   : %8ld bytes\n", getName(), (long) c_len);
    printf("%-13s: decompressor : %8ld bytes\n", getName(), (long) codesize);
    printf("%-13s: tls          : %8ld bytes\n", getName(), (long) sotls);
    printf("%-13s: resources    : %8ld bytes\n", getName(), (long) soresources);
    printf("%-13s: imports      : %8ld bytes\n", getName(), (long) soimpdlls);
    printf("%-13s: exports      : %8ld bytes\n", getName(), (long) soexport);
    printf("%-13s: relocs       : %8ld bytes\n", getName(), (long) soxrelocs);
    printf("%-13s: loadconf     : %8ld bytes\n", getName(), (long) soloadconf);
#endif

    // verify
    verifyOverlappingDecompression();

    // copy the overlay
    copyOverlay(fo, overlay, &obuf);

    // finally check the compression ratio
    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();
}

/*
vi:ts=4:et
*/

