/* p_lx_elf.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2018 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2018 Laszlo Molnar
   Copyright (C) 2000-2018 John F. Reiser
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
   <markus@oberhumer.com>               <ezerotven+github@gmail.com>

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
#define PT_GNU_STACK32  Elf32_Phdr::PT_GNU_STACK
#define PT_GNU_STACK64  Elf64_Phdr::PT_GNU_STACK

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

#if 0  //{ unused
static unsigned
up8(unsigned x)
{
    return ~7u & (7+ x);
}
#endif  //}

static off_t
fpad4(OutputFile *fo)
{
    off_t len = fo->st_size();
    unsigned d = 3u & (0 - len);
    unsigned zero = 0;
    fo->write(&zero, d);
    return d + len;
}

static off_t
fpad8(OutputFile *fo)
{
    off_t len = fo->st_size();
    unsigned d = 7u & (0 - len);
    upx_uint64_t zero = 0;
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

static void alloc_file_image(MemBuffer &mb, off_t size)
{
    assert(mem_size_valid_bytes(size));
    if (mb.getVoidPtr() == NULL) {
        mb.alloc(size);
    } else {
        assert(size <= (off_t) mb.getSize());
    }
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
    : super(f), e_phnum(0), dynstr(NULL),
    sz_phdrs(0), sz_elf_hdrs(0), sz_pack2(0), sz_pack2a(0),
    lg2_page(12), page_size(1u<<lg2_page), is_big(0), is_pie(0),
    xct_off(0), xct_va(0), jni_onload_va(0),
    e_machine(0), ei_class(0), ei_data(0), ei_osabi(0), osabi_note(NULL),
    o_elf_shnum(0)
{
}

PackLinuxElf::~PackLinuxElf()
{
}

void
PackLinuxElf32::PackLinuxElf32help1(InputFile *f)
{
    e_type  = get_te16(&ehdri.e_type);
    e_phnum = get_te16(&ehdri.e_phnum);
    e_shnum = get_te16(&ehdri.e_shnum);
    unsigned const e_phentsize = get_te16(&ehdri.e_phentsize);
    if (ehdri.e_ident[Elf32_Ehdr::EI_CLASS]!=Elf32_Ehdr::ELFCLASS32
    || sizeof(Elf32_Phdr) != e_phentsize
    || (Elf32_Ehdr::ELFDATA2MSB == ehdri.e_ident[Elf32_Ehdr::EI_DATA]
            && &N_BELE_RTP::be_policy != bele)
    || (Elf32_Ehdr::ELFDATA2LSB == ehdri.e_ident[Elf32_Ehdr::EI_DATA]
            && &N_BELE_RTP::le_policy != bele)) {
        e_phoff = 0;
        e_shoff = 0;
        sz_phdrs = 0;
        return;
    }
    if (0==e_phnum) throwCantUnpack("0==e_phnum");
    e_phoff = get_te32(&ehdri.e_phoff);
    if ((unsigned long)file_size < ((unsigned long long)e_phoff + e_phnum * sizeof(Elf32_Phdr))) {
        throwCantUnpack("bad e_phoff");
    }
    e_shoff = get_te32(&ehdri.e_shoff);
    if ((unsigned long)file_size < ((unsigned long long)e_shoff + e_shnum * sizeof(Elf32_Shdr))) {
        throwCantUnpack("bad e_shoff");
    }
    sz_phdrs = e_phnum * e_phentsize;

    if (f && Elf32_Ehdr::ET_DYN!=e_type) {
        unsigned const len = sz_phdrs + e_phoff;
        alloc_file_image(file_image, len);
        f->seek(0, SEEK_SET);
        f->readx(file_image, len);
        phdri= (Elf32_Phdr       *)(e_phoff + file_image);  // do not free() !!
    }
    if (f && Elf32_Ehdr::ET_DYN==e_type) {
        // The DT_SYMTAB has no designated length.  Read the whole file.
        alloc_file_image(file_image, file_size);
        f->seek(0, SEEK_SET);
        f->readx(file_image, file_size);
        phdri= (Elf32_Phdr *)(e_phoff + file_image);  // do not free() !!
        shdri= (Elf32_Shdr *)(e_shoff + file_image);  // do not free() !!
        sec_dynsym = elf_find_section_type(Elf32_Shdr::SHT_DYNSYM);
        if (sec_dynsym) {
            unsigned t = get_te32(&sec_dynsym->sh_link);
            if (e_shnum <= t)
                throwCantPack("bad dynsym->sh_link");
            sec_dynstr = &shdri[t];
        }

        Elf32_Phdr const *phdr= phdri;
        for (int j = e_phnum; --j>=0; ++phdr)
        if (Elf32_Phdr::PT_DYNAMIC==get_te32(&phdr->p_type)) {
            dynseg= (Elf32_Dyn const *)(check_pt_dynamic(phdr) + file_image);
            break;
        }
        // elf_find_dynamic() returns 0 if 0==dynseg.
        dynstr =      (char const *)elf_find_dynamic(Elf32_Dyn::DT_STRTAB);
        dynsym = (Elf32_Sym const *)elf_find_dynamic(Elf32_Dyn::DT_SYMTAB);
        gashtab = (unsigned const *)elf_find_dynamic(Elf32_Dyn::DT_GNU_HASH);
        hashtab = (unsigned const *)elf_find_dynamic(Elf32_Dyn::DT_HASH);
        jni_onload_sym = elf_lookup("JNI_OnLoad");
        if (jni_onload_sym) {
            jni_onload_va = get_te32(&jni_onload_sym->st_value);
            jni_onload_va = 0;
        }
    }
}

off_t PackLinuxElf::pack3(OutputFile *fo, Filter &ft) // return length of output
{
    unsigned disp;
    unsigned const zero = 0;
    unsigned len = sz_pack2a;  // after headers and all PT_LOAD

    unsigned const t = (4 & len) ^ ((!!xct_off)<<2);  // 0 or 4
    fo->write(&zero, t);
    len += t;

    set_te32(&disp, sz_elf_hdrs + sizeof(p_info) + sizeof(l_info) +
        (!!xct_off & !!opt->o_unix.android_shlib));  // |1 iff android shlib
    fo->write(&disp, sizeof(disp));  // offset(b_info)
    len += sizeof(disp);
    set_te32(&disp, len);  // distance back to beginning (detect dynamic reloc)
    fo->write(&disp, sizeof(disp));
    len += sizeof(disp);

    if (xct_off) {  // is_shlib
        upx_uint64_t const firstpc_va = (jni_onload_va
            ? jni_onload_va
            : elf_unsigned_dynamic(Elf32_Dyn::DT_INIT) );
        set_te32(&disp, firstpc_va - load_va);
        fo->write(&disp, sizeof(disp));  // DT_INIT.d_val
        len += sizeof(disp);

        set_te32(&disp, hatch_off);
        fo->write(&disp, sizeof(disp));  // offset(hatch)
        len += sizeof(disp);

        if (opt->o_unix.android_shlib) {
            xct_off += asl_delta;  // the extra page
        }
        set_te32(&disp, xct_off);
        fo->write(&disp, sizeof(disp));  // offset(dst for f_exp)
        len += sizeof(disp);
    }
    sz_pack2 = len;  // 0 mod 8

    super::pack3(fo, ft);  // append the decompressor
    set_te16(&linfo.l_lsize, up4(  // MATCH03: up4
    get_te16(&linfo.l_lsize) + len - sz_pack2a));

    return fpad4(fo);  // MATCH03
}

off_t PackLinuxElf32::pack3(OutputFile *fo, Filter &ft)
{
    off_t flen = super::pack3(fo, ft);  // loader follows compressed PT_LOADs
    // NOTE: PackLinuxElf::pack3  adjusted xct_off for the extra page

    unsigned v_hole = sz_pack2 + lsize;
    set_te32(&elfout.phdr[0].p_filesz, v_hole);
    set_te32(&elfout.phdr[0].p_memsz,  v_hole);
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
    flen = fpad4(fo);

    set_te32(&elfout.phdr[0].p_filesz, sz_pack2 + lsize);
    set_te32(&elfout.phdr[0].p_memsz,  sz_pack2 + lsize);
    if (0==xct_off) { // not shared library; adjust PT_LOAD
        // .p_align can be big for segments, but Linux uses 4KiB pages.
        // This allows [vvar], [vdso], etc to sneak into the gap
        // between end_text and data, which we wish to prevent
        // because the expanded program will use that space.
        // So: pretend 4KiB pages.
        unsigned pm = (Elf64_Ehdr::EM_PPC64 == e_machine)
            ? page_mask  // reducing to 4KiB DOES NOT WORK ??
            : ((~(unsigned)0)<<12);
        pm = page_mask;  // Revert until consequences can be analyzed
        v_hole = pm & (~pm + v_hole + get_te32(&elfout.phdr[0].p_vaddr));
        set_te32(&elfout.phdr[1].p_vaddr, v_hole);
        set_te32(&elfout.phdr[1].p_align, ((unsigned)0) - pm);
        elfout.phdr[1].p_paddr = elfout.phdr[1].p_vaddr;
        elfout.phdr[1].p_offset = 0;
        set_te32(&elfout.phdr[1].p_memsz, getbrk(phdri, e_phnum) - v_hole);
        set_te32(&elfout.phdr[1].p_flags, Elf32_Phdr::PF_W|Elf32_Phdr::PF_R);
    }
    if (0!=xct_off) {  // shared library
        Elf32_Phdr *phdr = (Elf32_Phdr *)lowmem.subref(
                "bad e_phoff", e_phoff, e_phnum * sizeof(Elf32_Phdr));
        unsigned off = fo->st_size();
        unsigned off_init = 0;  // where in file
        unsigned va_init = sz_pack2;   // virtual address
        so_slide = 0;
        for (unsigned j = 0; j < e_phnum; ++j, ++phdr) {
            unsigned const len  = get_te32(&phdr->p_filesz);
            unsigned const ioff = get_te32(&phdr->p_offset);
            unsigned       align= get_te32(&phdr->p_align);
            unsigned const type = get_te32(&phdr->p_type);
            if (Elf32_Phdr::PT_INTERP==type) {
                // Rotate to highest position, so it can be lopped
                // by decrementing e_phnum.
                memcpy((unsigned char *)ibuf, phdr, sizeof(*phdr));
                memmove(phdr, 1+phdr, j * sizeof(*phdr));  // overlapping
                memcpy(&phdr[j], (unsigned char *)ibuf, sizeof(*phdr));
                --phdr;
                set_te16(&ehdri.e_phnum, --e_phnum);
                continue;
            }
            if (PT_LOAD32==type) {
                if (xct_off < ioff) {  // Slide up non-first PT_LOAD.
                    if ((1u<<12) < align) {
                        align = 1u<<12;
                        set_te32(&phdr->p_align, align);
                    }
                    off += (align-1) & (ioff - off);
                    fo->seek(  off, SEEK_SET);
                    fo->write(ioff + file_image, len);
                    so_slide = off - ioff;
                    set_te32(&phdr->p_offset, so_slide + ioff);
                }
                else {  // Change length of first PT_LOAD.
                    va_init += get_te32(&phdr->p_vaddr);
                    set_te32(&phdr->p_filesz, sz_pack2 + lsize);
                    set_te32(&phdr->p_memsz,  sz_pack2 + lsize);
                }
                continue;  // all done with this PT_LOAD
            }
            // Compute new offset of &DT_INIT.d_val.
            if (Elf32_Phdr::PT_DYNAMIC==type) {
                off_init = ioff + ((xct_off < ioff) ? so_slide : 0);
                Elf32_Dyn *dyn = (Elf32_Dyn *)(void *)&file_image[ioff];
                for (int j2 = len; j2 > 0; ++dyn, j2 -= sizeof(*dyn)) {
                    if (Elf32_Dyn::DT_INIT==get_te32(&dyn->d_tag)) {
                        unsigned const t = (unsigned char *)&dyn->d_val -
                                           (unsigned char *)&file_image[ioff];
                        off_init += t;
                        break;
                    }
                }
                // fall through to relocate .p_offset
            }
            if (xct_off < ioff)
                set_te32(&phdr->p_offset, so_slide + ioff);
        }  // end each Phdr

        if (off_init) {  // change DT_INIT.d_val
            fo->seek(off_init, SEEK_SET);
            va_init |= (Elf32_Ehdr::EM_ARM==e_machine);  // THUMB mode
            unsigned word; set_te32(&word, va_init);
            fo->rewrite(&word, sizeof(word));
            flen = fo->seek(0, SEEK_END);
        }
        if (opt->o_unix.android_shlib) {
            // Update {DYNAMIC}.sh_offset by so_slide.
            Elf32_Shdr *shdr = (Elf32_Shdr *)lowmem.subref(
                    "bad e_shoff", xct_off - asl_delta, e_shnum * sizeof(Elf32_Shdr));
            for (unsigned j = 0; j < e_shnum; ++shdr, ++j) {
                unsigned sh_type = get_te32(&shdr->sh_type);
                if (Elf32_Shdr::SHT_DYNAMIC == get_te32(&shdr->sh_type)) {
                    unsigned offset = get_te32(&shdr->sh_offset);
                    set_te32(&shdr->sh_offset, so_slide + offset );
                    fo->seek(j * sizeof(Elf32_Shdr) + xct_off - asl_delta, SEEK_SET);
                    fo->rewrite(shdr, sizeof(*shdr));
                    fo->seek(0, SEEK_END);
                }
                if (Elf32_Shdr::SHT_REL == sh_type
                &&  n_jmp_slot
                &&  !strcmp(".rel.plt", get_te32(&shdr->sh_name) + shstrtab)) {
                    unsigned f_off = elf_get_offset_from_address(plt_off);
                    fo->seek(so_slide + f_off, SEEK_SET);  // FIXME: assumes PT_LOAD[1]
                    fo->rewrite(&file_image[f_off], n_jmp_slot * sizeof(void *));
                 }
            }
        }
        else { // !opt->o_unix.android_shlib)
            ehdri.e_shnum = 0;
            ehdri.e_shoff = 0;
            ehdri.e_shstrndx = 0;
        }
    }
    return flen;
}

off_t PackLinuxElf64::pack3(OutputFile *fo, Filter &ft)
{
    unsigned flen = super::pack3(fo, ft);  // loader follows compressed PT_LOADs
    // NOTE: PackLinuxElf::pack3  adjusted xct_off for the extra page

    unsigned v_hole = sz_pack2 + lsize;
    set_te64(&elfout.phdr[0].p_filesz, v_hole);
    set_te64(&elfout.phdr[0].p_memsz,  v_hole);
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
    flen = fpad4(fo);

    set_te64(&elfout.phdr[0].p_filesz, sz_pack2 + lsize);
    set_te64(&elfout.phdr[0].p_memsz,  sz_pack2 + lsize);
    if (0==xct_off) { // not shared library; adjust PT_LOAD
        // On amd64: (2<<20)==.p_align, but Linux uses 4KiB pages.
        // This allows [vvar], [vdso], etc to sneak into the gap
        // between end_text and data, which we wish to prevent
        // because the expanded program will use that space.
        // So: pretend 4KiB pages.
        upx_uint64_t const pm = (
               Elf64_Ehdr::EM_X86_64 ==e_machine
            || Elf64_Ehdr::EM_AARCH64==e_machine
            //|| Elf64_Ehdr::EM_PPC64  ==e_machine  /* DOES NOT WORK! */
            )
            ? ((~(upx_uint64_t)0)<<12)
            : page_mask;
        v_hole = pm & (~pm + v_hole + get_te64(&elfout.phdr[0].p_vaddr));
        set_te64(&elfout.phdr[1].p_vaddr, v_hole);
        set_te64(&elfout.phdr[1].p_align, ((upx_uint64_t)0) - pm);
        elfout.phdr[1].p_paddr = elfout.phdr[1].p_vaddr;
        elfout.phdr[1].p_offset = 0;
        set_te64(&elfout.phdr[1].p_memsz, getbrk(phdri, e_phnum) - v_hole);
        set_te32(&elfout.phdr[1].p_flags, Elf32_Phdr::PF_W|Elf32_Phdr::PF_R);
    }
    if (0!=xct_off) {  // shared library
        Elf64_Phdr *phdr = (Elf64_Phdr *)lowmem.subref(
                "bad e_phoff", e_phoff, e_phnum * sizeof(Elf64_Phdr));
        unsigned off = fo->st_size();
        unsigned off_init = 0;  // where in file
        upx_uint64_t va_init = sz_pack2;   // virtual address
        so_slide = 0;
        for (unsigned j = 0; j < e_phnum; ++j, ++phdr) {
            upx_uint64_t const len  = get_te64(&phdr->p_filesz);
            upx_uint64_t const ioff = get_te64(&phdri[j].p_offset);
            upx_uint64_t       align= get_te64(&phdr->p_align);
            unsigned const type = get_te32(&phdr->p_type);
            if (Elf64_Phdr::PT_INTERP==type) {
                // Rotate to highest position, so it can be lopped
                // by decrementing e_phnum.
                memcpy((unsigned char *)ibuf, phdr, sizeof(*phdr));
                memmove(phdr, 1+phdr, j * sizeof(*phdr));  // overlapping
                memcpy(&phdr[j], (unsigned char *)ibuf, sizeof(*phdr));
                --phdr;
                set_te16(&ehdri.e_phnum, --e_phnum);
                continue;
            }
            if (PT_LOAD64 == type) {
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
                    fo->seek(  off, SEEK_SET);
                    fo->write(&file_image[ioff], len);
                    so_slide = off - ioff;
                    set_te64(&phdr->p_offset, so_slide + ioff);
                }
                else {  // Change length of first PT_LOAD.
                    va_init += get_te64(&phdr->p_vaddr);
                    set_te64(&phdr->p_filesz, sz_pack2 + lsize);
                    set_te64(&phdr->p_memsz,  sz_pack2 + lsize);
                }
                continue;  // all done with this PT_LOAD
            }
            // Compute new offset of &DT_INIT.d_val.
            if (Elf64_Phdr::PT_DYNAMIC==type) {
                off_init = ioff + ((xct_off < ioff) ? so_slide : 0);
                Elf64_Dyn *dyn = (Elf64_Dyn *)&file_image[ioff];
                for (int j2 = len; j2 > 0; ++dyn, j2 -= sizeof(*dyn)) {
                    if (Elf64_Dyn::DT_INIT==get_te64(&dyn->d_tag)) {
                        unsigned const t = (unsigned char *)&dyn->d_val -
                                           (unsigned char *)&file_image[ioff];
                        off_init += t;
                        break;
                    }
                }
                // fall through to relocate .p_offset
            }
            if (xct_off < ioff)
                set_te64(&phdr->p_offset, so_slide + ioff);
        }  // end each Phdr

        if (off_init) {  // change DT_INIT.d_val
            fo->seek(off_init, SEEK_SET);
            upx_uint64_t word; set_te64(&word, va_init);
            fo->rewrite(&word, sizeof(word));
            flen = fo->seek(0, SEEK_END);
        }
        if (opt->o_unix.android_shlib) {
            // Update {DYNAMIC}.sh_offset by so_slide.
            Elf64_Shdr *shdr = (Elf64_Shdr *)lowmem.subref(
                    "bad e_shoff", xct_off - asl_delta, e_shnum * sizeof(Elf64_Shdr));
            for (unsigned j = 0; j < e_shnum; ++shdr, ++j) {
                unsigned sh_type = get_te32(&shdr->sh_type);
                if (Elf64_Shdr::SHT_DYNAMIC == sh_type) {
                    upx_uint64_t offset = get_te64(&shdr->sh_offset);
                    set_te64(&shdr->sh_offset, so_slide + offset);
                    fo->seek((j * sizeof(Elf64_Shdr)) + xct_off - asl_delta, SEEK_SET);
                    fo->rewrite(shdr, sizeof(*shdr));
                    fo->seek(0, SEEK_END);
                }
                if (Elf64_Shdr::SHT_RELA == sh_type
                &&  n_jmp_slot
                &&  !strcmp(".rela.plt", get_te32(&shdr->sh_name) + shstrtab)) {
                    upx_uint64_t f_off = elf_get_offset_from_address(plt_off);
                    fo->seek(so_slide + f_off, SEEK_SET);  // FIXME: assumes PT_LOAD[1]
                    fo->rewrite(&file_image[f_off], n_jmp_slot * sizeof(void *));
                }
            }
        }
        else { // !opt->o_unix.android_shlib)
            ehdri.e_shnum = 0;
            ehdri.e_shoff = 0;
            ehdri.e_shstrndx = 0;
        }
    }
    return flen;
}

void
PackLinuxElf::addStubEntrySections(Filter const *)
{
    bool all_pages = opt->o_unix.unmap_all_pages | is_big;
    addLoader("ELFMAINX", NULL);
    if (hasLoaderSection("ELFMAINXu")) {
            // brk() trouble if static
        all_pages |= (Elf32_Ehdr::EM_ARM==e_machine && 0x8000==load_va);
        addLoader("ELFMAINXu", NULL);
    }
   //addLoader(getDecompressorSections(), NULL);
    addLoader(
        ( M_IS_NRV2E(ph.method) ? "NRV_HEAD,NRV2E,NRV_TAIL"
        : M_IS_NRV2D(ph.method) ? "NRV_HEAD,NRV2D,NRV_TAIL"
        : M_IS_NRV2B(ph.method) ? "NRV_HEAD,NRV2B,NRV_TAIL"
        : M_IS_LZMA(ph.method)  ? "LZMA_ELF00,LZMA_DEC20,LZMA_DEC30"
        : NULL), NULL);
    if (hasLoaderSection("CFLUSH"))
        addLoader("CFLUSH");
    addLoader("ELFMAINY,IDENTSTR", NULL);
    if (hasLoaderSection("ELFMAINZe")) { // ppc64 big-endian only
        addLoader("ELFMAINZe", NULL);
    }
    addLoader("+40,ELFMAINZ", NULL);
    if (hasLoaderSection("ANDMAJNZ")) { // Android trouble with args to DT_INIT
        if (opt->o_unix.android_shlib) {
            addLoader("ANDMAJNZ", NULL);  // constant PAGE_SIZE
        }
        else {
            addLoader("ELFMAJNZ", NULL);  // PAGE_SIZE from AT_PAGESZ
        }
        addLoader("ELFMAKNZ", NULL);
    }
    if (hasLoaderSection("ELFMAINZu")) {
        addLoader("ELFMAINZu", NULL);
    }
    addLoader("FOLDEXEC", NULL);
}


void PackLinuxElf::defineSymbols(Filter const *)
{
    linker->defineSymbol("O_BINFO", (!!opt->o_unix.is_ptinterp) | o_binfo);
}

void PackLinuxElf32::defineSymbols(Filter const *ft)
{
    PackLinuxElf::defineSymbols(ft);
}

void PackLinuxElf64::defineSymbols(Filter const *ft)
{
    PackLinuxElf::defineSymbols(ft);
}

PackLinuxElf32::PackLinuxElf32(InputFile *f)
    : super(f), phdri(NULL), shdri(NULL),
    gnu_stack(NULL), note_body(NULL),
    page_mask(~0u<<lg2_page),
    dynseg(NULL), hashtab(NULL), gashtab(NULL), dynsym(NULL),
    jni_onload_sym(NULL),
    shstrtab(NULL),
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
}

PackLinuxElf64::PackLinuxElf64(InputFile *f)
    : super(f), phdri(NULL), shdri(NULL),
    gnu_stack(NULL), note_body(NULL),
    page_mask(~0ull<<lg2_page),
    dynseg(NULL), hashtab(NULL), gashtab(NULL), dynsym(NULL),
    jni_onload_sym(NULL),
    shstrtab(NULL),
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
}

// FIXME: should be templated with PackLinuxElf32help1
void
PackLinuxElf64::PackLinuxElf64help1(InputFile *f)
{
    e_type  = get_te16(&ehdri.e_type);
    e_phnum = get_te16(&ehdri.e_phnum);
    e_shnum = get_te16(&ehdri.e_shnum);
    unsigned const e_phentsize = get_te16(&ehdri.e_phentsize);
    if (ehdri.e_ident[Elf64_Ehdr::EI_CLASS]!=Elf64_Ehdr::ELFCLASS64
    || sizeof(Elf64_Phdr) != e_phentsize
    || (Elf64_Ehdr::ELFDATA2MSB == ehdri.e_ident[Elf64_Ehdr::EI_DATA]
            && &N_BELE_RTP::be_policy != bele)
    || (Elf64_Ehdr::ELFDATA2LSB == ehdri.e_ident[Elf64_Ehdr::EI_DATA]
            && &N_BELE_RTP::le_policy != bele)) {
        e_phoff = 0;
        e_shoff = 0;
        sz_phdrs = 0;
        return;
    }
    if (0==e_phnum) throwCantUnpack("0==e_phnum");
    e_phoff = get_te64(&ehdri.e_phoff);
    if ((unsigned long)file_size < (e_phoff + e_phnum * sizeof(Elf64_Phdr))) {
        throwCantUnpack("bad e_phoff");
    }
    e_shoff = get_te64(&ehdri.e_shoff);
    if ((unsigned long)file_size < (e_shoff + e_shnum * sizeof(Elf64_Shdr))) {
        throwCantUnpack("bad e_shoff");
    }
    sz_phdrs = e_phnum * e_phentsize;

    if (f && Elf64_Ehdr::ET_DYN!=e_type) {
        unsigned const len = sz_phdrs + e_phoff;
        alloc_file_image(file_image, len);
        f->seek(0, SEEK_SET);
        f->readx(file_image, len);
        phdri= (Elf64_Phdr       *)(e_phoff + file_image);  // do not free() !!
    }
    if (f && Elf64_Ehdr::ET_DYN==e_type) {
        // The DT_SYMTAB has no designated length.  Read the whole file.
        alloc_file_image(file_image, file_size);
        f->seek(0, SEEK_SET);
        f->readx(file_image, file_size);
        phdri= (Elf64_Phdr *)(e_phoff + file_image);  // do not free() !!
        shdri= (Elf64_Shdr *)(e_shoff + file_image);  // do not free() !!
        sec_dynsym = elf_find_section_type(Elf64_Shdr::SHT_DYNSYM);
        if (sec_dynsym) {
            unsigned t = get_te32(&sec_dynsym->sh_link);
            if (e_shnum <= t)
                throwCantPack("bad dynsym->sh_link");
            sec_dynstr = &shdri[t];
        }

        Elf64_Phdr const *phdr= phdri;
        for (int j = e_phnum; --j>=0; ++phdr)
        if (Elf64_Phdr::PT_DYNAMIC==get_te64(&phdr->p_type)) {
            dynseg= (Elf64_Dyn const *)(check_pt_dynamic(phdr) + file_image);
            break;
        }
        // elf_find_dynamic() returns 0 if 0==dynseg.
        dynstr =      (char const *)elf_find_dynamic(Elf64_Dyn::DT_STRTAB);
        dynsym = (Elf64_Sym const *)elf_find_dynamic(Elf64_Dyn::DT_SYMTAB);
        gashtab = (unsigned const *)elf_find_dynamic(Elf64_Dyn::DT_GNU_HASH);
        hashtab = (unsigned const *)elf_find_dynamic(Elf64_Dyn::DT_HASH);
        jni_onload_sym = elf_lookup("JNI_OnLoad");
        if (jni_onload_sym) {
            jni_onload_va = get_te64(&jni_onload_sym->st_value);
            jni_onload_va = 0;
        }
    }
}

Linker* PackLinuxElf64amd::newLinker() const
{
    return new ElfLinkerAMD64;
}

Linker* PackLinuxElf64arm::newLinker() const
{
    return new ElfLinkerArm64LE;
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
PackLinuxElf64ppcle::getFilters() const
{
    static const int filters[] = {
        0xd0,
    FT_END };
    return filters;
}

int const *
PackLinuxElf64ppc::getFilters() const
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

int const *
PackLinuxElf64arm::getFilters() const
{
    static const int filters[] = {
        0x52,
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
    unsigned start = linker->getSymbolOffset("_start");
    unsigned vbase = get_te32(&elfout.phdr[0].p_vaddr);
    set_te32(&elfout.ehdr.e_entry, start + sz_pack2 + vbase);
}

void PackLinuxElf64::updateLoader(OutputFile * /*fo*/)
{
    if (xct_off) {
        return;  // FIXME elfout has no values at all
    }
    upx_uint64_t const vbase = get_te64(&elfout.phdr[0].p_vaddr);
    unsigned start = linker->getSymbolOffset("_start");

    if (get_te16(&elfout.ehdr.e_machine)==Elf64_Ehdr::EM_PPC64
    &&  elfout.ehdr.e_ident[Elf64_Ehdr::EI_DATA]==Elf64_Ehdr::ELFDATA2MSB) {
        unsigned descr = linker->getSymbolOffset("entry_descr");

        // External relocation of PPC64 function descriptor.
        upx_uint64_t dot_entry = start + sz_pack2 + vbase;
        upx_byte *p = getLoader();

        set_te64(&p[descr], dot_entry);
        set_te64(&elfout.ehdr.e_entry, descr + sz_pack2 + vbase);
    }
    else {
        set_te64(&elfout.ehdr.e_entry, start + sz_pack2 + vbase);
    }
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

PackLinuxElf64ppcle::PackLinuxElf64ppcle(InputFile *f)
    : super(f), lg2_page(16), page_size(1u<<lg2_page)
{
    e_machine = Elf64_Ehdr::EM_PPC64;
    ei_class  = Elf64_Ehdr::ELFCLASS64;
    ei_data   = Elf64_Ehdr::ELFDATA2LSB;
    ei_osabi  = Elf64_Ehdr::ELFOSABI_LINUX;
}

PackLinuxElf64ppc::PackLinuxElf64ppc(InputFile *f)
    : super(f), lg2_page(16), page_size(1u<<lg2_page)
{
    e_machine = Elf64_Ehdr::EM_PPC64;
    ei_class  = Elf64_Ehdr::ELFCLASS64;
    ei_data   = Elf64_Ehdr::ELFDATA2MSB;
    ei_osabi  = Elf32_Ehdr::ELFOSABI_LINUX;
}

PackLinuxElf64ppcle::~PackLinuxElf64ppcle()
{
}

PackLinuxElf64ppc::~PackLinuxElf64ppc()
{
}

Linker* PackLinuxElf64ppcle::newLinker() const
{
    return new ElfLinkerPpc64le;
}

Linker* PackLinuxElf64ppc::newLinker() const
{
    return new ElfLinkerPpc64;
}

PackLinuxElf64amd::PackLinuxElf64amd(InputFile *f)
    : super(f)
{
    e_machine = Elf64_Ehdr::EM_X86_64;
    ei_class = Elf64_Ehdr::ELFCLASS64;
    ei_data = Elf64_Ehdr::ELFDATA2LSB;
    ei_osabi  = Elf32_Ehdr::ELFOSABI_LINUX;
}

PackLinuxElf64arm::PackLinuxElf64arm(InputFile *f)
    : super(f)
{
    e_machine = Elf64_Ehdr::EM_AARCH64;
    ei_class = Elf64_Ehdr::ELFCLASS64;
    ei_data = Elf64_Ehdr::ELFDATA2LSB;
    ei_osabi  = Elf32_Ehdr::ELFOSABI_LINUX;
}

PackLinuxElf64amd::~PackLinuxElf64amd()
{
}

PackLinuxElf64arm::~PackLinuxElf64arm()
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
    addLoader("FOLDEXEC", NULL);
}

void PackLinuxElf32x86::defineSymbols(Filter const *const ft)
{
    PackLinuxElf32::defineSymbols(ft);

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
    unsigned char *const cprLoader = New(unsigned char, sizeof(h) + h.sz_cpr);
    {
    unsigned h_sz_cpr = h.sz_cpr;
    int r = upx_compress(uncLoader, h.sz_unc, sizeof(h) + cprLoader, &h_sz_cpr,
        NULL, ph.method, 10, NULL, NULL );
    h.sz_cpr = h_sz_cpr;
    if (r != UPX_E_OK || h.sz_cpr >= h.sz_unc)
        throwInternalError("loader compression failed");
    }
#if 0  //{  debugging only
    if (M_IS_LZMA(ph.method)) {
        ucl_uint tmp_len = h.sz_unc;  // LZMA uses this as EOF
        unsigned char *tmp = New(unsigned char, tmp_len);
        memset(tmp, 0, tmp_len);
        int r = upx_decompress(sizeof(h) + cprLoader, h.sz_cpr, tmp, &tmp_len, h.b_method, NULL);
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
    unsigned char *const cprLoader = New(unsigned char, sizeof(h) + h.sz_cpr);
    {
    unsigned h_sz_cpr = h.sz_cpr;
    int r = upx_compress(uncLoader, h.sz_unc, sizeof(h) + cprLoader, &h_sz_cpr,
        NULL, ph.method, 10, NULL, NULL );
    h.sz_cpr = h_sz_cpr;
    if (r != UPX_E_OK || h.sz_cpr >= h.sz_unc)
        throwInternalError("loader compression failed");
    }
#if 0  //{  debugging only
    if (M_IS_LZMA(ph.method)) {
        ucl_uint tmp_len = h.sz_unc;  // LZMA uses this as EOF
        unsigned char *tmp = New(unsigned char, tmp_len);
        memset(tmp, 0, tmp_len);
        int r = upx_decompress(sizeof(h) + cprLoader, h.sz_cpr, tmp, &tmp_len, h.b_method, NULL);
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
PackLinuxElf64amd::defineSymbols(Filter const *ft)
{
    PackLinuxElf64::defineSymbols(ft);
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
#include "stub/arm.v5a-linux.elf-entry.h"
static const
#include "stub/arm.v5a-linux.elf-fold.h"
static const
#include "stub/arm.v5t-linux.shlib-init.h"

static const
#include "stub/arm.v4a-linux.elf-entry.h"
static const
#include "stub/arm.v4a-linux.elf-fold.h"
#if 0
static const
#include "stub/arm.v4a-linux.shlib-init.h"
#endif

static const
#include "stub/armeb.v4a-linux.elf-entry.h"
static const
#include "stub/armeb.v4a-linux.elf-fold.h"

#include "mem.h"

void
PackLinuxElf32armBe::buildLoader(Filter const *ft)
{
    buildLinuxLoader(
        stub_armeb_v4a_linux_elf_entry, sizeof(stub_armeb_v4a_linux_elf_entry),
        stub_armeb_v4a_linux_elf_fold,  sizeof(stub_armeb_v4a_linux_elf_fold), ft);
}

void
PackLinuxElf32armLe::buildLoader(Filter const *ft)
{
    if (Elf32_Ehdr::ELFOSABI_LINUX==ei_osabi) {

        if (0!=xct_off) {  // shared library
            buildLinuxLoader(
                stub_arm_v5t_linux_shlib_init, sizeof(stub_arm_v5t_linux_shlib_init),
                NULL,                      0,                                ft );
            return;
        }
        buildLinuxLoader(
            stub_arm_v5a_linux_elf_entry, sizeof(stub_arm_v5a_linux_elf_entry),
            stub_arm_v5a_linux_elf_fold,  sizeof(stub_arm_v5a_linux_elf_fold), ft);
    }
    else {
        buildLinuxLoader(
            stub_arm_v4a_linux_elf_entry, sizeof(stub_arm_v4a_linux_elf_entry),
            stub_arm_v4a_linux_elf_fold,  sizeof(stub_arm_v4a_linux_elf_fold), ft);
    }
}

static const
#include "stub/mipsel.r3000-linux.elf-entry.h"
static const
#include "stub/mipsel.r3000-linux.elf-fold.h"
static const
#include "stub/mipsel.r3000-linux.shlib-init.h"

void
PackLinuxElf32mipsel::buildLoader(Filter const *ft)
{
    if (0!=xct_off) {  // shared library
        buildLinuxLoader(
            stub_mipsel_r3000_linux_shlib_init, sizeof(stub_mipsel_r3000_linux_shlib_init),
            NULL,                        0,                                 ft );
        return;
    }
    buildLinuxLoader(
        stub_mipsel_r3000_linux_elf_entry, sizeof(stub_mipsel_r3000_linux_elf_entry),
        stub_mipsel_r3000_linux_elf_fold,  sizeof(stub_mipsel_r3000_linux_elf_fold), ft);
}

static const
#include "stub/mips.r3000-linux.elf-entry.h"
static const
#include "stub/mips.r3000-linux.elf-fold.h"
static const
#include "stub/mips.r3000-linux.shlib-init.h"

void
PackLinuxElf32mipseb::buildLoader(Filter const *ft)
{
    if (0!=xct_off) {  // shared library
        buildLinuxLoader(
            stub_mips_r3000_linux_shlib_init, sizeof(stub_mips_r3000_linux_shlib_init),
            NULL,                        0,                                 ft );
        return;
    }
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
#include "stub/powerpc64le-linux.elf-entry.h"
static const
#include "stub/powerpc64le-linux.elf-fold.h"

void
PackLinuxElf64ppcle::buildLoader(const Filter *ft)
{
    buildLinuxLoader(
        stub_powerpc64le_linux_elf_entry, sizeof(stub_powerpc64le_linux_elf_entry),
        stub_powerpc64le_linux_elf_fold,  sizeof(stub_powerpc64le_linux_elf_fold), ft);
}

static const
#include "stub/powerpc64-linux.elf-entry.h"
static const
#include "stub/powerpc64-linux.elf-fold.h"

void
PackLinuxElf64ppc::buildLoader(const Filter *ft)
{
    buildLinuxLoader(
        stub_powerpc64_linux_elf_entry, sizeof(stub_powerpc64_linux_elf_entry),
        stub_powerpc64_linux_elf_fold,  sizeof(stub_powerpc64_linux_elf_fold), ft);
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

static const
#include "stub/arm64-linux.elf-entry.h"
static const
#include "stub/arm64-linux.elf-fold.h"
static const
#include "stub/arm64-linux.shlib-init.h"

void
PackLinuxElf64arm::buildLoader(const Filter *ft)
{
    if (0!=xct_off) {  // shared library
        buildLinuxLoader(
            stub_arm64_linux_shlib_init, sizeof(stub_arm64_linux_shlib_init),
            NULL,                        0,                                 ft );
        return;
    }
    buildLinuxLoader(
        stub_arm64_linux_elf_entry, sizeof(stub_arm64_linux_elf_entry),
        stub_arm64_linux_elf_fold,  sizeof(stub_arm64_linux_elf_fold), ft);
}

Elf32_Phdr const *
PackLinuxElf32::elf_find_ptype(unsigned type, Elf32_Phdr const *phdr, unsigned phnum)
{
    for (unsigned j = 0; j < phnum; ++j, ++phdr) {
        if (type == get_te32(&phdr->p_type)) {
            return phdr;
        }
    }
    return 0;
}

Elf64_Phdr const *
PackLinuxElf64::elf_find_ptype(unsigned type, Elf64_Phdr const *phdr, unsigned phnum)
{
    for (unsigned j = 0; j < phnum; ++j, ++phdr) {
        if (type == get_te32(&phdr->p_type)) {
            return phdr;
        }
    }
    return 0;
}

Elf32_Shdr const *PackLinuxElf32::elf_find_section_name(
    char const *const name
) const
{
    Elf32_Shdr const *shdr = shdri;
    int j = e_shnum;
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
    int j = e_shnum;
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
    int j = e_shnum;
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
    int j = e_shnum;
    for (; 0 <=--j; ++shdr) {
        if (type==get_te32(&shdr->sh_type)) {
            return shdr;
        }
    }
    return 0;
}

bool PackLinuxElf64::calls_crt1(Elf64_Rela const *rela, int sz)
{
    for (; 0 < sz; (sz -= sizeof(Elf64_Rela)), ++rela) {
        unsigned const symnum = get_te64(&rela->r_info) >> 32;
        char const *const symnam = get_te32(&dynsym[symnum].st_name) + dynstr;
        if (0==strcmp(symnam, "__libc_start_main")  // glibc
        ||  0==strcmp(symnam, "__libc_init")  // Android
        ||  0==strcmp(symnam, "__uClibc_main")
        ||  0==strcmp(symnam, "__uClibc_start_main"))
            return true;
    }
    return false;
}

bool PackLinuxElf32::calls_crt1(Elf32_Rel const *rel, int sz)
{
    for (; 0 < sz; (sz -= sizeof(Elf32_Rel)), ++rel) {
        unsigned const symnum = get_te32(&rel->r_info) >> 8;
        char const *const symnam = get_te32(&dynsym[symnum].st_name) + dynstr;
        if (0==strcmp(symnam, "__libc_start_main")  // glibc
        ||  0==strcmp(symnam, "__libc_init")  // Android
        ||  0==strcmp(symnam, "__uClibc_main")
        ||  0==strcmp(symnam, "__uClibc_start_main"))
            return true;
    }
    return false;
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
    if (e_phoff != sizeof(*ehdr)) {// Phdrs not contiguous with Ehdr
        throwCantPack("non-contiguous Ehdr/Phdr; try '--force-execve'");
        return false;
    }

    unsigned char osabi0 = u.buf[Elf32_Ehdr::EI_OSABI];
    // The first PT_LOAD32 must cover the beginning of the file (0==p_offset).
    Elf32_Phdr const *phdr = phdri;
    note_size = 0;
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        if (j >= 14) {
            throwCantPack("too many Elf32_Phdr; try '--force-execve'");
            return false;
        }
        unsigned const p_type = get_te32(&phdr->p_type);
        unsigned const p_offset = get_te32(&phdr->p_offset);
        if (1!=exetype && PT_LOAD32 == p_type) { // 1st PT_LOAD
            exetype = 1;
            load_va = get_te32(&phdr->p_vaddr);  // class data member

            // Cast on next line is to avoid a compiler bug (incorrect complaint) in
            // Microsoft (R) C/C++ Optimizing Compiler Version 19.00.24215.1 for x64
            // error C4319: '~': zero extending 'unsigned int' to 'upx_uint64_t' of greater size
            unsigned const off = ~page_mask & (unsigned)load_va;

            if (off && off == p_offset) { // specific hint
                throwCantPack("Go-language PT_LOAD: try hemfix.c, or try '--force-execve'");
                // Fixing it inside upx fails because packExtent() reads original file.
                return false;
            }
            if (0 != p_offset) { // 1st PT_LOAD must cover Ehdr and Phdr
                throwCantPack("first PT_LOAD.p_offset != 0; try '--force-execve'");
                return false;
            }
            hatch_off = ~3u & (3+ get_te32(&phdr->p_memsz));
        }
        if (PT_NOTE32 == p_type) {
            unsigned const x = get_te32(&phdr->p_memsz);
            if ( sizeof(elfout.notes) < x  // beware overflow of note_size
            ||  (sizeof(elfout.notes) < (note_size += x)) ) {
                throwCantPack("PT_NOTEs too big; try '--force-execve'");
                return false;
            }
            if (osabi_note && Elf32_Ehdr::ELFOSABI_NONE==osabi0) { // Still seems to be generic.
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
    }
    if (Elf32_Ehdr::ELFOSABI_NONE ==osabi0
    ||  Elf32_Ehdr::ELFOSABI_LINUX==osabi0) { // No EI_OSBAI, no PT_NOTE.
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
        // The DT_SYMTAB has no designated length.  Read the whole file.
        alloc_file_image(file_image, file_size);
        fi->seek(0, SEEK_SET);
        fi->readx(file_image, file_size);
        memcpy(&ehdri, ehdr, sizeof(Elf32_Ehdr));
        phdri= (Elf32_Phdr *)((size_t)e_phoff + file_image);  // do not free() !!
        shdri= (Elf32_Shdr *)((size_t)e_shoff + file_image);  // do not free() !!

        sec_strndx = &shdri[get_te16(&ehdr->e_shstrndx)];
        shstrtab = (char const *)(get_te32(&sec_strndx->sh_offset) + file_image);
        sec_dynsym = elf_find_section_type(Elf32_Shdr::SHT_DYNSYM);
        if (sec_dynsym)
            sec_dynstr = get_te32(&sec_dynsym->sh_link) + shdri;

        sec_strndx = &shdri[get_te16(&ehdr->e_shstrndx)];
        shstrtab = (char const *)(get_te32(&sec_strndx->sh_offset) + file_image);
        if (Elf32_Shdr::SHT_STRTAB != get_te32(&sec_strndx->sh_type)
        || 0!=strcmp((char const *)".shstrtab",
                    &shstrtab[get_te32(&sec_strndx->sh_name)]) ) {
            throwCantPack("bad e_shstrndx");
        }

        phdr= phdri;
        for (int j= e_phnum; --j>=0; ++phdr)
        if (Elf32_Phdr::PT_DYNAMIC==get_te32(&phdr->p_type)) {
            dynseg= (Elf32_Dyn const *)(check_pt_dynamic(phdr) + file_image);
            break;
        }
        // elf_find_dynamic() returns 0 if 0==dynseg.
        dynstr=          (char const *)elf_find_dynamic(Elf32_Dyn::DT_STRTAB);
        dynsym=     (Elf32_Sym const *)elf_find_dynamic(Elf32_Dyn::DT_SYMTAB);

        if (Elf32_Dyn::DF_1_PIE & elf_unsigned_dynamic(Elf32_Dyn::DT_FLAGS_1)
        ||  calls_crt1((Elf32_Rel const *)elf_find_dynamic(Elf32_Dyn::DT_REL),
                                 (int)elf_unsigned_dynamic(Elf32_Dyn::DT_RELSZ))
        ||  calls_crt1((Elf32_Rel const *)elf_find_dynamic(Elf32_Dyn::DT_JMPREL),
                                 (int)elf_unsigned_dynamic(Elf32_Dyn::DT_PLTRELSZ))) {
            is_pie = true;
            goto proceed;  // calls C library init for main program
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

        if (/*jni_onload_sym ||*/ elf_find_dynamic(Elf32_Dyn::DT_INIT)) {
            if (this->e_machine!=Elf32_Ehdr::EM_386
            &&  this->e_machine!=Elf32_Ehdr::EM_MIPS
            &&  this->e_machine!=Elf32_Ehdr::EM_ARM)
                goto abandon;  // need stub: EM_PPC
            if (elf_has_dynamic(Elf32_Dyn::DT_TEXTREL)) {
                throwCantPack("DT_TEXTREL found; re-compile with -fPIC");
                goto abandon;
            }
            Elf32_Shdr const *shdr = shdri;
            xct_va = ~0u;
            if (e_shnum) {
                for (int j= e_shnum; --j>=0; ++shdr) {
                    if (Elf32_Shdr::SHF_EXECINSTR & get_te32(&shdr->sh_flags)) {
                        xct_va = umin(xct_va, get_te32(&shdr->sh_addr));
                    }
                }
            }
            else { // no Sections; use heuristics
                unsigned const strsz  = elf_unsigned_dynamic(Elf32_Dyn::DT_STRSZ);
                unsigned const strtab = elf_unsigned_dynamic(Elf32_Dyn::DT_STRTAB);
                unsigned const relsz  = elf_unsigned_dynamic(Elf32_Dyn::DT_RELSZ);
                unsigned const rel    = elf_unsigned_dynamic(Elf32_Dyn::DT_REL);
                unsigned const init   = elf_unsigned_dynamic(Elf32_Dyn::DT_INIT);
                if ((init == (relsz + rel   ) && rel    == (strsz + strtab))
                ||  (init == (strsz + strtab) && strtab == (relsz + rel   ))
                ) {
                    xct_va = init;
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
                throwCantPack("DT_ tag above stub");
                goto abandon;
            }
            if (!opt->o_unix.android_shlib) {
                phdr = phdri;
                for (unsigned j= 0; j < e_phnum; ++phdr, ++j) {
                    unsigned const vaddr = get_te32(&phdr->p_vaddr);
                    if (PT_NOTE32 == get_te32(&phdr->p_type)
                    && xct_va < vaddr) {
                        char buf[40]; snprintf(buf, sizeof(buf),
                           "PT_NOTE %#x above stub", vaddr);
                        throwCantPack(buf);
                        goto abandon;
                    }
                }
            }
            xct_off = elf_get_offset_from_address(xct_va);
            if (opt->debug.debug_level) {
                fprintf(stderr, "shlib canPack: xct_va=%#lx  xct_off=%lx\n",
                    (long)xct_va, (long)xct_off);
            }
            goto proceed;  // But proper packing depends on checking xct_va.
        }
        else
            throwCantPack("need DT_INIT; try \"void _init(void){}\"");
abandon:
        return false;
proceed: ;
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
PackLinuxElf64::canPack()
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
    if (e_phoff != sizeof(*ehdr)) {// Phdrs not contiguous with Ehdr
        throwCantPack("non-contiguous Ehdr/Phdr; try '--force-execve'");
        return false;
    }

    // The first PT_LOAD64 must cover the beginning of the file (0==p_offset).
    Elf64_Phdr const *phdr = phdri;
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        if (j >= 14)
            return false;
        unsigned const p_type = get_te32(&phdr->p_type);
        if (1!=exetype && PT_LOAD64 == p_type) { // 1st PT_LOAD
            exetype = 1;
            load_va = get_te64(&phdr->p_vaddr);  // class data member
            upx_uint64_t const p_offset = get_te64(&phdr->p_offset);
            upx_uint64_t const off = ~page_mask & load_va;
            if (off && off == p_offset) { // specific hint
                throwCantPack("Go-language PT_LOAD: try hemfix.c, or try '--force-execve'");
                // Fixing it inside upx fails because packExtent() reads original file.
                return false;
            }
            if (0 != p_offset) { // 1st PT_LOAD must cover Ehdr and Phdr
                throwCantPack("first PT_LOAD.p_offset != 0; try '--force-execve'");
                return false;
            }
            hatch_off = ~3ul & (3+ get_te64(&phdr->p_memsz));
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

    if (Elf64_Ehdr::ET_DYN==get_te16(&ehdr->e_type)) {
        // The DT_SYMTAB has no designated length.  Read the whole file.
        alloc_file_image(file_image, file_size);
        fi->seek(0, SEEK_SET);
        fi->readx(file_image, file_size);
        memcpy(&ehdri, ehdr, sizeof(Elf64_Ehdr));
        phdri= (Elf64_Phdr *)((size_t)e_phoff + file_image);  // do not free() !!
        shdri= (Elf64_Shdr *)((size_t)e_shoff + file_image);  // do not free() !!

        sec_dynsym = elf_find_section_type(Elf64_Shdr::SHT_DYNSYM);
        if (sec_dynsym)
            sec_dynstr = get_te32(&sec_dynsym->sh_link) + shdri;

        sec_strndx = &shdri[get_te16(&ehdr->e_shstrndx)];
        shstrtab = (char const *)(get_te64(&sec_strndx->sh_offset) + file_image);
        if (Elf64_Shdr::SHT_STRTAB != get_te32(&sec_strndx->sh_type)
        || 0!=strcmp((char const *)".shstrtab",
                    &shstrtab[get_te32(&sec_strndx->sh_name)]) ) {
            throwCantPack("bad e_shstrndx");
        }

        phdr= phdri;
        for (int j= e_phnum; --j>=0; ++phdr)
        if (Elf64_Phdr::PT_DYNAMIC==get_te32(&phdr->p_type)) {
            dynseg= (Elf64_Dyn const *)(check_pt_dynamic(phdr) + file_image);
            break;
        }
        // elf_find_dynamic() returns 0 if 0==dynseg.
        dynstr=          (char const *)elf_find_dynamic(Elf64_Dyn::DT_STRTAB);
        dynsym=     (Elf64_Sym const *)elf_find_dynamic(Elf64_Dyn::DT_SYMTAB);

        if (Elf64_Dyn::DF_1_PIE & elf_unsigned_dynamic(Elf64_Dyn::DT_FLAGS_1)
        ||  calls_crt1((Elf64_Rela const *)elf_find_dynamic(Elf64_Dyn::DT_RELA),
                                  (int)elf_unsigned_dynamic(Elf64_Dyn::DT_RELASZ))
        ||  calls_crt1((Elf64_Rela const *)elf_find_dynamic(Elf64_Dyn::DT_JMPREL),
                                  (int)elf_unsigned_dynamic(Elf64_Dyn::DT_PLTRELSZ))) {
            is_pie = true;
            goto proceed;  // calls C library init for main program
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
                throwCantPack("DT_TEXTREL found; re-compile with -fPIC");
                goto abandon;
            }
            Elf64_Shdr const *shdr = shdri;
            xct_va = ~0ull;
            if (e_shnum) {
                for (int j= e_shnum; --j>=0; ++shdr) {
                    if (Elf64_Shdr::SHF_EXECINSTR & get_te32(&shdr->sh_flags)) {
                        xct_va = umin64(xct_va, get_te64(&shdr->sh_addr));
                    }
                }
            }
            else { // no Sections; use heuristics
                upx_uint64_t const strsz  = elf_unsigned_dynamic(Elf64_Dyn::DT_STRSZ);
                upx_uint64_t const strtab = elf_unsigned_dynamic(Elf64_Dyn::DT_STRTAB);
                upx_uint64_t const relsz  = elf_unsigned_dynamic(Elf64_Dyn::DT_RELSZ);
                upx_uint64_t const rel    = elf_unsigned_dynamic(Elf64_Dyn::DT_REL);
                upx_uint64_t const init   = elf_unsigned_dynamic(Elf64_Dyn::DT_INIT);
                if ((init == (relsz + rel   ) && rel    == (strsz + strtab))
                ||  (init == (strsz + strtab) && strtab == (relsz + rel   ))
                ) {
                    xct_va = init;
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
                throwCantPack("DT_ tag above stub");
                goto abandon;
            }
            if (!opt->o_unix.android_shlib) {
                phdr = phdri;
                for (unsigned j= 0; j < e_phnum; ++phdr, ++j) {
                    upx_uint64_t const vaddr = get_te64(&phdr->p_vaddr);
                    if (PT_NOTE64 == get_te32(&phdr->p_type)
                    && xct_va < vaddr) {
                        char buf[40]; snprintf(buf, sizeof(buf),
                           "PT_NOTE %#lx above stub", (unsigned long)vaddr);
                        throwCantPack(buf);
                        goto abandon;
                    }
                }
            }
            xct_off = elf_get_offset_from_address(xct_va);
            if (opt->debug.debug_level) {
                fprintf(stderr, "shlib canPack: xct_va=%#lx  xct_off=%lx\n",
                    (long)xct_va, (long)xct_off);
            }
            goto proceed;  // But proper packing depends on checking xct_va.
        }
        else
            throwCantPack("need DT_INIT; try \"void _init(void){}\"");
abandon:
        return false;
proceed: ;
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
        if (PT_LOAD32 == get_te32(&phdr->p_type)) {
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
    unsigned phnum_o = get_te16(&h2->ehdr.e_phnum);
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
    if (gnu_stack) {
        sz_elf_hdrs += sizeof(Elf32_Phdr);
        memcpy(&h2->phdr[phnum_o++], gnu_stack, sizeof(*gnu_stack));
        set_te16(&h2->ehdr.e_phnum, phnum_o);
    }
    o_binfo =  sizeof(Elf32_Ehdr) + sizeof(Elf32_Phdr)*phnum_o + sizeof(l_info) + sizeof(p_info);
    set_te32(&h2->phdr[0].p_filesz, sizeof(*h2));  // + identsize;
              h2->phdr[0].p_memsz = h2->phdr[0].p_filesz;

    for (unsigned j=0; j < phnum_o; ++j) {
        if (PT_LOAD32==get_te32(&h3->phdr[j].p_type)) {
            set_te32(&h3->phdr[j].p_align, page_size);
        }
    }

    // Info for OS kernel to set the brk()
    if (brka) {
        // linux-2.6.14 binfmt_elf.c: SIGKILL if (0==.p_memsz) on a page boundary
        upx_uint32_t lo_va_user = ~0u;  // infinity
        upx_uint32_t memsz(0);
        for (int j= e_phnum; --j>=0; ) {
            if (PT_LOAD32 == get_te32(&phdri[j].p_type)) {
                upx_uint32_t const vaddr = get_te32(&phdri[j].p_vaddr);
                lo_va_user = umin(lo_va_user, vaddr);
                if (vaddr == lo_va_user) {
                    memsz = get_te32(&phdri[j].p_memsz);
                }
            }
        }
        set_te32(&h2->phdr[0].p_paddr, lo_va_user);
        set_te32(&h2->phdr[0].p_vaddr, lo_va_user);
        unsigned const brkb = page_mask & (~page_mask +
            get_te32(&h2->phdr[0].p_vaddr) + memsz);
        set_te32(&h2->phdr[1].p_type, PT_LOAD32);  // be sure
        h2->phdr[1].p_offset = 0;
        set_te32(&h2->phdr[1].p_vaddr, brkb);
        set_te32(&h2->phdr[1].p_paddr, brkb);
        h2->phdr[1].p_filesz = 0;
        set_te32(&h2->phdr[1].p_memsz, brka - brkb);
        set_te32(&h2->phdr[1].p_flags, Elf32_Phdr::PF_R | Elf32_Phdr::PF_W);
    }
    if (ph.format==getFormat()) {
        assert((2u+ !!gnu_stack) == phnum_o);
        set_te32(&h2->phdr[0].p_flags, ~Elf32_Phdr::PF_W & get_te32(&h2->phdr[0].p_flags));
        if (!gnu_stack) {
            memset(&h2->linfo, 0, sizeof(h2->linfo));
            fo->write(h2, sizeof(*h2));
        }
        else {
            memset(&h3->linfo, 0, sizeof(h3->linfo));
            fo->write(h3, sizeof(*h3));
        }
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
        set_te32(&phdr->p_type, PT_NOTE32);
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
        set_te32(&phdr->p_type, PT_NOTE32);
        set_te32(&phdr->p_offset, note_offset);
        set_te32(&phdr->p_vaddr, note_offset);
        set_te32(&phdr->p_paddr, note_offset);
        set_te32(&phdr->p_filesz, sz_PaX);
        set_te32(&phdr->p_memsz,  sz_PaX);
        set_te32(&phdr->p_flags, Elf32_Phdr::PF_R);
        set_te32(&phdr->p_align, 4);

        /* &np_PaX->body[4] */
        const unsigned char *p4 =  &(ACC_CCAST(const unsigned char *, (1+ np_PaX)))[4];
        unsigned bits = get_te32(p4);
        bits &= ~PAX_MPROTECT;
        bits |=  PAX_NOMPROTECT;
        set_te32(ACC_UNCONST_CAST(unsigned char *, p4), bits);

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

        // The 'if' guards on these two calls to memcpy are required
        // because the C Standard Committee did not debug the Standard
        // before publishing.  An empty region (0==size) must nevertheless
        // have a valid (non-NULL) pointer.
        if (sz_NetBSD) memcpy(&((char *)phdr)[0],         np_NetBSD, sz_NetBSD);
        if (sz_PaX)    memcpy(&((char *)phdr)[sz_NetBSD], np_PaX,    sz_PaX);

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

    set_te32(&h3->phdr[2].p_type, PT_NOTE32);
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
    // Too many kernels have bugs when 0==.p_memsz
    set_te32(&h3->phdr[1].p_memsz, 1);
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
    h3->ehdr.e_ident[Elf64_Ehdr::EI_OSABI] = ei_osabi;
    if (Elf64_Ehdr::ELFOSABI_LINUX == ei_osabi  // proper
    &&  Elf64_Ehdr::ELFOSABI_NONE  == ehdri.e_ident[Elf64_Ehdr::EI_OSABI]  // sloppy
    ) { // propagate sloppiness so that decompression does not complain
        h3->ehdr.e_ident[Elf64_Ehdr::EI_OSABI] = ehdri.e_ident[Elf64_Ehdr::EI_OSABI];
    }
    if (Elf64_Ehdr::EM_PPC64 == ehdri.e_machine) {
        h3->ehdr.e_flags = ehdri.e_flags;  // "0x1, abiv1" vs "0x2, abiv2"
    }

    unsigned phnum_o = get_te16(&h2->ehdr.e_phnum);

    assert(get_te64(&h2->ehdr.e_phoff)     == sizeof(Elf64_Ehdr));
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
    if (gnu_stack) {
        sz_elf_hdrs += sizeof(Elf64_Phdr);
        memcpy(&h2->phdr[phnum_o++], gnu_stack, sizeof(*gnu_stack));
        set_te16(&h2->ehdr.e_phnum, phnum_o);
    }
    o_binfo =  sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr)*phnum_o + sizeof(l_info) + sizeof(p_info);
    set_te64(&h2->phdr[0].p_filesz, sizeof(*h2));  // + identsize;
                  h2->phdr[0].p_memsz = h2->phdr[0].p_filesz;

    for (unsigned j=0; j < 4; ++j) {
        if (PT_LOAD64==get_te32(&h3->phdr[j].p_type)) {
            set_te64(&h3->phdr[j].p_align, page_size);
        }
    }

    // Info for OS kernel to set the brk()
    if (brka) {
        // linux-2.6.14 binfmt_elf.c: SIGKILL if (0==.p_memsz) on a page boundary
        upx_uint64_t lo_va_user(~(upx_uint64_t)0);  // infinity
        for (int j= e_phnum; --j>=0; ) {
            if (PT_LOAD64 == get_te32(&phdri[j].p_type)) {
                upx_uint64_t const vaddr = get_te64(&phdri[j].p_vaddr);
                lo_va_user = umin64(lo_va_user, vaddr);
            }
        }
        set_te64(&h2->phdr[0].p_paddr, lo_va_user);
        set_te64(&h2->phdr[0].p_vaddr, lo_va_user);
        set_te32(&h2->phdr[1].p_type, PT_LOAD64);  // be sure
        h2->phdr[1].p_offset = 0;
        h2->phdr[1].p_filesz = 0;
        // .p_memsz = brka;  temporary until sz_pack2
        set_te64(&h2->phdr[1].p_memsz, brka);
        set_te32(&h2->phdr[1].p_flags, Elf64_Phdr::PF_R | Elf64_Phdr::PF_W);
    }
    if (ph.format==getFormat()) {
        assert((2u+ !!gnu_stack) == phnum_o);
        set_te32(&h2->phdr[0].p_flags, ~Elf64_Phdr::PF_W & get_te32(&h2->phdr[0].p_flags));
        if (!gnu_stack) {
            memset(&h2->linfo, 0, sizeof(h2->linfo));
            fo->write(h2, sizeof(*h2));
        }
        else {
            memset(&h3->linfo, 0, sizeof(h3->linfo));
            fo->write(h3, sizeof(*h3));
        }
    }
    else {
        assert(false);  // unknown ph.format, PackLinuxElf64
    }
}

#define WANT_REL_ENUM
#include "p_elf_enum.h"
#undef WANT_REL_ENUM

void PackLinuxElf32::pack1(OutputFile *fo, Filter & /*ft*/)
{
    fi->seek(0, SEEK_SET);
    fi->readx(&ehdri, sizeof(ehdri));
    assert(e_phoff == sizeof(Elf32_Ehdr));  // checked by canPack()
    sz_phdrs = e_phnum * get_te16(&ehdri.e_phentsize);

    // Remember all PT_NOTE, and find lg2_page from PT_LOAD.
    Elf32_Phdr *phdr = phdri;
    note_size = 0;
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        if (PT_NOTE32 == get_te32(&phdr->p_type)) {
            note_size += up4(get_te32(&phdr->p_filesz));
        }
    }
    if (note_size) {
        note_body = New(unsigned char, note_size);
        note_size = 0;
    }
    phdr = phdri;
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        unsigned const type = get_te32(&phdr->p_type);
        if (PT_NOTE32 == type) {
            unsigned const len = get_te32(&phdr->p_filesz);
            fi->seek(get_te32(&phdr->p_offset), SEEK_SET);
            fi->readx(&note_body[note_size], len);
            note_size += up4(len);
        }
        if (PT_LOAD32 == type) {
            unsigned x = get_te32(&phdr->p_align) >> lg2_page;
            while (x>>=1) {
                ++lg2_page;
            }
        }
        if (PT_GNU_STACK32 == type) {
            // MIPS stub cannot handle GNU_STACK yet.
            if (Elf32_Ehdr::EM_MIPS != this->e_machine) {
                gnu_stack = phdr;
            }
        }
    }
    page_size =  1u<<lg2_page;
    page_mask = ~0u<<lg2_page;

    progid = 0;  // getRandomId();  not useful, so do not clutter
    sz_elf_hdrs = sizeof(ehdri) + sz_phdrs;
    if (0!=xct_off) {  // shared library
        sz_elf_hdrs = xct_off;
        lowmem.alloc(xct_off + (!opt->o_unix.android_shlib
            ? 0
            : e_shnum * sizeof(Elf32_Shdr)));
        memcpy(lowmem, file_image, xct_off);  // android omits Shdr here
        fo->write(lowmem, xct_off);  // < SHF_EXECINSTR (typ: in .plt or .init)
        if (opt->o_unix.android_shlib) {
            // In order to pacify the runtime linker on Android "O" ("Oreo"),
            // we will splice-in a 4KiB page that contains an "extra" copy
            // of the Shdr, any PT_NOTE above xct_off, and shstrtab.
            // File order: Ehdr, Phdr[], section contents below xct_off,
            //    Shdr_copy[], PT_NOTEs.hi, shstrtab.
            xct_va  += asl_delta;
            //xct_off += asl_delta;  // not yet

            // Relocate PT_DYNAMIC (in 2nd PT_LOAD)
            Elf32_Dyn *dyn = const_cast<Elf32_Dyn *>(dynseg);
            for (; dyn->d_tag; ++dyn) {
                unsigned d_tag = get_te32(&dyn->d_tag);
                if (Elf32_Dyn::DT_FINI       == d_tag
                ||  Elf32_Dyn::DT_FINI_ARRAY == d_tag
                ||  Elf32_Dyn::DT_INIT_ARRAY == d_tag
                ||  Elf32_Dyn::DT_PREINIT_ARRAY == d_tag
                ||  Elf32_Dyn::DT_PLTGOT     == d_tag) {
                    unsigned d_val = get_te32(&dyn->d_val);
                    set_te32(&dyn->d_val, asl_delta + d_val);
                }
            }

            // Relocate dynsym (DT_SYMTAB) which is below xct_va
            unsigned const off_dynsym = get_te32(&sec_dynsym->sh_offset);
            unsigned const sz_dynsym  = get_te32(&sec_dynsym->sh_size);
            Elf32_Sym *dyntym = (Elf32_Sym *)lowmem.subref(
                "bad dynsym", off_dynsym, sz_dynsym);
            Elf32_Sym *sym = dyntym;
            for (int j = sz_dynsym / sizeof(Elf32_Sym); --j>=0; ++sym) {
                unsigned symval = get_te32(&sym->st_value);
                unsigned symsec = get_te16(&sym->st_shndx);
                if (Elf32_Sym::SHN_UNDEF != symsec
                &&  Elf32_Sym::SHN_ABS   != symsec
                &&  xct_off <= symval) {
                    set_te32(&sym->st_value, asl_delta + symval);
                }
            }

            // Relocate Phdr virtual addresses, but not physical offsets and sizes
            unsigned char buf_notes[512]; memset(buf_notes, 0, sizeof(buf_notes));
            unsigned len_notes = 0;
            phdr = (Elf32_Phdr *)lowmem.subref(
                "bad e_phoff", e_phoff, e_phnum * sizeof(Elf32_Phdr));
            for (unsigned j = 0; j < e_phnum; ++j, ++phdr) {
                upx_uint32_t offset = get_te32(&phdr->p_offset);
                if (xct_off <= offset) { // above the extra page
                    if (PT_NOTE32 == get_te32(&phdr->p_type)) {
                        upx_uint32_t memsz = get_te32(&phdr->p_memsz);
                        if (sizeof(buf_notes) < (memsz + len_notes)) {
                            throwCantPack("PT_NOTES too big");
                        }
                        set_te32(&phdr->p_vaddr,
                            len_notes + (e_shnum * sizeof(Elf32_Shdr)) + xct_off);
                        phdr->p_offset = phdr->p_paddr = phdr->p_vaddr;
                        memcpy(&buf_notes[len_notes], &file_image[offset], memsz);
                        len_notes += memsz;
                    }
                    else {
                        //set_te32(&phdr->p_offset, asl_delta + offset);  // physical
                        upx_uint32_t addr = get_te32(&phdr->p_paddr);
                        set_te32(&phdr->p_paddr, asl_delta + addr);
                                     addr = get_te32(&phdr->p_vaddr);
                        set_te32(&phdr->p_vaddr, asl_delta + addr);
                    }
                }
                // .p_filesz,.p_memsz are updated in ::pack3
            }

            Elf32_Ehdr *const ehdr = (Elf32_Ehdr *)&lowmem[0];
            upx_uint32_t e_entry = get_te32(&ehdr->e_entry);
            if (xct_off < e_entry) {
                set_te32(&ehdr->e_entry, asl_delta + e_entry);
            }
            // Relocate Shdr; and Rela, Rel (below xct_off)
            set_te32(&ehdr->e_shoff, xct_off);
            memcpy(&lowmem[xct_off], shdri, e_shnum * sizeof(Elf32_Shdr));
            Elf32_Shdr *const shdro = (Elf32_Shdr *)&lowmem[xct_off];
            Elf32_Shdr *shdr = shdro;
            unsigned sz_shstrtab  = get_te32(&sec_strndx->sh_size);
            for (unsigned j = 0; j < e_shnum; ++j, ++shdr) {

                unsigned sh_type = get_te32(&shdr->sh_type);
                unsigned sh_size = get_te32(&shdr->sh_size);
                unsigned  sh_offset = get_te32(&shdr->sh_offset);
                unsigned sh_entsize = get_te32(&shdr->sh_entsize);
                if (xct_off <= sh_offset) {
                    //set_te32(&shdr->sh_offset, asl_delta + sh_offset);  // FIXME ??
                    upx_uint32_t addr = get_te32(&shdr->sh_addr);
                    set_te32(&shdr->sh_addr, asl_delta + addr);
                }
                if (Elf32_Shdr::SHT_RELA== sh_type) {
                    if (sizeof(Elf32_Rela) != sh_entsize) {
                        char msg[50];
                        snprintf(msg, sizeof(msg), "bad Rela.sh_entsize %u", sh_entsize);
                        throwCantPack(msg);
                    }
                    n_jmp_slot = 0;
                    plt_off = ~0u;
                    Elf32_Rela *const relb = (Elf32_Rela *)lowmem.subref(
                         "bad Rela offset", sh_offset, sh_size);
                    Elf32_Rela *rela = relb;
                    for (int k = sh_size / sh_entsize; --k >= 0; ++rela) {
                        unsigned r_addend = get_te32(&rela->r_addend);
                        unsigned r_offset = get_te32(&rela->r_offset);
                        unsigned r_info   = get_te32(&rela->r_info);
                        unsigned r_type = ELF32_R_TYPE(r_info);
                        if (xct_off <= r_offset) {
                            set_te32(&rela->r_offset, asl_delta + r_offset);
                        }
                        if (Elf32_Ehdr::EM_ARM == e_machine) {
                            if (R_ARM_RELATIVE == r_type) {
                                if (xct_off <= r_addend) {
                                    set_te32(&rela->r_addend, asl_delta + r_addend);
                                }
                            }
                            if (R_ARM_JUMP_SLOT == r_type) {
                                // .rela.plt contains offset of the "first time" target
                                if (plt_off > r_offset) {
                                    plt_off = r_offset;
                                }
                                unsigned d = elf_get_offset_from_address(r_offset);
                                unsigned w = get_te32(&file_image[d]);
                                if (xct_off <= w) {
                                    set_te32(&file_image[d], asl_delta + w);
                                }
                                ++n_jmp_slot;
                            }
                        }
                    }
                    fo->seek(sh_offset, SEEK_SET);
                    fo->rewrite(relb, sh_size);
                }
                if (Elf32_Shdr::SHT_REL == sh_type) {
                    if (sizeof(Elf32_Rel) != sh_entsize) {
                        char msg[50];
                        snprintf(msg, sizeof(msg), "bad Rel.sh_entsize %u", sh_entsize);
                        throwCantPack(msg);
                    }
                    n_jmp_slot = 0;
                    plt_off = ~0u;
                    Elf32_Rel *const rel0 = (Elf32_Rel *)lowmem.subref(
                         "bad Rel offset", sh_offset, sh_size);
                    Elf32_Rel *rel = rel0;
                    for (int k = sh_size / sh_entsize; --k >= 0; ++rel) {
                        unsigned r_offset = get_te32(&rel->r_offset);
                        unsigned r_info = get_te32(&rel->r_info);
                        unsigned r_type = ELF32_R_TYPE(r_info);
                        unsigned d = elf_get_offset_from_address(r_offset);
                        unsigned w = get_te32(&file_image[d]);
                        if (xct_off <= r_offset) {
                            set_te32(&rel->r_offset, asl_delta + r_offset);
                        }
                        if (Elf32_Ehdr::EM_ARM == e_machine) {
                            if (R_ARM_RELATIVE  == r_type) {
                                if (xct_off <= w) {
                                    set_te32(&file_image[d], asl_delta + w);
                                }
                            }
                            if (R_ARM_JUMP_SLOT == r_type) {
                                if (plt_off > r_offset) {
                                    plt_off = r_offset;
                                }
                                if (xct_off <= w) {
                                    set_te32(&file_image[d], asl_delta + w);
                                }
                                ++n_jmp_slot;
                            }
                        }
                    }
                    fo->seek(sh_offset, SEEK_SET);
                    fo->rewrite(rel0, sh_size);
                }
                if (Elf32_Shdr::SHT_NOTE == sh_type) {
                    if (xct_off <= sh_offset) {
                        set_te32(&shdr->sh_offset,
                            (e_shnum * sizeof(Elf32_Shdr)) + xct_off);
                        shdr->sh_addr = shdr->sh_offset;
                    }
                }
            }
            // shstrndx will move
            set_te32(&shdro[get_te16(&ehdri.e_shstrndx)].sh_offset,
                len_notes + e_shnum * sizeof(Elf32_Shdr) + xct_off);

            // (Re-)write all changes below xct_off
            fo->seek(0, SEEK_SET);
            fo->rewrite(lowmem, xct_off);

            // New copy of Shdr
            Elf32_Shdr blank; memset(&blank, 0, sizeof(blank));
            set_te32(&blank.sh_offset, xct_off);  // hint for "upx -d"
            fo->write(&blank, sizeof(blank));
            fo->write(&shdro[1], (-1+ e_shnum) * sizeof(Elf32_Shdr));

            if (len_notes) {
                fo->write(buf_notes, len_notes);
            }

            // New copy of Shdr[.e_shstrndx].[ sh_offset, +.sh_size )
            fo->write(shstrtab,  sz_shstrtab);

            sz_elf_hdrs = fpad4(fo);
            //xct_off += asl_delta;  // wait until ::pack3
        }
        memset(&linfo, 0, sizeof(linfo));
        fo->write(&linfo, sizeof(linfo));
    }

    // if the preserve build-id option was specified
    if (opt->o_unix.preserve_build_id) {
        // set this so we can use elf_find_section_name
        e_shnum = get_te16(&ehdri.e_shnum);
        if (!shdri) {
            shdri = (Elf32_Shdr *)&file_image[get_te32(&ehdri.e_shoff)];
        }
        //set the shstrtab
        sec_strndx = &shdri[get_te16(&ehdri.e_shstrndx)];

        char *strtab = New(char, sec_strndx->sh_size);
        fi->seek(0,SEEK_SET);
        fi->seek(sec_strndx->sh_offset,SEEK_SET);
        fi->readx(strtab,sec_strndx->sh_size);

        shstrtab = (const char*)strtab;

        Elf32_Shdr const *buildid = elf_find_section_name(".note.gnu.build-id");
        if (buildid) {
            unsigned char *data = New(unsigned char, buildid->sh_size);
            memset(data,0,buildid->sh_size);
            fi->seek(0,SEEK_SET);
            fi->seek(buildid->sh_offset,SEEK_SET);
            fi->readx(data,buildid->sh_size);

            buildid_data  = data;

            o_elf_shnum = 3;
            memset(&shdrout,0,sizeof(shdrout));

            //setup the build-id
            memcpy(&shdrout.shdr[1], buildid, sizeof(shdrout.shdr[1]));
            shdrout.shdr[1].sh_name = 1;

            //setup the shstrtab
            memcpy(&shdrout.shdr[2], sec_strndx, sizeof(shdrout.shdr[2]));
            shdrout.shdr[2].sh_name = 20;
            shdrout.shdr[2].sh_size = 29; //size of our static shstrtab
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
        memcpy(&h3, stub_arm_v5a_linux_elf_fold, sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr));

        h3.ehdr.e_ident[Elf32_Ehdr::EI_ABIVERSION] = e_flags>>24;
    }
    else {
        memcpy(&h3, stub_arm_v4a_linux_elf_fold,        sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr));
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
    memcpy(&h3, stub_armeb_v4a_linux_elf_fold, sizeof(Elf32_Ehdr) + 2*sizeof(Elf32_Phdr));
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

void PackLinuxElf64ppcle::pack1(OutputFile *fo, Filter &ft)
{
    super::pack1(fo, ft);
    if (0!=xct_off)  // shared library
        return;
    generateElfHdr(fo, stub_powerpc64le_linux_elf_fold, getbrk(phdri, e_phnum) );
}

void PackLinuxElf64ppc::pack1(OutputFile *fo, Filter &ft)
{
    super::pack1(fo, ft);
    if (0!=xct_off)  // shared library
        return;
    generateElfHdr(fo, stub_powerpc64_linux_elf_fold, getbrk(phdri, e_phnum) );
}

void PackLinuxElf64::pack1(OutputFile *fo, Filter & /*ft*/)
{
    fi->seek(0, SEEK_SET);
    fi->readx(&ehdri, sizeof(ehdri));
    assert(e_phoff == sizeof(Elf64_Ehdr));  // checked by canPack()
    sz_phdrs = e_phnum * get_te16(&ehdri.e_phentsize);

    Elf64_Phdr *phdr = phdri;
    note_size = 0;
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        if (PT_NOTE64 == get_te32(&phdr->p_type)) {
            note_size += up4(get_te64(&phdr->p_filesz));
        }
    }
    if (note_size) {
        note_body = New(unsigned char, note_size);
        note_size = 0;
    }
    phdr = phdri;
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        unsigned const type = get_te32(&phdr->p_type);
        if (PT_NOTE64 == type) {
            unsigned const len = get_te64(&phdr->p_filesz);
            fi->seek(get_te64(&phdr->p_offset), SEEK_SET);
            fi->readx(&note_body[note_size], len);
            note_size += up4(len);
        }
        if (PT_LOAD64 == type) {
            unsigned x = get_te64(&phdr->p_align) >> lg2_page;
            while (x>>=1) {
                ++lg2_page;
            }
        }
        if (PT_GNU_STACK64 == type) {
            gnu_stack = phdr;
        }
    }
    page_size =  1u  <<lg2_page;
    page_mask = ~0ull<<lg2_page;

    progid = 0;  // getRandomId();  not useful, so do not clutter
    sz_elf_hdrs = sizeof(ehdri) + sz_phdrs;
    if (0!=xct_off) {  // shared library
        sz_elf_hdrs = xct_off;
        lowmem.alloc(xct_off + (!opt->o_unix.android_shlib
            ? 0
            : e_shnum * sizeof(Elf64_Shdr)));
        memcpy(lowmem, file_image, xct_off);  // android omits Shdr here
        fo->write(lowmem, xct_off);  // < SHF_EXECINSTR (typ: in .plt or .init)
        if (opt->o_unix.android_shlib) {
            // In order to pacify the runtime linker on Android "O" ("Oreo"),
            // we will splice-in a 4KiB page that contains an "extra" copy
            // of the Shdr, any PT_NOTE above xct_off, and shstrtab.
            // File order: Ehdr, Phdr[], section contents below xct_off,
            //    Shdr_copy[], PT_NOTEs.hi, shstrtab.
            xct_va  += asl_delta;
            //xct_off += asl_delta;  // not yet

            // Relocate PT_DYNAMIC (in 2nd PT_LOAD)
            Elf64_Dyn *dyn = const_cast<Elf64_Dyn *>(dynseg);
            for (; dyn->d_tag; ++dyn) {
                upx_uint64_t d_tag = get_te64(&dyn->d_tag);
                if (Elf64_Dyn::DT_FINI       == d_tag
                ||  Elf64_Dyn::DT_FINI_ARRAY == d_tag
                ||  Elf64_Dyn::DT_INIT_ARRAY == d_tag
                ||  Elf64_Dyn::DT_PREINIT_ARRAY == d_tag
                ||  Elf64_Dyn::DT_PLTGOT      == d_tag) {
                    upx_uint64_t d_val = get_te64(&dyn->d_val);
                    set_te64(&dyn->d_val, asl_delta + d_val);
                }
            }

            // Relocate dynsym (DT_SYMTAB) which is below xct_va
            upx_uint64_t const off_dynsym = get_te64(&sec_dynsym->sh_offset);
            upx_uint64_t const sz_dynsym  = get_te64(&sec_dynsym->sh_size);
            Elf64_Sym *dyntym = (Elf64_Sym *)lowmem.subref(
                "bad dynsym", off_dynsym, sz_dynsym);
            Elf64_Sym *sym = dyntym;
            for (int j = sz_dynsym / sizeof(Elf64_Sym); --j>=0; ++sym) {
                upx_uint64_t symval = get_te64(&sym->st_value);
                unsigned symsec = get_te16(&sym->st_shndx);
                if (Elf64_Sym::SHN_UNDEF != symsec
                &&  Elf64_Sym::SHN_ABS   != symsec
                &&  xct_off <= symval) {
                    set_te64(&sym->st_value, asl_delta + symval);
                }
            }

            // Relocate Phdr virtual addresses, but not physical offsets and sizes
            unsigned char buf_notes[512]; memset(buf_notes, 0, sizeof(buf_notes));
            unsigned len_notes = 0;
            phdr = (Elf64_Phdr *)lowmem.subref(
                "bad e_phoff", e_phoff, e_phnum * sizeof(Elf64_Phdr));
            for (unsigned j = 0; j < e_phnum; ++j, ++phdr) {
                upx_uint64_t offset = get_te64(&phdr->p_offset);
                if (xct_off <= offset) { // above the extra page
                    if (PT_NOTE64 == get_te32(&phdr->p_type)) {
                        upx_uint64_t memsz = get_te64(&phdr->p_memsz);
                        if (sizeof(buf_notes) < (memsz + len_notes)) {
                            throwCantPack("PT_NOTES too big");
                        }
                        set_te64(&phdr->p_vaddr,
                            len_notes + (e_shnum * sizeof(Elf64_Shdr)) + xct_off);
                        phdr->p_offset = phdr->p_paddr = phdr->p_vaddr;
                        memcpy(&buf_notes[len_notes], &file_image[offset], memsz);
                        len_notes += memsz;
                    }
                    else {
                        //set_te64(&phdr->p_offset, asl_delta + offset);  // physical
                        upx_uint64_t addr = get_te64(&phdr->p_paddr);
                        set_te64(&phdr->p_paddr, asl_delta + addr);
                                     addr = get_te64(&phdr->p_vaddr);
                        set_te64(&phdr->p_vaddr, asl_delta + addr);
                    }
                }
                // .p_filesz,.p_memsz are updated in ::pack3
            }

            Elf64_Ehdr *const ehdr = (Elf64_Ehdr *)&lowmem[0];
            upx_uint64_t e_entry = get_te64(&ehdr->e_entry);
            if (xct_off < e_entry) {
                set_te64(&ehdr->e_entry, asl_delta + e_entry);
            }
            // Relocate Shdr; and Rela, Rel (below xct_off)
            set_te64(&ehdr->e_shoff, xct_off);
            memcpy(&lowmem[xct_off], shdri, e_shnum * sizeof(Elf64_Shdr));
            Elf64_Shdr *const shdro = (Elf64_Shdr *)&lowmem[xct_off];
            Elf64_Shdr *shdr = shdro;
            upx_uint64_t sz_shstrtab  = get_te64(&sec_strndx->sh_size);
            for (unsigned j = 0; j < e_shnum; ++j, ++shdr) {
                unsigned sh_type = get_te32(&shdr->sh_type);
                upx_uint64_t sh_size = get_te64(&shdr->sh_size);
                upx_uint64_t sh_offset = get_te64(&shdr->sh_offset);
                upx_uint64_t sh_entsize = get_te64(&shdr->sh_entsize);

                if (xct_off <= sh_offset) {
                    upx_uint64_t addr = get_te64(&shdr->sh_addr);
                    set_te64(&shdr->sh_addr, asl_delta + addr);
                }
                if (Elf64_Shdr::SHT_RELA == sh_type) {
                    if (sizeof(Elf64_Rela) != sh_entsize) {
                        char msg[50];
                        snprintf(msg, sizeof(msg), "bad Rela.sh_entsize %lu", (long)sh_entsize);
                        throwCantPack(msg);
                    }
                    n_jmp_slot = 0;
                    plt_off = ~0ull;
                    Elf64_Rela *const relb = (Elf64_Rela *)lowmem.subref(
                         "bad Rela offset", sh_offset, sh_size);
                    Elf64_Rela *rela = relb;
                    for (int k = sh_size / sh_entsize; --k >= 0; ++rela) {
                        upx_uint64_t r_addend = get_te64(&rela->r_addend);
                        upx_uint64_t r_offset = get_te64(&rela->r_offset);
                        upx_uint64_t r_info   = get_te64(&rela->r_info);
                        unsigned r_type = ELF64_R_TYPE(r_info);
                        if (xct_off <= r_offset) {
                            set_te64(&rela->r_offset, asl_delta + r_offset);
                        }
                        if (Elf64_Ehdr::EM_AARCH64 == e_machine) {
                            if (R_AARCH64_RELATIVE == r_type) {
                                if (xct_off <= r_addend) {
                                    set_te64(&rela->r_addend, asl_delta + r_addend);
                                }
                            }
                            if (R_AARCH64_JUMP_SLOT == r_type) {
                                // .rela.plt contains offset of the "first time" target
                                if (plt_off > r_offset) {
                                    plt_off = r_offset;
                                }
                                upx_uint64_t d = elf_get_offset_from_address(r_offset);
                                upx_uint64_t w = get_te64(&file_image[d]);
                                if (xct_off <= w) {
                                    set_te64(&file_image[d], asl_delta + w);
                                }
                                ++n_jmp_slot;
                            }
                        }
                    }
                    fo->seek(sh_offset, SEEK_SET);
                    fo->rewrite(relb, sh_size);
                }
                if (Elf64_Shdr::SHT_REL == sh_type) {
                    if (sizeof(Elf64_Rel) != sh_entsize) {
                        char msg[50];
                        snprintf(msg, sizeof(msg), "bad Rel.sh_entsize %lu", (long)sh_entsize);
                        throwCantPack(msg);
                    }
                    Elf64_Rel *rel = (Elf64_Rel *)lowmem.subref(
                            "bad Rel sh_offset", sh_offset, sh_size);
                    for (int k = sh_size / sh_entsize; --k >= 0; ++rel) {
                        upx_uint64_t r_offset = get_te64(&rel->r_offset);
                        if (xct_off <= r_offset) {
                            set_te64(&rel->r_offset, asl_delta + r_offset);
                        }
                        // r_offset must be in 2nd PT_LOAD; .p_vaddr was already relocated
                        upx_uint64_t d = elf_get_offset_from_address(asl_delta + r_offset);
                        upx_uint64_t w = get_te64(&file_image[d]);
                        upx_uint64_t r_info = get_te64(&rel->r_info);
                        unsigned r_type = ELF64_R_TYPE(r_info);
                        if (xct_off <= w
                        &&  Elf64_Ehdr::EM_AARCH64 == e_machine
                        &&  (  R_AARCH64_RELATIVE  == r_type
                            || R_AARCH64_JUMP_SLOT == r_type)) {
                            set_te64(&file_image[d], asl_delta + w);
                        }
                    }
                }
                if (Elf64_Shdr::SHT_NOTE == sh_type) {
                    if (xct_off <= sh_offset) {
                        set_te64(&shdr->sh_offset,
                            (e_shnum * sizeof(Elf64_Shdr)) + xct_off);
                        shdr->sh_addr = shdr->sh_offset;
                    }
                }
            }
            // shstrndx will move
            set_te64(&shdro[get_te16(&ehdri.e_shstrndx)].sh_offset,
                len_notes + e_shnum * sizeof(Elf64_Shdr) + xct_off);

            // (Re-)write all changes below xct_off
            fo->seek(0, SEEK_SET);
            fo->rewrite(lowmem, xct_off);

            // New copy of Shdr
            Elf64_Shdr blank; memset(&blank, 0, sizeof(blank));
            set_te64(&blank.sh_offset, xct_off);  // hint for "upx -d"
            fo->write(&blank, sizeof(blank));
            fo->write(&shdro[1], (-1+ e_shnum) * sizeof(Elf64_Shdr));

            if (len_notes) {
                fo->write(buf_notes, len_notes);
            }

            // New copy of Shdr[.e_shstrndx].[ sh_offset, +.sh_size )
            fo->write(shstrtab,  sz_shstrtab);

            sz_elf_hdrs = fpad8(fo);
            //xct_off += asl_delta;  // wait until ::pack3
        }
        memset(&linfo, 0, sizeof(linfo));
        fo->write(&linfo, sizeof(linfo));
    }

    // only execute if option present
    if (opt->o_unix.preserve_build_id) {
        // set this so we can use elf_find_section_name
        e_shnum = get_te16(&ehdri.e_shnum);
        if (!shdri) {
            shdri = (Elf64_Shdr *)&file_image[get_te32(&ehdri.e_shoff)];
        }
        //set the shstrtab
        sec_strndx = &shdri[get_te16(&ehdri.e_shstrndx)];

        char *strtab = New(char, sec_strndx->sh_size);
        fi->seek(0,SEEK_SET);
        fi->seek(sec_strndx->sh_offset,SEEK_SET);
        fi->readx(strtab,sec_strndx->sh_size);

        shstrtab = (const char*)strtab;

        Elf64_Shdr const *buildid = elf_find_section_name(".note.gnu.build-id");
        if (buildid) {
            unsigned char *data = New(unsigned char, buildid->sh_size);
            memset(data,0,buildid->sh_size);
            fi->seek(0,SEEK_SET);
            fi->seek(buildid->sh_offset,SEEK_SET);
            fi->readx(data,buildid->sh_size);

            buildid_data  = data;

            o_elf_shnum = 3;
            memset(&shdrout,0,sizeof(shdrout));

            //setup the build-id
            memcpy(&shdrout.shdr[1], buildid, sizeof(shdrout.shdr[1]));
            shdrout.shdr[1].sh_name = 1;

            //setup the shstrtab
            memcpy(&shdrout.shdr[2], sec_strndx, sizeof(shdrout.shdr[2]));
            shdrout.shdr[2].sh_name = 20;
            shdrout.shdr[2].sh_size = 29; //size of our static shstrtab
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

void PackLinuxElf64arm::pack1(OutputFile *fo, Filter &ft)
{
    super::pack1(fo, ft);
    if (0!=xct_off)  // shared library
        return;
    generateElfHdr(fo, stub_arm64_linux_elf_fold, getbrk(phdri, e_phnum) );
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
                // FIXME: ??     ft.addvalue += asl_delta;
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
    unsigned hdr_u_len = (is_shlib ? xct_off : (sizeof(Elf64_Ehdr) + sz_phdrs));

    unsigned total_in =  (is_shlib ?           0 : xct_off);
    unsigned total_out = (is_shlib ? sz_elf_hdrs : xct_off);

    uip->ui_pass = 0;
    ft.addvalue = 0;

    int nx = 0;
    for (k = 0; k < e_phnum; ++k)
    if (PT_LOAD64==get_te32(&phdri[k].p_type)) {
        if (ft.id < 0x40) {
            // FIXME: ??    ft.addvalue = phdri[k].p_vaddr;
        }
        x.offset = get_te64(&phdri[k].p_offset);
        x.size   = get_te64(&phdri[k].p_filesz);
        if (0 == nx) { // 1st PT_LOAD64 must cover Ehdr at 0==p_offset
            unsigned const delta = hdr_u_len;
            if (ft.id < 0x40) {
                // FIXME: ??     ft.addvalue += asl_delta;
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
                &&  (sz_interp + pos_interp) <= (unsigned)file_size) {
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

void PackLinuxElf32::ARM_defineSymbols(Filter const *ft)
{
    PackLinuxElf32::defineSymbols(ft);

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

void PackLinuxElf64arm::defineSymbols(Filter const *ft)
{
    PackLinuxElf64::defineSymbols(ft);

#define MAP_PRIVATE      2     /* UNIX standard */
#define MAP_FIXED     0x10     /* UNIX standard */
#define MAP_ANONYMOUS 0x20     /* UNIX standard */
#define MAP_PRIVANON     3     /* QNX anonymous private memory */
    unsigned mflg = MAP_PRIVATE | MAP_ANONYMOUS;
    //if (ARM_is_QNX())
    //    mflg = MAP_PRIVANON;
    linker->defineSymbol("MFLG", mflg);
}

void PackLinuxElf32mipseb::defineSymbols(Filter const *ft)
{
    PackLinuxElf32::defineSymbols(ft);
}

void PackLinuxElf32mipsel::defineSymbols(Filter const *ft)
{
    PackLinuxElf32::defineSymbols(ft);
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

    fo->seek(0, SEEK_SET);
    if (0!=xct_off) {  // shared library
        fo->rewrite(&lowmem[0], sizeof(ehdri) + e_phnum * sizeof(*phdri));
        fo->seek(sz_elf_hdrs, SEEK_SET);
        fo->rewrite(&linfo, sizeof(linfo));

        if (jni_onload_va) {
            unsigned tmp = sz_pack2 + get_te32(&elfout.phdr[0].p_vaddr);
            tmp |= (Elf32_Ehdr::EM_ARM==e_machine);  // THUMB mode
            set_te32(&tmp, tmp);
            fo->seek(ptr_udiff(&jni_onload_sym->st_value, file_image), SEEK_SET);
            fo->rewrite(&tmp, sizeof(tmp));
        }
    }
    else {
        unsigned const reloc = get_te32(&elfout.phdr[0].p_vaddr);
        Elf32_Phdr *phdr = &elfout.phdr[2];
        unsigned const o_phnum = get_te16(&elfout.ehdr.e_phnum);
        for (unsigned j = 2; j < o_phnum; ++j, ++phdr) {
            if (PT_NOTE32 == get_te32(&phdr->p_type)) {
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

    fo->seek(0, SEEK_SET);
    if (0!=xct_off) {  // shared library
        fo->rewrite(&lowmem[0], sizeof(ehdri) + e_phnum * sizeof(Elf64_Phdr));
        fo->seek(sz_elf_hdrs, SEEK_SET);
        fo->rewrite(&linfo, sizeof(linfo));
    }
    else {
        if (PT_NOTE64 == get_te64(&elfout.phdr[2].p_type)) {
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

void
PackLinuxElf32::unRel32(
    unsigned dt_rel,
    Elf32_Rel *rel0,
    unsigned relsz,
    MemBuffer &ptload1,
    unsigned const load_off,
    OutputFile *fo
)
{
    Elf32_Rel *rel = rel0;
    for (int k = relsz / sizeof(Elf32_Rel); --k >= 0; ++rel) {
        unsigned r_offset = get_te32(&rel->r_offset);
        unsigned r_info   = get_te32(&rel->r_info);
        unsigned r_type = ELF32_R_TYPE(r_info);
        if (xct_off <= r_offset) {
            set_te32(&rel->r_offset, r_offset - asl_delta);
        }
        if (Elf32_Ehdr::EM_ARM == e_machine) {
            if (R_ARM_RELATIVE == r_type) {
                unsigned d = r_offset - load_off - asl_delta;
                unsigned w = get_te32(&ptload1[d]);
                if (xct_off <= w) {
                    set_te32(&ptload1[d], w - asl_delta);
                }
            }
            if (R_ARM_JUMP_SLOT == r_type) {
                ++n_jmp_slot;
                // .rel.plt contains offset of the "first time" target
                unsigned d = r_offset - load_off - asl_delta;
                if (plt_off > d) {
                    plt_off = d;
                }
                unsigned w = get_te32(&ptload1[d]);
                if (xct_off <= w) {
                    set_te32(&ptload1[d], w - asl_delta);
                }
            }
        }
    }
    fo->seek(dt_rel, SEEK_SET);
    fo->rewrite(rel0, relsz);
}

void
PackLinuxElf64::unRela64(
    upx_uint64_t dt_rela,
    Elf64_Rela *rela0,
    unsigned relasz,
    MemBuffer &ptload1,
    upx_uint64_t const load_off,
    OutputFile *fo
)
{
    Elf64_Rela *rela = rela0;
    for (int k = relasz / sizeof(Elf64_Rela); --k >= 0; ++rela) {
        upx_uint64_t r_addend = get_te64(&rela->r_addend);
        upx_uint64_t r_offset = get_te64(&rela->r_offset);
        upx_uint64_t r_info   = get_te64(&rela->r_info);
        unsigned r_type = ELF64_R_TYPE(r_info);
        if (xct_off <= r_offset) {
            set_te64(&rela->r_offset, r_offset - asl_delta);
        }
        if (Elf64_Ehdr::EM_AARCH64 == e_machine) {
            if (R_AARCH64_RELATIVE == r_type) {
                if (xct_off <= r_addend) {
                    set_te64(&rela->r_addend, r_addend - asl_delta);
                }
            }
            if (R_AARCH64_JUMP_SLOT == r_type) {
                ++n_jmp_slot;
                // .rela.plt contains offset of the "first time" target
                upx_uint64_t d = r_offset - load_off - asl_delta;
                if (plt_off > d) {
                    plt_off = d;
                }
                upx_uint64_t w = get_te64(&ptload1[d]);
                if (xct_off <= w) {
                    set_te64(&ptload1[d], w - asl_delta);
                }
            }
        }
    }
    fo->seek(dt_rela, SEEK_SET);
    fo->rewrite(rela0, relasz);
}

void PackLinuxElf64::unpack(OutputFile *fo)
{
    if (e_phoff != sizeof(Elf64_Ehdr)) {// Phdrs not contiguous with Ehdr
        throwCantUnpack("bad e_phoff");
    }
    unsigned const c_phnum = get_te16(&ehdri.e_phnum);
    upx_uint64_t old_data_off = 0;
    upx_uint64_t old_data_len = 0;
    upx_uint64_t old_dtinit = 0;
    unsigned is_asl = 0;  // is Android Shared Library

    unsigned szb_info = sizeof(b_info);
    {
        upx_uint64_t const e_entry = get_te64(&ehdri.e_entry);
        if (e_entry < 0x401180
        &&  get_te16(&ehdri.e_machine)==Elf64_Ehdr::EM_386) { /* old style, 8-byte b_info */
            szb_info = 2*sizeof(unsigned);
        }
    }

    fi->seek(overlay_offset - sizeof(l_info), SEEK_SET);
    fi->readx(&linfo, sizeof(linfo));
    lsize = get_te16(&linfo.l_lsize);
    if (UPX_MAGIC_LE32 != get_le32(&linfo.l_magic)) {
        throwCantUnpack("l_info corrupted");
    }
    p_info hbuf;  fi->readx(&hbuf, sizeof(hbuf));
    unsigned orig_file_size = get_te32(&hbuf.p_filesize);
    blocksize = get_te32(&hbuf.p_blocksize);
    if (file_size > (off_t)orig_file_size || blocksize > orig_file_size
        || !mem_size_valid(1, blocksize, OVERHEAD))
        throwCantUnpack("p_info corrupted");

    ibuf.alloc(blocksize + OVERHEAD);
    b_info bhdr; memset(&bhdr, 0, sizeof(bhdr));
    fi->readx(&bhdr, szb_info);
    ph.u_len = get_te32(&bhdr.sz_unc);
    ph.c_len = get_te32(&bhdr.sz_cpr);
    if (ph.c_len > (unsigned)file_size || ph.c_len == 0 || ph.u_len == 0
    ||  ph.u_len > orig_file_size)
        throwCantUnpack("b_info corrupted");
    ph.filter_cto = bhdr.b_cto8;

    MemBuffer u(ph.u_len);
    Elf64_Ehdr *const ehdr = (Elf64_Ehdr *)&u[0];
    Elf64_Phdr const *phdr = 0;

    // Uncompress Ehdr and Phdrs.
    if (ibuf.getSize() < ph.c_len)
        throwCompressedDataViolation();
    fi->readx(ibuf, ph.c_len);
    decompress(ibuf, (upx_byte *)ehdr, false);
    if (ehdr->e_type   !=ehdri.e_type
    ||  ehdr->e_machine!=ehdri.e_machine
    ||  ehdr->e_version!=ehdri.e_version
        // less strict for EM_PPC64 to workaround earlier bug
    ||  !( ehdr->e_flags==ehdri.e_flags
        || Elf64_Ehdr::EM_PPC64 == get_te16(&ehdri.e_machine))
    ||  ehdr->e_ehsize !=ehdri.e_ehsize
        // check EI_MAG[0-3], EI_CLASS, EI_DATA, EI_VERSION
    ||  memcmp(ehdr->e_ident, ehdri.e_ident, Elf64_Ehdr::EI_OSABI)) {
        throwCantUnpack("ElfXX_Ehdr corrupted");
    }
    fi->seek(- (off_t) (szb_info + ph.c_len), SEEK_CUR);

    unsigned const u_phnum = get_te16(&ehdr->e_phnum);
    unsigned total_in = 0;
    unsigned total_out = 0;
    unsigned c_adler = upx_adler32(NULL, 0);
    unsigned u_adler = upx_adler32(NULL, 0);
#define MAX_ELF_HDR 1024
    if ((MAX_ELF_HDR - sizeof(Elf64_Ehdr))/sizeof(Elf64_Phdr) < u_phnum) {
        throwCantUnpack("bad compressed e_phnum");
    }
#undef MAX_ELF_HDR

    // Packed ET_EXE has no PT_DYNAMIC.
    // Packed ET_DYN has original PT_DYNAMIC for info needed by rtld.
    bool const is_shlib = !!elf_find_ptype(Elf64_Phdr::PT_DYNAMIC, phdri, c_phnum);
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
        xct_off = overlay_offset;
        e_shoff = get_te64(&ehdri.e_shoff);
        if (e_shoff && shdri) { // --android-shlib
            upx_uint64_t xct_off2 = get_te64(&shdri->sh_offset);
            if (e_shoff == xct_off2) {
                xct_off = e_shoff;
            }
            // un-Relocate dynsym (DT_SYMTAB) which is below xct_off
            upx_uint64_t const off_dynsym = get_te64(&sec_dynsym->sh_offset);
            upx_uint64_t const sz_dynsym  = get_te64(&sec_dynsym->sh_size);
            Elf64_Sym *const sym0 = (Elf64_Sym *)ibuf.subref(
                "bad dynsym", off_dynsym, sz_dynsym);
            Elf64_Sym *sym = sym0;
            for (int j = sz_dynsym / sizeof(Elf64_Sym); --j>=0; ++sym) {
                upx_uint64_t symval = get_te64(&sym->st_value);
                unsigned symsec = get_te16(&sym->st_shndx);
                if (Elf64_Sym::SHN_UNDEF != symsec
                &&  Elf64_Sym::SHN_ABS   != symsec
                &&  xct_off <= symval) {
                    set_te64(&sym->st_value, symval - asl_delta);
                }
            }
        }
        if (fo) {
            fo->write(ibuf + ph.u_len, xct_off - ph.u_len);
        }
        // Search the Phdrs of compressed
        int n_ptload = 0;
        phdr = (Elf64_Phdr *) (void *) (1+ (Elf64_Ehdr *)(unsigned char *)ibuf);
        for (unsigned j=0; j < u_phnum; ++phdr, ++j) {
            if (PT_LOAD64==get_te32(&phdr->p_type) && 0!=n_ptload++) {
                old_data_off = get_te64(&phdr->p_offset);
                old_data_len = get_te64(&phdr->p_filesz);
                break;
            }
        }

        total_in  = xct_off;
        total_out = xct_off;
        ph.u_len = 0;

        // Decompress and unfilter the tail of first PT_LOAD.
        phdr = (Elf64_Phdr *) (void *) (1+ ehdr);
        for (unsigned j=0; j < u_phnum; ++phdr, ++j) {
            if (PT_LOAD64==get_te32(&phdr->p_type)) {
                ph.u_len = get_te64(&phdr->p_filesz) - xct_off;
                break;
            }
        }
        unpackExtent(ph.u_len, fo, total_in, total_out,
            c_adler, u_adler, false, szb_info);
    }
    else {  // main executable
        // Decompress each PT_LOAD.
        bool first_PF_X = true;
        phdr = (Elf64_Phdr *) (void *) (1+ ehdr);  // uncompressed
        for (unsigned j=0; j < u_phnum; ++phdr, ++j) {
            if (PT_LOAD64==get_te32(&phdr->p_type)) {
                unsigned const filesz = get_te64(&phdr->p_filesz);
                unsigned const offset = get_te64(&phdr->p_offset);
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
    }
    phdr = phdri;
    load_va = 0;
    for (unsigned j=0; j < c_phnum; ++j) {
        if (PT_LOAD64==get_te32(&phdr->p_type)) {
            load_va = get_te64(&phdr->p_vaddr);
            break;
        }
    }
    if (is_shlib
    ||  ((unsigned)(get_te64(&ehdri.e_entry) - load_va) + up4(lsize) +
                ph.getPackHeaderSize() + sizeof(overlay_offset))
            < up4(file_size)) {
        // Loader is not at end; skip past it.
        funpad4(fi);  // MATCH01
        unsigned d_info[6]; fi->readx(d_info, sizeof(d_info));
        if (0==old_dtinit) {
            old_dtinit = d_info[2 + (0==d_info[0])];
            is_asl = 1u& d_info[1];
        }
        fi->seek(lsize - sizeof(d_info), SEEK_CUR);
    }

    // The gaps between PT_LOAD and after last PT_LOAD
    phdr = (Elf64_Phdr *)&u[sizeof(*ehdr)];
    upx_uint64_t hi_offset(0);
    for (unsigned j = 0; j < u_phnum; ++j) {
        if (PT_LOAD64==phdr[j].p_type
        &&  hi_offset < phdr[j].p_offset)
            hi_offset = phdr[j].p_offset;
    }
    for (unsigned j = 0; j < u_phnum; ++j) {
        unsigned const size = find_LOAD_gap(phdr, j, u_phnum);
        if (size) {
            unsigned const where = get_te64(&phdr[j].p_offset) +
                                   get_te64(&phdr[j].p_filesz);
            if (fo)
                fo->seek(where, SEEK_SET);
            unpackExtent(size, fo, total_in, total_out,
                c_adler, u_adler, false, szb_info,
                (phdr[j].p_offset != hi_offset));
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

    if (is_shlib) {
        // DT_INIT must be restored.
        // If android_shlib, then the asl_delta relocations must be un-done.
        int n_ptload = 0;
        upx_uint64_t load_off = 0;
        phdr = (Elf64_Phdr *)&u[sizeof(*ehdr)];
        for (unsigned j= 0; j < u_phnum; ++j, ++phdr) {
            if (PT_LOAD64==get_te32(&phdr->p_type) && 0!=n_ptload++) {
                load_off = get_te64(&phdr->p_offset);
                load_va = get_te64(&phdr->p_vaddr);
                fi->seek(old_data_off, SEEK_SET);
                fi->readx(ibuf, old_data_len);
                total_in  += old_data_len;
                total_out += old_data_len;

                Elf64_Phdr *dynhdr = (Elf64_Phdr *)&u[sizeof(*ehdr)];
                for (unsigned j3= 0; j3 < u_phnum; ++j3, ++dynhdr)
                if (Elf64_Phdr::PT_DYNAMIC==get_te32(&dynhdr->p_type)) {
                    upx_uint64_t dt_pltrelsz(0), dt_jmprel(0);
                    upx_uint64_t dt_relasz(0), dt_rela(0);
                    upx_uint64_t const dyn_off = get_te64(&dynhdr->p_offset);
                    upx_uint64_t const dyn_len = get_te64(&dynhdr->p_filesz);
                    Elf64_Dyn *dyn = (Elf64_Dyn *)((unsigned char *)ibuf +
                        (dyn_off - load_off));
                    for (unsigned j2= 0; j2 < dyn_len; ++dyn, j2 += sizeof(*dyn)) {
                        upx_uint64_t const tag = get_te64(&dyn->d_tag);
                        upx_uint64_t       val = get_te64(&dyn->d_val);
                        if (Elf64_Dyn::DT_INIT == tag) {
                            set_te64(&dyn->d_val, old_dtinit);
                        }
                        if (is_asl) switch (tag) {
                        case Elf64_Dyn::DT_RELASZ:   { dt_relasz   = val; } break;
                        case Elf64_Dyn::DT_RELA:     { dt_rela     = val; } break;
                        case Elf64_Dyn::DT_PLTRELSZ: { dt_pltrelsz = val; } break;
                        case Elf64_Dyn::DT_JMPREL:   { dt_jmprel   = val; } break;

                        case Elf64_Dyn::DT_PLTGOT:
                        case Elf64_Dyn::DT_PREINIT_ARRAY:
                        case Elf64_Dyn::DT_INIT_ARRAY:
                        case Elf64_Dyn::DT_FINI_ARRAY:
                        case Elf64_Dyn::DT_FINI: {
                            set_te64(&dyn->d_val, val - asl_delta);
                        }; break;
                        } // end switch()
                        // Modified DT_*.d_val are re-written later from ibuf[]
                    }
                    if (is_asl) {
                        lowmem.alloc(xct_off);
                        fi->seek(0, SEEK_SET);
                        fi->read(lowmem, xct_off);  // contains relocation tables
                        if (dt_relasz && dt_rela) {
                            Elf64_Rela *const rela0 = (Elf64_Rela *)lowmem.subref(
                                "bad Rela offset", dt_rela, dt_relasz);
                            unRela64(dt_rela, rela0, dt_relasz, ibuf, load_va, fo);
                        }
                        if (dt_pltrelsz && dt_jmprel) { // FIXME:  overlap w/ DT_REL ?
                            Elf64_Rela *const jmp0 = (Elf64_Rela *)lowmem.subref(
                                "bad Jmprel offset", dt_jmprel, dt_pltrelsz);
                            unRela64(dt_jmprel, jmp0, dt_pltrelsz, ibuf, load_va, fo);
                        }
                        // Modified relocation tables are re-written by unRela64
                    }
                }
                if (fo) {
                    fo->seek(get_te64(&phdr->p_offset), SEEK_SET);
                    fo->rewrite(ibuf, old_data_len);
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
PackLinuxElf32::elf_get_offset_from_address(unsigned addr) const
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
PackLinuxElf32::elf_has_dynamic(unsigned int key) const
{
    Elf32_Dyn const *dynp= dynseg;
    if (dynp)
    for (; Elf32_Dyn::DT_NULL!=dynp->d_tag; ++dynp) if (get_te32(&dynp->d_tag)==key) {
        return dynp;
    }
    return 0;
}

unsigned  // checked .p_offset; sz_dynseg set
PackLinuxElf32::check_pt_dynamic(Elf32_Phdr const *const phdr)
{
    unsigned t = get_te32(&phdr->p_offset), s = sizeof(Elf32_Dyn) + t;
    unsigned filesz = get_te32(&phdr->p_filesz), memsz = get_te32(&phdr->p_memsz);
    if (s < t || file_size < (off_t)s
    ||  (3 & t) || (7 & (filesz | memsz))  // .balign 4; 8==sizeof(Elf32_Dyn)
    ||  filesz < sizeof(Elf32_Dyn)
    ||  memsz  < sizeof(Elf32_Dyn)
    ||  filesz < memsz) {
        char msg[50]; snprintf(msg, sizeof(msg), "bad PT_DYNAMIC phdr[%u]",
            (unsigned)(phdr - phdri));
        throwCantPack(msg);
    }
    sz_dynseg = memsz;
    return t;
}

void const *
PackLinuxElf32::elf_find_dynamic(unsigned int key) const
{
    Elf32_Dyn const *dynp= dynseg;
    if (dynp)
    for (; (unsigned)((char const *)dynp - (char const *)dynseg) < sz_dynseg
            && Elf32_Dyn::DT_NULL!=dynp->d_tag; ++dynp) if (get_te32(&dynp->d_tag)==key) {
        unsigned const t= elf_get_offset_from_address(get_te32(&dynp->d_val));
        if (t) {
            return t + file_image;
        }
        break;
    }
    return 0;
}

upx_uint64_t
PackLinuxElf32::elf_unsigned_dynamic(unsigned int key) const
{
    Elf32_Dyn const *dynp= dynseg;
    if (dynp)
    for (; (unsigned)((char const *)dynp - (char const *)dynseg) < sz_dynseg
            && Elf32_Dyn::DT_NULL!=dynp->d_tag; ++dynp) if (get_te32(&dynp->d_tag)==key) {
        return get_te32(&dynp->d_val);
    }
    return 0;
}

upx_uint64_t
PackLinuxElf64::elf_get_offset_from_address(upx_uint64_t addr) const
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
PackLinuxElf64::elf_has_dynamic(unsigned int key) const
{
    Elf64_Dyn const *dynp= dynseg;
    if (dynp)
    for (; Elf64_Dyn::DT_NULL!=dynp->d_tag; ++dynp) if (get_te64(&dynp->d_tag)==key) {
        return dynp;
    }
    return 0;
}

upx_uint64_t  // checked .p_offset; sz_dynseg set
PackLinuxElf64::check_pt_dynamic(Elf64_Phdr const *const phdr)
{
    upx_uint64_t t = get_te64(&phdr->p_offset), s = sizeof(Elf64_Dyn) + t;
    upx_uint64_t filesz = get_te64(&phdr->p_filesz), memsz = get_te64(&phdr->p_memsz);
    if (s < t || (upx_uint64_t)file_size < s
    ||  (7 & t) || (0xf & (filesz | memsz))  // .balign 8; 16==sizeof(Elf64_Dyn)
    ||  filesz < sizeof(Elf64_Dyn)
    ||  memsz  < sizeof(Elf64_Dyn)
    ||  filesz < memsz) {
        char msg[50]; snprintf(msg, sizeof(msg), "bad PT_DYNAMIC phdr[%u]",
            (unsigned)(phdr - phdri));
        throwCantPack(msg);
    }
    sz_dynseg = memsz;
    return t;
}

void const *
PackLinuxElf64::elf_find_dynamic(unsigned int key) const
{
    Elf64_Dyn const *dynp= dynseg;
    if (dynp)
    for (; (unsigned)((char const *)dynp - (char const *)dynseg) < sz_dynseg
            && Elf64_Dyn::DT_NULL!=dynp->d_tag; ++dynp) if (get_te64(&dynp->d_tag)==key) {
        upx_uint64_t const t= elf_get_offset_from_address(get_te64(&dynp->d_val));
        if (t) {
            return &((unsigned char const *)file_image)[(size_t)t];
        }
        break;
    }
    return 0;
}

upx_uint64_t
PackLinuxElf64::elf_unsigned_dynamic(unsigned int key) const
{
    Elf64_Dyn const *dynp= dynseg;
    if (dynp)
    for (; (unsigned)((char const *)dynp - (char const *)dynseg) < sz_dynseg
            && Elf64_Dyn::DT_NULL!=dynp->d_tag; ++dynp) if (get_te64(&dynp->d_tag)==key) {
        return get_te64(&dynp->d_val);
    }
    return 0;
}

unsigned PackLinuxElf::gnu_hash(char const *q)
{
    unsigned char const *p = (unsigned char const *)q;
    unsigned h;

    for (h= 5381; 0!=*p; ++p) {
        h += *p + (h << 5);
    }
    return h;
}

unsigned PackLinuxElf::elf_hash(char const *p)
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

Elf64_Sym const *PackLinuxElf64::elf_lookup(char const *name) const
{
    if (hashtab && dynsym && dynstr) {
        unsigned const nbucket = get_te32(&hashtab[0]);
        unsigned const *const buckets = &hashtab[2];
        unsigned const *const chains = &buckets[nbucket];
        unsigned const m = elf_hash(name) % nbucket;
        unsigned si;
        for (si= get_te32(&buckets[m]); 0!=si; si= get_te32(&chains[si])) {
            char const *const p= get_te64(&dynsym[si].st_name) + dynstr;
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
        upx_uint64_t const *const bitmask = (upx_uint64_t const *)(void const *)&gashtab[4];
        unsigned     const *const buckets = (unsigned const *)&bitmask[n_bitmask];

        unsigned const h = gnu_hash(name);
        unsigned const hbit1 = 077& h;
        unsigned const hbit2 = 077& (h>>gnu_shift);
        upx_uint64_t const w = get_te64(&bitmask[(n_bitmask -1) & (h>>6)]);

        if (1& (w>>hbit1) & (w>>hbit2)) {
            unsigned bucket = get_te32(&buckets[h % n_bucket]);
            if (0!=bucket) {
                Elf64_Sym const *dsp = dynsym;
                unsigned const *const hasharr = &buckets[n_bucket];
                unsigned const *hp = &hasharr[bucket - symbias];

                dsp += bucket;
                do if (0==((h ^ get_te32(hp))>>1)) {
                    char const *const p = get_te64(&dsp->st_name) + dynstr;
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
    if (e_phoff != sizeof(Elf32_Ehdr)) {// Phdrs not contiguous with Ehdr
        throwCantUnpack("bad e_phoff");
    }
    unsigned const c_phnum = get_te16(&ehdri.e_phnum);
    unsigned old_data_off = 0;
    unsigned old_data_len = 0;
    unsigned old_dtinit = 0;
    unsigned is_asl = 0;  // is Android Shared Library

    unsigned szb_info = sizeof(b_info);
    {
        if (get_te32(&ehdri.e_entry) < 0x401180
        &&  Elf32_Ehdr::EM_386 ==get_te16(&ehdri.e_machine)
        &&  Elf32_Ehdr::ET_EXEC==get_te16(&ehdri.e_type)) {
            // Beware ET_DYN.e_entry==0x10f0 (or so) does NOT qualify here.
            /* old style, 8-byte b_info */
            szb_info = 2*sizeof(unsigned);
        }
    }

    fi->seek(overlay_offset - sizeof(l_info), SEEK_SET);
    fi->readx(&linfo, sizeof(linfo));
    lsize = get_te16(&linfo.l_lsize);
    if (UPX_MAGIC_LE32 != get_le32(&linfo.l_magic)) {
        throwCantUnpack("l_info corrupted");
    }
    p_info hbuf;  fi->readx(&hbuf, sizeof(hbuf));
    unsigned orig_file_size = get_te32(&hbuf.p_filesize);
    blocksize = get_te32(&hbuf.p_blocksize);
    if (file_size > (off_t)orig_file_size || blocksize > orig_file_size
        || !mem_size_valid(1, blocksize, OVERHEAD))
        throwCantUnpack("p_info corrupted");

    ibuf.alloc(blocksize + OVERHEAD);
    b_info bhdr; memset(&bhdr, 0, sizeof(bhdr));
    fi->readx(&bhdr, szb_info);
    ph.u_len = get_te32(&bhdr.sz_unc);
    ph.c_len = get_te32(&bhdr.sz_cpr);
    if (ph.c_len > (unsigned)file_size || ph.c_len == 0 || ph.u_len == 0
    ||  ph.u_len > orig_file_size)
        throwCantUnpack("b_info corrupted");
    ph.filter_cto = bhdr.b_cto8;

    MemBuffer u(ph.u_len);
    Elf32_Ehdr *const ehdr = (Elf32_Ehdr *)&u[0];
    Elf32_Phdr const *phdr = 0;

    // Uncompress Ehdr and Phdrs.
    if (ibuf.getSize() < ph.c_len) {
        throwCompressedDataViolation();
    }
    fi->readx(ibuf, ph.c_len);
    decompress(ibuf, (upx_byte *)ehdr, false);
    if (ehdr->e_type   !=ehdri.e_type
    ||  ehdr->e_machine!=ehdri.e_machine
    ||  ehdr->e_version!=ehdri.e_version
    ||  ehdr->e_flags  !=ehdri.e_flags
    ||  ehdr->e_ehsize !=ehdri.e_ehsize
        // check EI_MAG[0-3], EI_CLASS, EI_DATA, EI_VERSION
    ||  memcmp(ehdr->e_ident, ehdri.e_ident, Elf32_Ehdr::EI_OSABI)) {
        throwCantUnpack("ElfXX_Ehdr corrupted");
    }
    fi->seek(- (off_t) (szb_info + ph.c_len), SEEK_CUR);

    unsigned const u_phnum = get_te16(&ehdr->e_phnum);
    unsigned total_in = 0;
    unsigned total_out = 0;
    unsigned c_adler = upx_adler32(NULL, 0);
    unsigned u_adler = upx_adler32(NULL, 0);
#define MAX_ELF_HDR 512
    if ((MAX_ELF_HDR - sizeof(Elf32_Ehdr))/sizeof(Elf32_Phdr) < u_phnum) {
        throwCantUnpack("bad compressed e_phnum");
    }
#undef MAX_ELF_HDR

    // Packed ET_EXE has no PT_DYNAMIC.
    // Packed ET_DYN has original PT_DYNAMIC for info needed by rtld.
    bool const is_shlib = !!elf_find_ptype(Elf32_Phdr::PT_DYNAMIC, phdri, c_phnum);
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
        xct_off = overlay_offset;
        e_shoff = get_te32(&ehdri.e_shoff);
        if (e_shoff && shdri) { // --android-shlib
            unsigned xct_off2 = get_te32(&shdri->sh_offset);
            if (e_shoff == xct_off2) {
                xct_off = e_shoff;
            }
            // un-Relocate dynsym (DT_SYMTAB) which is below xct_off
            unsigned const off_dynsym = get_te32(&sec_dynsym->sh_offset);
            unsigned const sz_dynsym  = get_te32(&sec_dynsym->sh_size);
            Elf32_Sym *const sym0 = (Elf32_Sym *)ibuf.subref(
                "bad dynsym", off_dynsym, sz_dynsym);
            Elf32_Sym *sym = sym0;
            for (int j = sz_dynsym / sizeof(Elf32_Sym); --j>=0; ++sym) {
                unsigned symval = get_te32(&sym->st_value);
                unsigned symsec = get_te16(&sym->st_shndx);
                if (Elf32_Sym::SHN_UNDEF != symsec
                &&  Elf32_Sym::SHN_ABS   != symsec
                &&  xct_off <= symval) {
                    set_te32(&sym->st_value, symval - asl_delta);
                }
            }
        }
        if (fo) {
            fo->write(ibuf + ph.u_len, xct_off - ph.u_len);
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

        total_in  = xct_off;
        total_out = xct_off;
        ph.u_len = 0;

        // Decompress and unfilter the tail of first PT_LOAD.
        phdr = (Elf32_Phdr *) (void *) (1+ ehdr);
        for (unsigned j=0; j < u_phnum; ++phdr, ++j) {
            if (PT_LOAD32==get_te32(&phdr->p_type)) {
                ph.u_len = get_te32(&phdr->p_filesz) - xct_off;
                break;
            }
        }
        unpackExtent(ph.u_len, fo, total_in, total_out,
            c_adler, u_adler, false, szb_info);
    }
    else {  // main executable
        // Decompress each PT_LOAD.
        bool first_PF_X = true;
        phdr = (Elf32_Phdr *) (void *) (1+ ehdr);  // uncompressed
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
    phdr = phdri;
    load_va = 0;
    for (unsigned j=0; j < c_phnum; ++j) {
        if (PT_LOAD32==get_te32(&phdr->p_type)) {
            load_va = get_te32(&phdr->p_vaddr);
            break;
        }
    }
    if (is_shlib
    ||  ((unsigned)(get_te32(&ehdri.e_entry) - load_va) + up4(lsize) +
                ph.getPackHeaderSize() + sizeof(overlay_offset))
            < up4(file_size)) {
        // Loader is not at end; skip past it.
        funpad4(fi);  // MATCH01
        unsigned d_info[4]; fi->readx(d_info, sizeof(d_info));
        if (0==old_dtinit) {
            old_dtinit = d_info[2 + (0==d_info[0])];
            is_asl = 1u& d_info[1];
        }
        fi->seek(lsize - sizeof(d_info), SEEK_CUR);
    }

    // The gaps between PT_LOAD and after last PT_LOAD
    phdr = (Elf32_Phdr *)&u[sizeof(*ehdr)];
    unsigned hi_offset(0);
    for (unsigned j = 0; j < u_phnum; ++j) {
        if (PT_LOAD32==phdr[j].p_type
        &&  hi_offset < phdr[j].p_offset)
            hi_offset = phdr[j].p_offset;
    }
    for (unsigned j = 0; j < u_phnum; ++j) {
        unsigned const size = find_LOAD_gap(phdr, j, u_phnum);
        if (size) {
            unsigned const where = get_te32(&phdr[j].p_offset) +
                                   get_te32(&phdr[j].p_filesz);
            if (fo)
                fo->seek(where, SEEK_SET);
            unpackExtent(size, fo, total_in, total_out,
                c_adler, u_adler, false, szb_info,
                (phdr[j].p_offset != hi_offset));
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

    if (is_shlib) {
        // DT_INIT must be restored.
        // If android_shlib, then the asl_delta relocations must be un-done.
        int n_ptload = 0;
        unsigned load_off = 0;
        phdr = (Elf32_Phdr *)&u[sizeof(*ehdr)];
        for (unsigned j= 0; j < u_phnum; ++j, ++phdr) {
            if (PT_LOAD32==get_te32(&phdr->p_type) && 0!=n_ptload++) {
                load_off = get_te32(&phdr->p_offset);
                load_va  = get_te32(&phdr->p_vaddr);
                fi->seek(old_data_off, SEEK_SET);
                fi->readx(ibuf, old_data_len);
                total_in  += old_data_len;
                total_out += old_data_len;

                Elf32_Phdr *dynhdr = (Elf32_Phdr *)&u[sizeof(*ehdr)];
                for (unsigned j3= 0; j3 < u_phnum; ++j3, ++dynhdr)
                if (Elf32_Phdr::PT_DYNAMIC==get_te32(&dynhdr->p_type)) {
                    unsigned dt_pltrelsz(0), dt_jmprel(0);
                    unsigned dt_relsz(0), dt_rel(0);
                    unsigned const dyn_off = get_te32(&dynhdr->p_offset);
                    unsigned const dyn_len = get_te32(&dynhdr->p_filesz);
                    Elf32_Dyn *dyn = (Elf32_Dyn *)((unsigned char *)ibuf +
                        (dyn_off - load_off));
                    for (unsigned j2= 0; j2 < dyn_len; ++dyn, j2 += sizeof(*dyn)) {
                        unsigned const tag = get_te32(&dyn->d_tag);
                        unsigned       val = get_te32(&dyn->d_val);
                        if (Elf32_Dyn::DT_INIT == tag) {
                            set_te32(&dyn->d_val, old_dtinit);
                        }
                        if (is_asl) switch (tag) {
                        case Elf32_Dyn::DT_RELSZ:    { dt_relsz    = val; } break;
                        case Elf32_Dyn::DT_REL:      { dt_rel      = val; } break;
                        case Elf32_Dyn::DT_PLTRELSZ: { dt_pltrelsz = val; } break;
                        case Elf32_Dyn::DT_JMPREL:   { dt_jmprel   = val; } break;

                        case Elf32_Dyn::DT_PLTGOT:
                        case Elf32_Dyn::DT_PREINIT_ARRAY:
                        case Elf32_Dyn::DT_INIT_ARRAY:
                        case Elf32_Dyn::DT_FINI_ARRAY:
                        case Elf32_Dyn::DT_FINI: {
                            set_te32(&dyn->d_val, val - asl_delta);
                        }; break;
                        } // end switch()
                        // Modified DT_*.d_val are re-written later from ibuf[]
                    }
                    if (is_asl) {
                        lowmem.alloc(xct_off);
                        fi->seek(0, SEEK_SET);
                        fi->read(lowmem, xct_off);  // contains relocation tables
                        if (dt_relsz && dt_rel) {
                            Elf32_Rel *const rel0 = (Elf32_Rel *)lowmem.subref(
                                "bad Rel offset", dt_rel, dt_relsz);
                            unRel32(dt_rel, rel0, dt_relsz, ibuf, load_va, fo);
                        }
                        if (dt_pltrelsz && dt_jmprel) { // FIXME:  overlap w/ DT_REL ?
                            Elf32_Rel *const jmp0 = (Elf32_Rel *)lowmem.subref(
                                "bad Jmprel offset", dt_jmprel, dt_pltrelsz);
                            unRel32(dt_jmprel, jmp0, dt_pltrelsz, ibuf, load_va, fo);
                        }
                        // Modified relocation tables are re-written by unRel32
                    }
                }
                if (fo) {
                    fo->seek(get_te32(&phdr->p_offset), SEEK_SET);
                    fo->rewrite(ibuf, old_data_len);
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
}

void PackLinuxElf::unpack(OutputFile * /*fo*/)
{
    throwCantUnpack("internal error");
}

/* vim:set ts=4 sw=4 et: */
