/* p_lx_elf.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2013 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2013 Laszlo Molnar
   Copyright (C) 2000-2013 John F. Reiser
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
#include "ui.h"

#define PT_LOAD32   Elf32_Phdr::PT_LOAD
#define PT_LOAD64   Elf64_Phdr::PT_LOAD
#define PT_NOTE32   Elf32_Phdr::PT_NOTE
#define PT_NOTE64   Elf64_Phdr::PT_NOTE

//static unsigned const EF_ARM_HASENTRY = 0x02;
static unsigned const EF_ARM_EABI_VER4 = 0x04000000;
static unsigned const EF_ARM_EABI_VER5 = 0x05000000;

unsigned char PackLinuxElf::o_shstrtab[] = {  \
/*start*/       '\0',
/*offset  1*/   '.','n','o','t','e','.','g','n','u','.','b','u','i','l','d','-','i','d','\0',
/*offset 20*/   '.','s','h','s','t','r','t','a','b','\0'
};

static unsigned
umin(unsigned a, unsigned b)
{
    return (a < b) ? a : b;
}

static upx_uint64_t
umin64(upx_uint64_t a, upx_uint64_t b)
{
    return (a < b) ? a : b;
}

static unsigned
up4(unsigned x)
{
    return ~3u & (3+ x);
}

static unsigned
fpad4(OutputFile *fo)
{
    unsigned len = fo->st_size();
    unsigned d = 3u & (0 - len);
    unsigned zero = 0;
    fo->write(&zero, d);
    return d + len;
}

static unsigned
funpad4(InputFile *fi)
{
    unsigned d = 3u & (0 - fi->tell());
    if (d)
        fi->seek(d, SEEK_CUR);
    return d;
}

int
PackLinuxElf32::checkEhdr(Elf32_Ehdr const *ehdr) const
{
    const unsigned char * const buf = ehdr->e_ident;

    if (0!=memcmp(buf, "\x7f\x45\x4c\x46", 4)  // "\177ELF"
    ||  buf[Elf32_Ehdr::EI_CLASS]!=ei_class
    ||  buf[Elf32_Ehdr::EI_DATA] !=ei_data
    ) {
        return -1;
    }
    if (!memcmp(buf+8, "FreeBSD", 7))                   // branded
        return 1;

    int const type = get_te16(&ehdr->e_type);
    if (type != Elf32_Ehdr::ET_EXEC && type != Elf32_Ehdr::ET_DYN)
        return 2;
    if (get_te16(&ehdr->e_machine) != (unsigned) e_machine)
        return 3;
    if (get_te32(&ehdr->e_version) != Elf32_Ehdr::EV_CURRENT)
        return 4;
    if (e_phnum < 1)
        return 5;
    if (get_te16(&ehdr->e_phentsize) != sizeof(Elf32_Phdr))
        return 6;

    if (type == Elf32_Ehdr::ET_EXEC) {
        // check for Linux kernels
        unsigned const entry = get_te32(&ehdr->e_entry);
        if (entry == 0xC0100000)    // uncompressed vmlinux
            return 1000;
        if (entry == 0x00001000)    // compressed vmlinux
            return 1001;
        if (entry == 0x00100000)    // compressed bvmlinux
            return 1002;
    }

    // FIXME: add more checks for kernels

    // FIXME: add special checks for other ELF i386 formats, like
    //        NetBSD, OpenBSD, Solaris, ....

    // success
    return 0;
}

int
PackLinuxElf64::checkEhdr(Elf64_Ehdr const *ehdr) const
{
    const unsigned char * const buf = ehdr->e_ident;
    unsigned char osabi0 = buf[Elf32_Ehdr::EI_OSABI];
    if (0==osabi0) {
        osabi0 = opt->o_unix.osabi0;
    }

    if (0!=memcmp(buf, "\x7f\x45\x4c\x46", 4)  // "\177ELF"
    ||  buf[Elf64_Ehdr::EI_CLASS]!=ei_class
    ||  buf[Elf64_Ehdr::EI_DATA] !=ei_data
    ||                     osabi0!=ei_osabi
    ) {
        return -1;
    }
    if (!memcmp(buf+8, "FreeBSD", 7))                   // branded
        return 1;

    int const type = get_te16(&ehdr->e_type);
    if (type != Elf64_Ehdr::ET_EXEC && type != Elf64_Ehdr::ET_DYN)
        return 2;
    if (get_te16(&ehdr->e_machine) != (unsigned) e_machine)
        return 3;
    if (get_te32(&ehdr->e_version) != Elf64_Ehdr::EV_CURRENT)
        return 4;
    if (e_phnum < 1)
        return 5;
    if (get_te16(&ehdr->e_phentsize) != sizeof(Elf64_Phdr))
        return 6;

    if (type == Elf64_Ehdr::ET_EXEC) {
        // check for Linux kernels
        upx_uint64_t const entry = get_te64(&ehdr->e_entry);
        if (entry == 0xC0100000)    // uncompressed vmlinux
            return 1000;
        if (entry == 0x00001000)    // compressed vmlinux
            return 1001;
        if (entry == 0x00100000)    // compressed bvmlinux
            return 1002;
    }

    // FIXME: add more checks for kernels

    // FIXME: add special checks for other ELF i386 formats, like
    //        NetBSD, OpenBSD, Solaris, ....

    // success
    return 0;
}

PackLinuxElf::PackLinuxElf(InputFile *f)
    : super(f), e_phnum(0), file_image(NULL), dynstr(NULL),
    sz_phdrs(0), sz_elf_hdrs(0), sz_pack2(0), sz_pack2a(0),
    lg2_page(12), page_size(1u<<lg2_page), xct_off(0), xct_va(0),
    e_machine(0), ei_class(0), ei_data(0), ei_osabi(0), osabi_note(NULL),
    o_elf_shnum(0)
{
}

PackLinuxElf::~PackLinuxElf()
{
    delete[] file_image; file_image = NULL;
}

void PackLinuxElf::pack3(OutputFile *fo, Filter &ft)
{
    unsigned disp;
    unsigned const zero = 0;
    unsigned len = sz_pack2a;  // after headers and all PT_LOAD

    unsigned const t = (4 & len) ^ ((!!xct_off)<<2);  // 0 or 4
    fo->write(&zero, t);
    len += t;

    set_te32(&disp, 2*sizeof(disp) + len - (sz_elf_hdrs + sizeof(p_info) + sizeof(l_info)));
    fo->write(&disp, sizeof(disp));  // .e_entry - &first_b_info
    len += sizeof(disp);
    set_te32(&disp, len);  // distance back to beginning (detect dynamic reloc)
    fo->write(&disp, sizeof(disp));
    len += sizeof(disp);

    if (xct_off) {  // is_shlib
        set_te32(&disp, elf_unsigned_dynamic(Elf32_Dyn::DT_INIT) - load_va);
        fo->write(&disp, sizeof(disp));
        len += sizeof(disp);

        set_te32(&disp, hatch_off);
        fo->write(&disp, sizeof(disp));
        len += sizeof(disp);

        set_te32(&disp, xct_off);
        fo->write(&disp, sizeof(disp));
        len += sizeof(disp);
    }
    sz_pack2 = len;  // 0 mod 8

    super::pack3(fo, ft);  // append the decompressor
    set_te16(&linfo.l_lsize, up4(  // MATCH03: up4
    get_te16(&linfo.l_lsize) + len - sz_pack2a));

    len = fpad4(fo);  // MATCH03
}

void PackLinuxElf32::pack3(OutputFile *fo, Filter &ft)
{
    super::pack3(fo, ft);  // loader follows compressed PT_LOADs
    // Then compressed gaps (including debuginfo.)
    unsigned total_in = 0, total_out = 0;
    for (unsigned k = 0; k < e_phnum; ++k) {
        Extent x;
        x.size = find_LOAD_gap(phdri, k, e_phnum);
        if (x.size) {
            x.offset = get_te32(&phdri[k].p_offset) +
                       get_te32(&phdri[k].p_filesz);
            packExtent(x, total_in, total_out, 0, fo);
        }
    }
    // write block end marker (uncompressed size 0)
    b_info hdr; memset(&hdr, 0, sizeof(hdr));
    set_le32(&hdr.sz_cpr, UPX_MAGIC_LE32);
    fo->write(&hdr, sizeof(hdr));
    fpad4(fo);

    set_te32(&elfout.phdr[0].p_filesz, sz_pack2 + lsize);
    set_te32(&elfout.phdr[0].p_memsz,  sz_pack2 + lsize);
    if (0!=xct_off) {  // shared library
        Elf32_Phdr *phdr = phdri;
        unsigned off = fo->st_size();
        unsigned off_init = 0;  // where in file
        unsigned va_init = sz_pack2;   // virtual address
        unsigned rel = 0;
        unsigned old_dtinit = 0;
        for (int j = e_phnum; --j>=0; ++phdr) {
            unsigned const len  = get_te32(&phdr->p_filesz);
            unsigned const ioff = get_te32(&phdr->p_offset);
            unsigned const type = get_te32(&phdr->p_type);
            if (phdr->PT_INTERP==type) {
                // Rotate to highest position, so it can be lopped
                // by decrementing e_phnum.
                memcpy((unsigned char *)ibuf, phdr, sizeof(*phdr));
                memmove(phdr, 1+phdr, j * sizeof(*phdr));  // overlapping
                memcpy(&phdr[j], (unsigned char *)ibuf, sizeof(*phdr));
                --phdr;
                set_te16(&ehdri.e_phnum, --e_phnum);
                continue;
            }
            if (phdr->PT_LOAD32==type) {
                if (xct_off < ioff) {  // Slide up non-first PT_LOAD.
                    fi->seek(ioff, SEEK_SET);
                    fi->readx(ibuf, len);
                    off += ~page_mask & (ioff - off);
                    fo->seek(off, SEEK_SET);
                    fo->write(ibuf, len);
                    rel = off - ioff;
                    set_te32(&phdr->p_offset, rel + ioff);
                }
                else {  // Change length of first PT_LOAD.
                    va_init += get_te32(&phdr->p_vaddr);
                    set_te32(&phdr->p_filesz, sz_pack2 + lsize);
                    set_te32(&phdr->p_memsz,  sz_pack2 + lsize);
                }
                continue;  // all done with this PT_LOAD
            }
            // Compute new offset of &DT_INIT.d_val.
            if (phdr->PT_DYNAMIC==type) {
                off_init = rel + ioff;
                fi->seek(ioff, SEEK_SET);
                fi->read(ibuf, len);
                Elf32_Dyn *dyn = (Elf32_Dyn *)(void *)ibuf;
                for (int j2 = len; j2 > 0; ++dyn, j2 -= sizeof(*dyn)) {
                    if (dyn->DT_INIT==get_te32(&dyn->d_tag)) {
                        old_dtinit = dyn->d_val;  // copy ONLY, never examined
                        unsigned const t = (unsigned char *)&dyn->d_val -
                                           (unsigned char *)ibuf;
                        off_init += t;
                        break;
                    }
                }
                // fall through to relocate .p_offset
            }
            if (xct_off < ioff)
                set_te32(&phdr->p_offset, rel + ioff);
        }
        if (off_init) {  // change DT_INIT.d_val
            fo->seek(off_init, SEEK_SET);
            va_init |= (Elf32_Ehdr::EM_ARM==e_machine);  // THUMB mode
            unsigned word; set_te32(&word, va_init);
            fo->rewrite(&word, sizeof(word));
            fo->seek(0, SEEK_END);
        }
        ehdri.e_shnum = 0;
        ehdri.e_shoff = old_dtinit;  // easy to find for unpacking
        ehdri.e_shentsize = 0;
        ehdri.e_shstrndx = 0;
    }
}

void PackLinuxElf64::pack3(OutputFile *fo, Filter &ft)
{
    super::pack3(fo, ft);  // loader follows compressed PT_LOADs
    // Then compressed gaps (including debuginfo.)
    unsigned total_in = 0, total_out = 0;
    for (unsigned k = 0; k < e_phnum; ++k) {
        Extent x;
        x.size = find_LOAD_gap(phdri, k, e_phnum);
        if (x.size) {
            x.offset = get_te64(&phdri[k].p_offset) +
                       get_te64(&phdri[k].p_filesz);
            packExtent(x, total_in, total_out, 0, fo);
        }
    }
    // write block end marker (uncompressed size 0)
    b_info hdr; memset(&hdr, 0, sizeof(hdr));
    set_le32(&hdr.sz_cpr, UPX_MAGIC_LE32);
    fo->write(&hdr, sizeof(hdr));
    fpad4(fo);

    set_te64(&elfout.phdr[0].p_filesz, sz_pack2 + lsize);
    set_te64(&elfout.phdr[0].p_memsz,  sz_pack2 + lsize);
    if (0!=xct_off) {  // shared library
        Elf64_Phdr *phdr = phdri;
        unsigned off = fo->st_size();
        unsigned off_init = 0;  // where in file
        upx_uint64_t va_init = sz_pack2;   // virtual address
        upx_uint64_t rel = 0;
        upx_uint64_t old_dtinit = 0;
        for (int j = e_phnum; --j>=0; ++phdr) {
            upx_uint64_t const len  = get_te64(&phdr->p_filesz);
            upx_uint64_t const ioff = get_te64(&phdr->p_offset);
            upx_uint64_t       align= get_te64(&phdr->p_align);
            unsigned const type = get_te32(&phdr->p_type);
            if (phdr->PT_INTERP==type) {
                // Rotate to highest position, so it can be lopped
                // by decrementing e_phnum.
                memcpy((unsigned char *)ibuf, phdr, sizeof(*phdr));
                memmove(phdr, 1+phdr, j * sizeof(*phdr));  // overlapping
                memcpy(&phdr[j], (unsigned char *)ibuf, sizeof(*phdr));
                --phdr;
                set_te16(&ehdri.e_phnum, --e_phnum);
                continue;
            }
            if (phdr->PT_LOAD==type) {
                if (xct_off < ioff) {  // Slide up non-first PT_LOAD.
                    // AMD64 chip supports page sizes of 4KiB, 2MiB, and 1GiB;
                    // the operating system chooses one.  .p_align typically
                    // is a forward-looking 2MiB.  In 2009 Linux chooses 4KiB.
                    // We choose 4KiB to waste less space.  If Linux chooses
                    // 2MiB later, then our output will not run.
                    if ((1u<<12) < align) {
                        align = 1u<<12;
                        set_te64(&phdr->p_align, align);
                    }
                    off += (align-1) & (ioff - off);
                    fi->seek(ioff, SEEK_SET); fi->readx(ibuf, len);
                    fo->seek( off, SEEK_SET); fo->write(ibuf, len);
                    rel = off - ioff;
                    set_te64(&phdr->p_offset, rel + ioff);
                }
                else {  // Change length of first PT_LOAD.
                    va_init += get_te64(&phdr->p_vaddr);
                    set_te64(&phdr->p_filesz, sz_pack2 + lsize);
                    set_te64(&phdr->p_memsz,  sz_pack2 + lsize);
                }
                continue;  // all done with this PT_LOAD
            }
            // Compute new offset of &DT_INIT.d_val.
            if (phdr->PT_DYNAMIC==type) {
                off_init = rel + ioff;
                fi->seek(ioff, SEEK_SET);
                fi->read(ibuf, len);
                Elf64_Dyn *dyn = (Elf64_Dyn *)(void *)ibuf;
                for (int j2 = len; j2 > 0; ++dyn, j2 -= sizeof(*dyn)) {
                    if (dyn->DT_INIT==get_te64(&dyn->d_tag)) {
                        old_dtinit = dyn->d_val;  // copy ONLY, never examined
                        unsigned const t = (unsigned char *)&dyn->d_val -
                                           (unsigned char *)ibuf;
                        off_init += t;
                        break;
                    }
                }
                // fall through to relocate .p_offset
            }
            if (xct_off < ioff)
                set_te64(&phdr->p_offset, rel + ioff);
        }
        if (off_init) {  // change DT_INIT.d_val
            fo->seek(off_init, SEEK_SET);
            upx_uint64_t word; set_te64(&word, va_init);
            fo->rewrite(&word, sizeof(word));
            fo->seek(0, SEEK_END);
        }
        ehdri.e_shnum = 0;
        ehdri.e_shoff = old_dtinit;  // easy to find for unpacking
        //ehdri.e_shentsize = 0;
        //ehdri.e_shstrndx = 0;
    }
}

void
PackLinuxElf::addStubEntrySections(Filter const *)
{
    addLoader("ELFMAINX", NULL);
    if (hasLoaderSection("ELFMAINXu")) {
        int const all_pages = opt->o_unix.unmap_all_pages ||
            // brk() trouble if static
            (Elf32_Ehdr::EM_ARM==e_machine && 0x8000==load_va);
        addLoader((all_pages ? "LUNMP000" : "LUNMP001"), "ELFMAINXu", NULL);
    }
   //addLoader(getDecompressorSections(), NULL);
    addLoader(
        ( M_IS_NRV2E(ph.method) ? "NRV_HEAD,NRV2E,NRV_TAIL"
        : M_IS_NRV2D(ph.method) ? "NRV_HEAD,NRV2D,NRV_TAIL"
        : M_IS_NRV2B(ph.method) ? "NRV_HEAD,NRV2B,NRV_TAIL"
        : M_IS_LZMA(ph.method)  ? "LZMA_ELF00,+80C,LZMA_DEC20,LZMA_DEC30"
        : NULL), NULL);
    if (hasLoaderSection("CFLUSH"))
        addLoader("CFLUSH");
    addLoader("ELFMAINY,IDENTSTR,+40,ELFMAINZ", NULL);
    if (hasLoaderSection("ELFMAINZu")) {
        addLoader((opt->o_unix.unmap_all_pages ? "LUNMP000" : "LUNMP001"), "ELFMAINZu", NULL);
    }
    addLoader("FOLDEXEC", NULL);
}


void PackLinuxElf::defineSymbols(Filter const *)
{
    // empty
}

PackLinuxElf32::PackLinuxElf32(InputFile *f)
    : super(f), phdri(NULL), note_body(NULL), shdri(NULL),
    page_mask(~0u<<lg2_page),
    dynseg(NULL), hashtab(NULL), gashtab(NULL), dynsym(NULL),
    shstrtab(NULL), n_elf_shnum(0),
    sec_strndx(NULL), sec_dynsym(NULL), sec_dynstr(NULL)
{
    memset(&ehdri, 0, sizeof(ehdri));
    if (f) {
        f->seek(0, SEEK_SET);
        f->readx(&ehdri, sizeof(ehdri));
    }
}

PackLinuxElf32::~PackLinuxElf32()
{
    delete[] note_body;
    delete[] phdri; phdri = NULL;
}

PackLinuxElf64::PackLinuxElf64(InputFile *f)
    : super(f), phdri(NULL), note_body(NULL), shdri(NULL),
    page_mask(~0ull<<lg2_page),
    dynseg(NULL), hashtab(NULL), gashtab(NULL), dynsym(NULL),
    shstrtab(NULL), n_elf_shnum(0),
    sec_strndx(NULL), sec_dynsym(NULL), sec_dynstr(NULL)
{
    memset(&ehdri, 0, sizeof(ehdri));
    if (f) {
        f->seek(0, SEEK_SET);
        f->readx(&ehdri, sizeof(ehdri));
    }
}

PackLinuxElf64::~PackLinuxElf64()
{
    delete[] note_body;
    delete[] phdri; phdri = NULL;
}

Linker* PackLinuxElf64amd::newLinker() const
{
    return new ElfLinkerAMD64;
}

int const *
PackLinuxElf::getCompressionMethods(int method, int level) const
{
    // No real dependency on LE32.
    return Packer::getDefaultCompressionMethods_le32(method, level);
}

int const *
PackLinuxElf32armLe::getCompressionMethods(int method, int level) const
{
    return Packer::getDefaultCompressionMethods_8(method, level);
}

int const *
PackLinuxElf32armBe::getCompressionMethods(int method, int level) const
{
    return Packer::getDefaultCompressionMethods_8(method, level);
}

int const *
PackLinuxElf32ppc::getFilters() const
{
    static const int filters[] = {
        0xd0,
    FT_END };
    return filters;
}

int const *
PackLinuxElf64amd::getFilters() const
{
    static const int filters[] = {
        0x49,
    FT_END };
    return filters;
}

void PackLinuxElf32::patchLoader()
{
}

void PackLinuxElf64::patchLoader()
{
}

void PackLinuxElf32::ARM_updateLoader(OutputFile * /*fo*/)
{
    set_te32(&elfout.ehdr.e_entry, sz_pack2 +
        linker->getSymbolOffset("_start") +
        get_te32(&elfout.phdr[0].p_vaddr));
}

void PackLinuxElf32armLe::updateLoader(OutputFile *fo)
{
    ARM_updateLoader(fo);
}

void PackLinuxElf32armBe::updateLoader(OutputFile *fo)
{
    ARM_updateLoader(fo);
}

void PackLinuxElf32mipsel::updateLoader(OutputFile *fo)
{
    ARM_updateLoader(fo);  // not ARM specific; (no 32-bit immediates)
}

void PackLinuxElf32mipseb::updateLoader(OutputFile *fo)
{
    ARM_updateLoader(fo);  // not ARM specific; (no 32-bit immediates)
}

void PackLinuxElf32::updateLoader(OutputFile * /*fo*/)
{
    set_te32(&elfout.ehdr.e_entry, sz_pack2 +
        get_te32(&elfout.phdr[0].p_vaddr));
}

void PackLinuxElf64::updateLoader(OutputFile * /*fo*/)
{
    set_te64(&elfout.ehdr.e_entry, sz_pack2 +
        get_te64(&elfout.phdr[0].p_vaddr));
}

PackLinuxElf32ppc::PackLinuxElf32ppc(InputFile *f)
    : super(f)
{
    e_machine = Elf32_Ehdr::EM_PPC;
    ei_class  = Elf32_Ehdr::ELFCLASS32;
    ei_data   = Elf32_Ehdr::ELFDATA2MSB;
    ei_osabi  = Elf32_Ehdr::ELFOSABI_LINUX;
}

PackLinuxElf32ppc::~PackLinuxElf32ppc()
{
}

Linker* PackLinuxElf32ppc::newLinker() const
{
    return new ElfLinkerPpc32;
}

PackLinuxElf64amd::PackLinuxElf64amd(InputFile *f)
    : super(f)
{
    e_machine = Elf64_Ehdr::EM_X86_64;
    ei_class = Elf64_Ehdr::ELFCLASS64;
    ei_data = Elf64_Ehdr::ELFDATA2LSB;
    ei_osabi  = Elf32_Ehdr::ELFOSABI_LINUX;
}

PackLinuxElf64amd::~PackLinuxElf64amd()
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

void PackLinuxElf32x86::addStubEntrySections(Filter const *ft)
{
    int const n_mru = ft->n_mru;  // FIXME: belongs to filter? packerf?

// Rely on "+80CXXXX" [etc] in getDecompressorSections() packer_c.cpp */
//    // Here is a quick summary of the format of the output file:
//    linker->setLoaderAlignOffset(
//            // Elf32_Ehdr
//        sizeof(elfout.ehdr) +
//            // Elf32_Phdr: 1 for exec86, 2 for sh86, 3 for elf86
//        (get_te16(&elfout.ehdr.e_phentsize) * get_te16(&elfout.ehdr.e_phnum)) +
//            // checksum UPX! lsize version format
//        sizeof(l_info) +
//            // PT_DYNAMIC with DT_NEEDED "forwarded" from original file
//        ((get_te16(&elfout.ehdr.e_phnum)==3)
//            ? (unsigned) get_te32(&elfout.phdr[2].p_memsz)
//            : 0) +
//            // p_progid, p_filesize, p_blocksize
//        sizeof(p_info) +
//            // compressed data
//        b_len + ph.c_len );

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
    if (Elf32_Ehdr::ET_DYN==get_te16(&ehdri.e_type)) {
        addLoader("LEXECDYN", NULL);
    }
    addLoader((opt->o_unix.unmap_all_pages ? "LUNMP000" : "LUNMP001"), "LEXEC025", NULL);
    addLoader("FOLDEXEC", NULL);
}

void PackLinuxElf32x86::defineSymbols(Filter const *const ft)
{
    if (0x80==(ft->id & 0xF0)) {
        int const mru = ft->n_mru ? 1+ ft->n_mru : 0;
        if (mru && mru!=256) {
            unsigned const is_pwr2 = (0==((mru -1) & mru));
            linker->defineSymbol("NMRU", mru - is_pwr2);
        }
    }
}

void
PackLinuxElf32::buildLinuxLoader(
    upx_byte const *const proto,
    unsigned        const szproto,
    upx_byte const *const fold,
    unsigned        const szfold,
    Filter const *ft
)
{
    initLoader(proto, szproto);

  if (0 < szfold) {
    struct b_info h; memset(&h, 0, sizeof(h));
    unsigned fold_hdrlen = 0;
    cprElfHdr1 const *const hf = (cprElfHdr1 const *)fold;
    fold_hdrlen = umax(0x80, sizeof(hf->ehdr) +
        get_te16(&hf->ehdr.e_phentsize) * get_te16(&hf->ehdr.e_phnum) +
            sizeof(l_info) );
    h.sz_unc = ((szfold < fold_hdrlen) ? 0 : (szfold - fold_hdrlen));
    h.b_method = (unsigned char) ph.method;
    h.b_ftid = (unsigned char) ph.filter;
    h.b_cto8 = (unsigned char) ph.filter_cto;
    unsigned char const *const uncLoader = fold_hdrlen + fold;

    h.sz_cpr = MemBuffer::getSizeForCompression(h.sz_unc + (0==h.sz_unc));
    unsigned char *const cprLoader = new unsigned char[sizeof(h) + h.sz_cpr];
    int r = upx_compress(uncLoader, h.sz_unc, sizeof(h) + cprLoader, &h.sz_cpr,
        NULL, ph.method, 10, NULL, NULL );
    if (r != UPX_E_OK || h.sz_cpr >= h.sz_unc)
        throwInternalError("loader compression failed");
#if 0  //{  debugging only
    if (M_IS_LZMA(ph.method)) {
        ucl_uint tmp_len = h.sz_unc;  // LZMA uses this as EOF
        unsigned char *tmp = new unsigned char[tmp_len];
        memset(tmp, 0, tmp_len);
        r = upx_decompress(sizeof(h) + cprLoader, h.sz_cpr, tmp, &tmp_len, h.b_method, NULL);
        if (r == UPX_E_OUT_OF_MEMORY)
            throwOutOfMemoryException();
        printf("\n%d %d: %d %d %d\n", h.b_method, r, h.sz_cpr, h.sz_unc, tmp_len);
        for (unsigned j=0; j < h.sz_unc; ++j) if (tmp[j]!=uncLoader[j]) {
            printf("%d: %x %x\n", j, tmp[j], uncLoader[j]);
        }
        delete[] tmp;
    }
#endif  //}
    unsigned const sz_cpr = h.sz_cpr;
    set_te32(&h.sz_cpr, h.sz_cpr);
    set_te32(&h.sz_unc, h.sz_unc);
    memcpy(cprLoader, &h, sizeof(h));

    // This adds the definition to the "library", to be used later.
    linker->addSection("FOLDEXEC", cprLoader, sizeof(h) + sz_cpr, 0);
    delete [] cprLoader;
  }
  else {
    linker->addSection("FOLDEXEC", "", 0, 0);
  }

    addStubEntrySections(ft);

    if (0==xct_off)
        defineSymbols(ft);  // main program only, not for shared lib
    relocateLoader();
}

void
PackLinuxElf64::buildLinuxLoader(
    upx_byte const *const proto,
    unsigned        const szproto,
    upx_byte const *const fold,
    unsigned        const szfold,
    Filter const *ft
)
{
    initLoader(proto, szproto);

  if (0 < szfold) {
    struct b_info h; memset(&h, 0, sizeof(h));
    unsigned fold_hdrlen = 0;
    cprElfHdr1 const *const hf = (cprElfHdr1 const *)fold;
    fold_hdrlen = umax(0x80, sizeof(hf->ehdr) +
        get_te16(&hf->ehdr.e_phentsize) * get_te16(&hf->ehdr.e_phnum) +
            sizeof(l_info) );
    h.sz_unc = ((szfold < fold_hdrlen) ? 0 : (szfold - fold_hdrlen));
    h.b_method = (unsigned char) ph.method;
    h.b_ftid = (unsigned char) ph.filter;
    h.b_cto8 = (unsigned char) ph.filter_cto;
    unsigned char const *const uncLoader = fold_hdrlen + fold;

    h.sz_cpr = MemBuffer::getSizeForCompression(h.sz_unc + (0==h.sz_unc));
    unsigned char *const cprLoader = new unsigned char[sizeof(h) + h.sz_cpr];
    int r = upx_compress(uncLoader, h.sz_unc, sizeof(h) + cprLoader, &h.sz_cpr,
        NULL, ph.method, 10, NULL, NULL );
    if (r != UPX_E_OK || h.sz_cpr >= h.sz_unc)
        throwInternalError("loader compression failed");
    unsigned const sz_cpr = h.sz_cpr;
    set_te32(&h.sz_cpr, h.sz_cpr);
    set_te32(&h.sz_unc, h.sz_unc);
    memcpy(cprLoader, &h, sizeof(h));

    // This adds the definition to the "library", to be used later.
    linker->addSection("FOLDEXEC", cprLoader, sizeof(h) + sz_cpr, 0);
    delete [] cprLoader;
  }
  else {
    linker->addSection("FOLDEXEC", "", 0, 0);
  }

    addStubEntrySections(ft);

    if (0==xct_off)
        defineSymbols(ft);  // main program only, not for shared lib
    relocateLoader();
}

void
PackLinuxElf64amd::defineSymbols(Filter const *)
{
    unsigned const hlen = sz_elf_hdrs + sizeof(l_info) + sizeof(p_info);

    // We want to know if compressed data, plus stub, plus a couple pages,
    // will fit below the uncompressed program in memory.  But we don't
    // know the final total compressed size yet, so use the uncompressed
    // size (total over all PT_LOAD64) as an upper bound.
    unsigned len = 0;
    upx_uint64_t lo_va_user = ~0ull;  // infinity
    for (int j= e_phnum; --j>=0; ) {
        if (PT_LOAD64 == get_te32(&phdri[j].p_type)) {
            len += (unsigned)get_te64(&phdri[j].p_filesz);
            upx_uint64_t const va = get_te64(&phdri[j].p_vaddr);
            if (va < lo_va_user) {
                lo_va_user = va;
            }
        }
    }
    lsize = /*getLoaderSize()*/  64 * 1024;  // XXX: upper bound; avoid circularity
    upx_uint64_t       lo_va_stub = get_te64(&elfout.phdr[0].p_vaddr);
    upx_uint64_t adrc;
    upx_uint64_t adrm;
    upx_uint64_t adru;
    upx_uint64_t adrx;
    unsigned cntc;
    unsigned lenm;
    unsigned lenu;
    len += (7&-lsize) + lsize;
    bool const is_big = (lo_va_user < (lo_va_stub + len + 2*page_size));
    if (is_big && ehdri.ET_EXEC==get_te16(&ehdri.e_type)) {
        set_te64(    &elfout.ehdr.e_entry,
            get_te64(&elfout.ehdr.e_entry) + lo_va_user - lo_va_stub);
        set_te64(&elfout.phdr[0].p_vaddr, lo_va_user);
        set_te64(&elfout.phdr[0].p_paddr, lo_va_user);
               lo_va_stub      = lo_va_user;
        adrc = lo_va_stub;
        adrm = getbrk(phdri, e_phnum);
        adru = page_mask & (~page_mask + adrm);  // round up to page boundary
        adrx = adru + hlen;
        lenm = page_size + len;
        lenu = page_size + len;
        cntc = len >> 3;  // over-estimate; corrected at runtime
    }
    else {
        adrm = lo_va_stub + len;
        adrc = adrm;
        adru = lo_va_stub;
        adrx = lo_va_stub + hlen;
        lenm = page_size;
        lenu = page_size + len;
        cntc = 0;
    }
    adrm = page_mask & (~page_mask + adrm);  // round up to page boundary
    adrc = page_mask & (~page_mask + adrc);  // round up to page boundary

    //linker->defineSymbol("ADRX", adrx); // compressed input for eXpansion
    ACC_UNUSED(adrx);

    // For actual moving, we need the true count, which depends on sz_pack2
    // and is not yet known.  So the runtime stub detects "no move"
    // if adrm==adrc, and otherwise uses actual sz_pack2 to compute cntc.
    //linker->defineSymbol("CNTC", cntc);  // count  for copy
    ACC_UNUSED(cntc);

    linker->defineSymbol("LENU", lenu);  // len  for unmap
    linker->defineSymbol("ADRC", adrc);  // addr for copy
    //linker->defineSymbol("ADRU", adru);  // addr for unmap
#define EI_NIDENT 16  /* <elf.h> */
    linker->defineSymbol("JMPU", EI_NIDENT -4 + lo_va_user);  // unmap trampoline
#undef EI_NIDENT
    linker->defineSymbol("LENM", lenm);  // len  for map
    linker->defineSymbol("ADRM", adrm);  // addr for map

    //linker->dumpSymbols();  // debug
}

static const
#include "stub/i386-linux.elf-entry.h"
static const
#include "stub/i386-linux.elf-fold.h"
static const
#include "stub/i386-linux.shlib-init.h"

void
PackLinuxElf32x86::buildLoader(const Filter *ft)
{
    if (0!=xct_off) {  // shared library
        buildLinuxLoader(
            stub_i386_linux_shlib_init, sizeof(stub_i386_linux_shlib_init),
            NULL,                       0,                                ft );
        return;
    }
    unsigned char tmp[sizeof(stub_i386_linux_elf_fold)];
    memcpy(tmp, stub_i386_linux_elf_fold, sizeof(stub_i386_linux_elf_fold));
    checkPatch(NULL, 0, 0, 0);  // reset
    if (opt->o_unix.is_ptinterp) {
        unsigned j;
        for (j = 0; j < sizeof(stub_i386_linux_elf_fold)-1; ++j) {
            if (0x60==tmp[  j]
            &&  0x47==tmp[1+j] ) {
                /* put INC EDI before PUSHA: inhibits auxv_up for PT_INTERP */
                tmp[  j] = 0x47;
                tmp[1+j] = 0x60;
                break;
            }
        }
    }
    buildLinuxLoader(
        stub_i386_linux_elf_entry, sizeof(stub_i386_linux_elf_entry),
        tmp,                       sizeof(stub_i386_linux_elf_fold),  ft );
}

static const
#include "stub/i386-bsd.elf-entry.h"
static const
#include "stub/i386-bsd.elf-fold.h"

void
PackBSDElf32x86::buildLoader(const Filter *ft)
{
    unsigned char tmp[sizeof(stub_i386_bsd_elf_fold)];
    memcpy(tmp, stub_i386_bsd_elf_fold, sizeof(stub_i386_bsd_elf_fold));
    checkPatch(NULL, 0, 0, 0);  // reset
    if (opt->o_unix.is_ptinterp) {
        unsigned j;
        for (j = 0; j < sizeof(stub_i386_bsd_elf_fold)-1; ++j) {
            if (0x60==tmp[  j]
            &&  0x47==tmp[1+j] ) {
                /* put INC EDI before PUSHA: inhibits auxv_up for PT_INTERP */
                tmp[  j] = 0x47;
                tmp[1+j] = 0x60;
                break;
            }
        }
    }
    buildLinuxLoader(
        stub_i386_bsd_elf_entry, sizeof(stub_i386_bsd_elf_entry),
        tmp,                     sizeof(stub_i386_bsd_elf_fold), ft);
}

static const
#include "stub/i386-netbsd.elf-entry.h"

static const
#include "stub/i386-netbsd.elf-fold.h"

#define WANT_NHDR_ENUM
#include "p_elf_enum.h"

void
PackNetBSDElf32x86::buildLoader(const Filter *ft)
{
    unsigned char tmp[sizeof(stub_i386_netbsd_elf_fold)];
    memcpy(tmp, stub_i386_netbsd_elf_fold, sizeof(stub_i386_netbsd_elf_fold));
    checkPatch(NULL, 0, 0, 0);  // reset
    if (opt->o_unix.is_ptinterp) {
        unsigned j;
        for (j = 0; j < sizeof(stub_i386_netbsd_elf_fold)-1; ++j) {
            if (0x60==tmp[  j]
            &&  0x47==tmp[1+j] ) {
                /* put INC EDI before PUSHA: inhibits auxv_up for PT_INTERP */
                tmp[  j] = 0x47;
                tmp[1+j] = 0x60;
                break;
            }
        }
    }
    buildLinuxLoader(
        stub_i386_netbsd_elf_entry, sizeof(stub_i386_netbsd_elf_entry),
        tmp,                        sizeof(stub_i386_netbsd_elf_fold), ft);
}

static const
#include "stub/i386-openbsd.elf-fold.h"

void
PackOpenBSDElf32x86::buildLoader(const Filter *ft)
{
    unsigned char tmp[sizeof(stub_i386_openbsd_elf_fold)];
    memcpy(tmp, stub_i386_openbsd_elf_fold, sizeof(stub_i386_openbsd_elf_fold));
    checkPatch(NULL, 0, 0, 0);  // reset
    if (opt->o_unix.is_ptinterp) {
        unsigned j;
        for (j = 0; j < sizeof(stub_i386_openbsd_elf_fold)-1; ++j) {
            if (0x60==tmp[  j]
            &&  0x47==tmp[1+j] ) {
                /* put INC EDI before PUSHA: inhibits auxv_up for PT_INTERP */
                tmp[  j] = 0x47;
                tmp[1+j] = 0x60;
                break;
            }
        }
    }
    buildLinuxLoader(
        stub_i386_bsd_elf_entry, sizeof(stub_i386_bsd_elf_entry),
        tmp,                     sizeof(stub_i386_openbsd_elf_fold), ft);
}

static const
#include "stub/armel-eabi-linux.elf-entry.h"
static const
#include "stub/armel-eabi-linux.elf-fold.h"
static const
#include "stub/thumb-eabi-linux.shlib-init.h"

static const
#include "stub/arm-linux.elf-entry.h"
static const
#include "stub/arm-linux.elf-fold.h"
#if 0
static const
#include "stub/arm-linux.shlib-init.h"
#endif

static const
#include "stub/armeb-linux.elf-entry.h"
static const
#include "stub/armeb-linux.elf-fold.h"

#include "mem.h"

void
PackLinuxElf32armBe::buildLoader(Filter const *ft)
{
    buildLinuxLoader(
        stub_armeb_linux_elf_entry, sizeof(stub_armeb_linux_elf_entry),
        stub_armeb_linux_elf_fold,  sizeof(stub_armeb_linux_elf_fold), ft);
}

void
PackLinuxElf32armLe::buildLoader(Filter const *ft)
{
    if (Elf32_Ehdr::ELFOSABI_LINUX==ei_osabi) {

        if (0!=xct_off) {  // shared library
            buildLinuxLoader(
                stub_thumb_eabi_linux_shlib_init, sizeof(stub_thumb_eabi_linux_shlib_init),
                NULL,                      0,                                ft );
            return;
        }
        buildLinuxLoader(
            stub_armel_eabi_linux_elf_entry, sizeof(stub_armel_eabi_linux_elf_entry),
            stub_armel_eabi_linux_elf_fold,  sizeof(stub_armel_eabi_linux_elf_fold), ft);
    }
    else {
        buildLinuxLoader(
            stub_arm_linux_elf_entry,        sizeof(stub_arm_linux_elf_entry),
            stub_arm_linux_elf_fold,         sizeof(stub_arm_linux_elf_fold), ft);
    }
}

static const
#include "stub/mipsel.r3000-linux.elf-entry.h"
static const
#include "stub/mipsel.r3000-linux.elf-fold.h"

void
PackLinuxElf32mipsel::buildLoader(Filter const *ft)
{
    buildLinuxLoader(
        stub_mipsel_r3000_linux_elf_entry, sizeof(stub_mipsel_r3000_linux_elf_entry),
        stub_mipsel_r3000_linux_elf_fold,  sizeof(stub_mipsel_r3000_linux_elf_fold), ft);
}

static const
#include "stub/mips.r3000-linux.elf-entry.h"
static const
#include "stub/mips.r3000-linux.elf-fold.h"

void
PackLinuxElf32mipseb::buildLoader(Filter const *ft)
{
    buildLinuxLoader(
        stub_mips_r3000_linux_elf_entry, sizeof(stub_mips_r3000_linux_elf_entry),
        stub_mips_r3000_linux_elf_fold,  sizeof(stub_mips_r3000_linux_elf_fold), ft);
}

static const
#include "stub/powerpc-linux.elf-entry.h"
static const
#include "stub/powerpc-linux.elf-fold.h"

void
PackLinuxElf32ppc::buildLoader(const Filter *ft)
{
    buildLinuxLoader(
        stub_powerpc_linux_elf_entry, sizeof(stub_powerpc_linux_elf_entry),
        stub_powerpc_linux_elf_fold,  sizeof(stub_powerpc_linux_elf_fold), ft);
}

static const
#include "stub/amd64-linux.elf-entry.h"
static const
#include "stub/amd64-linux.elf-fold.h"
static const
#include "stub/amd64-linux.shlib-init.h"

void
PackLinuxElf64amd::buildLoader(const Filter *ft)
{
    if (0!=xct_off) {  // shared library
        buildLinuxLoader(
            stub_amd64_linux_shlib_init, sizeof(stub_amd64_linux_shlib_init),
            NULL,                        0,                                 ft );
        return;
    }
    buildLinuxLoader(
        stub_amd64_linux_elf_entry, sizeof(stub_amd64_linux_elf_entry),
        stub_amd64_linux_elf_fold,  sizeof(stub_amd64_linux_elf_fold), ft);
}

Elf32_Shdr const *PackLinuxElf32::elf_find_section_name(
    char const *const name
) const
{
    Elf32_Shdr const *shdr = shdri;
    int j = n_elf_shnum;
    for (; 0 <=--j; ++shdr) {
        if (0==strcmp(name, &shstrtab[get_te32(&shdr->sh_name)])) {
            return shdr;
        }
    }
    return 0;
}

Elf64_Shdr const *PackLinuxElf64::elf_find_section_name(
    char const *const name
) const
{
    Elf64_Shdr const *shdr = shdri;
    int j = n_elf_shnum;
    for (; 0 <=--j; ++shdr) {
        unsigned ndx = get_te64(&shdr->sh_name);
        if (0==strcmp(name, &shstrtab[ndx])) {
            return shdr;
        }
    }
    return 0;
}

Elf32_Shdr const *PackLinuxElf32::elf_find_section_type(
    unsigned const type
) const
{
    Elf32_Shdr const *shdr = shdri;
    int j = n_elf_shnum;
    for (; 0 <=--j; ++shdr) {
        if (type==get_te32(&shdr->sh_type)) {
            return shdr;
        }
    }
    return 0;
}

Elf64_Shdr const *PackLinuxElf64::elf_find_section_type(
    unsigned const type
) const
{
    Elf64_Shdr const *shdr = shdri;
    int j = n_elf_shnum;
    for (; 0 <=--j; ++shdr) {
        if (type==get_te32(&shdr->sh_type)) {
            return shdr;
        }
    }
    return 0;
}

bool PackLinuxElf32::canPack()
{
    union {
        unsigned char buf[sizeof(Elf32_Ehdr) + 14*sizeof(Elf32_Phdr)];
        //struct { Elf32_Ehdr ehdr; Elf32_Phdr phdr; } e;
    } u;
    COMPILE_TIME_ASSERT(sizeof(u.buf) <= 512)

    fi->seek(0, SEEK_SET);
    fi->readx(u.buf, sizeof(u.buf));
    fi->seek(0, SEEK_SET);
    Elf32_Ehdr const *const ehdr = (Elf32_Ehdr *) u.buf;

    // now check the ELF header
    if (checkEhdr(ehdr) != 0)
        return false;

    // additional requirements for linux/elf386
    if (get_te16(&ehdr->e_ehsize) != sizeof(*ehdr)) {
        throwCantPack("invalid Ehdr e_ehsize; try '--force-execve'");
        return false;
    }
    unsigned const e_shoff = get_te32(&ehdr->e_shoff);
    unsigned const e_phoff = get_te32(&ehdr->e_phoff);
    if (e_phoff != sizeof(*ehdr)) {// Phdrs not contiguous with Ehdr
        throwCantPack("non-contiguous Ehdr/Phdr; try '--force-execve'");
        return false;
    }

    unsigned char osabi0 = u.buf[Elf32_Ehdr::EI_OSABI];
    // The first PT_LOAD32 must cover the beginning of the file (0==p_offset).
    Elf32_Phdr const *phdr = (Elf32_Phdr const *)(u.buf + e_phoff);
    note_size = 0;
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        if (j >= 14) {
            throwCantPack("too many Elf32_Phdr; try '--force-execve'");
            return false;
        }
        unsigned const p_type = get_te32(&phdr->p_type);
        unsigned const p_offset = get_te32(&phdr->p_offset);
        if (1!=exetype && phdr->PT_LOAD32 == p_type) {
            if (p_offset != 0) {
                throwCantPack("invalid Phdr p_offset; try '--force-execve'");
                return false;
            }
            load_va = get_te32(&phdr->p_vaddr);
            exetype = 1;
        }
        if (phdr->PT_NOTE == p_type) {
            unsigned const x = get_te32(&phdr->p_memsz);
            if ( sizeof(elfout.notes) < x  // beware overflow of note_size
            ||  (sizeof(elfout.notes) < (note_size += x)) ) {
                throwCantPack("PT_NOTEs too big; try '--force-execve'");
                return false;
            }
        }
        if (Elf32_Ehdr::ELFOSABI_NONE==osabi0  // Still seems to be generic.
        && NULL!=osabi_note && phdr->PT_NOTE == p_type) {
            struct {
                struct Elf32_Nhdr nhdr;
                char name[8];
                unsigned body;
            } note;
            memset(&note, 0, sizeof(note));
            fi->seek(p_offset, SEEK_SET);
            fi->readx(&note, sizeof(note));
            fi->seek(0, SEEK_SET);
            if (4==get_te32(&note.nhdr.descsz)
            &&  1==get_te32(&note.nhdr.type)
            // &&  0==note.end
            &&  (1+ strlen(osabi_note))==get_te32(&note.nhdr.namesz)
            &&  0==strcmp(osabi_note, (char const *)&note.name[0])
            ) {
                osabi0 = ei_osabi;  // Specified by PT_NOTE.
            }
        }
    }
    if (Elf32_Ehdr::ELFOSABI_NONE==osabi0) { // No EI_OSBAI, no PT_NOTE.
        unsigned const arm_eabi = 0xff000000u & get_te32(&ehdr->e_flags);
        if (Elf32_Ehdr::EM_ARM==e_machine
        &&   (EF_ARM_EABI_VER5==arm_eabi
          ||  EF_ARM_EABI_VER4==arm_eabi ) ) {
            // armel-eabi armeb-eabi ARM Linux EABI version 4 is a mess.
            ei_osabi = osabi0 = Elf32_Ehdr::ELFOSABI_LINUX;
        }
        else {
            osabi0 = opt->o_unix.osabi0;  // Possibly specified by command-line.
        }
    }
    if (osabi0!=ei_osabi) {
        return false;
    }

    // We want to compress position-independent executable (gcc -pie)
    // main programs, but compressing a shared library must be avoided
    // because the result is no longer usable.  In theory, there is no way
    // to tell them apart: both are just ET_DYN.  Also in theory,
    // neither the presence nor the absence of any particular symbol name
    // can be used to tell them apart; there are counterexamples.
    // However, we will use the following heuristic suggested by
    // Peter S. Mazinger <ps.m@gmx.net> September 2005:
    // If a ET_DYN has __libc_start_main as a global undefined symbol,
    // then the file is a position-independent executable main program
    // (that depends on libc.so.6) and is eligible to be compressed.
    // Otherwise (no __libc_start_main as global undefined): skip it.
    // Also allow  __uClibc_main  and  __uClibc_start_main .

    if (Elf32_Ehdr::ET_DYN==get_te16(&ehdr->e_type)) {
        // The DT_STRTAB has no designated length.  Read the whole file.
        file_image = new char[file_size];
        fi->seek(0, SEEK_SET);
        fi->readx(file_image, file_size);
        memcpy(&ehdri, ehdr, sizeof(Elf32_Ehdr));
        phdri= (Elf32_Phdr       *)(e_phoff + file_image);  // do not free() !!
        shdri= (Elf32_Shdr const *)(e_shoff + file_image);  // do not free() !!

        n_elf_shnum = get_te16(&ehdr->e_shnum);
        //sec_strndx = &shdri[ehdr->e_shstrndx];
        //shstrtab = (char const *)(sec_strndx->sh_offset + file_image);
        sec_dynsym = elf_find_section_type(Elf32_Shdr::SHT_DYNSYM);
        if (sec_dynsym)
            sec_dynstr = get_te32(&sec_dynsym->sh_link) + shdri;

        int j= e_phnum;
        phdr= phdri;
        for (; --j>=0; ++phdr)
        if (Elf32_Phdr::PT_DYNAMIC==get_te32(&phdr->p_type)) {
            dynseg= (Elf32_Dyn const *)(get_te32(&phdr->p_offset) + file_image);
            break;
        }
        // elf_find_dynamic() returns 0 if 0==dynseg.
        dynstr=          (char const *)elf_find_dynamic(Elf32_Dyn::DT_STRTAB);
        dynsym=     (Elf32_Sym const *)elf_find_dynamic(Elf32_Dyn::DT_SYMTAB);

        // Modified 2009-10-10 to detect a ProgramLinkageTable relocation
        // which references the symbol, because DT_GNU_HASH contains only
        // defined symbols, and there might be no DT_HASH.

        Elf32_Rel const *
        jmprel=     (Elf32_Rel const *)elf_find_dynamic(Elf32_Dyn::DT_JMPREL);
        for (   int sz = elf_unsigned_dynamic(Elf32_Dyn::DT_PLTRELSZ);
                0 < sz;
                (sz -= sizeof(Elf32_Rel)), ++jmprel
        ) {
            unsigned const symnum = get_te32(&jmprel->r_info) >> 8;
            char const *const symnam = get_te32(&dynsym[symnum].st_name) + dynstr;
            if (0==strcmp(symnam, "__libc_start_main")
            ||  0==strcmp(symnam, "__uClibc_main")
            ||  0==strcmp(symnam, "__uClibc_start_main"))
                goto proceed;
        }

        // Heuristic HACK for shared libraries (compare Darwin (MacOS) Dylib.)
        // If there is an existing DT_INIT, and if everything that the dynamic
        // linker ld-linux needs to perform relocations before calling DT_INIT
        // resides below the first SHT_EXECINSTR Section in one PT_LOAD, then
        // compress from the first executable Section to the end of that PT_LOAD.
        // We must not alter anything that ld-linux might touch before it calls
        // the DT_INIT function.
        //
        // Obviously this hack requires that the linker script put pieces
        // into good positions when building the original shared library,
        // and also requires ld-linux to behave.

        // Apparently glibc-2.13.90 insists on 0==e_ident[EI_PAD..15],
        // so compressing shared libraries may be doomed anyway.
        // 2011-06-01: stub.shlib-init.S works around by installing hatch
        // at end of .text.

        if (elf_find_dynamic(Elf32_Dyn::DT_INIT)) {
            if (this->e_machine!=Elf32_Ehdr::EM_386
            &&  this->e_machine!=Elf32_Ehdr::EM_ARM)
                goto abandon;  // need stub: EM_MIPS EM_PPC
            if (elf_has_dynamic(Elf32_Dyn::DT_TEXTREL)) {
                shdri = NULL;
                phdri = NULL;
                throwCantPack("DT_TEXTREL found; re-compile with -fPIC");
                goto abandon;
            }
            Elf32_Shdr const *shdr = shdri;
            xct_va = ~0u;
            for (j= n_elf_shnum; --j>=0; ++shdr) {
                if (Elf32_Shdr::SHF_EXECINSTR & get_te32(&shdr->sh_flags)) {
                    xct_va = umin(xct_va, get_te32(&shdr->sh_addr));
                }
            }
            // Rely on 0==elf_unsigned_dynamic(tag) if no such tag.
            unsigned const va_gash = elf_unsigned_dynamic(Elf32_Dyn::DT_GNU_HASH);
            unsigned const va_hash = elf_unsigned_dynamic(Elf32_Dyn::DT_HASH);
            if (xct_va < va_gash  ||  (0==va_gash && xct_va < va_hash)
            ||  xct_va < elf_unsigned_dynamic(Elf32_Dyn::DT_STRTAB)
            ||  xct_va < elf_unsigned_dynamic(Elf32_Dyn::DT_SYMTAB)
            ||  xct_va < elf_unsigned_dynamic(Elf32_Dyn::DT_REL)
            ||  xct_va < elf_unsigned_dynamic(Elf32_Dyn::DT_RELA)
            ||  xct_va < elf_unsigned_dynamic(Elf32_Dyn::DT_JMPREL)
            ||  xct_va < elf_unsigned_dynamic(Elf32_Dyn::DT_VERDEF)
            ||  xct_va < elf_unsigned_dynamic(Elf32_Dyn::DT_VERSYM)
            ||  xct_va < elf_unsigned_dynamic(Elf32_Dyn::DT_VERNEEDED) ) {
                phdri = NULL;
                shdri = NULL;
                throwCantPack("DT_ tag above stub");
                goto abandon;
            }
            for ((shdr= shdri), (j= n_elf_shnum); --j>=0; ++shdr) {
                unsigned const sh_addr = get_te32(&shdr->sh_addr);
                if ( sh_addr==va_gash
                ||  (sh_addr==va_hash && 0==va_gash) ) {
                    shdr= &shdri[get_te32(&shdr->sh_link)];  // the associated SHT_SYMTAB
                    hatch_off = (char *)&ehdri.e_ident[12] - (char *)&ehdri;
                    break;
                }
            }
            xct_off = elf_get_offset_from_address(xct_va);
            goto proceed;  // But proper packing depends on checking xct_va.
        }
        else
            infoWarning("no DT_INIT: %s", fi->getName());
abandon:
        phdri = NULL;  // Done with this
        shdri = NULL;
        return false;
proceed:
        phdri = NULL;
        shdri = NULL;
    }
    // XXX Theoretically the following test should be first,
    // but PackUnix::canPack() wants 0!=exetype ?
    if (!super::canPack())
        return false;
    assert(exetype == 1);

    exetype = 0;
    // set options
    opt->o_unix.blocksize = blocksize = file_size;
    return true;
}

bool
PackLinuxElf64amd::canPack()
{
    union {
        unsigned char buf[sizeof(Elf64_Ehdr) + 14*sizeof(Elf64_Phdr)];
        //struct { Elf64_Ehdr ehdr; Elf64_Phdr phdr; } e;
    } u;
    COMPILE_TIME_ASSERT(sizeof(u) <= 1024)

    fi->readx(u.buf, sizeof(u.buf));
    fi->seek(0, SEEK_SET);
    Elf64_Ehdr const *const ehdr = (Elf64_Ehdr *) u.buf;

    // now check the ELF header
    if (checkEhdr(ehdr) != 0)
        return false;

    // additional requirements for linux/elf386
    if (get_te16(&ehdr->e_ehsize) != sizeof(*ehdr)) {
        throwCantPack("invalid Ehdr e_ehsize; try '--force-execve'");
        return false;
    }
    upx_uint64_t const e_shoff = get_te64(&ehdr->e_shoff);
    upx_uint64_t const e_phoff = get_te64(&ehdr->e_phoff);
    if (e_phoff != sizeof(*ehdr)) {// Phdrs not contiguous with Ehdr
        throwCantPack("non-contiguous Ehdr/Phdr; try '--force-execve'");
        return false;
    }

    // The first PT_LOAD64 must cover the beginning of the file (0==p_offset).
    Elf64_Phdr const *phdr = (Elf64_Phdr const *)(u.buf + (unsigned) e_phoff);
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        if (j >= 14)
            return false;
        if (phdr->PT_LOAD64 == get_te32(&phdr->p_type)) {
            // Just avoid the "rewind" when unpacking?
            //if (phdr->p_offset != 0) {
            //    throwCantPack("invalid Phdr p_offset; try '--force-execve'");
            //    return false;
            //}
            load_va = get_te64(&phdr->p_vaddr);
            exetype = 1;
            break;
        }
    }
    // We want to compress position-independent executable (gcc -pie)
    // main programs, but compressing a shared library must be avoided
    // because the result is no longer usable.  In theory, there is no way
    // to tell them apart: both are just ET_DYN.  Also in theory,
    // neither the presence nor the absence of any particular symbol name
    // can be used to tell them apart; there are counterexamples.
    // However, we will use the following heuristic suggested by
    // Peter S. Mazinger <ps.m@gmx.net> September 2005:
    // If a ET_DYN has __libc_start_main as a global undefined symbol,
    // then the file is a position-independent executable main program
    // (that depends on libc.so.6) and is eligible to be compressed.
    // Otherwise (no __libc_start_main as global undefined): skip it.
    // Also allow  __uClibc_main  and  __uClibc_start_main .

    if (Elf32_Ehdr::ET_DYN==get_te16(&ehdr->e_type)) {
        // The DT_STRTAB has no designated length.  Read the whole file.
        file_image = new char[file_size];
        fi->seek(0, SEEK_SET);
        fi->readx(file_image, file_size);
        memcpy(&ehdri, ehdr, sizeof(Elf32_Ehdr));
        phdri= (Elf64_Phdr       *)((size_t)e_phoff + file_image);  // do not free() !!
        shdri= (Elf64_Shdr const *)((size_t)e_shoff + file_image);  // do not free() !!

        n_elf_shnum = get_te16(&ehdr->e_shnum);
        //sec_strndx = &shdri[ehdr->e_shstrndx];
        //shstrtab = (char const *)(sec_strndx->sh_offset + file_image);
        sec_dynsym = elf_find_section_type(Elf64_Shdr::SHT_DYNSYM);
        if (sec_dynsym)
            sec_dynstr = get_te32(&sec_dynsym->sh_link) + shdri;

        int j= e_phnum;
        phdr= phdri;
        for (; --j>=0; ++phdr)
        if (Elf64_Phdr::PT_DYNAMIC==get_te32(&phdr->p_type)) {
            dynseg= (Elf64_Dyn const *)(get_te32(&phdr->p_offset) + file_image);
            break;
        }
        // elf_find_dynamic() returns 0 if 0==dynseg.
        dynstr=          (char const *)elf_find_dynamic(Elf64_Dyn::DT_STRTAB);
        dynsym=     (Elf64_Sym const *)elf_find_dynamic(Elf64_Dyn::DT_SYMTAB);

        // Modified 2009-10-10 to detect a ProgramLinkageTable relocation
        // which references the symbol, because DT_GNU_HASH contains only
        // defined symbols, and there might be no DT_HASH.

        Elf64_Rela const *
        jmprela= (Elf64_Rela const *)elf_find_dynamic(Elf64_Dyn::DT_JMPREL);
        for (   int sz = elf_unsigned_dynamic(Elf64_Dyn::DT_PLTRELSZ);
                0 < sz;
                (sz -= sizeof(Elf64_Rela)), ++jmprela
        ) {
            unsigned const symnum = get_te64(&jmprela->r_info) >> 32;
            char const *const symnam = get_te32(&dynsym[symnum].st_name) + dynstr;
            if (0==strcmp(symnam, "__libc_start_main")
            ||  0==strcmp(symnam, "__uClibc_main")
            ||  0==strcmp(symnam, "__uClibc_start_main"))
                goto proceed;
        }

        // Heuristic HACK for shared libraries (compare Darwin (MacOS) Dylib.)
        // If there is an existing DT_INIT, and if everything that the dynamic
        // linker ld-linux needs to perform relocations before calling DT_INIT
        // resides below the first SHT_EXECINSTR Section in one PT_LOAD, then
        // compress from the first executable Section to the end of that PT_LOAD.
        // We must not alter anything that ld-linux might touch before it calls
        // the DT_INIT function.
        //
        // Obviously this hack requires that the linker script put pieces
        // into good positions when building the original shared library,
        // and also requires ld-linux to behave.

        if (elf_find_dynamic(Elf64_Dyn::DT_INIT)) {
            if (elf_has_dynamic(Elf64_Dyn::DT_TEXTREL)) {
                shdri = NULL;
                phdri = NULL;
                throwCantPack("DT_TEXTREL found; re-compile with -fPIC");
                goto abandon;
            }
            Elf64_Shdr const *shdr = shdri;
            xct_va = ~0ull;
            for (j= n_elf_shnum; --j>=0; ++shdr) {
                if (Elf64_Shdr::SHF_EXECINSTR & get_te32(&shdr->sh_flags)) {
                    xct_va = umin64(xct_va, get_te64(&shdr->sh_addr));
                }
            }
            // Rely on 0==elf_unsigned_dynamic(tag) if no such tag.
            upx_uint64_t const va_gash = elf_unsigned_dynamic(Elf64_Dyn::DT_GNU_HASH);
            upx_uint64_t const va_hash = elf_unsigned_dynamic(Elf64_Dyn::DT_HASH);
            if (xct_va < va_gash  ||  (0==va_gash && xct_va < va_hash)
            ||  xct_va < elf_unsigned_dynamic(Elf64_Dyn::DT_STRTAB)
            ||  xct_va < elf_unsigned_dynamic(Elf64_Dyn::DT_SYMTAB)
            ||  xct_va < elf_unsigned_dynamic(Elf64_Dyn::DT_REL)
            ||  xct_va < elf_unsigned_dynamic(Elf64_Dyn::DT_RELA)
            ||  xct_va < elf_unsigned_dynamic(Elf64_Dyn::DT_JMPREL)
            ||  xct_va < elf_unsigned_dynamic(Elf64_Dyn::DT_VERDEF)
            ||  xct_va < elf_unsigned_dynamic(Elf64_Dyn::DT_VERSYM)
            ||  xct_va < elf_unsigned_dynamic(Elf64_Dyn::DT_VERNEEDED) ) {
                phdri = NULL;
                shdri = NULL;
                throwCantPack("DT_ tag above stub");
                goto abandon;
            }
            for ((shdr= shdri), (j= n_elf_shnum); --j>=0; ++shdr) {
                upx_uint64_t const sh_addr = get_te64(&shdr->sh_addr);
                if ( sh_addr==va_gash
                ||  (sh_addr==va_hash && 0==va_gash) ) {
                    shdr= &shdri[get_te32(&shdr->sh_link)];  // the associated SHT_SYMTAB
                    hatch_off = (char *)&ehdri.e_ident[11] - (char *)&ehdri;
                    break;
                }
            }
            xct_off = elf_get_offset_from_address(xct_va);
            goto proceed;  // But proper packing depends on checking xct_va.
        }
abandon:
        phdri = 0;  // Done with this
        return false;
proceed:
        phdri = 0;
    }
    // XXX Theoretically the following test should be first,
    // but PackUnix::canPack() wants 0!=exetype ?
    if (!super::canPack())
        return false;
    assert(exetype == 1);

    exetype = 0;

    // set options
    opt->o_unix.blocksize = blocksize = file_size;
    return true;
}

off_t
PackLinuxElf32::getbrk(const Elf32_Phdr *phdr, int nph) const
{
    off_t brka = 0;
    for (int j = 0; j < nph; ++phdr, ++j) {
        if (PT_LOAD32 == get_te32(&phdr->p_type)) {
            off_t b = get_te32(&phdr->p_vaddr) + get_te32(&phdr->p_memsz);
            if (b > brka)
                brka = b;
        }
    }
    return brka;
}

off_t
PackLinuxElf32::getbase(const Elf32_Phdr *phdr, int nph) const
{
    off_t base = ~0u;
    for (int j = 0; j < nph; ++phdr, ++j) {
        if (phdr->PT_LOAD == get_te32(&phdr->p_type)) {
            unsigned const vaddr = get_te32(&phdr->p_vaddr);
            if (vaddr < (unsigned) base)
                base = vaddr;
        }
    }
    if (0!=base) {
        return base;
    }
    return 0x12000;
}

off_t
PackLinuxElf64::getbrk(const Elf64_Phdr *phdr, int nph) const
{
    off_t brka = 0;
    for (int j = 0; j < nph; ++phdr, ++j) {
        if (PT_LOAD64 == get_te32(&phdr->p_type)) {
            off_t b = get_te64(&phdr->p_vaddr) + get_te64(&phdr->p_memsz);
            if (b > brka)
                brka = b;
        }
    }
    return brka;
}

void
PackLinuxElf32::generateElfHdr(
    OutputFile *fo,
    void const *proto,
    unsigned const brka
)
{
    cprElfHdr2 *const h2 = (cprElfHdr2 *)(void *)&elfout;
    cprElfHdr3 *const h3 = (cprElfHdr3 *)(void *)&elfout;
    memcpy(h3, proto, sizeof(*h3));  // reads beyond, but OK
    h3->ehdr.e_type = ehdri.e_type;  // ET_EXEC vs ET_DYN (gcc -pie -fPIC)
    h3->ehdr.e_ident[Elf32_Ehdr::EI_OSABI] = ei_osabi;
    if (Elf32_Ehdr::EM_MIPS==e_machine) { // MIPS R3000  FIXME
        h3->ehdr.e_ident[Elf32_Ehdr::EI_OSABI] = Elf32_Ehdr::ELFOSABI_NONE;
        h3->ehdr.e_flags = ehdri.e_flags;
    }

    assert(get_te32(&h2->ehdr.e_phoff)     == sizeof(Elf32_Ehdr));
                         h2->ehdr.e_shoff = 0;
    assert(get_te16(&h2->ehdr.e_ehsize)    == sizeof(Elf32_Ehdr));
    assert(get_te16(&h2->ehdr.e_phentsize) == sizeof(Elf32_Phdr));
           set_te16(&h2->ehdr.e_shentsize, sizeof(Elf32_Shdr));
    if (o_elf_shnum) {
                        h2->ehdr.e_shnum = o_elf_shnum;
                        h2->ehdr.e_shstrndx = o_elf_shnum - 1;
    }
    else {
                        h2->ehdr.e_shnum = 0;
                        h2->ehdr.e_shstrndx = 0;
    }

    sz_elf_hdrs = sizeof(*h2) - sizeof(linfo);  // default
    set_te32(&h2->phdr[0].p_filesz, sizeof(*h2));  // + identsize;
              h2->phdr[0].p_memsz = h2->phdr[0].p_filesz;

    for (unsigned j=0; j < 3; ++j) {
        set_te32(&h3->phdr[j].p_align, page_size);
    }

    // Info for OS kernel to set the brk()
    if (brka) {
        // linux-2.6.14 binfmt_elf.c: SIGKILL if (0==.p_memsz) on a page boundary
        unsigned const brkb = brka | ((0==(~page_mask & brka)) ? 0x20 : 0);
        set_te32(&h2->phdr[1].p_type, PT_LOAD32);  // be sure
        set_te32(&h2->phdr[1].p_offset, ~page_mask & brkb);
        set_te32(&h2->phdr[1].p_vaddr, brkb);
        set_te32(&h2->phdr[1].p_paddr, brkb);
        h2->phdr[1].p_filesz = 0;
        h2->phdr[1].p_memsz =  0;
        if (ARM_is_QNX())
            set_te32(&h2->phdr[1].p_memsz, 1);  // 0==.p_memsz invalid on QNX 6.3.0
        set_te32(&h2->phdr[1].p_flags, Elf32_Phdr::PF_R | Elf32_Phdr::PF_W);
    }
    if (ph.format==getFormat()) {
        assert(2==get_te16(&h2->ehdr.e_phnum));
        set_te32(&h2->phdr[0].p_flags, ~Elf32_Phdr::PF_W & get_te32(&h2->phdr[0].p_flags));
        memset(&h2->linfo, 0, sizeof(h2->linfo));
        fo->write(h2, sizeof(*h2));
    }
    else {
        assert(false);  // unknown ph.format, PackLinuxElf32
    }
}

void
PackNetBSDElf32x86::generateElfHdr(
    OutputFile *fo,
    void const *proto,
    unsigned const brka
)
{
    super::generateElfHdr(fo, proto, brka);
    cprElfHdr2 *const h2 = (cprElfHdr2 *)(void *)&elfout;

    sz_elf_hdrs = sizeof(*h2) - sizeof(linfo);
    unsigned note_offset = sz_elf_hdrs;

    // Find the NetBSD PT_NOTE and the PaX PT_NOTE.
    Elf32_Nhdr const *np_NetBSD = 0;  unsigned sz_NetBSD = 0;
    Elf32_Nhdr const *np_PaX    = 0;  unsigned sz_PaX    = 0;
    unsigned char *cp = note_body;
    unsigned j;
    for (j=0; j < note_size; ) {
        Elf32_Nhdr const *const np = (Elf32_Nhdr const *)(void *)cp;
        int k = sizeof(*np) + up4(get_te32(&np->namesz))
            + up4(get_te32(&np->descsz));

        if (NHDR_NETBSD_TAG == np->type && 7== np->namesz
        &&  NETBSD_DESCSZ == np->descsz
        &&  0==strcmp(ELF_NOTE_NETBSD_NAME,
                /* &np->body */ (char const *)(1+ np))) {
            np_NetBSD = np;
            sz_NetBSD = k;
        }
        if (NHDR_PAX_TAG == np->type && 4== np->namesz
        &&  PAX_DESCSZ==np->descsz
        &&  0==strcmp(ELF_NOTE_PAX_NAME,
                /* &np->body */ (char const *)(1+ np))) {
            np_PaX = np;
            sz_PaX = k;
        }
        cp += k;
        j += k;
    }

    // Add PT_NOTE for the NetBSD note and PaX note, if any.
    note_offset += (np_NetBSD ? sizeof(Elf32_Phdr) : 0);
    note_offset += (np_PaX    ? sizeof(Elf32_Phdr) : 0);
    Elf32_Phdr *phdr = &elfout.phdr[2];
    if (np_NetBSD) {
        set_te32(&phdr->p_type, Elf32_Phdr::PT_NOTE);
        set_te32(&phdr->p_offset, note_offset);
        set_te32(&phdr->p_vaddr, note_offset);
        set_te32(&phdr->p_paddr, note_offset);
        set_te32(&phdr->p_filesz, sz_NetBSD);
        set_te32(&phdr->p_memsz,  sz_NetBSD);
        set_te32(&phdr->p_flags, Elf32_Phdr::PF_R);
        set_te32(&phdr->p_align, 4);

        sz_elf_hdrs += sz_NetBSD + sizeof(*phdr);
        note_offset += sz_NetBSD;
        ++phdr;
    }
    if (np_PaX) {
        set_te32(&phdr->p_type, Elf32_Phdr::PT_NOTE);
        set_te32(&phdr->p_offset, note_offset);
        set_te32(&phdr->p_vaddr, note_offset);
        set_te32(&phdr->p_paddr, note_offset);
        set_te32(&phdr->p_filesz, sz_PaX);
        set_te32(&phdr->p_memsz,  sz_PaX);
        set_te32(&phdr->p_flags, Elf32_Phdr::PF_R);
        set_te32(&phdr->p_align, 4);

        unsigned bits = get_te32(  /* &np_PaX->body[4] */
            &(ACC_UNCONST_CAST(unsigned char *, (1+ np_PaX)))[4] );
        bits &= ~PAX_MPROTECT;
        bits |=  PAX_NOMPROTECT;
        set_te32(  /* &np_PaX->body[4] */
            &(ACC_UNCONST_CAST(unsigned char *, (1+ np_PaX)))[4],  bits);

        sz_elf_hdrs += sz_PaX + sizeof(*phdr);
        note_offset += sz_PaX;
        ++phdr;
    }
    set_te32(&h2->phdr[0].p_filesz, note_offset);
              h2->phdr[0].p_memsz = h2->phdr[0].p_filesz;

    if (ph.format==getFormat()) {
        set_te16(&h2->ehdr.e_phnum, !!sz_NetBSD + !!sz_PaX +
        get_te16(&h2->ehdr.e_phnum));
        fo->seek(0, SEEK_SET);
        fo->rewrite(h2, sizeof(*h2) - sizeof(h2->linfo));

        memcpy(&((char *)phdr)[0],         np_NetBSD, sz_NetBSD);
        memcpy(&((char *)phdr)[sz_NetBSD], np_PaX,    sz_PaX);

        fo->write(&elfout.phdr[2],
            &((char *)phdr)[sz_PaX + sz_NetBSD] - (char *)&elfout.phdr[2]);

        l_info foo; memset(&foo, 0, sizeof(foo));
        fo->rewrite(&foo, sizeof(foo));
    }
    else {
        assert(false);  // unknown ph.format, PackLinuxElf32
    }
}

void
PackOpenBSDElf32x86::generateElfHdr(
    OutputFile *fo,
    void const *proto,
    unsigned const brka
)
{
    cprElfHdr3 *const h3 = (cprElfHdr3 *)(void *)&elfout;
    memcpy(h3, proto, sizeof(*h3));  // reads beyond, but OK
    h3->ehdr.e_ident[Elf32_Ehdr::EI_OSABI] = ei_osabi;
    assert(2==get_te16(&h3->ehdr.e_phnum));
    set_te16(&h3->ehdr.e_phnum, 3);

    assert(get_te32(&h3->ehdr.e_phoff)     == sizeof(Elf32_Ehdr));
                         h3->ehdr.e_shoff = 0;
    assert(get_te16(&h3->ehdr.e_ehsize)    == sizeof(Elf32_Ehdr));
    assert(get_te16(&h3->ehdr.e_phentsize) == sizeof(Elf32_Phdr));
           set_te16(&h3->ehdr.e_shentsize, sizeof(Elf32_Shdr));
                         h3->ehdr.e_shnum = 0;
                         h3->ehdr.e_shstrndx = 0;

    struct {
        Elf32_Nhdr nhdr;
        char name[8];
        unsigned body;
    } elfnote;

    unsigned const note_offset = sizeof(*h3) - sizeof(linfo);
    sz_elf_hdrs = sizeof(elfnote) + note_offset;

    set_te32(&h3->phdr[2].p_type, Elf32_Phdr::PT_NOTE);
    set_te32(&h3->phdr[2].p_offset, note_offset);
    set_te32(&h3->phdr[2].p_vaddr, note_offset);
    set_te32(&h3->phdr[2].p_paddr, note_offset);
    set_te32(&h3->phdr[2].p_filesz, sizeof(elfnote));
    set_te32(&h3->phdr[2].p_memsz,  sizeof(elfnote));
    set_te32(&h3->phdr[2].p_flags, Elf32_Phdr::PF_R);
    set_te32(&h3->phdr[2].p_align, 4);

    // Q: Same as this->note_body[0 .. this->note_size-1] ?
    set_te32(&elfnote.nhdr.namesz, 8);
    set_te32(&elfnote.nhdr.descsz, OPENBSD_DESCSZ);
    set_te32(&elfnote.nhdr.type,   NHDR_OPENBSD_TAG);
    memcpy(elfnote.name, "OpenBSD", sizeof(elfnote.name));
    elfnote.body = 0;

    set_te32(&h3->phdr[0].p_filesz, sz_elf_hdrs);
              h3->phdr[0].p_memsz = h3->phdr[0].p_filesz;

    unsigned const brkb = brka | ((0==(~page_mask & brka)) ? 0x20 : 0);
    set_te32(&h3->phdr[1].p_type, PT_LOAD32);  // be sure
    set_te32(&h3->phdr[1].p_offset, ~page_mask & brkb);
    set_te32(&h3->phdr[1].p_vaddr, brkb);
    set_te32(&h3->phdr[1].p_paddr, brkb);
    h3->phdr[1].p_filesz = 0;
    h3->phdr[1].p_memsz =  0;
    set_te32(&h3->phdr[1].p_flags, Elf32_Phdr::PF_R | Elf32_Phdr::PF_W);

    if (ph.format==getFormat()) {
        memset(&h3->linfo, 0, sizeof(h3->linfo));
        fo->write(h3, sizeof(*h3) - sizeof(h3->linfo));
        fo->write(&elfnote, sizeof(elfnote));
        fo->write(&h3->linfo, sizeof(h3->linfo));
    }
    else {
        assert(false);  // unknown ph.format, PackLinuxElf32
    }
}

void
PackLinuxElf64::generateElfHdr(
    OutputFile *fo,
    void const *proto,
    unsigned const brka
)
{
    cprElfHdr2 *const h2 = (cprElfHdr2 *)(void *)&elfout;
    cprElfHdr3 *const h3 = (cprElfHdr3 *)(void *)&elfout;
    memcpy(h3, proto, sizeof(*h3));  // reads beyond, but OK
    h3->ehdr.e_type = ehdri.e_type;  // ET_EXEC vs ET_DYN (gcc -pie -fPIC)
    h3->ehdr.e_ident[Elf32_Ehdr::EI_OSABI] = ei_osabi;

    assert(get_te32(&h2->ehdr.e_phoff)     == sizeof(Elf64_Ehdr));
                         h2->ehdr.e_shoff = 0;
    assert(get_te16(&h2->ehdr.e_ehsize)    == sizeof(Elf64_Ehdr));
    assert(get_te16(&h2->ehdr.e_phentsize) == sizeof(Elf64_Phdr));
           set_te16(&h2->ehdr.e_shentsize, sizeof(Elf64_Shdr));
    if (o_elf_shnum) {
                        h2->ehdr.e_shnum = o_elf_shnum;
                        h2->ehdr.e_shstrndx = o_elf_shnum - 1;
    }
    else {
                        h2->ehdr.e_shnum = 0;
                        h2->ehdr.e_shstrndx = 0;
    }

    sz_elf_hdrs = sizeof(*h2) - sizeof(linfo);  // default
    set_te64(&h2->phdr[0].p_filesz, sizeof(*h2));  // + identsize;
                  h2->phdr[0].p_memsz = h2->phdr[0].p_filesz;

    for (unsigned j=0; j < 3; ++j) {
        set_te64(&h3->phdr[j].p_align, page_size);
    }

    // Info for OS kernel to set the brk()
    if (brka) {
        // linux-2.6.14 binfmt_elf.c: SIGKILL if (0==.p_memsz) on a page boundary
        unsigned const brkb = brka | ((0==(~page_mask & brka)) ? 0x20 : 0);
        set_te32(&h2->phdr[1].p_type, PT_LOAD64);  // be sure
        set_te64(&h2->phdr[1].p_offset, ~page_mask & brkb);
        set_te64(&h2->phdr[1].p_vaddr, brkb);
        set_te64(&h2->phdr[1].p_paddr, brkb);
        h2->phdr[1].p_filesz = 0;
        h2->phdr[1].p_memsz =  0;
        set_te32(&h2->phdr[1].p_flags, Elf64_Phdr::PF_R | Elf64_Phdr::PF_W);
    }
    if (ph.format==getFormat()) {
        assert(2==get_te16(&h2->ehdr.e_phnum));
        set_te32(&h2->phdr[0].p_flags, ~Elf64_Phdr::PF_W & get_te32(&h2->phdr[0].p_flags));
        memset(&h2->linfo, 0, sizeof(h2->linfo));
        fo->write(h2, sizeof(*h2));
    }
    else {
        assert(false);  // unknown ph.format, PackLinuxElf64
    }
}

void PackLinuxElf32::pack1(OutputFile *fo, Filter & /*ft*/)
{
    fi->seek(0, SEEK_SET);
    fi->readx(&ehdri, sizeof(ehdri));
    unsigned const e_phoff = get_te32(&ehdri.e_phoff);
    assert(e_phoff == sizeof(Elf32_Ehdr));  // checked by canPack()
    sz_phdrs = e_phnum * get_te16(&ehdri.e_phentsize);

    phdri = new Elf32_Phdr[e_phnum];
    fi->seek(e_phoff, SEEK_SET);
    fi->readx(phdri, sz_phdrs);

    // Remember all PT_NOTE, and find lg2_page from PT_LOAD.
    Elf32_Phdr const *phdr = phdri;
    note_size = 0;
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        if (phdr->PT_NOTE32 == get_te32(&phdr->p_type)) {
            note_size += up4(get_te32(&phdr->p_filesz));
        }
    }
    if (note_size) {
        note_body = new unsigned char[note_size];
        note_size = 0;
    }
    phdr = phdri;
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        unsigned const type = get_te32(&phdr->p_type);
        if (phdr->PT_NOTE32 == type) {
            unsigned const len = get_te32(&phdr->p_filesz);
            fi->seek(get_te32(&phdr->p_offset), SEEK_SET);
            fi->readx(&note_body[note_size], len);
            note_size += up4(len);
        }
        if (phdr->PT_LOAD32 == type) {
            unsigned x = get_te32(&phdr->p_align) >> lg2_page;
            while (x>>=1) {
                ++lg2_page;
            }
        }
    }
    page_size =  1u<<lg2_page;
    page_mask = ~0u<<lg2_page;

    progid = 0;  // getRandomId();  not useful, so do not clutter
    if (0!=xct_off) {  // shared library
        fi->seek(0, SEEK_SET);
        fi->readx(ibuf, xct_off);

        sz_elf_hdrs = xct_off;
        fo->write(ibuf, xct_off);
        memset(&linfo, 0, sizeof(linfo));
        fo->write(&linfo, sizeof(linfo));
    }

    // if the preserve build-id option was specified
    if (opt->o_unix.preserve_build_id) {
        n_elf_shnum = ehdri.e_shnum;
        Elf32_Shdr *shdr = NULL;

        Elf32_Shdr const *tmp = shdri;

        if (! shdri) {
            shdr = new Elf32_Shdr[n_elf_shnum];

            fi->seek(0,SEEK_SET);
            fi->seek(ehdri.e_shoff,SEEK_SET);
            fi->readx((void*)shdr,ehdri.e_shentsize*ehdri.e_shnum);

            // set this so we can use elf_find_section_name
            shdri = (Elf32_Shdr *)shdr;
        }

        //set the shstrtab
        sec_strndx = &shdr[ehdri.e_shstrndx];

        char *strtab = new char[(unsigned) sec_strndx->sh_size];
        fi->seek(0,SEEK_SET);
        fi->seek(sec_strndx->sh_offset,SEEK_SET);
        fi->readx(strtab,sec_strndx->sh_size);

        shstrtab = (const char*)strtab;

        Elf32_Shdr const *buildid = elf_find_section_name(".note.gnu.build-id");
        if (buildid) {
            unsigned char *data = new unsigned char[(unsigned) buildid->sh_size];
            memset(data,0,buildid->sh_size);
            fi->seek(0,SEEK_SET);
            fi->seek(buildid->sh_offset,SEEK_SET);
            fi->readx(data,buildid->sh_size);

            buildid_data  = data;

            o_elf_shnum = 3;
            memset(&shdrout,0,sizeof(shdrout));

            //setup the build-id
            memcpy(&shdrout.shdr[1],buildid, sizeof(shdrout.shdr[1]));
            shdrout.shdr[1].sh_name = 1;

            //setup the shstrtab
            memcpy(&shdrout.shdr[2],sec_strndx, sizeof(shdrout.shdr[2]));
            shdrout.shdr[2].sh_name = 20;
            shdrout.shdr[2].sh_size = 29; //size of our static shstrtab
        }

        // repoint shdr in case it is used by code some where else
        if (shdr) {
            shdri = tmp;
            delete [] shdr;
            shdr = NULL;
        }
    }
}

void PackLinuxElf32x86::pack1(OutputFile *fo, Filter &ft)
{
    super::pack1(fo, ft);
    if (0!=xct_off)  // shared library
        return;
    generateElfHdr(fo, stub_i386_linux_elf_fold, getbrk(phdri, e_phnum) );
}

void PackBSDElf32x86::pack1(OutputFile *fo, Filter &ft)
{
    PackLinuxElf32::pack1(fo, ft);
    if (0!=xct_off) // shared library
        return;
    generateElfHdr(fo, stub_i386_bsd_elf_fold, getbrk(phdri, e_phnum) );
}

void PackLinuxElf32armLe::pack1(OutputFile *fo, Filter &ft)
{
    super::pack1(fo, ft);
    if (0!=xct_off)  // shared library
        return;
    unsigned const e_flags = get_te32(&ehdri.e_flags);
    cprElfHdr3 h3;
    if (Elf32_Ehdr::ELFOSABI_LINUX==ei_osabi) {
        memcpy(&h3, stub_armel_eabi_linux_elf_fold, sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr));

        h3.ehdr.e_ident[Elf32_Ehdr::EI_ABIVERSION] = e_flags>>24;
    }
    else {
        memcpy(&h3, stub_arm_linux_elf_fold,        sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr));
    }
    // Fighting over .e_ident[EI_ABIVERSION]: Debian armhf is latest culprit.
    // So copy from input to output; but see PackLinuxElf32::generateElfHdr
    memcpy(&h3.ehdr.e_ident[0], &ehdri.e_ident[0], sizeof(ehdri.e_ident));
    set_te32(&h3.ehdr.e_flags, e_flags);
    generateElfHdr(fo, &h3, getbrk(phdri, e_phnum) );
}

void PackLinuxElf32armBe::pack1(OutputFile *fo, Filter &ft)
{
    super::pack1(fo, ft);
    if (0!=xct_off)  // shared library
        return;
    unsigned const e_flags = get_te32(&ehdri.e_flags);
    cprElfHdr3 h3;
    memcpy(&h3, stub_armeb_linux_elf_fold, sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr));
    set_te32(&h3.ehdr.e_flags, e_flags);
    generateElfHdr(fo, &h3, getbrk(phdri, e_phnum) );
}

void PackLinuxElf32mipsel::pack1(OutputFile *fo, Filter &ft)
{
    super::pack1(fo, ft);
    if (0!=xct_off)  // shared library
        return;
    cprElfHdr3 h3;
    memcpy(&h3, stub_mipsel_r3000_linux_elf_fold, sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr));
    generateElfHdr(fo, &h3, getbrk(phdri, e_phnum) );
}

void PackLinuxElf32mipseb::pack1(OutputFile *fo, Filter &ft)
{
    super::pack1(fo, ft);
    if (0!=xct_off)  // shared library
        return;
    cprElfHdr3 h3;
    memcpy(&h3, stub_mips_r3000_linux_elf_fold, sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr));
    generateElfHdr(fo, &h3, getbrk(phdri, e_phnum) );
}

void PackLinuxElf32ppc::pack1(OutputFile *fo, Filter &ft)
{
    super::pack1(fo, ft);
    if (0!=xct_off)  // shared library
        return;
    generateElfHdr(fo, stub_powerpc_linux_elf_fold, getbrk(phdri, e_phnum) );
}

void PackLinuxElf64::pack1(OutputFile *fo, Filter & /*ft*/)
{
    fi->seek(0, SEEK_SET);
    fi->readx(&ehdri, sizeof(ehdri));
    unsigned const e_phoff = get_te32(&ehdri.e_phoff);
    assert(e_phoff == sizeof(Elf64_Ehdr));  // checked by canPack()
    sz_phdrs = e_phnum * get_te16(&ehdri.e_phentsize);

    phdri = new Elf64_Phdr[e_phnum];
    fi->seek(e_phoff, SEEK_SET);
    fi->readx(phdri, sz_phdrs);

    Elf64_Phdr const *phdr = phdri;
    note_size = 0;
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        if (phdr->PT_NOTE64 == get_te32(&phdr->p_type)) {
            note_size += up4(get_te64(&phdr->p_filesz));
        }
    }
    if (note_size) {
        note_body = new unsigned char[note_size];
        note_size = 0;
    }
    phdr = phdri;
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        unsigned const type = get_te32(&phdr->p_type);
        if (phdr->PT_NOTE64 == type) {
            unsigned const len = get_te64(&phdr->p_filesz);
            fi->seek(get_te64(&phdr->p_offset), SEEK_SET);
            fi->readx(&note_body[note_size], len);
            note_size += up4(len);
        }
        if (phdr->PT_LOAD64 == type) {
            unsigned x = get_te64(&phdr->p_align) >> lg2_page;
            while (x>>=1) {
                ++lg2_page;
            }
        }
    }
    page_size =  1u  <<lg2_page;
    page_mask = ~0ull<<lg2_page;

    progid = 0;  // getRandomId();  not useful, so do not clutter
    if (0!=xct_off) {  // shared library
        fi->seek(0, SEEK_SET);
        fi->readx(ibuf, xct_off);

        sz_elf_hdrs = xct_off;
        fo->write(ibuf, xct_off);
        memset(&linfo, 0, sizeof(linfo));
        fo->write(&linfo, sizeof(linfo));
    }

    // only execute if option present
    if (opt->o_unix.preserve_build_id) {
        // set this so we can use elf_find_section_name
        n_elf_shnum = ehdri.e_shnum;

        // there is a class member similar to this, but I did not
        // want to assume it would be available
        Elf64_Shdr const *tmp = shdri;

        Elf64_Shdr *shdr = NULL;

        if (! shdri) {
            shdr = new Elf64_Shdr[n_elf_shnum];

            fi->seek(0,SEEK_SET);
            fi->seek(ehdri.e_shoff,SEEK_SET);
            fi->readx((void*)shdr,ehdri.e_shentsize*ehdri.e_shnum);

            // set this so we can use elf_find_section_name
            shdri = (Elf64_Shdr *)shdr;
        }

        //set the shstrtab
        sec_strndx = &shdri[ehdri.e_shstrndx];

        char *strtab = new char[(unsigned) sec_strndx->sh_size];
        fi->seek(0,SEEK_SET);
        fi->seek(sec_strndx->sh_offset,SEEK_SET);
        fi->readx(strtab,sec_strndx->sh_size);

        shstrtab = (const char*)strtab;

        Elf64_Shdr const *buildid = elf_find_section_name(".note.gnu.build-id");
        if (buildid) {
            unsigned char *data = new unsigned char[(unsigned) buildid->sh_size];
            memset(data,0,buildid->sh_size);
            fi->seek(0,SEEK_SET);
            fi->seek(buildid->sh_offset,SEEK_SET);
            fi->readx(data,buildid->sh_size);

            buildid_data  = data;

            o_elf_shnum = 3;
            memset(&shdrout,0,sizeof(shdrout));

            //setup the build-id
            memcpy(&shdrout.shdr[1],buildid, sizeof(shdrout.shdr[1]));
            shdrout.shdr[1].sh_name = 1;

            //setup the shstrtab
            memcpy(&shdrout.shdr[2],sec_strndx, sizeof(shdrout.shdr[2]));
            shdrout.shdr[2].sh_name = 20;
            shdrout.shdr[2].sh_size = 29; //size of our static shstrtab
        }

        if (shdr) {
            shdri = tmp;
            delete [] shdr;
            shdr = NULL;
        }
    }
}

void PackLinuxElf64amd::pack1(OutputFile *fo, Filter &ft)
{
    super::pack1(fo, ft);
    if (0!=xct_off)  // shared library
        return;
    generateElfHdr(fo, stub_amd64_linux_elf_fold, getbrk(phdri, e_phnum) );
}

// Determine length of gap between PT_LOAD phdr[k] and closest PT_LOAD
// which follows in the file (or end-of-file).  Optimize for common case
// where the PT_LOAD are adjacent ascending by .p_offset.  Assume no overlap.

unsigned PackLinuxElf32::find_LOAD_gap(
    Elf32_Phdr const *const phdr,
    unsigned const k,
    unsigned const nph
)
{
    if (PT_LOAD32!=get_te32(&phdr[k].p_type)) {
        return 0;
    }
    unsigned const hi = get_te32(&phdr[k].p_offset) +
                        get_te32(&phdr[k].p_filesz);
    unsigned lo = ph.u_file_size;
    if (lo < hi)
        throwCantPack("bad input: PT_LOAD beyond end-of-file");
    unsigned j = k;
    for (;;) { // circular search, optimize for adjacent ascending
        ++j;
        if (nph==j) {
            j = 0;
        }
        if (k==j) {
            break;
        }
        if (PT_LOAD32==get_te32(&phdr[j].p_type)) {
            unsigned const t = get_te32(&phdr[j].p_offset);
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

int PackLinuxElf32::pack2(OutputFile *fo, Filter &ft)
{
    Extent x;
    unsigned k;
    bool const is_shlib = (0!=xct_off);

    // count passes, set ptload vars
    uip->ui_total_passes = 0;
    for (k = 0; k < e_phnum; ++k) {
        if (PT_LOAD32==get_te32(&phdri[k].p_type)) {
            uip->ui_total_passes++;
            if (find_LOAD_gap(phdri, k, e_phnum)) {
                uip->ui_total_passes++;
            }
        }
    }
    uip->ui_total_passes -= !!is_shlib;  // not .data of shlib

    // compress extents
    unsigned hdr_u_len = sizeof(Elf32_Ehdr) + sz_phdrs;

    unsigned total_in = xct_off - (is_shlib ? hdr_u_len : 0);
    unsigned total_out = xct_off;

    uip->ui_pass = 0;
    ft.addvalue = 0;

    int nx = 0;
    for (k = 0; k < e_phnum; ++k) if (PT_LOAD32==get_te32(&phdri[k].p_type)) {
        if (ft.id < 0x40) {
            // FIXME: ??    ft.addvalue = phdri[k].p_vaddr;
        }
        x.offset = get_te32(&phdri[k].p_offset);
        x.size   = get_te32(&phdri[k].p_filesz);
        if (0 == nx) { // 1st PT_LOAD32 must cover Ehdr at 0==p_offset
            unsigned const delta = !is_shlib
                ? (sizeof(Elf32_Ehdr) + sz_phdrs)  // main executable
                : xct_off;  // shared library
            if (ft.id < 0x40) {
                // FIXME: ??     ft.addvalue += delta;
            }
            x.offset += delta;
            x.size   -= delta;
        }
        // compressWithFilters() always assumes a "loader", so would
        // throw NotCompressible for small .data Extents, which PowerPC
        // sometimes marks as PF_X anyway.  So filter only first segment.
        if (0==nx || !is_shlib)
        packExtent(x, total_in, total_out,
            ((0==nx && (Elf32_Phdr::PF_X & get_te32(&phdri[k].p_flags)))
                ? &ft : 0 ), fo, hdr_u_len);
        else
            total_in += x.size;
        hdr_u_len = 0;
        ++nx;
    }
    sz_pack2a = fpad4(fo);  // MATCH01

    // Accounting only; ::pack3 will do the compression and output
    for (k = 0; k < e_phnum; ++k) {
        total_in += find_LOAD_gap(phdri, k, e_phnum);
    }

    if ((off_t)total_in != file_size)
        throwEOFException();

    return 0;  // omit end-of-compression bhdr for now
}

// Determine length of gap between PT_LOAD phdr[k] and closest PT_LOAD
// which follows in the file (or end-of-file).  Optimize for common case
// where the PT_LOAD are adjacent ascending by .p_offset.  Assume no overlap.

unsigned PackLinuxElf64::find_LOAD_gap(
    Elf64_Phdr const *const phdr,
    unsigned const k,
    unsigned const nph
)
{
    if (PT_LOAD64!=get_te32(&phdr[k].p_type)) {
        return 0;
    }
    unsigned const hi = get_te64(&phdr[k].p_offset) +
                        get_te64(&phdr[k].p_filesz);
    unsigned lo = ph.u_file_size;
    if (lo < hi)
        throwCantPack("bad input: PT_LOAD beyond end-of-file");
    unsigned j = k;
    for (;;) { // circular search, optimize for adjacent ascending
        ++j;
        if (nph==j) {
            j = 0;
        }
        if (k==j) {
            break;
        }
        if (PT_LOAD64==get_te32(&phdr[j].p_type)) {
            unsigned const t = get_te64(&phdr[j].p_offset);
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

int PackLinuxElf64::pack2(OutputFile *fo, Filter &ft)
{
    Extent x;
    unsigned k;
    bool const is_shlib = (0!=xct_off);

    // count passes, set ptload vars
    uip->ui_total_passes = 0;
    for (k = 0; k < e_phnum; ++k) {
        if (PT_LOAD64==get_te32(&phdri[k].p_type)) {
            uip->ui_total_passes++;
            if (find_LOAD_gap(phdri, k, e_phnum)) {
                uip->ui_total_passes++;
            }
        }
    }
    uip->ui_total_passes -= !!is_shlib;  // not .data of shlib

    // compress extents
    unsigned hdr_u_len = sizeof(Elf64_Ehdr) + sz_phdrs;

    unsigned total_in = xct_off - (is_shlib ? hdr_u_len : 0);
    unsigned total_out = xct_off;

    uip->ui_pass = 0;
    ft.addvalue = 0;

    int nx = 0;
    for (k = 0; k < e_phnum; ++k) if (PT_LOAD64==get_te32(&phdri[k].p_type)) {
        if (ft.id < 0x40) {
            // FIXME: ??    ft.addvalue = phdri[k].p_vaddr;
        }
        x.offset = get_te64(&phdri[k].p_offset);
        x.size   = get_te64(&phdri[k].p_filesz);
        if (0 == nx) { // 1st PT_LOAD64 must cover Ehdr at 0==p_offset
            unsigned const delta = !is_shlib
                ? (sizeof(Elf64_Ehdr) + sz_phdrs)  // main executable
                : xct_off;  // shared library
            if (ft.id < 0x40) {
                // FIXME: ??     ft.addvalue += delta;
            }
            x.offset += delta;
            x.size   -= delta;
        }
        // compressWithFilters() always assumes a "loader", so would
        // throw NotCompressible for small .data Extents, which PowerPC
        // sometimes marks as PF_X anyway.  So filter only first segment.
        if (0==nx || !is_shlib)
        packExtent(x, total_in, total_out,
            ((0==nx && (Elf64_Phdr::PF_X & get_te64(&phdri[k].p_flags)))
                ? &ft : 0 ), fo, hdr_u_len);
        else
            total_in += x.size;
        hdr_u_len = 0;
        ++nx;
    }
    sz_pack2a = fpad4(fo);  // MATCH01

    // Accounting only; ::pack3 will do the compression and output
    for (k = 0; k < e_phnum; ++k) { //
        total_in += find_LOAD_gap(phdri, k, e_phnum);
    }

    if ((off_t)total_in != file_size)
        throwEOFException();

    return 0;  // omit end-of-compression bhdr for now
}

// Filter 0x50, 0x51 assume HostPolicy::isLE
static const int *
ARM_getFilters(bool const isBE)
{
    static const int f50[] = { 0x50, FT_END };
    static const int f51[] = { 0x51, FT_END };
    if (isBE)
        return f51;
    return f50;
}

const int *
PackLinuxElf32armBe::getFilters() const
{
    return ARM_getFilters(true);
}

const int *
PackLinuxElf32armLe::getFilters() const
{
    return ARM_getFilters(false);
}

const int *
PackLinuxElf32mipseb::getFilters() const
{
    static const int f_none[] = { FT_END };
    return f_none;
}

const int *
PackLinuxElf32mipsel::getFilters() const
{
    static const int f_none[] = { FT_END };
    return f_none;
}

// October 2011: QNX 6.3.0 has no unique signature?
int PackLinuxElf32::ARM_is_QNX(void)
{
    if (Elf32_Ehdr::EM_ARM==get_te16(&ehdri.e_machine)
    &&  Elf32_Ehdr::ELFDATA2MSB== ehdri.e_ident[Elf32_Ehdr::EI_DATA]
    &&  Elf32_Ehdr::ELFOSABI_ARM==ehdri.e_ident[Elf32_Ehdr::EI_OSABI]
    &&  0x100000==(page_mask & get_te32(&phdri[0].p_vaddr))) {
        Elf32_Phdr const *phdr = phdri;
        for (int j = get_te16(&ehdri.e_phnum); --j>=0; ++phdr) {
            if (Elf32_Phdr::PT_INTERP==get_te32(&phdr->p_type)) {
                char interp[64];
                unsigned const sz_interp = get_te32(&phdr->p_filesz);
                unsigned const pos_interp = get_te32(&phdr->p_offset);
                if (sz_interp <= sizeof(interp)
                &&  (sz_interp + pos_interp) <= (unsigned) fi->st_size()) {
                    fi->seek(pos_interp, SEEK_SET);
                    fi->readx(interp, sz_interp);
                    for (int k = sz_interp - 5; k>=0; --k) {
                        if (0==memcmp("ldqnx", &interp[k], 5))
                            return 1;
                    }
                }
            }
        }
    }
    return 0;
}

void PackLinuxElf32::ARM_defineSymbols(Filter const * /*ft*/)
{
    lsize = /*getLoaderSize()*/  4 * 1024;  // upper bound; avoid circularity
    unsigned lo_va_user = ~0u;  // infinity
    for (int j= e_phnum; --j>=0; ) {
        if (PT_LOAD32 == get_te32(&phdri[j].p_type)) {
            unsigned const va = get_te32(&phdri[j].p_vaddr);
            if (va < lo_va_user) {
                lo_va_user = va;
            }
        }
    }
    unsigned lo_va_stub = get_te32(&elfout.phdr[0].p_vaddr);
    unsigned adrc;
    unsigned adrm;

    bool const is_big = true;  // kernel disallows mapping below 0x8000.
    if (is_big) {
        set_te32(    &elfout.ehdr.e_entry, linker->getSymbolOffset("_start") +
            get_te32(&elfout.ehdr.e_entry) + lo_va_user - lo_va_stub);
        set_te32(&elfout.phdr[0].p_vaddr, lo_va_user);
        set_te32(&elfout.phdr[0].p_paddr, lo_va_user);
                              lo_va_stub    = lo_va_user;
        adrc = lo_va_stub;
        adrm = getbrk(phdri, e_phnum);
    }
    adrc = page_mask & (~page_mask + adrc);  // round up to page boundary
    adrm = page_mask & (~page_mask + adrm);  // round up to page boundary
    adrm += page_size;  // Try: hole so that kernel does not extend the brk(0)
    linker->defineSymbol("ADRM", adrm);  // addr for map

    linker->defineSymbol("CPR0", 4+ linker->getSymbolOffset("cpr0"));
    linker->defineSymbol("LENF", 4+ linker->getSymbolOffset("end_decompress"));

#define MAP_PRIVATE      2     /* UNIX standard */
#define MAP_FIXED     0x10     /* UNIX standard */
#define MAP_ANONYMOUS 0x20     /* UNIX standard */
#define MAP_PRIVANON     3     /* QNX anonymous private memory */
    unsigned mflg = MAP_PRIVATE | MAP_ANONYMOUS;
    if (ARM_is_QNX())
        mflg = MAP_PRIVANON;
    linker->defineSymbol("MFLG", mflg);
}

void PackLinuxElf32armLe::defineSymbols(Filter const *ft)
{
    ARM_defineSymbols(ft);
}

void PackLinuxElf32armBe::defineSymbols(Filter const *ft)
{
    ARM_defineSymbols(ft);
}

void PackLinuxElf32mipseb::defineSymbols(Filter const * /*ft*/)
{
    unsigned const hlen = sz_elf_hdrs + sizeof(l_info) + sizeof(p_info);

    // We want to know if compressed data, plus stub, plus a couple pages,
    // will fit below the uncompressed program in memory.  But we don't
    // know the final total compressed size yet, so use the uncompressed
    // size (total over all PT_LOAD32) as an upper bound.
    unsigned len = 0;
    unsigned lo_va_user = ~0u;  // infinity
    for (int j= e_phnum; --j>=0; ) {
        if (PT_LOAD32 == get_te32(&phdri[j].p_type)) {
            len += (unsigned)get_te32(&phdri[j].p_filesz);
            unsigned const va = get_te32(&phdri[j].p_vaddr);
            if (va < lo_va_user) {
                lo_va_user = va;
            }
        }
    }
    lsize = /*getLoaderSize()*/  64 * 1024;  // XXX: upper bound; avoid circularity
    unsigned lo_va_stub = get_te32(&elfout.phdr[0].p_vaddr);
    unsigned adrc;
    unsigned adrm;
    unsigned adru;
    unsigned adrx;
    unsigned cntc;
    unsigned lenm;
    unsigned lenu;
    len += (7&-lsize) + lsize;
    bool const is_big = (lo_va_user < (lo_va_stub + len + 2*page_size));
    if (is_big) {
        set_te32(    &elfout.ehdr.e_entry,
            get_te32(&elfout.ehdr.e_entry) + lo_va_user - lo_va_stub);
        set_te32(&elfout.phdr[0].p_vaddr, lo_va_user);
        set_te32(&elfout.phdr[0].p_paddr, lo_va_user);
               lo_va_stub      = lo_va_user;
        adrc = lo_va_stub;
        adrm = getbrk(phdri, e_phnum);
        adru = page_mask & (~page_mask + adrm);  // round up to page boundary
        adrx = adru + hlen;
        lenm = page_size + len;
        lenu = page_size + len;
        cntc = len >> 3;  // over-estimate; corrected at runtime
    }
    else {
        adrm = lo_va_stub + len;
        adrc = adrm;
        adru = lo_va_stub;
        adrx = lo_va_stub + hlen;
        lenm = page_size;
        lenu = page_size + len;
        cntc = 0;
    }
    adrm = page_mask & (~page_mask + adrm);  // round up to page boundary
    adrc = page_mask & (~page_mask + adrc);  // round up to page boundary

    linker->defineSymbol("ADRX", adrx); // compressed input for eXpansion

    // For actual moving, we need the true count, which depends on sz_pack2
    // and is not yet known.  So the runtime stub detects "no move"
    // if adrm==adrc, and otherwise uses actual sz_pack2 to compute cntc.
    //linker->defineSymbol("CNTC", cntc);  // count  for copy
    ACC_UNUSED(cntc);

    linker->defineSymbol("ADRC", adrc);  // addr for copy
    linker->defineSymbol("LENU", lenu);  // len  for unmap
    linker->defineSymbol("ADRU", adru);  // addr for unmap
    linker->defineSymbol("LENM", lenm);  // len  for map
    linker->defineSymbol("ADRM", adrm);  // addr for map

    //linker->dumpSymbols();  // debug
}

void PackLinuxElf32mipsel::defineSymbols(Filter const * /*ft*/)
{
    unsigned const hlen = sz_elf_hdrs + sizeof(l_info) + sizeof(p_info);

    // We want to know if compressed data, plus stub, plus a couple pages,
    // will fit below the uncompressed program in memory.  But we don't
    // know the final total compressed size yet, so use the uncompressed
    // size (total over all PT_LOAD32) as an upper bound.
    unsigned len = 0;
    unsigned lo_va_user = ~0u;  // infinity
    for (int j= e_phnum; --j>=0; ) {
        if (PT_LOAD32 == get_te32(&phdri[j].p_type)) {
            len += (unsigned)get_te32(&phdri[j].p_filesz);
            unsigned const va = get_te32(&phdri[j].p_vaddr);
            if (va < lo_va_user) {
                lo_va_user = va;
            }
        }
    }
    lsize = /*getLoaderSize()*/  64 * 1024;  // XXX: upper bound; avoid circularity
    unsigned lo_va_stub = get_te32(&elfout.phdr[0].p_vaddr);
    unsigned adrc;
    unsigned adrm;
    unsigned adru;
    unsigned adrx;
    unsigned cntc;
    unsigned lenm;
    unsigned lenu;
    len += (7&-lsize) + lsize;
    bool const is_big = (lo_va_user < (lo_va_stub + len + 2*page_size));
    if (is_big) {
        set_te32(    &elfout.ehdr.e_entry,
            get_te32(&elfout.ehdr.e_entry) + lo_va_user - lo_va_stub);
        set_te32(&elfout.phdr[0].p_vaddr, lo_va_user);
        set_te32(&elfout.phdr[0].p_paddr, lo_va_user);
               lo_va_stub      = lo_va_user;
        adrc = lo_va_stub;
        adrm = getbrk(phdri, e_phnum);
        adru = page_mask & (~page_mask + adrm);  // round up to page boundary
        adrx = adru + hlen;
        lenm = page_size + len;
        lenu = page_size + len;
        cntc = len >> 3;  // over-estimate; corrected at runtime
    }
    else {
        adrm = lo_va_stub + len;
        adrc = adrm;
        adru = lo_va_stub;
        adrx = lo_va_stub + hlen;
        lenm = page_size;
        lenu = page_size + len;
        cntc = 0;
    }
    adrm = page_mask & (~page_mask + adrm);  // round up to page boundary
    adrc = page_mask & (~page_mask + adrc);  // round up to page boundary

    linker->defineSymbol("ADRX", adrx); // compressed input for eXpansion

    // For actual moving, we need the true count, which depends on sz_pack2
    // and is not yet known.  So the runtime stub detects "no move"
    // if adrm==adrc, and otherwise uses actual sz_pack2 to compute cntc.
    //linker->defineSymbol("CNTC", cntc);  // count  for copy
    ACC_UNUSED(cntc);

    linker->defineSymbol("ADRC", adrc);  // addr for copy
    linker->defineSymbol("LENU", lenu);  // len  for unmap
    linker->defineSymbol("ADRU", adru);  // addr for unmap
    linker->defineSymbol("LENM", lenm);  // len  for map
    linker->defineSymbol("ADRM", adrm);  // addr for map

    //linker->dumpSymbols();  // debug
}

void PackLinuxElf32::pack4(OutputFile *fo, Filter &ft)
{
    overlay_offset = sz_elf_hdrs + sizeof(linfo);

    if (opt->o_unix.preserve_build_id) {
        // calc e_shoff here and write shdrout, then o_shstrtab
        //NOTE: these are pushed last to ensure nothing is stepped on
        //for the UPX structure.
        unsigned const len = fpad4(fo);
        set_te32(&elfout.ehdr.e_shoff,len);

        int const ssize = sizeof(shdrout);

        shdrout.shdr[2].sh_offset = len+ssize;
        shdrout.shdr[1].sh_offset = shdrout.shdr[2].sh_offset+shdrout.shdr[2].sh_size;

        fo->write(&shdrout, ssize);

        fo->write(o_shstrtab,shdrout.shdr[2].sh_size);
        fo->write(buildid_data,shdrout.shdr[1].sh_size);
    }

    // Cannot pre-round .p_memsz.  If .p_filesz < .p_memsz, then kernel
    // tries to make .bss, which requires PF_W.
    // But strict SELinux (or PaX, grSecurity) disallows PF_W with PF_X.
    set_te32(&elfout.phdr[0].p_filesz, sz_pack2 + lsize);
              elfout.phdr[0].p_memsz = elfout.phdr[0].p_filesz;
    super::pack4(fo, ft);  // write PackHeader and overlay_offset

    // rewrite Elf header
    if (Elf32_Ehdr::ET_DYN==get_te16(&ehdri.e_type)) {
        unsigned const base= get_te32(&elfout.phdr[0].p_vaddr);
        set_te16(&elfout.ehdr.e_type, Elf32_Ehdr::ET_DYN);
        set_te16(&elfout.ehdr.e_phnum, 1);
        set_te32(    &elfout.ehdr.e_entry,
            get_te32(&elfout.ehdr.e_entry) -  base);
        set_te32(&elfout.phdr[0].p_vaddr, get_te32(&elfout.phdr[0].p_vaddr) - base);
        set_te32(&elfout.phdr[0].p_paddr, get_te32(&elfout.phdr[0].p_paddr) - base);
        // Strict SELinux (or PaX, grSecurity) disallows PF_W with PF_X
        //elfout.phdr[0].p_flags |= Elf32_Phdr::PF_W;
    }

    fo->seek(0, SEEK_SET);
    if (0!=xct_off) {  // shared library
        fo->rewrite(&ehdri, sizeof(ehdri));
        fo->rewrite(phdri, e_phnum * sizeof(*phdri));
        fo->seek(sz_elf_hdrs, SEEK_SET);
        fo->rewrite(&linfo, sizeof(linfo));
    }
    else {
        unsigned const reloc = get_te32(&elfout.phdr[0].p_vaddr);
        Elf32_Phdr *phdr = &elfout.phdr[2];
        unsigned const o_phnum = get_te16(&elfout.ehdr.e_phnum);
        for (unsigned j = 2; j < o_phnum; ++j, ++phdr) {
            if (Elf32_Phdr::PT_NOTE==get_te32(&phdr->p_type)) {
                set_te32(            &phdr->p_vaddr,
                    reloc + get_te32(&phdr->p_vaddr));
                set_te32(            &phdr->p_paddr,
                    reloc + get_te32(&phdr->p_paddr));
            }
        }
        fo->rewrite(&elfout, sizeof(Elf32_Phdr) * o_phnum + sizeof(Elf32_Ehdr));
        fo->seek(sz_elf_hdrs, SEEK_SET);  // skip over PT_NOTE bodies, if any
        fo->rewrite(&linfo, sizeof(linfo));
    }
}

void PackLinuxElf64::pack4(OutputFile *fo, Filter &ft)
{
    overlay_offset = sz_elf_hdrs + sizeof(linfo);

    if (opt->o_unix.preserve_build_id) {
        // calc e_shoff here and write shdrout, then o_shstrtab
        //NOTE: these are pushed last to ensure nothing is stepped on
        //for the UPX structure.
        unsigned const len = fpad4(fo);
        set_te64(&elfout.ehdr.e_shoff,len);

        int const ssize = sizeof(shdrout);

        shdrout.shdr[2].sh_offset = len+ssize;
        shdrout.shdr[1].sh_offset = shdrout.shdr[2].sh_offset+shdrout.shdr[2].sh_size;

        fo->write(&shdrout, ssize);

        fo->write(o_shstrtab,shdrout.shdr[2].sh_size);
        fo->write(buildid_data,shdrout.shdr[1].sh_size);
    }

    // Cannot pre-round .p_memsz.  If .p_filesz < .p_memsz, then kernel
    // tries to make .bss, which requires PF_W.
    // But strict SELinux (or PaX, grSecurity) disallows PF_W with PF_X.
    set_te64(&elfout.phdr[0].p_filesz, sz_pack2 + lsize);
              elfout.phdr[0].p_memsz = elfout.phdr[0].p_filesz;
    super::pack4(fo, ft);  // write PackHeader and overlay_offset

    // rewrite Elf header
    if (Elf64_Ehdr::ET_DYN==get_te16(&ehdri.e_type)) {
        upx_uint64_t const base= get_te64(&elfout.phdr[0].p_vaddr);
        set_te16(&elfout.ehdr.e_type, Elf64_Ehdr::ET_DYN);
        set_te16(&elfout.ehdr.e_phnum, 1);
        set_te64(    &elfout.ehdr.e_entry,
            get_te64(&elfout.ehdr.e_entry) -  base);
        set_te64(&elfout.phdr[0].p_vaddr, get_te64(&elfout.phdr[0].p_vaddr) - base);
        set_te64(&elfout.phdr[0].p_paddr, get_te64(&elfout.phdr[0].p_paddr) - base);
        // Strict SELinux (or PaX, grSecurity) disallows PF_W with PF_X
        //elfout.phdr[0].p_flags |= Elf64_Phdr::PF_W;
    }

    fo->seek(0, SEEK_SET);
    if (0!=xct_off) {  // shared library
        fo->rewrite(&ehdri, sizeof(ehdri));
        fo->rewrite(phdri, e_phnum * sizeof(*phdri));
    }
    else {
        if (Elf64_Phdr::PT_NOTE==get_te64(&elfout.phdr[2].p_type)) {
            upx_uint64_t const reloc = get_te64(&elfout.phdr[0].p_vaddr);
            set_te64(            &elfout.phdr[2].p_vaddr,
                reloc + get_te64(&elfout.phdr[2].p_vaddr));
            set_te64(            &elfout.phdr[2].p_paddr,
                reloc + get_te64(&elfout.phdr[2].p_paddr));
            fo->rewrite(&elfout, sz_elf_hdrs);
            // FIXME   fo->rewrite(&elfnote, sizeof(elfnote));
        }
        else {
            fo->rewrite(&elfout, sz_elf_hdrs);
        }
        fo->rewrite(&linfo, sizeof(linfo));
    }
}

void PackLinuxElf64::unpack(OutputFile *fo)
{
#define MAX_ELF_HDR 1024
    union {
        unsigned char buf[MAX_ELF_HDR];
        //struct { Elf64_Ehdr ehdr; Elf64_Phdr phdr; } e;
    } u;
    Elf64_Ehdr *const ehdr = (Elf64_Ehdr *) u.buf;
    Elf64_Phdr const *phdr = (Elf64_Phdr *) (u.buf + sizeof(*ehdr));

    unsigned szb_info = sizeof(b_info);
    {
        fi->seek(0, SEEK_SET);
        fi->readx(u.buf, MAX_ELF_HDR);
        upx_uint64_t const e_entry = get_te64(&ehdr->e_entry);
        if (e_entry < 0x401180
        &&  ehdr->e_machine==Elf64_Ehdr::EM_386) { /* old style, 8-byte b_info */
            szb_info = 2*sizeof(unsigned);
        }
    }

    Elf64_Phdr phdr0x;
    fi->seek(get_te64(&ehdri.e_phoff), SEEK_SET);
    fi->readx(&phdr0x, sizeof(phdr0x));
    load_va = get_te64(&phdr0x.p_vaddr);

    fi->seek(overlay_offset - sizeof(l_info), SEEK_SET);
    fi->readx(&linfo, sizeof(linfo));
    lsize = get_te16(&linfo.l_lsize);
    p_info hbuf;  fi->readx(&hbuf, sizeof(hbuf));
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

    // decompress PT_LOAD64
    bool first_PF_X = true;
    unsigned const u_phnum = get_te16(&ehdr->e_phnum);
    fi->seek(- (off_t) (szb_info + ph.c_len), SEEK_CUR);
    for (unsigned j=0; j < u_phnum; ++phdr, ++j) {
        if (PT_LOAD64==get_te32(&phdr->p_type)) {
            upx_uint64_t const filesz = get_te64(&phdr->p_filesz);
            upx_uint64_t const offset = get_te64(&phdr->p_offset);
            if (fo)
                fo->seek(offset, SEEK_SET);
            if (Elf64_Phdr::PF_X & get_te32(&phdr->p_flags)) {
                unpackExtent(filesz, fo, total_in, total_out,
                    c_adler, u_adler, first_PF_X, szb_info);
                first_PF_X = false;
            }
            else {
                unpackExtent(filesz, fo, total_in, total_out,
                    c_adler, u_adler, false, szb_info);
            }
        }
    }
    bool const is_shlib = 0;  // XXX ??
    if (is_shlib
    ||  ((unsigned)(get_te64(&ehdri.e_entry) - load_va) + up4(lsize) +
                ph.getPackHeaderSize() + sizeof(overlay_offset))
            < up4(fi->st_size())) {
        // Loader is not at end; skip past it.
        funpad4(fi);  // MATCH01
        fi->seek(lsize, SEEK_CUR);
    }

    // The gaps between PT_LOAD and after last PT_LOAD
    phdr = (Elf64_Phdr *) (u.buf + sizeof(*ehdr));
    for (unsigned j = 0; j < u_phnum; ++j) {
        unsigned const size = find_LOAD_gap(phdr, j, u_phnum);
        if (size) {
            unsigned const where = get_te64(&phdr[j].p_offset) +
                                   get_te64(&phdr[j].p_filesz);
            if (fo)
                fo->seek(where, SEEK_SET);
            unpackExtent(size, fo, total_in, total_out,
                c_adler, u_adler, false, szb_info);
        }
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
#undef MAX_ELF_HDR
}


/*************************************************************************
//
**************************************************************************/

PackLinuxElf32x86::PackLinuxElf32x86(InputFile *f) : super(f)
{
    e_machine = Elf32_Ehdr::EM_386;
    ei_class  = Elf32_Ehdr::ELFCLASS32;
    ei_data   = Elf32_Ehdr::ELFDATA2LSB;
    ei_osabi  = Elf32_Ehdr::ELFOSABI_LINUX;
}

PackLinuxElf32x86::~PackLinuxElf32x86()
{
}

Linker* PackLinuxElf32x86::newLinker() const
{
    return new ElfLinkerX86;
}

PackBSDElf32x86::PackBSDElf32x86(InputFile *f) : super(f)
{
    e_machine = Elf32_Ehdr::EM_386;
    ei_class  = Elf32_Ehdr::ELFCLASS32;
    ei_data   = Elf32_Ehdr::ELFDATA2LSB;
}

PackBSDElf32x86::~PackBSDElf32x86()
{
}

PackFreeBSDElf32x86::PackFreeBSDElf32x86(InputFile *f) : super(f)
{
    ei_osabi  = Elf32_Ehdr::ELFOSABI_FREEBSD;
}

PackFreeBSDElf32x86::~PackFreeBSDElf32x86()
{
}

PackNetBSDElf32x86::PackNetBSDElf32x86(InputFile *f) : super(f)
{
    ei_osabi  = Elf32_Ehdr::ELFOSABI_NETBSD;
    osabi_note = "NetBSD";
}

PackNetBSDElf32x86::~PackNetBSDElf32x86()
{
}

PackOpenBSDElf32x86::PackOpenBSDElf32x86(InputFile *f) : super(f)
{
    ei_osabi  = Elf32_Ehdr::ELFOSABI_OPENBSD;
    osabi_note = "OpenBSD";
}

PackOpenBSDElf32x86::~PackOpenBSDElf32x86()
{
}

int const *
PackLinuxElf32x86::getFilters() const
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
    FT_END };
    return filters;
}

PackLinuxElf32armLe::PackLinuxElf32armLe(InputFile *f) : super(f)
{
    e_machine = Elf32_Ehdr::EM_ARM;
    ei_class  = Elf32_Ehdr::ELFCLASS32;
    ei_data   = Elf32_Ehdr::ELFDATA2LSB;
    ei_osabi  = Elf32_Ehdr::ELFOSABI_ARM;
}

PackLinuxElf32armLe::~PackLinuxElf32armLe()
{
}

PackLinuxElf32mipseb::PackLinuxElf32mipseb(InputFile *f) : super(f)
{
    e_machine = Elf32_Ehdr::EM_MIPS;
    ei_class  = Elf32_Ehdr::ELFCLASS32;
    ei_data   = Elf32_Ehdr::ELFDATA2MSB;
    ei_osabi  = Elf32_Ehdr::ELFOSABI_LINUX;
}

PackLinuxElf32mipseb::~PackLinuxElf32mipseb()
{
}

PackLinuxElf32mipsel::PackLinuxElf32mipsel(InputFile *f) : super(f)
{
    e_machine = Elf32_Ehdr::EM_MIPS;
    ei_class  = Elf32_Ehdr::ELFCLASS32;
    ei_data   = Elf32_Ehdr::ELFDATA2LSB;
    ei_osabi  = Elf32_Ehdr::ELFOSABI_LINUX;
}

PackLinuxElf32mipsel::~PackLinuxElf32mipsel()
{
}

Linker* PackLinuxElf32armLe::newLinker() const
{
    return new ElfLinkerArmLE();
}

Linker* PackLinuxElf32mipseb::newLinker() const
{
    return new ElfLinkerMipsBE();
}

Linker* PackLinuxElf32mipsel::newLinker() const
{
    return new ElfLinkerMipsLE();
}

PackLinuxElf32armBe::PackLinuxElf32armBe(InputFile *f) : super(f)
{
    e_machine = Elf32_Ehdr::EM_ARM;
    ei_class  = Elf32_Ehdr::ELFCLASS32;
    ei_data   = Elf32_Ehdr::ELFDATA2MSB;
    ei_osabi  = Elf32_Ehdr::ELFOSABI_ARM;
}

PackLinuxElf32armBe::~PackLinuxElf32armBe()
{
}

Linker* PackLinuxElf32armBe::newLinker() const
{
    return new ElfLinkerArmBE();
}

unsigned
PackLinuxElf32::elf_get_offset_from_address(unsigned const addr) const
{
    Elf32_Phdr const *phdr = phdri;
    int j = e_phnum;
    for (; --j>=0; ++phdr) if (PT_LOAD32 == get_te32(&phdr->p_type)) {
        unsigned const t = addr - get_te32(&phdr->p_vaddr);
        if (t < get_te32(&phdr->p_filesz)) {
            return t + get_te32(&phdr->p_offset);
        }
    }
    return 0;
}

Elf32_Dyn const *
PackLinuxElf32::elf_has_dynamic(unsigned int const key) const
{
    Elf32_Dyn const *dynp= dynseg;
    if (dynp)
    for (; Elf32_Dyn::DT_NULL!=dynp->d_tag; ++dynp) if (get_te32(&dynp->d_tag)==key) {
        return dynp;
    }
    return 0;
}

void const *
PackLinuxElf32::elf_find_dynamic(unsigned int const key) const
{
    Elf32_Dyn const *dynp= dynseg;
    if (dynp)
    for (; Elf32_Dyn::DT_NULL!=dynp->d_tag; ++dynp) if (get_te32(&dynp->d_tag)==key) {
        unsigned const t= elf_get_offset_from_address(get_te32(&dynp->d_val));
        if (t) {
            return t + file_image;
        }
        break;
    }
    return 0;
}

upx_uint64_t
PackLinuxElf32::elf_unsigned_dynamic(unsigned int const key) const
{
    Elf32_Dyn const *dynp= dynseg;
    if (dynp)
    for (; Elf32_Dyn::DT_NULL!=dynp->d_tag; ++dynp) if (get_te32(&dynp->d_tag)==key) {
        return get_te32(&dynp->d_val);
    }
    return 0;
}

upx_uint64_t
PackLinuxElf64::elf_get_offset_from_address(upx_uint64_t const addr) const
{
    Elf64_Phdr const *phdr = phdri;
    int j = e_phnum;
    for (; --j>=0; ++phdr) if (PT_LOAD64 == get_te32(&phdr->p_type)) {
        upx_uint64_t const t = addr - get_te64(&phdr->p_vaddr);
        if (t < get_te64(&phdr->p_filesz)) {
            return t + get_te64(&phdr->p_offset);
        }
    }
    return 0;
}

Elf64_Dyn const *
PackLinuxElf64::elf_has_dynamic(unsigned int const key) const
{
    Elf64_Dyn const *dynp= dynseg;
    if (dynp)
    for (; Elf64_Dyn::DT_NULL!=dynp->d_tag; ++dynp) if (get_te64(&dynp->d_tag)==key) {
        return dynp;
    }
    return 0;
}

void const *
PackLinuxElf64::elf_find_dynamic(unsigned int const key) const
{
    Elf64_Dyn const *dynp= dynseg;
    if (dynp)
    for (; Elf64_Dyn::DT_NULL!=dynp->d_tag; ++dynp) if (get_te64(&dynp->d_tag)==key) {
        upx_uint64_t const t= elf_get_offset_from_address(get_te64(&dynp->d_val));
        if (t) {
            return (size_t)t + file_image;
        }
        break;
    }
    return 0;
}

upx_uint64_t
PackLinuxElf64::elf_unsigned_dynamic(unsigned int const key) const
{
    Elf64_Dyn const *dynp= dynseg;
    if (dynp)
    for (; Elf64_Dyn::DT_NULL!=dynp->d_tag; ++dynp) if (get_te64(&dynp->d_tag)==key) {
        return get_te64(&dynp->d_val);
    }
    return 0;
}

unsigned PackLinuxElf32::gnu_hash(char const *q)
{
    unsigned char const *p = (unsigned char const *)q;
    unsigned h;

    for (h= 5381; 0!=*p; ++p) {
        h += *p + (h << 5);
    }
    return h;
}

unsigned PackLinuxElf32::elf_hash(char const *p)
{
    unsigned h;
    for (h= 0; 0!=*p; ++p) {
        h = *p + (h<<4);
        {
            unsigned const t = 0xf0000000u & h;
            h &= ~t;
            h ^= t>>24;
        }
    }
    return h;
}

Elf32_Sym const *PackLinuxElf32::elf_lookup(char const *name) const
{
    if (hashtab && dynsym && dynstr) {
        unsigned const nbucket = get_te32(&hashtab[0]);
        unsigned const *const buckets = &hashtab[2];
        unsigned const *const chains = &buckets[nbucket];
        unsigned const m = elf_hash(name) % nbucket;
        unsigned si;
        for (si= get_te32(&buckets[m]); 0!=si; si= get_te32(&chains[si])) {
            char const *const p= get_te32(&dynsym[si].st_name) + dynstr;
            if (0==strcmp(name, p)) {
                return &dynsym[si];
            }
        }
    }
    if (gashtab && dynsym && dynstr) {
        unsigned const n_bucket = get_te32(&gashtab[0]);
        unsigned const symbias  = get_te32(&gashtab[1]);
        unsigned const n_bitmask = get_te32(&gashtab[2]);
        unsigned const gnu_shift = get_te32(&gashtab[3]);
        unsigned const *const bitmask = &gashtab[4];
        unsigned const *const buckets = &bitmask[n_bitmask];

        unsigned const h = gnu_hash(name);
        unsigned const hbit1 = 037& h;
        unsigned const hbit2 = 037& (h>>gnu_shift);
        unsigned const w = get_te32(&bitmask[(n_bitmask -1) & (h>>5)]);

        if (1& (w>>hbit1) & (w>>hbit2)) {
            unsigned bucket = get_te32(&buckets[h % n_bucket]);
            if (0!=bucket) {
                Elf32_Sym const *dsp = dynsym;
                unsigned const *const hasharr = &buckets[n_bucket];
                unsigned const *hp = &hasharr[bucket - symbias];

                dsp += bucket;
                do if (0==((h ^ get_te32(hp))>>1)) {
                    char const *const p = get_te32(&dsp->st_name) + dynstr;
                    if (0==strcmp(name, p)) {
                        return dsp;
                    }
                } while (++dsp, 0==(1u& get_te32(hp++)));
            }
        }
    }
    return 0;

}

void PackLinuxElf32::unpack(OutputFile *fo)
{
#define MAX_ELF_HDR 512
    union {
        unsigned char buf[MAX_ELF_HDR];
#if (ACC_CC_BORLANDC || ACC_CC_SUNPROC)
#else
        struct { Elf32_Ehdr ehdr; Elf32_Phdr phdr; } e;
#endif
    } u;
    COMPILE_TIME_ASSERT(sizeof(u) == MAX_ELF_HDR)
    Elf32_Ehdr *const ehdr = (Elf32_Ehdr *) u.buf;
    Elf32_Phdr const *phdr = (Elf32_Phdr *) (u.buf + sizeof(*ehdr));
    unsigned old_data_off = 0;
    unsigned old_data_len = 0;
    unsigned old_dtinit = 0;

    unsigned szb_info = sizeof(b_info);
    {
        fi->seek(0, SEEK_SET);
        fi->readx(u.buf, MAX_ELF_HDR);
        if (get_te32(&ehdr->e_entry) < 0x401180
        &&  Elf32_Ehdr::EM_386 ==get_te16(&ehdr->e_machine)
        &&  Elf32_Ehdr::ET_EXEC==get_te16(&ehdr->e_type)) {
            // Beware ET_DYN.e_entry==0x10f0 (or so) does NOT qualify here.
            /* old style, 8-byte b_info */
            szb_info = 2*sizeof(unsigned);
        }
    }
    old_dtinit = ehdr->e_shoff;  // copy ONLY, never examined

    Elf32_Phdr phdr0x;
    fi->seek(get_te32(&ehdri.e_phoff), SEEK_SET);
    fi->readx(&phdr0x, sizeof(phdr0x));
    load_va = get_te32(&phdr0x.p_vaddr);

    fi->seek(overlay_offset - sizeof(l_info), SEEK_SET);
    fi->readx(&linfo, sizeof(linfo));
    lsize = get_te16(&linfo.l_lsize);
    p_info hbuf;  fi->readx(&hbuf, sizeof(hbuf));
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
    bool const is_shlib = (ehdr->e_shoff!=0);

    // Peek at resulting Ehdr and Phdrs for use in controlling unpacking.
    // Uncompress an extra time, and don't verify or update checksums.
    if (ibuf.getSize() < ph.c_len  ||  sizeof(u) < ph.u_len)
        throwCompressedDataViolation();
    fi->readx(ibuf, ph.c_len);
    decompress(ibuf, (upx_byte *)ehdr, false);
    fi->seek(- (off_t) (szb_info + ph.c_len), SEEK_CUR);

    unsigned const u_phnum = get_te16(&ehdr->e_phnum);
    unsigned total_in = 0;
    unsigned total_out = 0;
    unsigned c_adler = upx_adler32(NULL, 0);
    unsigned u_adler = upx_adler32(NULL, 0);

    if (is_shlib) {
        // Unpack and output the Ehdr and Phdrs for real.
        // This depends on position within input file fi.
        unpackExtent(ph.u_len, fo, total_in, total_out,
            c_adler, u_adler, false, szb_info);

        // The first PT_LOAD.  Part is not compressed (for benefit of rtld.)
        // Read enough to position the input for next unpackExtent.
        fi->seek(0, SEEK_SET);
        fi->readx(ibuf, overlay_offset + sizeof(hbuf) + szb_info + ph.c_len);
        overlay_offset -= sizeof(linfo);
        if (fo) {
            fo->write(ibuf + ph.u_len, overlay_offset - ph.u_len);
        }
        // Search the Phdrs of compressed
        int n_ptload = 0;
        phdr = (Elf32_Phdr *) (void *) (1+ (Elf32_Ehdr *)(unsigned char *)ibuf);
        for (unsigned j=0; j < u_phnum; ++phdr, ++j) {
            if (PT_LOAD32==get_te32(&phdr->p_type) && 0!=n_ptload++) {
                old_data_off = get_te32(&phdr->p_offset);
                old_data_len = get_te32(&phdr->p_filesz);
                break;
            }
        }

        total_in  = overlay_offset;
        total_out = overlay_offset;
        ph.u_len = 0;

        // Decompress and unfilter the tail of first PT_LOAD.
        phdr = (Elf32_Phdr *) (void *) (1+ ehdr);
        for (unsigned j=0; j < u_phnum; ++phdr, ++j) {
            if (PT_LOAD32==get_te32(&phdr->p_type)) {
                ph.u_len = get_te32(&phdr->p_filesz) - overlay_offset;
                break;
            }
        }
        unpackExtent(ph.u_len, fo, total_in, total_out,
            c_adler, u_adler, false, szb_info);
    }
    else {  // main executable
        // Decompress each PT_LOAD.
        bool first_PF_X = true;
        for (unsigned j=0; j < u_phnum; ++phdr, ++j) {
            if (PT_LOAD32==get_te32(&phdr->p_type)) {
                unsigned const filesz = get_te32(&phdr->p_filesz);
                unsigned const offset = get_te32(&phdr->p_offset);
                if (fo)
                    fo->seek(offset, SEEK_SET);
                if (Elf32_Phdr::PF_X & get_te32(&phdr->p_flags)) {
                    unpackExtent(filesz, fo, total_in, total_out,
                        c_adler, u_adler, first_PF_X, szb_info);
                    first_PF_X = false;
                }
                else {
                    unpackExtent(filesz, fo, total_in, total_out,
                        c_adler, u_adler, false, szb_info);
                }
            }
        }
    }
    if (is_shlib
    ||  ((unsigned)(get_te32(&ehdri.e_entry) - load_va) + up4(lsize) +
                ph.getPackHeaderSize() + sizeof(overlay_offset))
            < up4(fi->st_size())) {
        // Loader is not at end; skip past it.
        funpad4(fi);  // MATCH01
        fi->seek(lsize, SEEK_CUR);
    }

    // The gaps between PT_LOAD and after last PT_LOAD
    phdr = (Elf32_Phdr *) (u.buf + sizeof(*ehdr));
    for (unsigned j = 0; j < u_phnum; ++j) {
        unsigned const size = find_LOAD_gap(phdr, j, u_phnum);
        if (size) {
            unsigned const where = get_te32(&phdr[j].p_offset) +
                                   get_te32(&phdr[j].p_filesz);
            if (fo)
                fo->seek(where, SEEK_SET);
            unpackExtent(size, fo, total_in, total_out,
                c_adler, u_adler, false, szb_info);
        }
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

    if (is_shlib) {  // the non-first PT_LOAD
        int n_ptload = 0;
        unsigned load_off = 0;
        phdr = (Elf32_Phdr *) (u.buf + sizeof(*ehdr));
        for (unsigned j= 0; j < u_phnum; ++j, ++phdr) {
            if (PT_LOAD32==get_te32(&phdr->p_type) && 0!=n_ptload++) {
                load_off = get_te32(&phdr->p_offset);
                fi->seek(old_data_off, SEEK_SET);
                fi->readx(ibuf, old_data_len);
                total_in  += old_data_len;
                total_out += old_data_len;
                if (fo) {
                    fo->seek(get_te32(&phdr->p_offset), SEEK_SET);
                    fo->rewrite(ibuf, old_data_len);
                }
            }
        }
        // Restore DT_INIT.d_val
        phdr = (Elf32_Phdr *) (u.buf + sizeof(*ehdr));
        for (unsigned j= 0; j < u_phnum; ++j, ++phdr) {
            if (phdr->PT_DYNAMIC==get_te32(&phdr->p_type)) {
                unsigned const dyn_off = get_te32(&phdr->p_offset);
                unsigned const dyn_len = get_te32(&phdr->p_filesz);
                Elf32_Dyn *dyn = (Elf32_Dyn *)((unsigned char *)ibuf +
                    (dyn_off - load_off));
                for (unsigned j2= 0; j2 < dyn_len; ++dyn, j2 += sizeof(*dyn)) {
                    if (dyn->DT_INIT==get_te32(&dyn->d_tag)) {
                        if (fo) {
                            fo->seek(sizeof(unsigned) + j2 + dyn_off, SEEK_SET);
                            fo->rewrite(&old_dtinit, sizeof(old_dtinit));
                            fo->seek(0, SEEK_END);
                        }
                        break;
                    }
                }
            }
        }
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

void PackLinuxElf::unpack(OutputFile * /*fo*/)
{
    throwCantUnpack("internal error");
}

/*
vi:ts=4:et
*/

