/* p_lx_exc.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
   Copyright (C) 2001-2008 John F. Reiser
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

#define PT_LOAD     Elf32_Phdr::PT_LOAD
#define PT_DYNAMIC  Elf32_Phdr::PT_DYNAMIC
#if 0 // UNUSED
#define DT_NULL     Elf32_Dyn::DT_NULL
#define DT_NEEDED   Elf32_Dyn::DT_NEEDED
#define DT_STRTAB   Elf32_Dyn::DT_STRTAB
#define DT_STRSZ    Elf32_Dyn::DT_STRSZ
#endif


/*************************************************************************
// linux/386 (generic "execve" format)
**************************************************************************/

PackLinuxI386::PackLinuxI386(InputFile *f) : super(f),
    ei_osabi(Elf32_Ehdr::ELFOSABI_LINUX), osabi_note(NULL)
{
    bele = &N_BELE_RTP::le_policy;
}

PackBSDI386::PackBSDI386(InputFile *f) : super(f)
{
    // Shell scripts need help specifying the target operating system.
    // Elf input will override this with .e_ident[EI_OSABI] or PT_NOTE.
    // [2006-09-27: Today's only runtime stub for shell is for linux.]
    if (Elf32_Ehdr::ELFOSABI_LINUX==opt->o_unix.osabi0) {
        // Disallow an incompatibility.
        ei_osabi = Elf32_Ehdr::ELFOSABI_NONE;
    }
    else {
        ei_osabi = opt->o_unix.osabi0;  // might be ELFOSABI_NONE
    }
}

static const
#include "stub/i386-linux.elf.execve-entry.h"
static const
#include "stub/i386-linux.elf.execve-fold.h"

static const
#include "stub/i386-bsd.elf.execve-entry.h"
static const
#include "stub/i386-bsd.elf.execve-fold.h"


const int *PackLinuxI386::getCompressionMethods(int method, int level) const
{
    return Packer::getDefaultCompressionMethods_le32(method, level);
}

const int *PackLinuxI386::getFilters() const
{
    static const int filters[] = {
        0x49, 0x46,
        0x26, 0x24, 0x11, 0x14, 0x13, 0x16, 0x25, 0x15, 0x12,
#if 0
// 0x80..0x87 are regarded as "untested".
        0x83, 0x86, 0x80, 0x84, 0x87, 0x81, 0x82, 0x85,
        0x24, 0x16, 0x13, 0x14, 0x11, 0x25, 0x15, 0x12,
#endif
    FT_END };
    return filters;
}

static void
set_stub_brk(Elf_LE32_Phdr *const phdr1, unsigned brka)
{
#define PAGE_MASK (~0ul<<12)
        // linux-2.6.14 binfmt_elf.c: SIGKILL if (0==.p_memsz) on a page boundary
        unsigned const brkb = brka | ((0==(~PAGE_MASK & brka)) ? 0x20 : 0);
        phdr1->p_type = PT_LOAD;  // be sure
        phdr1->p_offset = ~PAGE_MASK & brkb;
        phdr1->p_vaddr = brkb;
        phdr1->p_paddr = brkb;
        phdr1->p_filesz = 0;
        phdr1->p_memsz =  0;
        if (0==phdr1->p_flags) {
            phdr1->p_flags = Elf32_Phdr::PF_R|Elf32_Phdr::PF_W;
        }
        if (0==phdr1->p_align) {
            phdr1->p_align = 0x1000;
        }
#undef PAGE_MASK
}

void
PackLinuxI386::generateElfHdr(
    OutputFile *fo,
    void const *proto,
    unsigned const brka
)
{
    cprElfHdr2 *const h2 = (cprElfHdr2 *)&elfout;
    cprElfHdr3 *const h3 = (cprElfHdr3 *)&elfout;
    memcpy(h3, proto, sizeof(*h3));  // reads beyond, but OK

    assert(h2->ehdr.e_phoff     == sizeof(Elf32_Ehdr));
    assert(h2->ehdr.e_shoff     == 0);
    assert(h2->ehdr.e_ehsize    == sizeof(Elf32_Ehdr));
    assert(h2->ehdr.e_phentsize == sizeof(Elf32_Phdr));
    assert(h2->ehdr.e_shnum     == 0);

#if 0  //{
    unsigned identsize;
    char const *const ident = getIdentstr(&identsize);
#endif  //}
    h2->phdr[0].p_filesz = sizeof(*h2);  // + identsize;
    h2->phdr[0].p_memsz  = h2->phdr[0].p_filesz;

    // Info for OS kernel to set the brk()
    if (brka) {
        set_stub_brk(&h2->phdr[1], brka);
    }

    if (ph.format==UPX_F_LINUX_i386
    ||  ph.format==UPX_F_LINUX_SH_i386
    ||  ph.format==UPX_F_BSD_i386
    ) {
        // SELinux, PAx, grSecurity demand no PF_W if PF_X.
        // kernel-2.6.12-2.3.legacy_FC3 has a bug which demands
        // a PT_LOAD with PF_W, else SIGSEGV when clearing page fragment
        // on low page of ".bss", which is the high page of .text.
        // So the minimum number of PT_LOAD is 2.
        assert(h2->ehdr.e_phnum==2);
        memset(&h2->linfo, 0, sizeof(h2->linfo));
        fo->write(h2, sizeof(*h2));
    }
    else if (ph.format==UPX_F_LINUX_ELFI_i386) {
        assert(h3->ehdr.e_phnum==3);
        memset(&h3->linfo, 0, sizeof(h3->linfo));
        fo->write(h3, sizeof(*h3));
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

    generateElfHdr(fo, stub_i386_linux_elf_execve_fold, 0);
}

void
PackBSDI386::pack1(OutputFile *fo, Filter &)
{
    // create a pseudo-unique program id for our paranoid stub
    progid = getRandomId();

    generateElfHdr(fo, stub_i386_bsd_elf_execve_fold, 0);
}

void
PackLinuxI386::pack4(OutputFile *fo, Filter &ft)
{
    overlay_offset = sizeof(elfout.ehdr) +
        (elfout.ehdr.e_phentsize * elfout.ehdr.e_phnum) +
        sizeof(l_info) +
        ((elfout.ehdr.e_phnum==3) ? (unsigned) elfout.phdr[2].p_memsz : 0) ;
    unsigned nw = fo->getBytesWritten();
    elfout.phdr[0].p_filesz = nw;
    nw = 0u-((0u-elfout.phdr[0].p_align) & (0u-nw));  // ALIGN_UP
    super::pack4(fo, ft);  // write PackHeader and overlay_offset
    set_stub_brk(&elfout.phdr[1], nw + elfout.phdr[0].p_vaddr);

#if 0  // {
    // /usr/bin/strip from RedHat 8.0 (binutils-2.13.90.0.2-2)
    // generates a 92-byte [only] output, because the "linking view"
    // is empty.  This code supplies a "linking view".
    // However, 'strip' then generates _plausible_ junk that gets
    // "Illegal instruction"  because 'strip' changes p_hdr[1].p_align,
    // .p_offset, and .p_vaddr incorrectly.  So the "cure" is worse than
    // the disease.  It is obvious that a 92-byte file is bad,
    // but it is not obvious that the changed .p_align is bad.
    // Also, having a totally empty "linking view" is easier for 'strip'
    // to fix: just detect that, and do nothing.
    // So, we don't use this code for now [2003-01-11].

    // Supply a "linking view" that covers everything,
    // so that 'strip' does not omit everything.
    Elf_LE32_Shdr shdr;
    // The section header string table.
    char const shstrtab[] = "\0.\0.shstrtab";

    unsigned eod = elfout.phdr[0].p_filesz;
    elfout.ehdr.e_shoff = eod;
    elfout.ehdr.e_shentsize = sizeof(shdr);
    elfout.ehdr.e_shnum = 3;
    elfout.ehdr.e_shstrndx = 2;

    // An empty Elf32_Shdr for space as a null index.
    memset(&shdr, 0, sizeof(shdr));
    shdr.sh_type = Elf32_Shdr::SHT_NULL;
    fo->write(&shdr, sizeof(shdr));

    // Cover all the bits we need at runtime.
    memset(&shdr, 0, sizeof(shdr));
    shdr.sh_name = 1;
    shdr.sh_type = Elf32_Shdr::SHT_PROGBITS;
    shdr.sh_flags = Elf32_Shdr::SHF_ALLOC;
    shdr.sh_addr = elfout.phdr[0].p_vaddr;
    shdr.sh_offset = overlay_offset;
    shdr.sh_size = eod - overlay_offset;
    shdr.sh_addralign = 4096;
    fo->write(&shdr, sizeof(shdr));

    // A section header for the section header string table.
    memset(&shdr, 0, sizeof(shdr));
    shdr.sh_name = 3;
    shdr.sh_type = Elf32_Shdr::SHT_STRTAB;
    shdr.sh_offset = 3*sizeof(shdr) + eod;
    shdr.sh_size = sizeof(shstrtab);
    fo->write(&shdr, sizeof(shdr));

    fo->write(shstrtab, sizeof(shstrtab));
#endif  // }


    // Cannot pre-round .p_memsz.  If .p_filesz < .p_memsz, then kernel
    // tries to make .bss, which requires PF_W.
    // But strict SELinux (or PaX, grSecurity) disallows PF_W with PF_X.
#if 0  /*{*/
#undef PAGE_MASK
#define PAGE_MASK (~0u<<12)
    // pre-calculate for benefit of runtime disappearing act via munmap()
    elfout.phdr[0].p_memsz =  PAGE_MASK & (~PAGE_MASK + elfout.phdr[0].p_filesz);
#undef PAGE_MASK
#else  /*}{*/
    elfout.phdr[0].p_memsz =  elfout.phdr[0].p_filesz;
#endif  /*}*/

    // rewrite Elf header
    fo->seek(0, SEEK_SET);
    fo->rewrite(&elfout, overlay_offset);
}

static unsigned
umax(unsigned a, unsigned b)
{
    if (a <= b) {
        return b;
    }
    return a;
}

Linker *PackLinuxI386::newLinker() const
{
    return new ElfLinkerX86;
}

void
PackLinuxI386::buildLinuxLoader(
    upx_byte const *const proto,
    unsigned        const szproto,
    upx_byte const *const fold,
    unsigned        const szfold,
    Filter const *ft
)
{
    initLoader(proto, szproto);

    unsigned fold_hdrlen = 0;
  if (0 < szfold) {
    cprElfHdr1 const *const hf = (cprElfHdr1 const *)fold;
    fold_hdrlen = sizeof(hf->ehdr) + hf->ehdr.e_phentsize * hf->ehdr.e_phnum +
         sizeof(l_info);
    if (0 == get_le32(fold_hdrlen + fold)) {
        // inconsistent SIZEOF_HEADERS in *.lds (ld, binutils)
        fold_hdrlen = umax(0x80, fold_hdrlen);
    }
  }
    // This adds the definition to the "library", to be used later.
    // NOTE: the stub is NOT compressed!  The savings is not worth it.
    linker->addSection("FOLDEXEC", fold + fold_hdrlen, szfold - fold_hdrlen, 0);

    n_mru = ft->n_mru;

// Rely on "+80CXXXX" [etc] in getDecompressorSections() packer_c.cpp */
//    // Here is a quick summary of the format of the output file:
//    linker->setLoaderAlignOffset(
//            // Elf32_Edhr
//        sizeof(elfout.ehdr) +
//            // Elf32_Phdr: 1 for exec86, 2 for sh86, 3 for elf86
//        (elfout.ehdr.e_phentsize * elfout.ehdr.e_phnum) +
//            // checksum UPX! lsize version format
//        sizeof(l_info) +
//            // PT_DYNAMIC with DT_NEEDED "forwarded" from original file
//        ((elfout.ehdr.e_phnum==3) ? (unsigned) elfout.phdr[2].p_memsz : 0) +
//            // p_progid, p_filesize, p_blocksize
//        sizeof(p_info) +
//            // compressed data
//        b_len + ph.c_len );
//            // entry to stub
    addLoader("LEXEC000", NULL);

    if (ft->id) {
        if (n_mru) {
            addLoader("LEXEC009", NULL);
        }
    }
    addLoader("LEXEC010", NULL);
    linker->defineSymbol("filter_cto", ft->cto);
    linker->defineSymbol("filter_length",
                         (ft->id & 0xf) % 3 == 0 ? ft->calls :
                         ft->lastcall - ft->calls * 4);
    addLoader(getDecompressorSections(), NULL);
    addLoader("LEXEC015", NULL);
    if (ft->id) {
        {  // decompr, unfilter not separate
            if (0x80==(ft->id & 0xF0)) {
                addLoader("LEXEC110", NULL);
                if (n_mru) {
                    addLoader("LEXEC100", NULL);
                }
                // bug in APP: jmp and label must be in same .asx/.asy
                addLoader("LEXEC016", NULL);
            }
        }
        addFilter32(ft->id);
        {  // decompr always unfilters
            addLoader("LEXEC017", NULL);
        }
    }
    else {
        addLoader("LEXEC017", NULL);
    }

    addLoader("IDENTSTR", NULL);
    addLoader("LEXEC020", NULL);
    addLoader("FOLDEXEC", NULL);
    if (M_IS_LZMA(ph.method)) {
        const lzma_compress_result_t *res = &ph.compress_result.result_lzma;
        acc_uint32e_t properties = // lc, lp, pb, dummy
            (res->lit_context_bits << 0) |
            (res->lit_pos_bits << 8) |
            (res->pos_bits << 16);
        if (linker->bele->isBE()) // big endian - bswap32
            acc_swab32s(&properties);
        linker->defineSymbol("lzma_properties", properties);
        // -2 for properties
        linker->defineSymbol("lzma_c_len", ph.c_len - 2);
        linker->defineSymbol("lzma_u_len", ph.u_len);
        unsigned const stack = getDecompressorWrkmemSize();
        linker->defineSymbol("lzma_stack_adjust", 0u - stack);
    }
    if (0x80==(ft->id & 0xF0)) {
        int const mru = ft->n_mru ? 1+ ft->n_mru : 0;
        if (mru && mru!=256) {
            unsigned const is_pwr2 = (0==((mru -1) & mru));
            linker->defineSymbol("NMRU", mru - is_pwr2);
        }
    }
    relocateLoader();
}

void
PackLinuxI386::buildLoader(Filter const *ft)
{
    unsigned const sz_fold = sizeof(stub_i386_linux_elf_execve_fold);
    MemBuffer buf(sz_fold);
    memcpy(buf, stub_i386_linux_elf_execve_fold, sz_fold);

    // patch loader
    // note: we only can use /proc/<pid>/fd when exetype > 0.
    //   also, we sleep much longer when compressing a script.
    checkPatch(NULL, 0, 0, 0);  // reset
    patch_le32(buf,sz_fold,"UPX4",exetype > 0 ? 3 : 15);   // sleep time
    patch_le32(buf,sz_fold,"UPX3",progid);
    patch_le32(buf,sz_fold,"UPX2",exetype > 0 ? 0 : 0x7fffffff);

    buildLinuxLoader(
        stub_i386_linux_elf_execve_entry, sizeof(stub_i386_linux_elf_execve_entry),
        buf, sz_fold, ft );
}

void
PackBSDI386::buildLoader(Filter const *ft)
{
    unsigned const sz_fold = sizeof(stub_i386_bsd_elf_execve_fold);
    MemBuffer buf(sz_fold);
    memcpy(buf, stub_i386_bsd_elf_execve_fold, sz_fold);

    // patch loader
    // note: we only can use /proc/<pid>/fd when exetype > 0.
    //   also, we sleep much longer when compressing a script.
    checkPatch(NULL, 0, 0, 0);  // reset
    patch_le32(buf,sz_fold,"UPX4",exetype > 0 ? 3 : 15);   // sleep time
    patch_le32(buf,sz_fold,"UPX3",progid);
    patch_le32(buf,sz_fold,"UPX2",exetype > 0 ? 0 : 0x7fffffff);

    buildLinuxLoader(
        stub_i386_bsd_elf_execve_entry, sizeof(stub_i386_bsd_elf_execve_entry),
        buf, sz_fold, ft );
}

// FIXME: getLoaderPrefixSize is unused?
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
    if (!memcmp(buf+8, "FreeBSD", 7))                  // branded
        return 1;
    if (ehdr->e_type != Elf32_Ehdr::ET_EXEC
    &&  ehdr->e_type != Elf32_Ehdr::ET_DYN )           // executable
        return 2;
    if (ehdr->e_machine != Elf32_Ehdr::EM_386)         // Intel 80386
        return 3;
    if (ehdr->e_version != Elf32_Ehdr::EV_CURRENT)     // version
        return 4;
    if (ehdr->e_phnum < 1)
        return 5;
    if (ehdr->e_phentsize != sizeof(Elf32_Phdr))
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



/*************************************************************************
//
**************************************************************************/

bool PackLinuxI386::canPack()
{
    if (exetype != 0)
        return super::canPack();

    Elf_LE32_Ehdr ehdr;
    unsigned char *buf = ehdr.e_ident;

    fi->seek(0, SEEK_SET);
    fi->readx(&ehdr, sizeof(ehdr));
    fi->seek(0, SEEK_SET);

    exetype = 0;
    const unsigned l = get_le32(buf);

    int elf = checkEhdr(&ehdr);
    if (elf >= 0) {
        // NOTE: ELF executables are now handled by p_lx_elf.cpp,
        //   so we only handle them here if force_execve
        if (elf == 0 && opt->o_unix.force_execve) {
            exetype = 1;

            unsigned osabi0 = ehdr.e_ident[Elf32_Ehdr::EI_OSABI];
            switch (osabi0) {
            case Elf32_Ehdr::ELFOSABI_LINUX:
            case Elf32_Ehdr::ELFOSABI_FREEBSD:
            case Elf32_Ehdr::ELFOSABI_NETBSD:
            case Elf32_Ehdr::ELFOSABI_OPENBSD:
                ei_osabi = osabi0;  // Proudly declares its osabi in Ehdr.
                break;
            default:
            unsigned const e_phnum = get_te16(&ehdr.e_phnum);
            if (e_phnum<=(512/sizeof(Elf32_Phdr))) {
                char buf2[512];
                fi->seek(get_te32(&ehdr.e_phoff), SEEK_SET);
                fi->readx(buf2, sizeof(buf2));
                fi->seek(0, SEEK_SET);
                Elf32_Phdr const *phdr = (Elf32_Phdr const *)buf2;
                for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
                    if (phdr->PT_NOTE == get_te32(&phdr->p_type)) {
                        unsigned const offset = get_te32(&phdr->p_offset);
                        struct Elf32_Note note; memset(&note, 0, sizeof(note));
                        fi->seek(offset, SEEK_SET);
                        fi->readx(&note, sizeof(note));
                        fi->seek(0, SEEK_SET);
                        if (4==get_te32(&note.descsz)
                        &&  1==get_te32(&note.type)
                        &&  0==note.end ) {
                            if (0==strcmp("NetBSD", (char const *)&note.text)) {
                                ei_osabi = Elf32_Ehdr::ELFOSABI_NETBSD;
                                break;
                            }
                            if (0==strcmp("OpenBSD", (char const *)&note.text)) {
                                ei_osabi = Elf32_Ehdr::ELFOSABI_OPENBSD;
                                break;
                            }
                        }
                    }
                }
            }
            }
        }
        if (UPX_F_BSD_i386==getFormat()
        && !(Elf32_Ehdr::ELFOSABI_FREEBSD==ei_osabi
          || Elf32_Ehdr::ELFOSABI_NETBSD ==ei_osabi
          || Elf32_Ehdr::ELFOSABI_OPENBSD==ei_osabi )) {
            return false;
        }
    }
    else if (l == 0x00640107 || l == 0x00640108 || l == 0x0064010b || l == 0x006400cc)
    {
        // OMAGIC / NMAGIC / ZMAGIC / QMAGIC
        exetype = 2;
        // FIXME: N_TRSIZE, N_DRSIZE
        // FIXME: check for aout shared libraries
    }
    else { // shell scripts and other interpreters
        if (Elf32_Ehdr::ELFOSABI_LINUX!=ei_osabi) {
            return false;  // so far, only Linux has runtime stub for shell
        }
        else if (!memcmp(buf, "#!/", 3))                    // #!/bin/sh
            exetype = -1;
        else if (!memcmp(buf, "#! /", 4))                   // #! /bin/sh
            exetype = -1;
        else if (!memcmp(buf, "\xca\xfe\xba\xbe", 4))       // Java bytecode
            exetype = -2;
    }

    return super::canPack();
}


void PackLinuxI386::patchLoader() { }


void PackLinuxI386::patchLoaderChecksum()
{
    unsigned char *const ptr = getLoader();
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

