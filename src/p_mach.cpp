/* p_mach.cpp -- pack Mach Object executable

   This file is part of the UPX executable compressor.

   Copyright (C) 2004-2009 John Reiser
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


const int *PackMachPPC32::getFilters() const
{
    static const int filters[] = { 0xd0, FT_END };
    return filters;
}

int const *PackMachI386::getFilters() const
{
    static const int filters[] = { 0x49, FT_END };
    return filters;
}

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

void PackMachARMEL::addStubEntrySections(Filter const */*ft*/)
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
    segcmdo.vmsize += h.sz_unc - h.sz_cpr + GAP + NO_LAP;

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
    lp->l_lsize = (unsigned short) lsize;
    lp->l_version = (unsigned char) ph.version;
    lp->l_format  = (unsigned char) ph.format;
    // INFO: lp->l_checksum is currently unused
    lp->l_checksum = upx_adler32(ptr, lsize);
}

template <class T>
int __acc_cdecl_qsort
PackMachBase<T>::compare_segment_command(void const *const aa, void const *const bb)
{
    Mach_segment_command const *const a = (Mach_segment_command const *)aa;
    Mach_segment_command const *const b = (Mach_segment_command const *)bb;
    unsigned const xa = a->cmd - Mach_segment_command::LC_SEGMENT;
    unsigned const xb = b->cmd - Mach_segment_command::LC_SEGMENT;
           if (xa < xb)        return -1;  // LC_SEGMENT first
           if (xa > xb)        return  1;
    if (a->vmaddr < b->vmaddr) return -1;  // ascending by .vmaddr
    if (a->vmaddr > b->vmaddr) return  1;
                               return  0;
}

void PackMachPPC32::pack4(OutputFile *fo, Filter &ft)  // append PackHeader
{
    // offset of p_info in compressed file
    overlay_offset = sizeof(mhdro) + sizeof(segcmdo) + sizeof(threado) + sizeof(linfo);

    super::pack4(fo, ft);
    segcmdo.filesize = fo->getBytesWritten();
    segcmdo.vmsize += segcmdo.filesize;
    fo->seek(sizeof(mhdro), SEEK_SET);
    fo->rewrite(&segcmdo, sizeof(segcmdo));
    fo->rewrite(&threado, sizeof(threado));
    fo->rewrite(&linfo, sizeof(linfo));
}

void PackMachI386::pack4(OutputFile *fo, Filter &ft)  // append PackHeader
{
    // offset of p_info in compressed file
    overlay_offset = sizeof(mhdro) + sizeof(segcmdo) + sizeof(threado) + sizeof(linfo);

    super::pack4(fo, ft);
    segcmdo.filesize = fo->getBytesWritten();
    segcmdo.vmsize += segcmdo.filesize;
    fo->seek(sizeof(mhdro), SEEK_SET);
    fo->rewrite(&segcmdo, sizeof(segcmdo));
    fo->rewrite(&threado, sizeof(threado));
    fo->rewrite(&linfo, sizeof(linfo));
}

void PackMachARMEL::pack4(OutputFile *fo, Filter &ft)  // append PackHeader
{
    // offset of p_info in compressed file
    overlay_offset = sizeof(mhdro) + sizeof(segcmdo) + sizeof(threado) + sizeof(linfo);

    super::pack4(fo, ft);
    segcmdo.filesize = fo->getBytesWritten();
    segcmdo.vmsize += segcmdo.filesize;
    fo->seek(sizeof(mhdro), SEEK_SET);
    fo->rewrite(&segcmdo, sizeof(segcmdo));
    fo->rewrite(&threado, sizeof(threado));
    fo->rewrite(&linfo, sizeof(linfo));
}

#undef PAGE_MASK
#undef PAGE_SIZE
#define PAGE_MASK (~0u<<12)
#define PAGE_SIZE -PAGE_MASK

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
        printf("Unrecognized Macho cmd  offset=0x%x  cmd=0x%x  size=0x%x\n", (char *)seg - (char *)rawmseg, (unsigned)seg->cmd, (unsigned)seg->cmdsize);
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
    case Mach_segment_command::LC_ROUTINES_64:
    case Mach_segment_command::LC_ROUTINES: {
        Mach_routines_command cmd = *(Mach_routines_command const *)seg;
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
                o_end_txt = segcmdtmp.filesize + segcmdtmp.fileoff;
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
            Mach_section_command const *secp = (Mach_section_command const *)(1+ seg);
            unsigned const nsects = segcmdtmp.nsects;
            Mach_section_command seccmdtmp;
            for (unsigned j = 0; j < nsects; ++secp, ++j) {
                seccmdtmp = *secp;
                seccmdtmp.offset += slide;
                if (seccmdtmp.nreloc) {
                    seccmdtmp.reloff += slide;
                }
                fo->rewrite(&seccmdtmp, sizeof(seccmdtmp));
                hdrpos += sizeof(seccmdtmp);
            }

            if (!is_text) {
                fo->seek(opos, SEEK_SET);
                fi->seek(seg->fileoff, SEEK_SET);
                unsigned const len = seg->filesize;
                char data[len];
                fi->readx(data, len);
                fo->write(data, len);
                opos += len;
            }
        }
    } break;
    case Mach_segment_command::LC_SYMTAB: {
        Mach_symtab_command cmd = *(Mach_symtab_command const *)seg;
        if (o_end_txt <= cmd.symoff) { cmd.symoff += slide; }
        if (o_end_txt <= cmd.stroff) { cmd.stroff += slide; }
        fo->seek(hdrpos, SEEK_SET);
        fo->rewrite(&cmd, sizeof(cmd));
        hdrpos += sizeof(cmd);
    } break;
    case Mach_segment_command::LC_DYSYMTAB: {
        Mach_dysymtab_command cmd = *(Mach_dysymtab_command const *)seg;
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
        Mach_segsplit_info_command cmd = *(Mach_segsplit_info_command const *)seg;
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

void PackMachPPC32::pack3(OutputFile *fo, Filter &ft)  // append loader
{
    BE32 disp;
    unsigned const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 3& (0u-len));
    len += (3& (0u-len)) + sizeof(disp);
    disp = 4+ len - sz_mach_headers;  // 4: sizeof(instruction)
    fo->write(&disp, sizeof(disp));

    threado.state.srr0 = len + segcmdo.vmaddr;  /* entry address */
    super::pack3(fo, ft);
}

void PackMachI386::pack3(OutputFile *fo, Filter &ft)  // append loader
{
    LE32 disp;
    unsigned const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 3& (0u-len));
    len += (3& (0u-len));
    disp = len - sz_mach_headers;
    fo->write(&disp, sizeof(disp));

    threado.state.eip = len + sizeof(disp) + segcmdo.vmaddr;  /* entry address */
    super::pack3(fo, ft);
}

void PackMachARMEL::pack3(OutputFile *fo, Filter &ft)  // append loader
{
    LE32 disp;
    unsigned const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 3& (0u-len));
    len += (3& (0u-len));
    disp = len - sz_mach_headers;
    fo->write(&disp, sizeof(disp));

    threado.state.pc = len + sizeof(disp) + segcmdo.vmaddr;  /* entry address */
    super::pack3(fo, ft);
}

void PackDylibI386::pack3(OutputFile *fo, Filter &ft)  // append loader
{
    LE32 disp;
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
    if (Mach_segment_command::LC_SEGMENT!=msegcmd[k].cmd
    ||  0==msegcmd[k].filesize ) {
        return 0;
    }
    unsigned const hi = msegcmd[k].fileoff + msegcmd[k].filesize;
    unsigned lo = ph.u_file_size;
    unsigned j = k;
    for (;;) { // circular search, optimize for adjacent ascending
        ++j;
        if (n_segment==j) {
            j = 0;
        }
        if (k==j) {
            break;
        }
        if (Mach_segment_command::LC_SEGMENT==msegcmd[j].cmd
        &&  0!=msegcmd[j].filesize ) {
            unsigned const t = msegcmd[j].fileoff;
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
void PackMachBase<T>::pack2(OutputFile *fo, Filter &ft)  // append compressed body
{
    Extent x;
    unsigned k;

    // count passes, set ptload vars
    uip->ui_total_passes = 0;
    for (k = 0; k < n_segment; ++k) {
        if (Mach_segment_command::LC_SEGMENT==msegcmd[k].cmd
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

    unsigned hdr_u_len = mhdri.sizeofcmds;

    uip->ui_pass = 0;
    ft.addvalue = 0;

    // Packer::compressWithFilters chooses a filter for us, and the stubs
    // can handle only one filter, and most filters are for executable
    // instructions.  So filter only the largest executable segment.
    unsigned exe_filesize_max = 0;
    for (k = 0; k < n_segment; ++k)
    if (Mach_segment_command::LC_SEGMENT==msegcmd[k].cmd
    &&  0!=(Mach_segment_command::VM_PROT_EXECUTE & msegcmd[k].initprot)
    &&  exe_filesize_max < msegcmd[k].filesize) {
        exe_filesize_max = msegcmd[k].filesize;
    }

    int nx = 0;
    for (k = 0; k < n_segment; ++k)
    if (Mach_segment_command::LC_SEGMENT==msegcmd[k].cmd
    &&  0!=msegcmd[k].filesize ) {
        x.offset = msegcmd[k].fileoff;
        x.size   = msegcmd[k].filesize;
        if (0 == nx) { // 1st LC_SEGMENT must cover Mach_header at 0==fileoffset
            unsigned const delta = mhdri.sizeofcmds;
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
    segcmdo.filesize = fo->getBytesWritten();
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
    mhdro = mhdri;
    if (my_filetype==Mach_header::MH_EXECUTE) {
        mhdro.ncmds = 2;
        mhdro.sizeofcmds = sizeof(segcmdo) + my_thread_command_size;
        mhdro.flags = Mach_header::MH_NOUNDEFS;
    }
    fo->write(&mhdro, sizeof(mhdro));

    segcmdo.cmd = Mach_segment_command::LC_SEGMENT;
    segcmdo.cmdsize = sizeof(segcmdo);
    strncpy((char *)segcmdo.segname, "__TEXT", sizeof(segcmdo.segname));
    if (my_filetype==Mach_header::MH_EXECUTE) {
        segcmdo.vmaddr = PAGE_MASK & (~PAGE_MASK +
            msegcmd[n_segment -1].vmsize + msegcmd[n_segment -1].vmaddr );
    }
    if (my_filetype==Mach_header::MH_DYLIB) {
        segcmdo.vmaddr = 0;
    }
    segcmdo.vmsize = 0;    // adjust later
    segcmdo.fileoff = 0;
    segcmdo.filesize = 0;  // adjust later
    segcmdo.initprot = segcmdo.maxprot =
        Mach_segment_command::VM_PROT_READ |
        Mach_segment_command::VM_PROT_WRITE |
        Mach_segment_command::VM_PROT_EXECUTE;
    segcmdo.nsects = 0;
    segcmdo.flags = 0;
    if (my_filetype==Mach_header::MH_EXECUTE) {
        fo->write(&segcmdo, sizeof(segcmdo));
        pack1_setup_threado(fo);
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
    fi->seek(0, SEEK_SET);
    fi->readx(&mhdri, sizeof(mhdri));
    rawmseg = (Mach_segment_command *)new char[(unsigned) mhdri.sizeofcmds];
    fi->readx(rawmseg, mhdri.sizeofcmds);
    overlay_offset = sizeof(mhdri) + mhdri.sizeofcmds + sizeof(linfo);
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
        n_segment += (Mach_segment_command::LC_SEGMENT==msegcmd[j].cmd);
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
        if (Mach_segment_command::LC_SEGMENT==sc->cmd
        &&  0!=sc->filesize ) {
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
            if (Mach_segment_command::LC_SEGMENT==rc->cmd
            &&  0!=rc->filesize ) {
                fi->seek(rc->fileoff, SEEK_SET);
                fo->seek(sc->fileoff, SEEK_SET);
                unsigned const len = rc->filesize;
                char data[len];
                fi->readx(data, len);
                fo->write(data, len);
            }
        }
    }
    else
    for (unsigned j = 0; j < ncmds; ++j) {
        unsigned const size = find_SEGMENT_gap(j);
        if (size) {
            unsigned const where = msegcmd[k].fileoff +msegcmd[k].filesize;
            if (fo)
                fo->seek(where, SEEK_SET);
            unpackExtent(size, fo, total_in, total_out,
                c_adler, u_adler, false, sizeof(bhdr));
        }
    }
    delete mhdr;
}


template <class T>
bool PackMachBase<T>::canPack()
{
    fi->seek(0, SEEK_SET);
    fi->readx(&mhdri, sizeof(mhdri));

    if (Mach_header::MH_MAGIC         !=mhdri.magic
    ||  my_cputype                    !=mhdri.cputype
    ||  my_filetype                   !=mhdri.filetype
    )
        return false;

    rawmseg = (Mach_segment_command *)new char[(unsigned) mhdri.sizeofcmds];
    fi->readx(rawmseg, mhdri.sizeofcmds);

    msegcmd = new Mach_segment_command[(unsigned) mhdri.ncmds];
    unsigned char const *ptr = (unsigned char const *)rawmseg;
    for (unsigned j= 0; j < mhdri.ncmds; ++j) {
        msegcmd[j] = *(Mach_segment_command const *)ptr;
        if (((Mach_segment_command const *)ptr)->cmd ==
              Mach_segment_command::LC_ROUTINES) {
            o_routines_cmd = (char *)ptr - (char *)rawmseg;
            prev_init_address = ((Mach_routines_command const *)ptr)->init_address;
        }
        ptr += (unsigned) ((Mach_segment_command *)ptr)->cmdsize;
    }
    if (Mach_header::MH_DYLIB==my_filetype && 0==o_routines_cmd) {
        return false;
    }

    // Put LC_SEGMENT together at the beginning, ascending by .vmaddr.
    qsort(msegcmd, mhdri.ncmds, sizeof(*msegcmd), compare_segment_command);

    // Check that LC_SEGMENTs form one contiguous chunk of the file.
    for (unsigned j= 0; j < mhdri.ncmds; ++j) {
        if (Mach_segment_command::LC_SEGMENT==msegcmd[j].cmd) {
            if (~PAGE_MASK & (msegcmd[j].fileoff | msegcmd[j].vmaddr)) {
                return false;
            }
            if (0 < j) {
                unsigned const sz = PAGE_MASK
                                  & (~PAGE_MASK + msegcmd[j-1].filesize);
                if ((sz + msegcmd[j-1].fileoff)!=msegcmd[j].fileoff) {
                    return false;
                }
            }
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
        switch (fat_head.arch[j].cputype) {
        case PackMachFat::CPU_TYPE_I386: {
            PackMachI386 packer(fi);
            packer.initPackHeader();
            packer.canPack();
            packer.updatePackHeader();
            packer.pack(fo);
        } break;
        case PackMachFat::CPU_TYPE_POWERPC: {
            PackMachPPC32 packer(fi);
            packer.initPackHeader();
            packer.canPack();
            packer.updatePackHeader();
            packer.pack(fo);
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
    fo->seek(0, SEEK_SET);
    fo->write(&fat_head, sizeof(fat_head.fat) +
        fat_head.fat.nfat_arch * sizeof(fat_head.arch[0]));
    unsigned length;
    for (unsigned j=0; j < fat_head.fat.nfat_arch; ++j) {
        unsigned base = fo->unset_extent();  // actual length
        base += ~(~0u<<fat_head.arch[j].align) & (0-base);  // align up
        fo->seek(base, SEEK_SET);
        fo->set_extent(base, ~0u);

        ph.u_file_size = fat_head.arch[j].size;
        fi->set_extent(fat_head.arch[j].offset, fat_head.arch[j].size);
        switch (fat_head.arch[j].cputype) {
        case PackMachFat::CPU_TYPE_I386: {
            PackMachI386 packer(fi);
            packer.initPackHeader();
            packer.canUnpack();
            packer.unpack(fo);
        } break;
        case PackMachFat::CPU_TYPE_POWERPC: {
            PackMachPPC32 packer(fi);
            packer.initPackHeader();
            packer.canUnpack();
            packer.unpack(fo);
        } break;
        }  // switch cputype
        fat_head.arch[j].offset = base;
        length = fo->unset_extent();
        fat_head.arch[j].size = length - base;
    }
    fo->unset_extent();
    fo->seek(0, SEEK_SET);
    fo->rewrite(&fat_head, sizeof(fat_head.fat) +
        fat_head.fat.nfat_arch * sizeof(fat_head.arch[0]));
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
        switch (arch[j].cputype) {
        default: return false;
        case PackMachFat::CPU_TYPE_I386: {
            PackMachI386 packer(fi);
            if (!packer.canPack())
                return false;
        } break;
        case PackMachFat::CPU_TYPE_POWERPC: {
            PackMachPPC32 packer(fi);
            if (!packer.canPack())
                return false;
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
        switch (arch[j].cputype) {
        default: return false;
        case PackMachFat::CPU_TYPE_I386: {
            PackMachI386 packer(fi);
            if (!packer.canUnpack())
                return 0;
            ph.format = packer.getFormat(); // FIXME: copy entire PackHeader
        } break;
        case PackMachFat::CPU_TYPE_POWERPC: {
            PackMachPPC32 packer(fi);
            if (!packer.canUnpack())
                return 0;
            ph.format = packer.getFormat(); // FIXME: copy entire PackHeader
        } break;
        }  // switch cputype
    }
    return 1;
}

void PackMachFat::buildLoader(const Filter */*ft*/)
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
