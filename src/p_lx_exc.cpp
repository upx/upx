/* p_lx_exc.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2002 Laszlo Molnar
   Copyright (C) 2001-2002 John F. Reiser
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
#define PT_DYNAMIC  Elf_LE32_Phdr::PT_DYNAMIC
#define DT_NULL     Elf_LE32_Dyn::DT_NULL
#define DT_NEEDED   Elf_LE32_Dyn::DT_NEEDED
#define DT_STRTAB   Elf_LE32_Dyn::DT_STRTAB
#define DT_STRSZ    Elf_LE32_Dyn::DT_STRSZ


/*************************************************************************
// linux/386 (generic "execve" format)
**************************************************************************/

static const
#include "stub/l_lx_exec86.h"
static const
#include "stub/fold_exec86.h"


const int *PackLinuxI386::getCompressionMethods(int method, int level) const
{
    return Packer::getDefaultCompressionMethods_le32(method, level);
}

const int *PackLinuxI386::getFilters() const
{
    static const int filters[] = {
#if 1
        0x26, 0x24, 0x11, 0x14, 0x13, 0x16, 0x25, 0x15, 0x12,
#else
        0x49, 0x46,
        0x83, 0x86, 0x80, 0x84, 0x87, 0x81, 0x82, 0x85,
        0x24, 0x16, 0x13, 0x14, 0x11, 0x25, 0x15, 0x12,
#endif
    -1 };
    return filters;
}

void
PackLinuxI386::generateElfHdr(
    OutputFile *const fo,
    void const *const proto,
    unsigned const brka
)
{
    cprElfHdr1 *const h1 = (cprElfHdr1 *)&elfout;
    cprElfHdr2 *const h2 = (cprElfHdr2 *)&elfout;
    memcpy(h2, proto, sizeof(*h2));

    assert(h2->ehdr.e_phoff     == sizeof(Elf_LE32_Ehdr));
    assert(h2->ehdr.e_shoff     == 0);
    assert(h2->ehdr.e_ehsize    == sizeof(Elf_LE32_Ehdr));
    assert(h2->ehdr.e_phentsize == sizeof(Elf_LE32_Phdr));
    assert(h2->ehdr.e_shnum     == 0);

#if 0  //{
    unsigned identsize;
    char const *const ident = getIdentstr(&identsize);
#endif  //}
    h2->phdr[0].p_filesz = sizeof(*h2);  // + identsize;
    h2->phdr[0].p_memsz  = h2->phdr[0].p_filesz;

    // Info for OS kernel to set the brk()
    if (brka) {
        h2->phdr[1].p_type = PT_LOAD;  // be sure
        h2->phdr[1].p_offset = 0xfff&brka;
        h2->phdr[1].p_vaddr = brka;
        h2->phdr[1].p_paddr = brka;
        h2->phdr[1].p_filesz = 0;
        h2->phdr[1].p_memsz =  0;
    }

    if (ph.format==UPX_F_LINUX_i386 ) {
        assert(h1->ehdr.e_phnum==1);
        memset(&h1->linfo, 0, sizeof(h1->linfo));
        fo->write(h1, sizeof(*h1));
    }
    else if (ph.format==UPX_F_LINUX_ELF_i386) {
        assert(h2->ehdr.e_phnum==2);
        memset(&h2->linfo, 0, sizeof(h2->linfo));
        fo->write(h2, sizeof(*h2));
    }
    else if (ph.format==UPX_F_LINUX_SH_i386) {
        assert(h2->ehdr.e_phnum==1);
        h2->ehdr.e_phnum = 2;
        memset(&h2->linfo, 0, sizeof(h2->linfo));
        fo->write(h2, sizeof(*h2));
    }
    else {
        assert(false);  // unknown ph.format, PackUnix::generateElfHdr
    }
}

void
PackLinuxI386::pack1(OutputFile *fo, Filter &)
{
    // create a pseudo-unique program id for our paranoid stub
    progid = getRandomId();

    generateElfHdr(fo, linux_i386exec_fold, 0);
}

void
PackLinuxI386::pack4(OutputFile *fo, Filter &ft)
{
    overlay_offset = sizeof(elfout.ehdr) +
        (elfout.ehdr.e_phentsize * elfout.ehdr.e_phnum) +
        sizeof(l_info) +
        ((elfout.ehdr.e_phnum==3) ? elfout.phdr[2].p_memsz : 0) ;
    super::pack4(fo, ft);  // write PackHeader and overlay_offset

    elfout.phdr[0].p_filesz = fo->getBytesWritten();

#define PAGE_MASK (~0<<12)
    // pre-calculate for benefit of runtime disappearing act via munmap()
    elfout.phdr[0].p_memsz =  PAGE_MASK & (~PAGE_MASK + elfout.phdr[0].p_filesz);
#undef PAGE_MASK

    // rewrite Elf header
    fo->seek(0, SEEK_SET);
    fo->rewrite(&elfout, sizeof(elfout.ehdr) +
        elfout.ehdr.e_phnum * sizeof(elfout.phdr[0]) +
        sizeof(l_info) );
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
PackLinuxI386::buildLinuxLoader(
    upx_byte const *const proto,
    unsigned        const szproto,
    upx_byte const *const fold,
    unsigned        const szfold,
    Filter const *ft
)
{
    initLoader(proto, szproto);

    cprElfHdr1 const *const hf = (cprElfHdr1 const *)fold;
    unsigned const fold_hdrlen = umax(0x80, sizeof(hf->ehdr) +
        hf->ehdr.e_phentsize * hf->ehdr.e_phnum + sizeof(l_info) );
    struct b_info h; memset(&h, 0, sizeof(h));
    h.sz_unc = szfold - fold_hdrlen;
    h.b_method = ph.method;
    h.b_ftid = ph.filter;
    h.b_cto8 = ph.filter_cto;
    unsigned char const *const uncLoader = fold_hdrlen + fold;

    unsigned char *const cprLoader = new unsigned char[sizeof(h) + h.sz_unc];
    int r = upx_compress(uncLoader, h.sz_unc, sizeof(h) + cprLoader, &h.sz_cpr,
        NULL, ph.method, 10, NULL, NULL );
    if (r != UPX_E_OK || h.sz_cpr >= h.sz_unc)
        throwInternalError("loader compression failed");
    memcpy(cprLoader, &h, sizeof(h));

    // This adds the definition to the "library", to be used later.
    linker->addSection("FOLDEXEC", cprLoader, sizeof(h) + h.sz_cpr);
    delete cprLoader;

    n_mru = ft->n_mru;

    // Here is a quick summary of the format of the output file:
    linker->setLoaderAlignOffset(
            // Elf32_Edhr
        sizeof(elfout.ehdr) +
            // Elf32_Phdr: 1 for exec86, 2 for sh86, 3 for elf86
        (elfout.ehdr.e_phentsize * elfout.ehdr.e_phnum) +
            // checksum UPX! lsize version format
        sizeof(l_info) +
            // PT_DYNAMIC with DT_NEEDED "forwarded" from original file
        ((elfout.ehdr.e_phnum==3) ? elfout.phdr[2].p_memsz : 0) +
            // p_progid, p_filesize, p_blocksize
        sizeof(p_info) +
            // compressed data
        b_len + ph.c_len );
            // entry to stub
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

    addLoader("IDENTSTR", 0);
    addLoader("LEXEC020", 0);
    addLoader("FOLDEXEC", 0);

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
PackLinuxI386::buildLoader(Filter const *ft)
{
    unsigned const sz_fold = sizeof(linux_i386exec_fold);
    MemBuffer buf(sz_fold);
    memcpy(buf, linux_i386exec_fold, sz_fold);

    // patch loader
    // note: we only can use /proc/<pid>/fd when exetype > 0.
    //   also, we sleep much longer when compressing a script.
    checkPatch(NULL, 0, 0, 0);  // reset
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
    bool success = fold_ft.filter(buf + sizeof(cprElfHdr2), sz_fold - sizeof(cprElfHdr2));
    (void)success;

    return buildLinuxLoader(
        linux_i386exec_loader, sizeof(linux_i386exec_loader),
        buf, sz_fold, ft );
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


void PackLinuxI386::patchLoader() { }


void PackLinuxI386::patchLoaderChecksum()
{
    unsigned char *const ptr = const_cast<unsigned char *>(getLoader());
    l_info *const lp = (l_info *)(sizeof(elfout.ehdr) +
        (elfout.ehdr.e_phnum * elfout.ehdr.e_phentsize) + (char *)&elfout );
    // checksum for loader + p_info
    lp->l_checksum = 0;
    lp->l_magic = UPX_ELF_MAGIC;
    lp->l_lsize = (unsigned short) lsize;
    lp->l_version = (unsigned char) ph.version;
    lp->l_format  = (unsigned char) ph.format;
    // INFO: lp->l_checksum is currently unused
    lp->l_checksum = upx_adler32(ptr, lsize);
}


void PackLinuxI386::updateLoader(OutputFile *fo)
{
    elfout.ehdr.e_entry = fo->getBytesWritten() + elfout.phdr[0].p_vaddr;
}


/*
vi:ts=4:et
*/

