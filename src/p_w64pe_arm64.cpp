/* p_w64pe_arm64.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) Markus Franz Xaver Johannes Oberhumer
   Copyright (C) Laszlo Molnar
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

// win64/arm64 (AArch64) PE packer; see p_w64pe_amd64.cpp for the amd64 sibling.
// The runtime loader lives in stub/src/arm64-win64.pe.S.

#include "conf.h"
#include "file.h"
#include "filter.h"
#include "packer.h"
#include "pefile.h"
#include "p_w64pe_arm64.h"
#define WANT_EHDR_ENUM 1
#include "p_elf_enum.h"
#include "linker.h"

static const CLANG_FORMAT_DUMMY_STATEMENT
#include "stub/arm64-win64.pe.h"

/*************************************************************************
//
**************************************************************************/

PackW64PeArm64::PackW64PeArm64(InputFile *f) : super(f) { use_stub_relocs = false; }

Linker *PackW64PeArm64::newLinker() const { return new ElfLinkerArm64LE; }

const int *PackW64PeArm64::getCompressionMethods(int method, int level) const {
    bool small = ih.codesize + ih.datasize <= 256 * 1024;
    return Packer::getDefaultCompressionMethods_le32(method, level, small);
}

const int *PackW64PeArm64::getFilters() const {
    static const int filters[] = {
        0x52,
        0x53,
        FT_END,
    };
    return filters;
}

/*************************************************************************
// pack
**************************************************************************/

tribool PackW64PeArm64::canPack() {
    if (!readFileHeader())
        return false;
    checkMachine(ih.cpu);
    if (ih.cpu != IMAGE_FILE_MACHINE_ARM64)
        return false;
    return true;
}

void PackW64PeArm64::buildLoader(const Filter *ft) {
    unsigned tmp_tlsindex = tlsindex;
    const unsigned oam1 = ih.objectalign - 1;
    const unsigned newvsize = (ph.u_len + rvamin + ph.overlap_overhead + oam1) & ~oam1;
    if (tlsindex && !isdll && ((newvsize - ph.c_len - 1024 + oam1) & ~oam1) > tlsindex + 4)
        tmp_tlsindex = 0;

    initLoader(EM_AARCH64, stub_arm64_win64_pe, sizeof(stub_arm64_win64_pe), 2);

    addLoader("START", isdll ? "PEISDLL1" : "", "PEMAIN01", tmp_tlsindex ? "PETLSHAK" : "",
              "PEMAIN02");
    addLoader(M_IS_LZMA(ph.method)    ? "PECALLLZ"
              : M_IS_NRV2B(ph.method) ? "PECALL2B"
              : M_IS_NRV2D(ph.method) ? "PECALL2D"
              : M_IS_NRV2E(ph.method) ? "PECALL2E"
                                      : "UNKNOWN_COMPRESSION_METHOD");
    addLoader("PEMAIN10");
    if (tmp_tlsindex)
        addLoader("PETLSHAK2");
    if (ft->id)
        addLoader("PEFILTER");
    if (soimport)
        addLoader("PEIMPORT");
    if (sorelocs)
        addLoader("PERELOC1");
    addLoader("PECACHE");
    if (use_tls_callbacks)
        addLoader("PETLSC");
    addLoader("PEDOJUMP");
    addLoader(M_IS_LZMA(ph.method)    ? "LZMA_ELF00,LZMA_DEC20,LZMA_DEC30"
              : M_IS_NRV2B(ph.method) ? "NRV_HEAD,NRV2B"
              : M_IS_NRV2D(ph.method) ? "NRV_HEAD,NRV2D"
              : M_IS_NRV2E(ph.method) ? "NRV_HEAD,NRV2E"
                                      : "UNKNOWN_COMPRESSION_METHOD");
    if (use_tls_callbacks)
        addLoader("PETLSC2");
    addLoader("PEFILTSYM");
    addLoader("IDENTSTR,UPX1HEAD");
}

void PackW64PeArm64::defineFilterSymbols(const Filter *ft) {
    if (ft->id) {
        // 0x52 skips the final word of a word-aligned buffer.
        linker->defineSymbol("filter_length", (ft->buf_len - (ft->id == 0x52)) & ~3u);
        linker->defineSymbol("filter_cto", ft->cto);
        linker->defineSymbol("filter_id", ft->id);
    } else {
        super::defineFilterSymbols(ft);
    }
}

void PackW64PeArm64::addStubImports() {
    // The AArch64 loader resolves the original imports via LoadLibraryA/
    // GetProcAddress and uses FlushInstructionCache for I-cache coherency of the
    // decompressed code (Windows ARM64 traps EL0 cache maintenance ops, so the
    // kernel32 API is required instead of inline dc/ic).
    addKernelImport("LoadLibraryA");
    addKernelImport("GetProcAddress");
    addKernelImport("FlushInstructionCache");
}

bool PackW64PeArm64::needForceOption() const {
    // return true if we need `--force` to pack this file
    bool r = false;
    r |= (ih.opthdrsize != 0xf0);
    r |= ((ih.flags & IMAGE_FILE_EXECUTABLE_IMAGE) == 0);
    r |= ((ih.flags & IMAGE_FILE_32BIT_MACHINE) != 0);
    r |= (ih.coffmagic != 0x20b);
    r |= (ih.entry == 0 && !isdll);
    r |= (ih.ddirsentries != 16);
    return r;
}

void PackW64PeArm64::defineSymbols(unsigned ncsection, unsigned upxsection, unsigned ic,
                                   unsigned s1addr) {
    const unsigned myimport = ncsection + soresources - rvamin;

    linker->defineSymbol("original_entry", ih.entry);
    linker->defineSymbol("start_of_relocs", crelocs);

    if (ilinker) {
        linker->defineSymbol("GetProcAddress", ilinkerGetAddress("kernel32.dll", "GetProcAddress"));
        linker->defineSymbol("LoadLibraryA", ilinkerGetAddress("kernel32.dll", "LoadLibraryA"));
        linker->defineSymbol("FlushInstructionCache",
                             ilinkerGetAddress("kernel32.dll", "FlushInstructionCache"));
        linker->defineSymbol("start_of_imports", myimport);
        linker->defineSymbol("compressed_imports", cimports);
    }

    linker->defineSymbol("tls_value",
                         (tlsindex + 4 > s1addr) ? get_le32(obuf + tlsindex - s1addr - ic) : 0);
    linker->defineSymbol("tls_address", tlsindex - rvamin);

    const unsigned esi0 = s1addr + ic;
    linker->defineSymbol("start_of_uncompressed", 0u - esi0 + rvamin);
    linker->defineSymbol("start_of_compressed", esi0);

    // AArch64-specific: the decompressor needs the compressed length to find
    // end-of-input, and the cache flush needs the decompressed image size.
    linker->defineSymbol("comp_len", ph.c_len);
    linker->defineSymbol("sizeof_image", ph.u_len);
    linker->defineSymbol("filter_buffer_start", ih.codebase - rvamin);

    if (use_tls_callbacks)
        linker->defineSymbol("tls_callbacks_ptr", tlscb_ptr - ih.imagebase);

    linker->defineSymbol("START", upxsection);
}

void PackW64PeArm64::setOhHeaderSize(const pe_section_t *osection) {
    // SizeOfHeaders
    oh.headersize =
        ALIGN_UP(pe_offset + usizeof(oh) + usizeof(*osection) * oh.objects, oh.filealign);
}

void PackW64PeArm64::pack(OutputFile *fo) {
    unsigned mask = (1u << IMAGE_SUBSYSTEM_WINDOWS_GUI) | (1u << IMAGE_SUBSYSTEM_WINDOWS_CUI) |
                    (1u << IMAGE_SUBSYSTEM_EFI_APPLICATION) |
                    (1u << IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER) |
                    (1u << IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER) | (1u << IMAGE_SUBSYSTEM_EFI_ROM);
    super::pack0(fo, mask, 0x0000000140000000ULL);
}

/*************************************************************************
// pack
**************************************************************************/

tribool PackW64PeArm64EC::canPack() {
    if (!readFileHeader())
        return false;
    checkMachine(ih.cpu);
    if (ih.cpu != IMAGE_FILE_MACHINE_ARM64EC)
        return false;
    throwCantPack("win64/arm64ec is not yet implemented");
    return true;
}

/* vim:set ts=4 sw=4 et: */
