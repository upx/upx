/* p_vmlinx.cpp -- pack vmlinux ET_EXEC file (before bootsect or setup)

   This file is part of the UPX executable compressor.

   Copyright (C)      2004 John Reiser
   Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2004 Laszlo Molnar
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
   markus@oberhumer.com      ml1050@cdata.tvnet.hu

   John Reiser
   jreiser@users.sourceforge.net
 */


#include "conf.h"

#include "file.h"
#include "filter.h"
#include "packer.h"
#include "p_vmlinx.h"

static const
#include "stub/l_vmlinx.h"

/*************************************************************************
//
**************************************************************************/

PackVmlinuxI386::PackVmlinuxI386(InputFile *f) :
    super(f), shdri(NULL)
{
}

PackVmlinuxI386::~PackVmlinuxI386()
{
    delete [] shdri;
}

const int *PackVmlinuxI386::getCompressionMethods(int method, int level) const
{
    return Packer::getDefaultCompressionMethods_le32(method, level);
}


const int *PackVmlinuxI386::getFilters() const
{
    static const int filters[] = {
        0x49,
    -1 };
    return filters;
}


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
    ||   ehdri.e_phoff != sizeof(ehdri)  // Phdrs not contiguous with Ehdr
    ||  ehdri.e_phnum!=2 || ehdri.e_phentsize!=sizeof(Elf_LE32_Phdr)
    ||  ehdri.e_type!=Elf_LE32_Ehdr::ET_EXEC
    ||  0x00100000!=(0x001fffff & ehdri.e_entry) // entry not an odd 1MB
    ) {
        return false;
    }

    fi->seek(ehdri.e_phoff, SEEK_SET);
    fi->readx(phdri, sizeof(phdri));
    if (Elf_LE32_Phdr::PT_LOAD!=phdri[0].p_type || 0!=(0xfff & phdri[0].p_offset)
    ||  Elf_LE32_Phdr::PT_LOAD!=phdri[1].p_type || 0!=(0xfff & phdri[1].p_offset)
    ||  0!=(0xfff & phdri[0].p_paddr) || 0!=(0xfff & phdri[0].p_vaddr)
    ||  0!=(0xfff & phdri[1].p_paddr) || 0!=(0xfff & phdri[1].p_vaddr)
    ||  0x1000!=phdri[0].p_offset
    ||  phdri[1].p_offset!=(phdri[0].p_offset + (~0xfff & (0xfff + phdri[0].p_filesz)))
    ) {
        return false;
    }

    return true;
}

int PackVmlinuxI386::buildLoader(const Filter *ft)
{
    // prepare loader
    initLoader(nrv_loader, sizeof(nrv_loader));
    addLoader("LINUX000",
              (0x40==(0xf0 & ft->id)) ? "LXCKLLT1" : (ft->id ? "LXCALLT1" : ""),
              "LXMOVEUP",
              getDecompressor(),
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
    addLoader("LINUX990""IDENTSTR""UPX1HEAD", NULL);
    return getLoaderSize();
}


void PackVmlinuxI386::pack(OutputFile *fo)
{
    unsigned fo_off = 0;
    Elf_LE32_Ehdr ehdro;

    // NULL
    // .text(PT_LOADs) .note(1st page) .note(rest)
    // .shstrtab /* .symtab .strtab */
    Elf_LE32_Shdr shdro[1+3+1/*+2*/];
    memset(shdro, 0, sizeof(shdro));
    char const shstrtab[]= "\0.text\0.note\0.shstrtab\0.symtab\0.strtab";
    char const *p = shstrtab;

    ibuf.alloc(file_size);
    obuf.allocForCompression(file_size);

    // .e_ident, .e_machine, .e_version, .e_flags
    memcpy(&ehdro, &ehdri, sizeof(ehdro));
    ehdro.e_type = Elf_LE32_Ehdr::ET_REL;
    ehdro.e_entry = 0;
    ehdro.e_phoff = 0;
    ehdro.e_shoff = 0;  // later
    ehdro.e_phentsize = 0;
    ehdro.e_phnum = 0;
    ehdro.e_shnum = 1+3+1/*+2*/;
    ehdro.e_shstrndx = 4;
    fo->write(&ehdro, sizeof(ehdro)); fo_off+= sizeof(ehdro);

    // .data length + (.text length padded to PAGE_SIZE)
    ph.u_len = phdri[1].p_filesz + (phdri[1].p_offset - phdri[0].p_offset);
    fi->seek(phdri[0].p_offset, SEEK_SET);
    fi->readx(ibuf, ph.u_len);
    checkAlreadyPacked(ibuf + ph.u_len - 1024, 1024);

    // prepare filter
    ph.filter = 0;
    Filter ft(ph.level);
    ft.buf_len = ph.u_len;
    ft.addvalue = 0;

    compressWithFilters(&ft, 1 << 20);

    const unsigned lsize = getLoaderSize();
    MemBuffer loader(lsize);
    memcpy(loader, getLoader(), lsize);

    patchPackHeader(loader, lsize);
    patch_le32(loader, lsize, "ULEN", ph.u_len);
    patchFilter32(loader, lsize, &ft);

    while (0!=*p++) ;
    shdro[1].sh_name = p - shstrtab;
    shdro[1].sh_type = Elf_LE32_Shdr::SHT_PROGBITS;
    shdro[1].sh_flags = Elf_LE32_Shdr::SHF_ALLOC | Elf_LE32_Shdr::SHF_EXECINSTR;
    shdro[1].sh_offset = fo_off;
    shdro[1].sh_size = 1+ 4+ ph.c_len + lsize;
    shdro[1].sh_addralign = 1;

    char const call = 0xE8;  // opcode for CALL d32
    fo->write(&call, 1); fo_off+=1;
    fo->write(&ph.c_len, sizeof(ph.c_len));  fo_off += 4;  // XXX LE32
    fo->write(obuf, ph.c_len); fo_off += ph.c_len;
    fo->write(loader, lsize); fo_off += lsize;

    verifyOverlappingDecompression();

    // .note with 1st page --------------------------------
    fi->seek(0, SEEK_SET);
    ph.u_len = phdri[0].p_offset;
    fi->readx(ibuf, ph.u_len);
    compress(ibuf, obuf);

    while (0!=*p++) ;
    shdro[2].sh_name = p - shstrtab;
    shdro[2].sh_type = Elf_LE32_Shdr::SHT_NOTE;
    shdro[2].sh_offset = fo_off;
    shdro[2].sh_size = sizeof(ph.u_len) + ph.c_len;
    shdro[2].sh_addralign = 1;
    fo->write(&ph.u_len, sizeof(ph.u_len));  // XXX LE32
    fo->write(obuf, ph.c_len); fo_off += shdro[2].sh_size;

    // .note with rest     --------------------------------
    fi->seek(phdri[1].p_offset + phdri[1].p_filesz, SEEK_SET);
    ph.u_len = file_size - (phdri[1].p_offset + phdri[1].p_filesz);
    fi->readx(ibuf, ph.u_len);
    compress(ibuf, obuf);

    // while (0!=*p++) ;  // name is the same
    shdro[3].sh_name = p - shstrtab;
    shdro[3].sh_type = Elf_LE32_Shdr::SHT_NOTE;
    shdro[3].sh_offset = fo_off;
    shdro[3].sh_size = sizeof(ph.u_len) + ph.c_len;
    shdro[3].sh_addralign = 1;
    fo->write(&ph.u_len, sizeof(ph.u_len));  // XXX LE32
    fo->write(obuf, ph.c_len); fo_off += shdro[3].sh_size;

    while (0!=*p++) ;
    shdro[4].sh_name = p - shstrtab;
    shdro[4].sh_type = Elf_LE32_Shdr::SHT_STRTAB;
    shdro[4].sh_offset = fo_off;
    shdro[4].sh_size = 1+ sizeof(shstrtab);  // 1+: terminating '\0'
    shdro[4].sh_addralign = 1;
    fo->write(shstrtab, shdro[4].sh_size); fo_off += shdro[4].sh_size;

#if 0  /*{ no symbols! */
    while (0!=*p++) ;
    fo_off = ~3 & (3+ fo_off);
    shdro[5].sh_name = p - shstrtab;
    shdro[5].sh_type = Elf_LE32_Shdr::SHT_SYMTAB;
    shdro[5].sh_offset = fo_off;
    shdro[5].sh_size = 16;  // XXX ?
    shdro[5].sh_link = 6;  // to .strtab for symbols
    shdro[5].sh_addralign = 4;
    shdro[5].sh_entsize = 16;  // XXX Elf32_Sym
    fo->seek(fo_off, SEEK_SET);
    fo->write("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16); fo_off += 16;

    while (0!=*p++) ;
    shdro[6].sh_name = p - shstrtab;
    shdro[6].sh_type = Elf_LE32_Shdr::SHT_STRTAB;
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
    ||  ehdri.e_type != Elf_LE32_Ehdr::ET_REL
    ||  ehdri.e_shnum < 4
    ||  file_size < (ehdri.e_shnum * sizeof(Elf_LE32_Shdr) + ehdri.e_shoff)
    )
        return false;

    // find the .shstrtab section
    char shstrtab[40];
    Elf_LE32_Shdr *p, *p_shstrtab=0;
    shdri = new Elf_LE32_Shdr[ehdri.e_shnum];
    fi->seek(ehdri.e_shoff, SEEK_SET);
    fi->readx(shdri, ehdri.e_shnum * sizeof(Elf_LE32_Shdr));
    int j;
    for (p = shdri, j= ehdri.e_shnum; --j>=0; ++p) {
        if (Elf_LE32_Shdr::SHT_STRTAB==p->sh_type
        &&  p->sh_size <= sizeof(shstrtab)
        &&  (p->sh_size + p->sh_offset) <= file_size
        &&  (10+ p->sh_name) <= p->sh_size  // 1+ strlen(".shstrtab")
        ) {
            fi->seek(p->sh_offset, SEEK_SET);
            fi->readx(shstrtab, p->sh_size);
            if (0==strcmp(".shstrtab", &shstrtab[p->sh_name])) {
                p_shstrtab = p;
                break;
            }
        }
    }
    if (0==p_shstrtab) {
        return false;
    }

    // check for .text .note .note  and sane (.sh_size + .sh_offset)
    p_note0 = p_note1 = p_text = 0;
    for (p= shdri, j= ehdri.e_shnum; --j>=0; ++p) {
        if (file_size < (p->sh_size + p->sh_offset)
        ||  p_shstrtab->sh_size < (5+ p->sh_name) ) {
            continue;
        }
        if (0==strcmp(".text", &shstrtab[p->sh_name])) {
            p_text = p;
        }
        if (0==strcmp(".note", &shstrtab[p->sh_name])) {
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
    struct {
        unsigned char opcode;
        unsigned char d32[4];
    } call;
    PackHeader const ph_tmp(ph);

    fi->seek(p_note0->sh_offset, SEEK_SET);
    fi->readx(&ph.u_len, sizeof(ph.u_len)); // XXX LE32
    ibuf.alloc(ph.c_len = p_note0->sh_size - sizeof(ph.u_len));
    fi->readx(ibuf, ph.c_len);
    obuf.allocForUncompression(ph.u_len);
    decompress(ibuf, obuf, false);
    fo->write(obuf, ph.u_len);
    obuf.dealloc();
    ibuf.dealloc();
    
    ph = ph_tmp;
    fi->seek(p_text->sh_offset, SEEK_SET);
    fi->readx(&call, 5);
    if (0xE8!=call.opcode
    ||  *(int *)&call.d32!=(int)ph.c_len  // XXX LE32
    ) {
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
    fi->readx(&ph.u_len, sizeof(ph.u_len)); // XXX LE32
    ibuf.alloc(ph.c_len = p_note1->sh_size - sizeof(ph.u_len));
    fi->readx(ibuf, p_note1->sh_size - sizeof(ph.u_len));
    obuf.allocForUncompression(ph.u_len);
    decompress(ibuf, obuf, false);
    fo->write(obuf, ph.u_len);
    obuf.dealloc();
    ibuf.dealloc();

    ph = ph_tmp;
}

//
// Example usage within build system of Linux kernel-2.6.7:
//
//----- arch/i386/boot/compressed/Makefile
//#
//# linux/arch/i386/boot/compressed/Makefile
//#
//# create a compressed vmlinux image from the original vmlinux
//#
//
//targets		:= vmlinux upx-head.o upx-piggy.o
//
//LDFLAGS_vmlinux := -Ttext $(IMAGE_OFFSET) -e startup_32
//
//$(obj)/vmlinux: $(obj)/upx-head.o $(obj)/upx-piggy.o FORCE
//	$(call if_changed,ld)
//	@:
//
//$(obj)/upx-piggy.o: vmlinux FORCE
//	rm -f  $(obj)/upx-piggy.o
//	upx -o $(obj)/upx-piggy.o $<
//-----
//
//----- arch/i386/boot/compressed/upx-head.S
//#include <asm/segment.h>
//
//	.text
//startup_32: .globl startup_32  # In: %esi=0x90000 setup data "real_mode pointer"
//	cli  # but if it matters, then there is a race!
//
//	movl $ __BOOT_DS,%eax
//	movl %eax,%ss; movl $0x99000,%esp  # 2.6.7 setup had ss:sp of 9000:8ffe
//		/* Avoid EBDA (Extended BIOS Data Area) below 0xA0000. */
//
//	pushl $0; popf  # subsumes "cli; cld"; also clears NT for buggy BIOS
//
//	movl %eax,%ds  # all non-code segments identical
//	movl %eax,%es
//	movl %eax,%fs
//	movl %eax,%gs
//
//	movl $ 0x100000,%eax  # destination of uncompression (and entry point)
//	pushl $ __BOOT_CS
///* Fall into .text of upx-compressed vmlinux. */
//-----

//
// Example test jig:
//  gcc -o test-piggy -nostartfiles -nostdlib test-piggy.o piggy.o
//  gdb test-piggy
//  (gdb) run >dumped
//----- test-piggy.S
//#include <asm/mman.h>
//#include <asm/unistd.h>
//
//dump:
//	movl $0x456789,%edx  # length  MODIFY THIS VALUE TO SUIT YOUR CASE
//	movl $0x100000,%ecx  # base
//	movl $1,%ebx         # stdout
//	movl $ __NR_write,%eax
//	int $0x80
//	nop
//	hlt
//mmap:
//	pushl %ebx
//	leal 2*4(%esp),%ebx
//	pushl $ __NR_mmap; popl %eax
//	int $0x80
//	popl %ebx
//	ret $6*4
//
//_start: .globl _start
//	nop
//	int3  # enter debugger!
//	pushl $0
//	pushl $0
//	pushl $ MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED
//	pushl $ PROT_EXEC | PROT_WRITE | PROT_READ
//	pushl $0x600000  # 6MB
//	pushl $0x100000  # 1MB
//	call mmap
//	push %cs
///* Fall into .text of upx-compressed vmlinux. */
//-----

/*
vi:ts=4:et
*/


