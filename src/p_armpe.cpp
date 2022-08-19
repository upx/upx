/* p_armpe.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2022 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2022 Laszlo Molnar
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
#include "p_armpe.h"
#include "linker.h"

static const
#include "stub/arm.v4a-wince.pe.h"
static const
#include "stub/arm.v4t-wince.pe.h"

/*************************************************************************
//
**************************************************************************/

PackArmPe::PackArmPe(InputFile *f) : super(f)
{
    use_thumb_stub = false;
}


PackArmPe::~PackArmPe()
{}


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

void PackArmPe::processImports2(unsigned myimport, unsigned iat_off) // pass 2
{
    PeFile::processImports2(myimport, iat_off);

    // adjust import data
    for (import_desc *im = (import_desc*) oimpdlls; im->dllname; im++)
    {
        im->oft = im->iat;
        bool is_coredll = strcasecmp(kernelDll(), (char*) oimpdlls +
                                     im->dllname - myimport) == 0;
        im->iat = is_coredll ? iat_off : iat_off + 12;
    }
}

void PackArmPe::addStubImports()
{
    // the order of procedure names below should match the
    // assumptions of the assembly stubs
    // WARNING! these names are sorted alphanumerically by the ImportLinker
    addKernelImport("CacheSync");
    addKernelImport("GetProcAddressA");
    addKernelImport("LoadLibraryW");
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
        addLoader("DllStart", nullptr);
    addLoader("ExeStart", nullptr);

    if (ph.method == M_NRV2E_8)
        addLoader("Call2E", nullptr);
    else if (ph.method == M_NRV2B_8)
        addLoader("Call2B", nullptr);
    else if (ph.method == M_NRV2D_8)
        addLoader("Call2D", nullptr);
    else if (M_IS_LZMA(ph.method))
        addLoader("+40C,CallLZMA", nullptr);


    if (ft->id == 0x50)
        addLoader("+40C,Unfilter_0x50", nullptr);

    if (sorelocs)
        addLoader("+40C,Relocs", nullptr);

    addLoader("+40C,Imports", nullptr);
    addLoader("ProcessEnd", nullptr);

    if (ph.method == M_NRV2E_8)
        addLoader(".ucl_nrv2e_decompress_8", nullptr);
    else if (ph.method == M_NRV2B_8)
        addLoader(".ucl_nrv2b_decompress_8", nullptr);
    else if (ph.method == M_NRV2D_8)
        addLoader(".ucl_nrv2d_decompress_8", nullptr);
    else if (M_IS_LZMA(ph.method))
        addLoader("+40C,LZMA_DECODE,LZMA_DEC10", nullptr);

    addLoader("IDENTSTR,UPX1HEAD", nullptr);
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
                        ih_codebase, rvamin, 0, nullptr, 0);
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
                              unsigned ic, unsigned s1addr)
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
    super::pack0(fo, (1u<<IMAGE_SUBSYSTEM_WINDOWS_CE_GUI), 0x10000, true);
}

/* vim:set ts=4 sw=4 et: */
