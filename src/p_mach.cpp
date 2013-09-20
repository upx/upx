/* p_mach.cpp -- pack Mach Object executable

   This file is part of the UPX executable compressor.

   Copyright (C) 2004-2013 John Reiser
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

   John Reiser
   <jreiser@users.sourceforge.net>
 */


#include "conf.h"

#include "file.h"
#include "filter.h"
#include "linker.h"
#include "packer.h"
#include "p_mach.h"
#include "ui.h"

static const
#include "stub/powerpc-darwin.macho-entry.h"
static const
#include "stub/powerpc-darwin.macho-fold.h"

static const
#include "stub/i386-darwin.macho-entry.h"
static const
#include "stub/i386-darwin.macho-fold.h"

static const
#include "stub/amd64-darwin.macho-entry.h"
static const
#include "stub/amd64-darwin.macho-fold.h"

static const
#include "stub/arm-darwin.macho-entry.h"
static const
#include "stub/arm-darwin.macho-fold.h"

// Packing a Darwin (Mach-o) Mac OS X dylib (dynamic shared library)
// is restricted.  UPX gets control as the -init function, at the very
// end of processing by dyld.  Relocation, loading of dependent libraries,
// etc., already have taken place before decompression.  So the Mach-o
// headers, the __IMPORT segment, the __LINKEDIT segment, anything
// that is modifed by relocation, etc., cannot be compressed.
// We simplify arbitrarily by compressing only the __TEXT segment,
// which must be the first segment.

static const
#include "stub/i386-darwin.dylib-entry.h"
// The runtime stub for the dyld -init routine does not use "fold"ed code.
//#include "stub/i386-darwin.dylib-fold.h"

static const
#include "stub/powerpc-darwin.dylib-entry.h"
static const
#include "stub/amd64-darwin.dylib-entry.h"

static const unsigned lc_segment[2] = {
    0x1, 0x19
    //Mach_segment_command::LC_SEGMENT,
    //Mach_segment_command::LC_SEGMENT_64
};

static const unsigned lc_routines[2] = {
    0x11, 0x1a
    //Mach_segment_command::LC_ROUTINES,
    //Mach_segment_command::LC_ROUTINES_64
};

template <class T>
PackMachBase<T>::PackMachBase(InputFile *f, unsigned cputype, unsigned filetype,
        unsigned flavor, unsigned count, unsigned size) :
    super(f), my_cputype(cputype), my_filetype(filetype), my_thread_flavor(flavor),
    my_thread_state_word_count(count), my_thread_command_size(size),
    n_segment(0), rawmseg(NULL), msegcmd(NULL), o_routines_cmd(0),
    prev_init_address(0)
{
    MachClass::compileTimeAssertions();
    bele = N_BELE_CTP::getRTP((const BeLePolicy*) NULL);
}

template <class T>
PackMachBase<T>::~PackMachBase()
{
    delete [] rawmseg;
    delete [] msegcmd;
}

PackDylibI386::PackDylibI386(InputFile *f) : super(f)
{
    my_filetype = Mach_header::MH_DYLIB;
}

PackDylibAMD64::PackDylibAMD64(InputFile *f) : super(f)
{
    my_filetype = Mach_header::MH_DYLIB;
}

PackDylibPPC32::PackDylibPPC32(InputFile *f) : super(f)
{
    my_filetype = Mach_header::MH_DYLIB;
}

template <class T>
const int *PackMachBase<T>::getCompressionMethods(int method, int level) const
{
    // There really is no LE bias.
    return Packer::getDefaultCompressionMethods_le32(method, level);
}

const int *PackMachARMEL::getCompressionMethods(int method, int level) const
{
    return Packer::getDefaultCompressionMethods_8(method, level);
}


PackMachPPC32::PackMachPPC32(InputFile *f) : super(f, Mach_header::CPU_TYPE_POWERPC,
        Mach_header::MH_EXECUTE, Mach_thread_command::PPC_THREAD_STATE,
        sizeof(Mach_ppc_thread_state)>>2, sizeof(threado))
{ }

const int *PackMachPPC32::getFilters() const
{
    static const int filters[] = { 0xd0, FT_END };
    return filters;
}

PackMachI386::PackMachI386(InputFile *f) : super(f, Mach_header::CPU_TYPE_I386,
        Mach_header::MH_EXECUTE, (unsigned)Mach_thread_command::x86_THREAD_STATE32,
        sizeof(Mach_i386_thread_state)>>2, sizeof(threado))
{ }

int const *PackMachI386::getFilters() const
{
    static const int filters[] = { 0x49, FT_END };
    return filters;
}

PackMachAMD64::PackMachAMD64(InputFile *f) : super(f, Mach_header::CPU_TYPE_X86_64,
        Mach_header::MH_EXECUTE, (unsigned)Mach_thread_command::x86_THREAD_STATE64,
        sizeof(Mach_AMD64_thread_state)>>2, sizeof(threado))
{ }

int const *PackMachAMD64::getFilters() const
{
    static const int filters[] = { 0x49, FT_END };
    return filters;
}

PackMachARMEL::PackMachARMEL(InputFile *f) : super(f, Mach_header::CPU_TYPE_ARM,
        Mach_header::MH_EXECUTE, (unsigned)Mach_thread_command::ARM_THREAD_STATE,
        sizeof(Mach_ARM_thread_state)>>2, sizeof(threado))
{ }

int const *PackMachARMEL::getFilters() const
{
    static const int filters[] = { 0x50, FT_END };
    return filters;
}

Linker *PackMachPPC32::newLinker() const
{
    return new ElfLinkerPpc32;
}

Linker *PackMachI386::newLinker() const
{
    return new ElfLinkerX86;
}

Linker *PackMachAMD64::newLinker() const
{
    return new ElfLinkerAMD64;
}

Linker *PackMachARMEL::newLinker() const
{
    return new ElfLinkerArmLE;
}

template <class T>
void
PackMachBase<T>::addStubEntrySections(Filter const *)
{
    addLoader("MACOS000", NULL);
   //addLoader(getDecompressorSections(), NULL);
    addLoader(
        ( M_IS_NRV2E(ph.method) ? "NRV_HEAD,NRV2E,NRV_TAIL"
        : M_IS_NRV2D(ph.method) ? "NRV_HEAD,NRV2D,NRV_TAIL"
        : M_IS_NRV2B(ph.method) ? "NRV_HEAD,NRV2B,NRV_TAIL"
        : M_IS_LZMA(ph.method)  ? "LZMA_ELF00,LZMA_DEC20,LZMA_DEC30"
        : NULL), NULL);
    if (hasLoaderSection("CFLUSH"))
        addLoader("CFLUSH");
    addLoader("ELFMAINY,IDENTSTR,+40,ELFMAINZ,FOLDEXEC", NULL);
}

void PackMachI386::addStubEntrySections(Filter const *ft)
{
    int const n_mru = ft->n_mru;  // FIXME: belongs to filter? packerf?

            // entry to stub
    addLoader("LEXEC000", NULL);

    if (ft->id) {
        { // decompr, unfilter are separate
            addLoader("LXUNF000", NULL);
            addLoader("LXUNF002", NULL);
                if (0x80==(ft->id & 0xF0)) {
                    if (256==n_mru) {
                        addLoader("MRUBYTE0", NULL);
                    }
                    else if (n_mru) {
                        addLoader("LXMRU005", NULL);
                    }
                    if (n_mru) {
                        addLoader("LXMRU006", NULL);
                    }
                    else {
                        addLoader("LXMRU007", NULL);
                    }
            }
            else if (0x40==(ft->id & 0xF0)) {
                addLoader("LXUNF008", NULL);
            }
            addLoader("LXUNF010", NULL);
        }
        if (n_mru) {
            addLoader("LEXEC009", NULL);
        }
    }
    addLoader("LEXEC010", NULL);
    addLoader(getDecompressorSections(), NULL);
    addLoader("LEXEC015", NULL);
    if (ft->id) {
        {  // decompr, unfilter are separate
            if (0x80!=(ft->id & 0xF0)) {
                addLoader("LXUNF042", NULL);
            }
        }
        addFilter32(ft->id);
        { // decompr, unfilter are separate
            if (0x80==(ft->id & 0xF0)) {
                if (0==n_mru) {
                    addLoader("LXMRU058", NULL);
                }
            }
            addLoader("LXUNF035", NULL);
        }
    }
    else {
        addLoader("LEXEC017", NULL);
    }

    addLoader("IDENTSTR", NULL);
    addLoader("LEXEC020", NULL);
    addLoader("FOLDEXEC", NULL);
}

void PackMachAMD64::addStubEntrySections(Filter const * /*ft*/)
{
    addLoader("MACHMAINX", NULL);
   //addLoader(getDecompressorSections(), NULL);
    addLoader(
        ( M_IS_NRV2E(ph.method) ? "NRV_HEAD,NRV2E,NRV_TAIL"
        : M_IS_NRV2D(ph.method) ? "NRV_HEAD,NRV2D,NRV_TAIL"
        : M_IS_NRV2B(ph.method) ? "NRV_HEAD,NRV2B,NRV_TAIL"
        : M_IS_LZMA(ph.method)  ? "LZMA_ELF00,+80C,LZMA_DEC20,LZMA_DEC30"
        : NULL), NULL);
    if (hasLoaderSection("CFLUSH"))
        addLoader("CFLUSH");
    addLoader("MACHMAINY,IDENTSTR,+40,MACHMAINZ,FOLDEXEC", NULL);
}

void PackMachARMEL::addStubEntrySections(Filter const * /*ft*/)
{
    addLoader("MACHMAINX", NULL);
   //addLoader(getDecompressorSections(), NULL);
    addLoader(
        ( M_IS_NRV2E(ph.method) ? "NRV_HEAD,NRV2E,NRV_TAIL"
        : M_IS_NRV2D(ph.method) ? "NRV_HEAD,NRV2D,NRV_TAIL"
        : M_IS_NRV2B(ph.method) ? "NRV_HEAD,NRV2B,NRV_TAIL"
        : M_IS_LZMA(ph.method)  ? "LZMA_ELF00,+80C,LZMA_DEC20,LZMA_DEC30"
        : NULL), NULL);
    if (hasLoaderSection("CFLUSH"))
        addLoader("CFLUSH");
    addLoader("MACHMAINY,IDENTSTR,+40,MACHMAINZ,FOLDEXEC", NULL);
}

template <class T>
void PackMachBase<T>::defineSymbols(Filter const *)
{
    // empty
}


template <class T>
void
PackMachBase<T>::buildMachLoader(
    upx_byte const *const proto,
    unsigned        const szproto,
    upx_byte const *const fold,
    unsigned        const szfold,
    Filter const *ft
)
{
    initLoader(proto, szproto);

    struct b_info h; memset(&h, 0, sizeof(h));
    unsigned fold_hdrlen = 0;
  if (0 < szfold) {
    h.sz_unc = (szfold < fold_hdrlen) ? 0 : (szfold - fold_hdrlen);
    h.b_method = (unsigned char) ph.method;
    h.b_ftid = (unsigned char) ph.filter;
    h.b_cto8 = (unsigned char) ph.filter_cto;
  }
    unsigned char const *const uncLoader = fold_hdrlen + fold;

    unsigned char *const cprLoader = new unsigned char[sizeof(h) + h.sz_unc];
  if (0 < szfold) {
    unsigned sz_cpr = 0;
    int r = upx_compress(uncLoader, h.sz_unc, sizeof(h) + cprLoader, &sz_cpr,
        NULL, ph.method, 10, NULL, NULL );
    h.sz_cpr = sz_cpr;
    if (r != UPX_E_OK || h.sz_cpr >= h.sz_unc)
        throwInternalError("loader compression failed");
  }
    memcpy(cprLoader, &h, sizeof(h));

    // This adds the definition to the "library", to be used later.
    linker->addSection("FOLDEXEC", cprLoader, sizeof(h) + h.sz_cpr, 0);
    delete [] cprLoader;

    int const GAP = 128;  // must match stub/l_mac_ppc.S
    int const NO_LAP = 64;  // must match stub/src/*darwin*.S
    segTEXT.vmsize = h.sz_unc - h.sz_cpr + GAP + NO_LAP;

    addStubEntrySections(ft);

    defineSymbols(ft);
    relocateLoader();
}

void
PackMachPPC32::buildLoader(const Filter *ft)
{
    buildMachLoader(
        stub_powerpc_darwin_macho_entry, sizeof(stub_powerpc_darwin_macho_entry),
        stub_powerpc_darwin_macho_fold,  sizeof(stub_powerpc_darwin_macho_fold),  ft );
}

void
PackMachI386::buildLoader(const Filter *ft)
{
    buildMachLoader(
        stub_i386_darwin_macho_entry, sizeof(stub_i386_darwin_macho_entry),
        stub_i386_darwin_macho_fold,  sizeof(stub_i386_darwin_macho_fold),  ft );
}

void
PackMachAMD64::buildLoader(const Filter *ft)
{
    buildMachLoader(
        stub_amd64_darwin_macho_entry, sizeof(stub_amd64_darwin_macho_entry),
        stub_amd64_darwin_macho_fold,  sizeof(stub_amd64_darwin_macho_fold),  ft );
}

void
PackMachARMEL::buildLoader(const Filter *ft)
{
    buildMachLoader(
        stub_arm_darwin_macho_entry, sizeof(stub_arm_darwin_macho_entry),
        stub_arm_darwin_macho_fold,  sizeof(stub_arm_darwin_macho_fold),  ft );
}

void
PackDylibI386::buildLoader(const Filter *ft)
{
    buildMachLoader(
        stub_i386_darwin_dylib_entry, sizeof(stub_i386_darwin_dylib_entry),
        0,  0,  ft );
}

void
PackDylibAMD64::buildLoader(const Filter *ft)
{
    buildMachLoader(
        stub_amd64_darwin_dylib_entry, sizeof(stub_amd64_darwin_dylib_entry),
        0,  0,  ft );
}

void
PackDylibPPC32::buildLoader(const Filter *ft)
{
    buildMachLoader(
        stub_powerpc_darwin_dylib_entry, sizeof(stub_powerpc_darwin_dylib_entry),
        0,  0,  ft );
}

template <class T>
void PackMachBase<T>::patchLoader() { }

template <class T>
void PackMachBase<T>::updateLoader(OutputFile *) {}

template <class T>
void PackMachBase<T>::patchLoaderChecksum()
{
    unsigned char *const ptr = getLoader();
    l_info *const lp = &linfo;
    // checksum for loader; also some PackHeader info
    lp->l_checksum = 0;
    lp->l_magic = UPX_MAGIC_LE32;  // LE32 always
    set_te16(&lp->l_lsize, (unsigned short) lsize);
    lp->l_version = (unsigned char) ph.version;
    lp->l_format  = (unsigned char) ph.format;
    // INFO: lp->l_checksum is currently unused
    set_te32(&lp->l_checksum, upx_adler32(ptr, lsize));
}

template <class T>
int __acc_cdecl_qsort
PackMachBase<T>::compare_segment_command(void const *const aa, void const *const bb)
{
    Mach_segment_command const *const a = (Mach_segment_command const *)aa;
    Mach_segment_command const *const b = (Mach_segment_command const *)bb;
    unsigned const lc_seg = lc_segment[sizeof(Addr)>>3];
    unsigned const xa = a->cmd - lc_seg;
    unsigned const xb = b->cmd - lc_seg;
           if (xa < xb)        return -1;  // LC_SEGMENT first
           if (xa > xb)        return  1;
    // Beware 0==.vmsize (some MacOSX __DWARF debug info: a "comment")
    if (a->vmsize!=0 && b->vmsize!=0) {
        if (a->vmaddr < b->vmaddr) return -1;  // ascending by .vmaddr
        if (a->vmaddr > b->vmaddr) return  1;
    }
    else { // 0==.vmsize goes last, except ordered by fileoff
        if (a->vmsize)         return -1;  // 'a' is first
        if (b->vmsize)         return  1;  // 'a' is last
        if (a->fileoff < b->fileoff)
                               return -1;
        if (a->fileoff > b->fileoff)
                               return  1;
    }
                               return  0;
}

#undef PAGE_MASK
#undef PAGE_SIZE
#define PAGE_MASK (~0u<<12)
#define PAGE_SIZE (0u-PAGE_MASK)

#undef PAGE_MASK64
#undef PAGE_SIZE64
#define PAGE_MASK64 (~(upx_uint64_t)0<<12)
#define PAGE_SIZE64 ((upx_uint64_t)0-PAGE_MASK64)

// At 2013-02-03 part of the source for codesign was
//    http://opensource.apple.com/source/cctools/cctools-836/libstuff/ofile.c

void PackMachPPC32::pack4(OutputFile *fo, Filter &ft)  // append PackHeader
{
    // offset of p_info in compressed file
    overlay_offset = sizeof(mhdro) + sizeof(segZERO)
        + sizeof(segXHDR) + sizeof(secXHDR)
        + sizeof(segTEXT) + sizeof(secTEXT)
        + sizeof(segLINK) + sizeof(threado) + sizeof(linfo);
    if (my_filetype==Mach_header::MH_EXECUTE) {
        overlay_offset += sizeof(uuid_cmd) + sizeof(linkitem);
    }

    super::pack4(fo, ft);
    unsigned const t = fo->getBytesWritten();
    segTEXT.filesize = t;
    segTEXT.vmsize  += t;  // utilize GAP + NO_LAP + sz_unc - sz_cpr
    secTEXT.offset = overlay_offset - sizeof(linfo);
    secTEXT.addr = segTEXT.vmaddr + secTEXT.offset;
    secTEXT.size = segTEXT.vmsize - secTEXT.offset;
    secXHDR.offset = overlay_offset - sizeof(linfo);
    if (my_filetype==Mach_header::MH_EXECUTE) {
        secXHDR.offset -= sizeof(uuid_cmd) + sizeof(linkitem);
    }
    secXHDR.addr += secXHDR.offset;
    unsigned foff1 = (PAGE_MASK & (~PAGE_MASK + segTEXT.filesize));
    if (foff1 < segTEXT.vmsize)
        foff1 += PAGE_SIZE;  // codesign disallows overhang
    segLINK.fileoff = foff1;
    segLINK.vmaddr = segTEXT.vmaddr + foff1;
    fo->seek(foff1 - 1, SEEK_SET); fo->write("", 1);
    fo->seek(sizeof(mhdro), SEEK_SET);
    fo->rewrite(&segZERO, sizeof(segZERO));
    fo->rewrite(&segXHDR, sizeof(segXHDR));
    fo->rewrite(&secXHDR, sizeof(secXHDR));
    fo->rewrite(&segTEXT, sizeof(segTEXT));
    fo->rewrite(&secTEXT, sizeof(secTEXT));
    fo->rewrite(&segLINK, sizeof(segLINK));
    fo->rewrite(&threado, sizeof(threado));
    if (my_filetype==Mach_header::MH_EXECUTE) {
        fo->rewrite(&uuid_cmd, sizeof(uuid_cmd));
        fo->rewrite(&linkitem, sizeof(linkitem));
    }
    fo->rewrite(&linfo, sizeof(linfo));
}

void PackMachI386::pack4(OutputFile *fo, Filter &ft)  // append PackHeader
{
    // offset of p_info in compressed file
    overlay_offset = sizeof(mhdro) + sizeof(segZERO)
        + sizeof(segXHDR) + sizeof(secXHDR)
        + sizeof(segTEXT) + sizeof(secTEXT)
        + sizeof(segLINK) + sizeof(threado) + sizeof(linfo);
    if (my_filetype==Mach_header::MH_EXECUTE) {
        overlay_offset += sizeof(uuid_cmd) + sizeof(linkitem);
    }

    super::pack4(fo, ft);
    unsigned const t = fo->getBytesWritten();
    segTEXT.filesize = t;
    segTEXT.vmsize  += t;  // utilize GAP + NO_LAP + sz_unc - sz_cpr
    secTEXT.offset = overlay_offset - sizeof(linfo);
    secTEXT.addr = segTEXT.vmaddr + secTEXT.offset;
    secTEXT.size = segTEXT.vmsize - secTEXT.offset;
    secXHDR.offset = overlay_offset - sizeof(linfo);
    if (my_filetype==Mach_header::MH_EXECUTE) {
        secXHDR.offset -= sizeof(uuid_cmd) + sizeof(linkitem);
    }
    secXHDR.addr += secXHDR.offset;
    unsigned foff1 = (PAGE_MASK & (~PAGE_MASK + segTEXT.filesize));
    if (foff1 < segTEXT.vmsize)
        foff1 += PAGE_SIZE;  // codesign disallows overhang
    segLINK.fileoff = foff1;
    segLINK.vmaddr = segTEXT.vmaddr + foff1;
    fo->seek(foff1 - 1, SEEK_SET); fo->write("", 1);
    fo->seek(sizeof(mhdro), SEEK_SET);
    fo->rewrite(&segZERO, sizeof(segZERO));
    fo->rewrite(&segXHDR, sizeof(segXHDR));
    fo->rewrite(&secXHDR, sizeof(secXHDR));
    fo->rewrite(&segTEXT, sizeof(segTEXT));
    fo->rewrite(&secTEXT, sizeof(secTEXT));
    fo->rewrite(&segLINK, sizeof(segLINK));
    fo->rewrite(&threado, sizeof(threado));
    if (my_filetype==Mach_header::MH_EXECUTE) {
        fo->rewrite(&uuid_cmd, sizeof(uuid_cmd));
        fo->rewrite(&linkitem, sizeof(linkitem));
    }
    fo->rewrite(&linfo, sizeof(linfo));
}

void PackMachAMD64::pack4(OutputFile *fo, Filter &ft)  // append PackHeader
{
    // offset of p_info in compressed file
    overlay_offset = sizeof(mhdro) + sizeof(segZERO)
        + sizeof(segXHDR) + sizeof(secXHDR)
        + sizeof(segTEXT) + sizeof(secTEXT)
        + sizeof(segLINK) + sizeof(threado) + sizeof(linfo);
    if (my_filetype==Mach_header::MH_EXECUTE) {
        overlay_offset += sizeof(uuid_cmd) + sizeof(linkitem);
    }

    super::pack4(fo, ft);
    unsigned const t = fo->getBytesWritten();
    segTEXT.filesize = t;
    segTEXT.vmsize  += t;  // utilize GAP + NO_LAP + sz_unc - sz_cpr
    secTEXT.offset = overlay_offset - sizeof(linfo);
    secTEXT.addr = segTEXT.vmaddr + secTEXT.offset;
    secTEXT.size = segTEXT.vmsize - secTEXT.offset;
    secXHDR.offset = overlay_offset - sizeof(linfo);
    if (my_filetype==Mach_header::MH_EXECUTE) {
        secXHDR.offset -= sizeof(uuid_cmd) + sizeof(linkitem);
    }
    secXHDR.addr += secXHDR.offset;
    unsigned foff1 = (PAGE_MASK & (~PAGE_MASK + segTEXT.filesize));
    if (foff1 < segTEXT.vmsize)
        foff1 += PAGE_SIZE;  // codesign disallows overhang
    segLINK.fileoff = foff1;
    segLINK.vmaddr = segTEXT.vmaddr + foff1;
    fo->seek(foff1 - 1, SEEK_SET); fo->write("", 1);
    fo->seek(sizeof(mhdro), SEEK_SET);
    fo->rewrite(&segZERO, sizeof(segZERO));
    fo->rewrite(&segXHDR, sizeof(segXHDR));
    fo->rewrite(&secXHDR, sizeof(secXHDR));
    fo->rewrite(&segTEXT, sizeof(segTEXT));
    fo->rewrite(&secTEXT, sizeof(secTEXT));
    fo->rewrite(&segLINK, sizeof(segLINK));
    fo->rewrite(&threado, sizeof(threado));
    if (my_filetype==Mach_header::MH_EXECUTE) {
        fo->rewrite(&uuid_cmd, sizeof(uuid_cmd));
        fo->rewrite(&linkitem, sizeof(linkitem));
    }
    fo->rewrite(&linfo, sizeof(linfo));
}

void PackMachARMEL::pack4(OutputFile *fo, Filter &ft)  // append PackHeader
{
    // offset of p_info in compressed file
    overlay_offset = sizeof(mhdro) + sizeof(segZERO)
        + sizeof(segXHDR) + sizeof(secXHDR)
        + sizeof(segTEXT) + sizeof(secTEXT)
        + sizeof(segLINK) + sizeof(threado) + sizeof(linfo);
    if (my_filetype==Mach_header::MH_EXECUTE) {
        overlay_offset += sizeof(uuid_cmd) + sizeof(linkitem);
    }

    super::pack4(fo, ft);
    unsigned const t = fo->getBytesWritten();
    segTEXT.filesize = t;
    segTEXT.vmsize  += t;  // utilize GAP + NO_LAP + sz_unc - sz_cpr
    secTEXT.offset = overlay_offset - sizeof(linfo);
    secTEXT.addr = segTEXT.vmaddr + secTEXT.offset;
    secTEXT.size = segTEXT.vmsize - secTEXT.offset;
    secXHDR.offset = overlay_offset - sizeof(linfo);
    if (my_filetype==Mach_header::MH_EXECUTE) {
        secXHDR.offset -= sizeof(uuid_cmd) + sizeof(linkitem);
    }
    secXHDR.addr += secXHDR.offset;
    unsigned foff1 = (PAGE_MASK & (~PAGE_MASK + segTEXT.filesize));
    if (foff1 < segTEXT.vmsize)
        foff1 += PAGE_SIZE;  // codesign disallows overhang
    segLINK.fileoff = foff1;
    segLINK.vmaddr = segTEXT.vmaddr + foff1;
    fo->seek(foff1 - 1, SEEK_SET); fo->write("", 1);
    fo->seek(sizeof(mhdro), SEEK_SET);
    fo->rewrite(&segZERO, sizeof(segZERO));
    fo->rewrite(&segXHDR, sizeof(segXHDR));
    fo->rewrite(&secXHDR, sizeof(secXHDR));
    fo->rewrite(&segTEXT, sizeof(segTEXT));
    fo->rewrite(&secTEXT, sizeof(secTEXT));
    fo->rewrite(&segLINK, sizeof(segLINK));
    fo->rewrite(&threado, sizeof(threado));
    if (my_filetype==Mach_header::MH_EXECUTE) {
        fo->rewrite(&uuid_cmd, sizeof(uuid_cmd));
        fo->rewrite(&linkitem, sizeof(linkitem));
    }
    fo->rewrite(&linfo, sizeof(linfo));
}

template <class T>
void PackMachBase<T>::pack4dylib(  // append PackHeader
    OutputFile *const fo,
    Filter &ft,
    Addr init_address
)
{
    unsigned opos = sizeof(mhdro);
    fo->seek(opos, SEEK_SET);

    // Append each non-__TEXT segment, page aligned.
    int slide = 0;
    unsigned o_end_txt = 0;
    unsigned hdrpos = sizeof(mhdro);
    Mach_segment_command const *seg = rawmseg;
    Mach_segment_command const *const endseg =
        (Mach_segment_command const *)(mhdri.sizeofcmds + (char const *)seg);
    for ( ; seg < endseg; seg = (Mach_segment_command const *)(
            seg->cmdsize + (char const *)seg )
    ) switch (~Mach_segment_command::LC_REQ_DYLD & seg->cmd) {
    default:  // unknown if any file offset field must slide
        printf("Unrecognized Macho cmd  offset=0x%lx  cmd=0x%lx  size=0x%lx\n", (unsigned long)((const char *)seg - (const char *)rawmseg), (unsigned long)seg->cmd, (unsigned long)seg->cmdsize);
        // fall through
    case Mach_segment_command::LC_THREAD:
    case Mach_segment_command::LC_UNIXTHREAD:
    case Mach_segment_command::LC_LOAD_DYLIB:
    case Mach_segment_command::LC_ID_DYLIB:
    case Mach_segment_command::LC_LOAD_DYLINKER:
    case Mach_segment_command::LC_UUID:
    case Mach_segment_command::LC_RPATH:
    case Mach_segment_command::LC_CODE_SIGNATURE:
    case Mach_segment_command::LC_REEXPORT_DYLIB:
        hdrpos += seg->cmdsize;
        break;  // contain no file offset fields
    case Mach_segment_command::LC_TWOLEVEL_HINTS: {
        Mach_twolevel_hints_command cmd; memcpy(&cmd, seg, sizeof(cmd));
        if (o_end_txt <= cmd.offset) { cmd.offset += slide; }
        fo->seek(hdrpos, SEEK_SET);
        fo->rewrite(&cmd, sizeof(cmd));
        hdrpos += sizeof(cmd);
    } break;
    case Mach_segment_command::LC_ROUTINES_64:
    case Mach_segment_command::LC_ROUTINES: {
        Mach_routines_command cmd; memcpy(&cmd, seg, sizeof(cmd));
        cmd.reserved1 = cmd.init_address;
        cmd.init_address = init_address;
        fo->seek(hdrpos, SEEK_SET);
        fo->rewrite(&cmd, sizeof(cmd));
        hdrpos += sizeof(cmd);
    } break;
    case Mach_segment_command::LC_SEGMENT_64:
    case Mach_segment_command::LC_SEGMENT: {
        // non-__TEXT might be observed and relocated by dyld before us.
        Mach_segment_command segcmdtmp = *seg;
        bool const is_text = 0==strncmp(&seg->segname[0], "__TEXT", 1+ 6);
        {
            if (is_text) {
                slide = 0;
                segcmdtmp.filesize = fo->getBytesWritten();
                segcmdtmp.maxprot  |= Mach_segment_command::VM_PROT_WRITE;
                segcmdtmp.initprot |= Mach_segment_command::VM_PROT_WRITE;
                opos = o_end_txt = segcmdtmp.filesize + segcmdtmp.fileoff;
            }
            else {
                opos += ~PAGE_MASK & (0u - opos);  // advance to PAGE_SIZE boundary
                slide = opos - segcmdtmp.fileoff;
                segcmdtmp.fileoff = opos;
            }

            fo->seek(hdrpos, SEEK_SET);
            fo->rewrite(&segcmdtmp, sizeof(segcmdtmp));
            hdrpos += sizeof(segcmdtmp);

            // Update the sections.
            Mach_section_command const *secp = (Mach_section_command const *)(const void*)(const char*)(1+ seg);
            unsigned const nsects = segcmdtmp.nsects;
            Mach_section_command seccmdtmp;
            for (unsigned j = 0; j < nsects; ++secp, ++j) {
                seccmdtmp = *secp;
                if (o_end_txt <= seccmdtmp.offset) { seccmdtmp.offset += slide; }
                if (o_end_txt <= seccmdtmp.reloff) { seccmdtmp.reloff += slide; }
                if (0==strncmp(&seccmdtmp.sectname[0], "__mod_init_func", 1+ 15)) {
                    if (seccmdtmp.flags==9  // FIXME: S_MOD_INIT_FUNC_POINTERS
                    &&  seccmdtmp.nreloc==0 && seccmdtmp.size==sizeof(Addr) ) {
                        seccmdtmp.addr = seccmdtmp.offset = init_address -4*4 - 8;
                    }
                    else
                        infoWarning("unknown __mod_init_func section");
                }
#if 0  /*{*/
// 2010-03-12  Stop work because I don't understand what is going on,
// and I cannot find good documentation on the meaning of various parts
// of .dylib.  amd64(x86_64) is almost certain to fail in the dynamic
// loader, before the upx stub gets control.  For instance:
//
// Program received signal EXC_BAD_ACCESS, Could not access memory.
// Reason: KERN_INVALID_ADDRESS at address: 0x000000015f00329a
// 0x00007fff5fc10ce3 in __dyld__ZN16ImageLoaderMachO23setupLazyPointerHandlerERKN11ImageLoader11LinkContextE ()
// (gdb) bt
// #0  0x00007fff5fc10ce3 in __dyld__ZN16ImageLoaderMachO23setupLazyPointerHandlerERKN11ImageLoader11LinkContextE ()
// #1  0x00007fff5fc138c2 in __dyld__ZN16ImageLoaderMachO6doBindERKN11ImageLoader11LinkContextEb ()
// #2  0x00007fff5fc0c0ab in __dyld__ZN11ImageLoader13recursiveBindERKNS_11LinkContextEb ()
// #3  0x00007fff5fc0c08f in __dyld__ZN11ImageLoader13recursiveBindERKNS_11LinkContextEb ()
// #4  0x00007fff5fc0f49e in __dyld__ZN11ImageLoader4linkERKNS_11LinkContextEbbRKNS_10RPathChainE ()
// #5  0x00007fff5fc04c56 in __dyld__ZN4dyld4linkEP11ImageLoaderbRKNS0_10RPathChainE ()
// #6  0x00007fff5fc06e04 in __dyld__ZN4dyld5_mainEPK11mach_headermiPPKcS5_S5_ ()
// #7  0x00007fff5fc01695 in __dyld__ZN13dyldbootstrap5startEPK11mach_headeriPPKcl ()
// #8  0x00007fff5fc0103a in __dyld__dyld_start ()
// #9  0x0000000100000000 in ?? ()
// #10 0x0000000000000001 in ?? ()
// #11 0x00007fff5fbffbd0 in ?? ()
//
// The various paragraphs below are experiments in "commenting out" pieces of
// the compressed .dylib, trying to isolate the bug(s).
                // FIXME
                unsigned const t = seccmdtmp.flags & 0xff;
                if (t==6  // FIXME: S_NON_LAZY_SYMBOL_POINTERS
                ||  t==7  // FIXME: S_LAZY_SYMBOL_POINTERS
                ||  t==8  // FIXME: S_SYMBOL_STUBS
                ||  t==11 // FIXME: S_COALESCED
                ) {
                    seccmdtmp.flags = 0;  // FIXME: S_REGULAR
                    strcpy(seccmdtmp.sectname, "__data");
                }
                // FIXME
                if (0==strncmp("__stub_helper", &seccmdtmp.sectname[0], 1+ 13)) {
                    strcpy(seccmdtmp.sectname, "__text");
                }
                // FIXME
                if (0==strncmp("__dyld", &seccmdtmp.sectname[0], 1+ 6)) {
                    strcpy(seccmdtmp.sectname, "__text");
                }
#endif  /*}*/
                fo->rewrite(&seccmdtmp, sizeof(seccmdtmp));
                hdrpos += sizeof(seccmdtmp);
            }

            if (!is_text) {
                fo->seek(opos, SEEK_SET);
                fi->seek(seg->fileoff, SEEK_SET);
                unsigned const len = seg->filesize;
                MemBuffer data(len);
                fi->readx(data, len);
                fo->write(data, len);
                opos += len;
            }
        }
    } break;
    case Mach_segment_command::LC_SYMTAB: {
        Mach_symtab_command cmd; memcpy(&cmd, seg, sizeof(cmd));
        if (o_end_txt <= cmd.symoff) { cmd.symoff += slide; }
        if (o_end_txt <= cmd.stroff) { cmd.stroff += slide; }
        fo->seek(hdrpos, SEEK_SET);
        fo->rewrite(&cmd, sizeof(cmd));
        hdrpos += sizeof(cmd);
    } break;
    case Mach_segment_command::LC_DYSYMTAB: {
        Mach_dysymtab_command cmd; memcpy(&cmd, seg, sizeof(cmd));
        if (o_end_txt <= cmd.tocoff)         { cmd.tocoff         += slide; }
        if (o_end_txt <= cmd.modtaboff)      { cmd.modtaboff      += slide; }
        if (o_end_txt <= cmd.extrefsymoff)   { cmd.extrefsymoff   += slide; }
        if (o_end_txt <= cmd.indirectsymoff) { cmd.indirectsymoff += slide; }
        if (o_end_txt <= cmd.extreloff)      { cmd.extreloff      += slide; }
        if (o_end_txt <= cmd.locreloff)      { cmd.locreloff      += slide; }
        fo->seek(hdrpos, SEEK_SET);
        fo->rewrite(&cmd, sizeof(cmd));
        hdrpos += sizeof(cmd);
    } break;
    case Mach_segment_command::LC_SEGMENT_SPLIT_INFO: {
        Mach_segsplit_info_command cmd; memcpy(&cmd, seg, sizeof(cmd));
        if (o_end_txt <= cmd.dataoff) { cmd.dataoff += slide; }
        fo->seek(hdrpos, SEEK_SET);
        fo->rewrite(&cmd, sizeof(cmd));
        hdrpos += sizeof(cmd);
    } break;
    }  // end 'switch'
    fo->seek(opos, SEEK_SET);  // BUG: "fo->seek(0, SEEK_END);" is broken

    // offset of p_info in compressed file
    overlay_offset = sizeof(mhdro) + mhdro.sizeofcmds + sizeof(linfo);
    PackMachBase<T>::pack4(fo, ft);
}

void PackDylibI386::pack4(OutputFile *fo, Filter &ft)  // append PackHeader
{
    pack4dylib(fo, ft, threado.state.eip);
}

void PackDylibAMD64::pack4(OutputFile *fo, Filter &ft)  // append PackHeader
{
    pack4dylib(fo, ft, threado.state.rip);
}

void PackDylibPPC32::pack4(OutputFile *fo, Filter &ft)  // append PackHeader
{
    pack4dylib(fo, ft, threado.state.srr0);
}

void PackMachPPC32::pack3(OutputFile *fo, Filter &ft)  // append loader
{
    TE32 disp;
    unsigned const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 3& (0u-len));
    len += (3& (0u-len)) + sizeof(disp);
    disp = 4+ len - sz_mach_headers;  // 4: sizeof(instruction)
    fo->write(&disp, sizeof(disp));

    threado.state.srr0 = len + segTEXT.vmaddr;  /* entry address */
    super::pack3(fo, ft);
}

void PackMachI386::pack3(OutputFile *fo, Filter &ft)  // append loader
{
    TE32 disp;
    unsigned const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 3& (0u-len));
    len += (3& (0u-len));
    disp = len - sz_mach_headers;
    fo->write(&disp, sizeof(disp));

    threado.state.eip = len + sizeof(disp) + segTEXT.vmaddr;  /* entry address */
    super::pack3(fo, ft);
}

void PackMachAMD64::pack3(OutputFile *fo, Filter &ft)  // append loader
{
    TE32 disp;
    unsigned const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 3& (0u-len));
    len += (3& (0u-len));
    disp = len - sz_mach_headers;
    fo->write(&disp, sizeof(disp));

    threado.state.rip = len + sizeof(disp) + segTEXT.vmaddr;  /* entry address */
    super::pack3(fo, ft);
}

void PackMachARMEL::pack3(OutputFile *fo, Filter &ft)  // append loader
{
    TE32 disp;
    unsigned const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 3& (0u-len));
    len += (3& (0u-len));
    disp = len - sz_mach_headers;
    fo->write(&disp, sizeof(disp));

    threado.state.pc = len + sizeof(disp) + segTEXT.vmaddr;  /* entry address */
    super::pack3(fo, ft);
}

void PackDylibI386::pack3(OutputFile *fo, Filter &ft)  // append loader
{
    TE32 disp;
    unsigned const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 3& (0u-len));
    len += (3& (0u-len)) + 4*sizeof(disp);

    disp = prev_init_address;
    fo->write(&disp, sizeof(disp));  // user .init_address

    disp = sizeof(mhdro) + mhdro.sizeofcmds + sizeof(l_info) + sizeof(p_info);
    fo->write(&disp, sizeof(disp));  // src offset(compressed __TEXT)

    disp = len - disp - 3*sizeof(disp);
    fo->write(&disp, sizeof(disp));  // length(compressed __TEXT)

    unsigned const save_sz_mach_headers(sz_mach_headers);
    sz_mach_headers = 0;
    super::pack3(fo, ft);
    sz_mach_headers = save_sz_mach_headers;
}

void PackDylibAMD64::pack3(OutputFile *fo, Filter &ft)  // append loader
{
    TE32 disp;
    TE64 disp64;
    unsigned const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 7& (0u-len));
    len += (7& (0u-len)) + sizeof(disp64) + 4*sizeof(disp);

    disp64= len;
    fo->write(&disp64, sizeof(disp64));  // __mod_init_func

    disp = prev_init_address;
    fo->write(&disp, sizeof(disp));  // user .init_address

    disp = sizeof(mhdro) + mhdro.sizeofcmds + sizeof(l_info) + sizeof(p_info);
    fo->write(&disp, sizeof(disp));  // src offset(compressed __TEXT)

    disp = len - disp - 3*sizeof(disp);
    fo->write(&disp, sizeof(disp));  // length(compressed __TEXT)

    unsigned const save_sz_mach_headers(sz_mach_headers);
    sz_mach_headers = 0;
    super::pack3(fo, ft);
    sz_mach_headers = save_sz_mach_headers;
}

void PackDylibPPC32::pack3(OutputFile *fo, Filter &ft)  // append loader
{
    TE32 disp;
    unsigned const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 3& (0u-len));
    len += (3& (0u-len)) + 4*sizeof(disp);

    disp = prev_init_address;
    fo->write(&disp, sizeof(disp));  // user .init_address

    disp = sizeof(mhdro) + mhdro.sizeofcmds + sizeof(l_info) + sizeof(p_info);
    fo->write(&disp, sizeof(disp));  // src offset(compressed __TEXT)

    disp = len - disp - 3*sizeof(disp);
    fo->write(&disp, sizeof(disp));  // length(compressed __TEXT)

    unsigned const save_sz_mach_headers(sz_mach_headers);
    sz_mach_headers = 0;
    super::pack3(fo, ft);
    sz_mach_headers = save_sz_mach_headers;
}

// Determine length of gap between PT_LOAD phdri[k] and closest PT_LOAD
// which follows in the file (or end-of-file).  Optimize for common case
// where the PT_LOAD are adjacent ascending by .p_offset.  Assume no overlap.

template <class T>
unsigned PackMachBase<T>::find_SEGMENT_gap(
    unsigned const k
)
{
    unsigned const lc_seg = lc_segment[sizeof(Addr)>>3];
    if (lc_seg!=msegcmd[k].cmd
    ||  0==msegcmd[k].filesize ) {
        return 0;
    }
    unsigned const hi = msegcmd[k].fileoff + msegcmd[k].filesize;
    unsigned lo = ph.u_file_size;
    if (lo < hi)
        throwCantPack("bad input: LC_SEGMENT beyond end-of-file");
    unsigned j = k;
    for (;;) { // circular search, optimize for adjacent ascending
        ++j;
        if (n_segment==j) {
            j = 0;
        }
        if (k==j) {
            break;
        }
        if (lc_seg==msegcmd[j].cmd
        &&  0!=msegcmd[j].filesize ) {
            unsigned const t = (unsigned) msegcmd[j].fileoff;
            if ((t - hi) < (lo - hi)) {
                lo = t;
                if (hi==lo) {
                    break;
                }
            }
        }
    }
    return lo - hi;
}

template <class T>
void PackMachBase<T>::pack4(OutputFile *fo, Filter &ft)
{
    PackUnix::pack4(fo, ft);  // FIXME  super() does not work?
}

template <class T>
void PackMachBase<T>::pack3(OutputFile *fo, Filter &ft)
{
    PackUnix::pack3(fo, ft);  // FIXME  super() does not work?
}

template <class T>
int  PackMachBase<T>::pack2(OutputFile *fo, Filter &ft)  // append compressed body
{
    unsigned const lc_seg = lc_segment[sizeof(Addr)>>3];
    Extent x;
    unsigned k;

    // count passes, set ptload vars
    uip->ui_total_passes = 0;
    for (k = 0; k < n_segment; ++k) {
        if (lc_seg==msegcmd[k].cmd
        &&  0!=msegcmd[k].filesize ) {
            uip->ui_total_passes++;
            if (my_filetype==Mach_header::MH_DYLIB) {
                break;
            }
            if (find_SEGMENT_gap(k)) {
                uip->ui_total_passes++;
            }
        }
    }

    // compress extents
    unsigned total_in = 0;
    unsigned total_out = 0;

    unsigned hdr_u_len = mhdri.sizeofcmds + sizeof(mhdri);

    uip->ui_pass = 0;
    ft.addvalue = 0;

    // Packer::compressWithFilters chooses a filter for us, and the stubs
    // can handle only one filter, and most filters are for executable
    // instructions.  So filter only the largest executable segment.
    unsigned exe_filesize_max = 0;
    for (k = 0; k < n_segment; ++k)
    if (lc_seg==msegcmd[k].cmd
    &&  0!=(Mach_segment_command::VM_PROT_EXECUTE & msegcmd[k].initprot)
    &&  exe_filesize_max < msegcmd[k].filesize) {
        exe_filesize_max = (unsigned) msegcmd[k].filesize;
    }

    int nx = 0;
    for (k = 0; k < n_segment; ++k)
    if (lc_seg==msegcmd[k].cmd
    &&  0!=msegcmd[k].filesize ) {
        x.offset = msegcmd[k].fileoff;
        x.size   = msegcmd[k].filesize;
        if (0 == nx) { // 1st LC_SEGMENT must cover Mach_header at 0==fileoffset
            unsigned const delta = mhdri.sizeofcmds + sizeof(mhdri);
            x.offset    += delta;
            x.size      -= delta;
        }
        bool const do_filter = (msegcmd[k].filesize==exe_filesize_max)
            && 0!=(Mach_segment_command::VM_PROT_EXECUTE & msegcmd[k].initprot);
        packExtent(x, total_in, total_out,
            (do_filter ? &ft : 0 ), fo, hdr_u_len );
        if (do_filter) {
            exe_filesize_max = 0;
        }
        hdr_u_len = 0;
        ++nx;
        if (my_filetype==Mach_header::MH_DYLIB) {
            break;
        }
    }
    if (my_filetype!=Mach_header::MH_DYLIB)
    for (k = 0; k < n_segment; ++k) {
        x.size = find_SEGMENT_gap(k);
        if (x.size) {
            x.offset = msegcmd[k].fileoff +msegcmd[k].filesize;
            packExtent(x, total_in, total_out, 0, fo);
        }
    }

    if (my_filetype!=Mach_header::MH_DYLIB)
    if ((off_t)total_in != file_size)
        throwEOFException();
    segTEXT.filesize = fo->getBytesWritten();
    secTEXT.size = segTEXT.filesize - overlay_offset + sizeof(linfo);

    return 1;
}

void PackMachPPC32::pack1_setup_threado(OutputFile *const fo)
{
    threado.cmd = Mach_segment_command::LC_UNIXTHREAD;
    threado.cmdsize = sizeof(threado);
    threado.flavor = my_thread_flavor;
    threado.count =  my_thread_state_word_count;
    memset(&threado.state, 0, sizeof(threado.state));
    fo->write(&threado, sizeof(threado));
}

void PackMachI386::pack1_setup_threado(OutputFile *const fo)
{
    threado.cmd = Mach_segment_command::LC_UNIXTHREAD;
    threado.cmdsize = sizeof(threado);
    threado.flavor = my_thread_flavor;
    threado.count =  my_thread_state_word_count;
    memset(&threado.state, 0, sizeof(threado.state));
    fo->write(&threado, sizeof(threado));
}

void PackMachAMD64::pack1_setup_threado(OutputFile *const fo)
{
    threado.cmd = Mach_segment_command::LC_UNIXTHREAD;
    threado.cmdsize = sizeof(threado);
    threado.flavor = my_thread_flavor;
    threado.count =  my_thread_state_word_count;
    memset(&threado.state, 0, sizeof(threado.state));
    fo->write(&threado, sizeof(threado));
}

void PackMachARMEL::pack1_setup_threado(OutputFile *const fo)
{
    threado.cmd = Mach_segment_command::LC_UNIXTHREAD;
    threado.cmdsize = sizeof(threado);
    threado.flavor = my_thread_flavor;
    threado.count =  my_thread_state_word_count;
    memset(&threado.state, 0, sizeof(threado.state));
    fo->write(&threado, sizeof(threado));
}

template <class T>
void PackMachBase<T>::pack1(OutputFile *const fo, Filter &/*ft*/)  // generate executable header
{
    unsigned const lc_seg = lc_segment[sizeof(Addr)>>3];
    mhdro = mhdri;
    if (my_filetype==Mach_header::MH_EXECUTE) {
        mhdro.ncmds = 5;  // __ZERO, __XHDR, __TEXT, __LINKEDIT, THREAD_STATE
        mhdro.sizeofcmds = sizeof(segZERO)
            + sizeof(segXHDR) + sizeof(secXHDR)
            + sizeof(segTEXT) + sizeof(secTEXT) + sizeof(segLINK) +
            my_thread_command_size /* + sizeof(uuid_cmd) + sizeof(linkitem) */ ;
        mhdro.flags = Mach_header::MH_NOUNDEFS | Mach_header::MH_DYLDLINK;
    }
    fo->write(&mhdro, sizeof(mhdro));

    memset(&segZERO, 0, sizeof(segZERO));
    segZERO.cmd = lc_seg;
    segZERO.cmdsize = sizeof(segZERO);
    strncpy((char *)segZERO.segname, "__PAGEZERO", sizeof(segZERO.segname));
    segZERO.vmsize = PAGE_SIZE;

    segTEXT.cmd = lc_seg;
    segTEXT.cmdsize = sizeof(segTEXT) + sizeof(secTEXT);
    strncpy((char *)segTEXT.segname, "__TEXT", sizeof(segTEXT.segname));
    if (my_filetype==Mach_header::MH_EXECUTE) {
        int k;  // must ignore zero-length segments, which sort last
        for (k=n_segment; --k>=0; )
            if (msegcmd[k].vmsize!=0)
                break;
        segTEXT.vmaddr = PAGE_MASK64 & (~PAGE_MASK64 +
            msegcmd[k].vmsize + msegcmd[k].vmaddr );
    }
    if (my_filetype==Mach_header::MH_DYLIB) {
        segTEXT.vmaddr = 0;
    }
    segTEXT.vmsize = 0;    // adjust later
    segTEXT.fileoff = 0;
    segTEXT.filesize = 0;  // adjust later
    segTEXT.initprot = segTEXT.maxprot =
        Mach_segment_command::VM_PROT_READ |
        Mach_segment_command::VM_PROT_WRITE |
        Mach_segment_command::VM_PROT_EXECUTE;
    segTEXT.nsects = 1;  // secTEXT
    segTEXT.flags = 0;

    memset(&secTEXT, 0, sizeof(secTEXT));
    strncpy((char *)secTEXT.sectname, "__text", sizeof(secTEXT.sectname));
    memcpy(secTEXT.segname, segTEXT.segname, sizeof(secTEXT.segname));
    secTEXT.align = 2;  // (1<<2) ==> 4

    segXHDR = segTEXT;
    segXHDR.vmaddr = PAGE_SIZE;
    segXHDR.vmsize = PAGE_SIZE;
    segXHDR.filesize = PAGE_SIZE;
    strncpy((char *)segXHDR.segname,  "__XHDR", sizeof(segXHDR.segname));

    memset(&secXHDR, 0, sizeof(secXHDR));
    strncpy((char *)secXHDR.sectname, "__xhdr", sizeof(secXHDR.sectname));
    memcpy(secXHDR.segname,  segXHDR.segname, sizeof(secXHDR.segname));
    secXHDR.addr = PAGE_SIZE;
    secXHDR.size = 0;  // empty so far
    secXHDR.align = 2;  // (1<<2) ==> 4

    segLINK = segTEXT;
    segLINK.cmdsize = sizeof(segLINK);
    strncpy((char *)segLINK.segname, "__LINKEDIT", sizeof(segLINK.segname));
    segLINK.nsects = 0;
    segLINK.initprot = Mach_segment_command::VM_PROT_READ;
    // Adjust later: .vmaddr .vmsize .fileoff .filesize

    if (my_filetype==Mach_header::MH_EXECUTE) {
        fo->write(&segZERO, sizeof(segZERO));
        fo->write(&segXHDR, sizeof(segXHDR));
        fo->write(&secXHDR, sizeof(secXHDR));
        fo->write(&segTEXT, sizeof(segTEXT));
        fo->write(&secTEXT, sizeof(secTEXT));
        fo->write(&segLINK, sizeof(segLINK));
        pack1_setup_threado(fo);
        memset(&uuid_cmd, 0, sizeof(uuid_cmd));
        fo->write(&uuid_cmd, sizeof(uuid_cmd));
        memset(&linkitem, 0, sizeof(linkitem));
        fo->write(&linkitem, sizeof(linkitem));
    }
    if (my_filetype==Mach_header::MH_DYLIB) {
        fo->write(rawmseg, mhdri.sizeofcmds);
    }
    sz_mach_headers = fo->getBytesWritten();

    memset((char *)&linfo, 0, sizeof(linfo));
    fo->write(&linfo, sizeof(linfo));

    return;
}

template <class T>
void PackMachBase<T>::unpack(OutputFile *fo)
{
    unsigned const lc_seg = lc_segment[sizeof(Addr)>>3];
    fi->seek(0, SEEK_SET);
    fi->readx(&mhdri, sizeof(mhdri));
    rawmseg = (Mach_segment_command *)new char[(unsigned) mhdri.sizeofcmds];
    fi->readx(rawmseg, mhdri.sizeofcmds);

    // FIXME forgot space left for LC_CODE_SIGNATURE;
    // but PackUnix::canUnpack() sets overlay_offset anyway.
    //overlay_offset = sizeof(mhdri) + mhdri.sizeofcmds + sizeof(linfo);

    fi->seek(overlay_offset, SEEK_SET);
    p_info hbuf;
    fi->readx(&hbuf, sizeof(hbuf));
    unsigned orig_file_size = get_te32(&hbuf.p_filesize);
    blocksize = get_te32(&hbuf.p_blocksize);
    if (file_size > (off_t)orig_file_size || blocksize > orig_file_size)
        throwCantUnpack("file header corrupted");

    ibuf.alloc(blocksize + OVERHEAD);
    b_info bhdr; memset(&bhdr, 0, sizeof(bhdr));
    fi->readx(&bhdr, sizeof(bhdr));
    ph.u_len = get_te32(&bhdr.sz_unc);
    ph.c_len = get_te32(&bhdr.sz_cpr);
    ph.method = bhdr.b_method;
    ph.filter = bhdr.b_ftid;
    ph.filter_cto = bhdr.b_cto8;

    // Uncompress Macho headers
    fi->readx(ibuf, ph.c_len);
    Mach_header *const mhdr = (Mach_header *)new upx_byte[ph.u_len];
    decompress(ibuf, (upx_byte *)mhdr, false);
    unsigned const ncmds = mhdr->ncmds;

    msegcmd = new Mach_segment_command[ncmds];
    unsigned char *ptr = (unsigned char *)(1+mhdr);
    for (unsigned j= 0; j < ncmds; ++j) {
        msegcmd[j] = *(Mach_segment_command *)ptr;
        ptr += (unsigned) ((Mach_segment_command *)ptr)->cmdsize;
    }

    // Put LC_SEGMENT together at the beginning, ascending by .vmaddr.
    qsort(msegcmd, ncmds, sizeof(*msegcmd), compare_segment_command);
    n_segment = 0;
    for (unsigned j= 0; j < ncmds; ++j) {
        n_segment += (lc_seg==msegcmd[j].cmd);
    }

    unsigned total_in = 0;
    unsigned total_out = 0;
    unsigned c_adler = upx_adler32(NULL, 0);
    unsigned u_adler = upx_adler32(NULL, 0);
    Mach_segment_command const *sc = (Mach_segment_command const *)(void *)(1+ mhdr);
    unsigned k;

    fi->seek(- (off_t)(sizeof(bhdr) + ph.c_len), SEEK_CUR);
    for (
        k = 0;
        k < ncmds;
        (++k), (sc = (Mach_segment_command const *)(sc->cmdsize + (char const *)sc))
    ) {
        if (lc_seg==sc->cmd
        &&  0!=sc->filesize ) {
            if (fo)
                fo->seek(sc->fileoff, SEEK_SET);
            unsigned filesize = sc->filesize;
            unpackExtent(filesize, fo, total_in, total_out,
                c_adler, u_adler, false, sizeof(bhdr));
            if (my_filetype==Mach_header::MH_DYLIB) {
                break;
            }
        }
    }
    if (my_filetype==Mach_header::MH_DYLIB) {
        Mach_segment_command const *rc = rawmseg;
        rc = (Mach_segment_command const *)(rc->cmdsize + (char const *)rc);
        sc = (Mach_segment_command const *)(sc->cmdsize + (char const *)sc);
        for (
            k=1;
            k < ncmds;
            (++k), (sc = (Mach_segment_command const *)(sc->cmdsize + (char const *)sc)),
                   (rc = (Mach_segment_command const *)(rc->cmdsize + (char const *)rc))
        ) {
            if (lc_seg==rc->cmd
            &&  0!=rc->filesize ) {
                fi->seek(rc->fileoff, SEEK_SET);
                if (fo)
                    fo->seek(sc->fileoff, SEEK_SET);
                unsigned const len = rc->filesize;
                MemBuffer data(len);
                fi->readx(data, len);
                if (fo)
                    fo->write(data, len);
            }
        }
    }
    else
    for (unsigned j = 0; j < ncmds; ++j) {
        unsigned const size = find_SEGMENT_gap(j);
        if (size) {
            unsigned const where = msegcmd[j].fileoff +msegcmd[j].filesize;
            if (fo)
                fo->seek(where, SEEK_SET);
            unpackExtent(size, fo, total_in, total_out,
                c_adler, u_adler, false, sizeof(bhdr));
        }
    }
    delete [] mhdr;
}


template <class T>
bool PackMachBase<T>::canPack()
{
    unsigned const lc_seg = lc_segment[sizeof(Addr)>>3];
    unsigned const lc_rout = lc_routines[sizeof(Addr)>>3];
    fi->seek(0, SEEK_SET);
    fi->readx(&mhdri, sizeof(mhdri));

    if (((unsigned) Mach_header::MH_MAGIC + (sizeof(Addr)>>3)) !=mhdri.magic
    ||  my_cputype   !=mhdri.cputype
    ||  my_filetype  !=mhdri.filetype
    )
        return false;

    rawmseg = (Mach_segment_command *)new char[(unsigned) mhdri.sizeofcmds];
    fi->readx(rawmseg, mhdri.sizeofcmds);

    unsigned const ncmds = mhdri.ncmds;
    msegcmd = new Mach_segment_command[ncmds];
    unsigned char const *ptr = (unsigned char const *)rawmseg;
    for (unsigned j= 0; j < ncmds; ++j) {
        msegcmd[j] = *(Mach_segment_command const *)ptr;
        if (((Mach_segment_command const *)ptr)->cmd == lc_rout) {
            o_routines_cmd = (const char *)ptr - (const char *)rawmseg;
            prev_init_address =
                ((Mach_routines_command const *)ptr)->init_address;
        }
        ptr += (unsigned) ((const Mach_segment_command *)ptr)->cmdsize;
    }
    if (Mach_header::MH_DYLIB==my_filetype && 0==o_routines_cmd) {
        infoWarning("missing -init function");
        return false;
    }

    // Put LC_SEGMENT together at the beginning, ascending by .vmaddr.
    qsort(msegcmd, ncmds, sizeof(*msegcmd), compare_segment_command);

    // Check alignment of non-null LC_SEGMENT.
    for (unsigned j= 0; j < ncmds; ++j) {
        if (lc_seg==msegcmd[j].cmd) {
            if (~PAGE_MASK & (msegcmd[j].fileoff | msegcmd[j].vmaddr)) {
                return false;
            }
            if (msegcmd[j].vmsize==0)
                break;  // was sorted last

            // We used to check that LC_SEGMENTS were contiguous,
            // but apparently that is not needed anymore,
            // and Google compilers generate strange layouts.

            ++n_segment;
            sz_segment = msegcmd[j].filesize + msegcmd[j].fileoff - msegcmd[0].fileoff;
        }
    }

    // info: currently the header is 36 (32+4) bytes before EOF
    unsigned char buf[256];
    fi->seek(-(off_t)sizeof(buf), SEEK_END);
    fi->readx(buf, sizeof(buf));
    checkAlreadyPacked(buf, sizeof(buf));

    // set options
    opt->o_unix.blocksize = file_size;
    return 0 < n_segment;
}

template class PackMachBase<MachClass_BE32>;
template class PackMachBase<MachClass_LE32>;
template class PackMachBase<MachClass_LE64>;


PackMachFat::PackMachFat(InputFile *f) : super(f)
{
    bele = &N_BELE_RTP::le_policy;  // sham
}

PackMachFat::~PackMachFat()
{
}

const int *PackMachFat::getCompressionMethods(int /*method*/, int /*level*/) const
{
    static const int m_nrv2e[] = { M_NRV2E_LE32, M_END };
    return m_nrv2e;  // sham
}

const int *PackMachFat::getFilters() const
{
    static const int filters[] = { 0x49, FT_END };
    return filters;  // sham
}

void PackMachFat::pack(OutputFile *fo)
{
    unsigned const in_size = this->file_size;
    fo->write(&fat_head, sizeof(fat_head.fat) +
        fat_head.fat.nfat_arch * sizeof(fat_head.arch[0]));
    unsigned length = 0;
    for (unsigned j=0; j < fat_head.fat.nfat_arch; ++j) {
        unsigned base = fo->unset_extent();  // actual length
        base += ~(~0u<<fat_head.arch[j].align) & (0-base);  // align up
        fo->seek(base, SEEK_SET);
        fo->set_extent(base, ~0u);

        ph.u_file_size = fat_head.arch[j].size;
        fi->set_extent(fat_head.arch[j].offset, fat_head.arch[j].size);
        fi->seek(0, SEEK_SET);
        switch (fat_head.arch[j].cputype) {
        case PackMachFat::CPU_TYPE_I386: {
            typedef N_Mach::Mach_header<MachClass_LE32::MachITypes> Mach_header;
            Mach_header hdr;
            fi->readx(&hdr, sizeof(hdr));
            if (hdr.filetype==Mach_header::MH_EXECUTE) {
                PackMachI386 packer(fi);
                packer.initPackHeader();
                packer.canPack();
                packer.updatePackHeader();
                packer.pack(fo);
            }
            else if (hdr.filetype==Mach_header::MH_DYLIB) {
                PackDylibI386 packer(fi);
                packer.initPackHeader();
                packer.canPack();
                packer.updatePackHeader();
                packer.pack(fo);
            }
        } break;
        case PackMachFat::CPU_TYPE_X86_64: {
            typedef N_Mach::Mach_header<MachClass_LE64::MachITypes> Mach_header;
            Mach_header hdr;
            fi->readx(&hdr, sizeof(hdr));
            if (hdr.filetype==Mach_header::MH_EXECUTE) {
                PackMachAMD64 packer(fi);
                packer.initPackHeader();
                packer.canPack();
                packer.updatePackHeader();
                packer.pack(fo);
            }
            else if (hdr.filetype==Mach_header::MH_DYLIB) {
                PackDylibAMD64 packer(fi);
                packer.initPackHeader();
                packer.canPack();
                packer.updatePackHeader();
                packer.pack(fo);
            }
        } break;
        case PackMachFat::CPU_TYPE_POWERPC: {
            typedef N_Mach::Mach_header<MachClass_BE32::MachITypes> Mach_header;
            Mach_header hdr;
            fi->readx(&hdr, sizeof(hdr));
            if (hdr.filetype==Mach_header::MH_EXECUTE) {
                PackMachPPC32 packer(fi);
                packer.initPackHeader();
                packer.canPack();
                packer.updatePackHeader();
                packer.pack(fo);
            }
            else if (hdr.filetype==Mach_header::MH_DYLIB) {
                PackDylibPPC32 packer(fi);
                packer.initPackHeader();
                packer.canPack();
                packer.updatePackHeader();
                packer.pack(fo);
            }
        } break;
        }  // switch cputype
        fat_head.arch[j].offset = base;
        length = fo->unset_extent();
        fat_head.arch[j].size = length - base;
    }
    ph.u_file_size = in_size;
    fi->set_extent(0, in_size);

    fo->seek(0, SEEK_SET);
    fo->rewrite(&fat_head, sizeof(fat_head.fat) +
        fat_head.fat.nfat_arch * sizeof(fat_head.arch[0]));
    fo->set_extent(0, length);
}

void PackMachFat::unpack(OutputFile *fo)
{
    if (fo) {  // test mode ("-t") sets fo = NULL
        fo->seek(0, SEEK_SET);
        fo->write(&fat_head, sizeof(fat_head.fat) +
            fat_head.fat.nfat_arch * sizeof(fat_head.arch[0]));
    }
    unsigned length;
    for (unsigned j=0; j < fat_head.fat.nfat_arch; ++j) {
        unsigned base = (fo ? fo->unset_extent() : 0);  // actual length
        base += ~(~0u<<fat_head.arch[j].align) & (0-base);  // align up
        if (fo) {
            fo->seek(base, SEEK_SET);
            fo->set_extent(base, ~0u);
        }

        ph.u_file_size = fat_head.arch[j].size;
        fi->set_extent(fat_head.arch[j].offset, fat_head.arch[j].size);
        fi->seek(0, SEEK_SET);
        switch (fat_head.arch[j].cputype) {
        case PackMachFat::CPU_TYPE_I386: {
            N_Mach::Mach_header<MachClass_LE32::MachITypes> hdr;
            typedef N_Mach::Mach_header<MachClass_LE32::MachITypes> Mach_header;
            fi->readx(&hdr, sizeof(hdr));
            if (hdr.filetype==Mach_header::MH_EXECUTE) {
                PackMachI386 packer(fi);
                packer.initPackHeader();
                packer.canUnpack();
                packer.unpack(fo);
            }
            else if (hdr.filetype==Mach_header::MH_DYLIB) {
                PackDylibI386 packer(fi);
                packer.initPackHeader();
                packer.canUnpack();
                packer.unpack(fo);
            }
        } break;
        case PackMachFat::CPU_TYPE_X86_64: {
            N_Mach::Mach_header<MachClass_LE64::MachITypes> hdr;
            typedef N_Mach::Mach_header<MachClass_LE64::MachITypes> Mach_header;
            fi->readx(&hdr, sizeof(hdr));
            if (hdr.filetype==Mach_header::MH_EXECUTE) {
                PackMachAMD64 packer(fi);
                packer.initPackHeader();
                packer.canUnpack();
                packer.unpack(fo);
            }
            else if (hdr.filetype==Mach_header::MH_DYLIB) {
                PackDylibAMD64 packer(fi);
                packer.initPackHeader();
                packer.canUnpack();
                packer.unpack(fo);
            }
        } break;
        case PackMachFat::CPU_TYPE_POWERPC: {
            N_Mach::Mach_header<MachClass_BE32::MachITypes> hdr;
            typedef N_Mach::Mach_header<MachClass_BE32::MachITypes> Mach_header;
            fi->readx(&hdr, sizeof(hdr));
            if (hdr.filetype==Mach_header::MH_EXECUTE) {
                PackMachPPC32 packer(fi);
                packer.initPackHeader();
                packer.canUnpack();
                packer.unpack(fo);
            }
            else if (hdr.filetype==Mach_header::MH_DYLIB) {
                PackDylibPPC32 packer(fi);
                packer.initPackHeader();
                packer.canUnpack();
                packer.unpack(fo);
            }
        } break;
        }  // switch cputype
        fat_head.arch[j].offset = base;
        length = (fo ? fo->unset_extent() : 0);
        fat_head.arch[j].size = length - base;
    }
    if (fo) {
        fo->unset_extent();
        fo->seek(0, SEEK_SET);
        fo->rewrite(&fat_head, sizeof(fat_head.fat) +
            fat_head.fat.nfat_arch * sizeof(fat_head.arch[0]));
    }
}

bool PackMachFat::canPack()
{
    struct Mach_fat_arch *arch = &fat_head.arch[0];

    fi->readx(&fat_head, sizeof(fat_head));
    if (Mach_fat_header::FAT_MAGIC!=fat_head.fat.magic
    ||  N_FAT_ARCH < fat_head.fat.nfat_arch) {
        return false;
    }
    for (unsigned j=0; j < fat_head.fat.nfat_arch; ++j) {
        fi->set_extent(fat_head.arch[j].offset, fat_head.arch[j].size);
        fi->seek(0, SEEK_SET);
        switch (arch[j].cputype) {
        default:
            infoWarning("unknown cputype 0x%x: %s",
                (unsigned)arch[j].cputype, fi->getName());
            return false;
        case PackMachFat::CPU_TYPE_I386: {
            PackMachI386 packer(fi);
            if (!packer.canPack()) {
                PackDylibI386 pack2r(fi);
                if (!pack2r.canPack())
                    return false;
            }
        } break;
        case PackMachFat::CPU_TYPE_X86_64: {
            PackMachAMD64 packer(fi);
            if (!packer.canPack()) {
                PackDylibI386 pack2r(fi);
                if (!pack2r.canPack())
                    return false;
            }
        } break;
        case PackMachFat::CPU_TYPE_POWERPC: {
            PackMachPPC32 packer(fi);
            if (!packer.canPack()) {
                PackDylibPPC32 pack2r(fi);
                if (!pack2r.canPack())
                    return false;
            }
        } break;
        }  // switch cputype
    }

    // info: currently the header is 36 (32+4) bytes before EOF
    unsigned char buf[256];
    fi->seek(-(off_t)sizeof(buf), SEEK_END);
    fi->readx(buf, sizeof(buf));
    checkAlreadyPacked(buf, sizeof(buf));

    return true;
}

int PackMachFat::canUnpack()
{
    struct Mach_fat_arch *arch = &fat_head.arch[0];

    fi->readx(&fat_head, sizeof(fat_head));
    if (Mach_fat_header::FAT_MAGIC!=fat_head.fat.magic
    ||  N_FAT_ARCH < fat_head.fat.nfat_arch) {
        return false;
    }
    for (unsigned j=0; j < fat_head.fat.nfat_arch; ++j) {
        fi->set_extent(fat_head.arch[j].offset, fat_head.arch[j].size);
        fi->seek(0, SEEK_SET);
        switch (arch[j].cputype) {
        default: return false;
        case PackMachFat::CPU_TYPE_I386: {
            PackMachI386 packer(fi);
            if (!packer.canUnpack()) {
                PackDylibI386 pack2r(fi);
                if (!pack2r.canUnpack())
                    return 0;
                else
                    ph.format = pack2r.getFormat(); // FIXME: copy entire PackHeader
            }
            else
                ph.format = packer.getFormat(); // FIXME: copy entire PackHeader
        } break;
        case PackMachFat::CPU_TYPE_X86_64: {
            PackMachAMD64 packer(fi);
            if (!packer.canUnpack()) {
                PackDylibAMD64 pack2r(fi);
                if (!pack2r.canUnpack())
                    return 0;
                else
                    ph.format = pack2r.getFormat(); // FIXME: copy entire PackHeader
            }
            else
                ph.format = packer.getFormat(); // FIXME: copy entire PackHeader
        } break;
        case PackMachFat::CPU_TYPE_POWERPC: {
            PackMachPPC32 packer(fi);
            if (!packer.canUnpack()) {
                PackDylibPPC32 pack2r(fi);
                if (!pack2r.canUnpack())
                    return 0;
                else
                    ph.format = pack2r.getFormat(); // FIXME: copy entire PackHeader
            }
            else
                ph.format = packer.getFormat(); // FIXME: copy entire PackHeader
        } break;
        }  // switch cputype
    }
    return 1;
}

void PackMachFat::buildLoader(const Filter * /*ft*/)
{
    assert(false);
}

Linker* PackMachFat::newLinker() const
{
    return new ElfLinkerX86;  // sham
}

void PackMachFat::list()
{
    assert(false);
}

/*
vi:ts=4:et
*/
