/* p_lx_elf.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2001 Laszlo Molnar
   Copyright (C) 2000-2001 John F. Reiser
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

   Markus F.X.J. Oberhumer   Laszlo Molnar           John F. Reiser
   markus@oberhumer.com      ml1050@cdata.tvnet.hu   jreiser@BitWagon.com
 */


#include "conf.h"

#include "file.h"
#include "filter.h"
#include "linker.h"
#include "packer.h"
#include "p_elf.h"
#include "p_unix.h"
#include "p_lx_exc.h"
#include "p_lx_elf.h"

#define PT_LOAD     Elf_LE32_Phdr::PT_LOAD


/*************************************************************************
//
**************************************************************************/

static const
#include "stub/l_lx_elf86.h"
static const
#include "stub/fold_elf86.h"


PackLinuxI386elf::PackLinuxI386elf(InputFile *f) :
    super(f), phdri(NULL)
{
}

PackLinuxI386elf::~PackLinuxI386elf()
{
    delete[] phdri;
}

int const *
PackLinuxI386elf::getFilters() const
{
    static const int filters[] = {
        0x49, 0x46,
        0x83, 0x36, 0x26,
              0x86, 0x80,
        0x84, 0x87, 0x81,
        0x82, 0x85,
        0x24, 0x16, 0x13, 0x14, 0x11, 0x25, 0x15, 0x12,
    -1 };
    return filters;
}

int
PackLinuxI386elf::buildLoader(const Filter *ft)
{
    return buildLinuxLoader(
        linux_i386elf_loader, sizeof(linux_i386elf_loader),
        linux_i386elf_fold,   sizeof(linux_i386elf_fold),
        ft, getbrk(phdri, ehdri.e_phnum) );
}


void PackLinuxI386elf::updateLoader(OutputFile *fo)
{
#define PAGE_MASK (~0<<12)
    upx_byte *const ptr = const_cast<upx_byte *>(getLoader());
    Elf_LE32_Phdr *const phdro = (Elf_LE32_Phdr *)(sizeof(Elf_LE32_Ehdr) + ptr);
    off_t const totlen = fo->getBytesWritten();
    phdro->p_filesz = totlen;

    // pre-calculate for benefit of runtime disappearing act via munmap()
    phdro->p_memsz =  PAGE_MASK & (~PAGE_MASK + totlen);

    patchLoaderChecksum();
#undef PAGE_MASK
}

void PackLinuxI386elf::patchLoader()
{
    unsigned char *const ptr = const_cast<unsigned char *>(getLoader());
    lsize = getLoaderSize();
    assert(lsize > 128 && lsize < 4096);
    patchVersion(ptr, lsize);

    patchLoaderChecksum();
}


bool PackLinuxI386elf::canPack()
{
    unsigned char buf[sizeof(Elf_LE32_Ehdr) + 14*sizeof(Elf_LE32_Phdr)];
    COMPILE_TIME_ASSERT(sizeof(buf) <= 512);

    exetype = 0;

    fi->readx(buf, sizeof(buf));
    fi->seek(0, SEEK_SET);
    Elf_LE32_Ehdr const *const ehdr = (Elf_LE32_Ehdr const *)buf;

    // now check the ELF header
    if (checkEhdr(ehdr) != 0)
        return false;

    // additional requirements for linux/elf386
    if (ehdr->e_ehsize != sizeof(*ehdr)) {
        throwCantPack("invalid Ehdr e_ehsize; try `--force-execve'");
        return false;
    }
    if (ehdr->e_phoff != sizeof(*ehdr)) {// Phdrs not contiguous with Ehdr
        throwCantPack("non-contiguous Ehdr/Phdr; try `--force-execve'");
        return false;
    }

    // The first PT_LOAD must cover the beginning of the file (0==p_offset).
    Elf_LE32_Phdr const *phdr = (Elf_LE32_Phdr const *)(buf + ehdr->e_phoff);
    for (unsigned j=0; j < ehdr->e_phnum; ++phdr, ++j) {
        if (j >= 14)
            return false;
        if (phdr->PT_LOAD == phdr->p_type) {
            if (phdr->p_offset != 0) {
                throwCantPack("invalid Phdr p_offset; try `--force-execve'");
                return false;
            }
#if 1
            // FIXME: what about these checks ?
            if (phdr->p_vaddr != 0x08048000) {
                throwCantPack("invalid Phdr p_vaddr; try `--force-execve'");
                return false;
            }
            if (phdr->p_paddr != 0x08048000) {
                throwCantPack("invalid Phdr p_paddr; try `--force-execve'");
                return false;
            }
#endif
            exetype = 1;
            break;
        }
    }

    return super::canPack();
}

struct cprBlkHdr {
    unsigned sz_unc;  // uncompressed size (0 means EOF)
    unsigned sz_cpr;  // (compressed_size<<8) | cto8
};

void PackLinuxI386elf::packExtent(
    Extent const &x,
    unsigned &total_in,
    unsigned &total_out,
    Filter *ft
)
{
    fi->seek(x.offset, SEEK_SET);
    for (off_t rest = x.size; 0 != rest; )
    {
        int l = fi->readx(ibuf, UPX_MIN(rest, (off_t)blocksize));
        if (l == 0)
            break;
        rest -= l;

        // Note: compression for a block can fail if the
        //       file is e.g. blocksize + 1 bytes long

        // compress
        unsigned char *const hdrptr = obuf;
        obuf.seek(sizeof(cprBlkHdr), SEEK_CUR);
        ph.u_len = l;
        ph.overlap_overhead = 0;
        if (ft) {
            ft->buf_len = l;
            compressWithFilters(ft, OVERHEAD, ((opt->filter > 0) ? -2 : 2));
        }
        else {
            (void) compress(ibuf, obuf);    // ignore return value
        }

        if (ph.c_len < ph.u_len)
        {
            ph.overlap_overhead = OVERHEAD;
            if (!testOverlappingDecompression(obuf, ph.overlap_overhead))
                throwNotCompressible();
        }
        else
        {
            ph.c_len = ph.u_len;
            // must update checksum of compressed data
            ph.c_adler = upx_adler32(ph.c_adler, ibuf, ph.u_len);
        }

        // write block sizes
        set_native32(0+hdrptr, ph.u_len);
        set_native32(4+hdrptr, (ph.c_len<<8) | ph.filter_cto);

        // write compressed data
        if (ph.c_len < ph.u_len)
        {
            obuf.write(obuf, ph.c_len);
            // FIXME: obuf is not discardable!
            // verifyOverlappingDecompression();
        }
        else
            obuf.write(ibuf, ph.u_len);

        total_in += ph.u_len;
        total_out += ph.c_len;
    }
}


void PackLinuxI386elf::pack(OutputFile *fo)
{
    // set options
    opt->unix.blocksize = blocksize = file_size;
    progid = 0;  // not used

    fi->readx(&ehdri, sizeof(ehdri));
    assert(ehdri.e_phoff == sizeof(Elf_LE32_Ehdr));  // checked by canPack()
    off_t const sz_phdrs = ehdri.e_phnum * ehdri.e_phentsize;

    phdri = new Elf_LE32_Phdr[ehdri.e_phnum];
    fi->seek(ehdri.e_phoff, SEEK_SET);
    fi->readx(phdri, sz_phdrs);

    // init compression buffers
    ibuf.alloc(blocksize);
    obuf.allocForCompression(blocksize);
    {
        p_info hbuf;
        set_native32(&hbuf.p_progid, progid);
        set_native32(&hbuf.p_filesize, file_size);
        set_native32(&hbuf.p_blocksize, blocksize);
        obuf.write(&hbuf, sizeof(hbuf));
    }

    Extent x;
    unsigned k;

    // count passes, set ptload vars
    ui_total_passes = 0;
    off_t ptload0hi = 0, ptload1lo = 0;
    int nx = 0;
    for (k = 0; k < ehdri.e_phnum; ++k) {
        if (PT_LOAD == phdri[k].p_type) {
            x.offset = phdri[k].p_offset;
            x.size   = phdri[k].p_filesz;
            if (0 == ptload0hi) {
                ptload0hi = x.offset + x.size;
            }
            else if (0 == ptload1lo) {
                ptload1lo = x.offset;
            }
            ui_total_passes++;
        } else {
            if (nx++ == 0)
                ui_total_passes++;
        }
    }
    if (ptload0hi < ptload1lo)
        ui_total_passes++;

    // compress extents
    unsigned total_in = 0;
    unsigned total_out = 0;

    ui_pass = -1;
    x.offset = 0;
    x.size = sizeof(Elf_LE32_Ehdr) + sz_phdrs;
    {
        int const old_level = ph.level; ph.level = 10;
        packExtent(x, total_in, total_out, 0);
        ph.level = old_level;
    }

    ui_pass = 0;
    Filter ft(ph.level);
    ft.addvalue = 0;

    nx = 0;
    for (k = 0; k < ehdri.e_phnum; ++k) if (PT_LOAD==phdri[k].p_type) {
        if (ft.id < 0x40) {
            // FIXME: ??    ft.addvalue = phdri[k].p_vaddr;
        }
        x.offset = phdri[k].p_offset;
        x.size   = phdri[k].p_filesz;
        if (0 == nx) { // 1st PT_LOAD must cover Ehdr at 0==p_offset
            unsigned const delta = sizeof(Elf_LE32_Ehdr) + sz_phdrs;
            if (ft.id < 0x40) {
                // FIXME: ??     ft.addvalue += delta;
            }
            x.offset    += delta;
            x.size      -= delta;
        }
        packExtent(x, total_in, total_out,
            ((Elf_LE32_Phdr::PF_X & phdri[k].p_flags)
                ? &ft : 0 ) );
        ++nx;
    }
    if (ptload0hi < ptload1lo) { // alignment hole?
        x.offset = ptload0hi;
        x.size   = ptload1lo - ptload0hi;
        packExtent(x, total_in, total_out, 0);
    }
    if ((off_t)total_in < file_size) {  // non-PT_LOAD stuff
        x.offset = total_in;
        x.size = file_size - total_in;
        packExtent(x, total_in, total_out, 0);
    }

    if ((off_t)total_in != file_size)
        throwEOFException();

    // write block end marker (uncompressed size 0)
    set_native32(obuf, 0);
    obuf.write(obuf, 4);

    // update header with totals
    ph.u_len = total_in;
    ph.c_len = total_out;

    upx_byte const *p = getLoader();
    lsize = getLoaderSize();
    patchFilter32(const_cast<upx_byte *>(p), lsize, &ft);
    fo->write(p, lsize);

    unsigned pos = obuf.seek(0, SEEK_CUR);
    fo->write(obuf - pos, pos);

    // write packheader
    writePackHeader(fo);

    // write overlay offset (needed for decompression)
    set_native32(obuf, lsize);
    fo->write(obuf, 4);

    updateLoader(fo);
    fo->seek(0, SEEK_SET);
    fo->rewrite(p, lsize);

    // finally check the compression ratio
    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();
}


void PackLinuxI386elf::unpackExtent(unsigned wanted, OutputFile *fo,
    unsigned &total_in, unsigned &total_out,
    unsigned &c_adler, unsigned &u_adler,
    bool first_PF_X
)
{
    while (wanted) {
        fi->readx(ibuf, 8);
        int const sz_unc = ph.u_len = get_native32(ibuf+0);
        unsigned const tmp =          get_native32(ibuf+4);
        int const sz_cpr = ph.c_len = tmp>>8;
        ph.filter_cto = tmp<<24;

        if (sz_unc == 0) { // must never happen while 0!=wanted
            throwCompressedDataViolation();
            break;
        }
        if (sz_unc <= 0 || sz_cpr <= 0)
            throwCompressedDataViolation();
        if (sz_cpr > sz_unc || sz_unc > (int)blocksize)
            throwCompressedDataViolation();

        int j = blocksize + OVERHEAD - sz_cpr;
        fi->readx(ibuf+j, sz_cpr);
        // update checksum of compressed data
        c_adler = upx_adler32(c_adler, ibuf + j, sz_cpr);
        // decompress
        if (sz_cpr < sz_unc)
        {
            decompress(ibuf+j, ibuf, false);
            if (first_PF_X) { // Elf32_Ehdr is never filtered
                first_PF_X = false;  // but everything else might be
            }
            else if (ph.filter) {
                Filter ft(ph.level);
                ft.init(ph.filter, 0);
                ft.cto = ph.filter_cto;
                ft.unfilter(ibuf, sz_unc);
            }
            j = 0;
        }
        // update checksum of uncompressed data
        u_adler = upx_adler32(u_adler, ibuf + j, sz_unc);
        total_in  += sz_cpr;
        total_out += sz_unc;
        // write block
        if (fo)
            fo->write(ibuf + j, sz_unc);
        wanted -= sz_unc;
    }
}


void PackLinuxI386elf::unpack(OutputFile *fo)
{
#define MAX_ELF_HDR 512
    char bufehdr[MAX_ELF_HDR];
    Elf_LE32_Ehdr *const ehdr = (Elf_LE32_Ehdr *)bufehdr;
    Elf_LE32_Phdr const *phdr = (Elf_LE32_Phdr *)(1+ehdr);

    fi->seek(overlay_offset, SEEK_SET);
    p_info hbuf;
    fi->readx(&hbuf, sizeof(hbuf));
    unsigned orig_file_size = get_native32(&hbuf.p_filesize);
    blocksize = get_native32(&hbuf.p_blocksize);
    if (file_size > (off_t)orig_file_size || blocksize > orig_file_size)
        throwCantUnpack("file header corrupted");

    ibuf.alloc(blocksize + OVERHEAD);
    fi->readx(ibuf, 2*4);
    ph.u_len = get_native32(0+ibuf);
    unsigned const tmp = get_native32(4+ibuf);
    ph.c_len = tmp>>8;
    ph.filter_cto = tmp<<24;

    // Uncompress Ehdr and Phdrs.
    fi->readx(ibuf, ph.c_len);
    decompress(ibuf, (upx_byte *)ehdr, false);

    unsigned total_in = 0;
    unsigned total_out = 0;
    unsigned c_adler = upx_adler32(0, NULL, 0);
    unsigned u_adler = upx_adler32(0, NULL, 0);
    off_t ptload0hi=0, ptload1lo=0;

    // decompress PT_LOAD
    bool first_PF_X = true;
    fi->seek(- (off_t) (2*4 + ph.c_len), SEEK_CUR);
    for (unsigned j=0; j < ehdr->e_phnum; ++phdr, ++j) {
        if (PT_LOAD==phdr->p_type) {
            if (0==ptload0hi) {
                ptload0hi = phdr->p_filesz + phdr->p_offset;
            }
            else if (0==ptload1lo) {
                ptload1lo = phdr->p_offset;
            }
            if (fo)
                fo->seek(phdr->p_offset, SEEK_SET);
            if (Elf_LE32_Phdr::PF_X & phdr->p_flags) {
                unpackExtent(phdr->p_filesz, fo, total_in, total_out,
                    c_adler, u_adler, first_PF_X);
                first_PF_X = false;
            }
            else {
                unpackExtent(phdr->p_filesz, fo, total_in, total_out,
                    c_adler, u_adler, false);
            }
        }
    }

    if (ptload0hi < ptload1lo) {  // alignment hole?
        if (fo)
            fo->seek(ptload0hi, SEEK_SET);
        unpackExtent(ptload1lo - ptload0hi, fo, total_in, total_out,
            c_adler, u_adler, false);
    }
    if (total_out != orig_file_size) {  // non-PT_LOAD stuff
        if (fo)
            fo->seek(0, SEEK_END);
        unpackExtent(orig_file_size - total_out, fo, total_in, total_out,
            c_adler, u_adler, false);
    }

    // check for end-of-file
    fi->readx(ibuf, 2*4);
    unsigned const sz_unc = ph.u_len = get_native32(ibuf+0);

    if (sz_unc == 0) { // uncompressed size 0 -> EOF
        // note: magic is always stored le32
        unsigned const sz_cpr = get_le32(ibuf+4);
        if (sz_cpr != UPX_MAGIC_LE32)  // sz_cpr must be h->magic
            throwCompressedDataViolation();
    }
    else { // extra bytes after end?
        throwCompressedDataViolation();
    }

    // update header with totals
    ph.c_len = total_in;
    ph.u_len = total_out;

    // all bytes must be written
    if (total_out != orig_file_size)
        throwEOFException();

    // finally test the checksums
    if (ph.c_adler != c_adler || ph.u_adler != u_adler)
        throwChecksumError();
#undef MAX_ELF_HDR
}


/*
vi:ts=4:et
*/

