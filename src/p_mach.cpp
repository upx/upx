/* p_mach.cpp -- pack Mach Object executable

   This file is part of the UPX executable compressor.

   Copyright (C) 2004-2022 John Reiser
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
#define WANT_MACH_SEGMENT_ENUM
#define WANT_MACH_SECTION_ENUM
#include "p_mach_enum.h"
#include "p_mach.h"
#include "ui.h"

#if (ACC_CC_CLANG)
#  pragma clang diagnostic ignored "-Wcast-align"
#endif
#if (ACC_CC_GNUC >= 0x040200)
#  pragma GCC diagnostic ignored "-Wcast-align"
#endif

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
#include "stub/powerpc64-darwin.macho-entry.h"
static const
#include "stub/powerpc64-darwin.macho-fold.h"
static const
#include "stub/powerpc64-darwin.dylib-entry.h"

// Packing a Darwin (Mach-o) Mac OS X dylib (dynamic shared library)
// is restricted.  UPX gets control as the -init function, at the very
// end of processing by dyld.  Relocation, loading of dependent libraries,
// etc., already have taken place before decompression.  So the Mach-o
// headers, the __IMPORT segment, the __LINKEDIT segment, anything
// that is modifed by relocation, etc., cannot be compressed.
// We simplify arbitrarily by compressing only the __TEXT segment,
// which must be the first segment.

struct Lc_seg_info {
    unsigned char segment_cmd;
    unsigned char segcmdsize;
    unsigned char seccmdsize;
    unsigned char routines_cmd;
    unsigned char routinessize;
};
static const Lc_seg_info lc_seg_info[2] = {
    {LC_SEGMENT,     sizeof(Mach32_segment_command), sizeof(Mach32_section_command),
     LC_ROUTINES,    sizeof(Mach32_routines_command)},
    {LC_SEGMENT_64,  sizeof(Mach64_segment_command), sizeof(Mach64_section_command),
     LC_ROUTINES_64, sizeof(Mach64_routines_command)},
};

// Used to validate LC_ commands in order to defend against fuzzers.
// = 0 : illegal or unknown to us
// > 0 : actual size
// < 0 : neg. of minimum size; total must be (0 mod 4) or (0 mod 8)
//
static const signed char lc_cmd_size[] = {
// 2021-12: gcc 11.2.1 does not support 'sizeof' in designated initializer.
// 2021-12: gcc 11.2.1 does not support [enum] as designator.
// 2021-12: "clang++-10 -std=c++14":
//          error: array designators are a C99 extension [-Werror,-Wc99-designator]
// 2021-12: "Microsoft (R) C/C++ Optimizing Compiler Version 19.29.30138 for x64":
//          error C2143: syntax error: missing ']' before 'constant'
// Therefore, use the old brittle style with explicit consecutive enumeration.
// #define P(where, value) [(where)] = (value)
#   define P(where, value)             (value)
    P(0x00, 0),
    P(0x01 /*LC_SEGMENT*/, -56),  // see lc_seg_info[]
    P(0x02 /*LC_SYMTAB*/, 24), // sizeof(Mach32_symtab_command)
    P(0x03 /*LC_SYMSEG*/, 0), // obsolete
    P(0x04 /*LC_THREAD*/, -16), // uint32_t[4] + XXX_thread_state
    P(0x05 /*LC_UNIXTHREAD*/, -16), // uint32_t[4] + XXX_thread_state
    P(0x06 /*LC_LOADFVMLIB*/, 0),
    P(0x07 /*LC_IDFVMLIB*/, 0),
    P(0x08 /*LC_IDENT*/, 0), // obsolete
    P(0x09 /*LC_FVMFILE*/, 0), // Apple internal
    P(0x0a /*LC_PREPAGE*/, 0), // Apple internal
    P(0x0b /*LC_DYSYMTAB*/, 80), // sizeof(Mach32_dysymtab_command
    P(0x0c /*LC_LOAD_DYLIB*/, -24), // sizeof(dylib_command) + string
    P(0x0d /*LC_ID_DYLIB*/, -24), // sizeof(dylib_command) + string
    P(0x0e /*LC_LOAD_DYLINKER*/, -12), // sizeof(dylinker_command) + string
    P(0x0f /*LC_ID_DYLINKER*/, -12), // sizeof(dylinker_command) + string
    P(0x10 /*LC_PREBOUND_DYLIB*/, 0),
    P(0x11 /*LC_ROUTINES*/, 0),  // FIXME
    P(0x12 /*LC_SUB_FRAMEWORK*/, 0),
    P(0x13 /*LC_SUB_UMBRELLA*/, 0),
    P(0x14 /*LC_SUB_CLIENT*/, 0),
    P(0x15 /*LC_SUB_LIBRARY*/, 0),
    P(0x16 /*lC_TWOLEVEL_HINTS*/, -16), // sizeof(Mach32_twolevel_hints_command) + hints
    P(0x17 /*LC_PREBIND_CKSUM*/, 0),
    P(0x18 /*lo(LC_LOAD_WEAK_DYLIB)*/, -24), // sizeof(dylib_command) + string
    P(0x19 /*LC_SEGMENT_64*/, -72),  // see lc_seg_info[]
    P(0x1a /*LC_ROUTINES_64*/, 0),  // FIXME
    P(0x1b /*LC_UUID*/, 24), // sizeof(Mach32_uuid_command)
    P(0x1c /*LC_RPATH*/, -12), // sizeof(rpath_command) + string
    P(0x1d /*LC_CODE_SIGNATURE*/, 16), // sizeof(linkedit_data_command)
    P(0x1e /*LC_SEGMENT_SPLIT_INFO*/, 16), // sizeof(linkedit_data_command)
    P(0x1F /*lo(LC_REEXPORT_DYLIB)*/, -24), // sizeof(dylib_command) + string
    P(0x20 /*LC_LAZY_LOAD_DYLIB*/, 8), // ???
    P(0x21 /*LC_ENCRYPTION_INFO*/, 20), // sizeof(encryption_info_command)
    P(0x22 /*LC_DYLD_INFO*/, 48), // sizeof(dyld_info_command)
    P(0x23 /*LC_LOAD_UPWARD_DYLIB*/, 0),
    P(0x24 /*LC_VERSION_MIN_MACOSX*/, 16), // sizeof(Mach32_version_min_command)
    P(0x25 /*LC_VERSION_MIN_IPHONEOS*/, 16), // sizeof(Mach32_version_min_command)
    P(0x26 /*LC_FUNCTION_STARTS*/, 16), // sizeof(linkedit_data_command)
    P(0x27 /*LC_DYLD_ENVIRONMENT*/, -12), // sizeof(dylinker_command) + string
    P(0x28 /*lo(LC_MAIN)*/, 24), // sizeof(entry_point_command)
    P(0x29 /*LC_DATA_IN_CODE*/, 16), // sizeof(linkedit_data_command)
    P(0x2a /*LC_SOURCE_VERSION*/, 16), // sizeof(Mach32_source_version_command)
    P(0x2b /*LC_DYLIB_CODE_SIGN_DRS*/, 16), // sizeof(linkedit_data_command)
    P(0x2c /*LC_ENCRYPTION_INFO_64*/, 24), // sizeof(encryption_info_command_64)
    P(0x2d /*LC_LINKER_OPTION*/, 0),
    P(0x2e /*LC_LINKER_OPTIMIZATION_HINT*/, 0),
    P(0x2f /*LC_VERSION_MIN_TVOS*/, 16), // sizeof(Mach32_version_min_command)
    P(0x30 /*LC_VERSION_MIN_WATCHOS*/, 16), // sizeof(Mach32_version_min_command)
    P(0x31 /*LC_NOTE*/, -40), // sizeof(note_command) + data
    P(0x32 /*LC_BUILD_VERSION*/, -24), // sizeof(Mach32_build_version_command) + N*2*4
    P(0x33 /*lo(LC_DYLD_EXPORTS_TRIE)*/, 16), // sizeof(linkedit_data_command)
    P(0x34 /*lo(LC_DYLD_CHAINED_FIXUPS)*/, 16), // sizeof(linkedit_data_command)
    P(0x35 /*lo(LC_FILESET_ENTRY)*/, -32), // sizeof(fileset_entry_command) + ???
#undef P
};

static int is_bad_linker_command(
    unsigned cmd, unsigned cmdsize,
    unsigned headway, unsigned lc_seg, unsigned szAddr)
{
    cmd &= ~LC_REQ_DYLD;
   return !cmd  // there is no LC_ cmd 0
   || sizeof(lc_cmd_size) <= cmd  // beyond table of known sizes
   || !lc_cmd_size[cmd]  // obsolete, or proper size not known to us
   || !cmdsize || ((-1+ szAddr) & cmdsize)  // size not aligned
   || headway < cmdsize  // not within header area
   || (lc_seg == cmd  // lc_seg must have following lc_sections
       && (cmdsize - lc_seg_info[szAddr>>3].segcmdsize) %
                     lc_seg_info[szAddr>>3].seccmdsize)
   || (0 < lc_cmd_size[cmd] &&  lc_cmd_size[cmd] != (int)cmdsize)  // not known size
   || (0 > lc_cmd_size[cmd] && -lc_cmd_size[cmd]  > (int)cmdsize)  // below minimum size
   ;
}

#if 0 // NOT USED
static const unsigned lc_routines[2] = {
    0x11, 0x1a
    //Mach_command::LC_ROUTINES,
    //Mach_command::LC_ROUTINES_64
};
#endif

template <class T>
PackMachBase<T>::PackMachBase(InputFile *f, unsigned cputype, unsigned filetype,
        unsigned flavor, unsigned count, unsigned size, unsigned page_shift) :
    super(f), my_page_size(1ull<<page_shift), my_page_mask(~0ull<<page_shift),
    my_cputype(cputype), my_filetype(filetype), my_thread_flavor(flavor),
    my_thread_state_word_count(count), my_thread_command_size(size),
    n_segment(0), rawmseg(nullptr), msegcmd(nullptr), o__mod_init_func(0),
    prev_mod_init_func(0), pagezero_vmsize(0)
{
    MachClass::compileTimeAssertions();
    bele = N_BELE_CTP::getRTP((const BeLePolicy*) nullptr);
    memset(&cmdUUID, 0, sizeof(cmdUUID));
    memset(&cmdSRCVER, 0, sizeof(cmdSRCVER));
    memset(&cmdVERMIN, 0, sizeof(cmdVERMIN));
    memset(&linkitem, 0, sizeof(linkitem));
}

template <class T>
PackMachBase<T>::~PackMachBase()
{
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
PackDylibPPC64::PackDylibPPC64(InputFile *f) : super(f)
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
    // Un-aligned fetch does not work on 32-bit ARM, so use 8-bit methods
    return Packer::getDefaultCompressionMethods_8(method, level);
}

PackMachPPC32::PackMachPPC32(InputFile *f) : super(f, Mach_header::CPU_TYPE_POWERPC,
        Mach_header::MH_EXECUTE, Mach_thread_command::PPC_THREAD_STATE,
        sizeof(Mach_ppc_thread_state)>>2, sizeof(threado), 12)
{ }

PackMachPPC64::PackMachPPC64(InputFile *f) : super(f, Mach_header::CPU_TYPE_POWERPC64,
        Mach_header::MH_EXECUTE, Mach_thread_command::PPC_THREAD_STATE64,
        sizeof(Mach_ppc_thread_state64)>>2, sizeof(threado), 16)
{ }

const int *PackMachPPC32::getFilters() const
{
    static const int filters[] = { 0xd0, FT_END };
    return filters;
}

const int *PackMachPPC64::getFilters() const
{
    static const int filters[] = { 0xd0, FT_END };
    return filters;
}

PackMachI386::PackMachI386(InputFile *f) : super(f, Mach_header::CPU_TYPE_I386,
        Mach_header::MH_EXECUTE, (unsigned)Mach_thread_command::x86_THREAD_STATE32,
        sizeof(Mach_i386_thread_state)>>2, sizeof(threado), 12)
{ }

int const *PackMachI386::getFilters() const
{
    static const int filters[] = { 0x49, FT_END };
    return filters;
}

PackMachAMD64::PackMachAMD64(InputFile *f) : super(f, Mach_header::CPU_TYPE_X86_64,
        Mach_header::MH_EXECUTE, (unsigned)Mach_thread_command::x86_THREAD_STATE64,
        sizeof(Mach_AMD64_thread_state)>>2, sizeof(threado), 12)
{ }

int const *PackMachAMD64::getFilters() const
{
    static const int filters[] = { 0x49, FT_END };
    return filters;
}

PackMachARMEL::PackMachARMEL(InputFile *f) : super(f, Mach_header::CPU_TYPE_ARM,
        Mach_header::MH_EXECUTE, (unsigned)Mach_thread_command::ARM_THREAD_STATE,
        sizeof(Mach_ARM_thread_state)>>2, sizeof(threado), 12)
{ }

PackMachARM64EL::PackMachARM64EL(InputFile *f) : super(f, Mach_header::CPU_TYPE_ARM64,
        Mach_header::MH_EXECUTE, (unsigned)Mach_thread_command::ARM_THREAD_STATE64,
        sizeof(Mach_ARM64_thread_state)>>2, sizeof(threado), 14)
{ }

int const *PackMachARMEL::getFilters() const
{
    static const int filters[] = { 0x50, FT_END };
    return filters;
}

int const *PackMachARM64EL::getFilters() const
{
    static const int filters[] = { 0x52, FT_END };
    return filters;
}

Linker *PackMachPPC32::newLinker() const
{
    return new ElfLinkerPpc32;
}

Linker *PackMachPPC64::newLinker() const
{
    return new ElfLinkerPpc64;
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
    addLoader("MACOS000", nullptr);
   //addLoader(getDecompressorSections(), nullptr);
    addLoader(
        ( M_IS_NRV2E(ph.method) ? "NRV_HEAD,NRV2E,NRV_TAIL"
        : M_IS_NRV2D(ph.method) ? "NRV_HEAD,NRV2D,NRV_TAIL"
        : M_IS_NRV2B(ph.method) ? "NRV_HEAD,NRV2B,NRV_TAIL"
        : M_IS_LZMA(ph.method)  ? "LZMA_ELF00,LZMA_DEC20,LZMA_DEC30"
        : nullptr), nullptr);
    if (hasLoaderSection("CFLUSH"))
        addLoader("CFLUSH");
    addLoader("ELFMAINY,IDENTSTR,+40,ELFMAINZ,FOLDEXEC", nullptr);
}

void PackMachI386::addStubEntrySections(Filter const * /*ft*/)
{
    addLoader("MACHMAINX", nullptr);  // different for MY_DYLIB vs MH_EXECUTE
    if (my_filetype==Mach_header::MH_EXECUTE) {
        addLoader("MACH_UNC", nullptr);
    }
   //addLoader(getDecompressorSections(), nullptr);
    addLoader(
        ( M_IS_NRV2E(ph.method) ? "NRV_HEAD,NRV2E,NRV_TAIL"
        : M_IS_NRV2D(ph.method) ? "NRV_HEAD,NRV2D,NRV_TAIL"
        : M_IS_NRV2B(ph.method) ? "NRV_HEAD,NRV2B,NRV_TAIL"
        : M_IS_LZMA(ph.method)  ? "LZMA_ELF00,LZMA_DEC20,LZMA_DEC30"
        : nullptr), nullptr);
    if (hasLoaderSection("CFLUSH"))
        addLoader("CFLUSH");
    addLoader("MACHMAINY,IDENTSTR,+40,MACHMAINZ,FOLDEXEC", nullptr);
}

void PackMachAMD64::addStubEntrySections(Filter const * /*ft*/)
{
    addLoader("MACHMAINX", nullptr);  // different for MY_DYLIB vs MH_EXECUTE
    if (my_filetype==Mach_header::MH_EXECUTE) {
        addLoader("MACH_UNC", nullptr);
    }
   //addLoader(getDecompressorSections(), nullptr);
    addLoader(
        ( M_IS_NRV2E(ph.method) ? "NRV_HEAD,NRV2E,NRV_TAIL"
        : M_IS_NRV2D(ph.method) ? "NRV_HEAD,NRV2D,NRV_TAIL"
        : M_IS_NRV2B(ph.method) ? "NRV_HEAD,NRV2B,NRV_TAIL"
        : M_IS_LZMA(ph.method)  ? "LZMA_ELF00,LZMA_DEC20,LZMA_DEC30"
        : nullptr), nullptr);
    if (hasLoaderSection("CFLUSH"))
        addLoader("CFLUSH");
    addLoader("MACHMAINY,IDENTSTR,+40,MACHMAINZ,FOLDEXEC", nullptr);
}

void PackMachPPC32::addStubEntrySections(Filter const * /*ft*/)
{
    if (my_filetype!=Mach_header::MH_EXECUTE) {
        addLoader("MACHMAINX", nullptr);
    }
    else {
        addLoader("PPC32BXX", nullptr);
    }
    addLoader("MACH_UNC", nullptr);
   //addLoader(getDecompressorSections(), nullptr);
    addLoader(
        ( M_IS_NRV2E(ph.method) ? "NRV_HEAD,NRV2E,NRV_TAIL"
        : M_IS_NRV2D(ph.method) ? "NRV_HEAD,NRV2D,NRV_TAIL"
        : M_IS_NRV2B(ph.method) ? "NRV_HEAD,NRV2B,NRV_TAIL"
        : M_IS_LZMA(ph.method)  ? "LZMA_ELF00,LZMA_DEC20,LZMA_DEC30"
        : nullptr), nullptr);
    if (hasLoaderSection("CFLUSH"))
        addLoader("CFLUSH");
    addLoader("MACHMAINY,IDENTSTR,+40,MACHMAINZ", nullptr);
    if (my_filetype!=Mach_header::MH_EXECUTE) {
        addLoader("FOLDEXEC", nullptr);
    }
}

void PackMachARMEL::addStubEntrySections(Filter const * /*ft*/)
{
    addLoader("MACHMAINX", nullptr);
   //addLoader(getDecompressorSections(), nullptr);
    addLoader(
        ( M_IS_NRV2E(ph.method) ? "NRV_HEAD,NRV2E,NRV_TAIL"
        : M_IS_NRV2D(ph.method) ? "NRV_HEAD,NRV2D,NRV_TAIL"
        : M_IS_NRV2B(ph.method) ? "NRV_HEAD,NRV2B,NRV_TAIL"
        : M_IS_LZMA(ph.method)  ? "LZMA_ELF00,LZMA_DEC20,LZMA_DEC30"
        : nullptr), nullptr);
    if (hasLoaderSection("CFLUSH"))
        addLoader("CFLUSH");
    addLoader("MACHMAINY,IDENTSTR,+40,MACHMAINZ,FOLDEXEC", nullptr);
}

void PackMachARM64EL::addStubEntrySections(Filter const * /*ft*/)
{
    addLoader("MACHMAINX", nullptr);
   //addLoader(getDecompressorSections(), nullptr);
    addLoader(
        ( M_IS_NRV2E(ph.method) ? "NRV_HEAD,NRV2E,NRV_TAIL"
        : M_IS_NRV2D(ph.method) ? "NRV_HEAD,NRV2D,NRV_TAIL"
        : M_IS_NRV2B(ph.method) ? "NRV_HEAD,NRV2B,NRV_TAIL"
        : M_IS_LZMA(ph.method)  ? "LZMA_ELF00,LZMA_DEC20,LZMA_DEC30"
        : nullptr), nullptr);
    if (hasLoaderSection("CFLUSH"))
        addLoader("CFLUSH");
    addLoader("MACHMAINY,IDENTSTR,+40,MACHMAINZ,FOLDEXEC", nullptr);
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

    MemBuffer cprLoader_buf(sizeof(h) + h.sz_unc);
    unsigned char *const cprLoader = (unsigned char *)cprLoader_buf.getVoidPtr();
  if (0 < szfold) {
    unsigned sz_cpr = 0;
    int r = upx_compress(uncLoader, h.sz_unc, sizeof(h) + cprLoader, &sz_cpr,
        nullptr, ph.method, 10, nullptr, nullptr );
    h.sz_cpr = sz_cpr;
    if (r != UPX_E_OK || h.sz_cpr >= h.sz_unc)
        throwInternalError("loader compression failed");
  }
    memcpy(cprLoader, &h, sizeof(h));

    // This adds the definition to the "library", to be used later.
    linker->addSection("FOLDEXEC", cprLoader, sizeof(h) + h.sz_cpr, 0);

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
    unsigned const lc_seg = lc_seg_info[sizeof(Addr)>>3].segment_cmd;
    unsigned const xa = a->cmd - lc_seg;
    unsigned const xb = b->cmd - lc_seg;
           if (xa < xb)        return -1;  // LC_SEGMENT first
           if (xa > xb)        return  1;
           if (0 != xa)        return  0;  // not LC_SEGMENT
    // Ascending by .fileoff so that find_SEGMENT_gap works
    if (a->fileoff < b->fileoff)
                               return -1;
    if (a->fileoff > b->fileoff)
                               return  1;
    // Ascending by .vmaddr
    if (a->vmaddr < b->vmaddr) return -1;
    if (a->vmaddr > b->vmaddr) return  1;
    // Descending by .vmsize
    if (a->vmsize)             return -1;  // 'a' is first
    if (b->vmsize)             return  1;  // 'a' is last
    // What could remain?
    return 0;
}

// At 2013-02-03 part of the source for codesign was
//    http://opensource.apple.com/source/cctools/cctools-836/libstuff/ofile.c

unsigned const blankLINK = 16;  // size of our empty __LINK segment
// Note: "readelf --segments"  ==>  "otool -hl" or "otool -hlv" etc. (Xcode on MacOS)

template <class T>
void PackMachBase<T>::pack4(OutputFile *fo, Filter &ft)  // append PackHeader
{
    // offset of p_info in compressed file
    overlay_offset = secTEXT.offset + sizeof(linfo);
    super::pack4(fo, ft);

    if (Mach_header::MH_EXECUTE == my_filetype) {
        unsigned len = fo->getBytesWritten();
        MemBuffer page(my_page_size); memset(page, 0, my_page_size);
        fo->write(page, ~my_page_mask & (0u - len));
        len +=          ~my_page_mask & (0u - len) ;

        segTEXT.filesize = len;
        segTEXT.vmsize   = len;  // FIXME?  utilize GAP + NO_LAP + sz_unc - sz_cpr
        secTEXT.offset = overlay_offset - sizeof(linfo);
        secTEXT.addr = segTEXT.vmaddr   + secTEXT.offset;
        secTEXT.size = segTEXT.filesize - secTEXT.offset;
        secXHDR.offset = overlay_offset - sizeof(linfo);
        if (my_filetype==Mach_header::MH_EXECUTE) {
            secXHDR.offset -= sizeof(linkitem);
        }
        secXHDR.addr += secXHDR.offset;
        unsigned offLINK = segLINK.fileoff;


        segLINK.fileoff = len;  // must be in the file
        segLINK.vmaddr =  len + segTEXT.vmaddr;
        fo->write(page, blankLINK); len += blankLINK;
        segLINK.vmsize = my_page_size;
        segLINK.filesize = blankLINK;

        // Get a writeable copy of the stub to make editing easier.
        ByteArray(upxstub, sz_stub_main);
        memcpy(upxstub, stub_main, sz_stub_main);

        Mach_header *const mhp = (Mach_header *)upxstub;
        mhp->cpusubtype = my_cpusubtype;
        mhp->flags = mhdro.flags;
        char *tail = (char *)(1+ mhp);
        char *const lcp_end = mhdro.sizeofcmds + tail;
        Mach_command *lcp = (Mach_command *)(1+ mhp);
        Mach_command *lcp_next;
        unsigned const ncmds = mhdro.ncmds;
        //unsigned cmdsize = mhdro.sizeofcmds;
        unsigned delta = 0;

    for (unsigned j = 0; j < ncmds; ++j) {
        unsigned skip = 0;
        unsigned sz_cmd = lcp->cmdsize;
        lcp_next = (Mach_command *)(sz_cmd + (char *)lcp);

        switch (lcp->cmd) {
        case Mach_command::LC_SEGMENT: // fall through
        case Mach_command::LC_SEGMENT_64: {
            Mach_segment_command *const segptr = (Mach_segment_command *)lcp;
            if (!strcmp("__PAGEZERO", segptr->segname)) {
                segptr->vmsize = pagezero_vmsize;
            }
            if (!strcmp("__TEXT", segptr->segname)) {
                sz_cmd = (segTEXT.nsects * sizeof(secTEXT)) + sizeof(segTEXT);
                mhp->sizeofcmds += sizeof(secTEXT) * (1 - segptr->nsects);
                memcpy(tail, &segTEXT, sz_cmd); tail += sz_cmd;
                goto next;
            }
            if (!strcmp("__LINKEDIT", segptr->segname)) {
                segLINK.initprot = Mach_command::VM_PROT_READ;
                delta = offLINK - segptr->fileoff;  // relocation constant

                sz_cmd = sizeof(segLINK);
                if (Mach_header::CPU_TYPE_I386==mhdri.cputype
                &&  Mach_header::MH_EXECUTE==mhdri.filetype) {
                    segLINK.maxprot = 0
                        | Mach_command::VM_PROT_EXECUTE
                        | Mach_command::VM_PROT_WRITE
                        | Mach_command::VM_PROT_READ;
                    segLINK.initprot = 0
                        | Mach_command::VM_PROT_WRITE
                        | Mach_command::VM_PROT_READ;
                }
                memcpy(tail, &segLINK, sz_cmd); tail += sz_cmd;
                goto next;
            }
        } break;
        case Mach_command::LC_DYLD_INFO_ONLY: {
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
        case Mach_command::LC_SYMTAB: {
            // Apple codesign requires that string table is last in the file.
            Mach_symtab_command *p = (Mach_symtab_command *)lcp;
            p->symoff = segLINK.filesize + segLINK.fileoff;
            p->nsyms = 0;
            p->stroff = segLINK.fileoff;
            p->strsize = segLINK.filesize;
            skip = 1;
        } break;
        case Mach_command::LC_DYSYMTAB: {
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
        case Mach_command::LC_MAIN: {
                // Replace later with LC_UNIXTHREAD.
// LC_MAIN requires libSystem.B.dylib to provide the environment for main(), and CALLs the entryoff.
// LC_UNIXTHREAD does not need libSystem.B.dylib, and JMPs to the .rip with %rsp/argc and argv= 8+%rsp
            threado_setPC(segTEXT.vmaddr +
                (((Mach_main_command const *)lcp)->entryoff - segTEXT.fileoff));
            skip = 1;
        } break;
        case Mach_command::LC_UNIXTHREAD: { // pre-LC_MAIN
            skip = 1;
        } break;
        case Mach_command::LC_LOAD_DYLIB: {
            skip = 1;
        } break;

        case Mach_command::LC_FUNCTION_STARTS:
        case Mach_command::LC_DATA_IN_CODE: {
            Mach_linkedit_data_command *p = (Mach_linkedit_data_command *)lcp;
            if (p->dataoff) p->dataoff += delta;
            skip = 1;
        } break;
        case Mach_command::LC_LOAD_DYLINKER: {
            skip = 1;
        } break;
        case Mach_command::LC_SOURCE_VERSION: { // copy from saved original
            memcpy(lcp, &cmdSRCVER, sizeof(cmdSRCVER));
            if (Mach_command::LC_SOURCE_VERSION != cmdSRCVER.cmd) {
                skip = 1;  // was not seen
            }
        } break;
        case Mach_command::LC_VERSION_MIN_MACOSX: { // copy from saved original
            memcpy(lcp, &cmdVERMIN, sizeof(cmdVERMIN));
            if (Mach_command::LC_VERSION_MIN_MACOSX != cmdVERMIN.cmd) {
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

// At 2013-02-03 part of the source for codesign was:
//    http://opensource.apple.com/source/cctools/cctools-836/libstuff/ofile.c

template <class T>
void PackMachBase<T>::pack4dylib(  // append PackHeader
    OutputFile *const fo,
    Filter &ft,
    Addr init_address
)
{
    fo->seek(0, SEEK_SET);
    fo->rewrite(&mhdro, sizeof(mhdro));  // segTEXT.nsect=1 (only secTEXT)
    fo->rewrite(&segTEXT, sizeof(segTEXT));  // .vmsize
    unsigned opos = sizeof(mhdro);

    // Append each non-__TEXT segment, page aligned.
    int slide = 0;
    unsigned o_end_txt = 0;
    unsigned hdrpos = sizeof(mhdro);
    Mach_segment_command const *seg = rawmseg;
    Mach_segment_command const *const endseg =
        (Mach_segment_command const *)(mhdri.sizeofcmds + (char const *)seg);
    for ( ; seg < endseg; seg = (Mach_segment_command const *)(
            seg->cmdsize + (char const *)seg )
    ) switch (seg->cmd & ~Mach_command::LC_REQ_DYLD) {
    default:  // unknown if any file offset field must slide
        fprintf(stderr, "Unrecognized Macho cmd  offset=0x%lx  cmd=0x%lx  size=0x%lx\n",
            (unsigned long)((const char *)seg - (const char *)rawmseg),
            (unsigned long)seg->cmd, (unsigned long)seg->cmdsize);
        // fall through
    case Mach_command::LC_VERSION_MIN_MACOSX:
    case Mach_command::LC_SOURCE_VERSION:
    case Mach_command::LC_THREAD:
    case Mach_command::LC_UNIXTHREAD:
    case Mach_command::LC_LOAD_DYLIB:
    case Mach_command::LC_ID_DYLIB:
    case Mach_command::LC_LOAD_DYLINKER:
    case Mach_command::LC_UUID:
    case Mach_command::LC_RPATH:
    case Mach_command::LC_REEXPORT_DYLIB: { // contain no file offset fields
        fo->seek(hdrpos, SEEK_SET);
        fo->rewrite(seg, seg->cmdsize);
        hdrpos += seg->cmdsize;
    } break;

    case Mach_command::LC_CODE_SIGNATURE:
    case Mach_command::LC_SEGMENT_SPLIT_INFO:
    case Mach_command::LC_DYLIB_CODE_SIGN_DRS:
    case Mach_command::LC_DATA_IN_CODE:
    case Mach_command::LC_FUNCTION_STARTS: {
        Mach_linkedit_data_command cmd; memcpy(&cmd, seg, sizeof(cmd));
        if (o_end_txt <= cmd.dataoff) { cmd.dataoff += slide; }
        fo->seek(hdrpos, SEEK_SET);
        fo->rewrite(&cmd, sizeof(cmd));
        hdrpos += sizeof(cmd);
    } break;
    case Mach_command::LC_DYLD_INFO_ONLY & ~Mach_command::LC_REQ_DYLD: {
        Mach_dyld_info_only_command cmd; memcpy(&cmd, seg, sizeof(cmd));
        if (o_end_txt <= cmd.rebase_off)    { cmd.rebase_off    += slide; }
        if (o_end_txt <= cmd.bind_off)      { cmd.bind_off      += slide; }
        if (o_end_txt <= cmd.weak_bind_off) { cmd.weak_bind_off += slide; }
        if (o_end_txt <= cmd.lazy_bind_off) { cmd.lazy_bind_off += slide; }
        if (o_end_txt <= cmd.export_off)    { cmd.export_off    += slide; }
        fo->seek(hdrpos, SEEK_SET);
        fo->rewrite(&cmd, sizeof(cmd));
        hdrpos += sizeof(cmd);
    } break;
    case Mach_command::LC_TWOLEVEL_HINTS: {
        Mach_twolevel_hints_command cmd; memcpy(&cmd, seg, sizeof(cmd));
        if (o_end_txt <= cmd.offset) { cmd.offset += slide; }
        fo->seek(hdrpos, SEEK_SET);
        fo->rewrite(&cmd, sizeof(cmd));
        hdrpos += sizeof(cmd);
    } break;
    case Mach_command::LC_ROUTINES_64:
    case Mach_command::LC_ROUTINES: {
        Mach_routines_command cmd; memcpy(&cmd, seg, sizeof(cmd));
        cmd.reserved1 = cmd.init_address;
        cmd.init_address = init_address;
        fo->seek(hdrpos, SEEK_SET);
        fo->rewrite(&cmd, sizeof(cmd));
        hdrpos += sizeof(cmd);
    } break;
    case Mach_command::LC_SEGMENT_64:
    case Mach_command::LC_SEGMENT: {
        // non-__TEXT might be observed and relocated by dyld before us.
        Mach_segment_command segcmdtmp = *seg;
        bool const is_text = 0==strncmp(&seg->segname[0], "__TEXT", 1+ 6);
        {
            if (is_text) {
                slide = 0;
                segTEXT.vmsize = segTEXT.filesize = fo->getBytesWritten();
                segTEXT.maxprot  |= Mach_command::VM_PROT_WRITE;
                segcmdtmp = segTEXT;
                opos = o_end_txt = segcmdtmp.filesize + segcmdtmp.fileoff;
            }
            else {
                opos += ~my_page_mask & (0u - opos);  // advance to my_page_size boundary
                slide = opos - segcmdtmp.fileoff;
                segcmdtmp.fileoff = opos;
            }

            fo->seek(hdrpos, SEEK_SET);
            fo->rewrite(&segcmdtmp, sizeof(segcmdtmp));
            hdrpos += sizeof(segcmdtmp);

            // Update the sections.
            Mach_section_command const *secp =
                (Mach_section_command const *)(const void*)(const char*)(1+ seg);
            if (is_text) {
                secTEXT.offset = secp->offset;
                secTEXT.addr = segTEXT.vmaddr   + secTEXT.offset;
                secTEXT.size = segTEXT.filesize - secTEXT.offset;
                secp = &secTEXT;
            }
            unsigned const nsects = (is_text ? 1 : segcmdtmp.nsects);
            Mach_section_command seccmdtmp;
            for (unsigned j = 0; j < nsects; ++secp, ++j) {
                seccmdtmp = *secp;
                if (o_end_txt <= seccmdtmp.offset) { seccmdtmp.offset += slide; }
                if (o_end_txt <= seccmdtmp.reloff) { seccmdtmp.reloff += slide; }
                fo->rewrite(&seccmdtmp, sizeof(seccmdtmp));
                hdrpos += sizeof(seccmdtmp);
            }

            if (!is_text) {
                unsigned const len = seg->filesize;
                MemBuffer data(len);
                fi->seek(seg->fileoff, SEEK_SET);
                fi->readx(data, len);
                unsigned const pos = o__mod_init_func - seg->fileoff;
                if (pos < seg->filesize) {
                    if (*(unsigned *)(pos + data) != (unsigned)prev_mod_init_func) {
                        throwCantPack("__mod_init_func inconsistent");
                    }
                    *(unsigned *)(pos + data) = (unsigned)entryVMA;
                }
                fo->seek(opos, SEEK_SET);
                fo->write(data, len);
                opos += len;
            }
        }
    } break;
    case Mach_command::LC_SYMTAB: {
        Mach_symtab_command cmd; memcpy(&cmd, seg, sizeof(cmd));
        if (o_end_txt <= cmd.symoff) { cmd.symoff += slide; }
        if (o_end_txt <= cmd.stroff) { cmd.stroff += slide; }
        fo->seek(hdrpos, SEEK_SET);
        fo->rewrite(&cmd, sizeof(cmd));
        hdrpos += sizeof(cmd);
    } break;
    case Mach_command::LC_DYSYMTAB: {
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
    }  // end 'switch'
    fo->seek(opos, SEEK_SET);  // BUG: "fo->seek(0, SEEK_END);" is broken

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

void PackDylibPPC64::pack4(OutputFile *fo, Filter &ft)  // append PackHeader
{
    pack4dylib(fo, ft, threado.state64.srr0);
}

template <class T>
off_t PackMachBase<T>::pack3(OutputFile *fo, Filter &ft)  // append loader
{
    TE32 disp;
    upx_uint64_t const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 3& (0u-len));
    len += (3& (0u-len));

    disp = len;  // backward offset to Mach_header
    fo->write(&disp, sizeof(disp));
    len += sizeof(disp);

    if (my_filetype!=Mach_header::MH_DYLIB) {
        disp = len - sz_mach_headers;  // backward offset to start of compressed data
        fo->write(&disp, sizeof(disp));
        len += sizeof(disp);
    }
    segTEXT.vmsize = segTEXT.filesize;
    threado_setPC(entryVMA= len + segTEXT.vmaddr);

    return super::pack3(fo, ft);
}

off_t PackDylibI386::pack3(OutputFile *fo, Filter &ft)  // append loader
{
    TE32 disp;
    upx_uint32_t const zero = 0;
    off_t len = fo->getBytesWritten();
    fo->write(&zero, 3& (0u-len));
    len += (3& (0u-len)) + 4*sizeof(disp);

    disp = prev_mod_init_func;
    fo->write(&disp, sizeof(disp));  // user .init_address

    disp = secTEXT.offset + sizeof(l_info) + sizeof(p_info);
    fo->write(&disp, sizeof(disp));  // src offset(compressed __TEXT)

    disp = len - disp - 3*sizeof(disp);
    fo->write(&disp, sizeof(disp));  // length(compressed __TEXT)

    unsigned const save_sz_mach_headers(sz_mach_headers);
    sz_mach_headers = 0;
    len = super::pack3(fo, ft);
    sz_mach_headers = save_sz_mach_headers;
    return len;
}

off_t PackDylibAMD64::pack3(OutputFile *fo, Filter &ft)  // append loader
{
    TE32 disp;
    upx_uint64_t const zero = 0;
    off_t len = fo->getBytesWritten();
    fo->write(&zero, 3& (0u-len));
    len += (3& (0u-len)) + 3*sizeof(disp);

    disp = prev_mod_init_func;
    fo->write(&disp, sizeof(disp));  // user .init_address

    disp = secTEXT.offset + sizeof(l_info) + sizeof(p_info);
    fo->write(&disp, sizeof(disp));  // src offset(b_info)

    disp = rawmseg[0].vmsize;
    fo->write(&disp, sizeof(disp));  // __TEXT.vmsize when expanded

    unsigned const save_sz_mach_headers(sz_mach_headers);
    sz_mach_headers = 0;
    len = super::pack3(fo, ft);
    sz_mach_headers = save_sz_mach_headers;
    return len;
}

off_t PackDylibPPC32::pack3(OutputFile *fo, Filter &ft)  // append loader
{
    TE32 disp;
    upx_uint32_t const zero = 0;
    off_t len = fo->getBytesWritten();
    fo->write(&zero, 3& (0u-len));
    len += (3& (0u-len)) + 4*sizeof(disp);

    disp = prev_mod_init_func;
    fo->write(&disp, sizeof(disp));  // user .init_address

    disp = secTEXT.offset + sizeof(l_info) + sizeof(p_info);
    fo->write(&disp, sizeof(disp));  // src offset(compressed __TEXT)

    disp = len - disp - 3*sizeof(disp);
    fo->write(&disp, sizeof(disp));  // length(compressed __TEXT)

    unsigned const save_sz_mach_headers(sz_mach_headers);
    sz_mach_headers = 0;
    len = super::pack3(fo, ft);
    sz_mach_headers = save_sz_mach_headers;
    return len;
}

off_t PackDylibPPC64::pack3(OutputFile *fo, Filter &ft)  // append loader
{
    TE64 disp;
    upx_uint64_t const zero = 0;
    off_t len = fo->getBytesWritten();
    fo->write(&zero, 3& (0u-len));
    len += (3& (0u-len)) + 4*sizeof(disp);

    disp = prev_mod_init_func;
    fo->write(&disp, sizeof(disp));  // user .init_address

    disp = secTEXT.offset + sizeof(l_info) + sizeof(p_info);
    fo->write(&disp, sizeof(disp));  // src offset(compressed __TEXT)

    disp = len - disp - 3*sizeof(disp);
    fo->write(&disp, sizeof(disp));  // length(compressed __TEXT)

    unsigned const save_sz_mach_headers(sz_mach_headers);
    sz_mach_headers = 0;
    len = super::pack3(fo, ft);
    sz_mach_headers = save_sz_mach_headers;
    return len;
}

// Determine length of gap between PT_LOAD phdri[k] and closest PT_LOAD
// which follows in the file (or end-of-file).  Optimize for common case
// where the PT_LOAD are adjacent ascending by .p_offset.  Assume no overlap.

template <class T>
unsigned PackMachBase<T>::find_SEGMENT_gap(
    unsigned const k, unsigned pos_eof
)
{
    unsigned const lc_seg = lc_seg_info[sizeof(Addr)>>3].segment_cmd;
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
    unsigned const lc_seg = lc_seg_info[sizeof(Addr)>>3].segment_cmd;
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
    total_in = 0;
    total_out = 0;

    unsigned hdr_u_len = mhdri.sizeofcmds + sizeof(mhdri);

    uip->ui_pass = 0;
    ft.addvalue = 0;

    // Packer::compressWithFilters chooses a filter for us, and the stubs
    // can handle only one filter, and most filters are for executable
    // instructions.  So filter only the largest executable segment.
    unsigned exe_filesize_max = 0;
    for (k = 0; k < n_segment; ++k)
    if (lc_seg==msegcmd[k].cmd
    &&  0!=(Mach_command::VM_PROT_EXECUTE & msegcmd[k].initprot)
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
            && 0!=(Mach_command::VM_PROT_EXECUTE & msegcmd[k].initprot);
        Mach_segment_command const *ptr = rawmseg;
        unsigned b_extra = 0;
        for (unsigned j= 0; j < mhdri.ncmds; ++j) {
            if (msegcmd[k].cmd    == ptr->cmd
            &&  msegcmd[k].vmaddr == ptr->vmaddr
            &&  msegcmd[k].vmsize == ptr->vmsize) {
                b_extra = j;
                break;
            }
            ptr = (Mach_segment_command const *)(ptr->cmdsize + (char const *)ptr);
        }
        packExtent(x,
            (do_filter ? &ft : nullptr), fo, hdr_u_len, b_extra );
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
            packExtent(x, nullptr, fo);
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
    threado.cmd = Mach_command::LC_UNIXTHREAD;
    threado.cmdsize = sizeof(threado);
    threado.flavor = my_thread_flavor;
    threado.count =  my_thread_state_word_count;
    memset(&threado.state, 0, sizeof(threado.state));
    fo->write(&threado, sizeof(threado));
}

void PackMachPPC64::pack1_setup_threado(OutputFile *const fo)
{
    threado.cmd = Mach_command::LC_UNIXTHREAD;
    threado.cmdsize = sizeof(threado);
    threado.flavor = my_thread_flavor;
    threado.count =  my_thread_state_word_count;
    memset(&threado.state64, 0, sizeof(threado.state64));
    fo->write(&threado, sizeof(threado));
}

void PackMachI386::pack1_setup_threado(OutputFile *const fo)
{
    threado.cmd = Mach_command::LC_UNIXTHREAD;
    threado.cmdsize = sizeof(threado);
    threado.flavor = my_thread_flavor;
    threado.count =  my_thread_state_word_count;
    memset(&threado.state, 0, sizeof(threado.state));
    fo->write(&threado, sizeof(threado));
}

void PackMachAMD64::pack1_setup_threado(OutputFile *const fo)
{
    threado.cmd = Mach_command::LC_UNIXTHREAD;
    threado.cmdsize = sizeof(threado);
    threado.flavor = my_thread_flavor;
    threado.count =  my_thread_state_word_count;
    memset(&threado.state, 0, sizeof(threado.state));
    fo->write(&threado, sizeof(threado));
}

void PackMachARMEL::pack1_setup_threado(OutputFile *const fo)
{
    threado.cmd = Mach_command::LC_UNIXTHREAD;
    threado.cmdsize = sizeof(threado);
    threado.flavor = my_thread_flavor;
    threado.count =  my_thread_state_word_count;
    memset(&threado.state, 0, sizeof(threado.state));
    fo->write(&threado, sizeof(threado));
}

void PackMachARM64EL::pack1_setup_threado(OutputFile *const fo)
{
    threado.cmd = Mach_command::LC_UNIXTHREAD;
    threado.cmdsize = sizeof(threado);
    threado.flavor = my_thread_flavor;
    threado.count =  my_thread_state_word_count;
    memset(&threado.state, 0, sizeof(threado.state));
    fo->write(&threado, sizeof(threado));
}

template <class T>
void PackMachBase<T>::pack1(OutputFile *const fo, Filter &/*ft*/)  // generate executable header
{
    unsigned const lc_seg = lc_seg_info[sizeof(Addr)>>3].segment_cmd;
    mhdro = mhdri;
    if (my_filetype==Mach_header::MH_EXECUTE) {
        memcpy(&mhdro, stub_main, sizeof(mhdro));
        mhdro.flags = mhdri.flags & ~(
              Mach_header::MH_DYLDLINK  // no dyld at this time
            | Mach_header::MH_TWOLEVEL  // dyld-specific
            | Mach_header::MH_BINDATLOAD  // dyld-specific
            );
        COMPILE_TIME_ASSERT(sizeof(mhdro.flags) == sizeof(unsigned))
    }
    unsigned pos = sizeof(mhdro);
    fo->write(&mhdro, sizeof(mhdro));

    memset(&segZERO, 0, sizeof(segZERO));
    segZERO.cmd = lc_seg;
    segZERO.cmdsize = sizeof(segZERO);
    strncpy((char *)segZERO.segname, "__PAGEZERO", sizeof(segZERO.segname));
    segZERO.vmsize = pagezero_vmsize;

    segTEXT.cmd = lc_seg;
    segTEXT.cmdsize = sizeof(segTEXT) + sizeof(secTEXT);
    strncpy((char *)segTEXT.segname, "__TEXT", sizeof(segTEXT.segname));
    if (my_filetype==Mach_header::MH_EXECUTE) {
        if (Mach_header::MH_PIE & mhdri.flags) {
            segTEXT.vmaddr = segZERO.vmsize;  // contiguous
        }
        else { // not MH_PIE
            // Start above all eventual mappings.
            // Cannot enlarge segZERO.vmsize because MacOS 10.13 (HighSierra)
            // won't permit re-map of PAGEZERO.
            // Stub will fill with PROT_NONE first.
            segTEXT.vmaddr = vma_max;
        }
    }
    if (my_filetype==Mach_header::MH_DYLIB) {
        segTEXT.vmaddr = 0;
    }
    segTEXT.vmsize = 0;    // adjust later
    segTEXT.fileoff = 0;
    segTEXT.filesize = 0;  // adjust later
    segTEXT.maxprot =
        Mach_command::VM_PROT_READ |
        Mach_command::VM_PROT_WRITE |
        Mach_command::VM_PROT_EXECUTE;
    segTEXT.initprot =
        Mach_command::VM_PROT_READ |
        Mach_command::VM_PROT_EXECUTE;
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
    segXHDR.vmsize = my_page_size;
    segXHDR.filesize = my_page_size;
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
    segLINK.initprot = Mach_command::VM_PROT_READ;
    segLINK.nsects = 0;
    segLINK.vmsize = 0;
    // Adjust later: .vmaddr .vmsize .fileoff .filesize

    unsigned gap = 0;
    if (my_filetype == Mach_header::MH_EXECUTE) {
        unsigned cmdsize = mhdro.sizeofcmds;
        Mach_header const *const ptr0 = (Mach_header const *)stub_main;
        Mach_command const *ptr1 = (Mach_command const *)(1+ ptr0);
        for (unsigned j = 0; j < mhdro.ncmds -1; ++j, (cmdsize -= ptr1->cmdsize),
                ptr1 = (Mach_command const *)(ptr1->cmdsize + (char const *)ptr1)) {
            if (lc_seg == ptr1->cmd) {
                Mach_segment_command const *const segptr = (Mach_segment_command const *)ptr1;
                Mach_section_command const *const secptr = (Mach_section_command const *)(1+ segptr);
                if (!strcmp("__TEXT", segptr->segname)) {
                    strncpy((char *)secTEXT.segname,   "__TEXT", sizeof(secTEXT.segname));
                    strncpy((char *)secTEXT.sectname, "upxTEXT", sizeof(secTEXT.sectname));
                    secTEXT.addr   = secptr->addr;
                    secTEXT.size   = secptr->size;  // update later
                    secTEXT.offset = secptr->offset;
                    secTEXT.align  = secptr->align;
                }
                if (!strcmp("__LINKEDIT", segptr->segname)) {
                    // Mach_command before __LINKEDIT
                    pos += (char const *)ptr1 - (char const *)(1+ ptr0);
                    fo->write((1+ ptr0), (char const *)ptr1 - (char const *)(1+ ptr0));

                    // Mach_command __LINKEDIT and after
                    pos += cmdsize;
                    fo->write((char const *)ptr1, cmdsize);

                    // 400: space for LC_UUID, LC_RPATH, LC_CODE_SIGNATURE, etc.
                    gap = 400 + threado_size();
                    secTEXT.offset = gap + pos;
                    secTEXT.addr = secTEXT.offset + segTEXT.vmaddr;
                    break;
                }
            }
        }
        unsigned const sz_threado = threado_size();
        MemBuffer space(sz_threado); memset(space, 0, sz_threado);
        fo->write(space, sz_threado);
    }
    else if (my_filetype == Mach_header::MH_DYLIB) {
        Mach_command const *ptr = (Mach_command const *)rawmseg;
        unsigned cmdsize = mhdri.sizeofcmds;
        for (unsigned j = 0; j < mhdri.ncmds; ++j, (cmdsize -= ptr->cmdsize),
                ptr = (Mach_command const *)(ptr->cmdsize + (char const *)ptr)) {
            if (lc_seg == ptr->cmd) {
                Mach_segment_command const *const segptr = (Mach_segment_command const *)ptr;
                Mach_section_command const *const secptr = (Mach_section_command const *)(1+ segptr);
                if (!strcmp("__TEXT", segptr->segname)) {
                    if (!(1 <= segptr->nsects)) {
                        throwCantPack("TEXT.nsects == 0");
                    }
                    strncpy((char *)secTEXT.sectname, "upxTEXT", sizeof(secTEXT.sectname));
                    secTEXT.addr   = secptr->addr;
                    secTEXT.size   = secptr->size;  // update later
                    secTEXT.offset = secptr->offset;
                    secTEXT.align  = secptr->align;
                    fo->write(&segTEXT, sizeof(segTEXT));
                    fo->write(&secTEXT, sizeof(secTEXT));
                }
                else { // not __TEXT
                    fo->write(ptr, ptr->cmdsize);
                }
            }
            else { // not LC_SEGMENT*
                fo->write(ptr, ptr->cmdsize);
            }
        }
        memset(&linkitem, 0, sizeof(linkitem));
        fo->write(&linkitem, sizeof(linkitem));
    }
    sz_mach_headers = fo->getBytesWritten();
    gap = secTEXT.offset - sz_mach_headers;
    MemBuffer filler(gap); filler.clear();
    fo->write(filler, gap);
    sz_mach_headers += gap;

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
    unsigned const lc_seg = lc_seg_info[sizeof(Addr)>>3].segment_cmd;
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
    rawmseg_buf.dealloc();  // discard "same" contents from ::canUnpack()
    rawmseg_buf.alloc(sz_cmds);
    rawmseg = (Mach_segment_command *)rawmseg_buf.getVoidPtr();
    fi->readx(rawmseg, mhdri.sizeofcmds);

    // FIXME forgot space left for LC_CODE_SIGNATURE;
    // but canUnpack() sets overlay_offset anyway.
    //overlay_offset = sizeof(mhdri) + mhdri.sizeofcmds + sizeof(linfo);

    fi->seek(overlay_offset, SEEK_SET);
    p_info hbuf;
    fi->readx(&hbuf, sizeof(hbuf));
    unsigned const orig_file_size = get_te32(&hbuf.p_filesize);
    blocksize = get_te32(&hbuf.p_blocksize);  // emacs-21.2.1 was 0x01d47e6c (== 30703212)
    if (blocksize > orig_file_size || blocksize > UPX_RSIZE_MAX_MEM)
        throwCantUnpack("file header corrupted");
    if (file_size > (off_t)orig_file_size) {
        opt->info_mode += !opt->info_mode ? 1 : 0;  // make visible
        opt->backup = 1;
        infoWarning("packed size too big; discarding appended data, keeping backup");
    }

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
    MemBuffer mhdr_buf(ph.u_len);
    Mach_header *const mhdr = (Mach_header *)mhdr_buf.getVoidPtr();
    decompress(ibuf, (upx_byte *)mhdr, false);
    if (mhdri.magic      != mhdr->magic
    ||  mhdri.cputype    != mhdr->cputype
    ||  mhdri.cpusubtype != mhdr->cpusubtype
    ||  mhdri.filetype   != mhdr->filetype)
        throwCantUnpack("file header corrupted");
    unsigned const ncmds = mhdr->ncmds;
    if (!ncmds || 24 < ncmds) { // arbitrary limit
        char msg[40]; snprintf(msg, sizeof(msg),
            "bad Mach_header.ncmds = %d", ncmds);
        throwCantUnpack(msg);
    }

    msegcmd_buf.alloc(sizeof(Mach_segment_command) * ncmds);
    msegcmd = (Mach_segment_command *)msegcmd_buf.getVoidPtr();
    unsigned char const *ptr = (unsigned char const *)(1+mhdr);
    unsigned headway = mhdr_buf.getSize() - sizeof(*mhdr);
    for (unsigned j= 0; j < ncmds; ++j) {
        unsigned cmdsize = ((Mach_command const *)ptr)->cmdsize;
        if (is_bad_linker_command( ((Mach_command const *)ptr)->cmd, cmdsize,
                headway, lc_seg, sizeof(Addr))) {
            char msg[50]; snprintf(msg, sizeof(msg),
                "bad packed Mach load_command @%#x", ptr_udiff_bytes(ptr, mhdr));
            throwCantUnpack(msg);
        }
        memcpy(&msegcmd[j], ptr, umin(sizeof(Mach_segment_command), cmdsize));
        headway -= cmdsize;
        ptr     += cmdsize;
    }

    // Put LC_SEGMENT together at the beginning
    qsort(msegcmd, ncmds, sizeof(*msegcmd), compare_segment_command);
    n_segment = 0;
    for (unsigned j= 0; j < ncmds; ++j) {
        n_segment += (lc_seg==msegcmd[j].cmd);
    }

    total_in = 0;
    total_out = 0;
    unsigned c_adler = upx_adler32(nullptr, 0);
    unsigned u_adler = upx_adler32(nullptr, 0);

    fi->seek(- (off_t)(sizeof(bhdr) + ph.c_len), SEEK_CUR);
    for (unsigned k = 0; k < ncmds; ++k) {
        if (msegcmd[k].cmd==lc_seg && msegcmd[k].filesize!=0) {
            if (!strcmp("__TEXT", msegcmd[k].segname)) {
                segTEXT = msegcmd[k];
            }
            if (fo)
                fo->seek(msegcmd[k].fileoff, SEEK_SET);
            unpackExtent(msegcmd[k].filesize, fo,
                c_adler, u_adler, false, sizeof(bhdr));
            if (my_filetype==Mach_header::MH_DYLIB) {
                break;  // only the first lc_seg when MH_DYLIB
            }
        }
    }
    Mach_segment_command const *sc = (Mach_segment_command const *)(void *)(1+ mhdr);
    if (my_filetype==Mach_header::MH_DYLIB) { // rest of lc_seg are not compressed
        upx_uint64_t cpr_mod_init_func(0);
                TE32 unc_mod_init_func; *(int *)&unc_mod_init_func = 0;
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
                if (!strcmp("__DATA", rc->segname)) {
                    cpr_mod_init_func = get_mod_init_func(rc);
                    fi->seek(cpr_mod_init_func - 4*sizeof(TE32), SEEK_SET);
                    fi->readx(&unc_mod_init_func, sizeof(unc_mod_init_func));
                }
                fi->seek(rc->fileoff, SEEK_SET);
                if (fo)
                    fo->seek(sc->fileoff, SEEK_SET);
                unsigned const len = rc->filesize;
                MemBuffer data(len);
                fi->readx(data, len);
                if (!strcmp("__DATA", rc->segname)) {
                    set_te32(&data[o__mod_init_func - rc->fileoff], unc_mod_init_func);
                }
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
            unpackExtent(size, fo,
                c_adler, u_adler, false, sizeof(bhdr));
        }
    }
}

// The prize is the value of overlay_offset: the offset of compressed data
template <class T>
int PackMachBase<T>::canUnpack()
{
    unsigned const lc_seg = lc_seg_info[sizeof(Addr)>>3].segment_cmd;
    fi->seek(0, SEEK_SET);
    fi->readx(&mhdri, sizeof(mhdri));

    if (((unsigned) Mach_header::MH_MAGIC + (sizeof(Addr)>>3)) !=mhdri.magic
    ||  my_cputype   !=mhdri.cputype
    ||  my_filetype  !=mhdri.filetype
    )
        return false;
    my_cpusubtype = mhdri.cpusubtype;

    unsigned const ncmds = mhdri.ncmds;
    int headway = (int)mhdri.sizeofcmds;
    // old style:   LC_SEGMENT + LC_UNIXTHREAD  [smaller, varies by $ARCH]
    // new style: 3*LC_SEGMENT + LC_MAIN        [larger]
    if ((2 == ncmds
        && headway < (int)(sizeof(Mach_segment_command) + 4*4))
    ||  (3 <= ncmds
        && headway < (int)(3 * sizeof(Mach_segment_command)
                    + sizeof(Mach_main_command)))) {
        infoWarning("Mach_header.sizeofcmds = %d too small", headway);
        throwCantUnpack("file corrupted");
    }
    sz_mach_headers = headway + sizeof(mhdri);
    if (2048 < headway) {
        infoWarning("Mach_header.sizeofcmds(%d) > 2048", headway);
    }
    if (!headway) {
        throwCantPack("Mach_header.sizeofcmds == 0");
    }
    rawmseg_buf.alloc(mhdri.sizeofcmds);
    rawmseg = (Mach_segment_command *)rawmseg_buf.getVoidPtr();
    fi->readx(rawmseg, mhdri.sizeofcmds);

    Mach_segment_command const *ptrTEXT = nullptr;
    upx_uint64_t rip = 0;
    unsigned style = 0;
    off_t offLINK = 0;
    unsigned pos_next = 0;
    unsigned nseg = 0;
    Mach_command const *ptr = (Mach_command const *)rawmseg;
    for (unsigned j= 0; j < ncmds;
            ptr = (Mach_command const *)(ptr->cmdsize + (char const *)ptr), ++j) {
        unsigned const cmd = ptr->cmd;
        unsigned const cmdsize = ptr->cmdsize;
        if (is_bad_linker_command(cmd, cmdsize, headway, lc_seg, sizeof(Addr))) {
                infoWarning("bad Mach_command[%u]{@0x%lx,+0x%x}: file_size=0x%lx  cmdsize=0x%lx",
                    j, (unsigned long) (sizeof(mhdri) + ((char const *)ptr - (char const *)rawmseg)), headway,
                    (unsigned long) file_size, (unsigned long)ptr->cmdsize);
                throwCantUnpack("file corrupted");
        }
        if (lc_seg == ptr->cmd) {
            Mach_segment_command const *const segptr = (Mach_segment_command const *)ptr;
            if ((unsigned long)file_size < segptr->filesize
            ||  (unsigned long)file_size < segptr->fileoff
            ||  (unsigned long)file_size < (segptr->filesize + segptr->fileoff)) {
                infoWarning("bad Mach_segment_command[%u]{@0x%lx,+0x%x}: file_size=0x%lx  cmdsize=0x%lx"
                      "  filesize=0x%lx  fileoff=0x%lx",
                    j, (unsigned long) (sizeof(mhdri) + ((char const *)ptr - (char const *)rawmseg)), headway,
                    (unsigned long) file_size, (unsigned long)ptr->cmdsize,
                    (unsigned long)segptr->filesize, (unsigned long)segptr->fileoff);
                throwCantUnpack("file corrupted");
            }
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
                if (segptr->filesize == blankLINK) {
                    style = 395;
                }
                if (offLINK < (off_t) pos_next) {
                    offLINK = pos_next;
                }
            }
            pos_next = segptr->filesize + segptr->fileoff;
            if ((headway -= ptr->cmdsize) < 0) {
                infoWarning("Mach_command[%u]{@%lu}.cmdsize = %u", j,
                    (unsigned long) (sizeof(mhdri) + mhdri.sizeofcmds - (headway + ptr->cmdsize)),
                    (unsigned)ptr->cmdsize);
                throwCantUnpack("sum(.cmdsize) exceeds .sizeofcmds");
            }
        }
        else if (Mach_command::LC_UNIXTHREAD==ptr->cmd) {
            rip = entryVMA = threadc_getPC(ptr);
        }
    }
    if (3==nseg && 395 != style) { // __PAGEZERO, __TEXT, __LINKEDIT;  no __XHDR, no UPX_DATA
        style = 392;
    }
    if (391==style && 0==offLINK && 2==ncmds && ptrTEXT) { // pre-3.91 ?
        offLINK = ptrTEXT->fileoff + ptrTEXT->filesize;  // fake __LINKEDIT at EOF
    }
    if (0 == style || 0 == offLINK) {
        return false;
    }

    int const small = 32 + sizeof(overlay_offset);
    unsigned bufsize = 4096 + sizeof(PackHeader);
    if (391 == style) { // PackHeader precedes __LINKEDIT
        fi->seek(offLINK - bufsize, SEEK_SET);
    } else
    if (392 == style) {
        if (MH_DYLIB == my_filetype) {
            fi->seek(fi->st_size() - bufsize, SEEK_SET);
        }
        else { // PackHeader follows loader at __LINKEDIT
            if ((off_t)bufsize > (fi->st_size() - offLINK)) {
                bufsize = fi->st_size() - offLINK;
            }
            fi->seek(offLINK, SEEK_SET);
        }
    } else
    if (395 == style) {
        fi->seek(offLINK - bufsize, SEEK_SET);
    }
    MemBuffer buf(bufsize);
    MemBuffer buf3(bufsize);

    fi->readx(buf, bufsize);
    // Do not overwrite buf[]; For scratch space, then use buf3 instead.

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
            fi->readx(buf3, bufsize);
            unsigned char const *b = &buf3[0];
            unsigned disp = *(TE32 const *)&b[1];
            // Emulate the code
            if (0xe8==b[0] && disp < bufsize
                // This has been obsoleted by amd64-darwin.macho-entry.S
                // searching for "executable_path=" etc.
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
            if (395 == style) { // Desperation
                infoWarning("file corrupted: %s", fi->getName());
                fi->seek(file_size - bufsize, SEEK_SET);
                fi->readx(buf3, bufsize);
                unsigned const *p = (unsigned const *)&buf3[bufsize];
                for (; buf3 < (void const *)--p; ) {
                    unsigned x = *p;
                    if (x) {
                        if (!(3& x) && x < bufsize) {
                            fi->seek(0, SEEK_SET);
                            fi->readx(buf3, bufsize);
                            p = (unsigned const *)&buf3[x];
                            if (0 == p[0] && 0 != p[1] && p[1] == p[2]  // p_info
                            &&  sz_mach_headers < p[3] && p[4] < p[3]  // b_info
                            ) {
                                overlay_offset = x;
                                infoWarning("attempting recovery, overlay_offset = %#x",
                                    overlay_offset);
                                return true;
                            }
                        }
                        break;
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
            fi->readx(buf3, bufsize);
            struct p_info const *const p_ptr = (struct p_info const *)&buf3[0];
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
            // is the total length of compressed data which precedes it
            //(distance to l_info), so that's another method.
            fi->seek(offLINK - 0x1000, SEEK_SET);
            fi->readx(buf3, 0x1000);
            unsigned const *const lo = (unsigned const *)&buf3[0];
            unsigned const *p;
            for (p = (unsigned const *)&buf3[0x1000]; p > lo; ) if (*--p) {
                overlay_offset  = *(TE32 const *)p;
                if ((off_t)overlay_offset < offLINK) {
                    overlay_offset = ((char const *)p - (char const *)lo) +
                        (offLINK - 0x1000) - overlay_offset + sizeof(l_info);
                    fi->seek(overlay_offset, SEEK_SET);
                    fi->readx(buf3, bufsize);
                    if (b_ptr->sz_unc < 0x4000
                    &&  b_ptr->sz_cpr < b_ptr->sz_unc ) {
                        return true;
                    }
                }
            }
        }
    }

    overlay_offset = 0;  // impossible value
    int l = ph.buf_offset + ph.getPackHeaderSize();
    if (0 <= l && (unsigned)(l + sizeof(TE32)) <=bufsize) {
        overlay_offset = get_te32(buf + i + l);
    }
    if (       overlay_offset < sz_mach_headers
    ||  (off_t)overlay_offset >= file_size) {
        infoWarning("file corrupted: %s", fi->getName());
        MemBuffer buf2(umin(1<<14, file_size));
        fi->seek(sz_mach_headers, SEEK_SET);
        fi->readx(buf2, buf2.getSize());
        unsigned const *p = (unsigned const *)&buf2[0];
        unsigned const *const e_buf2 = (unsigned const *)&buf2[buf2.getSize() - 4*sizeof(*p)];
        for (; p <= e_buf2; ++p)
        if (   0==p[0]  // p_info.p_progid
        &&     0!=p[1]  // p_info.p_filesize
        &&  p[2]==p[1]  // p_info.p_blocksize == p_info.p_filesize
        &&  (unsigned)file_size < get_te32(&p[1])  // compression was worthwhile
        &&  sz_mach_headers==get_te32(&p[3])  // b_info.sz_unc
        ) {
            overlay_offset = ((char const *)p - (char const *)&buf2[0]) + sz_mach_headers;
            if (!(3&overlay_offset  // not word aligned
                    ||        overlay_offset < sz_mach_headers
                    || (off_t)overlay_offset >= file_size)) {
                infoWarning("attempting recovery, overlay_offset = %#x", overlay_offset);
                return true;
            }
        }
        throwCantUnpack("file corrupted");
    }
    return true;
}

template <class T>
upx_uint64_t PackMachBase<T>::get_mod_init_func(Mach_segment_command const *segptr)
{
    for (Mach_section_command const *secptr = (Mach_section_command const *)(1+ segptr);
        ptr_udiff_bytes(secptr, segptr) < segptr->cmdsize;
        ++secptr
    ) {
        if (sizeof(Addr) == secptr->size
        && !strcmp("__mod_init_func", secptr->sectname)) {
            o__mod_init_func = secptr->offset;
            fi->seek(o__mod_init_func, SEEK_SET);
            Addr tmp;
            fi->readx(&tmp, sizeof(Addr));
            return tmp;
        }
    }
    return 0;
}

template <class T>
bool PackMachBase<T>::canPack()
{
    unsigned const lc_seg = lc_seg_info[sizeof(Addr)>>3].segment_cmd;
    fi->seek(0, SEEK_SET);
    fi->readx(&mhdri, sizeof(mhdri));

    if (((unsigned) Mach_header::MH_MAGIC + (sizeof(Addr)>>3)) !=mhdri.magic
    ||  my_cputype   !=mhdri.cputype
    ||  my_filetype  !=mhdri.filetype
    )
        return false;
    my_cpusubtype = mhdri.cpusubtype;

    unsigned const ncmds = mhdri.ncmds;
    if (!ncmds || 256 < ncmds) { // arbitrary, but guard against garbage
        throwCantPack("256 < Mach_header.ncmds");
    }
    unsigned const sz_mhcmds = (unsigned)mhdri.sizeofcmds;
    unsigned headway = file_size - sizeof(mhdri);
    if (headway < sz_mhcmds) {
        char buf[32]; snprintf(buf, sizeof(buf), "bad sizeofcmds %d", sz_mhcmds);
        throwCantPack(buf);
    }
    if (!sz_mhcmds
    ||  16384 < sz_mhcmds) { // somewhat arbitrary, but amd64-darwin.macho-upxmain.c
        throwCantPack("16384 < Mach_header.sizeofcmds (or ==0)");
    }
    rawmseg_buf.alloc(sz_mhcmds);
    rawmseg = (Mach_segment_command *)(void *)rawmseg_buf;
    fi->readx(rawmseg, mhdri.sizeofcmds);

    msegcmd_buf.alloc(sizeof(Mach_segment_command) * ncmds);
    msegcmd = (Mach_segment_command *)msegcmd_buf.getVoidPtr();
    unsigned char const *ptr = (unsigned char const *)rawmseg;
    for (unsigned j= 0; j < ncmds; ++j) {
        Mach_segment_command const *segptr = (Mach_segment_command const *)ptr;
        unsigned const cmd     = segptr->cmd &~ LC_REQ_DYLD;
        unsigned const cmdsize = segptr->cmdsize;
        if (is_bad_linker_command(cmd, cmdsize, headway, lc_seg, sizeof(Addr))) {
            char buf[80]; snprintf(buf, sizeof(buf),
                "bad Mach_command[%d]{cmd=%#x, size=%#x}", j,
                cmd, cmdsize);
            throwCantPack(buf);
        }
        headway -= cmdsize;
        if (lc_seg == cmd) {
            msegcmd[j] = *segptr;
            if (!strcmp("__TEXT", segptr->segname)) {
                Mach_section_command const *secp =
                    (Mach_section_command const *)(const void*)(const char*)(1+ segptr);
                unsigned const offset = secp->offset;
                if (offset < file_size) {
                    struct l_info h;
                    fi->seek(offset, SEEK_SET);
                    fi->readx(&h, sizeof(h));
                    checkAlreadyPacked(&h, sizeof(h));
                }
            }
            if (!strcmp("__DATA", segptr->segname)) {
                prev_mod_init_func = get_mod_init_func(segptr);
            }
        }
        else {
            memcpy(&msegcmd[j], ptr, 2*sizeof(unsigned)); // cmd and cmdsize
        }
        switch (((Mach_uuid_command const *)ptr)->cmd) {
        default: break;
        case Mach_command::LC_UUID: {
            memcpy(&cmdUUID, ptr, sizeof(cmdUUID));  // remember the UUID
            // Set output UUID to be 1 more than the input UUID.
            for (unsigned k = 0; k < sizeof(cmdUUID.uuid); ++k) {
                if (0 != ++cmdUUID.uuid[k]) { // no Carry
                    break;
                }
            }
        } break;
        case Mach_command::LC_VERSION_MIN_MACOSX: {
            memcpy(&cmdVERMIN, ptr, sizeof(cmdVERMIN));
        } break;
        case Mach_command::LC_SOURCE_VERSION: {
            memcpy(&cmdSRCVER, ptr, sizeof(cmdSRCVER));
        } break;
        }
        ptr += (unsigned) ((Mach_command const *)ptr)->cmdsize;
    }
    if (Mach_header::MH_DYLIB==my_filetype && 0==o__mod_init_func) {
        infoWarning("missing -init function");
        return false;
    }

    // Put LC_SEGMENT together at the beginning
    qsort(msegcmd, ncmds, sizeof(*msegcmd), compare_segment_command);

    if (lc_seg==msegcmd[0].cmd && 0==msegcmd[0].vmaddr
    &&  !strcmp("__PAGEZERO", msegcmd[0].segname)) {
        pagezero_vmsize = msegcmd[0].vmsize;
    }

    // Check alignment of non-null LC_SEGMENT.
    vma_max = 0;
    for (unsigned j= 0; j < ncmds; ++j) {
        if (lc_seg==msegcmd[j].cmd) {
            ++n_segment;
            if (~my_page_mask & (msegcmd[j].fileoff | msegcmd[j].vmaddr)) {
                return false;
            }
            upx_uint64_t t = msegcmd[j].vmsize + msegcmd[j].vmaddr;
            if (vma_max < t) {
                vma_max = t;
            }
            // Segments need not be contigous (esp. "rust"/"go")
            sz_segment = msegcmd[j].filesize + msegcmd[j].fileoff - msegcmd[0].fileoff;
        }
    }
    vma_max = my_page_mask & (~my_page_mask + vma_max);

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
    static struct {
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
                   stub_i386_darwin_dylib_entry,  nullptr, nullptr
        },
        {CPU_TYPE_X86_64, MH_EXECUTE,
            sizeof(stub_amd64_darwin_macho_entry),
            sizeof(stub_amd64_darwin_macho_fold),
            0, //sizeof(stub_amd64_darwin_macho_upxmain_exe),
                   stub_amd64_darwin_macho_entry,
                   stub_amd64_darwin_macho_fold,
                   nullptr // stub_amd64_darwin_macho_upxmain_exe
        },
        {CPU_TYPE_X86_64, MH_DYLIB,
            sizeof(stub_amd64_darwin_dylib_entry), 0, 0,
                   stub_amd64_darwin_dylib_entry,  nullptr, nullptr
        },
        {CPU_TYPE_ARM, MH_EXECUTE,
            sizeof(stub_arm_v5a_darwin_macho_entry),
            sizeof(stub_arm_v5a_darwin_macho_fold),
            0,
                   stub_arm_v5a_darwin_macho_entry,
                   stub_arm_v5a_darwin_macho_fold,
                   nullptr
        },
        {CPU_TYPE_ARM64, MH_EXECUTE,
            sizeof(stub_arm64_darwin_macho_entry),
            sizeof(stub_arm64_darwin_macho_fold),
            0,
                   stub_arm64_darwin_macho_entry,
                   stub_arm64_darwin_macho_fold,
                   nullptr
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
                   stub_powerpc_darwin_dylib_entry,  nullptr, nullptr
        },
        {CPU_TYPE_POWERPC64, MH_EXECUTE,
            sizeof(stub_powerpc64_darwin_macho_entry),
            sizeof(stub_powerpc64_darwin_macho_fold),
            0,
                   stub_powerpc64_darwin_macho_entry,
                   stub_powerpc64_darwin_macho_fold,
                   nullptr
        },
        {CPU_TYPE_POWERPC64, MH_DYLIB,
            sizeof(stub_powerpc64_darwin_dylib_entry), 0, 0,
                   stub_powerpc64_darwin_dylib_entry,  nullptr, nullptr
        },
        {0,0, 0,0,0, nullptr,nullptr,nullptr}
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
            if (!stub_main) { // development stub
                static struct {
                    Mach_header mhdri;
                    Mach_segment_command segZERO;
                    Mach_segment_command segTEXT;
                    Mach_section_command secTEXT;
                    Mach_segment_command segLINK;
                    Mach_version_min_command cmdVERMIN;
                    Mach_source_version_command cmdSRCVER;
                } fsm;  // fake_stub_main
                fsm.mhdri = mhdri;
                fsm.mhdri.ncmds = 5;
                fsm.mhdri.sizeofcmds = sizeof(fsm) - sizeof(fsm.mhdri);
                fsm.mhdri.flags = MH_NOUNDEFS | MH_PIE;

                fsm.segZERO.cmd = LC_SEGMENT + (fsm.mhdri.cputype >> 24)
                    * (LC_SEGMENT_64 - LC_SEGMENT);
                fsm.segZERO.cmdsize = sizeof(Mach_segment_command);
                strncpy(fsm.segZERO.segname, "__PAGEZERO", sizeof(fsm.segZERO.segname));
                fsm.segZERO.vmaddr = 0;
                fsm.segZERO.vmsize = (4<<16);
                if __acc_cte(8==sizeof(void *)) fsm.segZERO.vmsize <<= (32 - 18);
                fsm.segZERO.fileoff = 0;
                fsm.segZERO.filesize = 0;
                fsm.segZERO.maxprot = 0;
                fsm.segZERO.initprot = 0;
                fsm.segZERO.nsects = 0;
                fsm.segZERO.flags = 0;

                unsigned const slop = 400;
                fsm.segTEXT.cmd = fsm.segZERO.cmd;
                fsm.segTEXT.cmdsize = sizeof(Mach_segment_command)
                    + sizeof(Mach_section_command);
                strncpy(fsm.segTEXT.segname, "__TEXT", sizeof(fsm.segTEXT.segname));
                fsm.segTEXT.vmaddr = fsm.segZERO.vmsize;
                fsm.segTEXT.vmsize = slop + threado_size() + sizeof(fsm);  // dummy
                fsm.segTEXT.fileoff = 0;
                fsm.segTEXT.filesize = fsm.segTEXT.vmsize;  // dummy
                fsm.segTEXT.maxprot = VM_PROT_EXECUTE | VM_PROT_READ;
                fsm.segTEXT.initprot = VM_PROT_EXECUTE | VM_PROT_READ;
                fsm.segTEXT.nsects = 1;
                fsm.segTEXT.flags = 0;

                strncpy(fsm.secTEXT.sectname, "__text", sizeof(fsm.secTEXT.sectname));
                memcpy(fsm.secTEXT.segname, fsm.segTEXT.segname, sizeof(fsm.secTEXT.segname));
                unsigned const d = slop + fsm.mhdri.sizeofcmds;
                fsm.secTEXT.addr = fsm.segTEXT.vmaddr + d;  // dummy
                fsm.secTEXT.size = fsm.segTEXT.vmsize - d;  // dummy
                fsm.secTEXT.offset = d;  // dummy
                fsm.secTEXT.align = 3;  // (1<<2)
                fsm.secTEXT.reloff = 0;
                fsm.secTEXT.nreloc = 0;
                fsm.secTEXT.flags = S_REGULAR | S_ATTR_PURE_INSTRUCTIONS | S_ATTR_SOME_INSTRUCTIONS;
                fsm.secTEXT.reserved1 = 0;
                fsm.secTEXT.reserved2 = 0;

                fsm.segLINK = fsm.segTEXT;
                fsm.segLINK.cmdsize = sizeof(Mach_segment_command);
                strncpy(fsm.segLINK.segname, "__LINKEDIT", sizeof(fsm.segLINK.segname));
                fsm.segLINK.vmaddr = fsm.segTEXT.vmaddr + fsm.segTEXT.vmsize;  // dummy
                fsm.segLINK.vmsize = 0x1000;  // dummy
                fsm.segLINK.fileoff = fsm.segTEXT.fileoff + fsm.segTEXT.filesize;
                fsm.segLINK.filesize = fsm.segLINK.vmsize;
                fsm.segLINK.maxprot = VM_PROT_READ;
                fsm.segLINK.initprot = VM_PROT_READ;
                fsm.segLINK.nsects = 0;

                fsm.cmdVERMIN.cmd = LC_VERSION_MIN_MACOSX;  // LC_VERSION_MIN_IPHONEOS
                fsm.cmdVERMIN.cmdsize = 4*4;
                fsm.cmdVERMIN.version = (10<<16)|(12<<8);
                fsm.cmdVERMIN.sdk = fsm.cmdVERMIN.version;

                fsm.cmdSRCVER.cmd = LC_SOURCE_VERSION;
                fsm.cmdSRCVER.cmdsize = 4*4;
                fsm.cmdSRCVER.version = 0;
                fsm.cmdSRCVER.__pad = 0;

                sz_stub_main  = sizeof(fsm);
                   stub_main  = (unsigned char const *)&fsm;
            }
            break;
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
        case PackMachFat::CPU_TYPE_POWERPC64: {
            typedef N_Mach::Mach_header<MachClass_LE64::MachITypes> Mach_header;
            Mach_header hdr;
            fi->readx(&hdr, sizeof(hdr));
            if (hdr.filetype==Mach_header::MH_EXECUTE) {
                PackMachPPC64 packer(fi);
                packer.initPackHeader();
                packer.canPack();
                packer.updatePackHeader();
                packer.pack(fo);
            }
            else if (hdr.filetype==Mach_header::MH_DYLIB) {
                PackDylibPPC64 packer(fi);
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
    if (fo) {  // test mode ("-t") sets fo = nullptr
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
        case PackMachFat::CPU_TYPE_POWERPC64: {
            N_Mach::Mach_header<MachClass_LE64::MachITypes> hdr;
            typedef N_Mach::Mach_header<MachClass_LE64::MachITypes> Mach_header;
            fi->readx(&hdr, sizeof(hdr));
            if (hdr.filetype==Mach_header::MH_EXECUTE) {
                PackMachPPC64 packer(fi);
                packer.initPackHeader();
                packer.canUnpack();
                packer.unpack(fo);
            }
            else if (hdr.filetype==Mach_header::MH_DYLIB) {
                PackDylibPPC64 packer(fi);
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
        case PackMachFat::CPU_TYPE_ARM64: {
            PackMachARM64EL packer(fi);
            if (!packer.canPack()) {
                //PackDylibARM64EL pack2r(fi);  FIXME: not yet
                //if (!pack2r.canPack())
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
        case PackMachFat::CPU_TYPE_POWERPC64: {
            PackMachPPC64 packer(fi);
            if (!packer.canPack()) {
                PackDylibPPC64 pack2r(fi);
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
        case PackMachFat::CPU_TYPE_POWERPC64: {
            PackMachPPC64 packer(fi);
            if (!packer.canUnpack()) {
                PackDylibPPC64 pack2r(fi);
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
