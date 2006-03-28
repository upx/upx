/* p_mach.cpp -- pack Mach Object executable

   This file is part of the UPX executable compressor.

   Copyright (C)      2004 John Reiser
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
   jreiser@users.sourceforge.net
 */


#include "conf.h"

#include "file.h"
#include "filter.h"
#include "linker.h"
#include "packer.h"
#include "p_mach.h"

static const
#include "stub/l_mac_ppc32.h"

static const
#include "stub/fold_machppc32.h"

PackMachPPC32::PackMachPPC32(InputFile *f) :
    super(f), n_segment(0), rawmseg(0), msegcmd(0)
{
}

PackMachPPC32::~PackMachPPC32()
{
    delete [] msegcmd;
    delete [] rawmseg;
}

const int *PackMachPPC32::getCompressionMethods(int /*method*/, int /*level*/) const
{
    // There really is no LE bias in M_NRV2E_LE32.
    static const int m_nrv2e[] = { M_NRV2E_LE32, -1 };
    return m_nrv2e;
}


const int *PackMachPPC32::getFilters() const
{
    static const int filters[] = { 0xd0, -1 };
    return filters;
}

int
PackMachPPC32::buildMachLoader(
    upx_byte const *const proto,
    unsigned        const szproto,
    upx_byte const *const fold,
    unsigned        const szfold,
    Filter const */*ft*/
)
{
    int eof_empty = -1;
    initLoader(&eof_empty, 4096, 0, 0);

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
    unsigned sz_cpr;
    int r = upx_compress(uncLoader, h.sz_unc, sizeof(h) + cprLoader, &sz_cpr,
        NULL, ph.method, 10, NULL, NULL );
    h.sz_cpr = sz_cpr;
    if (r != UPX_E_OK || h.sz_cpr >= h.sz_unc)
        throwInternalError("loader compression failed");
  }
    memcpy(cprLoader, &h, sizeof(h));

    // This adds the definition to the "library", to be used later.
    linker->addSection("FOLDEXEC", cprLoader, sizeof(h) + h.sz_cpr);
    delete [] cprLoader;

    int const GAP = 128;  // must match stub/l_mac_ppc.S
    segcmdo.vmsize += h.sz_unc - h.sz_cpr + GAP + 64;

    linker->addSection("MACOS000", proto, szproto);

    addLoader("MACOS000", NULL);
    addLoader("FOLDEXEC", NULL);
    return getLoaderSize();
}

int
PackMachPPC32::buildLoader(const Filter *ft)
{
    return buildMachLoader(
        l_mac_ppc32_loader, sizeof(l_mac_ppc32_loader),
        fold_machppc32,     sizeof(fold_machppc32),  ft );
}
void PackMachPPC32::patchLoader() { }
void PackMachPPC32::updateLoader(OutputFile *) {}

void
PackMachPPC32::patchLoaderChecksum()
{
    unsigned char *const ptr = const_cast<unsigned char *>(getLoader());
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

static int __acc_cdecl_qsort
compare_segment_command(void const *const aa, void const *const bb)
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

void
PackMachPPC32::pack4(OutputFile *fo, Filter &ft)  // append PackHeader
{
    // offset of p_info in compressed file
    overlay_offset = sizeof(mhdro) + sizeof(segcmdo) + sizeof(threado) + sizeof(linfo);

    super::pack4(fo, ft);
    segcmdo.filesize = fo->getBytesWritten();
    segcmdo.vmsize += segcmdo.filesize;
    fo->seek(sizeof(mhdro), SEEK_SET);
    fo->write(&segcmdo, sizeof(segcmdo));
    fo->write(&threado, sizeof(threado));
    fo->write(&linfo, sizeof(linfo));
}

void
PackMachPPC32::pack3(OutputFile *fo, Filter &ft)  // append loader
{
    BE32 disp;
    unsigned const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 3& -len);
    len += (3& -len) + sizeof(disp);
    set_be32(&disp, 4+ len - sz_mach_headers);  // 4: sizeof(instruction)
    fo->write(&disp, sizeof(disp));

    threado.state.srr0 = len + segcmdo.vmaddr;  /* entry address */
    super::pack3(fo, ft);
}

// Determine length of gap between PT_LOAD phdri[k] and closest PT_LOAD
// which follows in the file (or end-of-file).  Optimize for common case
// where the PT_LOAD are adjacent ascending by .p_offset.  Assume no overlap.

unsigned PackMachPPC32::find_SEGMENT_gap(
    unsigned const k
)
{
    if (Mach_segment_command::LC_SEGMENT!=msegcmd[k].cmd
    ||  0==msegcmd[k].filesize ) {
        return 0;
    }
    unsigned const hi = get_native32(&msegcmd[k].fileoff) +
                        get_native32(&msegcmd[k].filesize);
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
            unsigned const t = get_native32(&msegcmd[j].fileoff);
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

void
PackMachPPC32::pack2(OutputFile *fo, Filter &ft)  // append compressed body
{
    Extent x;
    unsigned k;

    // count passes, set ptload vars
    ui_total_passes = 0;
    for (k = 0; k < n_segment; ++k) {
        if (Mach_segment_command::LC_SEGMENT==msegcmd[k].cmd
        &&  0!=msegcmd[k].filesize ) {
            ui_total_passes++;
            if (find_SEGMENT_gap(k)) {
                ui_total_passes++;
            }
        }
    }

    // compress extents
    unsigned total_in = 0;
    unsigned total_out = 0;

    unsigned hdr_ulen = mhdri.sizeofcmds;

    ui_pass = 0;
    ft.addvalue = 0;

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
        packExtent(x, total_in, total_out,
            ((Mach_segment_command::VM_PROT_EXECUTE & msegcmd[k].initprot)
                ? &ft : 0 ), fo, hdr_ulen );
        hdr_ulen = 0;
        ++nx;
    }
    for (k = 0; k < n_segment; ++k) {
        x.size = find_SEGMENT_gap(k);
        if (x.size) {
            x.offset = get_native32(&msegcmd[k].fileoff) +
                       get_native32(&msegcmd[k].filesize);
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
void
PackMachPPC32::pack1(OutputFile *fo, Filter &/*ft*/)  // generate executable header
{
    mhdro = mhdri;
    mhdro.ncmds = 2;
    mhdro.sizeofcmds = sizeof(segcmdo) + sizeof(threado);
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

    threado.cmd = Mach_segment_command::LC_UNIXTHREAD;
    threado.cmdsize = sizeof(threado);
    threado.flavor = Mach_thread_command::PPC_THREAD_STATE;
    threado.count =  Mach_thread_command::PPC_THREAD_STATE_COUNT;
    memset(&threado.state, 0, sizeof(threado.state));
    fo->write(&threado, sizeof(threado));
    sz_mach_headers = fo->getBytesWritten();

    memset((char *)&linfo, 0, sizeof(linfo));
    fo->write(&linfo, sizeof(linfo));

    return;
}

void
PackMachPPC32::unpack(OutputFile *fo)
{
    fi->seek(overlay_offset, SEEK_SET);
    p_info hbuf;
    fi->readx(&hbuf, sizeof(hbuf));
    unsigned orig_file_size = get_native32(&hbuf.p_filesize);
    blocksize = get_native32(&hbuf.p_blocksize);
    if (file_size > (off_t)orig_file_size || blocksize > orig_file_size)
        throwCantUnpack("file header corrupted");

    ibuf.alloc(blocksize + OVERHEAD);
    b_info bhdr; memset(&bhdr, 0, sizeof(bhdr));
    fi->readx(&bhdr, sizeof(bhdr));
    ph.u_len = get_native32(&bhdr.sz_unc);
    ph.c_len = get_native32(&bhdr.sz_cpr);
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
            unsigned filesize = get_be32(&sc->filesize);
            unpackExtent(filesize, fo, total_in, total_out, c_adler, u_adler, false, sizeof(bhdr));
        }
    }
    for (unsigned j = 0; j < ncmds; ++j) {
        unsigned const size = find_SEGMENT_gap(j);
        if (size) {
            unsigned const where = get_native32(&msegcmd[k].fileoff) +
                                   get_native32(&msegcmd[k].filesize);
            if (fo)
                fo->seek(where, SEEK_SET);
            unpackExtent(size, fo, total_in, total_out,
                c_adler, u_adler, false, sizeof(bhdr));
        }
    }
}


bool
PackMachPPC32::canPack()
{
    fi->seek(0, SEEK_SET);
    fi->readx(&mhdri, sizeof(mhdri));

    if (Mach_header::MH_MAGIC         !=mhdri.magic
    ||  Mach_header::CPU_TYPE_POWERPC !=mhdri.cputype
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
    // set options
    opt->o_unix.blocksize = file_size;
    return 0 < n_segment;
}

/*
vi:ts=4:et
*/
