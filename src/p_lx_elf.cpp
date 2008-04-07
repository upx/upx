/* p_lx_elf.cpp --

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
#include "ui.h"

#define PT_LOAD32   Elf32_Phdr::PT_LOAD
#define PT_LOAD64   Elf64_Phdr::PT_LOAD


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
    if (get_te16(&ehdr->e_machine) != e_machine)
        return 3;
    if (get_te32(&ehdr->e_version) != Elf32_Ehdr::EV_CURRENT)
        return 4;
    if (get_te16(&ehdr->e_phnum) < 1)
        return 5;
    if (get_te16(&ehdr->e_phentsize) != sizeof(Elf32_Phdr))
        return 6;

    // check for Linux kernels
    unsigned const entry = get_te32(&ehdr->e_entry);
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

int
PackLinuxElf64::checkEhdr(Elf64_Ehdr const *ehdr) const
{
    const unsigned char * const buf = ehdr->e_ident;
    unsigned osabi0 = buf[Elf32_Ehdr::EI_OSABI];
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
    if (get_te16(&ehdr->e_machine) != e_machine)
        return 3;
    if (get_te32(&ehdr->e_version) != Elf64_Ehdr::EV_CURRENT)
        return 4;
    if (get_te16(&ehdr->e_phnum) < 1)
        return 5;
    if (get_te16(&ehdr->e_phentsize) != sizeof(Elf64_Phdr))
        return 6;

    // check for Linux kernels
    acc_uint64l_t const entry = get_te64(&ehdr->e_entry);
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

PackLinuxElf::PackLinuxElf(InputFile *f)
    : super(f), file_image(NULL), dynstr(NULL),
    sz_phdrs(0), sz_elf_hdrs(0), sz_pack2(0),
    lg2_page(12), page_size(1u<<lg2_page),
    e_machine(0), ei_class(0), ei_data(0), ei_osabi(0), osabi_note(NULL)
{
}

PackLinuxElf::~PackLinuxElf()
{
    delete[] file_image;
}

void PackLinuxElf::pack3(OutputFile *fo, Filter &ft)
{
    unsigned disp;
    unsigned const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 3& -len);  // ALIGN_UP 0 mod 4
    len += (3& -len); // 0 mod 4
    if (0==(4 & len)) {
        fo->write(&zero, 4);
        len += 4;
    } // 4 mod 8
    set_te32(&disp, len);  // FIXME?  -(sz_elf_hdrs+sizeof(l_info)+sizeof(p_info))
    fo->write(&disp, sizeof(disp));
    sz_pack2 = sizeof(disp) + len;  // 0 mod 8

    super::pack3(fo, ft);
}

void PackLinuxElf32::pack3(OutputFile *fo, Filter &ft)
{
    super::pack3(fo, ft);
    set_te32(&elfout.phdr[0].p_filesz, sz_pack2);
}

void PackLinuxElf64::pack3(OutputFile *fo, Filter &ft)
{
    super::pack3(fo, ft);
    set_te64(&elfout.phdr[0].p_filesz, sz_pack2);
}

void
PackLinuxElf::addStubEntrySections(Filter const *)
{
    addLoader("ELFMAINX", NULL);
   //addLoader(getDecompressorSections(), NULL);
    addLoader(
        ( M_IS_NRV2E(ph.method) ? "NRV_HEAD,NRV2E,NRV_TAIL"
        : M_IS_NRV2D(ph.method) ? "NRV_HEAD,NRV2D,NRV_TAIL"
        : M_IS_NRV2B(ph.method) ? "NRV_HEAD,NRV2B,NRV_TAIL"
        : M_IS_LZMA(ph.method)  ? "LZMA_ELF00,+80C,LZMA_DEC20,LZMA_DEC30"
        : NULL), NULL);
    if (hasLoaderSection("CFLUSH"))
        addLoader("CFLUSH");
    addLoader("ELFMAINY,IDENTSTR,+40,ELFMAINZ,FOLDEXEC", NULL);
}


void PackLinuxElf::defineSymbols(Filter const *)
{
    // empty
}

PackLinuxElf32::PackLinuxElf32(InputFile *f)
    : super(f), phdri(NULL), shdri(NULL), page_mask(~0u<<lg2_page),
    dynseg(NULL), hashtab(NULL), dynsym(NULL),
    shstrtab(NULL), n_elf_shnum(0),
    sec_strndx(NULL), sec_dynsym(NULL), sec_dynstr(NULL)
{
}

PackLinuxElf32::~PackLinuxElf32()
{
    delete[] phdri;
}

PackLinuxElf64::PackLinuxElf64(InputFile *f)
    : super(f), phdri(NULL), page_mask(~0ull<<lg2_page)
{
}

PackLinuxElf64::~PackLinuxElf64()
{
    delete[] phdri;
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

void PackLinuxElf32::ARM_updateLoader(OutputFile *fo)
{
    set_te32(&elfout.ehdr.e_entry, fo->getBytesWritten() +
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

void PackLinuxElf32::updateLoader(OutputFile *fo)
{
    set_te32(&elfout.ehdr.e_entry, fo->getBytesWritten() +
        get_te32(&elfout.phdr[0].p_vaddr));
}

void PackLinuxElf64::updateLoader(OutputFile *fo)
{
    set_te64(&elfout.ehdr.e_entry, fo->getBytesWritten() +
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
//            // Elf32_Edhr
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

    struct b_info h; memset(&h, 0, sizeof(h));
    unsigned fold_hdrlen = 0;
  if (0 < szfold) {
    cprElfHdr1 const *const hf = (cprElfHdr1 const *)fold;
    fold_hdrlen = umax(0x80, sizeof(hf->ehdr) +
        get_te16(&hf->ehdr.e_phentsize) * get_te16(&hf->ehdr.e_phnum) +
            sizeof(l_info) );
    h.sz_unc = ((szfold < fold_hdrlen) ? 0 : (szfold - fold_hdrlen));
    h.b_method = (unsigned char) ph.method;  // FIXME: endian trouble
    h.b_ftid = (unsigned char) ph.filter;
    h.b_cto8 = (unsigned char) ph.filter_cto;
  }
    unsigned char const *const uncLoader = fold_hdrlen + fold;

    h.sz_cpr = MemBuffer::getSizeForCompression(h.sz_unc);
    unsigned char *const cprLoader = new unsigned char[sizeof(h) + h.sz_cpr];
  if (0 < szfold) {
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
  }
    unsigned const sz_cpr = h.sz_cpr;
    set_te32(&h.sz_cpr, h.sz_cpr);
    set_te32(&h.sz_unc, h.sz_unc);
    memcpy(cprLoader, &h, sizeof(h));

    // This adds the definition to the "library", to be used later.
    linker->addSection("FOLDEXEC", cprLoader, sizeof(h) + sz_cpr, 0);
    delete [] cprLoader;

    addStubEntrySections(ft);

    defineSymbols(ft);
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

    struct b_info h; memset(&h, 0, sizeof(h));
    unsigned fold_hdrlen = 0;
  if (0 < szfold) {
    cprElfHdr1 const *const hf = (cprElfHdr1 const *)fold;
    fold_hdrlen = umax(0x80, sizeof(hf->ehdr) +
        get_te16(&hf->ehdr.e_phentsize) * get_te16(&hf->ehdr.e_phnum) +
            sizeof(l_info) );
    h.sz_unc = ((szfold < fold_hdrlen) ? 0 : (szfold - fold_hdrlen));
    h.b_method = (unsigned char) ph.method;  // FIXME: endian trouble
    h.b_ftid = (unsigned char) ph.filter;
    h.b_cto8 = (unsigned char) ph.filter_cto;
  }
    unsigned char const *const uncLoader = fold_hdrlen + fold;

    h.sz_cpr = MemBuffer::getSizeForCompression(h.sz_unc);
    unsigned char *const cprLoader = new unsigned char[sizeof(h) + h.sz_cpr];
  if (0 < szfold) {
    int r = upx_compress(uncLoader, h.sz_unc, sizeof(h) + cprLoader, &h.sz_cpr,
        NULL, ph.method, 10, NULL, NULL );
    if (r != UPX_E_OK || h.sz_cpr >= h.sz_unc)
        throwInternalError("loader compression failed");
  }
    unsigned const sz_cpr = h.sz_cpr;
    set_te32(&h.sz_cpr, h.sz_cpr);
    set_te32(&h.sz_unc, h.sz_unc);
    memcpy(cprLoader, &h, sizeof(h));

    // This adds the definition to the "library", to be used later.
    linker->addSection("FOLDEXEC", cprLoader, sizeof(h) + sz_cpr, 0);
    delete [] cprLoader;

    addStubEntrySections(ft);

    defineSymbols(ft);
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
    acc_uint64l_t lo_va_user = ~0ull;  // infinity
    for (int j= get_te16(&ehdri.e_phnum); --j>=0; ) {
        if (PT_LOAD64 == get_te32(&phdri[j].p_type)) {
            len += (unsigned)get_te64(&phdri[j].p_filesz);
            acc_uint64l_t const va = get_te64(&phdri[j].p_vaddr);
            if (va < lo_va_user) {
                lo_va_user = va;
            }
        }
    }
    lsize = /*getLoaderSize()*/  64 * 1024;  // XXX: upper bound; avoid circularity
    acc_uint64l_t       lo_va_stub = get_te64(&elfout.phdr[0].p_vaddr);
    acc_uint64l_t adrc;
    acc_uint64l_t adrm;
    acc_uint64l_t adru;
    acc_uint64l_t adrx;
    unsigned cntc;
    unsigned lenm;
    unsigned lenu;
    len += (7&-lsize) + lsize;
    bool const is_big = (lo_va_user < (lo_va_stub + len + 2*page_size));
    if (is_big) {
        set_te64(    &elfout.ehdr.e_entry,
            get_te64(&elfout.ehdr.e_entry) + lo_va_user - lo_va_stub);
        set_te64(&elfout.phdr[0].p_vaddr, lo_va_user);
        set_te64(&elfout.phdr[0].p_paddr, lo_va_user);
               lo_va_stub      = lo_va_user;
        adrc = lo_va_stub;
        adrm = getbrk(phdri, get_te16(&ehdri.e_phnum));
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

    linker->defineSymbol("LENU", lenu);  // len  for unmap
    linker->defineSymbol("ADRC", adrc);  // addr for copy
    linker->defineSymbol("ADRU", adru);  // addr for unmap
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

void
PackLinuxElf32x86::buildLoader(const Filter *ft)
{
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

#if 0  //{  re-use for OpenBSD, too
static const
#include "stub/i386-bsd.elf-entry.h"
#endif  //}
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
#include "stub/arm-linux.elf-entry.h"
static const
#include "stub/arm-linux.elf-fold.h"

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
    buildLinuxLoader(
        stub_arm_linux_elf_entry, sizeof(stub_arm_linux_elf_entry),
        stub_arm_linux_elf_fold,  sizeof(stub_arm_linux_elf_fold), ft);
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

void
PackLinuxElf64amd::buildLoader(const Filter *ft)
{
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
        if (0==strcmp(name, &shstrtab[shdr->sh_name])) {
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
        if (type==shdr->sh_type) {
            return shdr;
        }
    }
    return 0;
}

bool PackLinuxElf32::canPack()
{
    unsigned char buf[sizeof(Elf32_Ehdr) + 14*sizeof(Elf32_Phdr)];
    COMPILE_TIME_ASSERT(sizeof(buf) <= 512);

    fi->seek(0, SEEK_SET);
    fi->readx(buf, sizeof(buf));
    fi->seek(0, SEEK_SET);
    Elf32_Ehdr const *const ehdr = (Elf32_Ehdr const *)buf;

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

    unsigned osabi0 = buf[Elf32_Ehdr::EI_OSABI];
    // The first PT_LOAD32 must cover the beginning of the file (0==p_offset).
    unsigned const e_phnum = get_te16(&ehdr->e_phnum);
    Elf32_Phdr const *phdr = (Elf32_Phdr const *)(buf + e_phoff);
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        if (j >= 14)
                return false;
        unsigned const p_type = get_te32(&phdr->p_type);
        unsigned const p_offset = get_te32(&phdr->p_offset);
        if (1!=exetype && phdr->PT_LOAD32 == p_type) {
            if (p_offset != 0) {
                throwCantPack("invalid Phdr p_offset; try '--force-execve'");
                return false;
            }
            exetype = 1;
        }
        if (Elf32_Ehdr::ELFOSABI_NONE==osabi0  // Still seems to be generic.
        && NULL!=osabi_note && phdr->PT_NOTE == p_type) {
            struct Elf32_Note note; memset(&note, 0, sizeof(note));
            fi->seek(p_offset, SEEK_SET);
            fi->readx(&note, sizeof(note));
            fi->seek(0, SEEK_SET);
            if (4==get_te32(&note.descsz)
            &&  1==get_te32(&note.type)
            &&  0==note.end
            &&  (1+ strlen(osabi_note))==get_te32(&note.namesz)
            &&  0==strcmp(osabi_note, (char const *)&note.text)
            ) {
                osabi0 = ei_osabi;  // Specified by PT_NOTE.
            }
        }
    }
    if (Elf32_Ehdr::ELFOSABI_NONE==osabi0) { // No EI_OSBAI, no PT_NOTE.
        osabi0 = opt->o_unix.osabi0;  // Possibly specified by command-line.
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
        ehdri= *ehdr;
        phdri= (Elf32_Phdr *)(e_phoff + file_image);  // do not free() !!
        shdri= (Elf32_Shdr *)(e_shoff + file_image);  // do not free() !!

        n_elf_shnum = get_te16(&ehdr->e_shnum);
        shdri = (Elf32_Shdr const *)(e_shoff + file_image);
        //sec_strndx = &shdri[ehdr->e_shstrndx];
        //shstrtab = (char const *)(sec_strndx->sh_offset + file_image);
        sec_dynsym = elf_find_section_type(Elf32_Shdr::SHT_DYNSYM);
        sec_dynstr = get_te32(&sec_dynsym->sh_link) + shdri;

        int j= e_phnum;
        phdr= phdri;
        for (; --j>=0; ++phdr) if (Elf32_Phdr::PT_DYNAMIC==get_te32(&phdr->p_type)) {
            dynseg= (Elf32_Dyn const *)(get_te32(&phdr->p_offset) + file_image);
            break;
        }
        // elf_find_dynamic() returns 0 if 0==dynseg.
        gashtab= (unsigned int const *)elf_find_dynamic(Elf32_Dyn::DT_GNU_HASH);
        hashtab= (unsigned int const *)elf_find_dynamic(Elf32_Dyn::DT_HASH);
        dynstr=          (char const *)elf_find_dynamic(Elf32_Dyn::DT_STRTAB);
        dynsym=     (Elf32_Sym const *)elf_find_dynamic(Elf32_Dyn::DT_SYMTAB);

        // FIXME 2007-09-10  It seems that DT_GNU_HASH does not have undefined
        // symbols.  So if no DT_HASH, then we would have to look in .dynsym.
        char const *const run_start[]= {
            "__libc_start_main", "__uClibc_main", "__uClibc_start_main",
        };
        for (j=0; j<3; ++j) {
            // elf_lookup() returns 0 if any required table is missing.
            Elf32_Sym const *const lsm = elf_lookup(run_start[j]);
            if (lsm && get_te16(&lsm->st_shndx)==Elf32_Sym::SHN_UNDEF
            && get_te16(&lsm->st_info)==lsm->Elf32_Sym::make_st_info(Elf32_Sym::STB_GLOBAL, Elf32_Sym::STT_FUNC)
            && get_te16(&lsm->st_other)==Elf32_Sym::STV_DEFAULT ) {
                break;
            }
            if (sec_dynsym) {
                Elf32_Sym const *symp = (Elf32_Sym const *)(get_te32(&sec_dynsym->sh_offset) + file_image);
                Elf32_Sym const *const symlwa = (Elf32_Sym const *)(
                    get_te32(&sec_dynsym->sh_size) +
                    get_te32(&sec_dynsym->sh_offset) + file_image);
                for (; symp < symlwa; ++symp)
                if (0==strcmp(run_start[j], get_te32(&symp->st_name) + dynstr)) {
                    goto found;
                }
            }
        }
found:
        phdri = 0;  // done "borrowing" this member
        if (3<=j) {
            return false;
        }
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
    unsigned char buf[sizeof(Elf64_Ehdr) + 14*sizeof(Elf64_Phdr)];
    COMPILE_TIME_ASSERT(sizeof(buf) <= 1024);

    fi->readx(buf, sizeof(buf));
    fi->seek(0, SEEK_SET);
    Elf64_Ehdr const *const ehdr = (Elf64_Ehdr const *)buf;

    // now check the ELF header
    if (checkEhdr(ehdr) != 0)
        return false;

    // additional requirements for linux/elf386
    if (get_te16(&ehdr->e_ehsize) != sizeof(*ehdr)) {
        throwCantPack("invalid Ehdr e_ehsize; try '--force-execve'");
        return false;
    }
    acc_uint64l_t const e_phoff = get_te64(&ehdr->e_phoff);
    if (e_phoff != sizeof(*ehdr)) {// Phdrs not contiguous with Ehdr
        throwCantPack("non-contiguous Ehdr/Phdr; try '--force-execve'");
        return false;
    }

    // The first PT_LOAD64 must cover the beginning of the file (0==p_offset).
    unsigned const e_phnum = get_te16(&ehdr->e_phnum);
    Elf64_Phdr const *phdr = (Elf64_Phdr const *)(buf + (unsigned) e_phoff);
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        if (j >= 14)
            return false;
        if (phdr->PT_LOAD64 == get_te32(&phdr->p_type)) {
            // Just avoid the "rewind" when unpacking?
            //if (phdr->p_offset != 0) {
            //    throwCantPack("invalid Phdr p_offset; try '--force-execve'");
            //    return false;
            //}
            exetype = 1;
            break;
        }
    }

    if (!super::canPack())
        return false;
    assert(exetype == 1);

    exetype = 0;

    // set options
    opt->o_unix.blocksize = blocksize = file_size;
    return true;
}

off_t
PackLinuxElf32::getbrk(const Elf32_Phdr *phdr, int e_phnum) const
{
    off_t brka = 0;
    for (int j = 0; j < e_phnum; ++phdr, ++j) {
        if (PT_LOAD32 == get_te32(&phdr->p_type)) {
            off_t b = get_te32(&phdr->p_vaddr) + get_te32(&phdr->p_memsz);
            if (b > brka)
                brka = b;
        }
    }
    return brka;
}

off_t
PackLinuxElf32::getbase(const Elf32_Phdr *phdr, int e_phnum) const
{
    off_t base = ~0u;
    for (int j = 0; j < e_phnum; ++phdr, ++j) {
        if (phdr->PT_LOAD == phdr->p_type) {
            if (phdr->p_vaddr < (unsigned) base)
                base = phdr->p_vaddr;
        }
    }
    if (0!=base) {
        return base;
    }
    return 0x12000;
}

off_t
PackLinuxElf64::getbrk(const Elf64_Phdr *phdr, int e_phnum) const
{
    off_t brka = 0;
    for (int j = 0; j < e_phnum; ++phdr, ++j) {
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
    cprElfHdr2 *const h2 = (cprElfHdr2 *)&elfout;
    cprElfHdr3 *const h3 = (cprElfHdr3 *)&elfout;
    memcpy(h3, proto, sizeof(*h3));  // reads beyond, but OK
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
                         h2->ehdr.e_shnum = 0;
                         h2->ehdr.e_shstrndx = 0;

    sz_elf_hdrs = sizeof(*h2) - sizeof(linfo);  // default
    set_te32(&h2->phdr[0].p_filesz, sizeof(*h2));  // + identsize;
                  h2->phdr[0].p_memsz = h2->phdr[0].p_filesz;

    unsigned const e_phnum = get_te16(&ehdri.e_phnum);
    Elf32_Phdr const *phdr = phdri;
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        if (phdr->PT_LOAD32 == get_te32(&phdr->p_type)) {
            unsigned x = get_te32(&phdr->p_align) >> lg2_page;
            while (x>>=1) {
                ++lg2_page;
            }
        }
    }
    page_size =  1u<<lg2_page;
    page_mask = ~0u<<lg2_page;
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
PackOpenBSDElf32x86::generateElfHdr(
    OutputFile *fo,
    void const *proto,
    unsigned const brka
)
{
    cprElfHdr3 *const h3 = (cprElfHdr3 *)&elfout;
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

    sz_elf_hdrs = sizeof(*h3) - sizeof(linfo);
    unsigned const note_offset = sz_elf_hdrs;
    set_te32(&h3->phdr[0].p_filesz, sizeof(*h3)+sizeof(elfnote));  // + identsize;
                  h3->phdr[0].p_memsz = h3->phdr[0].p_filesz;

    unsigned const brkb = brka | ((0==(~page_mask & brka)) ? 0x20 : 0);
    set_te32(&h3->phdr[1].p_type, PT_LOAD32);  // be sure
    set_te32(&h3->phdr[1].p_offset, ~page_mask & brkb);
    set_te32(&h3->phdr[1].p_vaddr, brkb);
    set_te32(&h3->phdr[1].p_paddr, brkb);
    h3->phdr[1].p_filesz = 0;
    h3->phdr[1].p_memsz =  0;
    set_te32(&h3->phdr[1].p_flags, Elf32_Phdr::PF_R | Elf32_Phdr::PF_W);

    set_te32(&h3->phdr[2].p_type, Elf32_Phdr::PT_NOTE);
    set_te32(&h3->phdr[2].p_offset, note_offset);
    set_te32(&h3->phdr[2].p_vaddr, note_offset);
    set_te32(&h3->phdr[2].p_paddr, note_offset);
    set_te32(&h3->phdr[2].p_filesz, sizeof(elfnote));
    set_te32(&h3->phdr[2].p_memsz,  sizeof(elfnote));
    set_te32(&h3->phdr[2].p_flags, Elf32_Phdr::PF_R);
    set_te32(&h3->phdr[2].p_align, 4);

    set_te32(&elfnote.namesz, 8);
    set_te32(&elfnote.descsz, 4);
    set_te32(&elfnote.type,   1);
    strcpy(elfnote.text, "OpenBSD");
                  elfnote.end   = 0;

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
    cprElfHdr2 *const h2 = (cprElfHdr2 *)&elfout;
    cprElfHdr3 *const h3 = (cprElfHdr3 *)&elfout;
    memcpy(h3, proto, sizeof(*h3));  // reads beyond, but OK
    h3->ehdr.e_ident[Elf32_Ehdr::EI_OSABI] = ei_osabi;

    assert(get_te32(&h2->ehdr.e_phoff)     == sizeof(Elf64_Ehdr));
                         h2->ehdr.e_shoff = 0;
    assert(get_te16(&h2->ehdr.e_ehsize)    == sizeof(Elf64_Ehdr));
    assert(get_te16(&h2->ehdr.e_phentsize) == sizeof(Elf64_Phdr));
           set_te16(&h2->ehdr.e_shentsize, sizeof(Elf64_Shdr));
                         h2->ehdr.e_shnum = 0;
                         h2->ehdr.e_shstrndx = 0;

    sz_elf_hdrs = sizeof(*h2) - sizeof(linfo);  // default
    set_te64(&h2->phdr[0].p_filesz, sizeof(*h2));  // + identsize;
                  h2->phdr[0].p_memsz = h2->phdr[0].p_filesz;

    unsigned const e_phnum = get_te16(&ehdri.e_phnum);
    Elf64_Phdr const *phdr = phdri;
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        if (phdr->PT_LOAD64 == get_te64(&phdr->p_type)) {
            unsigned x = get_te64(&phdr->p_align) >> lg2_page;
            while (x>>=1) {
                ++lg2_page;
            }
        }
    }
    page_size =  1u  <<lg2_page;
    page_mask = ~0ull<<lg2_page;
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

void PackLinuxElf32::pack1(OutputFile */*fo*/, Filter &/*ft*/)
{
    fi->seek(0, SEEK_SET);
    fi->readx(&ehdri, sizeof(ehdri));
    unsigned const e_phoff = get_te32(&ehdri.e_phoff);
    unsigned const e_phnum = get_te16(&ehdri.e_phnum);
    assert(e_phoff == sizeof(Elf32_Ehdr));  // checked by canPack()
    sz_phdrs = e_phnum * get_te16(&ehdri.e_phentsize);

    phdri = new Elf32_Phdr[e_phnum];
    fi->seek(e_phoff, SEEK_SET);
    fi->readx(phdri, sz_phdrs);

    progid = 0;  // getRandomId();  not useful, so do not clutter
}

void PackLinuxElf32x86::pack1(OutputFile *fo, Filter &ft)
{
    PackLinuxElf32::pack1(fo, ft);
    generateElfHdr(fo, stub_i386_linux_elf_fold, getbrk(phdri, get_te16(&ehdri.e_phnum)) );
}

void PackBSDElf32x86::pack1(OutputFile *fo, Filter &ft)
{
    PackLinuxElf32::pack1(fo, ft);
    generateElfHdr(fo, stub_i386_bsd_elf_fold, getbrk(phdri, get_te16(&ehdri.e_phnum)) );
}

void PackLinuxElf32armLe::pack1(OutputFile *fo, Filter &ft)
{
    super::pack1(fo, ft);
    cprElfHdr3 h3;
    memcpy(&h3, stub_arm_linux_elf_fold, sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr));
    generateElfHdr(fo, &h3, getbrk(phdri, get_te16(&ehdri.e_phnum)) );
}

void PackLinuxElf32armBe::pack1(OutputFile *fo, Filter &ft)
{
    super::pack1(fo, ft);
    cprElfHdr3 h3;
    memcpy(&h3, stub_armeb_linux_elf_fold, sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr));
    generateElfHdr(fo, &h3, getbrk(phdri, get_te16(&ehdri.e_phnum)) );
}

void PackLinuxElf32mipsel::pack1(OutputFile *fo, Filter &ft)
{
    super::pack1(fo, ft);
    cprElfHdr3 h3;
    memcpy(&h3, stub_mipsel_r3000_linux_elf_fold, sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr));
    generateElfHdr(fo, &h3, getbrk(phdri, get_te16(&ehdri.e_phnum)) );
}

void PackLinuxElf32mipseb::pack1(OutputFile *fo, Filter &ft)
{
    super::pack1(fo, ft);
    cprElfHdr3 h3;
    memcpy(&h3, stub_mips_r3000_linux_elf_fold, sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr));
    generateElfHdr(fo, &h3, getbrk(phdri, get_te16(&ehdri.e_phnum)) );
}

void PackLinuxElf32ppc::pack1(OutputFile *fo, Filter &ft)
{
    super::pack1(fo, ft);
    generateElfHdr(fo, stub_powerpc_linux_elf_fold, getbrk(phdri, get_te16(&ehdri.e_phnum)) );
}

void PackLinuxElf64::pack1(OutputFile */*fo*/, Filter &/*ft*/)
{
    fi->seek(0, SEEK_SET);
    fi->readx(&ehdri, sizeof(ehdri));
    unsigned const e_phoff = get_te32(&ehdri.e_phoff);
    unsigned const e_phnum = get_te16(&ehdri.e_phnum);
    assert(e_phoff == sizeof(Elf64_Ehdr));  // checked by canPack()
    sz_phdrs = e_phnum * get_te16(&ehdri.e_phentsize);

    phdri = new Elf64_Phdr[e_phnum];
    fi->seek(e_phoff, SEEK_SET);
    fi->readx(phdri, sz_phdrs);

    progid = 0;  // getRandomId();  not useful, so do not clutter
}

void PackLinuxElf64amd::pack1(OutputFile *fo, Filter &ft)
{
    super::pack1(fo, ft);
    generateElfHdr(fo, stub_amd64_linux_elf_fold, getbrk(phdri, get_te16(&ehdri.e_phnum)) );
}

// Determine length of gap between PT_LOAD phdr[k] and closest PT_LOAD
// which follows in the file (or end-of-file).  Optimize for common case
// where the PT_LOAD are adjacent ascending by .p_offset.  Assume no overlap.

unsigned PackLinuxElf32::find_LOAD_gap(
    Elf32_Phdr const *const phdr,
    unsigned const k,
    unsigned const e_phnum
)
{
    if (PT_LOAD32!=get_te32(&phdr[k].p_type)) {
        return 0;
    }
    unsigned const hi = get_te32(&phdr[k].p_offset) +
                        get_te32(&phdr[k].p_filesz);
    unsigned lo = ph.u_file_size;
    unsigned j = k;
    for (;;) { // circular search, optimize for adjacent ascending
        ++j;
        if (e_phnum==j) {
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

void PackLinuxElf32::pack2(OutputFile *fo, Filter &ft)
{
    Extent x;
    unsigned k;

    // count passes, set ptload vars
    uip->ui_total_passes = 0;
    unsigned const e_phnum = get_te16(&ehdri.e_phnum);
    for (k = 0; k < e_phnum; ++k) {
        if (PT_LOAD32 == get_te32(&phdri[k].p_type)) {
            uip->ui_total_passes++;
            if (find_LOAD_gap(phdri, k, e_phnum)) {
                uip->ui_total_passes++;
            }
        }
    }

    // compress extents
    unsigned total_in = 0;
    unsigned total_out = 0;

    unsigned hdr_u_len = sizeof(Elf32_Ehdr) + sz_phdrs;

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
            ((0==nx && (Elf32_Phdr::PF_X & get_te32(&phdri[k].p_flags)))
                ? &ft : 0 ), fo, hdr_u_len);
        hdr_u_len = 0;
        ++nx;
    }
    for (k = 0; k < e_phnum; ++k) {
        x.size = find_LOAD_gap(phdri, k, e_phnum);
        if (x.size) {
            x.offset = get_te32(&phdri[k].p_offset) +
                       get_te32(&phdri[k].p_filesz);
            packExtent(x, total_in, total_out, 0, fo);
        }
    }

    if ((off_t)total_in != file_size)
        throwEOFException();
}

// Determine length of gap between PT_LOAD phdr[k] and closest PT_LOAD
// which follows in the file (or end-of-file).  Optimize for common case
// where the PT_LOAD are adjacent ascending by .p_offset.  Assume no overlap.

unsigned PackLinuxElf64::find_LOAD_gap(
    Elf64_Phdr const *const phdr,
    unsigned const k,
    unsigned const e_phnum
)
{
    if (PT_LOAD64!=get_te32(&phdr[k].p_type)) {
        return 0;
    }
    unsigned const hi = get_te64(&phdr[k].p_offset) +
                        get_te64(&phdr[k].p_filesz);
    unsigned lo = ph.u_file_size;
    unsigned j = k;
    for (;;) { // circular search, optimize for adjacent ascending
        ++j;
        if (e_phnum==j) {
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

void PackLinuxElf64::pack2(OutputFile *fo, Filter &ft)
{
    Extent x;
    unsigned k;

    // count passes, set ptload vars
    uip->ui_total_passes = 0;
    unsigned const e_phnum = get_te16(&ehdri.e_phnum);
    for (k = 0; k < e_phnum; ++k) {
        if (PT_LOAD64==get_te32(&phdri[k].p_type)) {
            uip->ui_total_passes++;
            if (find_LOAD_gap(phdri, k, e_phnum)) {
                uip->ui_total_passes++;
            }
        }
    }

    // compress extents
    unsigned total_in = 0;
    unsigned total_out = 0;

    unsigned hdr_u_len = sizeof(Elf64_Ehdr) + sz_phdrs;

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
            unsigned const delta = sizeof(Elf64_Ehdr) + sz_phdrs;
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
            ((0==nx && (Elf64_Phdr::PF_X & get_te64(&phdri[k].p_flags)))
                ? &ft : 0 ), fo, hdr_u_len);
        hdr_u_len = 0;
        ++nx;
    }
    for (k = 0; k < e_phnum; ++k) {
        x.size = find_LOAD_gap(phdri, k, e_phnum);
        if (x.size) {
            x.offset = get_te64(&phdri[k].p_offset) +
                       get_te64(&phdri[k].p_filesz);
            packExtent(x, total_in, total_out, 0, fo);
        }
    }

    if ((off_t)total_in != file_size)
        throwEOFException();
}

#include "bele.h"
using namespace N_BELE_CTP;

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

void PackLinuxElf32::ARM_defineSymbols(Filter const * /*ft*/)
{
    unsigned const hlen = sz_elf_hdrs + sizeof(l_info) + sizeof(p_info);

    lsize = /*getLoaderSize()*/  4 * 1024;  // upper bound; avoid circularity
    unsigned const lo_va_user = 0x8000;  // XXX
    unsigned lo_va_stub = get_te32(&elfout.phdr[0].p_vaddr);
    unsigned adrc;
    unsigned adrm;
    unsigned adrx;

    bool const is_big = true;
    if (is_big) {
        set_te32(    &elfout.ehdr.e_entry, linker->getSymbolOffset("_start") +
            get_te32(&elfout.ehdr.e_entry) + lo_va_user - lo_va_stub);
        set_te32(&elfout.phdr[0].p_vaddr, lo_va_user);
        set_te32(&elfout.phdr[0].p_paddr, lo_va_user);
                              lo_va_stub    = lo_va_user;
        adrc = lo_va_stub;
        adrm = getbrk(phdri, get_te16(&ehdri.e_phnum));
        adrx = hlen + (page_mask & (~page_mask + adrm));  // round up to page boundary
    }
    adrm = page_mask & (~page_mask + adrm);  // round up to page boundary
    adrc = page_mask & (~page_mask + adrc);  // round up to page boundary

    linker->defineSymbol("CPR0", 4+ linker->getSymbolOffset("cpr0"));
    linker->defineSymbol("LENF", 4+ linker->getSymbolOffset("end_decompress"));

    linker->defineSymbol("ADRM", adrm);  // addr for map
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
    for (int j= get_te16(&ehdri.e_phnum); --j>=0; ) {
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
        adrm = getbrk(phdri, get_te16(&ehdri.e_phnum));
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
    for (int j= get_te16(&ehdri.e_phnum); --j>=0; ) {
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
        adrm = getbrk(phdri, get_te16(&ehdri.e_phnum));
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
    unsigned const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 3& -len);  // align to 0 mod 4
    len += 3& -len;
    set_te32(&elfout.phdr[0].p_filesz, len);
    super::pack4(fo, ft);  // write PackHeader and overlay_offset

    // Cannot pre-round .p_memsz.  If .p_filesz < .p_memsz, then kernel
    // tries to make .bss, which requires PF_W.
    // But strict SELinux (or PaX, grSecurity) disallows PF_W with PF_X.
#if 0  /*{*/
    // pre-calculate for benefit of runtime disappearing act via munmap()
    set_te32(&elfout.phdr[0].p_memsz, page_mask & (~page_mask + len));
#else  /*}{*/
    set_te32(&elfout.phdr[0].p_memsz, len);
#endif  /*}*/

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
    if (Elf32_Phdr::PT_NOTE==get_te32(&elfout.phdr[2].p_type)) {
        unsigned const reloc = get_te32(&elfout.phdr[0].p_vaddr);
        set_te32(            &elfout.phdr[2].p_vaddr,
            reloc + get_te32(&elfout.phdr[2].p_vaddr));
        set_te32(            &elfout.phdr[2].p_paddr,
            reloc + get_te32(&elfout.phdr[2].p_paddr));
        fo->rewrite(&elfout, sz_elf_hdrs);
        fo->rewrite(&elfnote, sizeof(elfnote));
    }
    else {
        fo->rewrite(&elfout, sz_elf_hdrs);
    }
    fo->rewrite(&linfo, sizeof(linfo));
}

void PackLinuxElf64::pack4(OutputFile *fo, Filter &ft)
{
    overlay_offset = sz_elf_hdrs + sizeof(linfo);
    unsigned const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 3& -len);  // align to 0 mod 4
    len += 3& -len;
    set_te64(&elfout.phdr[0].p_filesz, len);
    super::pack4(fo, ft);  // write PackHeader and overlay_offset

    // Cannot pre-round .p_memsz.  If .p_filesz < .p_memsz, then kernel
    // tries to make .bss, which requires PF_W.
    // But strict SELinux (or PaX, grSecurity) disallows PF_W with PF_X.
#if 0  /*{*/
    // pre-calculate for benefit of runtime disappearing act via munmap()
    set_te64(&elfout.phdr[0].p_memsz, page_mask & (~page_mask + len));
#else  /*}{*/
    set_te64(&elfout.phdr[0].p_memsz, len);
#endif  /*}*/

    // rewrite Elf header
    fo->seek(0, SEEK_SET);
    fo->rewrite(&elfout, sz_elf_hdrs);
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
        unsigned const e_entry = get_te32(&ehdr->e_entry);
        if (e_entry < 0x401180
        &&  ehdr->e_machine==Elf32_Ehdr::EM_386) { /* old style, 8-byte b_info */
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

    // decompress PT_LOAD32
    bool first_PF_X = true;
    unsigned const phnum = get_te16(&ehdr->e_phnum);
    fi->seek(- (off_t) (szb_info + ph.c_len), SEEK_CUR);
    for (unsigned j=0; j < phnum; ++phdr, ++j) {
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

    phdr = (Elf32_Phdr *)(1+ehdr);
    for (unsigned j = 0; j < phnum; ++j) {
        unsigned const size = find_LOAD_gap(phdr, j, phnum);
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

void PackLinuxElf64::unpack(OutputFile *fo)
{
#define MAX_ELF_HDR 1024
    char bufehdr[MAX_ELF_HDR];
    Elf64_Ehdr *const ehdr = (Elf64_Ehdr *)bufehdr;
    Elf64_Phdr const *phdr = (Elf64_Phdr *)(1+ehdr);

    unsigned szb_info = sizeof(b_info);
    {
        fi->seek(0, SEEK_SET);
        fi->readx(bufehdr, MAX_ELF_HDR);
        acc_uint64l_t const e_entry = get_te64(&ehdr->e_entry);
        if (e_entry < 0x401180
        &&  ehdr->e_machine==Elf64_Ehdr::EM_386) { /* old style, 8-byte b_info */
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

    // decompress PT_LOAD32
    bool first_PF_X = true;
    unsigned const phnum = get_te16(&ehdr->e_phnum);
    fi->seek(- (off_t) (szb_info + ph.c_len), SEEK_CUR);
    for (unsigned j=0; j < phnum; ++phdr, ++j) {
        if (PT_LOAD32==get_te32(&phdr->p_type)) {
            acc_uint64l_t const filesz = get_te64(&phdr->p_filesz);
            acc_uint64l_t const offset = get_te64(&phdr->p_offset);
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

    phdr = (Elf64_Phdr *)(1+ehdr);
    for (unsigned j = 0; j < phnum; ++j) {
        unsigned const size = find_LOAD_gap(phdr, j, phnum);
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
    int j = get_te16(&ehdri.e_phnum);
    for (; --j>=0; ++phdr) if (PT_LOAD32 == get_te32(&phdr->p_type)) {
        unsigned const t = addr - get_te32(&phdr->p_vaddr);
        if (t < get_te32(&phdr->p_filesz)) {
            return t + get_te32(&phdr->p_offset);
        }
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
            char const *const p= dynsym[si].st_name + dynstr;
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
                    char const *const p = dsp->st_name + dynstr;
                    if (0==strcmp(name, p)) {
                        return dsp;
                    }
                } while (++dsp, 0==(1u& get_te32(hp++)));
            }
        }
    }
    return 0;

}

void PackLinuxElf32x86::unpack(OutputFile *fo)
{
#define MAX_ELF_HDR 512
    char bufehdr[MAX_ELF_HDR];
    Elf32_Ehdr *const ehdr = (Elf32_Ehdr *)bufehdr;
    Elf32_Phdr const *phdr = (Elf32_Phdr *)(1+ehdr);

    unsigned szb_info = sizeof(b_info);
    {
        fi->seek(0, SEEK_SET);
        fi->readx(bufehdr, MAX_ELF_HDR);
        unsigned const e_entry = get_te32(&ehdr->e_entry);
        unsigned const e_type = get_te16(&ehdr->e_type);
        if (e_entry < 0x401180 && Elf32_Ehdr::ET_EXEC==e_type) {
            // beware ET_DYN.e_entry==0x10f0 or so
            /* old style, 8-byte b_info */
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

    // decompress PT_LOAD32
    bool first_PF_X = true;
    fi->seek(- (off_t) (szb_info + ph.c_len), SEEK_CUR);
    unsigned const phnum = get_te16(&ehdr->e_phnum);
    for (unsigned j=0; j < phnum; ++phdr, ++j) {
        if (PT_LOAD32==get_te32(&phdr->p_type)) {
            if (fo)
                fo->seek(get_te32(&phdr->p_offset), SEEK_SET);
            if (Elf32_Phdr::PF_X & phdr->p_flags) {
                unpackExtent(get_te32(&phdr->p_filesz), fo, total_in, total_out,
                    c_adler, u_adler, first_PF_X, szb_info);
                first_PF_X = false;
            }
            else {
                unpackExtent(get_te32(&phdr->p_filesz), fo, total_in, total_out,
                    c_adler, u_adler, false, szb_info);
            }
        }
    }

    phdr = (Elf32_Phdr *)(1+ehdr);
    for (unsigned j = 0; j < phnum; ++j) {
        unsigned const size = find_LOAD_gap(phdr, j, phnum);
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

