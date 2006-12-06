/* p_vmlinx.cpp -- pack vmlinux ET_EXEC file (before bootsect or setup)

   This file is part of the UPX executable compressor.

   Copyright (C) 2004-2006 John Reiser
   Copyright (C) 1996-2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2006 Laszlo Molnar
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

   Markus F.X.J. Oberhumer   Laszlo Molnar
   markus@oberhumer.com      ml1050@users.sourceforge.net

   John Reiser
   jreiser@users.sourceforge.net
 */


#include "conf.h"

#include "file.h"
#include "filter.h"
#include "packer.h"
#include "p_vmlinx.h"
#include "linker.h"

static const
#include "stub/i386-linux.kernel.vmlinux.h"

/*************************************************************************
//
**************************************************************************/

PackVmlinuxI386::PackVmlinuxI386(InputFile *f) :
    super(f), n_ptload(0), phdri(NULL), shdri(NULL), shstrtab(NULL)
{
    bele = &N_BELE_RTP::le_policy;
}

PackVmlinuxI386::~PackVmlinuxI386()
{
    delete [] shstrtab;
    delete [] phdri;
    delete [] shdri;
}

const int *PackVmlinuxI386::getCompressionMethods(int method, int level) const
{
    return Packer::getDefaultCompressionMethods_le32(method, level);
}


const int *PackVmlinuxI386::getFilters() const
{
    static const int filters[] = {
        0x49, 0x46,
    -1 };
    return filters;
}

// copied from PackUnix 2006-10-13.
int PackVmlinuxI386::getStrategy(Filter &/*ft*/)
{
    // Called just before reading and compressing each block.
    // Might want to adjust blocksize, etc.

    // If user specified the filter, then use it (-2==filter_strategy).
    // Else try the first two filters, and pick the better (2==filter_strategy).
    return (opt->no_filter ? -3 : ((opt->filter > 0) ? -2 : 2));
}


Elf_LE32_Shdr const *
PackVmlinuxI386::getElfSections()
{
    Elf_LE32_Shdr const *p, *shstrsec=0;
    shdri = new Elf_LE32_Shdr[(unsigned) ehdri.e_shnum];
    fi->seek(ehdri.e_shoff, SEEK_SET);
    fi->readx(shdri, ehdri.e_shnum * sizeof(*shdri));
    int j;
    for (p = shdri, j= ehdri.e_shnum; --j>=0; ++p) {
        if (Elf32_Shdr::SHT_STRTAB==p->sh_type
        &&  (p->sh_size + p->sh_offset) <= (unsigned) file_size
        &&  (10+ p->sh_name) <= p->sh_size  // 1+ strlen(".shstrtab")
        ) {
            delete [] shstrtab;
            shstrtab = new char[1+ p->sh_size];
            fi->seek(p->sh_offset, SEEK_SET);
            fi->readx(shstrtab, p->sh_size);
            shstrtab[p->sh_size] = '\0';
            if (0==strcmp(".shstrtab", shstrtab + p->sh_name)) {
                shstrsec = p;
                break;
            }
        }
    }
    return shstrsec;
}

static int __acc_cdecl_qsort
compare_Phdr(void const *aa, void const *bb)
{
    Elf32_Phdr const *const a = (Elf32_Phdr const *)aa;
    Elf32_Phdr const *const b = (Elf32_Phdr const *)bb;
    unsigned const xa = a->p_type - Elf32_Phdr::PT_LOAD;
    unsigned const xb = b->p_type - Elf32_Phdr::PT_LOAD;
            if (xa < xb)         return -1;  // PT_LOAD first
            if (xa > xb)         return  1;
    if (a->p_paddr < b->p_paddr) return -1;  // ascending by .p_paddr
    if (a->p_paddr > b->p_paddr) return  1;
                                 return  0;
}

//
// Examples as of 2004-07-16 [readelf --segments vmlinux  # before fiddling]:
//
//----- kernel-2.6.7 plain [defconfig?]
//Program Headers(2):
//  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
//  LOAD           0x001000 0x00100000 0x00100000 0x1c7e61 0x1c7e61 R E 0x1000
//  LOAD           0x1c8e64 0x002c8e64 0x002c8e64 0x00000 0x00000 RW  0x1000
//
//----- kernel-2.6.7-1.488 Fedora Core 3 test 1
//Program Headers(5):
//  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
//  LOAD           0x001000 0x02100000 0x02100000 0x202246 0x202246 R E 0x1000
//  LOAD           0x204000 0xffff3000 0x02303000 0x00664 0x00664 R E 0x1000
//  LOAD           0x205000 0x02304000 0x02304000 0x43562 0x43562 R   0x1000
//  LOAD           0x249000 0x02348000 0x02348000 0x81800 0xcb0fc RWE 0x1000
//  STACK          0x000000 0x00000000 0x00000000 0x00000 0x00000 RWE 0x4
//
//----- kernel-2.6.18-1.2778 Fedora Core 6 test 3
//Program Headers(3)
//  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
//  LOAD           0x001000 0xc0400000 0x00400000 0x279820 0x279820 R E 0x1000
//  LOAD           0x27b000 0xc067a000 0x0067a000 0x10ee64 0x1b07e8 RWE 0x1000
//  NOTE           0x000000 0x00000000 0x00000000 0x00000 0x00000 R   0x4

bool PackVmlinuxI386::canPack()
{
    fi->seek(0, SEEK_SET);
    fi->readx(&ehdri, sizeof(ehdri));

    // now check the ELF header
    if (memcmp(&ehdri, "\x7f\x45\x4c\x46\x01\x01\x01", 7) // ELF 32-bit LSB
    ||  !memcmp(&ehdri.e_ident[8], "FreeBSD", 7)  // branded
    ||  ehdri.e_machine != 3  // Intel 80386
    ||  ehdri.e_version != 1  // version
    )
        return false;

    // additional requirements for vmlinux
    if (ehdri.e_ehsize != sizeof(ehdri)  // different <elf.h> ?
    ||  ehdri.e_phoff != sizeof(ehdri)  // Phdrs not contiguous with Ehdr
    ||  ehdri.e_phentsize!=sizeof(Elf32_Phdr)
    ||  ehdri.e_type!=Elf32_Ehdr::ET_EXEC
    ||  0!=(0x000fffff & ehdri.e_entry) // entry not on whole 1MB
    ) {
        return false;
    }

    // A Linux kernel must have a __ksymtab section. [??]
    Elf_LE32_Shdr const *p, *const shstrsec = getElfSections();
    if (0==shstrsec) {
        return false;
    }
    {
    int j;
    for (p = shdri, j= ehdri.e_shnum; --j>=0; ++p) {
        if (Elf32_Shdr::SHT_PROGBITS==p->sh_type
        && 0==strcmp("__ksymtab", p->sh_name + shstrtab)) {
            break;
        }
    }
    if (j < 0)
        return false;
    }

    phdri = new Elf_LE32_Phdr[(unsigned) ehdri.e_phnum];
    fi->seek(ehdri.e_phoff, SEEK_SET);
    fi->readx(phdri, ehdri.e_phnum * sizeof(*phdri));

    // Put PT_LOAD together at the beginning, ascending by .p_paddr.
    qsort(phdri, ehdri.e_phnum, sizeof(*phdri), compare_Phdr);

    // Check that PT_LOADs form one contiguous chunk of the file.
    for (unsigned j = 0; j < ehdri.e_phnum; ++j) {
        if (Elf32_Phdr::PT_LOAD==phdri[j].p_type) {
            if (0xfff & (phdri[j].p_offset | phdri[j].p_paddr
                       | phdri[j].p_align  | phdri[j].p_vaddr) ) {
                return false;
            }
            if (0 < j) {
                unsigned const sz = (0u - phdri[j-1].p_align)
                                  & (phdri[j-1].p_align -1 + phdri[j-1].p_filesz);
                if ((sz + phdri[j-1].p_offset)!=phdri[j].p_offset) {
                    return false;
                }
            }
            ++n_ptload;
            sz_ptload = phdri[j].p_filesz + phdri[j].p_offset - phdri[0].p_offset;
        }
    }
    return 0 < n_ptload;
}


Linker* PackVmlinuxI386::newLinker() const
{
    return new ElfLinkerX86;
}


void PackVmlinuxI386::buildLoader(const Filter *ft)
{
    // prepare loader
    initLoader(stub_i386_linux_kernel_vmlinux, sizeof(stub_i386_linux_kernel_vmlinux));
    addLoader("LINUX000",
              (0x40==(0xf0 & ft->id)) ? "LXCKLLT1" : (ft->id ? "LXCALLT1" : ""),
              "LXMOVEUP",
              getDecompressorSections(),
              NULL
             );
    if (ft->id)
    {
        assert(ft->calls > 0);
        if (0x40==(0xf0 & ft->id)) {
            addLoader("LXCKLLT9", NULL);
        }
        else {
            addLoader("LXCALLT9", NULL);
        }
        addFilter32(ft->id);
    }
    addLoader("LINUX990,IDENTSTR,UPX1HEAD", NULL);
}


static bool defineFilterSymbols(Linker *linker, const Filter *ft)
{
    if (ft->id == 0) {
        linker->defineSymbol("filter_length", 0);
        return false;
    }
    assert(ft->calls > 0);

    linker->defineSymbol("filter_cto", ft->cto);
    linker->defineSymbol("filter_length",
                         (ft->id & 0xf) % 3 == 0 ? ft->calls :
                         ft->lastcall - ft->calls * 4);
    return true;
}

static const
#include "stub/i386-linux.kernel.vmlinux-head.h"

void PackVmlinuxI386::pack(OutputFile *fo)
{
    unsigned fo_off = 0;
    Elf32_Ehdr ehdro;
    LE32 tmp_le32;

    // NULL
    // .text(PT_LOADs) .note(1st page) .note(rest)
    // .shstrtab /* .symtab .strtab */
    Elf32_Shdr shdro[1+3+1/*+2*/];
    memset(shdro, 0, sizeof(shdro));

    ibuf.alloc(file_size);
    obuf.allocForCompression(file_size);

    // .e_ident, .e_machine, .e_version, .e_flags
    memcpy(&ehdro, &ehdri, sizeof(ehdro));
    ehdro.e_type = Elf32_Ehdr::ET_REL;
    ehdro.e_entry = 0;
    ehdro.e_phoff = 0;
    ehdro.e_shoff = 0;  // later
    ehdro.e_phentsize = 0;
    ehdro.e_phnum = 0;
    ehdro.e_shnum = 1+3+1/*+2*/;
    ehdro.e_shstrndx = 4;
    fo->write(&ehdro, sizeof(ehdro)); fo_off+= sizeof(ehdro);

    ph.u_len = sz_ptload;
    fi->seek(phdri[0].p_offset, SEEK_SET);
    fi->readx(ibuf, ph.u_len);
    checkAlreadyPacked(ibuf + ph.u_len - 1024, 1024);

    // prepare filter
    ph.filter = 0;
    Filter ft(ph.level);
    ft.buf_len = ph.u_len;
    ft.addvalue = 0;  // we are independent of actual runtime address; see ckt32

    upx_compress_config_t cconf; cconf.reset();
    // limit stack size needed for runtime decompression
    cconf.conf_lzma.max_num_probs = 1846 + (768 << 4); // ushort: ~28KB stack
    compressWithFilters(&ft, 512, &cconf, getStrategy(ft));

    const unsigned lsize = getLoaderSize();

    defineFilterSymbols(linker, &ft);
    if (0x40==(0xf0 & ft.id)) {
        linker->defineSymbol("filter_length", ph.u_len); // redefine
    }
    defineDecompressorSymbols();
    relocateLoader();

    MemBuffer loader(lsize);
    memcpy(loader, getLoader(), lsize);
    patchPackHeader(loader, lsize);

#define shstrtab local_shstrtab // avoid -Wshadow warning
    char const shstrtab[]= "\0.text\0.note\0.shstrtab\0.symtab\0.strtab";
    char const *p = shstrtab;
    while (0!=*p++) ;
    shdro[1].sh_name = ptr_diff(p, shstrtab);
    shdro[1].sh_type = Elf32_Shdr::SHT_PROGBITS;
    shdro[1].sh_flags = Elf32_Shdr::SHF_ALLOC | Elf32_Shdr::SHF_EXECINSTR;
    shdro[1].sh_offset = fo_off;
    shdro[1].sh_size = sizeof(stub_i386_linux_kernel_vmlinux_head) + ph.c_len + lsize;
    shdro[1].sh_addralign = 1;

    // ENTRY_POINT
    fo->write(&stub_i386_linux_kernel_vmlinux_head[0], sizeof(stub_i386_linux_kernel_vmlinux_head)-2*(1+ 4) +1);
    tmp_le32 = ehdri.e_entry; fo->write(&tmp_le32, 4);

    // COMPRESSED_LENGTH
    fo->write(&stub_i386_linux_kernel_vmlinux_head[sizeof(stub_i386_linux_kernel_vmlinux_head)-(1+ 4)], 1);
    tmp_le32 = ph.c_len;      fo->write(&tmp_le32, 4);

    fo_off += sizeof(stub_i386_linux_kernel_vmlinux_head);

    fo->write(obuf, ph.c_len); fo_off += ph.c_len;
    fo->write(loader, lsize); fo_off += lsize;

#if 0
    printf("%-13s: compressed   : %8u bytes\n", getName(), ph.c_len);
    printf("%-13s: decompressor : %8u bytes\n", getName(), lsize);
#endif
    verifyOverlappingDecompression();

    // .note with 1st page --------------------------------
    ph.u_len = phdri[0].p_offset;
    fi->seek(0, SEEK_SET);
    fi->readx(ibuf, ph.u_len);
    compress(ibuf, obuf);

    while (0!=*p++) ;
    shdro[2].sh_name = ptr_diff(p, shstrtab);
    shdro[2].sh_type = Elf32_Shdr::SHT_NOTE;
    shdro[2].sh_offset = fo_off;
    shdro[2].sh_size = sizeof(ph.u_len) + ph.c_len;
    shdro[2].sh_addralign = 1;
    tmp_le32 = ph.u_len; fo->write(&tmp_le32, 4);
    fo->write(obuf, ph.c_len); fo_off += shdro[2].sh_size;

    // .note with rest     --------------------------------
    ph.u_len = file_size - (sz_ptload + phdri[0].p_offset);
    fi->seek(sz_ptload + phdri[0].p_offset, SEEK_SET);
    fi->readx(ibuf, ph.u_len);

    // Temporarily decrease ph.level by about (1+ log2(sz_rest / sz_ptload))
    // to avoid spending unreasonable effort compressing large symbol tables
    // that are discarded 99.9% of the time anyway.
    int const old_level = ph.level;
    for (unsigned v = ((ph.u_len>>3) + ph.u_len) / sz_ptload; 0 < v; v>>=1) {
        if (0== --ph.level) {
            ph.level = 1;
        }
    }
    compress(ibuf, obuf, &cconf);
    ph.level = old_level;

    // while (0!=*p++) ;  // name is the same
    shdro[3].sh_name = ptr_diff(p, shstrtab);
    shdro[3].sh_type = Elf32_Shdr::SHT_NOTE;
    shdro[3].sh_offset = fo_off;
    shdro[3].sh_size = sizeof(ph.u_len) + ph.c_len;
    shdro[3].sh_addralign = 1;
    tmp_le32 = ph.u_len; fo->write(&tmp_le32, 4);
    fo->write(obuf, ph.c_len); fo_off += shdro[3].sh_size;

    while (0!=*p++) ;
    shdro[4].sh_name = ptr_diff(p, shstrtab);
    shdro[4].sh_type = Elf32_Shdr::SHT_STRTAB;
    shdro[4].sh_offset = fo_off;
    shdro[4].sh_size = sizeof(shstrtab);  // already includes terminating '\0'
    shdro[4].sh_addralign = 1;
    fo->write(shstrtab, shdro[4].sh_size); fo_off += shdro[4].sh_size;

#if 0  /*{ no symbols! */
    while (0!=*p++) ;
    fo_off = ~3 & (3+ fo_off);
    shdro[5].sh_name = ptr_diff(p, shstrtab);
    shdro[5].sh_type = Elf32_Shdr::SHT_SYMTAB;
    shdro[5].sh_offset = fo_off;
    shdro[5].sh_size = 16;  // XXX ?
    shdro[5].sh_link = 6;  // to .strtab for symbols
    shdro[5].sh_addralign = 4;
    shdro[5].sh_entsize = 16;  // XXX Elf32_Sym
    fo->seek(fo_off, SEEK_SET);
    fo->write("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16); fo_off += 16;

    while (0!=*p++) ;
    shdro[6].sh_name = ptr_diff(p, shstrtab);
    shdro[6].sh_type = Elf32_Shdr::SHT_STRTAB;
    shdro[6].sh_offset = fo_off;
    shdro[6].sh_size = 1;  // XXX ?
    shdro[6].sh_addralign = 1;
    fo->write("", 1); fo_off += 1;
#endif  /*}*/

    fo_off = ~3 & (3+ fo_off);
    fo->seek(fo_off, SEEK_SET);
    ehdro.e_shoff = fo_off;
    fo->write(shdro, sizeof(shdro));

    fo->seek(0, SEEK_SET);
    fo->write(&ehdro, sizeof(ehdro));

    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();
#undef shstrtab
}


/*************************************************************************
// unpack
**************************************************************************/

int PackVmlinuxI386::canUnpack()
{
    fi->seek(0, SEEK_SET);
    fi->readx(&ehdri, sizeof(ehdri));

    // now check the ELF header
    if (memcmp(&ehdri, "\x7f\x45\x4c\x46\x01\x01\x01", 7) // ELF 32-bit LSB
    ||  !memcmp(&ehdri.e_ident[8], "FreeBSD", 7)  // branded
    ||  ehdri.e_machine != 3  // Intel 80386
    ||  ehdri.e_version != 1  // version
    ||  ehdri.e_type != Elf32_Ehdr::ET_REL
    ||  ehdri.e_shnum < 4
    ||  (unsigned)file_size < (ehdri.e_shnum * sizeof(Elf32_Shdr) + ehdri.e_shoff)
    )
        return false;

    // find the .shstrtab section
    Elf_LE32_Shdr const *const shstrsec = getElfSections();
    if (0==shstrsec) {
        return false;
    }

    // check for .text .note .note  and sane (.sh_size + .sh_offset)
    p_note0 = p_note1 = p_text = 0;
    int j;
    Elf_LE32_Shdr *p;
    for (p= shdri, j= ehdri.e_shnum; --j>=0; ++p) {
        if ((unsigned)file_size < (p->sh_size + p->sh_offset)
        ||  shstrsec->sh_size < (5+ p->sh_name) ) {
            continue;
        }
        if (0==strcmp(".text", shstrtab + p->sh_name)) {
            p_text = p;
        }
        if (0==strcmp(".note", shstrtab + p->sh_name)) {
            if (0==p_note0) {
                p_note0 = p;
            } else
            if (0==p_note1) {
                p_note1 = p;
            }
        }
    }
    if (0==p_text || 0==p_note0 || 0==p_note1) {
        return false;
    }

    char buf[1024];
    fi->seek(p_text->sh_offset + p_text->sh_size - sizeof(buf), SEEK_SET);
    fi->readx(buf, sizeof(buf));
    if (!getPackHeader(buf, sizeof(buf)))
        return false;

    return true;
}


void PackVmlinuxI386::unpack(OutputFile *fo)
{
    unsigned char buf[5];
    PackHeader const ph_tmp(ph);

    fi->seek(p_note0->sh_offset, SEEK_SET);
    fi->readx(&buf[0], 4);
    ph.u_len = get_le32(buf);
    ph.c_len = p_note0->sh_size - 4;
    ibuf.alloc(ph.c_len);
    fi->readx(ibuf, ph.c_len);
    obuf.allocForUncompression(ph.u_len);
    decompress(ibuf, obuf, false);
    fo->write(obuf, ph.u_len);
    obuf.dealloc();
    ibuf.dealloc();

    ph = ph_tmp;
    fi->seek(p_text->sh_offset + sizeof(stub_i386_linux_kernel_vmlinux_head) -5, SEEK_SET);
    fi->readx(&buf[0], 5);
    if (0xE8!=buf[0] ||  get_le32(&buf[1]) != ph.c_len)
    {
        throwCantUnpack(".text corrupted");
    }
    ibuf.alloc(ph.c_len);
    fi->readx(ibuf, ph.c_len);
    obuf.allocForUncompression(ph.u_len);
    decompress(ibuf, obuf);

    Filter ft(ph.level);
    ft.init(ph.filter, 0);
    ft.cto = (unsigned char) ph.filter_cto;
    ft.unfilter(obuf, ph.u_len);
    fo->write(obuf, ph.u_len);
    obuf.dealloc();
    ibuf.dealloc();

    fi->seek(p_note1->sh_offset, SEEK_SET);
    fi->readx(&buf[0], 4);
    ph.u_len = get_le32(buf);
    ph.c_len = p_note1->sh_size - 4;
    ibuf.alloc(ph.c_len);
    fi->readx(ibuf, p_note1->sh_size - sizeof(ph.u_len));
    obuf.allocForUncompression(ph.u_len);
    decompress(ibuf, obuf, false);
    fo->write(obuf, ph.u_len);
    obuf.dealloc();
    ibuf.dealloc();

    ph = ph_tmp;
}

//
// Example usage within build system of Linux kernel-2.6.18:
//
//----- arch/i386/boot/compressed/Makefile
//#
//# linux/arch/i386/boot/compressed/Makefile
//#
//# create a compressed vmlinux image from the original vmlinux
//#
//
//targets := vmlinux upx-head.o upx-piggy.o
//
//LDFLAGS_vmlinux := -Ttext $(IMAGE_OFFSET) -e startup_32
//
//$(obj)/vmlinux: $(obj)/upx-head.o $(obj)/upx-piggy.o FORCE
//	$(call if_changed,ld)
//	@:
//
//$(obj)/upx-piggy.o: vmlinux FORCE
//	rm -f $@
//	upx --best -o $@ $<
//	touch $@
//
//#
//# The ORIGINAL build sequence using gzip is:
//#                   vmlinux         Elf executable at top level in tree
//#                                     (in same directory as MAINTAINERS)
//#   In arch/i386:
//#   boot/compressed/vmlinux.bin     by objcopy -O binary
//#   boot/compressed/vmlinux.bin.gz  by gzip
//#   boot/compressed/piggy.o         by ld --format binary --oformat elf32-i386
//#
//#                                   The 3 steps above create a linkable
//#                                   compressed blob.
//#   In arch/i386:
//#   boot/compressed/vmlinux         by ld head.o misc.o piggy.o
//#              boot/vmlinux.bin     by objcopy
//#              boot/bzImage         by arch/i386/boot/tools/build with
//#                                     bootsect and setup
//#
//#
//# The MODIFIED build sequence using upx is:
//#                   vmlinux         Elf executable at top level in tree
//#                                     (in same directory as MAINTAINERS)
//#   In arch/i386:
//#   boot/compressed/upx-piggy.o     by upx format vmlinux/386
//#
//#   In arch/i386/boot:
//#   boot/compressed/vmlinux         by ld upx-head.o upx-piggy.o
//#              boot/vmlinux.bin     by objcopy
//#              boot/bzImage         by arch/i386/boot/tools/build with
//#                                     bootsect and setup
//#
//-----
//
//----- arch/i386/boot/compressed/upx-head.S
//startup_32: .globl startup_32  # In: %esi=0x90000 setup data "real_mode pointer"
//  /* All code is in stub/src/i386-linux.kernel.vmlinux-head.S */
//-----

// Approximate translation for Linux 2.4.x:
// - - -
// arch/i386/Makefile: LD_FLAGS=-e startup_32
//----- arch/i386/boot/compressed/Makefile
//# linux/arch/i386/boot/compressed/Makefile
//#
//# create a compressed vmlinux image from the original vmlinux
//#
//
//HEAD = upx-head.o
//SYSTEM = $(TOPDIR)/vmlinux
//
//OBJECTS = $(HEAD)
//
//ZLDFLAGS = -e startup_32
//
//#
//# ZIMAGE_OFFSET is the load offset of the compression loader
//# BZIMAGE_OFFSET is the load offset of the high loaded compression loader
//#
//ZIMAGE_OFFSET = 0x1000
//BZIMAGE_OFFSET = 0x100000
//
//ZLINKFLAGS = -Ttext $(ZIMAGE_OFFSET) $(ZLDFLAGS)
//BZLINKFLAGS = -Ttext $(BZIMAGE_OFFSET) $(ZLDFLAGS)
//
//all: vmlinux
//
//vmlinux: upx-piggy.o $(OBJECTS)
//  $(LD) $(ZLINKFLAGS) -o vmlinux $(OBJECTS) upx-piggy.o
//
//bvmlinux: upx-piggy.o $(OBJECTS)
//  $(LD) $(BZLINKFLAGS) -o bvmlinux $(OBJECTS) upx-piggy.o
//
//upx-piggy.o:  $(SYSTEM)
//  $(RM) -f $@
//  upx --best -o $@ $<
//  touch $@
//
//clean:
//  rm -f vmlinux bvmlinux _tmp_*
//-----

//
// Example test jig:
//  $ gcc -o test-piggy -nostartfiles -nostdlib test-piggy.o piggy.o
//  $ gdb test-piggy
//  (gdb) run >dumped
//  (gdb)  /* Execute [single step, etc.; the decompressor+unfilter moves!]
//          * until reaching the 'lret' at the end of unfilter.
//          */
//  (gdb) set $pc= &dump
//  (gdb) stepi
//  (gdb) set $edx=<actual_uncompressed_length>
//  (gdb) continue
//  (gdb) q
//  $ # Compare file 'dumped' with the portion of vmlinux that made piggy.o.
//  $ dd if=vmlinux bs=<leader_size> skip=1  |  cmp - dumped
//  cmp: EOF on dumped
//  $
//----- test-piggy.S
//#include <asm/mman.h>
//#include <asm/unistd.h>
//
//dump:
//      movl $0x456789,%edx  # length  MODIFY THIS VALUE TO SUIT YOUR CASE
//      movl $0x100000,%ecx  # base
//      movl $1,%ebx         # stdout
//      movl $ __NR_write,%eax
//      int $0x80
//      nop
//      hlt
//mmap:
//      pushl %ebx
//      leal 2*4(%esp),%ebx
//      pushl $ __NR_mmap; popl %eax
//      int $0x80
//      popl %ebx
//      ret $6*4
//
//_start: .globl _start
//      nop
//      int3  # enter debugger!
//      pushl $0
//      pushl $0
//      pushl $ MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED
//      pushl $ PROT_EXEC | PROT_WRITE | PROT_READ
//      pushl $0x600000  # 6MB length
//      pushl $0x100000  # 1MB address
//      call mmap
//      leal -0x9000(%esp),%esi  # expect "lea 0x9000(%esi),%esp" later
//      push %cs
///* Fall into .text of upx-compressed vmlinux. */
//-----

/*
vi:ts=4:et
*/


