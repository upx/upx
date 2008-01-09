/* p_lx_interp.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
   Copyright (C) 2000-2008 John F. Reiser
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
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>

   John F. Reiser
   <jreiser@users.sourceforge.net>
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
#include "p_lx_interp.h"

#define PT_LOAD     Elf32_Phdr::PT_LOAD
#define PT_INTERP   Elf32_Phdr::PT_INTERP


/*************************************************************************
//
**************************************************************************/

static const
#include "stub/i386-linux.elf.interp-entry.h"
static const
#include "stub/i386-linux.elf.interp-fold.h"

PackLinuxElf32x86interp::PackLinuxElf32x86interp(InputFile *f) :
    super(f)
{
}

PackLinuxElf32x86interp::~PackLinuxElf32x86interp()
{
}

bool PackLinuxElf32x86interp::canPack()
{
    if (opt->o_unix.make_ptinterp) {
        return true;
    }
    if (!opt->o_unix.use_ptinterp) {
        return false;
    }
    return super::canPack();
}

void PackLinuxElf32x86interp::pack1(OutputFile *fo, Filter &)
{
    fi->seek(0, SEEK_SET);
    fi->readx(&ehdri, sizeof(ehdri));
    assert(ehdri.e_phoff == sizeof(Elf32_Ehdr));  // checked by canPack()
    sz_phdrs = ehdri.e_phnum * ehdri.e_phentsize;

    phdri = new Elf32_Phdr[(unsigned)ehdri.e_phnum];
    fi->seek(ehdri.e_phoff, SEEK_SET);
    fi->readx(phdri, sz_phdrs);

#define E Elf32_Ehdr
    cprElfHdr3 h3;
    memset(&h3, 0, sizeof(h3));
    memcpy(h3.ehdr.e_ident, "\177ELF", 4);
    h3.ehdr.e_ident[E::EI_CLASS] = E::ELFCLASS32;
    h3.ehdr.e_ident[E::EI_DATA] = E::ELFDATA2LSB;
    h3.ehdr.e_ident[E::EI_VERSION] = E::EV_CURRENT;
    h3.ehdr.e_ident[E::EI_OSABI] = E::ELFOSABI_LINUX;
    h3.ehdr.e_ident[E::EI_ABIVERSION] = E::EV_CURRENT;
    h3.ehdr.e_type = E::ET_EXEC;
    h3.ehdr.e_machine = E::EM_386;
    h3.ehdr.e_version = 1;
    h3.ehdr.e_phoff = sizeof(Elf32_Ehdr);
    h3.ehdr.e_ehsize = sizeof(Elf32_Ehdr);
    h3.ehdr.e_phentsize = sizeof(Elf32_Phdr);
    h3.ehdr.e_phnum = 3;
    h3.phdr[0].p_type = PT_LOAD;
    h3.phdr[0].p_flags = Elf32_Phdr::PF_X | Elf32_Phdr::PF_R;
    h3.phdr[0].p_align = 0x1000;
    h3.phdr[1].p_type = PT_LOAD;
    h3.phdr[1].p_flags = Elf32_Phdr::PF_W | Elf32_Phdr::PF_R;
    h3.phdr[1].p_align = 1;
    h3.phdr[2].p_type = PT_INTERP;
    h3.phdr[2].p_offset = (char *)&h3.phdr[2].p_vaddr - (char *)&h3;
    memcpy(&h3.phdr[2].p_vaddr, "/upxrun", h3.phdr[2].p_filesz = 8);
    h3.phdr[2].p_align = 1;

    if (opt->o_unix.make_ptinterp) { // unusual "once per release"
        elfout = h3;
        elfout.ehdr.e_phnum = 1;
        fo->write(&elfout, elfout.ehdr.e_ehsize + elfout.ehdr.e_phentsize);
    }
    else { // usual case
        generateElfHdr(fo, &h3, getbrk(phdri, ehdri.e_phnum));
    }
#undef E
}

void PackLinuxElf32x86interp::pack2(OutputFile *fo, Filter &ft)
{
    if (opt->o_unix.make_ptinterp) {
        return;  // ignore current input file!
    }
    super::pack2(fo, ft);
}

#undef PAGE_MASK
#define PAGE_MASK (~0u<<12)

void PackLinuxElf32x86interp::pack3(OutputFile *fo, Filter &/*ft*/)
{
    unsigned base = getbase(phdri, ehdri.e_phnum);
    unsigned sz = PAGE_MASK & (~PAGE_MASK + elfout.phdr[0].p_filesz);
    if (base < (0x11000 + sz)) {
        base =  0x11000 + sz;
    }
    if (opt->o_unix.make_ptinterp) {
        base = 0x10000;
    }
    elfout.phdr[0].p_paddr = elfout.phdr[0].p_vaddr = base - sz;
    if (opt->o_unix.make_ptinterp) {
        initLoader(stub_i386_linux_elf_interp_entry, sizeof(stub_i386_linux_elf_interp_entry));
        linker->addSection("FOLDEXEC", stub_i386_linux_elf_interp_fold, sizeof(stub_i386_linux_elf_interp_fold), 0);

        addLoader("LXPTI000", NULL);

        addLoader("LXPTI040", NULL);
        ph.method = M_NRV2B_LE32; addLoader(getDecompressorSections(), NULL);
        addLoader("LXPTI090", NULL);

        addLoader("LXPTI041", NULL);
        ph.method = M_NRV2D_LE32; addLoader(getDecompressorSections(), NULL);
        addLoader("LXPTI090", NULL);

        addLoader("LXPTI042", NULL);
        ph.method = M_NRV2E_LE32; addLoader(getDecompressorSections(), NULL);
        addLoader("LXPTI090", NULL);

        //addLoader("LXPTI043", NULL);
        //ph.method = M_CL1B_LE32;  addLoader(getDecompressorSections(), NULL);
        //addLoader("LXPTI090", NULL);

        addLoader("LXPTI091", NULL);

        addLoader("LXPTI140", NULL);

        addLoader("LXUNF002,LXUNF008,LXUNF010", NULL);
        addFilter32(0x46);
        addLoader("LXUNF042,LXUNF035", NULL);

        addLoader("LXUNF002,LXUNF008,LXUNF010", NULL);
        addFilter32(0x49);
        addLoader("LXUNF042,LXUNF035", NULL);

        addLoader("LXPTI200", NULL);
        addLoader("FOLDEXEC", NULL);
        upx_byte const *p = getLoader();
        lsize = getLoaderSize();
        updateLoader(fo);
        fo->write(p, lsize);
        elfout.phdr[0].p_filesz = fo->getBytesWritten();
    }
    else {
        updateLoader(fo);
    }
}


void PackLinuxElf32x86interp::unpack(OutputFile *fo)
{
#define MAX_INTERP_HDR 512
    char bufehdr[MAX_INTERP_HDR];
    Elf32_Ehdr *const ehdr = (Elf32_Ehdr *)bufehdr;
    Elf32_Phdr const *phdr = (Elf32_Phdr *)(1+ehdr);

    unsigned szb_info = sizeof(b_info);
    {
        fi->seek(0, SEEK_SET);
        fi->readx(bufehdr, MAX_INTERP_HDR);
        unsigned const e_entry = get_te32(&ehdr->e_entry);
        if (e_entry < 0x401180) { /* old style, 8-byte b_info */
            szb_info = 2*sizeof(unsigned);
        }
    }

    fi->seek(overlay_offset, SEEK_SET);
    p_info hbuf;
    fi->readx(&hbuf, sizeof(hbuf));
    unsigned orig_file_size = get_te32(&hbuf.p_filesize);
    blocksize = get_te32(&hbuf.p_blocksize);
    if (file_size > (off_t)orig_file_size || blocksize > orig_file_size)
        throwCantUnpack("file header corrupted");

    ibuf.alloc(blocksize + OVERHEAD);
    b_info bhdr; memset(&bhdr, 0, sizeof(bhdr));
    fi->readx(&bhdr, szb_info);
    ph.u_len = get_te32(&bhdr.sz_unc);
    ph.c_len = get_te32(&bhdr.sz_cpr);
    ph.filter_cto = bhdr.b_cto8;

    // Uncompress Ehdr and Phdrs.
    fi->readx(ibuf, ph.c_len);
    decompress(ibuf, (upx_byte *)ehdr, false);

    unsigned total_in = 0;
    unsigned total_out = 0;
    unsigned c_adler = upx_adler32(NULL, 0);
    unsigned u_adler = upx_adler32(NULL, 0);
    off_t ptload0hi=0, ptload1lo=0, ptload1sz=0;

    // decompress PT_LOAD
    bool first_PF_X = true;
    fi->seek(- (off_t) (szb_info + ph.c_len), SEEK_CUR);
    for (unsigned j=0; j < ehdr->e_phnum; ++phdr, ++j) {
        if (PT_LOAD==phdr->p_type) {
            if (0==ptload0hi) {
                ptload0hi = phdr->p_filesz + phdr->p_offset;
            }
            else if (0==ptload1lo) {
                ptload1lo = phdr->p_offset;
                ptload1sz = phdr->p_filesz;
            }
            if (fo)
                fo->seek(phdr->p_offset, SEEK_SET);
            if (Elf32_Phdr::PF_X & phdr->p_flags) {
                unpackExtent(phdr->p_filesz, fo, total_in, total_out,
                    c_adler, u_adler, first_PF_X, szb_info);
                first_PF_X = false;
            }
            else {
                unpackExtent(phdr->p_filesz, fo, total_in, total_out,
                    c_adler, u_adler, false, szb_info);
            }
        }
    }

    if (0!=ptload1sz && ptload0hi < ptload1lo) {  // alignment hole?
        if (fo)
            fo->seek(ptload0hi, SEEK_SET);
        unpackExtent(ptload1lo - ptload0hi, fo, total_in, total_out,
            c_adler, u_adler, false, szb_info);
    }
    if (total_out != orig_file_size) {  // non-PT_LOAD stuff
        if (fo)
            fo->seek(0, SEEK_END);
        unpackExtent(orig_file_size - total_out, fo, total_in, total_out,
            c_adler, u_adler, false, szb_info);
    }

    // check for end-of-file
    fi->readx(&bhdr, szb_info);
    unsigned const sz_unc = ph.u_len = get_te32(&bhdr.sz_unc);

    if (sz_unc == 0) { // uncompressed size 0 -> EOF
        // note: magic is always stored le32
        unsigned const sz_cpr = get_le32(&bhdr.sz_cpr);
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
#undef MAX_INTERP_HDR
}


/*
vi:ts=4:et
*/

