/* p_lx_exc.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2000 Laszlo Molnar
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

   Markus F.X.J. Oberhumer                   Laszlo Molnar
   markus.oberhumer@jk.uni-linz.ac.at        ml1050@cdata.tvnet.hu
 */


#include "conf.h"

#include "file.h"
#include "packer.h"
#include "p_elf.h"
#include "p_unix.h"
#include "p_lx_exc.h"

#define PT_LOAD     Elf_LE32_Phdr::PT_LOAD


/*************************************************************************
// linux/386 (generic "execve" format)
**************************************************************************/

static const
#include "stub/l_lx_n2b.h"
static const
#include "stub/l_lx_n2d.h"


int PackLinuxI386::getCompressionMethod() const
{
    if (M_IS_NRV2B(opt->method))
        return M_NRV2B_LE32;
    if (M_IS_NRV2D(opt->method))
        return M_NRV2D_LE32;
    return opt->level > 1 && file_size >= 512*1024 ? M_NRV2D_LE32 : M_NRV2B_LE32;
}


const upx_byte *PackLinuxI386::getLoader() const
{
    if (M_IS_NRV2B(opt->method))
        return linux_i386exec_nrv2b_loader;
    if (M_IS_NRV2D(opt->method))
        return linux_i386exec_nrv2d_loader;
    return NULL;
}


int PackLinuxI386::getLoaderSize() const
{
    if (0!=lsize) {
        return lsize;
    }
    if (M_IS_NRV2B(opt->method))
        return sizeof(linux_i386exec_nrv2b_loader);
    if (M_IS_NRV2D(opt->method))
        return sizeof(linux_i386exec_nrv2d_loader);
    return 0;
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

    // info: ELF executables are now handled by p_lx_elf.cpp
    if (memcmp(buf, "\x7f\x45\x4c\x46\x01\x01\x01", 7)) // ELF 32-bit LSB
        return -1;

    // now check the ELF header
    if (!memcmp(buf+8, "FreeBSD", 7))                   // branded
        return 1;
    if (ehdr->e_type != 2)                              // executable
        return 2;
    if (ehdr->e_machine != 3 && ehdr->e_machine != 6)   // Intel 80[34]86
        return 3;
    if (ehdr->e_version != 1)                           // version
        return 4;
    if (ehdr->e_phnum < 1)
        return 5;

    // FIXME: add special checks for uncompresed "vmlinux" kernel
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
    lsize = getLoaderSize();

    // patch loader
    // note: we only can use /proc/<pid>/fd when exetype > 0.
    //   also, we sleep much longer when compressing a script.
    patch_le32(loader,lsize,"UPX4",exetype > 0 ? 3 : 15);   // sleep time
    patch_le32(loader,lsize,"UPX3",exetype > 0 ? 0 : 0x7fffffff);
    patch_le32(loader,lsize,"UPX2",progid);
    patchVersion(loader,lsize);

    Elf_LE32_Ehdr *const ehdr = (Elf_LE32_Ehdr *)(void *)loader;
    Elf_LE32_Phdr *const phdr = (Elf_LE32_Phdr *)(1+ehdr);

    // stub/scripts/setfold.pl puts address of 'fold_begin' in phdr[1].p_offset
    off_t const fold_begin = phdr[1].p_offset;
    assert(fold_begin > 0);
    assert(fold_begin < (off_t)lsize);
    MemBuffer cprLoader(lsize);

    // compress compiled C-code portion of loader
    upx_uint const uncLsize = lsize - fold_begin;
    upx_uint       cprLsize;
    int r = upx_compress(loader + fold_begin, uncLsize, cprLoader, &cprLsize,
                         NULL, opt->method, 10, NULL, NULL);
    if (r != UPX_E_OK || cprLsize >= uncLsize)
        throwInternalError("loaded compression failed");

    memcpy(fold_begin+loader, cprLoader, cprLsize);
    lsize = fold_begin + cprLsize;
    phdr->p_filesz = lsize;
    // phdr->p_memsz is the decompressed size

    // The beginning of our loader consists of a elf_hdr (52 bytes) and
    // one     section elf_phdr (32 byte) now,
    // another section elf_phdr (32 byte) later, so we have 12 free bytes
    // from offset 116 to the program start at offset 128.
    // These 12 bytes are used for l_info by ::patchLoaderChecksum().
    assert(get_le32(loader + 28) == 52);        // e_phoff
    assert(get_le32(loader + 32) == 0);         // e_shoff
    assert(get_le16(loader + 40) == 52);        // e_ehsize
    assert(get_le16(loader + 42) == 32);        // e_phentsize
    assert(get_le16(loader + 44) == 1);         // e_phnum
    assert(get_le16(loader + 48) == 0);         // e_shnum
    assert(lsize > 128 && lsize < 4096);

    patchLoaderChecksum();
}


void PackLinuxI386::patchLoaderChecksum()
{
    l_info *const lp = (l_info *)(loader + getLoaderPrefixSize());
    // checksum for loader + p_info
    lp->l_checksum = 0;  // (this checksum is currently unused)
    lp->l_magic = UPX_ELF_MAGIC;
    lp->l_lsize = (unsigned short) lsize;
    lp->l_version = (unsigned char) ph.version;
    lp->l_format  = (unsigned char) ph.format;
    unsigned adler = upx_adler32(0,NULL,0);
    adler = upx_adler32(adler, loader, lsize + sizeof(p_info));
    lp->l_checksum = adler;
}


void PackLinuxI386::updateLoader(OutputFile *fo)
{
#define PAGE_MASK (~0<<12)
    Elf_LE32_Ehdr *const ehdr = (Elf_LE32_Ehdr *)(unsigned char *)loader;
    ehdr->e_phnum = 2;

    // The first Phdr maps the stub (instructions, data, bss) rwx.
    // The second Phdr maps the overlay r--,
    // to defend against /usr/bin/strip removing the overlay.
    Elf_LE32_Phdr *const phdro = 1+(Elf_LE32_Phdr *)(1+ehdr);

    phdro->p_type = PT_LOAD;
    phdro->p_offset = lsize;
    phdro->p_paddr = phdro->p_vaddr = 0x00400000 + (lsize &~ PAGE_MASK);
    phdro->p_memsz = phdro->p_filesz = fo->getBytesWritten() - lsize;
    phdro->p_flags = phdro->PF_R;
    phdro->p_align = (unsigned) (-PAGE_MASK);

    patchLoaderChecksum();
    fo->seek(0, SEEK_SET);
    fo->rewrite(loader, 0x80);
#undef PAGE_MASK
}


/*
vi:ts=4:et
*/

