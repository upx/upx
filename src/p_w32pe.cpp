/* p_w32pe.cpp --

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


#include "conf.h"
#include "file.h"
#include "filter.h"
#include "packer.h"
#include "pefile.h"
#include "p_w32pe.h"
#include "linker.h"

static const
#include "stub/i386-win32.pe.h"

/*************************************************************************
//
**************************************************************************/

PackW32Pe::PackW32Pe(InputFile *f) : super(f)
{}


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
    if (fi->st_size() >= 0x206) {
        char buf[6];
        fi->seek(0x200, SEEK_SET);
        fi->readx(buf, 6);
        isrtm = memcmp(buf, "32STUB" ,6) == 0;
    }
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
    if (isdll)
        addLoader("PEISDLL1");
    addLoader("PEMAIN01",
              use_stub_relocs ? "PESOCREL" : "PESOCPIC",
              "PESOUNC0",
              icondir_count > 1 ? (icondir_count == 2 ? "PEICONS1" : "PEICONS2") : "",
              tmp_tlsindex ? "PETLSHAK" : "",
              "PEMAIN02",
              ph.first_offset_found == 1 ? "PEMAIN03" : "",
              getDecompressorSections(),
              //multipass ? "PEMULTIP" : "",
              "PEMAIN10",
              nullptr
             );
    addLoader(tmp_tlsindex ? "PETLSHAK2" : "");
    if (ft->id)
    {
        const unsigned texv = ih.codebase - rvamin;
        assert(ft->calls > 0);
        addLoader(texv ? "PECTTPOS" : "PECTTNUL",nullptr);
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
                  nullptr
                 );
    if (sorelocs)
    {
        addLoader(soimport == 0 || soimport + cimports != crelocs ? "PERELOC1" : "PERELOC2",
                  "PERELOC3,RELOC320",
                  big_relocs ? "REL32BIG" : "",
                  "RELOC32J",
                  nullptr
                 );
        //FIXME: the following should be moved out of the above if
        addLoader(big_relocs&6 ? "PERLOHI0" : "",
                  big_relocs&4 ? "PERELLO0" : "",
                  big_relocs&2 ? "PERELHI0" : "",
                  nullptr
                 );
    }
    if (use_dep_hack)
        addLoader("PEDEPHAK", nullptr);

    //NEW: TLS callback support PART 1, the callback handler installation - Stefan Widmann
    if(use_tls_callbacks)
        addLoader("PETLSC", nullptr);

    addLoader("PEMAIN20", nullptr);
    if (use_clear_dirty_stack)
        addLoader("CLEARSTACK", nullptr);
    addLoader("PEMAIN21", nullptr);
    //NEW: last loader sections split up to insert TLS callback handler - Stefan Widmann
    addLoader(ih.entry || !ilinker ? "PEDOJUMP" : "PERETURN", nullptr);

    //NEW: TLS callback support PART 2, the callback handler - Stefan Widmann
    if(use_tls_callbacks)
        addLoader("PETLSC2", nullptr);

    addLoader("IDENTSTR,UPX1HEAD", nullptr);

}

bool PackW32Pe::handleForceOption()
{
    return (ih.cpu < 0x14c || ih.cpu > 0x150)
        || (ih.opthdrsize != 0xe0)
        || ((ih.flags & EXECUTABLE) == 0)
        || ((ih.flags & BITS_32_MACHINE) == 0) //NEW: 32 bit machine flag must be set - Stefan Widmann
        || (ih.coffmagic != 0x10B) //COFF magic is 0x10B in PE files, 0x20B in PE32+ files - Stefan Widmann
        || (ih.entry == 0 && !isdll)
        || (ih.ddirsentries != 16)
        || IDSIZE(PEDIR_EXCEPTION) // is this used on i386?
//        || IDSIZE(PEDIR_COPYRIGHT)
        ;
}

void PackW32Pe::defineSymbols(unsigned ncsection, unsigned upxsection,
                              unsigned sizeof_oh, unsigned ic,
                              unsigned s1addr)
{
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
        const unsigned swri = pe_offset + sizeof_oh + sizeof(pe_section_t) - 1;
        // make sure we only touch the minimum number of pages
        const unsigned addr = 0u - rvamin + swri;
        linker->defineSymbol("swri", addr &  0xfff);    // page offset
        // check whether osection[0].flags and osection[1].flags
        // are on the same page
        linker->defineSymbol("vp_size", ((addr & 0xfff) + 0x28 >= 0x1000) ?
                             0x2000 : 0x1000);          // 2 pages or 1 page
        linker->defineSymbol("vp_base", addr &~ 0xfff); // page mask
        linker->defineSymbol("VirtualProtect", 0u-rvamin +
                             ilinkerGetAddress("kernel32.dll", "VirtualProtect"));
    }
    linker->defineSymbol("reloc_delt", 0u - (unsigned) ih.imagebase - rvamin);
    linker->defineSymbol("start_of_relocs", crelocs);

    if (ilinker) {
        if (!isdll)
            linker->defineSymbol("ExitProcess", 0u-rvamin +
                                 ilinkerGetAddress("kernel32.dll", "ExitProcess"));
        linker->defineSymbol("GetProcAddress", 0u-rvamin +
                             ilinkerGetAddress("kernel32.dll", "GetProcAddress"));
        linker->defineSymbol("kernel32_ordinals", myimport);
        linker->defineSymbol("LoadLibraryA", 0u-rvamin +
                             ilinkerGetAddress("kernel32.dll", "LoadLibraryA"));
        linker->defineSymbol("start_of_imports", myimport);
        linker->defineSymbol("compressed_imports", cimports);
    }

    defineDecompressorSymbols();
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
    linker->defineSymbol("start_of_compressed", use_stub_relocs ? esi0 + ih.imagebase : esi0);

    if (use_tls_callbacks)
    {
        //esi is ih.imagebase + rvamin
        linker->defineSymbol("tls_callbacks_ptr", tlscb_ptr);
        linker->defineSymbol("tls_module_base", 0u - rvamin);
    }

    linker->defineSymbol(isdll ? "PEISDLL1" : "PEMAIN01", upxsection);
    //linker->dumpSymbols();
}

void PackW32Pe::addNewRelocations(Reloc &rel, unsigned base)
{
    if (use_stub_relocs)
        rel.add(base + linker->getSymbolOffset("PESOCREL") + 1, 3);
}

void PackW32Pe::setOhDataBase(const pe_section_t *osection)
{
    oh.database = osection[2].vaddr;
}

void PackW32Pe::setOhHeaderSize(const pe_section_t *osection)
{
    oh.headersize = ALIGN_UP(pe_offset + sizeof(oh) + sizeof(*osection) * oh.objects, oh.filealign);
}

void PackW32Pe::pack(OutputFile *fo)
{
    super::pack0(fo
        , (1u<<IMAGE_SUBSYSTEM_WINDOWS_GUI)
        | (1u<<IMAGE_SUBSYSTEM_WINDOWS_CUI)
        | (1u<<IMAGE_SUBSYSTEM_EFI_APPLICATION)
        | (1u<<IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER)
        | (1u<<IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER)
        | (1u<<IMAGE_SUBSYSTEM_EFI_ROM)
        , 0x400000
        , false);
}

/* vim:set ts=4 sw=4 et: */
