/* p_mach.cpp -- pack Mach Object executable

   This file is part of the UPX executable compressor.

   Copyright (C) 2004-2008 John Reiser
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

template <class T>
PackMachBase<T>::PackMachBase(InputFile *f, unsigned cputype, unsigned flavor,
        unsigned count, unsigned size) :
    super(f), my_cputype(cputype), my_thread_flavor(flavor),
    my_thread_state_word_count(count), my_thread_command_size(size),
    n_segment(0), rawmseg(NULL), msegcmd(NULL)
{
    MachClass::compileTimeAssertions();
    bele = N_BELE_CTP::getRTP<BeLePolicy>();
}

template <class T>
PackMachBase<T>::~PackMachBase()
{
    delete [] rawmseg;
    delete [] msegcmd;
}

template <class T>
const int *PackMachBase<T>::getCompressionMethods(int method, int level) const
{
    // There really is no LE bias.
    return Packer::getDefaultCompressionMethods_le32(method, level);
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

Linker *PackMachPPC32::newLinker() const
{
    return new ElfLinkerPpc32;
}

Linker *PackMachI386::newLinker() const
{
    return new ElfLinkerX86;
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
    segcmdo.vmsize += h.sz_unc - h.sz_cpr + GAP + 64;

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
    }
    for (k = 0; k < n_segment; ++k) {
        x.size = find_SEGMENT_gap(k);
        if (x.size) {
            x.offset = msegcmd[k].fileoff +msegcmd[k].filesize;
            packExtent(x, total_in, total_out, 0, fo);
        }
    }

    if ((off_t)total_in != file_size)
        throwEOFException();
    segcmdo.filesize = fo->getBytesWritten();
}

#undef PAGE_MASK
#undef PAGE_SIZE
#define PAGE_MASK (~0u<<12)
#define PAGE_SIZE -PAGE_MASK

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

template <class T>
void PackMachBase<T>::pack1(OutputFile *const fo, Filter &/*ft*/)  // generate executable header
{
    mhdro = mhdri;
    mhdro.ncmds = 2;
    mhdro.sizeofcmds = sizeof(segcmdo) + my_thread_command_size;
    mhdro.flags = Mach_header::MH_NOUNDEFS;
    fo->write(&mhdro, sizeof(mhdro));

    segcmdo.cmd = Mach_segment_command::LC_SEGMENT;
    segcmdo.cmdsize = sizeof(segcmdo);
    strncpy((char *)&segcmdo.segname, "__TEXT", sizeof(segcmdo.segname));
    segcmdo.vmaddr = PAGE_MASK & (~PAGE_MASK +
        msegcmd[n_segment -1].vmsize + msegcmd[n_segment -1].vmaddr );
    segcmdo.vmsize = 0;    // adjust later
    segcmdo.fileoff = 0;
    segcmdo.filesize = 0;  // adjust later
    segcmdo.initprot = segcmdo.maxprot =
        Mach_segment_command::VM_PROT_READ |
        Mach_segment_command::VM_PROT_WRITE |
        Mach_segment_command::VM_PROT_EXECUTE;
    segcmdo.nsects = 0;
    segcmdo.flags = 0;
    fo->write(&segcmdo, sizeof(segcmdo));

    pack1_setup_threado(fo);
    sz_mach_headers = fo->getBytesWritten();

    memset((char *)&linfo, 0, sizeof(linfo));
    fo->write(&linfo, sizeof(linfo));

    return;
}

template <class T>
void PackMachBase<T>::unpack(OutputFile *fo)
{
    overlay_offset = sizeof(mhdro) + sizeof(segcmdo) +
        my_thread_command_size + sizeof(linfo);
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
    Mach_segment_command const *sc = (Mach_segment_command const *)(1+ mhdr);
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
            unpackExtent(filesize, fo, total_in, total_out, c_adler, u_adler, false, sizeof(bhdr));
        }
    }
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
}


template <class T>
bool PackMachBase<T>::canPack()
{
    fi->seek(0, SEEK_SET);
    fi->readx(&mhdri, sizeof(mhdri));

    if (Mach_header::MH_MAGIC         !=mhdri.magic
    ||  my_cputype                    !=mhdri.cputype
    ||  Mach_header::MH_EXECUTE       !=mhdri.filetype
    )
        return false;

    rawmseg = (Mach_segment_command *)new char[mhdri.sizeofcmds];
    fi->readx(rawmseg, mhdri.sizeofcmds);

    msegcmd = new Mach_segment_command[mhdri.ncmds];
    unsigned char *ptr = (unsigned char *)rawmseg;
    for (unsigned j= 0; j < mhdri.ncmds; ++j) {
        msegcmd[j] = *(Mach_segment_command *)ptr;
        ptr += (unsigned) ((Mach_segment_command *)ptr)->cmdsize;
    }


    // Put LC_SEGMENT together at the beginning, ascending by .vmaddr.
    qsort(msegcmd, mhdri.ncmds, sizeof(*msegcmd), compare_segment_command);

    // Check that LC_SEGMENTs form one contiguous chunk of the file.
    for (unsigned j= 0; j < mhdri.ncmds; ++j) {
        if (Mach_segment_command::LC_SEGMENT==msegcmd[j].cmd) {
            if (0xfff & (msegcmd[j].fileoff | msegcmd[j].vmaddr)) {
                return false;
            }
            if (0 < j) {
                unsigned const sz = ~0xfff
                                  & (0xfff + msegcmd[j-1].filesize);
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
