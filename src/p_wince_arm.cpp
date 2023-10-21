/* p_wince_arm.cpp --

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
#include "p_wince_arm.h"
#include "linker.h"

static const CLANG_FORMAT_DUMMY_STATEMENT
#include "stub/arm.v4a-wince.pe.h"
static const CLANG_FORMAT_DUMMY_STATEMENT
#include "stub/arm.v4t-wince.pe.h"

/*************************************************************************
//
**************************************************************************/

PackWinCeArm::PackWinCeArm(InputFile *f) : super(f) {}

PackWinCeArm::~PackWinCeArm() noexcept {}

Linker *PackWinCeArm::newLinker() const { return new ElfLinkerArmLE; }

const int *PackWinCeArm::getCompressionMethods(int method, int level) const {
    static const int m_all[] = {M_NRV2B_8, M_NRV2E_8, M_LZMA, M_END};
    static const int m_lzma[] = {M_LZMA, M_END};
    static const int m_nrv2b[] = {M_NRV2B_8, M_END};
    static const int m_nrv2e[] = {M_NRV2E_8, M_END};

    if (!use_thumb_stub)
        return getDefaultCompressionMethods_8(method, level);

    if (method == M_ALL)
        return m_all;
    if (M_IS_LZMA(method))
        return m_lzma;
    if (M_IS_NRV2B(method))
        return m_nrv2b;
    if (M_IS_NRV2E(method))
        return m_nrv2e;
    return m_nrv2e;
}

const int *PackWinCeArm::getFilters() const {
    static const int filters[] = {0x50, FT_END};
    return filters;
}

/*************************************************************************
// import handling
**************************************************************************/

void PackWinCeArm::processImports2(unsigned myimport, unsigned iat_off) // pass 2
{
    PeFile::processImports2(myimport, iat_off);

    // adjust import data
    for (import_desc *im = (import_desc *) oimpdlls; im->dllname; im++) {
        im->oft = im->iat;
        bool is_coredll = strcasecmp(kernelDll(), oimpdlls + (im->dllname - myimport)) == 0;
        im->iat = is_coredll ? iat_off : iat_off + 12;
    }
}

void PackWinCeArm::addStubImports() {
    // the order of procedure names below should match the
    // assumptions of the assembly stubs
    // WARNING! these names are sorted alphanumerically by the ImportLinker
    addKernelImport("CacheSync");
    addKernelImport("GetProcAddressA");
    addKernelImport("LoadLibraryW");
}

void PackWinCeArm::processTls(Interval *) // pass 1
{
    if ((sotls = ALIGN_UP(IDSIZE(PEDIR_TLS), 4u)) == 0)
        return;

    // never should happen on wince/arm
    throwCantPack("Static TLS entries found. Send a report please.");
}

/*************************************************************************
// pack
**************************************************************************/

tribool PackWinCeArm::canPack() {
    if (!readFileHeader())
        return false;
    checkMachine(ih.cpu);
    if (ih.cpu != IMAGE_FILE_MACHINE_ARM && ih.cpu != IMAGE_FILE_MACHINE_THUMB)
        return false;
    use_thumb_stub |= ih.cpu == IMAGE_FILE_MACHINE_THUMB || (ih.entry & 1) == 1;
    // HACK FIXME later: don't misuse opt->cpu_x86, need an extra option to force thumb stub
    use_thumb_stub |= (opt->cpu_x86 == opt->CPU_8086);
    return true;
}

void PackWinCeArm::buildLoader(const Filter *ft) {
    const byte *loader = use_thumb_stub ? stub_arm_v4t_wince_pe : stub_arm_v4a_wince_pe;
    unsigned size = use_thumb_stub ? sizeof(stub_arm_v4t_wince_pe) : sizeof(stub_arm_v4a_wince_pe);

    // prepare loader
    initLoader(loader, size);

    if (isdll)
        addLoader("DllStart");
    addLoader("ExeStart");

    if (ph.method == M_NRV2E_8)
        addLoader("Call2E");
    else if (ph.method == M_NRV2B_8)
        addLoader("Call2B");
    else if (ph.method == M_NRV2D_8)
        addLoader("Call2D");
    else if (M_IS_LZMA(ph.method))
        addLoader("+40C,CallLZMA");

    if (ft->id == 0x50)
        addLoader("+40C,Unfilter_0x50");

    if (sorelocs)
        addLoader("+40C,Relocs");

    addLoader("+40C,Imports");
    addLoader("ProcessEnd");

    if (ph.method == M_NRV2E_8)
        addLoader(".ucl_nrv2e_decompress_8");
    else if (ph.method == M_NRV2B_8)
        addLoader(".ucl_nrv2b_decompress_8");
    else if (ph.method == M_NRV2D_8)
        addLoader(".ucl_nrv2d_decompress_8");
    else if (M_IS_LZMA(ph.method))
        addLoader("+40C,LZMA_DECODE,LZMA_DEC10");

    addLoader("IDENTSTR,UPX1HEAD");
}

bool PackWinCeArm::needForceOption() const {
    // return true if we need `--force` to pack this file
    bool r = false;
    r |= (ih.opthdrsize != 0xe0);
    r |= ((ih.flags & IMAGE_FILE_EXECUTABLE_IMAGE) == 0);
    r |= (ih.entry == 0 /*&& !isdll*/);
    r |= (ih.ddirsentries != 16);
    //// r |= (IDSIZE(PEDIR_EXCEPTION) != 0); // is this used on arm?
    //// r |= (IDSIZE(PEDIR_COPYRIGHT) != 0);
    return r;
}

void PackWinCeArm::callCompressWithFilters(Filter &ft, int filter_strategy, unsigned ih_codebase) {
    // limit stack size needed for runtime decompression
    upx_compress_config_t cconf;
    cconf.reset();
    cconf.conf_lzma.max_num_probs = 1846 + (768 << 4); // ushort: ~28 KiB stack
    compressWithFilters(&ft, 2048, &cconf, filter_strategy, ih_codebase, rvamin, 0, nullptr, 0);
}

void PackWinCeArm::addNewRelocations(Reloc &rel, unsigned upxsection) {
    static const char *const symbols_to_relocate[] = {"ONAM", "BIMP", "BREL", "FIBE",
                                                      "FIBS", "ENTR", "DST0", "SRC0"};
    for (unsigned s2r = 0; s2r < TABLESIZE(symbols_to_relocate); s2r++) {
        unsigned off = linker->getSymbolOffset(symbols_to_relocate[s2r]);
        if (off != 0xdeaddead)
            rel.add(off + upxsection, 3);
    }
}

unsigned PackWinCeArm::getProcessImportParam(unsigned upxsection) {
    return linker->getSymbolOffset("IATT") + upxsection;
}

void PackWinCeArm::defineSymbols(unsigned ncsection, unsigned, unsigned, unsigned ic,
                                 unsigned s1addr) {
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

void PackWinCeArm::setOhDataBase(const pe_section_t *osection) { oh.database = osection[2].vaddr; }

void PackWinCeArm::setOhHeaderSize(const pe_section_t *osection) {
    // SizeOfHeaders
    oh.headersize = osection[1].rawdataptr;
}

void PackWinCeArm::pack(OutputFile *fo) {
    super::pack0(fo, (1u << IMAGE_SUBSYSTEM_WINDOWS_CE_GUI), 0x10000, true);
}

/* vim:set ts=4 sw=4 et: */
