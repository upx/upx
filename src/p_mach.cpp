/* p_mach.cpp -- pack Mach Object executable

   This file is part of the UPX executable compressor.

   Copyright (C) 2004-2017 John Reiser
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
#include "stub/i386-darwin.macho-entry.h"
static const
#include "stub/i386-darwin.macho-fold.h"
static const
#include "stub/i386-darwin.macho-upxmain.h"
static const
#include "stub/i386-darwin.dylib-entry.h"

static const
#include "stub/amd64-darwin.macho-entry.h"
static const
#include "stub/amd64-darwin.macho-fold.h"
static const
#include "stub/amd64-darwin.macho-upxmain.h"
static const
#include "stub/amd64-darwin.dylib-entry.h"

static const
#include "stub/arm.v5a-darwin.macho-entry.h"
static const
#include "stub/arm.v5a-darwin.macho-fold.h"

static const
#include "stub/arm64-darwin.macho-entry.h"
static const
#include "stub/arm64-darwin.macho-fold.h"

static const
#include "stub/powerpc-darwin.macho-entry.h"
static const
#include "stub/powerpc-darwin.macho-fold.h"
static const
#include "stub/powerpc-darwin.macho-upxmain.h"
static const
#include "stub/powerpc-darwin.dylib-entry.h"

static const
#include "stub/powerpc64le-darwin.macho-entry.h"
static const
#include "stub/powerpc64le-darwin.macho-fold.h"
static const
#include "stub/powerpc64le-darwin.dylib-entry.h"

// Packing a Darwin (Mach-o) Mac OS X dylib (dynamic shared library)
// is restricted.  UPX gets control as the -init function, at the very
// end of processing by dyld.  Relocation, loading of dependent libraries,
// etc., already have taken place before decompression.  So the Mach-o
// headers, the __IMPORT segment, the __LINKEDIT segment, anything
// that is modifed by relocation, etc., cannot be compressed.
// We simplify arbitrarily by compressing only the __TEXT segment,
// which must be the first segment.

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
    memset(&cmdUUID, 0, sizeof(cmdUUID));
    memset(&cmdSRCVER, 0, sizeof(cmdSRCVER));
    memset(&cmdVERMIN, 0, sizeof(cmdVERMIN));
    memset(&linkitem, 0, sizeof(linkitem));
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
PackDylibPPC64LE::PackDylibPPC64LE(InputFile *f) : super(f)
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

const int *PackMachARM64EL::getCompressionMethods(int method, int level) const
{
    return Packer::getDefaultCompressionMethods_8(method, level);
}


PackMachPPC32::PackMachPPC32(InputFile *f) : super(f, Mach_header::CPU_TYPE_POWERPC,
        Mach_header::MH_EXECUTE, Mach_thread_command::PPC_THREAD_STATE,
        sizeof(Mach_ppc_thread_state)>>2, sizeof(threado))
{ }

PackMachPPC64LE::PackMachPPC64LE(InputFile *f) : super(f, Mach_header::CPU_TYPE_POWERPC64LE,
        Mach_header::MH_EXECUTE, Mach_thread_command::PPC_THREAD_STATE64,
        sizeof(Mach_ppcle_thread_state64)>>2, sizeof(threado))
{ }

const int *PackMachPPC32::getFilters() const
{
    static const int filters[] = { 0xd0, FT_END };
    return filters;
}

const int *PackMachPPC64LE::getFilters() const
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

PackMachARM64EL::PackMachARM64EL(InputFile *f) : super(f, Mach_header::CPU_TYPE_ARM,
        Mach_header::MH_EXECUTE, (unsigned)Mach_thread_command::ARM_THREAD_STATE,
        sizeof(Mach_ARM64_thread_state)>>2, sizeof(threado))
{ }

int const *PackMachARMEL::getFilters() const
{
    static const int filters[] = { 0x50, FT_END };
    return filters;
}

int const *PackMachARM64EL::getFilters() const
{
    static const int filters[] = { 0x51, FT_END };
    return filters;
}

Linker *PackMachPPC32::newLinker() const
{
    return new ElfLinkerPpc32;
}

Linker *PackMachPPC64LE::newLinker() const
{
    return new ElfLinkerPpc64le;
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

Linker *PackMachARM64EL::newLinker() const
{
    return new ElfLinkerArm64LE;
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

    if (Mach_header::MH_EXECUTE == my_filetype) {
        addLoader("I386BXX0", NULL);  // .word offset to f_exp
    }
    else {
        addLoader("LEXEC000", NULL);  // entry to stub
    }
    if (ft->id) { // decompr, unfilter are separate
        if (Mach_header::MH_EXECUTE != my_filetype) {
            addLoader("LXUNF000", NULL);  // 2-byte jump to f_exp
        }
        addLoader("LXUNF002", NULL);  // entry to f_unf
        // prolog to f_unf
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
        else {
            if (0x40==(ft->id & 0xF0)) {
                addLoader("LXUNF008", NULL);
            }
        }
        if (Mach_header::MH_EXECUTE == my_filetype) {
            addFilter32(ft->id);  // f_unf body
            if (0x80==(ft->id & 0xF0)) {
                if (0==n_mru) {
                    addLoader("LXMRU058", NULL);
                }
            }
            addLoader("LXUNF035", NULL);  // epilog to f_unf
        }
        else { // MH_DYLIB
            addLoader("LXUNF010", NULL);  // jmp32 lxunf0  # to rest of f_unf
            if (n_mru) {
                addLoader("LEXEC009", NULL);  // empty (unify source with other cases)
            }
        }
    }
    if (Mach_header::MH_EXECUTE == my_filetype) {
        addLoader("I386BXX1", NULL);
    }
    addLoader("LEXEC010", NULL);  // prolog to f_exp
    addLoader(getDecompressorSections(), NULL);
    addLoader("LEXEC015", NULL);  // epilog to f_exp
    if (ft->id) {
        if (Mach_header::MH_EXECUTE != my_filetype) {
            if (0x80!=(ft->id & 0xF0)) {
                addLoader("LXUNF042", NULL);  // lxunf0:
            }
            addFilter32(ft->id);  // body of f_unf
            if (0x80==(ft->id & 0xF0)) {
                if (0==n_mru) {
                    addLoader("LXMRU058", NULL);
                }
            }
            addLoader("LXUNF035", NULL);  // epilog to f_unf
        }
    }
    else {
        addLoader("LEXEC017", NULL);  // epilog to f_exp
    }

    addLoader("IDENTSTR", NULL);
    addLoader("LEXEC020", NULL);
    addLoader("FOLDEXEC", NULL);
}

void PackMachAMD64::addStubEntrySections(Filter const * /*ft*/)
{
    if (my_filetype!=Mach_header::MH_EXECUTE) {
        addLoader("MACHMAINX", NULL);
    }
    else {
        addLoader("AMD64BXX", NULL);
    }
    addLoader("MACH_UNC", NULL);
   //addLoader(getDecompressorSections(), NULL);
    addLoader(
        ( M_IS_NRV2E(ph.method) ? "NRV_HEAD,NRV2E,NRV_TAIL"
        : M_IS_NRV2D(ph.method) ? "NRV_HEAD,NRV2D,NRV_TAIL"
        : M_IS_NRV2B(ph.method) ? "NRV_HEAD,NRV2B,NRV_TAIL"
        : M_IS_LZMA(ph.method)  ? "LZMA_ELF00,+80C,LZMA_DEC20,LZMA_DEC30"
        : NULL), NULL);
    if (hasLoaderSection("CFLUSH"))
        addLoader("CFLUSH");
    addLoader("MACHMAINY,IDENTSTR,+40,MACHMAINZ", NULL);
    if (my_filetype!=Mach_header::MH_EXECUTE) {
        addLoader("FOLDEXEC", NULL);
    }
}

void PackMachPPC32::addStubEntrySections(Filter const * /*ft*/)
{
    if (my_filetype!=Mach_header::MH_EXECUTE) {
        addLoader("MACHMAINX", NULL);
    }
    else {
        addLoader("PPC32BXX", NULL);
    }
    addLoader("MACH_UNC", NULL);
   //addLoader(getDecompressorSections(), NULL);
    addLoader(
        ( M_IS_NRV2E(ph.method) ? "NRV_HEAD,NRV2E,NRV_TAIL"
        : M_IS_NRV2D(ph.method) ? "NRV_HEAD,NRV2D,NRV_TAIL"
        : M_IS_NRV2B(ph.method) ? "NRV_HEAD,NRV2B,NRV_TAIL"
        : M_IS_LZMA(ph.method)  ? "LZMA_ELF00,+80C,LZMA_DEC20,LZMA_DEC30"
        : NULL), NULL);
    if (hasLoaderSection("CFLUSH"))
        addLoader("CFLUSH");
    addLoader("MACHMAINY,IDENTSTR,+40,MACHMAINZ", NULL);
    if (my_filetype!=Mach_header::MH_EXECUTE) {
        addLoader("FOLDEXEC", NULL);
    }
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

void PackMachARM64EL::addStubEntrySections(Filter const * /*ft*/)
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

template <class T>
void
PackMachBase<T>::buildLoader(const Filter *ft)
{
    buildMachLoader(
        stub_entry, sz_stub_entry,
        stub_fold,  sz_stub_fold,  ft );
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
    set_te16(&lp->l_lsize, (upx_uint16_t) lsize);
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
           if (0 != xa)        return  0;  // not LC_SEGMENT
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
#define PAGE_MASK64 (~(upx_uint64_t)0<<16)
#define PAGE_SIZE64 ((upx_uint64_t)0-PAGE_MASK64)

// At 2013-02-03 part of the source for codesign was
//    http://opensource.apple.com/source/cctools/cctools-836/libstuff/ofile.c

#undef PAGE_MASK64
#undef PAGE_SIZE64
#define PAGE_MASK64 (~(upx_uint64_t)0<<12)
#define PAGE_SIZE64 ((upx_uint64_t)0-PAGE_MASK64)

template <class T>
void PackMachBase<T>::pack4(OutputFile *fo, Filter &ft)  // append PackHeader
{
    // offset of p_info in compressed file
    overlay_offset = sizeof(mhdro) + sizeof(segZERO)
        + sizeof(segXHDR) + sizeof(secXHDR)
        + sizeof(segTEXT) + sizeof(secTEXT)
        + sizeof(cmdUUID) + sizeof(cmdSRCVER) + sizeof(cmdVERMIN) + sizeof(Mach_main_command)
        + sizeof(Mach_dyld_info_only_command) + sizeof(Mach_dysymtab_command)
        + sizeof(Mach_load_dylinker_command) + sizeof(Mach_load_dylib_command)
        + sizeof(Mach_function_starts_command) + sizeof(Mach_data_in_code_command)
        + sizeof(linfo);
    if (my_filetype==Mach_header::MH_EXECUTE) {
        overlay_offset = PAGE_SIZE;  // FIXME
        overlay_offset += sizeof(linfo);
    }

    super::pack4(fo, ft);
    if (my_filetype == Mach_header::MH_EXECUTE) {
        upx_uint64_t const zero = 0;
        unsigned const len = fo->getBytesWritten();
        fo->write(&zero, 7& (0u-len));
    }
    unsigned const eofcmpr = fo->getBytesWritten();
    if (Mach_header::CPU_TYPE_X86_64 == my_cputype) {
        // sneak in a little below 4GiB
        segTEXT.vmaddr = segZERO.vmaddr + segZERO.vmsize;
    }
    else {
        // ::pack1 set segTEXT.vmaddr to be va_hi: no conflict
    }
    segTEXT.filesize = eofcmpr;
    segTEXT.vmsize  += eofcmpr;  // utilize GAP + NO_LAP + sz_unc - sz_cpr
    secTEXT.offset = overlay_offset - sizeof(linfo);
    secTEXT.addr = segTEXT.vmaddr   + secTEXT.offset;
    secTEXT.size = segTEXT.filesize - secTEXT.offset;
    secXHDR.offset = overlay_offset - sizeof(linfo);
    if (my_filetype==Mach_header::MH_EXECUTE) {
        secXHDR.offset -= sizeof(linkitem);
    }
    secXHDR.addr += secXHDR.offset;
    unsigned offLINK = segLINK.fileoff;
    segLINK.vmaddr = segTEXT.vmaddr + offLINK;
    if (my_filetype != Mach_header::MH_EXECUTE) {
        fo->seek(offLINK - 1, SEEK_SET); fo->write("", 1);
        fo->seek(sizeof(mhdro), SEEK_SET);
        fo->rewrite(&segZERO, sizeof(segZERO));
        fo->rewrite(&segXHDR, sizeof(segXHDR));
        fo->rewrite(&secXHDR, sizeof(secXHDR));
        fo->rewrite(&segTEXT, sizeof(segTEXT));
        fo->rewrite(&secTEXT, sizeof(secTEXT));
        fo->rewrite(&cmdUUID, sizeof(cmdUUID));
        fo->rewrite(&segLINK, sizeof(segLINK));
        threado_rewrite(fo);
        if (my_filetype==Mach_header::MH_EXECUTE) {
            fo->rewrite(&linkitem, sizeof(linkitem));
        }
        fo->rewrite(&linfo, sizeof(linfo));
    }
    if (my_filetype == Mach_header::MH_EXECUTE) {
        // Get a writeable copy of the stub to make editing easier.
        ByteArray(upxstub, sz_stub_main);
        memcpy(upxstub, stub_main, sz_stub_main);

        Mach_header *const mhp = (Mach_header *)upxstub;
        mhp->cpusubtype = my_cpusubtype;
        char *tail = (char *)(1+ mhp);
        Mach_segment_command *segtxt = 0;  // in temp for output
        Mach_section_command *sectxt = 0;  // in temp for output
        unsigned txt_addr = 0;
        char *const lcp_end = mhdro.sizeofcmds + tail;
        Mach_command *lcp = (Mach_command *)(1+ mhp);
        Mach_command *lcp_next;
        unsigned const ncmds = mhdro.ncmds;
        //unsigned cmdsize = mhdro.sizeofcmds;
        unsigned delta = 0;

    unsigned va_next = 0;
    for (unsigned j = 0; j < ncmds; ++j) {
        unsigned skip = 0;
        unsigned sz_cmd = lcp->cmdsize;
        lcp_next = (Mach_command *)(sz_cmd + (char *)lcp);

        switch (lcp->cmd) {
        case Mach_segment_command::LC_SEGMENT: // fall through
        case Mach_segment_command::LC_SEGMENT_64: {
            Mach_segment_command *const segptr = (Mach_segment_command *)lcp;
            if (!strcmp("__TEXT", segptr->segname)) {
                segtxt = segptr;  // remember base for appending
                sectxt = (Mach_section_command *)(1+ segptr);
                txt_addr = sectxt->addr;
                // Collapse into 1 section by appending onto the first section.
                unsigned off_hi = sectxt->size + sectxt->offset;
                Mach_section_command const *scpk = 1+ sectxt;
                for (unsigned k = 1; k < segptr->nsects; ++scpk, ++k) {
                    if (off_hi == scpk->offset || scpk->size==0) {
                        off_hi += scpk->size;
                        sectxt->size += scpk->size;
                    }
                    else {
                        if (!(Mach_header::CPU_TYPE_POWERPC == my_cputype
                                && 12==(unsigned)scpk->size
                                && !strcmp("__cstring", (char const *)&scpk->sectname))) {
                            printWarn(fi->getName(), "Unexpected .text section %s  size=%u\n",
                             (char const *)&scpk->sectname, (unsigned)scpk->size);
                        }
                        // Assume the maximum size: to end of the page.
                        sectxt->size = (segptr->vmsize + segptr->vmaddr) - sectxt->addr;
                        // Try to stop the cascade of errors.
                        off_hi = scpk->size + scpk->offset;
                        break;
                    }
                }
                int const d = (-1+ segptr->nsects) * sizeof(secTEXT);
                if (0 < d) {
                    segptr->nsects = 1;
                    sz_cmd -= d;
                    segptr->cmdsize -= d;
                    mhp->sizeofcmds -= d;
                }
                if (Mach_header::CPU_TYPE_I386 == my_cputype
                ||  Mach_header::CPU_TYPE_POWERPC == my_cputype ) {
                    // Avoid overlap by moving to va_hi.
                    upx_uint64_t const delt2 = segTEXT.vmaddr - segptr->vmaddr;
                    segptr->vmaddr += delt2;
                    sectxt->addr   += delt2;
                }
                memcpy(&segTEXT, segptr, sizeof(segTEXT));
                memcpy(&secTEXT, sectxt, sizeof(secTEXT));
                va_next  = segTEXT.vmsize + segTEXT.vmaddr;
                break;
            }
            if (!strcmp("__DATA", segptr->segname) && !segptr->vmsize) {
                // __DATA with no pages.
                // Use the space as Mach_segment_command for new segXHDR, or elide.
                segXHDR.cmdsize = sizeof(segXHDR);  // no need for sections
                segXHDR.vmaddr = segTEXT.vmsize + segTEXT.vmaddr;  // XXX FIXME
                segXHDR.vmsize = offLINK - segTEXT.vmsize;
                segXHDR.fileoff = segTEXT.filesize + segTEXT.fileoff;  // XXX FIXME: assumes no __DATA in stub
                segXHDR.filesize = offLINK - segTEXT.filesize;  // XXX FIXME: assumes no __DATA in stub;
                segXHDR.maxprot = Mach_segment_command::VM_PROT_READ;
                segXHDR.nsects = 0;
                if (!segtxt) { // replace __DATA with segXHDR
                    va_next  = segXHDR.vmsize + segXHDR.vmaddr;
                    memcpy(tail, &segXHDR, sizeof(segXHDR));
                    tail += sizeof(segXHDR);
                    goto next;
                }
                else { // append to .text
                    segtxt->vmsize = offLINK;
                    segtxt->filesize = offLINK;
                    sectxt->size = offLINK - (~PAGE_MASK & sectxt->offset);
                    va_next  = segtxt->vmsize + segtxt->vmaddr;
                    skip = 1;
                }
            }
            if (!strcmp("__LINKEDIT", segptr->segname)) {
                segLINK.initprot = Mach_segment_command::VM_PROT_READ
                                 | Mach_segment_command::VM_PROT_EXECUTE;
                delta = offLINK - segptr->fileoff;  // relocation constant

                // Update the __LINKEDIT header
                segLINK.filesize = eofcmpr - offLINK;
                segLINK.vmsize = PAGE_MASK64 & (~PAGE_MASK64 + eofcmpr - offLINK);
                if (Mach_header::CPU_TYPE_I386 == my_cputype) {
                    segLINK.fileoff = offLINK;  // otherwise written by ::pack3
                }
                segLINK.vmaddr = va_next;
                memcpy(tail, &segLINK, sizeof(segLINK));
                tail += sizeof(segLINK);
                goto next;
            }
        } break;
        case Mach_segment_command::LC_DYLD_INFO_ONLY: {
            Mach_dyld_info_only_command *p = (Mach_dyld_info_only_command *)lcp;
            if (p->rebase_off)    p->rebase_off    += delta;
            if (p->bind_off)      p->bind_off      += delta;
            if (p->lazy_bind_off) p->lazy_bind_off += delta;
            if (p->export_off)    p->export_off    += delta;
                // But we don't want any exported symbols.
                p->export_off = 0;
                p->export_size = 0;
            skip = 1;
        } break;
        case Mach_segment_command::LC_SYMTAB: {
            // Apple codesign requires that string table is last in the file.
            Mach_symtab_command *p = (Mach_symtab_command *)lcp;
            p->symoff = segLINK.filesize + segLINK.fileoff;
            p->nsyms = 0;
            p->stroff = segLINK.fileoff;
            p->strsize = segLINK.filesize;
            skip = 1;
        } break;
        case Mach_segment_command::LC_DYSYMTAB: {
            Mach_dysymtab_command *p = (Mach_dysymtab_command *)lcp;
            if (p->tocoff)         p->tocoff         += delta;
            if (p->modtaboff)      p->modtaboff      += delta;
            if (p->extrefsymoff)   p->extrefsymoff   += delta;
            if (p->indirectsymoff) p->indirectsymoff += delta;
            if (p->extreloff)      p->extreloff      += delta;
            if (p->locreloff)      p->locreloff      += delta;
                // But we don't want any symbols.
                p->ilocalsym = 0;
                p->nlocalsym = 0;
                p->iextdefsym = 0;
                p->nextdefsym = 0;
                p->iundefsym = 0;
                p->nundefsym = 0;
            skip = 1;
        } break;
        case Mach_segment_command::LC_FUNCTION_STARTS: {
            Mach_function_starts_command *p = (Mach_function_starts_command *)lcp;
            if (p->dataoff) p->dataoff += delta;
            skip = 1;
        } break;
        case Mach_segment_command::LC_MAIN: {
                // Replace later with LC_UNIXTHREAD.
// LC_MAIN requires libSystem.B.dylib to provide the environment for main(), and CALLs the entryoff.
// LC_UNIXTHREAD does not need libSystem.B.dylib, and JMPs to the .rip with %rsp/argc and argv= 8+%rsp
            threado_setPC(segTEXT.vmaddr +
                (((Mach_main_command const *)lcp)->entryoff - segTEXT.fileoff));
            skip = 1;
        } break;
        case Mach_segment_command::LC_UNIXTHREAD: { // pre-LC_MAIN
            threado_setPC(secTEXT.addr +
                (threadc_getPC(lcp) - txt_addr));
            skip = 1;
        } break;
        case Mach_segment_command::LC_LOAD_DYLIB: {
            skip = 1;
        } break;
        case Mach_segment_command::LC_DATA_IN_CODE: {
            Mach_data_in_code_command *p = (Mach_data_in_code_command *)lcp;
            if (p->dataoff) p->dataoff += delta;
            skip = 1;
        } break;
        case Mach_segment_command::LC_LOAD_DYLINKER: {
            skip = 1;
        } break;
        case Mach_segment_command::LC_SOURCE_VERSION: { // copy from saved original
            memcpy(lcp, &cmdSRCVER, sizeof(cmdSRCVER));
            if (Mach_segment_command::LC_SOURCE_VERSION != cmdSRCVER.cmd) {
                skip = 1;  // was not seen
            }
        } break;
        case Mach_segment_command::LC_VERSION_MIN_MACOSX: { // copy from saved original
            memcpy(lcp, &cmdVERMIN, sizeof(cmdVERMIN));
            if (Mach_segment_command::LC_VERSION_MIN_MACOSX != cmdVERMIN.cmd) {
                skip = 1;  // was not seen
            }
        } break;
        } // end switch

        if (skip) {
            mhp->ncmds -= 1;
            mhp->sizeofcmds -= sz_cmd;
        }
        else {
            if (tail != (char *)lcp) {
                memmove(tail, lcp, sz_cmd);
            }
            tail += sz_cmd;
        }
next:
        lcp = lcp_next;
    }  // end for each Mach_command

        // Append LC_UNIXTHREAD
        unsigned const sz_threado = threado_size();
        mhp->ncmds += 1;
        mhp->sizeofcmds += sz_threado;
        fo->seek(0, SEEK_SET);
        fo->rewrite(mhp, tail - (char *)mhp);
        threado_rewrite(fo);
        tail += sz_threado;
        //
        // Zero any remaining tail.
        if (tail < lcp_end) {
            unsigned sz_cmd = lcp_end - tail;
            memset(tail, 0, sz_cmd);
            fo->rewrite(tail, sz_cmd);
        }
        // Rewrite linfo in file.
        fo->seek(sz_mach_headers, SEEK_SET);
        fo->rewrite(&linfo, sizeof(linfo));
        fo->seek(0, SEEK_END);
    }
}

// At 2013-02-03 part of the source for codesign was
//    http://opensource.apple.com/source/cctools/cctools-836/libstuff/ofile.c

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

void PackDylibPPC64LE::pack4(OutputFile *fo, Filter &ft)  // append PackHeader
{
    pack4dylib(fo, ft, threado.state64.srr0);
}

template <class T>
void PackMachBase<T>::pack3(OutputFile *fo, Filter &ft)  // append loader
{
    TE32 disp;
    upx_uint32_t const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 3& (0u-len));
    len += (3& (0u-len));

    disp = len - sz_mach_headers;
    fo->write(&disp, sizeof(disp));
    len += sizeof(disp);

    char page[~PAGE_MASK]; memset(page, 0, sizeof(page));
    fo->write(page, ~PAGE_MASK & (0u - len));
    len += ~PAGE_MASK & (0u - len);
    segLINK.fileoff = len;

    threado_setPC(len + segTEXT.vmaddr);  /* entry address */
    super::pack3(fo, ft);
    len = fo->getBytesWritten();
    fo->write(&zero, 7& (0u-len));
}

void PackDylibI386::pack3(OutputFile *fo, Filter &ft)  // append loader
{
    TE32 disp;
    upx_uint32_t const zero = 0;
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
    upx_uint64_t const zero = 0;
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
    upx_uint32_t const zero = 0;
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

void PackDylibPPC64LE::pack3(OutputFile *fo, Filter &ft)  // append loader
{
    TE64 disp;
    upx_uint64_t const zero = 0;
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
    unsigned const k, unsigned pos_eof
)
{
    unsigned const lc_seg = lc_segment[sizeof(Addr)>>3];
    if (lc_seg!=msegcmd[k].cmd
    ||  0==msegcmd[k].filesize ) {
        return 0;
    }
    unsigned const hi = msegcmd[k].fileoff + msegcmd[k].filesize;
    unsigned lo = pos_eof;
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
            if (find_SEGMENT_gap(k, fi->st_size())) {
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
        x.size = find_SEGMENT_gap(k, fi->st_size());
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

void PackMachPPC64LE::pack1_setup_threado(OutputFile *const fo)
{
    threado.cmd = Mach_segment_command::LC_UNIXTHREAD;
    threado.cmdsize = sizeof(threado);
    threado.flavor = my_thread_flavor;
    threado.count =  my_thread_state_word_count;
    memset(&threado.state64, 0, sizeof(threado.state64));
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

void PackMachARM64EL::pack1_setup_threado(OutputFile *const fo)
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
        memcpy(&mhdro, stub_main, sizeof(mhdro));
        COMPILE_TIME_ASSERT(sizeof(mhdro.flags) == sizeof(unsigned))
        mhdro.flags &= ~ (unsigned) Mach_header::MH_PIE;  // we require fixed address
        mhdro.flags |= Mach_header::MH_BINDATLOAD;  // DT_BIND_NOW
    }
    unsigned pos = sizeof(mhdro);
    fo->write(&mhdro, sizeof(mhdro));

    memset(&segZERO, 0, sizeof(segZERO));
    segZERO.cmd = lc_seg;
    segZERO.cmdsize = sizeof(segZERO);
    strncpy((char *)segZERO.segname, "__PAGEZERO", sizeof(segZERO.segname));
    segZERO.vmsize = PAGE_SIZE;
    if __acc_cte(sizeof(segZERO.vmsize) == 8
    && mhdro.filetype == Mach_header::MH_EXECUTE
    && mhdro.cputype == Mach_header::CPU_TYPE_X86_64) {
        segZERO.vmsize <<= 20;  // (1ul<<32)
    }

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
    segTEXT.maxprot =
        Mach_segment_command::VM_PROT_READ |
        Mach_segment_command::VM_PROT_WRITE |
        Mach_segment_command::VM_PROT_EXECUTE;
    segTEXT.initprot =
        Mach_segment_command::VM_PROT_READ |
        Mach_segment_command::VM_PROT_EXECUTE;
    segTEXT.nsects = 1;  // secTEXT
    segTEXT.flags = 0;

    memset(&secTEXT, 0, sizeof(secTEXT));
    strncpy((char *)secTEXT.sectname, "__text", sizeof(secTEXT.sectname));
    memcpy(secTEXT.segname, segTEXT.segname, sizeof(secTEXT.segname));
    secTEXT.align = 2;  // (1<<2) ==> 4
    secTEXT.flags = Mach_section_command::S_REGULAR
        | Mach_section_command::S_ATTR_SOME_INSTRUCTIONS
        | Mach_section_command::S_ATTR_PURE_INSTRUCTIONS;

    segXHDR = segTEXT;
    segXHDR.cmdsize = sizeof(segXHDR) + sizeof(secXHDR);
    segXHDR.vmaddr = segZERO.vmsize;
    segXHDR.vmsize = PAGE_SIZE;
    segXHDR.filesize = PAGE_SIZE;
    segXHDR.nsects = 1;
    strncpy((char *)segXHDR.segname,  "UPX_DATA", sizeof(segXHDR.segname));

    memset(&secXHDR, 0, sizeof(secXHDR));
    strncpy((char *)secXHDR.sectname, "upx_data", sizeof(secXHDR.sectname));
    memcpy(secXHDR.segname,  segXHDR.segname, sizeof(secXHDR.segname));
    secXHDR.addr = segXHDR.vmaddr;
    secXHDR.size = 0;  // empty so far
    secXHDR.align = 2;  // (1<<2) ==> 4

    segLINK = segTEXT;
    segLINK.cmdsize = sizeof(segLINK);
    strncpy((char *)segLINK.segname, "__LINKEDIT", sizeof(segLINK.segname));
    segLINK.nsects = 0;
    segLINK.initprot = Mach_segment_command::VM_PROT_READ
                     | Mach_segment_command::VM_PROT_EXECUTE;
    // Adjust later: .vmaddr .vmsize .fileoff .filesize

    if (my_filetype == Mach_header::MH_EXECUTE) {
        unsigned cmdsize = mhdro.sizeofcmds - sizeof(segXHDR);
        Mach_header const *const ptr0 = (Mach_header const *)stub_main;
        Mach_command const *ptr1 = (Mach_command const *)(1+ ptr0);
        for (unsigned j = 0; j < mhdro.ncmds -1; ++j,
                (cmdsize -= ptr1->cmdsize),
                ptr1 = (Mach_command const *)(ptr1->cmdsize + (char const *)ptr1)) {
                Mach_segment_command const *const segptr = (Mach_segment_command const *)ptr1;
            if (lc_seg == ptr1->cmd) {
                if (!strcmp("__LINKEDIT", segptr->segname)) {
                    // Mach_command before __LINKEDIT
                    pos += (char const *)ptr1 - (char const *)(1+ ptr0);
                    fo->write((1+ ptr0), (char const *)ptr1 - (char const *)(1+ ptr0));
                    pos += sizeof(segXHDR);
                    fo->write(&segXHDR, sizeof(segXHDR));
                    // Mach_command __LINKEDIT and after
                    pos += cmdsize;
                    fo->write((char const *)ptr1, cmdsize);
                    // Contents before __LINKEDIT; put non-headers at same offset in file
                    fo->write(&stub_main[pos], segptr->fileoff - pos);
                    break;
                }
            }
        }
    }
    else { // not MH_EXECUTE; thus MH_DYLIB
        fo->write(&segZERO, sizeof(segZERO));
        fo->write(&segXHDR, sizeof(segXHDR));
        fo->write(&secXHDR, sizeof(secXHDR));
        fo->write(&segTEXT, sizeof(segTEXT));
        fo->write(&secTEXT, sizeof(secTEXT));
        fo->write(&cmdUUID, sizeof(cmdUUID));
        fo->write(&segLINK, sizeof(segLINK));
        pack1_setup_threado(fo);
        memset(&linkitem, 0, sizeof(linkitem));
        fo->write(&linkitem, sizeof(linkitem));
        fo->write(rawmseg, mhdri.sizeofcmds);
    }
    sz_mach_headers = fo->getBytesWritten();

    memset((char *)&linfo, 0, sizeof(linfo));
    fo->write(&linfo, sizeof(linfo));

    return;
}

#define WANT_MACH_HEADER_ENUM 1
#include "p_mach_enum.h"

static unsigned
umin(unsigned a, unsigned b)
{
    return (a <= b) ? a : b;
}

template <class T>
void PackMachBase<T>::unpack(OutputFile *fo)
{
    unsigned const lc_seg = lc_segment[sizeof(Addr)>>3];
    fi->seek(0, SEEK_SET);
    fi->readx(&mhdri, sizeof(mhdri));
    if ((MH_MAGIC + (sizeof(Addr)>>3)) != mhdri.magic
    &&  Mach_fat_header::FAT_MAGIC != mhdri.magic) {
        throwCantUnpack("file header corrupted");
    }
    unsigned const sz_cmds = mhdri.sizeofcmds;
    if ((sizeof(mhdri) + sz_cmds) > (size_t)fi->st_size()) {
        throwCantUnpack("file header corrupted");
    }
    rawmseg = (Mach_segment_command *)new char[sz_cmds];
    fi->readx(rawmseg, mhdri.sizeofcmds);

    // FIXME forgot space left for LC_CODE_SIGNATURE;
    // but canUnpack() sets overlay_offset anyway.
    //overlay_offset = sizeof(mhdri) + mhdri.sizeofcmds + sizeof(linfo);

    fi->seek(overlay_offset, SEEK_SET);
    p_info hbuf;
    fi->readx(&hbuf, sizeof(hbuf));
    unsigned const orig_file_size = get_te32(&hbuf.p_filesize);
    blocksize = get_te32(&hbuf.p_blocksize);
    if (file_size > (off_t)orig_file_size || blocksize > orig_file_size
    ||  blocksize > 0x05000000)  // emacs-21.2.1 was 0x01d47e6c (== 30703212)
        throwCantUnpack("file header corrupted");

    ibuf.alloc(blocksize + OVERHEAD);
    b_info bhdr; memset(&bhdr, 0, sizeof(bhdr));
    fi->readx(&bhdr, sizeof(bhdr));
    ph.u_len = get_te32(&bhdr.sz_unc);
    ph.c_len = get_te32(&bhdr.sz_cpr);
    if ((unsigned)file_size < ph.c_len || ph.c_len == 0 || ph.u_len == 0)
        throwCantUnpack("file header corrupted");
    ph.method = bhdr.b_method;
    ph.filter = bhdr.b_ftid;
    ph.filter_cto = bhdr.b_cto8;

    // Uncompress Macho headers
    fi->readx(ibuf, ph.c_len);
    Mach_header *const mhdr = (Mach_header *)new upx_byte[ph.u_len];
    decompress(ibuf, (upx_byte *)mhdr, false);
    if (mhdri.magic      != mhdr->magic
    ||  mhdri.cputype    != mhdr->cputype
    ||  mhdri.cpusubtype != mhdr->cpusubtype
    ||  mhdri.filetype   != mhdr->filetype)
        throwCantUnpack("file header corrupted");
    unsigned const ncmds = mhdr->ncmds;

    msegcmd = new Mach_segment_command[ncmds];
    unsigned char const *ptr = (unsigned char const *)(1+mhdr);
    for (unsigned j= 0; j < ncmds; ++j) {
        memcpy(&msegcmd[j], ptr, umin(sizeof(Mach_segment_command),
            ((Mach_command const *)ptr)->cmdsize));
        ptr += (unsigned) ((Mach_segment_command const *)ptr)->cmdsize;
        if ((unsigned)(ptr - (unsigned char const *)mhdr) > ph.u_len) {
            throwCantUnpack("cmdsize");
        }
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

    fi->seek(- (off_t)(sizeof(bhdr) + ph.c_len), SEEK_CUR);
    for (unsigned k = 0; k < ncmds; ++k) {
        if (msegcmd[k].cmd==lc_seg && msegcmd[k].filesize!=0) {
            if (fo)
                fo->seek(msegcmd[k].fileoff, SEEK_SET);
            unpackExtent(msegcmd[k].filesize, fo, total_in, total_out,
                c_adler, u_adler, false, sizeof(bhdr));
            if (my_filetype==Mach_header::MH_DYLIB) {
                break;  // only the first lc_seg when MH_DYLIB
            }
        }
    }
    Mach_segment_command const *sc = (Mach_segment_command const *)(void *)(1+ mhdr);
    if (my_filetype==Mach_header::MH_DYLIB) { // rest of lc_seg are not compressed
        Mach_segment_command const *rc = rawmseg;
        rc = (Mach_segment_command const *)(rc->cmdsize + (char const *)rc);
        sc = (Mach_segment_command const *)(sc->cmdsize + (char const *)sc);
        for (
            unsigned k=1;  // skip first lc_seg, which was uncompressed above
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
        unsigned const size = find_SEGMENT_gap(j, orig_file_size);
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

// The prize is the value of overlay_offset: the offset of compressed data
template <class T>
int PackMachBase<T>::canUnpack()
{
    unsigned const lc_seg = lc_segment[sizeof(Addr)>>3];
    fi->seek(0, SEEK_SET);
    fi->readx(&mhdri, sizeof(mhdri));

    if (((unsigned) Mach_header::MH_MAGIC + (sizeof(Addr)>>3)) !=mhdri.magic
    ||  my_cputype   !=mhdri.cputype
    ||  my_filetype  !=mhdri.filetype
    )
        return false;
    my_cpusubtype = mhdri.cpusubtype;

    rawmseg = (Mach_segment_command *)new char[(unsigned) mhdri.sizeofcmds];
    fi->readx(rawmseg, mhdri.sizeofcmds);

    Mach_segment_command const *ptrTEXT = 0;
    upx_uint64_t rip = 0;
    unsigned style = 0;
    off_t offLINK = 0;
    unsigned pos_next = 0;
    unsigned nseg = 0;
    unsigned const ncmds = mhdri.ncmds;
    Mach_command const *ptr = (Mach_command const *)rawmseg;
    for (unsigned j= 0; j < ncmds;
            ptr = (Mach_command const *)(ptr->cmdsize + (char const *)ptr), ++j) {
        Mach_segment_command const *const segptr = (Mach_segment_command const *)ptr;
        if (lc_seg == ptr->cmd) {
            ++nseg;
            if (!strcmp("__XHDR", segptr->segname)) {
                // PackHeader precedes __LINKEDIT (pre-Sierra MacOS 10.12)
                style = 391;  // UPX 3.91
            }
            if (!strcmp("__TEXT", segptr->segname)) {
                ptrTEXT = segptr;
                style = 391;  // UPX 3.91
            }
            if (!strcmp("UPX_DATA", segptr->segname)) {
                // PackHeader follows loader at __LINKEDIT (Sierra MacOS 10.12)
                style = 392;  // UPX 3.92
            }
            if (!strcmp("__LINKEDIT", segptr->segname)) {
                offLINK = segptr->fileoff;
                if (offLINK < (off_t) pos_next) {
                    offLINK = pos_next;
                }
            }
            pos_next = segptr->filesize + segptr->fileoff;
        }
        else if (Mach_segment_command::LC_UNIXTHREAD==ptr->cmd) {
            rip = entryVMA = threadc_getPC(ptr);
        }
    }
    if (3==nseg) { // __PAGEZERO, __TEXT, __LINKEDIT;  no __XHDR, no UPX_DATA
        style = 392;
    }
    if (391==style && 0==offLINK && 2==ncmds) { // pre-3.91 ?
        offLINK = ptrTEXT->fileoff + ptrTEXT->filesize;  // fake __LINKEDIT at EOF
    }
    if (0 == style || 0 == offLINK) {
        return false;
    }

    int const small = 32 + sizeof(overlay_offset);
    unsigned bufsize = 4096;
    if (391 == style) { // PackHeader precedes __LINKEDIT
        fi->seek(offLINK - bufsize, SEEK_SET);
    } else
    if (392 == style) { // PackHeader follows loader at __LINKEDIT
        if ((off_t)bufsize > (fi->st_size() - offLINK)) {
            bufsize = fi->st_size() - offLINK;
        }
        fi->seek(offLINK, SEEK_SET);
    }
    MemBuffer buf(bufsize);

    fi->readx(buf, bufsize);
    int i = bufsize;
    while (i > small && 0 == buf[--i]) { }
    i -= small;
    // allow incompressible extents
    if (i < 1 || !getPackHeader(buf + i, bufsize - i, true)) {
        // Breadcrumbs failed.
        // Pirates might overwrite the UPX! marker.  Try harder.
        upx_uint64_t const rip_off = ptrTEXT ? (rip - ptrTEXT->vmaddr) : 0;
        if (ptrTEXT && rip && rip_off < ptrTEXT->vmsize) {
            fi->seek(ptrTEXT->fileoff + rip_off, SEEK_SET);
            fi->readx(buf, bufsize);
            unsigned char const *b = &buf[0];
            unsigned disp = *(TE32 const *)&b[1];
            // Emulate the code
            if (0xe8==b[0] && disp < bufsize
            &&  0x5d==b[5+disp] && 0xe8==b[6+disp]) {
                unsigned disp2 = 0u - *(TE32 const *)&b[7+disp];
                if (disp2 < (12+disp) && 0x5b==b[11+disp-disp2]) {
                    struct b_info const *bptr = (struct b_info const *)&b[11+disp];
                    // This is the folded stub.
                    // FIXME: check b_method?
                    if (bptr->sz_cpr < bptr->sz_unc && bptr->sz_unc < 0x1000) {
                        b = bptr->sz_cpr + (unsigned char const *)(1+ bptr);
                        // FIXME: check PackHeader::putPackHeader(), packhead.cpp
                        overlay_offset = *(TE32 const *)(32 + b);
                        if (overlay_offset < 0x1000) {
                            return true;  // success
                        }
                        overlay_offset = 0;
                    }
                }
            }
        }
        if (391==style) {
            TE32 const *uptr = (TE32 const *)&buf[bufsize];
            while (0==*--uptr) /*empty*/ ;
            overlay_offset = *uptr;
            if (mhdri.sizeofcmds <= overlay_offset && overlay_offset < 0x1000) {
                return true;  // success
            }
            overlay_offset = 0;
            return false;
        }
        if (392==style) {
            overlay_offset = 0x100c;  // (l_info precedes;) p_info; b_info; cpr_data
            // p_info isn't used for execution, so it has less value for checking:
            //      0== .p_progid
            //      .p_filesize == .p_blocksize
            fi->seek(overlay_offset, SEEK_SET);
            fi->readx(buf, bufsize);
            struct p_info const *const p_ptr = (struct p_info const *)&buf[0];
            struct b_info const *const b_ptr = (struct b_info const *)(1+ p_ptr);
            TE32 const *uptr = (TE32 const *)(1+ b_ptr);
            if (b_ptr->sz_unc < 0x4000
            &&  b_ptr->sz_cpr < b_ptr->sz_unc ) {
                unsigned const method = b_ptr->b_method;
                if ((M_NRV2B_LE32 == method || M_NRV2E_LE32 == method)
                &&  (0xff>>2)==(uptr[0] >> (2+ 24))  // 1st 6 bytes are unique literals
                &&  (Mach_header::MH_MAGIC + (sizeof(Addr)>>3)) == uptr[1]) {
                    return true;
                }
                unsigned const magic = get_te32(1+ (char const *)uptr);
                if ((M_NRV2B_8 == method || M_NRV2E_8 == method)
                && 0xfc==(0xfc & uptr[0])
                &&  (Mach_header::MH_MAGIC + (sizeof(Addr)>>3)) == magic) {
                    return true;
                }
                // FIXME: M_LZMA
            }

            overlay_offset = 0;
            // The first non-zero word scanning backwards from __LINKEDIT.fileoff
            // is the total length of compressed data which preceeds it
            //(distance to l_info), so that's another method.
            fi->seek(offLINK-0x1000, SEEK_SET);
            fi->readx(buf, 0x1000);
            unsigned const *const lo = (unsigned const *)&buf[0];
            unsigned const *p;
            for (p = (unsigned const *)&buf[0x1000]; p > lo; ) if (*--p) {
                overlay_offset  = *(TE32 const *)p;
                if ((off_t)overlay_offset < offLINK) {
                    overlay_offset -= (char const *)p - (char const *)lo
                        + (offLINK - 0x1000) - sizeof(l_info);
                    fi->seek(overlay_offset, SEEK_SET);
                    fi->readx(buf, bufsize);
                    if (b_ptr->sz_unc < 0x4000
                    &&  b_ptr->sz_cpr < b_ptr->sz_unc ) {
                        return true;
                    }
                }
            }

            overlay_offset = 0;
        }
    }

    int l = ph.buf_offset + ph.getPackHeaderSize();
    if (l < 0 || (unsigned)(l + 4) > bufsize)
        throwCantUnpack("file corrupted");
    overlay_offset = get_te32(buf + i + l);
    if ((off_t)overlay_offset >= file_size)
        throwCantUnpack("file corrupted");

    return true;
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
    my_cpusubtype = mhdri.cpusubtype;

    rawmseg = (Mach_segment_command *)new char[(unsigned) mhdri.sizeofcmds];
    fi->readx(rawmseg, mhdri.sizeofcmds);

    unsigned const ncmds = mhdri.ncmds;
    msegcmd = new Mach_segment_command[ncmds];
    unsigned char const *ptr = (unsigned char const *)rawmseg;
    for (unsigned j= 0; j < ncmds; ++j) {
        Mach_segment_command const *segptr = (Mach_segment_command const *)ptr;
        if (lc_seg == segptr->cmd) {
            msegcmd[j] = *segptr;
        }
        else {
            memcpy(&msegcmd[j], ptr, 2*sizeof(unsigned)); // cmd and size
        }
        switch (((Mach_uuid_command const *)ptr)->cmd) {
        default: break;
        case Mach_segment_command::LC_UUID: {
            memcpy(&cmdUUID, ptr, sizeof(cmdUUID));  // remember the UUID
            // Set output UUID to be 1 more than the input UUID.
            for (unsigned k = 0; k < sizeof(cmdUUID.uuid); ++k) {
                if (0 != ++cmdUUID.uuid[k]) { // no Carry
                    break;
                }
            }
        } break;
        case Mach_segment_command::LC_VERSION_MIN_MACOSX: {
            memcpy(&cmdVERMIN, ptr, sizeof(cmdVERMIN));
        } break;
        case Mach_segment_command::LC_SOURCE_VERSION: {
            memcpy(&cmdSRCVER, ptr, sizeof(cmdSRCVER));
        } break;
        }
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
            if (msegcmd[j].vmsize==0)
                break;  // was sorted last
            if (~PAGE_MASK & (msegcmd[j].fileoff | msegcmd[j].vmaddr)) {
                return false;
            }

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
    if (!n_segment) {
        return false;
    }
    struct {
        unsigned cputype;
        unsigned short filetype;
        unsigned short sz_stub_entry;
        unsigned short sz_stub_fold;
        unsigned short sz_stub_main;
        upx_byte const *stub_entry;
        upx_byte const *stub_fold;
        upx_byte const *stub_main;
    } const stub_list[] = {
        {CPU_TYPE_I386, MH_EXECUTE,
            sizeof(stub_i386_darwin_macho_entry),
            sizeof(stub_i386_darwin_macho_fold),
            sizeof(stub_i386_darwin_macho_upxmain_exe),
                   stub_i386_darwin_macho_entry,
                   stub_i386_darwin_macho_fold,
                   stub_i386_darwin_macho_upxmain_exe
        },
        {CPU_TYPE_I386, MH_DYLIB,
            sizeof(stub_i386_darwin_dylib_entry), 0, 0,
                   stub_i386_darwin_dylib_entry,  0, 0
        },
        {CPU_TYPE_X86_64, MH_EXECUTE,
            sizeof(stub_amd64_darwin_macho_entry),
            sizeof(stub_amd64_darwin_macho_fold),
            sizeof(stub_amd64_darwin_macho_upxmain_exe),
                   stub_amd64_darwin_macho_entry,
                   stub_amd64_darwin_macho_fold,
                   stub_amd64_darwin_macho_upxmain_exe
        },
        {CPU_TYPE_X86_64, MH_DYLIB,
            sizeof(stub_amd64_darwin_dylib_entry), 0, 0,
                   stub_amd64_darwin_dylib_entry,  0, 0
        },
        {CPU_TYPE_ARM, MH_EXECUTE,
            sizeof(stub_arm_v5a_darwin_macho_entry),
            sizeof(stub_arm_v5a_darwin_macho_fold),
            0,
                   stub_arm_v5a_darwin_macho_entry,
                   stub_arm_v5a_darwin_macho_fold,
                   0
        },
        {CPU_TYPE_POWERPC, MH_EXECUTE,
            sizeof(stub_powerpc_darwin_macho_entry),
            sizeof(stub_powerpc_darwin_macho_fold),
            sizeof(stub_powerpc_darwin_macho_upxmain_exe),
                   stub_powerpc_darwin_macho_entry,
                   stub_powerpc_darwin_macho_fold,
                   stub_powerpc_darwin_macho_upxmain_exe
        },
        {CPU_TYPE_POWERPC, MH_DYLIB,
            sizeof(stub_powerpc_darwin_dylib_entry), 0, 0,
                   stub_powerpc_darwin_dylib_entry,  0, 0
        },
        {CPU_TYPE_POWERPC64LE, MH_EXECUTE,
            sizeof(stub_powerpc64le_darwin_macho_entry),
            sizeof(stub_powerpc64le_darwin_macho_fold),
            0,
                   stub_powerpc64le_darwin_macho_entry,
                   stub_powerpc64le_darwin_macho_fold,
                   0
        },
        {CPU_TYPE_POWERPC64LE, MH_DYLIB,
            sizeof(stub_powerpc64le_darwin_dylib_entry), 0, 0,
                   stub_powerpc64le_darwin_dylib_entry,  0, 0
        },
        {0,0, 0,0,0, 0,0,0}
    };
    for (unsigned j = 0; stub_list[j].cputype; ++j) {
        if (stub_list[j].cputype  == my_cputype
        &&  stub_list[j].filetype == my_filetype) {
            sz_stub_entry = stub_list[j].sz_stub_entry;
               stub_entry = stub_list[j].stub_entry;
            sz_stub_fold  = stub_list[j].sz_stub_fold;
               stub_fold  = stub_list[j].stub_fold;
            sz_stub_main  = stub_list[j].sz_stub_main;
               stub_main  = stub_list[j].stub_main;
        }
    }
    return true;
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

unsigned PackMachFat::check_fat_head()
{
    struct Mach_fat_arch const *const arch = &fat_head.arch[0];
    unsigned nfat = fat_head.fat.nfat_arch;
    if (Mach_fat_header::FAT_MAGIC!=fat_head.fat.magic
    ||  N_FAT_ARCH < nfat) {
        return 0;
    }
    for (unsigned j=0; j < nfat; ++j) {
        unsigned const align = arch[j].align;
        unsigned const mask = ~(~0u<<align);
        unsigned const size = arch[j].size;
        unsigned const offset = arch[j].offset;
        if (align < 12 || align > 24) { // heuristic
            throwUnknownExecutableFormat("align", 0);
        }
        if (mask > size) {
            throwUnknownExecutableFormat("size", 0);
        }
        if (mask & offset
        ||  (unsigned)fi->st_size_orig() < size + offset
        ||  (unsigned)fi->st_size_orig() <= offset) {  // redundant unless overflow
            throwUnknownExecutableFormat("offset", 0);
        }
    }
    return nfat;
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
        case PackMachFat::CPU_TYPE_POWERPC64LE: {
            typedef N_Mach::Mach_header<MachClass_LE64::MachITypes> Mach_header;
            Mach_header hdr;
            fi->readx(&hdr, sizeof(hdr));
            if (hdr.filetype==Mach_header::MH_EXECUTE) {
                PackMachPPC64LE packer(fi);
                packer.initPackHeader();
                packer.canPack();
                packer.updatePackHeader();
                packer.pack(fo);
            }
            else if (hdr.filetype==Mach_header::MH_DYLIB) {
                PackDylibPPC64LE packer(fi);
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
    unsigned const nfat = check_fat_head();
    unsigned length;
    for (unsigned j=0; j < nfat; ++j) {
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
        case PackMachFat::CPU_TYPE_POWERPC64LE: {
            N_Mach::Mach_header<MachClass_LE64::MachITypes> hdr;
            typedef N_Mach::Mach_header<MachClass_LE64::MachITypes> Mach_header;
            fi->readx(&hdr, sizeof(hdr));
            if (hdr.filetype==Mach_header::MH_EXECUTE) {
                PackMachPPC64LE packer(fi);
                packer.initPackHeader();
                packer.canUnpack();
                packer.unpack(fo);
            }
            else if (hdr.filetype==Mach_header::MH_DYLIB) {
                PackDylibPPC64LE packer(fi);
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
    struct Mach_fat_arch const *const arch = &fat_head.arch[0];

    fi->readx(&fat_head, sizeof(fat_head));
    unsigned const nfat = check_fat_head();
    if (0==nfat)
        return false;
    for (unsigned j=0; j < nfat; ++j) {
        fi->set_extent(arch[j].offset, arch[j].size);
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
        case PackMachFat::CPU_TYPE_POWERPC64LE: {
            PackMachPPC64LE packer(fi);
            if (!packer.canPack()) {
                PackDylibPPC64LE pack2r(fi);
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
    struct Mach_fat_arch const *const arch = &fat_head.arch[0];

    fi->readx(&fat_head, sizeof(fat_head));
    unsigned const nfat = check_fat_head();
    if (0 == nfat) {
        return false;
    }
    for (unsigned j=0; j < nfat; ++j) {
        fi->set_extent(arch[j].offset, arch[j].size);
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
        case PackMachFat::CPU_TYPE_POWERPC64LE: {
            PackMachPPC64LE packer(fi);
            if (!packer.canUnpack()) {
                PackDylibPPC64LE pack2r(fi);
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

/* vim:set ts=4 sw=4 et: */
