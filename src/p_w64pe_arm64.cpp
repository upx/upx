/* p_w64pe_arm64.cpp --

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

// TODO: implement this; see p_w64pe_amd64.cpp

#include "conf.h"
#include "file.h"
#include "filter.h"
#include "packer.h"
#include "pefile.h"
#include "p_w64pe_arm64.h"
#include "linker.h"

// static const CLANG_FORMAT_DUMMY_STATEMENT
// #include "stub/arm64-win64.pe.h"

/*************************************************************************
//
**************************************************************************/

PackW64PeArm64::PackW64PeArm64(InputFile *f) : super(f) {
    // use_stub_relocs = false;
}

Linker *PackW64PeArm64::newLinker() const { return new ElfLinkerArm64LE; }

const int *PackW64PeArm64::getCompressionMethods(int method, int level) const {
    bool small = ih.codesize + ih.datasize <= 256 * 1024;
    return Packer::getDefaultCompressionMethods_le32(method, level, small);
}

const int *PackW64PeArm64::getFilters() const { return nullptr; }

/*************************************************************************
// pack
**************************************************************************/

tribool PackW64PeArm64::canPack() {
    if (!readFileHeader())
        return false;
    checkMachine(ih.cpu);
    if (ih.cpu != IMAGE_FILE_MACHINE_ARM64)
        return false;
    throwCantPack("win64/arm64 is not yet implemented");
    return true;
}

void PackW64PeArm64::buildLoader(const Filter *ft) {
    UNUSED(ft);
    throwCantPack("not yet implemented");
}

bool PackW64PeArm64::needForceOption() const {
    // return true if we need `--force` to pack this file
    bool r = false;
    throwCantPack("not yet implemented");
    return r;
}

void PackW64PeArm64::defineSymbols(unsigned ncsection, unsigned upxsection, unsigned sizeof_oh,
                                   unsigned ic, unsigned s1addr) {
    UNUSED(ncsection);
    UNUSED(upxsection);
    UNUSED(sizeof_oh);
    UNUSED(ic);
    UNUSED(s1addr);
    throwCantPack("not yet implemented");
}

void PackW64PeArm64::setOhHeaderSize(const pe_section_t *osection) {
    // SizeOfHeaders
    UNUSED(osection);
    throwCantPack("not yet implemented");
}

void PackW64PeArm64::pack(OutputFile *fo) {
    UNUSED(fo);
    throwCantPack("not yet implemented");
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
