/* p_lx_elf.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2004 Laszlo Molnar
   Copyright (C) 2000-2004 John F. Reiser
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

#define PT_LOAD     Elf32_Phdr::PT_LOAD


int
PackLinuxElf32::checkEhdr(
    Elf32_Ehdr const *ehdr,
    unsigned char e_machine,
    unsigned char ei_class,
    unsigned char ei_data
) const
{
    const unsigned char * const buf = ehdr->e_ident;

    if (0!=memcmp(buf, "\x7f\x45\x4c\x46", 4)  // "\177ELF"
    ||  buf[Elf32_Ehdr::EI_CLASS]!=ei_class
    ||  buf[Elf32_Ehdr::EI_DATA] !=ei_data ) {
        return -1;
    }
    if (!memcmp(buf+8, "FreeBSD", 7))                   // branded
        return 1;

    if (get_native16(&ehdr->e_type) != Elf32_Ehdr::ET_EXEC)
        return 2;
    if (get_native16(&ehdr->e_machine) != e_machine)
        return 3;
    if (get_native32(&ehdr->e_version) != Elf32_Ehdr::EV_CURRENT)
        return 4;
    if (get_native16(&ehdr->e_phnum) < 1)
        return 5;
    if (get_native16(&ehdr->e_phentsize) != sizeof(Elf32_Phdr))
        return 6;

    // check for Linux kernels
    unsigned const entry = get_native32(&ehdr->e_entry);
    if (entry == 0xC0100000)    // uncompressed vmlinux
        return 1000;
    if (entry == 0x00001000)    // compressed vmlinux
        return 1001;
    if (entry == 0x00100000)    // compressed bvmlinux
        return 1002;

    // FIXME: add more checks for kernels

    // FIXME: add special checks for other ELF i386 formats, like
    //        NetBSD, OpenBSD, Solaris, ....

    // success
    return 0;
}

PackLinuxElf32::PackLinuxElf32(InputFile *f)
    : super(f), phdri(NULL)
{
}

PackLinuxElf32::~PackLinuxElf32()
{
    delete[] phdri;
}

int const *
PackLinuxElf32::getCompressionMethods(int method, int level) const
{
    // No real dependency on LE32.
    return Packer::getDefaultCompressionMethods_le32(method, level);
}

int const *
PackLinuxElf32ppc::getCompressionMethods(int /*method*/, int /*level*/) const
{
    // No real dependency on LE32.
    static const int m_nrv2e[] = { M_NRV2E_LE32, -1 };
    return m_nrv2e;
}

int const *
PackLinuxElf32ppc::getFilters() const
{
    static const int filters[] = {
        0xd0, -1
    };
    return filters;
}

void PackLinuxElf32::patchLoader() { }

void PackLinuxElf32::updateLoader(OutputFile *fo)
{
    set_native32(&elfout.ehdr.e_entry, fo->getBytesWritten() +
        get_native32(&elfout.phdr[0].p_vaddr));
}

int
PackLinuxElf32::buildLinuxLoader(
    upx_byte const *const /*proto*/,
    unsigned        const /*szproto*/,
    upx_byte const *const /*fold*/,
    unsigned        const /*szfold*/,
    Filter const */*ft*/
)
{
    return 0;
}

PackLinuxElf32ppc::PackLinuxElf32ppc(InputFile *f)
    : super(f)
{
}

PackLinuxElf32ppc::~PackLinuxElf32ppc()
{
}

static unsigned
umax(unsigned a, unsigned b)
{
    if (a <= b) {
        return b;
    }
    return a;
}

int
PackLinuxElf32ppc::buildLinuxLoader(
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
    unsigned sz_unc=0, sz_cpr;
  if (0 < szfold) {
    cprElfHdr1 const *const hf = (cprElfHdr1 const *)fold;
    fold_hdrlen = umax(0x80, sizeof(hf->ehdr) +
        get_native16(&hf->ehdr.e_phentsize) * get_native16(&hf->ehdr.e_phnum) +
            sizeof(l_info) );
    sz_unc = ((szfold < fold_hdrlen) ? 0 : (szfold - fold_hdrlen));
    set_native32(&h.sz_unc, ((szfold < fold_hdrlen) ? 0 : (szfold - fold_hdrlen)));
    h.b_method = (unsigned char) ph.method;
    h.b_ftid = (unsigned char) ph.filter;
    h.b_cto8 = (unsigned char) ph.filter_cto;
  }
    unsigned char const *const uncLoader = fold_hdrlen + fold;

    unsigned char *const cprLoader = new unsigned char[sizeof(h) + sz_unc];
  if (0 < szfold) {
    int r = upx_compress(uncLoader, sz_unc, sizeof(h) + cprLoader, &sz_cpr,
        NULL, ph.method, 10, NULL, NULL );
    set_native32(&h.sz_cpr, sz_cpr);
    if (r != UPX_E_OK || sz_cpr >= sz_unc)
        throwInternalError("loader compression failed");
  }
    memcpy(cprLoader, &h, sizeof(h));

    // This adds the definition to the "library", to be used later.
    linker->addSection("FOLDEXEC", cprLoader, sizeof(h) + sz_cpr);
    delete [] cprLoader;

    //int const GAP = 128;  // must match stub/l_mac_ppc.S
    //segcmdo.vmsize += sz_unc - sz_cpr + GAP + 64;

    linker->addSection("ELFPPC32", proto, szproto);

    addLoader("ELFPPC32", 0);
    addLoader("FOLDEXEC", 0);
    return getLoaderSize();
}

static const
#include "stub/l_lx_elfppc32.h"

static const
#include "stub/fold_elfppc32.h"

int
PackLinuxElf32ppc::buildLoader(const Filter *ft)
{
    return buildLinuxLoader(
        linux_elfppc32_loader, sizeof(linux_elfppc32_loader),
        linux_elfppc32_fold,   sizeof(linux_elfppc32_fold),  ft );
}

bool
PackLinuxElf32ppc::canPack()
{
    unsigned char buf[sizeof(Elf32_Ehdr) + 14*sizeof(Elf32_Phdr)];
    COMPILE_TIME_ASSERT(sizeof(buf) <= 512);

    exetype = 0;

    fi->readx(buf, sizeof(buf));
    fi->seek(0, SEEK_SET);
    Elf32_Ehdr const *const ehdr = (Elf32_Ehdr const *)buf;

    // now check the ELF header
    if (checkEhdr(ehdr, Elf32_Ehdr::EM_PPC,
        Elf32_Ehdr::ELFCLASS32, Elf32_Ehdr::ELFDATA2MSB) != 0)
        return false;

    // additional requirements for linux/elf386
    if (get_native16(&ehdr->e_ehsize) != sizeof(*ehdr)) {
        throwCantPack("invalid Ehdr e_ehsize; try `--force-execve'");
        return false;
    }
    unsigned const e_phoff = get_native32(&ehdr->e_phoff);
    if (e_phoff != sizeof(*ehdr)) {// Phdrs not contiguous with Ehdr
        throwCantPack("non-contiguous Ehdr/Phdr; try `--force-execve'");
        return false;
    }

    // The first PT_LOAD must cover the beginning of the file (0==p_offset).
    unsigned const e_phnum = get_native16(&ehdr->e_phnum);
    Elf32_Phdr const *phdr = (Elf32_Phdr const *)(buf + e_phoff);
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        if (j >= 14)
            return false;
        if (phdr->PT_LOAD == get_native32(&phdr->p_type)) {
            // Just avoid the "rewind" when unpacking?
            //if (phdr->p_offset != 0) {
            //    throwCantPack("invalid Phdr p_offset; try `--force-execve'");
            //    return false;
            //}

            // detect possible conflict upon invocation
            unsigned const p_vaddr = get_native32(&phdr->p_vaddr);
            if (p_vaddr < (unsigned)(0x400000 + file_size)
            ||  p_vaddr < (unsigned)(0x400000 + file_size) ) {
                throwAlreadyPackedByUPX();  // not necessarily, but mostly true
                return false;
            }
            exetype = 1;
            break;
        }
    }

    if (!super::canPack())
        return false;
    assert(exetype == 1);

    // set options
    opt->o_unix.blocksize = blocksize = file_size;
    return true;
}

off_t
PackLinuxElf32::getbrk(const Elf32_Phdr *phdr, int e_phnum) const
{
    off_t brka = 0;
    for (int j = 0; j < e_phnum; ++phdr, ++j) {
        if (PT_LOAD == get_native32(&phdr->p_type)) {
            off_t b = get_native32(&phdr->p_vaddr) + get_native32(&phdr->p_memsz);
            if (b > brka)
                brka = b;
        }
    }
    return brka;
}

void
PackLinuxElf32::generateElfHdr(
    OutputFile */*fo*/,
    void const *proto,
    unsigned const brka
)
{
    cprElfHdr2 *const h2 = (cprElfHdr2 *)&elfout;
    cprElfHdr3 *const h3 = (cprElfHdr3 *)&elfout;
    memcpy(h3, proto, sizeof(*h3));  // reads beyond, but OK

    assert(get_native32(&h2->ehdr.e_phoff)     == sizeof(Elf32_Ehdr));
                         h2->ehdr.e_shoff = 0;
    assert(get_native16(&h2->ehdr.e_ehsize)    == sizeof(Elf32_Ehdr));
    assert(get_native16(&h2->ehdr.e_phentsize) == sizeof(Elf32_Phdr));
                         h2->ehdr.e_shnum = 0;

#if 0  //{
    unsigned identsize;
    char const *const ident = getIdentstr(&identsize);
#endif  //}
    sz_elf_hdrs = sizeof(*h2) - sizeof(linfo);  // default
    set_native32(&h2->phdr[0].p_filesz, sizeof(*h2));  // + identsize;
                  h2->phdr[0].p_memsz = h2->phdr[0].p_filesz;

    // Info for OS kernel to set the brk()
    if (brka) {
        set_native32(&h2->phdr[1].p_type, PT_LOAD);  // be sure
        set_native32(&h2->phdr[1].p_offset, 0xfff&brka);
        set_native32(&h2->phdr[1].p_vaddr, brka);
        set_native32(&h2->phdr[1].p_paddr, brka);
        h2->phdr[1].p_filesz = 0;
        h2->phdr[1].p_memsz =  0;
    }
}

void
PackLinuxElf32ppc::generateElfHdr(
    OutputFile *fo,
    void const *proto,
    unsigned const brka
)
{
    super::generateElfHdr(fo, proto, brka);

    if (ph.format==UPX_F_LINUX_ELFPPC32) {
        cprElfHdr2 *const h2 = (cprElfHdr2 *)&elfout;
        assert(2==get_native16(&h2->ehdr.e_phnum));
        set_native32(&h2->phdr[0].p_flags, Elf32_Phdr::PF_W | get_native32(&h2->phdr[0].p_flags));
        memset(&h2->linfo, 0, sizeof(h2->linfo));
        fo->write(h2, sizeof(*h2));
    }
    else {
        assert(false);  // unknown ph.format, PackLinuxElf32ppc::generateElfHdr
    }
}

static const
#include "stub/l_lx_elf86.h"
static const
#include "stub/fold_elf86.h"

void PackLinuxElf32::pack1(OutputFile *fo, Filter &/*ft*/)
{
    fi->seek(0, SEEK_SET);
    fi->readx(&ehdri, sizeof(ehdri));
    unsigned const e_phoff = get_native32(&ehdri.e_phoff);
    unsigned const e_phnum = get_native16(&ehdri.e_phnum);
    assert(e_phoff == sizeof(Elf32_Ehdr));  // checked by canPack()
    sz_phdrs = e_phnum * get_native16(&ehdri.e_phentsize);

    phdri = new Elf32_Phdr[e_phnum];
    fi->seek(e_phoff, SEEK_SET);
    fi->readx(phdri, sz_phdrs);

    generateElfHdr(fo, linux_elfppc32_fold, getbrk(phdri, e_phnum) );
}

void PackLinuxElf32::pack2(OutputFile *fo, Filter &ft)
{
    Extent x;
    unsigned k;

    // count passes, set ptload vars
    ui_total_passes = 0;
    off_t ptload0hi = 0, ptload1lo = 0, ptload1sz = 0;
    unsigned const e_phnum = get_native16(&ehdri.e_phnum);
    for (k = 0; k < e_phnum; ++k) {
        if (PT_LOAD == get_native32(&phdri[k].p_type)) {
            x.offset = get_native32(&phdri[k].p_offset);
            x.size   = get_native32(&phdri[k].p_filesz);
            if (0 == ptload0hi) {
                ptload0hi = x.offset + x.size;
            }
            else if (0 == ptload1lo) {
                ptload1lo = x.offset;
                ptload1sz = x.size;
            }
            ui_total_passes++;
        }
    }
    if (0!=ptload1sz && ptload0hi < ptload1lo)
        ui_total_passes++;

    // compress extents
    unsigned total_in = 0;
    unsigned total_out = 0;

    ui_pass = -1;  // Compressing Elf headers is invisible to UI.
    x.offset = 0;
    x.size = sizeof(Elf32_Ehdr) + sz_phdrs;
    {
        int const old_level = ph.level; ph.level = 10;
        packExtent(x, total_in, total_out, 0, fo);
        ph.level = old_level;
    }

    ui_pass = 0;
    ft.addvalue = 0;

    int nx = 0;
    for (k = 0; k < e_phnum; ++k) if (PT_LOAD==get_native32(&phdri[k].p_type)) {
        if (ft.id < 0x40) {
            // FIXME: ??    ft.addvalue = phdri[k].p_vaddr;
        }
        x.offset = get_native32(&phdri[k].p_offset);
        x.size   = get_native32(&phdri[k].p_filesz);
        if (0 == nx) { // 1st PT_LOAD must cover Ehdr at 0==p_offset
            unsigned const delta = sizeof(Elf32_Ehdr) + sz_phdrs;
            if (ft.id < 0x40) {
                // FIXME: ??     ft.addvalue += delta;
            }
            x.offset    += delta;
            x.size      -= delta;
        }
        // compressWithFilters() always assumes a "loader", so would
        // throw NotCompressible for small .data Extents, which PowerPC
        // sometimes marks as PF_X anyway.  So filter only first segment.
        packExtent(x, total_in, total_out,
            ((0==nx && (Elf32_Phdr::PF_X & get_native32(&phdri[k].p_flags)))
                ? &ft : 0 ), fo );
        ++nx;
    }
    if (0!=ptload1sz && ptload0hi < ptload1lo) { // alignment hole?
        x.offset = ptload0hi;
        x.size   = ptload1lo - ptload0hi;
        packExtent(x, total_in, total_out, 0, fo);
    }
    if ((off_t)total_in < file_size) {  // non-PT_LOAD stuff
        x.offset = total_in;
        x.size = file_size - total_in;
        packExtent(x, total_in, total_out, 0, fo);
    }

    if ((off_t)total_in != file_size)
        throwEOFException();
    set_native32(&elfout.phdr[0].p_filesz, fo->getBytesWritten());
}

void PackLinuxElf32ppc::pack3(OutputFile *fo, Filter &ft)
{
    unsigned disp;
    unsigned const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 3& -len);  // align to 0 mod 4
    len += (3& -len) + sizeof(disp);
    set_native32(&disp, 4+ len - sz_elf_hdrs);  // 4: sizeof(instruction)
    fo->write(&disp, sizeof(disp));

    super::pack3(fo, ft);
}


void PackLinuxElf32::pack4(OutputFile *fo, Filter &ft)
{
    overlay_offset = sz_elf_hdrs;
    unsigned const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 3& -len);  // align to 0 mod 4
    len += 3& -len;
    set_native32(&elfout.phdr[0].p_filesz, len);
    super::pack4(fo, ft);  // write PackHeader and overlay_offset

#define PAGE_MASK (~0u<<12)
    // pre-calculate for benefit of runtime disappearing act via munmap()
    set_native32(&elfout.phdr[0].p_memsz, PAGE_MASK & (~PAGE_MASK + len));
#undef PAGE_MASK

    // rewrite Elf header
    fo->seek(0, SEEK_SET);
    fo->rewrite(&elfout, overlay_offset);
    fo->rewrite(&linfo, sizeof(linfo));
}

void PackLinuxElf32::unpack(OutputFile *fo)
{
#define MAX_ELF_HDR 512
    char bufehdr[MAX_ELF_HDR];
    Elf32_Ehdr *const ehdr = (Elf32_Ehdr *)bufehdr;
    Elf32_Phdr const *phdr = (Elf32_Phdr *)(1+ehdr);

    unsigned szb_info = sizeof(b_info);
    {
        fi->seek(0, SEEK_SET);
        fi->readx(bufehdr, MAX_ELF_HDR);
        unsigned const e_entry = get_native32(&ehdr->e_entry);
        if (e_entry < 0x401180) { /* old style, 8-byte b_info */
            szb_info = 2*sizeof(unsigned);
        }
    }

    fi->seek(overlay_offset, SEEK_SET);
    p_info hbuf;
    fi->readx(&hbuf, sizeof(hbuf));
    unsigned orig_file_size = get_native32(&hbuf.p_filesize);
    blocksize = get_native32(&hbuf.p_blocksize);
    if (file_size > (off_t)orig_file_size || blocksize > orig_file_size)
        throwCantUnpack("file header corrupted");

    ibuf.alloc(blocksize + OVERHEAD);
    b_info bhdr; memset(&bhdr, 0, sizeof(bhdr));
    fi->readx(&bhdr, szb_info);
    ph.u_len = get_native32(&bhdr.sz_unc);
    ph.c_len = get_native32(&bhdr.sz_cpr);
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
    unsigned const sz_unc = ph.u_len = get_native32(&bhdr.sz_unc);

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
#undef MAX_ELF_HDR
}
/*************************************************************************
//
**************************************************************************/


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
// FIXME 2002-11-11: We use stub/fold_elf86.asm, which calls the
// decompressor multiple times, and unfilter is independent of decompress.
// Currently only filters 0x49, 0x46, 0x80..0x87 can handle this;
// and 0x80..0x87 are regarded as "untested".
#if 0
        0x26, 0x24, 0x11, 0x14, 0x13, 0x16, 0x25, 0x15, 0x12,
#endif
#if 0
        0x83, 0x36, 0x26,
              0x86, 0x80,
        0x84, 0x87, 0x81,
        0x82, 0x85,
        0x24, 0x16, 0x13, 0x14, 0x11, 0x25, 0x15, 0x12,
#endif
    -1 };
    return filters;
}

int
PackLinuxI386elf::buildLoader(const Filter *ft)
{
    unsigned char tmp[sizeof(linux_i386elf_fold)];
    memcpy(tmp, linux_i386elf_fold, sizeof(linux_i386elf_fold));
    checkPatch(NULL, 0, 0, 0);  // reset
    if (opt->o_unix.is_ptinterp) {
        unsigned j;
        for (j = 0; j < sizeof(linux_i386elf_fold)-1; ++j) {
            if (0x60==tmp[  j]
            &&  0x47==tmp[1+j] ) {
                /* put INC EDI before PUSHA: inhibits auxv_up for PT_INTERP */
                tmp[  j] = 0x47;
                tmp[1+j] = 0x60;
                break;
            }
        }
    }
    return buildLinuxLoader(
        linux_i386elf_loader, sizeof(linux_i386elf_loader),
        tmp,                  sizeof(linux_i386elf_fold),  ft );
}


void PackLinuxI386elf::patchLoader() { }


bool PackLinuxI386elf::canPack()
{
    unsigned char buf[sizeof(Elf32_Ehdr) + 14*sizeof(Elf32_Phdr)];
    COMPILE_TIME_ASSERT(sizeof(buf) <= 512);

    exetype = 0;

    fi->readx(buf, sizeof(buf));
    fi->seek(0, SEEK_SET);
    Elf32_Ehdr const *const ehdr = (Elf32_Ehdr const *)buf;

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
    Elf32_Phdr const *phdr = (Elf32_Phdr const *)(buf + ehdr->e_phoff);
    for (unsigned j=0; j < ehdr->e_phnum; ++phdr, ++j) {
        if (j >= 14)
            return false;
        if (phdr->PT_LOAD == phdr->p_type) {
            if (phdr->p_offset != 0) {
                throwCantPack("invalid Phdr p_offset; try `--force-execve'");
                return false;
            }

            // detect possible conflict upon invocation
            if (phdr->p_vaddr < (unsigned)(0x400000 + file_size)
            ||  phdr->p_paddr < (unsigned)(0x400000 + file_size) ) {
                throwAlreadyPackedByUPX();  // not necessarily, but mostly true
                return false;
            }
            exetype = 1;
            break;
        }
    }

    if (!super::canPack())
        return false;
    assert(exetype == 1);

    // set options
    opt->o_unix.blocksize = blocksize = file_size;
    return true;
}


void PackLinuxI386elf::pack1(OutputFile *fo, Filter &)
{
    fi->seek(0, SEEK_SET);
    fi->readx(&ehdri, sizeof(ehdri));
    assert(ehdri.e_phoff == sizeof(Elf32_Ehdr));  // checked by canPack()
    sz_phdrs = ehdri.e_phnum * ehdri.e_phentsize;

    phdri = new Elf32_Phdr[(unsigned)ehdri.e_phnum];
    fi->seek(ehdri.e_phoff, SEEK_SET);
    fi->readx(phdri, sz_phdrs);

    generateElfHdr(fo, linux_i386elf_fold, getbrk(phdri, ehdri.e_phnum) );
}

void PackLinuxI386elf::pack2(OutputFile *fo, Filter &ft)
{
    Extent x;
    unsigned k;

    // count passes, set ptload vars
    ui_total_passes = 0;
    off_t ptload0hi = 0, ptload1lo = 0, ptload1sz = 0;
    for (k = 0; k < ehdri.e_phnum; ++k) {
        if (PT_LOAD == phdri[k].p_type) {
            x.offset = phdri[k].p_offset;
            x.size   = phdri[k].p_filesz;
            if (0 == ptload0hi) {
                ptload0hi = x.offset + x.size;
            }
            else if (0 == ptload1lo) {
                ptload1lo = x.offset;
                ptload1sz = x.size;
            }
            ui_total_passes++;
        }
    }
    if (0!=ptload1sz && ptload0hi < ptload1lo)
        ui_total_passes++;

    // compress extents
    unsigned total_in = 0;
    unsigned total_out = 0;

    ui_pass = -1;  // Compressing Elf headers is invisible to UI.
    x.offset = 0;
    x.size = sizeof(Elf32_Ehdr) + sz_phdrs;
    {
        int const old_level = ph.level; ph.level = 10;
        packExtent(x, total_in, total_out, 0, fo);
        ph.level = old_level;
    }

    ui_pass = 0;
    ft.addvalue = 0;

    int nx = 0;
    for (k = 0; k < ehdri.e_phnum; ++k) if (PT_LOAD==phdri[k].p_type) {
        if (ft.id < 0x40) {
            // FIXME: ??    ft.addvalue = phdri[k].p_vaddr;
        }
        x.offset = phdri[k].p_offset;
        x.size   = phdri[k].p_filesz;
        if (0 == nx) { // 1st PT_LOAD must cover Ehdr at 0==p_offset
            unsigned const delta = sizeof(Elf32_Ehdr) + sz_phdrs;
            if (ft.id < 0x40) {
                // FIXME: ??     ft.addvalue += delta;
            }
            x.offset    += delta;
            x.size      -= delta;
        }
        packExtent(x, total_in, total_out,
            ((Elf32_Phdr::PF_X & phdri[k].p_flags)
                ? &ft : 0 ), fo );
        ++nx;
    }
    if (0!=ptload1sz && ptload0hi < ptload1lo) { // alignment hole?
        x.offset = ptload0hi;
        x.size   = ptload1lo - ptload0hi;
        packExtent(x, total_in, total_out, 0, fo);
    }
    if ((off_t)total_in < file_size) {  // non-PT_LOAD stuff
        x.offset = total_in;
        x.size = file_size - total_in;
        packExtent(x, total_in, total_out, 0, fo);
    }

    if ((off_t)total_in != file_size)
        throwEOFException();
    elfout.phdr[0].p_filesz = fo->getBytesWritten();
}

void PackLinuxI386elf::pack3(OutputFile *fo, Filter &ft)
{
    // We have packed multiple blocks, so PackLinuxI386::buildLinuxLoader
    // needs an adjusted ph.c_len in order to get alignment correct.
    unsigned const save_c_len = ph.c_len;
    ph.c_len = fo->getBytesWritten() - (
            // Elf32_Edhr
        sizeof(elfout.ehdr) +
            // Elf32_Phdr: 1 for exec86, 2 for sh86, 3 for elf86
        (elfout.ehdr.e_phentsize * elfout.ehdr.e_phnum) +
            // checksum UPX! lsize version format
        sizeof(l_info) +
            // PT_DYNAMIC with DT_NEEDED "forwarded" from original file
        ((elfout.ehdr.e_phnum==3) ? (unsigned) elfout.phdr[2].p_memsz : 0u) +
            // p_progid, p_filesize, p_blocksize
        sizeof(p_info) +
            // compressed data
        b_len );
    buildLoader(&ft);  /* redo for final .align constraints */
    ph.c_len = save_c_len;
    super::pack3(fo, ft);
}


void PackLinuxI386elf::unpack(OutputFile *fo)
{
#define MAX_ELF_HDR 512
    char bufehdr[MAX_ELF_HDR];
    Elf32_Ehdr *const ehdr = (Elf32_Ehdr *)bufehdr;
    Elf32_Phdr const *phdr = (Elf32_Phdr *)(1+ehdr);

    unsigned szb_info = sizeof(b_info);
    {
        fi->seek(0, SEEK_SET);
        fi->readx(bufehdr, MAX_ELF_HDR);
        unsigned const e_entry = get_native32(&ehdr->e_entry);
        if (e_entry < 0x401180) { /* old style, 8-byte b_info */
            szb_info = 2*sizeof(unsigned);
        }
    }

    fi->seek(overlay_offset, SEEK_SET);
    p_info hbuf;
    fi->readx(&hbuf, sizeof(hbuf));
    unsigned orig_file_size = get_native32(&hbuf.p_filesize);
    blocksize = get_native32(&hbuf.p_blocksize);
    if (file_size > (off_t)orig_file_size || blocksize > orig_file_size)
        throwCantUnpack("file header corrupted");

    ibuf.alloc(blocksize + OVERHEAD);
    b_info bhdr; memset(&bhdr, 0, sizeof(bhdr));
    fi->readx(&bhdr, szb_info);
    ph.u_len = get_native32(&bhdr.sz_unc);
    ph.c_len = get_native32(&bhdr.sz_cpr);
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
    unsigned const sz_unc = ph.u_len = get_native32(&bhdr.sz_unc);

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
#undef MAX_ELF_HDR
}


/*
vi:ts=4:et
*/

