/* p_lx_elf.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2006 Laszlo Molnar
   Copyright (C) 2000-2006 John F. Reiser
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
   <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>

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

#define PT_LOAD32   Elf32_Phdr::PT_LOAD
#define PT_LOAD64   Elf64_Phdr::PT_LOAD


int
PackLinuxElf32::checkEhdr(Elf32_Ehdr const *ehdr) const
{
    const unsigned char * const buf = ehdr->e_ident;

    if (0!=memcmp(buf, "\x7f\x45\x4c\x46", 4)  // "\177ELF"
    ||  buf[Elf32_Ehdr::EI_CLASS]!=ei_class
    ||  buf[Elf32_Ehdr::EI_DATA] !=ei_data ) {
        return -1;
    }
    if (!memcmp(buf+8, "FreeBSD", 7))                   // branded
        return 1;

    int const type = get_native16(&ehdr->e_type);
    if (type != Elf32_Ehdr::ET_EXEC && type != Elf32_Ehdr::ET_DYN)
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

int
PackLinuxElf64::checkEhdr(Elf64_Ehdr const *ehdr) const
{
    const unsigned char * const buf = ehdr->e_ident;

    if (0!=memcmp(buf, "\x7f\x45\x4c\x46", 4)  // "\177ELF"
    ||  buf[Elf64_Ehdr::EI_CLASS]!=ei_class
    ||  buf[Elf64_Ehdr::EI_DATA] !=ei_data ) {
        return -1;
    }
    if (!memcmp(buf+8, "FreeBSD", 7))                   // branded
        return 1;

    if (get_native16(&ehdr->e_type) != Elf64_Ehdr::ET_EXEC)
        return 2;
    if (get_native16(&ehdr->e_machine) != e_machine)
        return 3;
    if (get_native32(&ehdr->e_version) != Elf64_Ehdr::EV_CURRENT)
        return 4;
    if (get_native16(&ehdr->e_phnum) < 1)
        return 5;
    if (get_native16(&ehdr->e_phentsize) != sizeof(Elf64_Phdr))
        return 6;

    // check for Linux kernels
    acc_uint64l_t const entry = get_native64(&ehdr->e_entry);
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
    sz_phdrs(0), sz_elf_hdrs(0),
    e_machine(0), ei_class(0), ei_data(0)
{
    delete[] file_image;
}

PackLinuxElf::~PackLinuxElf()
{
}

PackLinuxElf32::PackLinuxElf32(InputFile *f)
    : super(f), phdri(NULL),
    dynseg(NULL), hashtab(NULL), dynsym(NULL)
{
}

PackLinuxElf32::~PackLinuxElf32()
{
    delete[] phdri;
}

PackLinuxElf64::PackLinuxElf64(InputFile *f)
    : super(f), phdri(NULL)
{
}

PackLinuxElf64::~PackLinuxElf64()
{
    delete[] phdri;
}

int const *
PackLinuxElf::getCompressionMethods(int method, int level) const
{
    // No real dependency on LE32.
    return Packer::getDefaultCompressionMethods_le32(method, level);
}

int const *
PackLinuxElf32ppc::getCompressionMethods(int method, int level) const
{
    // No real dependency on LE32.
    static const int m_nrv2e[] = { M_NRV2E_LE32, -1 };
    static const int m_nrv2b[] = { M_NRV2B_LE32, -1 };

    /*return Packer::getDefaultCompressionMethods_le32(method, level);*/
    // 2005-04-23 FIXME: stub/l_lx_elfppc32.S hardwires ppc_d_nrv2e.S
UNUSED(method); UNUSED(level); UNUSED(m_nrv2b);
    return m_nrv2e;
}

int const *
PackLinuxElf64amd::getCompressionMethods(int method, int level) const
{
    // No real dependency on LE32.
    static const int m_nrv2b[] = { M_NRV2B_LE32, M_NRV2E_LE32, M_LZMA, -1 };
    static const int m_nrv2e[] = { M_NRV2E_LE32, M_NRV2B_LE32, M_LZMA, -1 };
    static const int m_lzma[]  = { M_LZMA,-1 };

    if (M_IS_NRV2B(method))
        return m_nrv2b;
    if (M_IS_NRV2E(method))
        return m_nrv2e;
    if (M_IS_LZMA(method))
        return m_lzma;
    if (1==level)
        return m_nrv2b;
    return m_nrv2e;
}

int const *
PackLinuxElf32armLe::getCompressionMethods(int /*method*/, int /*level*/) const
{
    static const int m_nrv2e[] = { M_NRV2E_8, -1 };

    return m_nrv2e;
}

int const *
PackLinuxElf32armBe::getCompressionMethods(int /*method*/, int /*level*/) const
{
    static const int m_nrv2e[] = { M_NRV2E_8, -1 };

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

int const *
PackLinuxElf64amd::getFilters() const
{
    static const int filters[] = {
        0x49, -1
    };
    return filters;
}

void PackLinuxElf32::patchLoader()
{
}

void PackLinuxElf64::patchLoader()
{
}

void PackLinuxElf32::updateLoader(OutputFile *fo)
{
    set_native32(&elfout.ehdr.e_entry, fo->getBytesWritten() +
        get_native32(&elfout.phdr[0].p_vaddr));
}

void PackLinuxElf64::updateLoader(OutputFile *fo)
{
    set_native64(&elfout.ehdr.e_entry, fo->getBytesWritten() +
        get_native64(&elfout.phdr[0].p_vaddr));
}

PackLinuxElf32ppc::PackLinuxElf32ppc(InputFile *f)
    : super(f)
{
    e_machine = Elf32_Ehdr::EM_PPC;
    ei_class  = Elf32_Ehdr::ELFCLASS32;
    ei_data   = Elf32_Ehdr::ELFDATA2MSB;
}

PackLinuxElf32ppc::~PackLinuxElf32ppc()
{
}

PackLinuxElf64amd::PackLinuxElf64amd(InputFile *f)
    : super(f)
{
    e_machine = Elf64_Ehdr::EM_X86_64;
    ei_class = Elf64_Ehdr::ELFCLASS64;
    ei_data = Elf64_Ehdr::ELFDATA2LSB;
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

int
PackLinuxElf32x86::buildLinuxLoader(
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
    fold_hdrlen = sizeof(hf->ehdr) + hf->ehdr.e_phentsize * hf->ehdr.e_phnum +
         sizeof(l_info);
    if (0 == get_le32(fold_hdrlen + fold)) {
        // inconsistent SIZEOF_HEADERS in *.lds (ld, binutils)
        fold_hdrlen = umax(0x80, fold_hdrlen);
    }
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
    if (r != UPX_E_OK || h.sz_cpr >= h.sz_unc)
        throwInternalError("loader compression failed");
    h.sz_cpr = sz_cpr;
  }
    memcpy(cprLoader, &h, sizeof(h));

    // This adds the definition to the "library", to be used later.
    linker->addSection("FOLDEXEC", cprLoader, sizeof(h) + h.sz_cpr);
    delete [] cprLoader;

    int const n_mru = ft->n_mru;  // FIXME: belongs to filter? packerf?

    // Here is a quick summary of the format of the output file:
    linker->setLoaderAlignOffset(
            // Elf32_Edhr
        sizeof(elfout.ehdr) +
            // Elf32_Phdr: 1 for exec86, 2 for sh86, 3 for elf86
        (elfout.ehdr.e_phentsize * elfout.ehdr.e_phnum) +
            // checksum UPX! lsize version format
        sizeof(l_info) +
            // PT_DYNAMIC with DT_NEEDED "forwarded" from original file
        ((elfout.ehdr.e_phnum==3) ? (unsigned) elfout.phdr[2].p_memsz : 0) +
            // p_progid, p_filesize, p_blocksize
        sizeof(p_info) +
            // compressed data
        b_len + ph.c_len );
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
    addLoader(getDecompressor(), NULL);
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

    char *ptr_cto = (char *)const_cast<unsigned char *>(getLoader());
    int sz_cto = getLoaderSize();
    if (0x20==(ft->id & 0xF0) || 0x30==(ft->id & 0xF0)) {  // push byte '?'  ; cto8
        patch_le16(ptr_cto, sz_cto, "\x6a?", 0x6a + (ft->cto << 8));
        checkPatch(NULL, 0, 0, 0);  // reset
    }
    // PackHeader and overlay_offset at the end of the output file,
    // after the compressed data.

    return getLoaderSize();
}

int
PackLinuxElf32::buildLinuxLoader(
    upx_byte const *const proto,
    unsigned        const szproto,
    upx_byte const *const fold,
    unsigned        const szfold,
    Filter const */*ft*/
)
{
    {
        int const MAX_LOADER_LEN = 8000;
        int *const eof_empty = new int[MAX_LOADER_LEN/sizeof(int)];
        eof_empty[0] = -1;
        initLoader(eof_empty, MAX_LOADER_LEN, 0, 0);
        delete[] eof_empty;
    }

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
    sz_cpr = 0;
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

    linker->addSection("ELFMAINX", proto, szproto);

    addLoader("ELFMAINX", NULL);
    addLoader("FOLDEXEC", NULL);
    return getLoaderSize();
}

int
PackLinuxElf64::buildLinuxLoader(
    upx_byte const *const proto,
    unsigned        const szproto,
    upx_byte const *const fold,
    unsigned        const szfold,
    Filter const */*ft*/
)
{
    {
        int const MAX_LOADER_LEN = 8000;
        int *const eof_empty = new int[MAX_LOADER_LEN/sizeof(int)];
        eof_empty[0] = -1;
        initLoader(eof_empty, MAX_LOADER_LEN, 0, 0);
        delete[] eof_empty;
    }

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
    sz_cpr = 0;
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

    linker->addSection("ELFMAINX", proto, szproto);

    addLoader("ELFMAINX", NULL);
    addLoader("FOLDEXEC", NULL);
    return getLoaderSize();
}

static const
#include "stub/i386-linux.elf-entry.h"
static const
#include "stub/i386-linux.elf-fold.h"

int
PackLinuxElf32x86::buildLoader(const Filter *ft)
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

static const
#include "stub/arm-linux.elf-entry.h"
static const
#include "stub/arm-linux.elf-fold.h"

#include "mem.h"

static void brev(
    unsigned char       *const dst,
    unsigned char const *const src,
    unsigned len
)
{
    assert(0==(3 & len));
    // Detect overlap which over-writes src before it is used.
    assert(!((4+ src)<=dst && dst < (len + src)));
    for (unsigned j = 0; j < len; j += 4) {
        // Simple way (and somewhat slow) to allow in-place brev().
        unsigned char tmp[4];
        memcpy(tmp, j + src, 4);
        dst[0+ j] = tmp[3];
        dst[1+ j] = tmp[2];
        dst[2+ j] = tmp[1];
        dst[3+ j] = tmp[0];
    }
}

static void
ehdr_bele(Elf_BE32_Ehdr *const ehdr_be, Elf_LE32_Ehdr const *const ehdr_le)
{
    memcpy(&ehdr_be->e_ident, &ehdr_le->e_ident, sizeof(ehdr_be->e_ident));
    ehdr_be->e_ident[Elf32_Ehdr::EI_DATA] = Elf32_Ehdr::ELFDATA2MSB;
    ehdr_be->e_type      = ehdr_le->e_type;
    ehdr_be->e_machine   = ehdr_le->e_machine;
    ehdr_be->e_version   = ehdr_le->e_version;
    ehdr_be->e_entry     = ehdr_le->e_entry;
    ehdr_be->e_phoff     = ehdr_le->e_phoff;
    ehdr_be->e_shoff     = ehdr_le->e_shoff;
    ehdr_be->e_flags     = ehdr_le->e_flags;
    ehdr_be->e_ehsize    = ehdr_le->e_ehsize;
    ehdr_be->e_phentsize = ehdr_le->e_phentsize;
    ehdr_be->e_phnum     = ehdr_le->e_phnum;
    ehdr_be->e_shentsize = ehdr_le->e_shentsize;
    ehdr_be->e_shnum     = ehdr_le->e_shnum;
    ehdr_be->e_shstrndx  = ehdr_le->e_shstrndx;
}

static void
ehdr_lebe(Elf_LE32_Ehdr *const ehdr_le, Elf_BE32_Ehdr const *const ehdr_be)
{
    memcpy(&ehdr_le->e_ident, &ehdr_be->e_ident, sizeof(ehdr_le->e_ident));
    ehdr_le->e_ident[Elf32_Ehdr::EI_DATA] = Elf32_Ehdr::ELFDATA2LSB;
    ehdr_le->e_type      = ehdr_be->e_type;
    ehdr_le->e_machine   = ehdr_be->e_machine;
    ehdr_le->e_version   = ehdr_be->e_version;
    ehdr_le->e_entry     = ehdr_be->e_entry;
    ehdr_le->e_phoff     = ehdr_be->e_phoff;
    ehdr_le->e_shoff     = ehdr_be->e_shoff;
    ehdr_le->e_flags     = ehdr_be->e_flags;
    ehdr_le->e_ehsize    = ehdr_be->e_ehsize;
    ehdr_le->e_phentsize = ehdr_be->e_phentsize;
    ehdr_le->e_phnum     = ehdr_be->e_phnum;
    ehdr_le->e_shentsize = ehdr_be->e_shentsize;
    ehdr_le->e_shnum     = ehdr_be->e_shnum;
    ehdr_le->e_shstrndx  = ehdr_be->e_shstrndx;
}

int
PackLinuxElf32::ARM_buildLoader(const Filter *ft, void (*fix_ehdr)(void *, void const *))
{
    unsigned const sz_loader = sizeof(linux_elf32arm_loader);
    unsigned const sz_fold   = sizeof(linux_elf32arm_fold);

    if (this->ei_data
    == ((Elf32_Ehdr const *)linux_elf32arm_fold)->e_ident[Elf32_Ehdr::EI_DATA] ) {
        return buildLinuxLoader(linux_elf32arm_loader, sz_loader,
            linux_elf32arm_fold, sz_fold, ft );
    }
    else {
        // linux_elf32arm_loader[] is all instructions, except for two strings
        // at the end: the copyright message, and the SELinux message.
        // The copyright message begins and ends with '\n', and the SELinux
        // message ends with '\n'.  So copy back to the third '\n' from the end,
        // and apply brev() only before that point.
        MemBuffer brev_loader(sz_loader);
        MemBuffer brev_fold  (sz_fold);
        int nl = 0;
        int j;
        for (j= sz_loader; --j>=0; ) {
            unsigned char const c = linux_elf32arm_loader[j];
            brev_loader[j] = c;
            if ('\n'==c) {
                if (3==++nl) {
                    break;
                }
            }
        }
        brev(brev_loader, linux_elf32arm_loader, j);
        brev(brev_fold,   linux_elf32arm_fold,   sz_fold);
        fix_ehdr(brev_fold.getVoidPtr(), (void const *)linux_elf32arm_fold);
        return buildLinuxLoader(brev_loader, sz_loader, brev_fold, sz_fold, ft);
    }
}

int
PackLinuxElf32armBe::buildLoader(Filter const *ft)
{
    return ARM_buildLoader(ft, (void (*)(void*, const void*))ehdr_bele);
}

int
PackLinuxElf32armLe::buildLoader(Filter const *ft)
{
    return ARM_buildLoader(ft, (void (*)(void*, const void*))ehdr_lebe);
}

static const
#include "stub/powerpc-linux.elf-entry.h"
static const
#include "stub/powerpc-linux.elf-fold.h"

int
PackLinuxElf32ppc::buildLoader(const Filter *ft)
{
    return buildLinuxLoader(
        linux_elfppc32_loader, sizeof(linux_elfppc32_loader),
        linux_elfppc32_fold,   sizeof(linux_elfppc32_fold),  ft );
}

static const
#include "stub/amd64-linux.elf-entry.h"
static const
#include "stub/amd64-linux.elf-fold.h"

int
PackLinuxElf64amd::buildLoader(const Filter *ft)
{
    return buildLinuxLoader(
        linux_elf64amd_loader, sizeof(linux_elf64amd_loader),
        linux_elf64amd_fold,   sizeof(linux_elf64amd_fold),  ft );
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
    if (get_native16(&ehdr->e_ehsize) != sizeof(*ehdr)) {
        throwCantPack("invalid Ehdr e_ehsize; try `--force-execve'");
        return false;
    }
    unsigned const e_phoff = get_native32(&ehdr->e_phoff);
    if (e_phoff != sizeof(*ehdr)) {// Phdrs not contiguous with Ehdr
        throwCantPack("non-contiguous Ehdr/Phdr; try `--force-execve'");
        return false;
    }

    // The first PT_LOAD32 must cover the beginning of the file (0==p_offset).
    unsigned const e_phnum = get_native16(&ehdr->e_phnum);
    Elf32_Phdr const *phdr = (Elf32_Phdr const *)(buf + e_phoff);
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        if (j >= 14)
            return false;
        if (phdr->PT_LOAD32 == get_native32(&phdr->p_type)) {
            if (phdr->p_offset != 0) {
                throwCantPack("invalid Phdr p_offset; try `--force-execve'");
                return false;
            }
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

    if (Elf32_Ehdr::ET_DYN==get_native16(&ehdr->e_type)) {
        // The DT_STRTAB has no designated length.  Read the whole file.
        file_image = new char[file_size];
        fi->seek(0, SEEK_SET);
        fi->readx(file_image, file_size);
        ehdri= *ehdr;
        phdri= (Elf32_Phdr *)(get_native32(&ehdr->e_phoff) + file_image);  // do not free() !!

        int j= get_native16(&ehdr->e_phnum);
        phdr= phdri;
        for (; --j>=0; ++phdr) if (Elf32_Phdr::PT_DYNAMIC==get_native32(&phdr->p_type)) {
            dynseg= (Elf32_Dyn const *)(get_native32(&phdr->p_offset) + file_image);
            break;
        }
        // elf_find_dynamic() returns 0 if 0==dynseg.
        hashtab= (unsigned int const *)elf_find_dynamic(Elf32_Dyn::DT_HASH);
        dynstr=          (char const *)elf_find_dynamic(Elf32_Dyn::DT_STRTAB);
        dynsym=     (Elf32_Sym const *)elf_find_dynamic(Elf32_Dyn::DT_SYMTAB);

        char const *const run_start[]= {
            "__libc_start_main", "__uClibc_main", "__uClibc_start_main",
        };
        for (j=0; j<3; ++j) {
            // elf_lookup() returns 0 if any required table is missing.
            Elf32_Sym const *const lsm = elf_lookup(run_start[j]);
            if (lsm && get_native16(&lsm->st_shndx)==Elf32_Sym::SHN_UNDEF
            && get_native16(&lsm->st_info)==lsm->Elf32_Sym::St_info(Elf32_Sym::STB_GLOBAL, Elf32_Sym::STT_FUNC)
            && get_native16(&lsm->st_other)==Elf32_Sym::STV_DEFAULT ) {
                break;
            }
        }
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
    if (get_native16(&ehdr->e_ehsize) != sizeof(*ehdr)) {
        throwCantPack("invalid Ehdr e_ehsize; try `--force-execve'");
        return false;
    }
    acc_uint64l_t const e_phoff = get_native64(&ehdr->e_phoff);
    if (e_phoff != sizeof(*ehdr)) {// Phdrs not contiguous with Ehdr
        throwCantPack("non-contiguous Ehdr/Phdr; try `--force-execve'");
        return false;
    }

    // The first PT_LOAD64 must cover the beginning of the file (0==p_offset).
    unsigned const e_phnum = get_native16(&ehdr->e_phnum);
    Elf64_Phdr const *phdr = (Elf64_Phdr const *)(buf + e_phoff);
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        if (j >= 14)
            return false;
        if (phdr->PT_LOAD64 == get_native32(&phdr->p_type)) {
            // Just avoid the "rewind" when unpacking?
            //if (phdr->p_offset != 0) {
            //    throwCantPack("invalid Phdr p_offset; try `--force-execve'");
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
        if (PT_LOAD32 == get_native32(&phdr->p_type)) {
            off_t b = get_native32(&phdr->p_vaddr) + get_native32(&phdr->p_memsz);
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
        if (PT_LOAD64 == get_native32(&phdr->p_type)) {
            off_t b = get_native64(&phdr->p_vaddr) + get_native64(&phdr->p_memsz);
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

    assert(get_native32(&h2->ehdr.e_phoff)     == sizeof(Elf32_Ehdr));
                         h2->ehdr.e_shoff = 0;
    assert(get_native16(&h2->ehdr.e_ehsize)    == sizeof(Elf32_Ehdr));
    assert(get_native16(&h2->ehdr.e_phentsize) == sizeof(Elf32_Phdr));
                         h2->ehdr.e_shentsize = 0;
                         h2->ehdr.e_shnum = 0;
                         h2->ehdr.e_shstrndx = 0;

#if 0  //{
    unsigned identsize;
    char const *const ident = getIdentstr(&identsize);
#endif  //}
    sz_elf_hdrs = sizeof(*h2) - sizeof(linfo);  // default
    set_native32(&h2->phdr[0].p_filesz, sizeof(*h2));  // + identsize;
                  h2->phdr[0].p_memsz = h2->phdr[0].p_filesz;

    // Info for OS kernel to set the brk()
    if (brka) {
#define PAGE_MASK (~0u<<12)
        // linux-2.6.14 binfmt_elf.c: SIGKILL if (0==.p_memsz) on a page boundary
        unsigned const brkb = brka | ((0==(~PAGE_MASK & brka)) ? 0x20 : 0);
        set_native32(&h2->phdr[1].p_type, PT_LOAD32);  // be sure
        set_native32(&h2->phdr[1].p_offset, ~PAGE_MASK & brkb);
        set_native32(&h2->phdr[1].p_vaddr, brkb);
        set_native32(&h2->phdr[1].p_paddr, brkb);
        h2->phdr[1].p_filesz = 0;
        h2->phdr[1].p_memsz =  0;
#undef PAGE_MASK
    }
    if (ph.format==getFormat()) {
        assert(2==get_native16(&h2->ehdr.e_phnum));
        set_native32(&h2->phdr[0].p_flags, ~Elf32_Phdr::PF_W & get_native32(&h2->phdr[0].p_flags));
        memset(&h2->linfo, 0, sizeof(h2->linfo));
        fo->write(h2, sizeof(*h2));
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

    assert(get_native32(&h2->ehdr.e_phoff)     == sizeof(Elf64_Ehdr));
                         h2->ehdr.e_shoff = 0;
    assert(get_native16(&h2->ehdr.e_ehsize)    == sizeof(Elf64_Ehdr));
    assert(get_native16(&h2->ehdr.e_phentsize) == sizeof(Elf64_Phdr));
                         h2->ehdr.e_shentsize = 0;
                         h2->ehdr.e_shnum = 0;
                         h2->ehdr.e_shstrndx = 0;

#if 0  //{
    unsigned identsize;
    char const *const ident = getIdentstr(&identsize);
#endif  //}
    sz_elf_hdrs = sizeof(*h2) - sizeof(linfo);  // default
    set_native64(&h2->phdr[0].p_filesz, sizeof(*h2));  // + identsize;
                  h2->phdr[0].p_memsz = h2->phdr[0].p_filesz;

    // Info for OS kernel to set the brk()
    if (brka) {
#define PAGE_MASK (~0ul<<12)
        // linux-2.6.14 binfmt_elf.c: SIGKILL if (0==.p_memsz) on a page boundary
        unsigned const brkb = brka | ((0==(~PAGE_MASK & brka)) ? 0x20 : 0);
        set_native32(&h2->phdr[1].p_type, PT_LOAD32);  // be sure
        set_native64(&h2->phdr[1].p_offset, ~PAGE_MASK & brkb);
        set_native64(&h2->phdr[1].p_vaddr, brkb);
        set_native64(&h2->phdr[1].p_paddr, brkb);
        h2->phdr[1].p_filesz = 0;
        h2->phdr[1].p_memsz =  0;
#undef PAGE_MASK
    }
    if (ph.format==getFormat()) {
        assert(2==get_native16(&h2->ehdr.e_phnum));
        set_native32(&h2->phdr[0].p_flags, ~Elf64_Phdr::PF_W & get_native32(&h2->phdr[0].p_flags));
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
    unsigned const e_phoff = get_native32(&ehdri.e_phoff);
    unsigned const e_phnum = get_native16(&ehdri.e_phnum);
    assert(e_phoff == sizeof(Elf32_Ehdr));  // checked by canPack()
    sz_phdrs = e_phnum * get_native16(&ehdri.e_phentsize);

    phdri = new Elf32_Phdr[e_phnum];
    fi->seek(e_phoff, SEEK_SET);
    fi->readx(phdri, sz_phdrs);

    progid = 0;  // getRandomId();  not useful, so do not clutter
}

void PackLinuxElf32x86::pack1(OutputFile *fo, Filter &ft)
{
    super::pack1(fo, ft);
    generateElfHdr(fo, linux_i386elf_fold, getbrk(phdri, ehdri.e_phnum) );
}

void PackLinuxElf32armLe::pack1(OutputFile *fo, Filter &ft)
{
    super::pack1(fo, ft);

    Elf32_Ehdr const *const fold = (Elf32_Ehdr const *)&linux_elf32arm_fold;
    cprElfHdr3 h3;
    // We need Elf32_Ehdr and Elf32_Phdr with the correct byte gender.
    // The stub may have been compiled differently.
    if (this->ei_data==fold->e_ident[Elf32_Ehdr::EI_DATA]) {
        memcpy(&h3, (Elf_BE32_Ehdr const *)linux_elf32arm_fold,
            sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr) );
    }
    else {
        ehdr_lebe((Elf_LE32_Ehdr *)&h3.ehdr, (Elf_BE32_Ehdr const *)linux_elf32arm_fold);
        brev((unsigned char *)&h3.phdr[0],
            sizeof(Elf32_Ehdr) + (unsigned char const *)&linux_elf32arm_fold,
            3*sizeof(Elf32_Phdr) );
    }
    generateElfHdr(fo, &h3, getbrk(phdri, ehdri.e_phnum) );
}

void PackLinuxElf32armBe::pack1(OutputFile *fo, Filter &ft)  // FIXME
{
    super::pack1(fo, ft);

    Elf32_Ehdr const *const fold = (Elf32_Ehdr const *)&linux_elf32arm_fold;
    cprElfHdr3 h3;
    // We need Elf32_Ehdr and Elf32_Phdr with the correct byte gender.
    // The stub may have been compiled differently.
    if (this->ei_data==fold->e_ident[Elf32_Ehdr::EI_DATA]) {
        memcpy(&h3, (Elf_BE32_Ehdr const *)linux_elf32arm_fold,
            sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr) );
    }
    else {
        ehdr_bele((Elf_BE32_Ehdr *)&h3.ehdr, (Elf_LE32_Ehdr const *)linux_elf32arm_fold);
        brev((unsigned char *)&h3.phdr[0],
            sizeof(Elf32_Ehdr) + (unsigned char const *)&linux_elf32arm_fold,
            3*sizeof(Elf32_Phdr) );
    }
    generateElfHdr(fo, &h3, getbrk(phdri, ehdri.e_phnum) );
}

void PackLinuxElf32ppc::pack1(OutputFile *fo, Filter &ft)
{
    super::pack1(fo, ft);
    generateElfHdr(fo, linux_elfppc32_fold, getbrk(phdri, ehdri.e_phnum) );
}

void PackLinuxElf64::pack1(OutputFile */*fo*/, Filter &/*ft*/)
{
    fi->seek(0, SEEK_SET);
    fi->readx(&ehdri, sizeof(ehdri));
    unsigned const e_phoff = get_native32(&ehdri.e_phoff);
    unsigned const e_phnum = get_native16(&ehdri.e_phnum);
    assert(e_phoff == sizeof(Elf64_Ehdr));  // checked by canPack()
    sz_phdrs = e_phnum * get_native16(&ehdri.e_phentsize);

    phdri = new Elf64_Phdr[e_phnum];
    fi->seek(e_phoff, SEEK_SET);
    fi->readx(phdri, sz_phdrs);

    progid = 0;  // getRandomId();  not useful, so do not clutter
}

void PackLinuxElf64amd::pack1(OutputFile *fo, Filter &ft)
{
    super::pack1(fo, ft);
    generateElfHdr(fo, linux_elf64amd_fold, getbrk(phdri, ehdri.e_phnum) );
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
    if (PT_LOAD32!=get_native32(&phdr[k].p_type)) {
        return 0;
    }
    unsigned const hi = get_native32(&phdr[k].p_offset) +
                        get_native32(&phdr[k].p_filesz);
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
        if (PT_LOAD32==get_native32(&phdr[j].p_type)) {
            unsigned const t = get_native32(&phdr[j].p_offset);
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
    ui_total_passes = 0;
    unsigned const e_phnum = get_native16(&ehdri.e_phnum);
    for (k = 0; k < e_phnum; ++k) {
        if (PT_LOAD32 == get_native32(&phdri[k].p_type)) {
            ui_total_passes++;
            if (find_LOAD_gap(phdri, k, e_phnum)) {
                ui_total_passes++;
            }
        }
    }

    // compress extents
    unsigned total_in = 0;
    unsigned total_out = 0;

    unsigned hdr_u_len = sizeof(Elf32_Ehdr) + sz_phdrs;

    ui_pass = 0;
    ft.addvalue = 0;

    int nx = 0;
    for (k = 0; k < e_phnum; ++k) if (PT_LOAD32==get_native32(&phdri[k].p_type)) {
        if (ft.id < 0x40) {
            // FIXME: ??    ft.addvalue = phdri[k].p_vaddr;
        }
        x.offset = get_native32(&phdri[k].p_offset);
        x.size   = get_native32(&phdri[k].p_filesz);
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
            ((0==nx && (Elf32_Phdr::PF_X & get_native32(&phdri[k].p_flags)))
                ? &ft : 0 ), fo, hdr_u_len);
        hdr_u_len = 0;
        ++nx;
    }
    for (k = 0; k < e_phnum; ++k) {
        x.size = find_LOAD_gap(phdri, k, e_phnum);
        if (x.size) {
            x.offset = get_native32(&phdri[k].p_offset) +
                       get_native32(&phdri[k].p_filesz);
            packExtent(x, total_in, total_out, 0, fo);
        }
    }

    if ((off_t)total_in != file_size)
        throwEOFException();
    set_native32(&elfout.phdr[0].p_filesz, fo->getBytesWritten());
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
    if (PT_LOAD64!=get_native32(&phdr[k].p_type)) {
        return 0;
    }
    unsigned const hi = get_native64(&phdr[k].p_offset) +
                        get_native64(&phdr[k].p_filesz);
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
        if (PT_LOAD64==get_native32(&phdr[j].p_type)) {
            unsigned const t = get_native64(&phdr[j].p_offset);
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
    ui_total_passes = 0;
    unsigned const e_phnum = get_native16(&ehdri.e_phnum);
    for (k = 0; k < e_phnum; ++k) {
        if (PT_LOAD64==get_native32(&phdri[k].p_type)) {
            ui_total_passes++;
            if (find_LOAD_gap(phdri, k, e_phnum)) {
                ui_total_passes++;
            }
        }
    }

    // compress extents
    unsigned total_in = 0;
    unsigned total_out = 0;

    unsigned hdr_u_len = sizeof(Elf64_Ehdr) + sz_phdrs;

    ui_pass = 0;
    ft.addvalue = 0;

    int nx = 0;
    for (k = 0; k < e_phnum; ++k) if (PT_LOAD64==get_native32(&phdri[k].p_type)) {
        if (ft.id < 0x40) {
            // FIXME: ??    ft.addvalue = phdri[k].p_vaddr;
        }
        x.offset = get_native64(&phdri[k].p_offset);
        x.size   = get_native64(&phdri[k].p_filesz);
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
            ((0==nx && (Elf64_Phdr::PF_X & get_native64(&phdri[k].p_flags)))
                ? &ft : 0 ), fo, hdr_u_len);
        hdr_u_len = 0;
        ++nx;
    }
    for (k = 0; k < e_phnum; ++k) {
        x.size = find_LOAD_gap(phdri, k, e_phnum);
        if (x.size) {
            x.offset = get_native64(&phdri[k].p_offset) +
                       get_native64(&phdri[k].p_filesz);
            packExtent(x, total_in, total_out, 0, fo);
        }
    }

    if ((off_t)total_in != file_size)
        throwEOFException();
    set_native64(&elfout.phdr[0].p_filesz, fo->getBytesWritten());
}

void PackLinuxElf32::pack3(OutputFile *fo, Filter &ft)
{
    unsigned disp;
    unsigned const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 3& -len);  // align to 0 mod 4
    len += (3& -len);
    set_native32(&disp, len);  // FIXME?  -(sz_elf_hdrs+sizeof(l_info)+sizeof(p_info))
    fo->write(&disp, sizeof(disp));

    super::pack3(fo, ft);
}

void PackLinuxElf64amd::pack3(OutputFile *fo, Filter &ft)
{
    char zero[8];
    unsigned const hlen = sz_elf_hdrs + sizeof(l_info) + sizeof(p_info);
    unsigned const len0 = fo->getBytesWritten();
    unsigned len = len0;
    unsigned const frag = 7 & -len; // align to 0 mod 8
    memset(zero, 0, sizeof(zero));
    fo->write(&zero, frag);
    len += frag;

#define PAGE_MASK (~0u<<12)
#define PAGE_SIZE (-PAGE_MASK)
    upx_byte *const p = const_cast<upx_byte *>(getLoader());
    lsize = getLoaderSize();
    acc_uint64l_t const lo_va_user = 0x400000;  // XXX
    acc_uint64l_t       lo_va_stub = elfout.phdr[0].p_vaddr;
    acc_uint64l_t adrc;
    acc_uint64l_t adrm;
    acc_uint64l_t adru;
    acc_uint64l_t adrx;
    unsigned cntc;
    unsigned lenm;
    unsigned lenu;
    len += (7&-lsize) + lsize;
    bool const is_big = (lo_va_user < (lo_va_stub + len + 2*PAGE_SIZE));
    if (is_big) {
        elfout.ehdr.e_entry += lo_va_user - lo_va_stub;
        elfout.phdr[0].p_vaddr = lo_va_user;
        elfout.phdr[0].p_paddr = lo_va_user;
               lo_va_stub      = lo_va_user;
        adrc = lo_va_stub;
        adrm = getbrk(phdri, ehdri.e_phnum);
        adru = PAGE_MASK & (~PAGE_MASK + adrm);  // round up to page boundary
        adrx = adru + hlen;
        lenm = PAGE_SIZE + len;
        lenu = PAGE_SIZE + len;
        cntc = len >> 3;
    }
    else {
        adrm = lo_va_stub + len;
        adrc = adrm;
        adru = lo_va_stub;
        adrx = lo_va_stub + hlen;
        lenm = PAGE_SIZE;
        lenu = PAGE_SIZE + len;
        cntc = 0;
    }
    adrm = PAGE_MASK & (~PAGE_MASK + adrm);  // round up to page boundary
    adrc = PAGE_MASK & (~PAGE_MASK + adrc);  // round up to page boundary

    // patch in order of descending address
    patch_le32(p,lsize,"LENX", len0 - hlen);
    patch_le32(p,lsize,"ADRX", adrx); // compressed input for eXpansion

    patch_le32(p,lsize,"CNTC", cntc);  // count  for copy
    patch_le32(p,lsize,"LENU", lenu);  // len  for unmap
    patch_le32(p,lsize,"ADRC", adrc);  // addr for copy
    patch_le32(p,lsize,"ADRU", adru);  // addr for unmap
    patch_le32(p,lsize,"JMPU", 12 + lo_va_user);  // trampoline for unmap
    patch_le32(p,lsize,"LENM", lenm);  // len  for map
    patch_le32(p,lsize,"ADRM", adrm);  // addr for map
#undef PAGE_SIZE
#undef PAGE_MASK

    super::pack3(fo, ft);
}

#include "bele.h"
using namespace NBELE;

void PackLinuxElf32::ARM_pack3(OutputFile *fo, Filter &ft, bool isBE)
{
    unsigned const hlen = sz_elf_hdrs + sizeof(l_info) + sizeof(p_info);
    unsigned const len0 = fo->getBytesWritten();
    unsigned len = len0;
    unsigned const zero = 0;
    fo->write(&zero, 3& -len);  // align to 0 mod 4
    len += (3& -len);

#define PAGE_MASK (~0u<<12)
#define PAGE_SIZE (-PAGE_MASK)
    upx_byte *const p = const_cast<upx_byte *>(getLoader());
    lsize = getLoaderSize();
    unsigned const lo_va_user = 0x8000;  // XXX
    unsigned lo_va_stub = get_native32(&elfout.phdr[0].p_vaddr);
    unsigned adrc;
    unsigned adrm;
    unsigned adrx;
    unsigned cntc;
    unsigned lenm;

    len += lsize;
    bool const is_big = true;
    if (is_big) {
        set_native32(    &elfout.ehdr.e_entry,
            get_native32(&elfout.ehdr.e_entry) + lo_va_user - lo_va_stub);
        set_native32(&elfout.phdr[0].p_vaddr, lo_va_user);
        set_native32(&elfout.phdr[0].p_paddr, lo_va_user);
                              lo_va_stub    = lo_va_user;
        adrc = lo_va_stub;
        adrm = getbrk(phdri, get_native16(&ehdri.e_phnum));
        adrx = hlen + (PAGE_MASK & (~PAGE_MASK + adrm));  // round up to page boundary
        lenm = PAGE_SIZE + len;
        cntc = len >> 5;
    }
    else {
        adrm = lo_va_stub + len;
        adrc = adrm;
        adrx = lo_va_stub + hlen;
        lenm = PAGE_SIZE;
        cntc = 0;
    }
    adrm = PAGE_MASK & (~PAGE_MASK + adrm);  // round up to page boundary
    adrc = PAGE_MASK & (~PAGE_MASK + adrc);  // round up to page boundary

    // Patch in order of descending address.
    //
    // Because Packer::patch_be32 is overloaded, it is impossible
    // to choose the right one to initialize a pointer to function.
    // Therefore, write either patch_be32 or patch_le32 literally.
    //
    // ::ARM_buildLoader() put the stub into native order.
    // util.c::find() uses host order.
    int const swap = (HostPolicy::isBE ^ isBE);
    if (isBE) {
        patch_be32(p,lsize, 4*swap + "ADRXXRDA", adrx); // compressed input for eXpansion
        patch_be32(p,lsize, 4*swap + "LENXXNEL", len0 - hlen);

        patch_be32(p,lsize, 4*swap + "CNTCCTNC", cntc);  // count  for copy
        patch_be32(p,lsize, 4*swap + "ADRCCRDA", adrc);  // addr for copy

        patch_be32(p,lsize, 4*swap + "LENMMNEL", lenm);  // len  for map
        patch_be32(p,lsize, 4*swap + "ADRMMRDA", adrm);  // addr for map
    }
    else {
        patch_le32(p,lsize, 4*swap + "ADRXXRDA", adrx); // compressed input for eXpansion
        patch_le32(p,lsize, 4*swap + "LENXXNEL", len0 - hlen);

        patch_le32(p,lsize, 4*swap + "CNTCCTNC", cntc);  // count  for copy
        patch_le32(p,lsize, 4*swap + "ADRCCRDA", adrc);  // addr for copy

        patch_le32(p,lsize, 4*swap + "LENMMNEL", lenm);  // len  for map
        patch_le32(p,lsize, 4*swap + "ADRMMRDA", adrm);  // addr for map
    }
#undef PAGE_SIZE
#undef PAGE_MASK

    super::pack3(fo, ft);
}

void PackLinuxElf32armLe::pack3(OutputFile *fo, Filter &ft)
{
    ARM_pack3(fo, ft, false);
}

void PackLinuxElf32armBe::pack3(OutputFile *fo, Filter &ft)
{
    ARM_pack3(fo, ft, true);
}

void PackLinuxElf::pack4(OutputFile *fo, Filter &ft)
{
    super::pack4(fo, ft);
}

void PackLinuxElf32::pack4(OutputFile *fo, Filter &ft)
{
    overlay_offset = sz_elf_hdrs + sizeof(linfo);
    unsigned const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 3& -len);  // align to 0 mod 4
    len += 3& -len;
    set_native32(&elfout.phdr[0].p_filesz, len);
    super::pack4(fo, ft);  // write PackHeader and overlay_offset

    // Cannot pre-round .p_memsz.  If .p_filesz < .p_memsz, then kernel
    // tries to make .bss, which requires PF_W.
    // But strict SELinux (or PaX, grSecurity) disallows PF_W with PF_X.
#if 0  /*{*/
#define PAGE_MASK (~0u<<12)
    // pre-calculate for benefit of runtime disappearing act via munmap()
    set_native32(&elfout.phdr[0].p_memsz, PAGE_MASK & (~PAGE_MASK + len));
#undef PAGE_MASK
#else  /*}{*/
    set_native32(&elfout.phdr[0].p_memsz, len);
#endif  /*}*/

    // rewrite Elf header
    if (Elf32_Ehdr::ET_DYN==ehdri.e_type) {
        unsigned const base= elfout.phdr[0].p_vaddr;
        elfout.ehdr.e_type= Elf32_Ehdr::ET_DYN;
        elfout.ehdr.e_phnum= 1;
        elfout.ehdr.e_entry    -= base;
        elfout.phdr[0].p_vaddr -= base;
        elfout.phdr[0].p_paddr -= base;
        // Strict SELinux (or PaX, grSecurity) disallows PF_W with PF_X
        //elfout.phdr[0].p_flags |= Elf32_Phdr::PF_W;
    }
    fo->seek(0, SEEK_SET);
    fo->rewrite(&elfout, sz_elf_hdrs);
    fo->rewrite(&linfo, sizeof(linfo));
}

void PackLinuxElf64::pack4(OutputFile *fo, Filter &ft)
{
    overlay_offset = sz_elf_hdrs + sizeof(linfo);
    unsigned const zero = 0;
    unsigned len = fo->getBytesWritten();
    fo->write(&zero, 3& -len);  // align to 0 mod 4
    len += 3& -len;
    set_native64(&elfout.phdr[0].p_filesz, len);
    super::pack4(fo, ft);  // write PackHeader and overlay_offset

    // Cannot pre-round .p_memsz.  If .p_filesz < .p_memsz, then kernel
    // tries to make .bss, which requires PF_W.
    // But strict SELinux (or PaX, grSecurity) disallows PF_W with PF_X.
#if 0  /*{*/
#define PAGE_MASK (~0u<<12)
    // pre-calculate for benefit of runtime disappearing act via munmap()
    set_native64(&elfout.phdr[0].p_memsz, PAGE_MASK & (~PAGE_MASK + len));
#undef PAGE_MASK
#else  /*}{*/
    set_native64(&elfout.phdr[0].p_memsz, len);
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
        unsigned const e_entry = get_native32(&ehdr->e_entry);
        if (e_entry < 0x401180
        &&  ehdr->e_machine==Elf32_Ehdr::EM_386) { /* old style, 8-byte b_info */
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

    // decompress PT_LOAD32
    bool first_PF_X = true;
    unsigned const phnum = get_native16(&ehdr->e_phnum);
    fi->seek(- (off_t) (szb_info + ph.c_len), SEEK_CUR);
    for (unsigned j=0; j < phnum; ++phdr, ++j) {
        if (PT_LOAD32==get_native32(&phdr->p_type)) {
            unsigned const filesz = get_native32(&phdr->p_filesz);
            unsigned const offset = get_native32(&phdr->p_offset);
            if (fo)
                fo->seek(offset, SEEK_SET);
            if (Elf32_Phdr::PF_X & get_native32(&phdr->p_flags)) {
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
            unsigned const where = get_native32(&phdr[j].p_offset) +
                                   get_native32(&phdr[j].p_filesz);
            if (fo)
                fo->seek(where, SEEK_SET);
            unpackExtent(size, fo, total_in, total_out,
                c_adler, u_adler, false, szb_info);
        }
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
        acc_uint64l_t const e_entry = get_native64(&ehdr->e_entry);
        if (e_entry < 0x401180
        &&  ehdr->e_machine==Elf64_Ehdr::EM_386) { /* old style, 8-byte b_info */
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

    // decompress PT_LOAD32
    bool first_PF_X = true;
    unsigned const phnum = get_native16(&ehdr->e_phnum);
    fi->seek(- (off_t) (szb_info + ph.c_len), SEEK_CUR);
    for (unsigned j=0; j < phnum; ++phdr, ++j) {
        if (PT_LOAD32==get_native32(&phdr->p_type)) {
            acc_uint64l_t const filesz = get_native64(&phdr->p_filesz);
            acc_uint64l_t const offset = get_native64(&phdr->p_offset);
            if (fo)
                fo->seek(offset, SEEK_SET);
            if (Elf64_Phdr::PF_X & get_native32(&phdr->p_flags)) {
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
            unsigned const where = get_native64(&phdr[j].p_offset) +
                                   get_native64(&phdr[j].p_filesz);
            if (fo)
                fo->seek(where, SEEK_SET);
            unpackExtent(size, fo, total_in, total_out,
                c_adler, u_adler, false, szb_info);
        }
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

PackLinuxElf32x86::PackLinuxElf32x86(InputFile *f) : super(f)
{
    e_machine = Elf32_Ehdr::EM_386;
    ei_class  = Elf32_Ehdr::ELFCLASS32;
    ei_data   = Elf32_Ehdr::ELFDATA2LSB;
}

PackLinuxElf32x86::~PackLinuxElf32x86()
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
    -1 };
    return filters;
}

PackLinuxElf32armLe::PackLinuxElf32armLe(InputFile *f) : super(f)
{
    e_machine = Elf32_Ehdr::EM_ARM;
    ei_class  = Elf32_Ehdr::ELFCLASS32;
    ei_data   = Elf32_Ehdr::ELFDATA2LSB;
}

PackLinuxElf32armLe::~PackLinuxElf32armLe()
{
}

PackLinuxElf32armBe::PackLinuxElf32armBe(InputFile *f) : super(f)
{
    e_machine = Elf32_Ehdr::EM_ARM;
    ei_class  = Elf32_Ehdr::ELFCLASS32;
    ei_data   = Elf32_Ehdr::ELFDATA2MSB;
}

PackLinuxElf32armBe::~PackLinuxElf32armBe()
{
}

const int *
PackLinuxElf32armLe::getFilters() const
{
    static const int filters[] = { 0x50, -1 };
    return filters;
}

const int *
PackLinuxElf32armBe::getFilters() const
{
    static const int filters[] = { 0x50, -1 };
    return filters;
}

unsigned
PackLinuxElf32::elf_get_offset_from_address(unsigned const addr) const
{
    Elf32_Phdr const *phdr = phdri;
    int j = ehdri.e_phnum;
    for (; --j>=0; ++phdr) if (PT_LOAD32 == get_native32(&phdr->p_type)) {
        unsigned const t = addr - get_native32(&phdr->p_vaddr);
        if (t < get_native32(&phdr->p_filesz)) {
            return t + get_native32(&phdr->p_offset);
        }
    }
    return 0;
}

void const *
PackLinuxElf32::elf_find_dynamic(unsigned int const key) const
{
    Elf32_Dyn const *dynp= dynseg;
    if (dynp)
    for (; Elf32_Dyn::DT_NULL!=dynp->d_tag; ++dynp) if (get_native32(&dynp->d_tag)==key) {
        unsigned const t= elf_get_offset_from_address(get_native32(&dynp->d_val));
        if (t) {
            return t + file_image;
        }
        break;
    }
    return 0;
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
        unsigned const nbucket = get_native32(&hashtab[0]);
        unsigned const *const buckets = &hashtab[2];
        unsigned const *const chains = &buckets[nbucket];
        unsigned const m = elf_hash(name) % nbucket;
        unsigned si;
        for (si= get_native32(&buckets[m]); 0!=si; si= get_native32(&chains[si])) {
            char const *const p= dynsym[si].st_name + dynstr;
            if (0==strcmp(name, p)) {
                return &dynsym[si];
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
        unsigned const e_entry = get_native32(&ehdr->e_entry);
        unsigned const e_type = get_native16(&ehdr->e_type);
        if (e_entry < 0x401180 && Elf32_Ehdr::ET_EXEC==e_type) {
            // beware ET_DYN.e_entry==0x10f0 or so
            /* old style, 8-byte b_info */
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

    // decompress PT_LOAD32
    bool first_PF_X = true;
    fi->seek(- (off_t) (szb_info + ph.c_len), SEEK_CUR);
    unsigned const phnum = get_native16(&ehdr->e_phnum);
    for (unsigned j=0; j < phnum; ++phdr, ++j) {
        if (PT_LOAD32==get_native32(&phdr->p_type)) {
            if (fo)
                fo->seek(get_native32(&phdr->p_offset), SEEK_SET);
            if (Elf32_Phdr::PF_X & phdr->p_flags) {
                unpackExtent(get_native32(&phdr->p_filesz), fo, total_in, total_out,
                    c_adler, u_adler, first_PF_X, szb_info);
                first_PF_X = false;
            }
            else {
                unpackExtent(get_native32(&phdr->p_filesz), fo, total_in, total_out,
                    c_adler, u_adler, false, szb_info);
            }
        }
    }

    phdr = (Elf32_Phdr *)(1+ehdr);
    for (unsigned j = 0; j < phnum; ++j) {
        unsigned const size = find_LOAD_gap(phdr, j, phnum);
        if (size) {
            unsigned const where = get_native32(&phdr[j].p_offset) +
                                   get_native32(&phdr[j].p_filesz);
            if (fo)
                fo->seek(where, SEEK_SET);
            unpackExtent(size, fo, total_in, total_out,
                c_adler, u_adler, false, szb_info);
        }
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

