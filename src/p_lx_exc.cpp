/* p_lx_exc.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2001 Laszlo Molnar
   Copyright (C) 2001 John F. Reiser
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

#define PT_LOAD     Elf_LE32_Phdr::PT_LOAD


/*************************************************************************
// linux/386 (generic "execve" format)
**************************************************************************/

static const
#include "stub/l_lx_exec86.h"
static const
#include "stub/fold_exec86.h"


const int *PackLinuxI386::getCompressionMethods(int method, int level) const
{
    static const int m_nrv2b[] = { M_NRV2B_LE32, M_NRV2D_LE32, -1 };
    static const int m_nrv2d[] = { M_NRV2D_LE32, M_NRV2B_LE32, -1 };

    if (M_IS_NRV2B(method))
        return m_nrv2b;
    if (M_IS_NRV2D(method))
        return m_nrv2d;
    if (level == 1 || file_size <= 512*1024)
        return m_nrv2b;
    return m_nrv2d;
}

int const *
PackLinuxI386::getFilters() const
{
    static const int filters[] = {
        0x49, 0x46,
        0x83, 0x86, 0x80, 0x84, 0x87, 0x81, 0x82, 0x85,
        0x24, 0x16, 0x13, 0x14, 0x11, 0x25, 0x15, 0x12,
    -1 };
    return filters;
}

struct cprElfhdr {
    struct Elf_LE32_Ehdr ehdr;
    struct Elf_LE32_Phdr phdr[2];
    struct PackUnix::l_info linfo;
};

int
PackLinuxI386::buildLinuxLoader(
    upx_byte const *const proto,
    unsigned        const szproto,
    upx_byte const *const fold,
    unsigned        const szfold,
    Filter const *ft,
    unsigned const brka
)
{
    initLoader(proto, szproto);

    struct cprElfhdr elfhdr = *(struct cprElfhdr const *)fold;
    memset(&elfhdr.linfo, 0, sizeof(elfhdr.linfo));

    // Info for OS kernel to set the brk()
    if (brka) {
        assert(ph.format==UPX_F_LINUX_ELF_i386
        ||     ph.format==UPX_F_LINUX_SH_i386 );
        if (ph.format==UPX_F_LINUX_SH_i386) {
            assert(elfhdr.ehdr.e_phnum==1);
            elfhdr.ehdr.e_phnum = 2;
        }
        elfhdr.phdr[1].p_offset = 0xfff&brka;
        elfhdr.phdr[1].p_vaddr = brka;
        elfhdr.phdr[1].p_paddr = brka;
        elfhdr.phdr[1].p_filesz = 0;
        elfhdr.phdr[1].p_memsz =  0;
    }

    // The beginning of our loader consists of a elf_hdr (52 bytes) and
    // two sections elf_phdr (2 * 32 byte), so we have 12 free bytes
    // from offset 116 to the program start at offset 128.
    assert(elfhdr.ehdr.e_phoff     == sizeof(Elf_LE32_Ehdr));
    assert(elfhdr.ehdr.e_shoff     == 0);
    assert(elfhdr.ehdr.e_ehsize    == sizeof(Elf_LE32_Ehdr));
    assert(elfhdr.ehdr.e_phentsize == sizeof(Elf_LE32_Phdr));
    assert(elfhdr.ehdr.e_phnum     == (unsigned)(1+ (0!=brka)));
    assert(elfhdr.ehdr.e_shnum     == 0);
    linker->addSection("ELFHEADX", (unsigned char const *)&elfhdr, sizeof(elfhdr));
    addLoader("ELFHEADX", 0);

    struct {
        upx_uint sz_unc;  // uncompressed
        upx_uint sz_cpr;  //   compressed
    } h = {
        szfold - sizeof(elfhdr), 0
    };
    unsigned char const *const uncLoader = (unsigned char const *)(1+
        (struct cprElfhdr const *)fold );
    unsigned char *const cprLoader = new unsigned char[sizeof(h) + h.sz_unc];

    int r = upx_compress(uncLoader, h.sz_unc, sizeof(h) + cprLoader, &h.sz_cpr,
        NULL, ph.method, 10, NULL, NULL );
    if (r != UPX_E_OK || h.sz_cpr >= h.sz_unc)
        throwInternalError("loader compression failed");
    memcpy(cprLoader, &h, sizeof(h));
    linker->addSection("FOLDEXEC", cprLoader, sizeof(h) + h.sz_cpr);
    delete cprLoader;

    n_mru = ft->n_mru;

    addLoader("IDENTSTR", 0);
    addLoader("LEXEC000", 0);

    if (ft->id) {
        if (ph.format==UPX_F_LINUX_ELF_i386) { // decompr, unfilter are separate
            addLoader("LXUNF000", 0);
            addLoader("LXUNF002", 0);
	        if (0x80==(ft->id & 0xF0)) {
	            if (256==n_mru) {
	                addLoader("MRUBYTE0", 0);
	            }
	            else if (n_mru) {
	                addLoader("LXMRU005", 0);
	            }
	            if (n_mru) {
	                addLoader("LXMRU006", 0);
	            }
	            else {
	                addLoader("LXMRU007", 0);
	            }
            }
            else if (0x40==(ft->id & 0xF0)) {
                addLoader("LXUNF008", 0);
            }
            addLoader("LXUNF010", 0);
        }
        if (n_mru) {
            addLoader("LEXEC009", 0);
        }
    }
    addLoader("LEXEC010", 0);
    addLoader(getDecompressor(), 0);
    addLoader("LEXEC015", 0);
    if (ft->id) {
        if (ph.format==UPX_F_LINUX_ELF_i386) {  // decompr, unfilter are separate
            if (0x80!=(ft->id & 0xF0)) {
                addLoader("LXUNF042", 0);
            }
        }
        else {  // decompr, unfilter not separate
            if (0x80==(ft->id & 0xF0)) {
                addLoader("LEXEC110", 0);
                if (n_mru) {
                    addLoader("LEXEC100", 0);
                }
                // bug in APP: jmp and label must be in same .asx/.asy
                addLoader("LEXEC016", 0);
            }
        }
        addFilter32(ft->id);
        if (ph.format==UPX_F_LINUX_ELF_i386) { // decompr, unfilter are separate
            if (0x80==(ft->id & 0xF0)) {
                if (0==n_mru) {
                    addLoader("LXMRU058", 0);
                }
            }
            addLoader("LXUNF035", 0);
        }
        else {  // decompr always unfilters
            addLoader("LEXEC017", 0);
        }
    }

    addLoader("LEXEC020", 0);
    addLoader("FOLDEXEC", 0);

    struct Elf_LE32_Ehdr *const ldrEhdr =
        (struct Elf_LE32_Ehdr *)const_cast<unsigned char *>(getLoader());
    unsigned e_entry = getLoaderSectionStart("LEXEC000");
    ldrEhdr->e_entry = e_entry + elfhdr.phdr[0].p_vaddr;

    char *ptr_cto = e_entry + (char *)ldrEhdr;
    int sz_cto = getLoaderSize() - e_entry;
    if (0x20==(ft->id & 0xF0) || 0x30==(ft->id & 0xF0)) {  // push byte '?'  ; cto8
        patch_le16(ptr_cto, sz_cto, "\x6a?", 0x6a + (ft->cto << 8));
    }

    // FIXME ??    patchVersion((char *)ldrEhdr, e_entry);
    return getLoaderSize();
}

int
PackLinuxI386::buildLoader(Filter const *ft)
{
    unsigned const sz_fold = sizeof(linux_i386exec_fold);
    MemBuffer buf(sz_fold);
    memcpy(buf, linux_i386exec_fold, sz_fold);

    // patch loader
    // note: we only can use /proc/<pid>/fd when exetype > 0.
    //   also, we sleep much longer when compressing a script.
    checkPatch(0,0,0,0);  // reset
    patch_le32(buf,sz_fold,"UPX4",exetype > 0 ? 3 : 15);   // sleep time
    patch_le32(buf,sz_fold,"UPX3",progid);
    patch_le32(buf,sz_fold,"UPX2",exetype > 0 ? 0 : 0x7fffffff);

    // get fresh filter
    Filter fold_ft = *ft;
    fold_ft.init(ft->id, ft->addvalue);
    int preferred_ctos[2] = {ft->cto, -1};
    fold_ft.preferred_ctos = preferred_ctos;

    // filter
    optimizeFilter(&fold_ft, buf, sz_fold);
    bool success = fold_ft.filter(buf + sizeof(cprElfhdr), sz_fold - sizeof(cprElfhdr));
    (void)success;

    return buildLinuxLoader(
        linux_i386exec_loader, sizeof(linux_i386exec_loader),
        buf, sz_fold, ft, 0 );
}

int PackLinuxI386::getLoaderPrefixSize() const
{
    return 116;
}


/*************************************************************************
// some ELF utitlity functions
**************************************************************************/

// basic check of an Linux ELF Ehdr
int PackLinuxI386::checkEhdr(const Elf_LE32_Ehdr *ehdr) const
{
    const unsigned char * const buf = ehdr->e_ident;

    if (memcmp(buf, "\x7f\x45\x4c\x46\x01\x01\x01", 7)) // ELF 32-bit LSB
        return -1;

    // now check the ELF header
    if (!memcmp(buf+8, "FreeBSD", 7))                   // branded
        return 1;
    if (ehdr->e_type != 2)                              // executable
        return 2;
    if (ehdr->e_machine != 3)                           // Intel 80386
        return 3;
    if (ehdr->e_version != 1)                           // version
        return 4;
    if (ehdr->e_phnum < 1)
        return 5;
    if (ehdr->e_phentsize != sizeof(Elf_LE32_Phdr))
        return 6;

    // check for Linux kernels
    if (ehdr->e_entry == 0xC0100000)                    // uncompressed vmlinux
        return 1000;
    if (ehdr->e_entry == 0x00001000)                    // compressed vmlinux
        return 1001;
    if (ehdr->e_entry == 0x00100000)                    // compressed bvmlinux
        return 1002;

    // FIXME: add more checks for kernels

    // FIXME: add special checks for other ELF i386 formats, like
    //        NetBSD, OpenBSD, Solaris, ....

    // success
    return 0;
}


off_t PackLinuxI386::getbrk(const Elf_LE32_Phdr *phdr, int e_phnum) const
{
    off_t brka = 0;
    for (int j = 0; j < e_phnum; ++phdr, ++j) {
        if (phdr->PT_LOAD == phdr->p_type) {
            off_t b = phdr->p_vaddr + phdr->p_memsz;
            if (b > brka)
                brka = b;
        }
    }
    return brka;
}


/*************************************************************************
//
**************************************************************************/

bool PackLinuxI386::canPack()
{
    if (exetype != 0)
        return super::canPack();

    Elf_LE32_Ehdr ehdr;
    unsigned char *buf = ehdr.e_ident;

    fi->readx(&ehdr, sizeof(ehdr));
    fi->seek(0, SEEK_SET);

    exetype = 0;
    const unsigned l = get_le32(buf);

    int elf = checkEhdr(&ehdr);
    if (elf >= 0)
    {
        // NOTE: ELF executables are now handled by p_lx_elf.cpp,
        //   so we only handle them here if force_execve
        if (elf == 0 && opt->unix.force_execve)
            exetype = 1;
    }
    else if (l == 0x00640107 || l == 0x00640108 || l == 0x0064010b || l == 0x006400cc)
    {
        // OMAGIC / NMAGIC / ZMAGIC / QMAGIC
        exetype = 2;
        // FIXME: N_TRSIZE, N_DRSIZE
        // FIXME: check for aout shared libraries
    }
#if defined(__linux__)
    // only compress scripts when running under Linux
    else if (!memcmp(buf, "#!/", 3))                    // #!/bin/sh
        exetype = -1;
    else if (!memcmp(buf, "#! /", 4))                   // #! /bin/sh
        exetype = -1;
    else if (!memcmp(buf, "\xca\xfe\xba\xbe", 4))       // Java bytecode
        exetype = -2;
#endif

    return super::canPack();
}


void PackLinuxI386::patchLoader()
{
}


void PackLinuxI386::patchLoaderChecksum()
{
    unsigned char *const ptr = const_cast<unsigned char *>(getLoader());
    l_info *const lp = (l_info *)(ptr + getLoaderPrefixSize());
    // checksum for loader + p_info
    lp->l_checksum = 0;
    lp->l_magic = UPX_ELF_MAGIC;
    lp->l_lsize = (unsigned short) lsize;
    lp->l_version = (unsigned char) ph.version;
    lp->l_format  = (unsigned char) ph.format;
    // INFO: lp->l_checksum is currently unused
    unsigned adler = upx_adler32(0,NULL,0);
    adler = upx_adler32(adler, ptr, lsize + sizeof(p_info));
    lp->l_checksum = adler;
}


void PackLinuxI386::updateLoader(OutputFile *fo)
{
#define PAGE_MASK (~0<<12)
    Elf_LE32_Ehdr *const ehdr = (Elf_LE32_Ehdr *)const_cast<upx_byte *>(getLoader());
    Elf_LE32_Phdr *const phdr = (Elf_LE32_Phdr *)(1+ehdr);

    ehdr->e_phnum = 2;
    phdr->p_filesz = lsize;
    // phdr->p_memsz is the decompressed size
    assert(lsize > 128 && lsize < 4096);

    // The first Phdr maps the stub (instructions, data, bss) rwx.
    // The second Phdr maps the overlay r--,
    // to defend against /usr/bin/strip removing the overlay.
    Elf_LE32_Phdr *const phdro = 1+phdr;

    phdro->p_type = PT_LOAD;
    phdro->p_offset = lsize;
    phdro->p_paddr = phdro->p_vaddr = 0x00400000 + (lsize &~ PAGE_MASK);
    phdro->p_memsz = phdro->p_filesz = fo->getBytesWritten() - lsize;
    phdro->p_flags = phdro->PF_R;
    phdro->p_align = (unsigned) (-PAGE_MASK);

    patchLoaderChecksum();
#undef PAGE_MASK
}


/*
vi:ts=4:et
*/

