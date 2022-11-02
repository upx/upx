/* p_lx_elf.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2022 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2022 Laszlo Molnar
   Copyright (C) 2000-2022 John F. Reiser
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

typedef upx_uint32_t u32_t;  // easier to type; more narrow
typedef upx_uint64_t u64_t;  // easier to type; more narrow

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

#define usizeof(x)      ((unsigned) sizeof(x))

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
    if (mb.getVoidPtr() == nullptr) {
        mb.alloc(size);
    } else {
        assert((u32_t)size <= mb.getSize());
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
    : super(f), e_phnum(0), dynstr(nullptr),
    sz_phdrs(0), sz_elf_hdrs(0), sz_pack2(0), sz_pack2a(0),
    lg2_page(12), page_size(1u<<lg2_page), is_pie(0),
    xct_off(0), xct_va(0), jni_onload_va(0),
    user_init_va(0), user_init_off(0),
    e_machine(0), ei_class(0), ei_data(0), ei_osabi(0), osabi_note(nullptr),
    shstrtab(nullptr),
    o_elf_shnum(0)
{
    memset(dt_table, 0, sizeof(dt_table));
}

PackLinuxElf::~PackLinuxElf()
{
}

int PackLinuxElf32::is_LOAD32(Elf32_Phdr const *phdr) const
{
    // (1+ PT_LOPROC) can confuse!
    return PT_LOAD32 == get_te32(&phdr->p_type);
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
    unsigned const last_Phdr = e_phoff + e_phnum * usizeof(Elf32_Phdr);
    if (last_Phdr < e_phoff  // wrap-around
    ||  e_phoff != sizeof(Elf32_Ehdr)  // must be contiguous
    ||  (unsigned long)file_size < last_Phdr) {
        throwCantUnpack("bad e_phoff");
    }
    e_shoff = get_te32(&ehdri.e_shoff);
    unsigned const last_Shdr = e_shoff + e_shnum * usizeof(Elf32_Shdr);
    if (last_Shdr < e_shoff  // wrap-around
    ||  (e_shnum && e_shoff < last_Phdr)
    ||  (unsigned long)file_size < last_Shdr) {
        if (opt->cmd == CMD_COMPRESS) {
            throwCantUnpack("bad e_shoff");
        }
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
        if (opt->cmd != CMD_COMPRESS) {
            shdri = nullptr;
        }
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
            unsigned offset = check_pt_dynamic(phdr);
            dynseg= (Elf32_Dyn const *)(offset + file_image);
            invert_pt_dynamic(dynseg,
                umin(get_te32(&phdr->p_filesz), file_size - offset));
        }
        else if (is_LOAD32(phdr)) {
            check_pt_load(phdr);
        }
        // elf_find_dynamic() returns 0 if 0==dynseg.
        dynstr =      (char const *)elf_find_dynamic(Elf32_Dyn::DT_STRTAB);
        dynsym = (Elf32_Sym const *)elf_find_dynamic(Elf32_Dyn::DT_SYMTAB);
        gashtab = (unsigned const *)elf_find_dynamic(Elf32_Dyn::DT_GNU_HASH);
        hashtab = (unsigned const *)elf_find_dynamic(Elf32_Dyn::DT_HASH);
        if (3& ((upx_uintptr_t)dynsym | (upx_uintptr_t)gashtab | (upx_uintptr_t)hashtab)) {
            throwCantPack("unaligned DT_SYMTAB, DT_GNU_HASH, or DT_HASH/n");
        }
        jni_onload_sym = elf_lookup("JNI_OnLoad");
        if (jni_onload_sym) {
            jni_onload_va = get_te32(&jni_onload_sym->st_value);
            jni_onload_va = 0;  // FIXME not understood; need example
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
    len += t;  // force sz_pack2 (0 mod 8)  [see below]

    set_te32(&disp, sz_elf_hdrs + usizeof(p_info) + usizeof(l_info) +
        (!!xct_off & !!opt->o_unix.android_shlib));  // |1 iff android shlib
    fo->write(&disp, sizeof(disp));  // offset(b_info)
    len += sizeof(disp);
    set_te32(&disp, len);  // distance back to beginning (detect dynamic reloc)
    fo->write(&disp, sizeof(disp));
    len += sizeof(disp);

    if (xct_off) {  // is_shlib
        upx_uint64_t const firstpc_va = (jni_onload_va
            ? jni_onload_va
            : user_init_va);
        set_te32(&disp, firstpc_va - load_va);
        fo->write(&disp, sizeof(disp));  // DT_INIT.d_val
        len += sizeof(disp);

        set_te32(&disp, hatch_off);
        fo->write(&disp, sizeof(disp));  // offset(hatch)
        len += sizeof(disp);

        if (opt->o_unix.android_shlib) {
            xct_off += asl_delta;  // the extra page
        }
        set_te32(&disp, overlay_offset - sizeof(linfo));
        fo->write(&disp, sizeof(disp));  // &{l_info; p_info; b_info}
        len += sizeof(disp);
    }
    sz_pack2 = len;  // 0 mod 8  [see above]

    super::pack3(fo, ft);  // append the decompressor
    set_te16(&linfo.l_lsize, up4(  // MATCH03: up4
    get_te16(&linfo.l_lsize) + len - sz_pack2a));

    return fpad4(fo);  // MATCH03
}

// C_BASE covers the convex hull of the PT_LOAD of the uncompressed module.
// It has (PF_W & .p_flags), and is ".bss": empty (0==.p_filesz, except a bug
// in Linux kernel forces 0x1000==.p_filesz) with .p_memsz equal to the brk(0).
// It is first in order to reserve all // pages, in particular so that if
// (64K == .p_align) but at runtime (4K == PAGE_SIZE) then the Linux kernel
// does not put [vdso] and [vvar] into alignment holes that the UPX runtime stub
// will overwrite.
//
// Note that C_TEXT[.p_vaddr, +.p_memsz) is a subset of C_BASE.
// This requires that the kernel process the ELFxx_Phdr in ascending order,
// and does not mind the overlap.  The UPX runtime stub will "re-program"
// the memory regions anyway.
enum { // ordinals in ELFxx_Phdr[] of compressed output
      C_BASE = 0  // reserve address space
    , C_TEXT = 1  // compressed data and stub
    , C_NOTE = 2  // PT_NOTE copied from input
    , C_GSTK = 3  // PT_GNU_STACK; will be 2 if no PT_NOTE
};

off_t PackLinuxElf32::pack3(OutputFile *fo, Filter &ft)
{
    off_t flen = super::pack3(fo, ft);  // loader follows compressed PT_LOADs
    // NOTE: PackLinuxElf::pack3  adjusted xct_off for the extra page

    unsigned v_hole = sz_pack2 + lsize;
    set_te32(&elfout.phdr[C_TEXT].p_filesz, v_hole);
    set_te32(&elfout.phdr[C_TEXT].p_memsz,  v_hole);
    // Then compressed gaps (including debuginfo.)
    for (unsigned k = 0; k < e_phnum; ++k) {
        Extent x;
        x.size = find_LOAD_gap(phdri, k, e_phnum);
        if (x.size) {
            x.offset = get_te32(&phdri[k].p_offset) +
                       get_te32(&phdri[k].p_filesz);
            packExtent(x, nullptr, fo);
        }
    }
    // write block end marker (uncompressed size 0)
    b_info hdr; memset(&hdr, 0, sizeof(hdr));
    set_le32(&hdr.sz_cpr, UPX_MAGIC_LE32);
    fo->write(&hdr, sizeof(hdr));
    flen = fpad4(fo);

    set_te32(&elfout.phdr[C_TEXT].p_filesz, sz_pack2 + lsize);
    set_te32(&elfout.phdr[C_TEXT].p_memsz,  sz_pack2 + lsize);
    if (0==xct_off) { // not shared library
        set_te32(&elfout.phdr[C_BASE].p_align, 0u - page_mask);
        elfout.phdr[C_BASE].p_paddr = elfout.phdr[C_BASE].p_vaddr;
        elfout.phdr[C_BASE].p_offset = 0;
        // vbase handles ET_EXEC.  FIXME: pre-linking?
        unsigned vbase = get_te32(&elfout.phdr[C_BASE].p_vaddr);
        unsigned abrk = getbrk(phdri, e_phnum);
        set_te32(&elfout.phdr[C_BASE].p_filesz, 0x1000);  // Linux kernel SIGSEGV if (0==.p_filesz)
        set_te32(&elfout.phdr[C_BASE].p_memsz, abrk - vbase);
        set_te32(&elfout.phdr[C_BASE].p_flags, Elf32_Phdr::PF_W|Elf32_Phdr::PF_R);
        set_te32(&elfout.phdr[C_TEXT].p_vaddr, abrk= (page_mask & (~page_mask + abrk)));
        elfout.phdr[C_TEXT].p_paddr = elfout.phdr[C_TEXT].p_vaddr;
        set_te32(&elfout.ehdr.e_entry, abrk + get_te32(&elfout.ehdr.e_entry) - vbase);
    }
    if (0!=xct_off) {  // shared library
        unsigned word = (Elf32_Ehdr::EM_ARM==e_machine) + load_va + sz_pack2;  // Thumb mode
        set_te32(&file_image[user_init_off], word);  // set the hook

        Elf32_Phdr *phdr = (Elf32_Phdr *)lowmem.subref(
                "bad e_phoff", e_phoff, e_phnum * sizeof(Elf32_Phdr));
        unsigned off = fo->st_size();
        so_slide = 0;
        for (unsigned j = 0; j < e_phnum; ++j, ++phdr) {
            unsigned const len  = get_te32(&phdr->p_filesz);
            unsigned const ioff = get_te32(&phdr->p_offset);
            unsigned       align= get_te32(&phdr->p_align);
            unsigned const type = get_te32(&phdr->p_type);
            if (Elf32_Phdr::PT_INTERP==type) {
                // Rotate to highest position, so it can be lopped
                // by decrementing e_phnum.
                memcpy((unsigned char *)ibuf, phdr, sizeof(*phdr));  // extract
                memmove(phdr, 1+phdr, (e_phnum - (1+ j))*sizeof(*phdr));  // overlapping
                memcpy(&phdr[e_phnum - (1+ j)], (unsigned char *)ibuf, sizeof(*phdr));  // to top
                --phdr; --e_phnum;
                set_te16(&ehdri.e_phnum, e_phnum);
                set_te16(&((Elf32_Ehdr *)(unsigned char *)lowmem)->e_phnum, e_phnum);
                continue;
            }
            if (PT_LOAD32 == type) {
                if ((xct_off - ioff) < len) { // Change length of compressed PT_LOAD.
                    set_te32(&phdr->p_filesz, sz_pack2 + lsize - ioff);
                    set_te32(&phdr->p_memsz,  sz_pack2 + lsize - ioff);
                    if (user_init_off < xct_off) { // MIPS puts PT_DYNAMIC here
                        // Allow for DT_INIT in a new [stolen] slot
                        unsigned off2 = user_init_off - sizeof(word);
                        fo->seek(off2, SEEK_SET);
                        fo->rewrite(&file_image[off2], 2*sizeof(word));
                    }
                }
                else if (xct_off < ioff) { // Slide subsequent PT_LOAD.
                    if ((1u<<12) < align) {
                        align = 1u<<12;
                        set_te32(&phdr->p_align, align);
                    }
                    off += (align-1) & (ioff - off);
                    fo->seek(  off, SEEK_SET);
                    fo->write(&file_image[ioff], len);
                    so_slide = off - ioff;
                    set_te32(&phdr->p_offset, so_slide + ioff);
                }
                continue;  // all done with this PT_LOAD
            }
            if (xct_off < ioff)
                set_te32(&phdr->p_offset, so_slide + ioff);
        }  // end each Phdr

        if (opt->o_unix.android_shlib) {
            // Update {DYNAMIC}.sh_offset by so_slide.
            Elf32_Shdr *shdr = (Elf32_Shdr *)lowmem.subref(
                    "bad e_shoff", xct_off - asl_delta, e_shnum * sizeof(Elf32_Shdr));
            for (unsigned j = 0; j < e_shnum; ++shdr, ++j) {
                unsigned sh_type = get_te32(&shdr->sh_type);
                if (Elf32_Shdr::SHT_DYNAMIC == get_te32(&shdr->sh_type)) {
                    unsigned offset = get_te32(&shdr->sh_offset);
                    set_te32(&shdr->sh_offset, so_slide + offset );
                    fo->seek((j * sizeof(Elf32_Shdr)) + xct_off - asl_delta, SEEK_SET);
                    fo->rewrite(shdr, sizeof(*shdr));
                    fo->seek(0, SEEK_END);
                }
                if (Elf32_Shdr::SHT_REL == sh_type
                &&  n_jmp_slot
                &&  !strcmp(".rel.plt", get_te32(&shdr->sh_name) + shstrtab)) {
                    unsigned f_off = elf_get_offset_from_address(plt_off);
                    fo->seek(so_slide + f_off, SEEK_SET);  // FIXME: assumes PT_LOAD[1]
                    fo->rewrite(&file_image[f_off], n_jmp_slot * 4);
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
    off_t flen = super::pack3(fo, ft);  // loader follows compressed PT_LOADs
    // NOTE: PackLinuxElf::pack3  adjusted xct_off for the extra page

    unsigned v_hole = sz_pack2 + lsize;
    set_te64(&elfout.phdr[C_TEXT].p_filesz, v_hole);
    set_te64(&elfout.phdr[C_TEXT].p_memsz,  v_hole);
    // Then compressed gaps (including debuginfo.)
    for (unsigned k = 0; k < e_phnum; ++k) {
        Extent x;
        x.size = find_LOAD_gap(phdri, k, e_phnum);
        if (x.size) {
            x.offset = get_te64(&phdri[k].p_offset) +
                       get_te64(&phdri[k].p_filesz);
            packExtent(x, nullptr, fo);
        }
    }
    // write block end marker (uncompressed size 0)
    b_info hdr; memset(&hdr, 0, sizeof(hdr));
    set_le32(&hdr.sz_cpr, UPX_MAGIC_LE32);
    fo->write(&hdr, sizeof(hdr));
    flen = fpad4(fo);

    set_te64(&elfout.phdr[C_TEXT].p_filesz, sz_pack2 + lsize);
    set_te64(&elfout.phdr[C_TEXT].p_memsz,  sz_pack2 + lsize);
    if (0==xct_off) { // not shared library
        set_te64(&elfout.phdr[C_BASE].p_align, ((upx_uint64_t)0) - page_mask);
        elfout.phdr[C_BASE].p_paddr = elfout.phdr[C_BASE].p_vaddr;
        elfout.phdr[C_BASE].p_offset = 0;
        upx_uint64_t abrk = getbrk(phdri, e_phnum);
        // vbase handles ET_EXEC.  FIXME: pre-linking?
        upx_uint64_t const vbase = get_te64(&elfout.phdr[C_BASE].p_vaddr);
        set_te64(&elfout.phdr[C_BASE].p_filesz, 0x1000);  // Linux kernel SIGSEGV if (0==.p_filesz)
        set_te64(&elfout.phdr[C_BASE].p_memsz, abrk - vbase);
        set_te32(&elfout.phdr[C_BASE].p_flags, Elf32_Phdr::PF_W|Elf32_Phdr::PF_R);
        set_te64(&elfout.phdr[C_TEXT].p_vaddr, abrk= (page_mask & (~page_mask + abrk)));
        elfout.phdr[C_TEXT].p_paddr = elfout.phdr[C_TEXT].p_vaddr;
        set_te64(&elfout.ehdr.e_entry, abrk + get_te64(&elfout.ehdr.e_entry) - vbase);
    }
    if (0!=xct_off) {  // shared library
        upx_uint64_t word = load_va + sz_pack2;
        set_te64(&file_image[user_init_off], word);  // set the hook

        Elf64_Phdr *phdr = (Elf64_Phdr *)lowmem.subref(
                "bad e_phoff", e_phoff, e_phnum * sizeof(Elf64_Phdr));
        unsigned off = fo->st_size();
        so_slide = 0;
        for (unsigned j = 0; j < e_phnum; ++j, ++phdr) {
            upx_uint64_t const len  = get_te64(&phdr->p_filesz);
            upx_uint64_t const ioff = get_te64(&phdri[j].p_offset);
            upx_uint64_t       align= get_te64(&phdr->p_align);
            unsigned const type = get_te32(&phdr->p_type);
            if (Elf64_Phdr::PT_INTERP==type) {
                // Rotate to highest position, so it can be lopped
                // by decrementing e_phnum.
                memcpy((unsigned char *)ibuf, phdr, sizeof(*phdr));  // extract
                memmove(phdr, 1+phdr, (e_phnum - (1+ j))*sizeof(*phdr));  // overlapping
                memcpy(&phdr[e_phnum - (1+ j)], (unsigned char *)ibuf, sizeof(*phdr));  // to top
                --phdr; --e_phnum;
                set_te16(&ehdri.e_phnum, e_phnum);
                set_te16(&((Elf64_Ehdr *)(unsigned char *)lowmem)->e_phnum, e_phnum);
                continue;
            }
            if (PT_LOAD64 == type) {
                if ((xct_off - ioff) < len) { // Change length of compressed PT_LOAD.
                    set_te64(&phdr->p_filesz, sz_pack2 + lsize);
                    set_te64(&phdr->p_memsz,  sz_pack2 + lsize);
                    if (user_init_off < xct_off) { // MIPS puts PT_DYNAMIC here
                        // Allow for DT_INIT in a new [stolen] slot
                        unsigned off2 = user_init_off - sizeof(word);
                        fo->seek(off2, SEEK_SET);
                        fo->rewrite(&file_image[off2], 2*sizeof(word));
                    }
                }
                else if (j && (Elf64_Phdr::PF_W & get_te64(&phdr->p_flags))
                     &&  xct_off < ioff) {  // Slide subsequent PT_LOAD.
                    // AMD64 chip supports page sizes of 4KiB, 2MiB, and 1GiB;
                    // the operating system chooses one.  .p_align typically
                    // is a forward-looking 2MiB.  In 2009 Linux chooses 4KiB.
                    // We choose 4KiB to waste less space.  If Linux chooses
                    // 2MiB later, then our output will not run.
                    if ((1u<<12) < align
                    &&  Elf64_Ehdr::EM_X86_64 ==e_machine
                    ) {
                        align = 1u<<12;
                        set_te64(&phdr->p_align, align);
                    }
                    off += (align-1) & (ioff - off);
                    set_te64(&phdr->p_offset, off);
                    so_slide = off - ioff;
                    fo->seek(  off, SEEK_SET);
                    fo->write(&file_image[ioff], len);
                    off += len;
                }
                continue;  // all done with this PT_LOAD
            }
            if (xct_off < ioff)
                set_te64(&phdr->p_offset, so_slide + ioff);
        }  // end each Phdr

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
                    fo->rewrite(&file_image[f_off], n_jmp_slot * 8);
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
    addLoader("ELFMAINX", nullptr);
    if (hasLoaderSection("ELFMAINXu")) {
            // brk() trouble if static
        addLoader("ELFMAINXu", nullptr);
    }
   //addLoader(getDecompressorSections(), nullptr);
    addLoader(
        ( M_IS_NRV2E(ph.method) ? "NRV_HEAD,NRV2E,NRV_TAIL"
        : M_IS_NRV2D(ph.method) ? "NRV_HEAD,NRV2D,NRV_TAIL"
        : M_IS_NRV2B(ph.method) ? "NRV_HEAD,NRV2B,NRV_TAIL"
        : M_IS_LZMA(ph.method)  ? "LZMA_ELF00,LZMA_DEC20,LZMA_DEC30"
        : nullptr), nullptr);
    if (hasLoaderSection("CFLUSH"))
        addLoader("CFLUSH");
    addLoader("ELFMAINY,IDENTSTR", nullptr);
    if (hasLoaderSection("ELFMAINZe")) { // ppc64 big-endian only
        addLoader("ELFMAINZe", nullptr);
    }
    addLoader("+40,ELFMAINZ", nullptr);
    if (hasLoaderSection("ANDMAJNZ")) { // Android trouble with args to DT_INIT
        if (opt->o_unix.android_shlib) {
            addLoader("ANDMAJNZ", nullptr);  // constant PAGE_SIZE
        }
        else {
            addLoader("ELFMAJNZ", nullptr);  // PAGE_SIZE from AT_PAGESZ
        }
        addLoader("ELFMAKNZ", nullptr);
    }
    if (hasLoaderSection("ELFMAINZu")) {
        addLoader("ELFMAINZu", nullptr);
    }
    addLoader("FOLDEXEC", nullptr);
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
    : super(f), phdri(nullptr), shdri(nullptr),
    gnu_stack(nullptr),
    page_mask(~0u<<lg2_page),
    dynseg(nullptr), hashtab(nullptr), hashend(nullptr),
                     gashtab(nullptr), gashend(nullptr), dynsym(nullptr),
    jni_onload_sym(nullptr),
    sec_strndx(nullptr), sec_dynsym(nullptr), sec_dynstr(nullptr)
    , symnum_end(0)
{
    memset(&ehdri, 0, sizeof(ehdri));
    if (f) {
        f->seek(0, SEEK_SET);
        f->readx(&ehdri, sizeof(ehdri));
    }
}

PackLinuxElf32::~PackLinuxElf32()
{
}

PackLinuxElf64::PackLinuxElf64(InputFile *f)
    : super(f), phdri(nullptr), shdri(nullptr),
    gnu_stack(nullptr),
    page_mask(~0ull<<lg2_page),
    dynseg(nullptr), hashtab(nullptr), hashend(nullptr),
                     gashtab(nullptr), gashend(nullptr), dynsym(nullptr),
    jni_onload_sym(nullptr),
    sec_strndx(nullptr), sec_dynsym(nullptr), sec_dynstr(nullptr)
    , symnum_end(0)
{
    memset(&ehdri, 0, sizeof(ehdri));
    if (f) {
        f->seek(0, SEEK_SET);
        f->readx(&ehdri, sizeof(ehdri));
    }
}

PackLinuxElf64::~PackLinuxElf64()
{
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
    upx_uint64_t const last_Phdr = e_phoff + e_phnum * sizeof(Elf64_Phdr);
    if (last_Phdr < e_phoff  // wrap-around
    ||  e_phoff != sizeof(Elf64_Ehdr)  // must be contiguous
    ||  (unsigned long)file_size < last_Phdr) {
        throwCantUnpack("bad e_phoff");
    }
    e_shoff = get_te64(&ehdri.e_shoff);
    upx_uint64_t const last_Shdr = e_shoff + e_shnum * sizeof(Elf64_Shdr);
    if (last_Shdr < e_shoff  // wrap-around
    ||  (e_shnum && e_shoff < last_Phdr)
    ||  (unsigned long)file_size < last_Shdr) {
        if (opt->cmd == CMD_COMPRESS) {
            throwCantUnpack("bad e_shoff");
        }
    }
    sz_phdrs = e_phnum * e_phentsize;
    sz_elf_hdrs = sz_phdrs + sizeof(Elf64_Ehdr);

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
        if (opt->cmd != CMD_COMPRESS) {
            shdri = nullptr;
        }
        sec_dynsym = elf_find_section_type(Elf64_Shdr::SHT_DYNSYM);
        if (sec_dynsym) {
            unsigned t = get_te32(&sec_dynsym->sh_link);
            if (e_shnum <= t)
                throwCantPack("bad dynsym->sh_link");
            sec_dynstr = &shdri[t];
        }

        Elf64_Phdr const *phdr= phdri;
        for (int j = e_phnum; --j>=0; ++phdr)
        if (Elf64_Phdr::PT_DYNAMIC==get_te32(&phdr->p_type)) {
            upx_uint64_t offset = check_pt_dynamic(phdr);
            dynseg= (Elf64_Dyn const *)(offset + file_image);
            invert_pt_dynamic(dynseg,
                umin(get_te64(&phdr->p_filesz), file_size - offset));
        }
        else if (PT_LOAD64==get_te32(&phdr->p_type)) {
            check_pt_load(phdr);
        }
        // elf_find_dynamic() returns 0 if 0==dynseg.
        dynstr =      (char const *)elf_find_dynamic(Elf64_Dyn::DT_STRTAB);
        dynsym = (Elf64_Sym const *)elf_find_dynamic(Elf64_Dyn::DT_SYMTAB);
        gashtab = (unsigned const *)elf_find_dynamic(Elf64_Dyn::DT_GNU_HASH);
        hashtab = (unsigned const *)elf_find_dynamic(Elf64_Dyn::DT_HASH);
        if (3& ((upx_uintptr_t)dynsym | (upx_uintptr_t)gashtab | (upx_uintptr_t)hashtab)) {
            throwCantPack("unaligned DT_SYMTAB, DT_GNU_HASH, or DT_HASH/n");
        }
        jni_onload_sym = elf_lookup("JNI_OnLoad");
        if (jni_onload_sym) {
            jni_onload_va = get_te64(&jni_onload_sym->st_value);
            jni_onload_va = 0;  // FIXME not understood; need example
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
        get_te32(&elfout.phdr[C_TEXT].p_vaddr));
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
    unsigned vbase = get_te32(&elfout.phdr[C_TEXT].p_vaddr);
    set_te32(&elfout.ehdr.e_entry, start + sz_pack2 + vbase);
}

void PackLinuxElf64::updateLoader(OutputFile * /*fo*/)
{
    if (xct_off) {
        return;  // FIXME elfout has no values at all
    }
    upx_uint64_t const vbase = get_te64(&elfout.phdr[C_TEXT].p_vaddr);
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
    // Why did PackLinuxElf64Le set lg2_page = 16 ?
    // It causes trouble for check_pt_dynamic() from canPack().
    lg2_page = 12;  page_size = 1u<<lg2_page;
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
//            ? (unsigned) get_te32(&elfout.phdr[C_NOTE].p_memsz)
//            : 0) +
//            // p_progid, p_filesize, p_blocksize
//        sizeof(p_info) +
//            // compressed data
//        b_len + ph.c_len );

            // entry to stub
    addLoader("LEXEC000", nullptr);

    if (ft->id) {
        { // decompr, unfilter are separate
            addLoader("LXUNF000", nullptr);
            addLoader("LXUNF002", nullptr);
                if (0x80==(ft->id & 0xF0)) {
                    if (256==n_mru) {
                        addLoader("MRUBYTE0", nullptr);
                    }
                    else if (n_mru) {
                        addLoader("LXMRU005", nullptr);
                    }
                    if (n_mru) {
                        addLoader("LXMRU006", nullptr);
                    }
                    else {
                        addLoader("LXMRU007", nullptr);
                    }
            }
            else if (0x40==(ft->id & 0xF0)) {
                addLoader("LXUNF008", nullptr);
            }
            addLoader("LXUNF010", nullptr);
        }
        if (n_mru) {
            addLoader("LEXEC009", nullptr);
        }
    }
    addLoader("LEXEC010", nullptr);
    addLoader(getDecompressorSections(), nullptr);
    addLoader("LEXEC015", nullptr);
    if (ft->id) {
        {  // decompr, unfilter are separate
            if (0x80!=(ft->id & 0xF0)) {
                addLoader("LXUNF042", nullptr);
            }
        }
        addFilter32(ft->id);
        { // decompr, unfilter are separate
            if (0x80==(ft->id & 0xF0)) {
                if (0==n_mru) {
                    addLoader("LXMRU058", nullptr);
                }
            }
            addLoader("LXUNF035", nullptr);
        }
    }
    else {
        addLoader("LEXEC017", nullptr);
    }

    addLoader("IDENTSTR", nullptr);
    addLoader("LEXEC020", nullptr);
    addLoader("FOLDEXEC", nullptr);
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
    fold_hdrlen = umax(0x80, usizeof(hf->ehdr) +
        get_te16(&hf->ehdr.e_phentsize) * get_te16(&hf->ehdr.e_phnum) +
            sizeof(l_info) );
    h.sz_unc = ((szfold < fold_hdrlen) ? 0 : (szfold - fold_hdrlen));
    h.b_method = (unsigned char) ph.method;
    h.b_ftid = (unsigned char) ph.filter;
    h.b_cto8 = (unsigned char) ph.filter_cto;
    unsigned char const *const uncLoader = fold_hdrlen + fold;

    MemBuffer mb_cprLoader;
    mb_cprLoader.allocForCompression(h.sz_unc + (0==h.sz_unc));
    h.sz_cpr = mb_cprLoader.getSize();
    unsigned char *const cprLoader = (unsigned char *)mb_cprLoader;
    {
    unsigned h_sz_cpr = h.sz_cpr;
    int r = upx_compress(uncLoader, h.sz_unc, sizeof(h) + cprLoader, &h_sz_cpr,
        nullptr, ph.method, 10, nullptr, nullptr );
    h.sz_cpr = h_sz_cpr;
    if (r != UPX_E_OK || h.sz_cpr >= h.sz_unc)
        throwInternalError("loader compression failed");
    }
#if 0  //{  debugging only
    if (M_IS_LZMA(ph.method)) {
        ucl_uint tmp_len = h.sz_unc;  // LZMA uses this as EOF
        MemBuffer mb_tmp(tmp_len);
        unsigned char *tmp = (unsigned char *)mb_tmp;
        memset(tmp, 0, tmp_len);
        int r = upx_decompress(sizeof(h) + cprLoader, h.sz_cpr, tmp, &tmp_len, h.b_method, nullptr);
        if (r == UPX_E_OUT_OF_MEMORY)
            throwOutOfMemoryException();
        printf("\n%d %d: %d %d %d\n", h.b_method, r, h.sz_cpr, h.sz_unc, tmp_len);
        for (unsigned j=0; j < h.sz_unc; ++j) if (tmp[j]!=uncLoader[j]) {
            printf("%d: %x %x\n", j, tmp[j], uncLoader[j]);
        }
    }
#endif  //}
    unsigned const sz_cpr = h.sz_cpr;
    set_te32(&h.sz_cpr, h.sz_cpr);
    set_te32(&h.sz_unc, h.sz_unc);
    memcpy(cprLoader, &h, sizeof(h));

    // This adds the definition to the "library", to be used later.
    linker->addSection("FOLDEXEC", cprLoader, sizeof(h) + sz_cpr, 0);
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
    fold_hdrlen = umax(0x80, usizeof(hf->ehdr) +
        get_te16(&hf->ehdr.e_phentsize) * get_te16(&hf->ehdr.e_phnum) +
            sizeof(l_info) );
    h.sz_unc = ((szfold < fold_hdrlen) ? 0 : (szfold - fold_hdrlen));
    h.b_method = (unsigned char) ph.method;
    h.b_ftid = (unsigned char) ph.filter;
    h.b_cto8 = (unsigned char) ph.filter_cto;
    unsigned char const *const uncLoader = fold_hdrlen + fold;

    MemBuffer mb_cprLoader;
    mb_cprLoader.allocForCompression(h.sz_unc + (0==h.sz_unc));
    h.sz_cpr = mb_cprLoader.getSize();
    unsigned char *const cprLoader = (unsigned char *)mb_cprLoader;
    {
    unsigned h_sz_cpr = h.sz_cpr;
    int r = upx_compress(uncLoader, h.sz_unc, sizeof(h) + cprLoader, &h_sz_cpr,
        nullptr, forced_method(ph.method), 10, nullptr, nullptr );
    h.sz_cpr = h_sz_cpr;
    if (r != UPX_E_OK || h.sz_cpr >= h.sz_unc)
        throwInternalError("loader compression failed");
    }
#if 0  //{  debugging only
    if (M_IS_LZMA(ph.method)) {
        ucl_uint tmp_len = h.sz_unc;  // LZMA uses this as EOF
        MemBuffer mb_tmp(tmp_len);
        unsigned char *tmp = (unsigned char *)mb_tmp;
        memset(tmp, 0, tmp_len);
        int r = upx_decompress(sizeof(h) + cprLoader, h.sz_cpr, tmp, &tmp_len, h.b_method, nullptr);
        if (r == UPX_E_OUT_OF_MEMORY)
            throwOutOfMemoryException();
        printf("\n%d %d: %d %d %d\n", h.b_method, r, h.sz_cpr, h.sz_unc, tmp_len);
        for (unsigned j=0; j < h.sz_unc; ++j) if (tmp[j]!=uncLoader[j]) {
            printf("%d: %x %x\n", j, tmp[j], uncLoader[j]);
        }
    }
#endif  //}
    unsigned const sz_cpr = h.sz_cpr;
    set_te32(&h.sz_cpr, h.sz_cpr);
    set_te32(&h.sz_unc, h.sz_unc);
    memcpy(cprLoader, &h, sizeof(h));

    // This adds the definition to the "library", to be used later.
    linker->addSection("FOLDEXEC", cprLoader, sizeof(h) + sz_cpr, 0);
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
            nullptr,                       0,                                ft );
        return;
    }
    unsigned char tmp[sizeof(stub_i386_linux_elf_fold)];
    memcpy(tmp, stub_i386_linux_elf_fold, sizeof(stub_i386_linux_elf_fold));
    checkPatch(nullptr, 0, 0, 0);  // reset
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
    checkPatch(nullptr, 0, 0, 0);  // reset
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
    checkPatch(nullptr, 0, 0, 0);  // reset
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
    checkPatch(nullptr, 0, 0, 0);  // reset
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
                nullptr,                      0,                                ft );
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
            nullptr,                        0,                                 ft );
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
            nullptr,                        0,                                 ft );
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
            nullptr,                        0,                                 ft );
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
            nullptr,                        0,                                 ft );
        return;
    }
    buildLinuxLoader(
        stub_arm64_linux_elf_entry, sizeof(stub_arm64_linux_elf_entry),
        stub_arm64_linux_elf_fold,  sizeof(stub_arm64_linux_elf_fold), ft);
}

void
PackLinuxElf32::invert_pt_dynamic(Elf32_Dyn const *dynp, unsigned headway)
{
    if (dt_table[Elf32_Dyn::DT_NULL]) {
        return;  // not 1st time; do not change upx_dt_init
    }
    Elf32_Dyn const *const dynp0 = dynp;
    unsigned ndx = 0;
    unsigned const limit = headway / sizeof(*dynp);
    if (dynp)
    for (; ; ++ndx, ++dynp) {
        if (limit <= ndx) {
            throwCantPack("DT_NULL not found");
        }
        unsigned const d_tag = get_te32(&dynp->d_tag);
        if (d_tag < DT_NUM) {
            if (Elf32_Dyn::DT_NEEDED != d_tag
            &&  dt_table[d_tag]
            &&    get_te32(&dynp->d_val)
               != get_te32(&dynp0[-1+ dt_table[d_tag]].d_val)) {
                char msg[50]; snprintf(msg, sizeof(msg),
                    "duplicate DT_%#x: [%#x] [%#x]",
                    d_tag, -1+ dt_table[d_tag], ndx);
                throwCantPack(msg);
            }
            dt_table[d_tag] = 1+ ndx;
        }
        if (Elf32_Dyn::DT_NULL == d_tag) {
            break;  // check here so that dt_table[DT_NULL] is set
        }
    }
    upx_dt_init = 0;
         if (dt_table[Elf32_Dyn::DT_INIT])          upx_dt_init = Elf32_Dyn::DT_INIT;
    else if (dt_table[Elf32_Dyn::DT_PREINIT_ARRAY]) upx_dt_init = Elf32_Dyn::DT_PREINIT_ARRAY;
    else if (dt_table[Elf32_Dyn::DT_INIT_ARRAY])    upx_dt_init = Elf32_Dyn::DT_INIT_ARRAY;

    unsigned const z_str = dt_table[Elf32_Dyn::DT_STRSZ];
    strtab_end = !z_str ? 0 : get_te64(&dynp0[-1+ z_str].d_val);
    if (!z_str || (u64_t)file_size <= strtab_end) { // FIXME: weak
        char msg[50]; snprintf(msg, sizeof(msg),
            "bad DT_STRSZ %#x", strtab_end);
        throwCantPack(msg);
    }
    unsigned const x_sym = dt_table[Elf32_Dyn::DT_SYMTAB];
    unsigned const x_str = dt_table[Elf32_Dyn::DT_STRTAB];
    if (x_sym && x_str) {
        upx_uint32_t const v_sym = get_te32(&dynp0[-1+ x_sym].d_val);
        upx_uint32_t const v_str = get_te32(&dynp0[-1+ x_str].d_val);
        unsigned const  z_sym = dt_table[Elf32_Dyn::DT_SYMENT];
        unsigned const sz_sym = !z_sym ? sizeof(Elf32_Sym)
            : get_te32(&dynp0[-1+ z_sym].d_val);
        if (sz_sym < sizeof(Elf32_Sym)) {
            char msg[50]; snprintf(msg, sizeof(msg),
                "bad DT_SYMENT %x", sz_sym);
            throwCantPack(msg);
        }
        if (v_sym < v_str) {
            symnum_end = (v_str - v_sym) / sz_sym;
        }
        if (symnum_end < 1) {
            throwCantPack("bad DT_SYMTAB");
        }
    }
    // DT_HASH often ends at DT_SYMTAB
    // FIXME: sort DT_HASH, DT_GNU_HASH, STRTAB, SYMTAB, REL, RELA, JMPREL
    // to partition the space.
    unsigned const v_hsh = elf_unsigned_dynamic(Elf32_Dyn::DT_HASH);
    if (v_hsh && file_image) {
        hashtab = (unsigned const *)elf_find_dynamic(Elf32_Dyn::DT_HASH);
        if (!hashtab) {
            char msg[40]; snprintf(msg, sizeof(msg),
               "bad DT_HASH %#x", v_hsh);
            throwCantPack(msg);
        }
        unsigned const nbucket = get_te32(&hashtab[0]);
        unsigned const *const buckets = &hashtab[2];
        unsigned const *const chains = &buckets[nbucket]; (void)chains;

        unsigned const v_sym = !x_sym ? 0 : get_te32(&dynp0[-1+ x_sym].d_val);
        if ((unsigned)file_size <= nbucket/sizeof(*buckets)  // FIXME: weak
        || !v_sym || (unsigned)file_size <= v_sym
        || ((v_hsh < v_sym) && (v_sym - v_hsh) < sizeof(*buckets)*(2+ nbucket))
        ) {
            char msg[80]; snprintf(msg, sizeof(msg),
                "bad DT_HASH nbucket=%#x  len=%#x",
                nbucket, (v_sym - v_hsh));
            throwCantPack(msg);
        }
        unsigned chmax = 0;
        for (unsigned j= 0; j < nbucket; ++j) {
            unsigned x = get_te32(&buckets[j]);
            if (chmax < x) {
                chmax = x;
            }
        }
        if ((v_hsh < v_sym) && (v_sym - v_hsh) <
                (sizeof(*buckets)*(2+ nbucket) + sizeof(*chains)*(1+ chmax))) {
            char msg[80]; snprintf(msg, sizeof(msg),
                "bad DT_HASH nbucket=%#x  len=%#x",
                nbucket, (v_sym - v_hsh));
            throwCantPack(msg);
        }
    }
    // DT_GNU_HASH often ends at DT_SYMTAB;  FIXME: not for Android?
    unsigned const v_gsh = elf_unsigned_dynamic(Elf32_Dyn::DT_GNU_HASH);
    if (v_gsh && file_image) {
        gashtab = (unsigned const *)elf_find_dynamic(Elf32_Dyn::DT_GNU_HASH);
        if (!gashtab) {
            char msg[40]; snprintf(msg, sizeof(msg),
               "bad DT_GNU_HASH %#x", v_gsh);
            throwCantPack(msg);
        }
        unsigned const n_bucket = get_te32(&gashtab[0]);
        unsigned const symbias  = get_te32(&gashtab[1]);
        unsigned const n_bitmask = get_te32(&gashtab[2]);
        unsigned const gnu_shift = get_te32(&gashtab[3]);
        unsigned const *const bitmask = (unsigned const *)(void const *)&gashtab[4];
        unsigned     const *const buckets = (unsigned const *)&bitmask[n_bitmask];
        unsigned     const *const hasharr = &buckets[n_bucket]; (void)hasharr;
        if (!n_bucket || (1u<<31) <= n_bucket  /* fie on fuzzers */
        || (void const *)&file_image[file_size] <= (void const *)hasharr) {
            char msg[80]; snprintf(msg, sizeof(msg),
                "bad n_bucket %#x\n", n_bucket);
            throwCantPack(msg);
        }
        //unsigned const *const gashend = &hasharr[n_bucket];  // minimum, except:
        // Rust and Android trim unused zeroes from high end of hasharr[]
        unsigned bmax = 0;
        for (unsigned j= 0; j < n_bucket; ++j) {
            unsigned bj = get_te32(&buckets[j]);
            if (bj) {
                if (bj < symbias) {
                    char msg[90]; snprintf(msg, sizeof(msg),
                            "bad DT_GNU_HASH bucket[%d] < symbias{%#x}\n",
                            bj, symbias);
                    throwCantPack(msg);
                }
                if (bmax < bj) {
                    bmax = bj;
                }
            }
        }
        if (1==n_bucket  && 0==buckets[0]
        &&  1==n_bitmask && 0==bitmask[0]) {
            // 2021-09-11 Rust on RaspberryPi apparently uses this to minimize space.
            // But then the DT_GNU_HASH symbol lookup algorithm always fails?
            // https://github.com/upx/upx/issues/525
        } else
        if ((1+ bmax) < symbias) {
            char msg[90]; snprintf(msg, sizeof(msg),
                    "bad DT_GNU_HASH (1+ max_bucket)=%#x < symbias=%#x", 1+ bmax, symbias);
            throwCantPack(msg);
        }
        bmax -= symbias;

        unsigned const v_sym = !x_sym ? 0 : get_te32(&dynp0[-1+ x_sym].d_val);
        unsigned r = 0;
        if (!n_bucket || !n_bitmask || !v_sym
        || (r=1, ((-1+ n_bitmask) & n_bitmask))  // not a power of 2
        || (r=2, (8*sizeof(unsigned) <= gnu_shift))  // shifted result always == 0
        || (r=3, (n_bucket>>30))  // fie on fuzzers
        || (r=4, (n_bitmask>>30))
        || (r=5, ((file_size/sizeof(unsigned))
                <= ((sizeof(*bitmask)/sizeof(unsigned))*n_bitmask + 2*n_bucket)))  // FIXME: weak
        || (r=6, ((v_gsh < v_sym) && (v_sym - v_gsh) < (sizeof(unsigned)*4  // headers
                + sizeof(*bitmask)*n_bitmask  // bitmask
                + sizeof(*buckets)*n_bucket  // buckets
                + sizeof(*hasharr)*(1+ bmax)  // hasharr
            )) )
        ) {
            char msg[90]; snprintf(msg, sizeof(msg),
                "bad DT_GNU_HASH n_bucket=%#x  n_bitmask=%#x  len=%#lx  r=%d",
                n_bucket, n_bitmask, (long unsigned)(v_sym - v_gsh), r);
            throwCantPack(msg);
        }
    }
    unsigned const e_shstrndx = get_te16(&ehdri.e_shstrndx);
    if (e_shnum <= e_shstrndx
    &&  !(0==e_shnum && 0==e_shstrndx) ) {
        char msg[40]; snprintf(msg, sizeof(msg),
            "bad .e_shstrndx %d >= .e_shnum %d", e_shstrndx, e_shnum);
        throwCantPack(msg);
    }
}

Elf32_Phdr const *
PackLinuxElf32::elf_find_ptype(unsigned type, Elf32_Phdr const *phdr, unsigned phnum)
{
    for (unsigned j = 0; j < phnum; ++j, ++phdr) {
        if (type == get_te32(&phdr->p_type)) {
            return phdr;
        }
    }
    return nullptr;
}

Elf64_Phdr const *
PackLinuxElf64::elf_find_ptype(unsigned type, Elf64_Phdr const *phdr, unsigned phnum)
{
    for (unsigned j = 0; j < phnum; ++j, ++phdr) {
        if (type == get_te32(&phdr->p_type)) {
            return phdr;
        }
    }
    return nullptr;
}

Elf32_Shdr const *PackLinuxElf32::elf_find_section_name(
    char const *const name
) const
{
    Elf32_Shdr const *shdr = shdri;
    if (!shdr) {
        return nullptr;
    }
    int j = e_shnum;
    for (; 0 <=--j; ++shdr) {
        unsigned const sh_name = get_te32(&shdr->sh_name);
        if ((u32_t)file_size <= sh_name) {  // FIXME: weak
            char msg[50]; snprintf(msg, sizeof(msg),
                "bad Elf32_Shdr[%d].sh_name %#x",
                -1+ e_shnum -j, sh_name);
            throwCantPack(msg);
        }
        if (0==strcmp(name, &shstrtab[sh_name])) {
            return shdr;
        }
    }
    return nullptr;
}

Elf64_Shdr const *PackLinuxElf64::elf_find_section_name(
    char const *const name
) const
{
    Elf64_Shdr const *shdr = shdri;
    if (!shdr) {
        return nullptr;
    }
    int j = e_shnum;
    for (; 0 <=--j; ++shdr) {
        unsigned const sh_name = get_te32(&shdr->sh_name);
        if ((u32_t)file_size <= sh_name) {  // FIXME: weak
            char msg[50]; snprintf(msg, sizeof(msg),
                "bad Elf64_Shdr[%d].sh_name %#x",
                -1+ e_shnum -j, sh_name);
            throwCantPack(msg);
        }
        if (0==strcmp(name, &shstrtab[sh_name])) {
            return shdr;
        }
    }
    return nullptr;
}

Elf32_Shdr const *PackLinuxElf32::elf_find_section_type(
    unsigned const type
) const
{
    Elf32_Shdr const *shdr = shdri;
    if (!shdr) {
        return nullptr;
    }
    int j = e_shnum;
    for (; 0 <=--j; ++shdr) {
        if (type==get_te32(&shdr->sh_type)) {
            return shdr;
        }
    }
    return nullptr;
}

Elf64_Shdr const *PackLinuxElf64::elf_find_section_type(
    unsigned const type
) const
{
    Elf64_Shdr const *shdr = shdri;
    if (!shdr) {
        return nullptr;
    }
    int j = e_shnum;
    for (; 0 <=--j; ++shdr) {
        if (type==get_te32(&shdr->sh_type)) {
            return shdr;
        }
    }
    return nullptr;
}

char const *PackLinuxElf64::get_str_name(unsigned st_name, unsigned symnum) const
{
    if (strtab_end <= st_name) {
        char msg[70]; snprintf(msg, sizeof(msg),
            "bad .st_name %#x in DT_SYMTAB[%d]", st_name, symnum);
        throwCantPack(msg);
    }
    return &dynstr[st_name];
}

char const *PackLinuxElf64::get_dynsym_name(unsigned symnum, unsigned relnum) const
{
    if (symnum_end <= symnum) {
        char msg[70]; snprintf(msg, sizeof(msg),
            "bad symnum %#x in Elf64_Rel[%d]", symnum, relnum);
        throwCantPack(msg);
    }
    return get_str_name(get_te32(&dynsym[symnum].st_name), symnum);
}

bool PackLinuxElf64::calls_crt1(Elf64_Rela const *rela, int sz)
{
    if (!dynsym || !dynstr || !rela) {
        return false;
    }
    for (unsigned relnum= 0; 0 < sz; (sz -= sizeof(Elf64_Rela)), ++rela, ++relnum) {
        unsigned const symnum = get_te64(&rela->r_info) >> 32;
        char const *const symnam = get_dynsym_name(symnum, relnum);
        if (0==strcmp(symnam, "__libc_start_main")  // glibc
        ||  0==strcmp(symnam, "__libc_init")  // Android
        ||  0==strcmp(symnam, "__uClibc_main")
        ||  0==strcmp(symnam, "__uClibc_start_main"))
            return true;
    }
    return false;
}

char const *PackLinuxElf32::get_str_name(unsigned st_name, unsigned symnum) const
{
    if (strtab_end <= st_name) {
        char msg[70]; snprintf(msg, sizeof(msg),
            "bad .st_name %#x in DT_SYMTAB[%d]\n", st_name, symnum);
        throwCantPack(msg);
    }
    return &dynstr[st_name];
}

char const *PackLinuxElf32::get_dynsym_name(unsigned symnum, unsigned relnum) const
{
    if (symnum_end <= symnum) {
        char msg[70]; snprintf(msg, sizeof(msg),
            "bad symnum %#x in Elf32_Rel[%d]\n", symnum, relnum);
        throwCantPack(msg);
    }
    return get_str_name(get_te32(&dynsym[symnum].st_name), symnum);
}

bool PackLinuxElf32::calls_crt1(Elf32_Rel const *rel, int sz)
{
    if (!dynsym || !dynstr || !rel) {
        return false;
    }
    for (unsigned relnum= 0; 0 < sz; (sz -= sizeof(Elf32_Rel)), ++rel, ++relnum) {
        unsigned const symnum = get_te32(&rel->r_info) >> 8;
        char const *const symnam = get_dynsym_name(symnum, relnum);
        if (0==strcmp(symnam, "__libc_start_main")  // glibc
        ||  0==strcmp(symnam, "__libc_init")  // Android
        ||  0==strcmp(symnam, "__uClibc_main")
        ||  0==strcmp(symnam, "__uClibc_start_main"))
            return true;
    }
    return false;
}

#define WANT_REL_ENUM
#include "p_elf_enum.h"
#undef WANT_REL_ENUM

int PackLinuxElf32::canUnpack()  // really 'bool'
{
    if (checkEhdr(&ehdri)) {
        return false;
    }
    if (Elf32_Ehdr::ET_DYN==get_te16(&ehdri.e_type)) {
        PackLinuxElf32help1(fi);
    }
    if (super::canUnpack()) {
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
            throwCantPack("too many ElfXX_Phdr; try '--force-execve'");
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

        sec_strndx = nullptr;
        shstrtab = nullptr;
        if (e_shnum) {
            unsigned const e_shstrndx = get_te16(&ehdr->e_shstrndx);
            if (e_shstrndx) {
                if (e_shnum <= e_shstrndx) {
                    char msg[40]; snprintf(msg, sizeof(msg),
                        "bad e_shstrndx %#x >= e_shnum %d", e_shstrndx, e_shnum);
                    throwCantPack(msg);
                }
                sec_strndx = &shdri[e_shstrndx];
                unsigned const sh_offset = get_te32(&sec_strndx->sh_offset);
                if ((u32_t)file_size <= sh_offset) {
                    char msg[50]; snprintf(msg, sizeof(msg),
                        "bad .e_shstrndx->sh_offset %#x", sh_offset);
                    throwCantPack(msg);
                }
                shstrtab = (char const *)(sh_offset + file_image);
            }
            sec_dynsym = elf_find_section_type(Elf32_Shdr::SHT_DYNSYM);
            if (sec_dynsym) {
                unsigned const sh_link = get_te32(&sec_dynsym->sh_link);
                if (e_shnum <= sh_link) {
                    char msg[50]; snprintf(msg, sizeof(msg),
                        "bad SHT_DYNSYM.sh_link %#x", sh_link);
                }
                sec_dynstr = &shdri[sh_link];
            }

            if (sec_strndx) {
                unsigned const sh_name = get_te32(&sec_strndx->sh_name);
                if (Elf32_Shdr::SHT_STRTAB != get_te32(&sec_strndx->sh_type)
                || (u32_t)file_size <= (sizeof(".shstrtab")
                    + sh_name + (shstrtab - (const char *)&file_image[0]))
                || (sh_name
                  && 0!=strcmp((char const *)".shstrtab", &shstrtab[sh_name]))
                ) {
                    throwCantPack("bad e_shstrtab");
                }
            }
        }

        Elf32_Phdr const *pload_x0(nullptr);  // first eXecutable PT_LOAD
        phdr= phdri;
        for (int j= e_phnum; --j>=0; ++phdr)
        if (Elf32_Phdr::PT_DYNAMIC==get_te32(&phdr->p_type)) {
            unsigned offset = check_pt_dynamic(phdr);
            dynseg= (Elf32_Dyn const *)(offset + file_image);
            invert_pt_dynamic(dynseg,
                umin(get_te32(&phdr->p_filesz), file_size - offset));
        }
        else if (is_LOAD32(phdr)) {
            if (!pload_x0
            &&  Elf32_Phdr::PF_X & get_te32(&phdr->p_flags)
            ) {
                pload_x0 = phdr;
            }
            check_pt_load(phdr);
        }
        if (!pload_x0) {
            throwCantPack("No PT_LOAD has (p_flags & PF_X)");
        }
        // elf_find_dynamic() returns 0 if 0==dynseg.
        dynstr=          (char const *)elf_find_dynamic(Elf32_Dyn::DT_STRTAB);
        dynsym=     (Elf32_Sym const *)elf_find_dynamic(Elf32_Dyn::DT_SYMTAB);

        if (opt->o_unix.force_pie
        ||      Elf32_Dyn::DF_1_PIE & elf_unsigned_dynamic(Elf32_Dyn::DT_FLAGS_1)
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

        if (/*jni_onload_sym ||*/ elf_find_dynamic(upx_dt_init)) {
            if (this->e_machine!=Elf32_Ehdr::EM_386
            &&  this->e_machine!=Elf32_Ehdr::EM_MIPS
            &&  this->e_machine!=Elf32_Ehdr::EM_ARM)
                goto abandon;  // need stub: EM_PPC
            if (elf_has_dynamic(Elf32_Dyn::DT_TEXTREL)) {
                throwCantPack("DT_TEXTREL found; re-compile with -fPIC");
                goto abandon;
            }
            if (!(Elf32_Dyn::DF_1_PIE & elf_unsigned_dynamic(Elf32_Dyn::DT_FLAGS_1))) {
                // not explicitly PIE main program
                if (Elf32_Ehdr::EM_ARM == e_machine  // Android is common
                &&  !opt->o_unix.android_shlib  // but not explicit
                ) {
                    opt->info_mode++;
                    info("note: use --android-shlib if appropriate");
                    opt->info_mode--;
                }
            }
            Elf32_Shdr const *shdr = shdri;
            xct_va = ~0u;
            if (e_shnum) {
                for (int j= e_shnum; --j>=0; ++shdr) {
                    unsigned const sh_type = get_te32(&shdr->sh_type);
                    if (Elf32_Shdr::SHF_EXECINSTR & get_te32(&shdr->sh_flags)) {
                        xct_va = umin(xct_va, get_te32(&shdr->sh_addr));
                    }
                    // Hook the first slot of DT_PREINIT_ARRAY or DT_INIT_ARRAY.
                    if ((     Elf32_Dyn::DT_PREINIT_ARRAY==upx_dt_init
                        &&  Elf32_Shdr::SHT_PREINIT_ARRAY==sh_type)
                    ||  (     Elf32_Dyn::DT_INIT_ARRAY   ==upx_dt_init
                        &&  Elf32_Shdr::SHT_INIT_ARRAY   ==sh_type) ) {
                        unsigned user_init_ava = get_te32(&shdr->sh_addr);
                        user_init_off = get_te32(&shdr->sh_offset);
                        if ((u32_t)file_size <= user_init_off) {
                            char msg[70]; snprintf(msg, sizeof(msg),
                                "bad Elf32_Shdr[%d].sh_offset %#x",
                                -1+ e_shnum - j, user_init_off);
                            throwCantPack(msg);
                        }
                        // Check that &file_image[user_init_off] has
                        // *_RELATIVE relocation, and fetch user_init_va.
                        // If Elf32_Rela then the actual value is in Rela.r_addend.
                        int z_rel = dt_table[Elf32_Dyn::DT_REL];
                        int z_rsz = dt_table[Elf32_Dyn::DT_RELSZ];
                        if (z_rel && z_rsz) {
                            unsigned rel_off = get_te32(&dynseg[-1+ z_rel].d_val);
                            if ((unsigned)file_size <= rel_off) {
                                char msg[70]; snprintf(msg, sizeof(msg),
                                     "bad Elf32_Dynamic[DT_REL] %#x\n",
                                     rel_off);
                                throwCantPack(msg);
                            }
                            Elf32_Rel *rp = (Elf32_Rel *)&file_image[rel_off];
                            unsigned relsz   = get_te32(&dynseg[-1+ z_rsz].d_val);
                            if ((unsigned)file_size <= relsz) {
                                char msg[70]; snprintf(msg, sizeof(msg),
                                     "bad Elf32_Dynamic[DT_RELSZ] %#x\n",
                                     relsz);
                                throwCantPack(msg);
                            }
                            Elf32_Rel *last = (Elf32_Rel *)(relsz + (char *)rp);
                            for (; rp < last; ++rp) {
                                unsigned r_va = get_te32(&rp->r_offset);
                                if (r_va == user_init_ava) { // found the Elf32_Rel
                                    unsigned r_info = get_te32(&rp->r_info);
                                    unsigned r_type = ELF32_R_TYPE(r_info);
                                    if ((Elf32_Ehdr::EM_ARM == e_machine && R_ARM_RELATIVE == r_type)
                                    ||  (Elf32_Ehdr::EM_386 == e_machine && R_386_RELATIVE == r_type) ) {
                                        user_init_va = get_te32(&file_image[user_init_off]);
                                    }
                                    else {
                                        char msg[50]; snprintf(msg, sizeof(msg),
                                            "bad relocation %#x DT_INIT_ARRAY[0]",
                                            r_info);
                                        throwCantPack(msg);
                                    }
                                    break;
                                }
                            }
                        }
                        unsigned const p_filesz = get_te32(&pload_x0->p_filesz);
                        if (!((user_init_va - xct_va) < p_filesz)) {
                            // Not in executable portion of first executable PT_LOAD.
                            if (0==user_init_va && opt->o_unix.android_shlib) {
                                // Android allows (0 ==> skip) ?
                                upx_dt_init = 0;  // force steal of 'extra' DT_NULL
                                // XXX: FIXME: depends on SHT_DYNAMIC coming later
                            }
                            else {
                                char msg[70]; snprintf(msg, sizeof(msg),
                                    "bad init address %#x in Elf32_Shdr[%d].%#x\n",
                                    (unsigned)user_init_va, -1+ e_shnum - j, user_init_off);
                                throwCantPack(msg);
                            }
                        }
                    }
                    // By default /usr/bin/ld leaves 4 extra DT_NULL to support pre-linking.
                    // Take one as a last resort.
                    if ((Elf32_Dyn::DT_INIT==upx_dt_init || !upx_dt_init)
                    &&  Elf32_Shdr::SHT_DYNAMIC == sh_type) {
                        unsigned const n = get_te32(&shdr->sh_size) / sizeof(Elf32_Dyn);
                        Elf32_Dyn *dynp = (Elf32_Dyn *)&file_image[get_te32(&shdr->sh_offset)];
                        for (; Elf32_Dyn::DT_NULL != dynp->d_tag; ++dynp) {
                            if (upx_dt_init == get_te32(&dynp->d_tag)) {
                                break;  // re-found DT_INIT
                            }
                        }
                        if ((1+ dynp) < (n+ dynseg)) { // not the terminator, so take it
                            user_init_va = get_te32(&dynp->d_val);  // 0 if (0==upx_dt_init)
                            set_te32(&dynp->d_tag, upx_dt_init = Elf32_Dyn::DT_INIT);
                            user_init_off = (char const *)&dynp->d_val - (char const *)&file_image[0];
                        }
                    }
                }
            }
            else { // no Sections; use heuristics
                unsigned const strsz  = elf_unsigned_dynamic(Elf32_Dyn::DT_STRSZ);
                unsigned const strtab = elf_unsigned_dynamic(Elf32_Dyn::DT_STRTAB);
                unsigned const relsz  = elf_unsigned_dynamic(Elf32_Dyn::DT_RELSZ);
                unsigned const rel    = elf_unsigned_dynamic(Elf32_Dyn::DT_REL);
                unsigned const init   = elf_unsigned_dynamic(upx_dt_init);
                if ((init == (relsz + rel   ) && rel    == (strsz + strtab))
                ||  (init == (strsz + strtab) && strtab == (relsz + rel   ))
                ) {
                    xct_va = init;
                    user_init_va = init;
                    user_init_off = elf_get_offset_from_address(init);
                }
            }
            // Rely on 0==elf_unsigned_dynamic(tag) if no such tag.
            unsigned const va_gash = elf_unsigned_dynamic(Elf32_Dyn::DT_GNU_HASH);
            unsigned const va_hash = elf_unsigned_dynamic(Elf32_Dyn::DT_HASH);
            unsigned y = 0;
            if ((y=1, xct_va < va_gash)  ||  (y=2, (0==va_gash && xct_va < va_hash))
            ||  (y=3, xct_va < elf_unsigned_dynamic(Elf32_Dyn::DT_STRTAB))
            ||  (y=4, xct_va < elf_unsigned_dynamic(Elf32_Dyn::DT_SYMTAB))
            ||  (y=5, xct_va < elf_unsigned_dynamic(Elf32_Dyn::DT_REL))
            ||  (y=6, xct_va < elf_unsigned_dynamic(Elf32_Dyn::DT_RELA))
            ||  (y=7, xct_va < elf_unsigned_dynamic(Elf32_Dyn::DT_JMPREL))
            ||  (y=8, xct_va < elf_unsigned_dynamic(Elf32_Dyn::DT_VERDEF))
            ||  (y=9, xct_va < elf_unsigned_dynamic(Elf32_Dyn::DT_VERSYM))
            ||  (y=10, xct_va < elf_unsigned_dynamic(Elf32_Dyn::DT_VERNEED)) ) {
                static char const *which[] = {
                    "unknown",
                    "DT_GNU_HASH",
                    "DT_HASH",
                    "DT_STRTAB",
                    "DT_SYMTAB",
                    "DT_REL",
                    "DT_RELA",
                    "DT_JMPREL",
                    "DT_VERDEF",
                    "DT_VERSYM",
                    "DT_VERNEED",
                };
                char buf[30]; snprintf(buf, sizeof(buf), "%s above stub", which[y]);
                throwCantPack(buf);
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
                fprintf(stderr, "shlib canPack: xct_va=%#lx  xct_off=%#lx\n",
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

int PackLinuxElf64::canUnpack()  // really 'bool'
{
    if (checkEhdr(&ehdri)) {
        return false;
    }
    if (Elf64_Ehdr::ET_DYN==get_te16(&ehdri.e_type)) {
        PackLinuxElf64help1(fi);
        Elf64_Phdr const *phdr = phdri, *last_LOAD = nullptr;
        for (unsigned j = 0; j < e_phnum; ++phdr, ++j)
            if (Elf64_Phdr::PT_LOAD==get_te32(&phdr->p_type)) {
                last_LOAD = phdr;
            }
        if (!last_LOAD)
            return false;
        off_t offset = get_te64(&last_LOAD->p_offset);
        unsigned filesz = get_te64(&last_LOAD->p_filesz);
        fi->seek(filesz+offset, SEEK_SET);
        MemBuffer buf(32 + sizeof(overlay_offset));
        fi->readx(buf, buf.getSize());
        bool x = PackUnix::find_overlay_offset(buf);
        if (x) {
            return x;
        }
    }
    if (super::canUnpack()) {
        return true;
    }
    return false;
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

    upx_uint64_t max_LOADsz = 0, max_offset = 0;
    Elf64_Phdr const *phdr = phdri;
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        if (j >= 14) {
            throwCantPack("too many ElfXX_Phdr; try '--force-execve'");
            return false;
        }
        unsigned const p_type = get_te32(&phdr->p_type);
        if (PT_LOAD64 == p_type) {
            // The first PT_LOAD64 must cover the beginning of the file (0==p_offset).
            if (1!= exetype) {
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
                // FIXME: bad for shlib!
                hatch_off = ~3ul & (3+ get_te64(&phdr->p_memsz));
            }
            max_LOADsz = UPX_MAX(max_LOADsz, get_te64(&phdr->p_filesz));
            max_offset = UPX_MAX(max_offset, get_te64(&phdr->p_filesz) + get_te64(&phdr->p_offset));
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

        sec_strndx = nullptr;
        shstrtab = nullptr;
        if (e_shnum) {
            unsigned const e_shstrndx = get_te16(&ehdr->e_shstrndx);
            if (e_shstrndx) {
                if (e_shnum <= e_shstrndx) {
                    char msg[40]; snprintf(msg, sizeof(msg),
                        "bad e_shstrndx %#x >= e_shnum %d", e_shstrndx, e_shnum);
                    throwCantPack(msg);
                }
                sec_strndx = &shdri[e_shstrndx];
                upx_uint64_t sh_offset = get_te64(&sec_strndx->sh_offset);
                if ((u64_t)file_size <= sh_offset) {
                    char msg[50]; snprintf(msg, sizeof(msg),
                        "bad .e_shstrndx->sh_offset %#lx", (long unsigned)sh_offset);
                    throwCantPack(msg);
                }
                shstrtab = (char const *)(sh_offset + file_image);
            }
            sec_dynsym = elf_find_section_type(Elf64_Shdr::SHT_DYNSYM);
            if (sec_dynsym) {
                upx_uint64_t const sh_link = get_te64(&sec_dynsym->sh_link);
                if (e_shnum <= sh_link) {
                    char msg[50]; snprintf(msg, sizeof(msg),
                        "bad SHT_DYNSYM.sh_link %#lx", (long unsigned)sh_link);
                }
                sec_dynstr = &shdri[sh_link];
            }

            if (sec_strndx) {
                unsigned const sh_name = get_te32(&sec_strndx->sh_name);
                if (Elf64_Shdr::SHT_STRTAB != get_te32(&sec_strndx->sh_type)
                || (u32_t)file_size <= (sizeof(".shstrtab")
                    + sh_name + (shstrtab - (const char *)&file_image[0]))
                || (sh_name
                  && 0!=strcmp((char const *)".shstrtab", &shstrtab[sh_name]))
                ) {
                    throwCantPack("bad e_shstrtab");
                }
            }
        }

        Elf64_Phdr const *pload_x0(nullptr);  // first eXecutable PT_LOAD
        phdr= phdri;
        for (int j= e_phnum; --j>=0; ++phdr)
        if (Elf64_Phdr::PT_DYNAMIC==get_te32(&phdr->p_type)) {
            upx_uint64_t offset = check_pt_dynamic(phdr);
            dynseg= (Elf64_Dyn const *)(offset + file_image);
            invert_pt_dynamic(dynseg,
                umin(get_te64(&phdr->p_filesz), file_size - offset));
        }
        else if (PT_LOAD64==get_te32(&phdr->p_type)) {
            if (!pload_x0
            &&  Elf32_Phdr::PF_X & get_te32(&phdr->p_flags)
            ) {
                pload_x0 = phdr;
            }
            check_pt_load(phdr);
        }
        if (!pload_x0) {
            throwCantPack("No PT_LOAD has (p_flags & PF_X)");
        }
        // elf_find_dynamic() returns 0 if 0==dynseg.
        dynstr=          (char const *)elf_find_dynamic(Elf64_Dyn::DT_STRTAB);
        dynsym=     (Elf64_Sym const *)elf_find_dynamic(Elf64_Dyn::DT_SYMTAB);

        if (opt->o_unix.force_pie
        ||       Elf64_Dyn::DF_1_PIE & elf_unsigned_dynamic(Elf64_Dyn::DT_FLAGS_1)
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

        if (elf_find_dynamic(upx_dt_init)) {
            if (elf_has_dynamic(Elf64_Dyn::DT_TEXTREL)) {
                throwCantPack("DT_TEXTREL found; re-compile with -fPIC");
                goto abandon;
            }
            if (!(Elf64_Dyn::DF_1_PIE & elf_unsigned_dynamic(Elf64_Dyn::DT_FLAGS_1))) {
                // not explicitly PIE main program
                if (Elf64_Ehdr::EM_AARCH64 == e_machine  // Android is common
                &&  !opt->o_unix.android_shlib  // but not explicit
                ) {
                    opt->info_mode++;
                    info("note: use --android-shlib if appropriate");
                    opt->info_mode--;
                }
            }
            Elf64_Shdr const *shdr = shdri;
            xct_va = ~0ull;
            if (e_shnum) {
                for (int j= e_shnum; --j>=0; ++shdr) {
                    unsigned const sh_type = get_te32(&shdr->sh_type);
                    if (Elf64_Shdr::SHF_EXECINSTR & get_te64(&shdr->sh_flags)) {
                        xct_va = umin(xct_va, get_te64(&shdr->sh_addr));
                    }
                    // Hook the first slot of DT_PREINIT_ARRAY or DT_INIT_ARRAY.
                    if ((     Elf64_Dyn::DT_PREINIT_ARRAY==upx_dt_init
                        &&  Elf64_Shdr::SHT_PREINIT_ARRAY==sh_type)
                    ||  (     Elf64_Dyn::DT_INIT_ARRAY   ==upx_dt_init
                        &&  Elf64_Shdr::SHT_INIT_ARRAY   ==sh_type) ) {
                        unsigned user_init_ava = get_te32(&shdr->sh_addr);
                        user_init_off = get_te64(&shdr->sh_offset);
                        if ((u64_t)file_size <= user_init_off) {
                            char msg[70]; snprintf(msg, sizeof(msg),
                                "bad Elf64_Shdr[%d].sh_offset %#x",
                                -1+ e_shnum - j, user_init_off);
                            throwCantPack(msg);
                        }
                        // Check that &file_image[user_init_off] has
                        // *_RELATIVE relocation, and fetch user_init_va.
                        // If Elf64_Rela then the actual value is in Rela.r_addend.
                        int z_rel = dt_table[Elf64_Dyn::DT_RELA];
                        int z_rsz = dt_table[Elf64_Dyn::DT_RELASZ];
                        if (z_rel && z_rsz) {
                            upx_uint64_t rel_off = get_te64(&dynseg[-1+ z_rel].d_val);
                            if ((u64_t)file_size <= rel_off) {
                                char msg[70]; snprintf(msg, sizeof(msg),
                                     "bad Elf64_Dynamic[DT_RELA] %#llx\n",
                                     rel_off);
                                throwCantPack(msg);
                            }
                            Elf64_Rela *rp = (Elf64_Rela *)&file_image[rel_off];
                            upx_uint64_t relsz   = get_te64(&dynseg[-1+ z_rsz].d_val);
                            if ((u64_t)file_size <= relsz) {
                                char msg[70]; snprintf(msg, sizeof(msg),
                                     "bad Elf64_Dynamic[DT_RELASZ] %#llx\n",
                                     relsz);
                                throwCantPack(msg);
                            }
                            Elf64_Rela *last = (Elf64_Rela *)(relsz + (char *)rp);
                            for (; rp < last; ++rp) {
                                upx_uint64_t r_va = get_te64(&rp->r_offset);
                                if (r_va == user_init_ava) { // found the Elf64_Rela
                                    upx_uint64_t r_info = get_te64(&rp->r_info);
                                    unsigned r_type = ELF64_R_TYPE(r_info);
                                    if (Elf64_Ehdr::EM_AARCH64 == e_machine
                                    &&  R_AARCH64_RELATIVE == r_type) {
                                        user_init_va = get_te64(&rp->r_addend);
                                    }
                                    else if (Elf64_Ehdr::EM_AARCH64 == e_machine
                                    &&  R_AARCH64_ABS64 == r_type) {
                                        user_init_va = get_te64(&file_image[user_init_off]);
                                    }
                                    else {
                                        char msg[50]; snprintf(msg, sizeof(msg),
                                            "bad relocation %#llx DT_INIT_ARRAY[0]",
                                            r_info);
                                        throwCantPack(msg);
                                    }
                                    break;
                                }
                            }
                        }
                        unsigned const p_filesz = get_te64(&pload_x0->p_filesz);
                        if (!((user_init_va - xct_va) < p_filesz)) {
                            // Not in executable portion of first executable PT_LOAD.
                            if (0==user_init_va && opt->o_unix.android_shlib) {
                                // Android allows (0 ==> skip) ?
                                upx_dt_init = 0;  // force steal of 'extra' DT_NULL
                                // XXX: FIXME: depends on SHT_DYNAMIC coming later
                            }
                            else {
                                char msg[70]; snprintf(msg, sizeof(msg),
                                    "bad init address %#x in Elf64_Shdr[%d].%#x\n",
                                    (unsigned)user_init_va, -1+ e_shnum - j, user_init_off);
                                throwCantPack(msg);
                            }
                        }
                    }
                    // By default /usr/bin/ld leaves 4 extra DT_NULL to support pre-linking.
                    // Take one as a last resort.
                    if ((Elf64_Dyn::DT_INIT==upx_dt_init || !upx_dt_init)
                    &&  Elf64_Shdr::SHT_DYNAMIC == sh_type) {
                        upx_uint64_t sh_offset = get_te64(&shdr->sh_offset);
                        upx_uint64_t sh_size = get_te64(&shdr->sh_size);
                        if ((upx_uint64_t)file_size < sh_size
                        ||  (upx_uint64_t)file_size < sh_offset
                        || ((upx_uint64_t)file_size - sh_offset) < sh_size) {
                            throwCantPack("bad SHT_DYNAMIC");
                        }
                        unsigned const n = sh_size / sizeof(Elf64_Dyn);
                        Elf64_Dyn *dynp = (Elf64_Dyn *)&file_image[sh_offset];
                        for (; Elf64_Dyn::DT_NULL != dynp->d_tag; ++dynp) {
                            if (upx_dt_init == get_te64(&dynp->d_tag)) {
                                break;  // re-found DT_INIT
                            }
                        }
                        if ((1+ dynp) < (n+ dynseg)) { // not the terminator, so take it
                            user_init_va = get_te64(&dynp->d_val);  // 0 if (0==upx_dt_init)
                            set_te64(&dynp->d_tag, upx_dt_init = Elf64_Dyn::DT_INIT);
                            user_init_off = (char const *)&dynp->d_val - (char const *)&file_image[0];
                        }
                    }
                }
            }
            else { // no Sections; use heuristics
                upx_uint64_t const strsz  = elf_unsigned_dynamic(Elf64_Dyn::DT_STRSZ);
                upx_uint64_t const strtab = elf_unsigned_dynamic(Elf64_Dyn::DT_STRTAB);
                upx_uint64_t const relsz  = elf_unsigned_dynamic(Elf64_Dyn::DT_RELSZ);
                upx_uint64_t const rel    = elf_unsigned_dynamic(Elf64_Dyn::DT_REL);
                upx_uint64_t const init   = elf_unsigned_dynamic(upx_dt_init);
                if ((init == (relsz + rel   ) && rel    == (strsz + strtab))
                ||  (init == (strsz + strtab) && strtab == (relsz + rel   ))
                ) {
                    xct_va = init;
                    user_init_va = init;
                    user_init_off = elf_get_offset_from_address(init);
                }
            }
            // Rely on 0==elf_unsigned_dynamic(tag) if no such tag.
            upx_uint64_t const va_gash = elf_unsigned_dynamic(Elf64_Dyn::DT_GNU_HASH);
            upx_uint64_t const va_hash = elf_unsigned_dynamic(Elf64_Dyn::DT_HASH);
            unsigned y = 0;
            if ((y=1, xct_va < va_gash)  ||  (y=2, (0==va_gash && xct_va < va_hash))
            ||  (y=3, xct_va < elf_unsigned_dynamic(Elf64_Dyn::DT_STRTAB))
            ||  (y=4, xct_va < elf_unsigned_dynamic(Elf64_Dyn::DT_SYMTAB))
            ||  (y=5, xct_va < elf_unsigned_dynamic(Elf64_Dyn::DT_REL))
            ||  (y=6, xct_va < elf_unsigned_dynamic(Elf64_Dyn::DT_RELA))
            ||  (y=7, xct_va < elf_unsigned_dynamic(Elf64_Dyn::DT_JMPREL))
            ||  (y=8, xct_va < elf_unsigned_dynamic(Elf64_Dyn::DT_VERDEF))
            ||  (y=9, xct_va < elf_unsigned_dynamic(Elf64_Dyn::DT_VERSYM))
            ||  (y=10, xct_va < elf_unsigned_dynamic(Elf64_Dyn::DT_VERNEED)) ) {
                static char const *which[] = {
                    "unknown",
                    "DT_GNU_HASH",
                    "DT_HASH",
                    "DT_STRTAB",
                    "DT_SYMTAB",
                    "DT_REL",
                    "DT_RELA",
                    "DT_JMPREL",
                    "DT_VERDEF",
                    "DT_VERSYM",
                    "DT_VERNEED",
                };
                char buf[30]; snprintf(buf, sizeof(buf), "%s above stub", which[y]);
                throwCantPack(buf);
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
                fprintf(stderr, "shlib canPack: xct_va=%#lx  xct_off=%#lx\n",
                    (long)xct_va, (long)xct_off);
            }
            goto proceed;  // But proper packing depends on checking xct_va.
        }
        else {
            throwCantPack("need DT_INIT; try \"void _init(void){}\"");
        }
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
    // this->blocksize: avoid over-allocating.
    // (file_size - max_offset): debug info, non-globl symbols, etc.
    opt->o_unix.blocksize = blocksize = UPX_MAX(max_LOADsz, file_size - max_offset);
    return true;
}

off_t
PackLinuxElf32::getbrk(Elf32_Phdr const *phdr, int nph) const
{
    off_t brka = 0;
    for (int j = 0; j < nph; ++phdr, ++j) {
        if (is_LOAD32(phdr)) {
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
        if (is_LOAD32(phdr)) {
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
    h3->ehdr =         ((cprElfHdr3 const *)proto)->ehdr;
    h3->phdr[C_BASE] = ((cprElfHdr3 const *)proto)->phdr[1];  // .data; .p_align
    h3->phdr[C_TEXT] = ((cprElfHdr3 const *)proto)->phdr[0];  // .text
    memset(&h3->linfo, 0, sizeof(h3->linfo));

    h3->ehdr.e_type = ehdri.e_type;  // ET_EXEC vs ET_DYN (gcc -pie -fPIC)
    h3->ehdr.e_ident[Elf32_Ehdr::EI_OSABI] = ei_osabi;
    if (Elf32_Ehdr::EM_MIPS==e_machine) { // MIPS R3000  FIXME
        h3->ehdr.e_ident[Elf32_Ehdr::EI_OSABI] = Elf32_Ehdr::ELFOSABI_NONE;
        h3->ehdr.e_flags = ehdri.e_flags;
    }

    unsigned const phnum_i = get_te16(&h2->ehdr.e_phnum);
    unsigned       phnum_o = phnum_i;

    assert(get_te32(&h2->ehdr.e_phoff)     == sizeof(Elf32_Ehdr));
                         h2->ehdr.e_shoff = 0;
    assert(get_te16(&h2->ehdr.e_ehsize)    == sizeof(Elf32_Ehdr));
    assert(get_te16(&h2->ehdr.e_phentsize) == sizeof(Elf32_Phdr));
    if (o_elf_shnum) {
        set_te16(&h2->ehdr.e_shentsize, sizeof(Elf32_Shdr));
        h2->ehdr.e_shnum = o_elf_shnum;
        h2->ehdr.e_shstrndx = o_elf_shnum - 1;
    }
    else {
        // https://bugzilla.redhat.com/show_bug.cgi?id=2131609
        // 0==.e_shnum is a special case for libbfd
        // that requires 0==.e_shentsize in order to force "no Shdrs"
        h2->ehdr.e_shentsize = 0;
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
    set_te32(&h2->phdr[C_TEXT].p_filesz, sizeof(*h2));  // + identsize;
              h2->phdr[C_TEXT].p_memsz = h2->phdr[C_TEXT].p_filesz;

    for (unsigned j=0; j < phnum_i; ++j) {
        if (is_LOAD32(&h3->phdr[j])) {
            set_te32(&h3->phdr[j].p_align, page_size);
        }
    }

    // Info for OS kernel to set the brk()
    if (brka) {
        // linux-2.6.14 binfmt_elf.c: SIGKILL if (0==.p_memsz) on a page boundary
        upx_uint32_t lo_va_user = ~0u;  // infinity
        for (int j= e_phnum; --j>=0; ) {
            if (is_LOAD32(&phdri[j])) {
                upx_uint32_t const vaddr = get_te32(&phdri[j].p_vaddr);
                lo_va_user = umin(lo_va_user, vaddr);
            }
        }
        set_te32(                 &h2->phdr[C_BASE].p_vaddr, lo_va_user);
        h2->phdr[C_BASE].p_paddr = h2->phdr[C_BASE].p_vaddr;
        h2->phdr[C_TEXT].p_vaddr = h2->phdr[C_BASE].p_vaddr;
        h2->phdr[C_TEXT].p_paddr = h2->phdr[C_BASE].p_vaddr;
        set_te32(&h2->phdr[C_BASE].p_type, PT_LOAD32);  // be sure
        h2->phdr[C_BASE].p_offset = 0;
        h2->phdr[C_BASE].p_filesz = 0;
        // .p_memsz = brka;  temporary until sz_pack2
        set_te32(&h2->phdr[C_BASE].p_memsz, brka - lo_va_user);
        set_te32(&h2->phdr[C_BASE].p_flags, Elf32_Phdr::PF_R | Elf32_Phdr::PF_W);
    }
    if (ph.format==getFormat()) {
        assert((2u+ !!gnu_stack) == phnum_o);
        set_te32(&h2->phdr[C_TEXT].p_flags, ~Elf32_Phdr::PF_W & get_te32(&h2->phdr[C_TEXT].p_flags));
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
    Elf32_Nhdr const *np_NetBSD = nullptr;  unsigned sz_NetBSD = 0;
    Elf32_Nhdr const *np_PaX    = nullptr;  unsigned sz_PaX    = 0;
    unsigned char *cp = (unsigned char *)note_body;
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
    Elf32_Phdr *phdr = &elfout.phdr[C_NOTE];
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
    set_te32(&h2->phdr[C_TEXT].p_filesz, note_offset);
              h2->phdr[C_TEXT].p_memsz = h2->phdr[C_TEXT].p_filesz;

    if (ph.format==getFormat()) {
        set_te16(&h2->ehdr.e_phnum, !!sz_NetBSD + !!sz_PaX +
        get_te16(&h2->ehdr.e_phnum));
        fo->seek(0, SEEK_SET);
        fo->rewrite(h2, sizeof(*h2) - sizeof(h2->linfo));

        // The 'if' guards on these two calls to memcpy are required
        // because the C Standard Committee did not debug the Standard
        // before publishing.  An empty region (0==size) must nevertheless
        // have a valid (non-nullptr) pointer.
        if (sz_NetBSD) memcpy(&((char *)phdr)[0],         np_NetBSD, sz_NetBSD);
        if (sz_PaX)    memcpy(&((char *)phdr)[sz_NetBSD], np_PaX,    sz_PaX);

        fo->write(&elfout.phdr[C_NOTE],
            &((char *)phdr)[sz_PaX + sz_NetBSD] - (char *)&elfout.phdr[C_NOTE]);

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
    h3->ehdr.e_shentsize = 0;
    h3->ehdr.e_shnum = 0;
    h3->ehdr.e_shstrndx = 0;

    struct {
        Elf32_Nhdr nhdr;
        char name[8];
        unsigned body;
    } elfnote;

    unsigned const note_offset = sizeof(*h3) - sizeof(linfo);
    sz_elf_hdrs = sizeof(elfnote) + note_offset;

    set_te32(&h3->phdr[C_NOTE].p_type, PT_NOTE32);
    set_te32(&h3->phdr[C_NOTE].p_offset, note_offset);
    set_te32(&h3->phdr[C_NOTE].p_vaddr, note_offset);
    set_te32(&h3->phdr[C_NOTE].p_paddr, note_offset);
    set_te32(&h3->phdr[C_NOTE].p_filesz, sizeof(elfnote));
    set_te32(&h3->phdr[C_NOTE].p_memsz,  sizeof(elfnote));
    set_te32(&h3->phdr[C_NOTE].p_flags, Elf32_Phdr::PF_R);
    set_te32(&h3->phdr[C_NOTE].p_align, 4);

    // Q: Same as this->note_body[0 .. this->note_size-1] ?
    set_te32(&elfnote.nhdr.namesz, 8);
    set_te32(&elfnote.nhdr.descsz, OPENBSD_DESCSZ);
    set_te32(&elfnote.nhdr.type,   NHDR_OPENBSD_TAG);
    memcpy(elfnote.name, "OpenBSD", sizeof(elfnote.name));
    elfnote.body = 0;

    set_te32(&h3->phdr[C_TEXT].p_filesz, sz_elf_hdrs);
              h3->phdr[C_TEXT].p_memsz = h3->phdr[C_TEXT].p_filesz;

    unsigned const brkb = brka | ((0==(~page_mask & brka)) ? 0x20 : 0);
    set_te32(&h3->phdr[C_BASE].p_type, PT_LOAD32);  // be sure
    set_te32(&h3->phdr[C_BASE].p_offset, ~page_mask & brkb);
    set_te32(&h3->phdr[C_BASE].p_vaddr, brkb);
    set_te32(&h3->phdr[C_BASE].p_paddr, brkb);
    h3->phdr[C_BASE].p_filesz = 0;
    // Too many kernels have bugs when 0==.p_memsz
    set_te32(&h3->phdr[C_BASE].p_memsz, 1);
    set_te32(&h3->phdr[C_BASE].p_flags, Elf32_Phdr::PF_R | Elf32_Phdr::PF_W);

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
    h3->ehdr =         ((cprElfHdr3 const *)proto)->ehdr;
    h3->phdr[C_BASE] = ((cprElfHdr3 const *)proto)->phdr[1];  // .data; .p_align
    h3->phdr[C_TEXT] = ((cprElfHdr3 const *)proto)->phdr[0];  // .text
    memset(&h3->linfo, 0, sizeof(h3->linfo));

    h3->ehdr.e_type = ehdri.e_type;  // ET_EXEC vs ET_DYN (gcc -pie -fPIC)
    h3->ehdr.e_ident[Elf64_Ehdr::EI_OSABI] = ei_osabi;
    if (Elf64_Ehdr::ELFOSABI_LINUX == ei_osabi  // proper
    &&  Elf64_Ehdr::ELFOSABI_NONE  == ehdri.e_ident[Elf64_Ehdr::EI_OSABI]  // sloppy
    ) { // propagate sloppiness so that decompression does not complain
        h3->ehdr.e_ident[Elf64_Ehdr::EI_OSABI] = ehdri.e_ident[Elf64_Ehdr::EI_OSABI];
    }
    if (Elf64_Ehdr::EM_PPC64 == get_te16(&ehdri.e_machine)) {
        h3->ehdr.e_flags = ehdri.e_flags;  // "0x1, abiv1" vs "0x2, abiv2"
    }

    unsigned const phnum_i = get_te16(&h2->ehdr.e_phnum);
    unsigned       phnum_o = phnum_i;

    assert(get_te64(&h2->ehdr.e_phoff)     == sizeof(Elf64_Ehdr));
                         h2->ehdr.e_shoff = 0;
    assert(get_te16(&h2->ehdr.e_ehsize)    == sizeof(Elf64_Ehdr));
    assert(get_te16(&h2->ehdr.e_phentsize) == sizeof(Elf64_Phdr));
    if (o_elf_shnum) {
        set_te16(&h2->ehdr.e_shentsize, sizeof(Elf64_Shdr));
        h2->ehdr.e_shnum = o_elf_shnum;
        h2->ehdr.e_shstrndx = o_elf_shnum - 1;
    }
    else {
        h2->ehdr.e_shentsize = 0;
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
    set_te64(&h2->phdr[C_TEXT].p_filesz, sizeof(*h2));  // + identsize;
                  h2->phdr[C_TEXT].p_memsz = h2->phdr[C_TEXT].p_filesz;

    for (unsigned j=0; j < phnum_i; ++j) {
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
        set_te64(                 &h2->phdr[C_BASE].p_vaddr, lo_va_user);
        h2->phdr[C_BASE].p_paddr = h2->phdr[C_BASE].p_vaddr;
        h2->phdr[C_TEXT].p_vaddr = h2->phdr[C_BASE].p_vaddr;
        h2->phdr[C_TEXT].p_paddr = h2->phdr[C_BASE].p_vaddr;
        set_te32(&h2->phdr[C_BASE].p_type, PT_LOAD64);  // be sure
        h2->phdr[C_BASE].p_offset = 0;
        h2->phdr[C_BASE].p_filesz = 0;
        // .p_memsz = brka;  temporary until sz_pack2
        set_te64(&h2->phdr[C_BASE].p_memsz, brka - lo_va_user);
        set_te32(&h2->phdr[C_BASE].p_flags, Elf64_Phdr::PF_R | Elf64_Phdr::PF_W);
    }
    if (ph.format==getFormat()) {
        assert((2u+ !!gnu_stack) == phnum_o);
        set_te32(&h2->phdr[C_TEXT].p_flags, ~Elf64_Phdr::PF_W & get_te32(&h2->phdr[C_TEXT].p_flags));
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

// Android shlib has ABS symbols that actually are relative.
static char const abs_symbol_names[][14] = {
      "__bss_end__"
    ,  "_bss_end__"
    , "__bss_start"
    , "__bss_start__"
    ,  "_edata"
    ,  "_end"
    , "__end__"
    , ""
};

int
PackLinuxElf32::adjABS(Elf32_Sym *sym, unsigned delta)
{
    unsigned st_name = get_te32(&sym->st_name);
    for (int j = 0; abs_symbol_names[j][0]; ++j) {
        if (!strcmp(abs_symbol_names[j], get_str_name(st_name, (unsigned)-1))) {
            sym->st_value += delta;
            return 1;
        }
    }
    return 0;
}

int
PackLinuxElf64::adjABS(Elf64_Sym *sym, unsigned delta)
{
    unsigned st_name = get_te32(&sym->st_name);
    for (int j = 0; abs_symbol_names[j][0]; ++j) {
        if (!strcmp(abs_symbol_names[j], get_str_name(st_name, (unsigned)-1))) {
            sym->st_value += delta;
            return 1;
        }
    }
    return 0;
}

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
        note_body.alloc(note_size);
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
                if (Elf32_Sym::SHN_ABS == symsec && xct_off <= symval) {
                    adjABS(sym, asl_delta);
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
                            throwCantPack("PT_NOTEs too big");
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
                unsigned   sh_flags = get_te32(&shdr->sh_flags);
                if (xct_off <= sh_offset
                // Omit .comment (0==.sh_addr; !SHF_ALLOC) etc.
                && (shdr->sh_addr || Elf32_Shdr::SHF_ALLOC & sh_flags)
                ) {
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
                        if (Elf32_Ehdr::EM_ARM == e_machine) switch (r_type) {
                            default: {
                                char msg[90]; snprintf(msg, sizeof(msg),
                                    "unexpected relocation %#x [%#x]",
                                    r_type, -1 + (sh_size / sh_entsize) - k);
                                throwCantPack(msg);
                            } break;
                            case R_ARM_ABS32:  // FALL THROUGH
                            case R_ARM_GLOB_DAT: // FALL THROUGH
                            case R_ARM_RELATIVE: {
                                if (xct_off <= w) {
                                    set_te32(&file_image[d], asl_delta + w);
                                }
                            } break;
                            case R_ARM_JUMP_SLOT: {
                                if (plt_off > r_offset) {
                                    plt_off = r_offset;
                                }
                                if (xct_off <= w) {
                                    set_te32(&file_image[d], asl_delta + w);
                                }
                                ++n_jmp_slot;
                            }; break;
                        }
                    }
                    fo->seek(sh_offset, SEEK_SET);
                    fo->rewrite(rel0, sh_size);
                }
                if (Elf32_Shdr::SHT_NOTE == sh_type) {
                    if (!(Elf32_Shdr::SHF_ALLOC & get_te32(&shdr->sh_flags))) {
                        // example: version number of 'gold' linker (static binder)
                        if (sizeof(buf_notes) < (sh_size + len_notes)) {
                            throwCantPack("SHT_NOTEs too big");
                        }
                        set_te32(&shdro[j].sh_offset,
                            len_notes + (e_shnum * sizeof(Elf32_Shdr)) + xct_off);
                        memcpy(&buf_notes[len_notes], &file_image[sh_offset], sh_size);
                        len_notes += sh_size;
                    }
                    else { // SHF_ALLOC, thus already in PT_LOAD
                        // Not sure why we need this conditional.
                        // Anyway, some Android have multiple SHT_NOTE sections.
                        if (xct_off <= sh_offset) {
                            upx_uint32_t pos = xct_off + e_shnum * sizeof(Elf32_Shdr);
                            set_te32(&shdr->sh_addr,   pos);
                            set_te32(&shdr->sh_offset, pos);
                        }
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
        MemBuffer mb_shdri;
        if (!shdri) {
            mb_shdri.alloc(e_shnum * sizeof(Elf32_Shdr));
            shdri = (Elf32_Shdr *)mb_shdri.getVoidPtr();
            e_shoff = get_te32(&ehdri.e_shoff);
            fi->seek(e_shoff, SEEK_SET);
            fi->readx(shdri, e_shnum * sizeof(Elf32_Shdr));
        }
        //set the shstrtab
        sec_strndx = &shdri[get_te16(&ehdri.e_shstrndx)];

        unsigned sh_size = get_te32(&sec_strndx->sh_size);
        mb_shstrtab.alloc(sh_size); shstrtab = (char *)mb_shstrtab.getVoidPtr();
        fi->seek(0,SEEK_SET);
        fi->seek(sec_strndx->sh_offset,SEEK_SET);
        fi->readx(mb_shstrtab, sh_size);

        Elf32_Shdr const *buildid = elf_find_section_name(".note.gnu.build-id");
        if (buildid) {
            unsigned bid_sh_size = get_te32(&buildid->sh_size);
            buildid_data.alloc(bid_sh_size);
            buildid_data.clear();
            fi->seek(0,SEEK_SET);
            fi->seek(buildid->sh_offset,SEEK_SET);
            fi->readx((void *)buildid_data, bid_sh_size);

            o_elf_shnum = 3;
            memset(&shdrout,0,sizeof(shdrout));

            //setup the build-id
            memcpy(&shdrout.shdr[1], buildid, sizeof(shdrout.shdr[1]));
            set_te32(&shdrout.shdr[1].sh_name, 1);

            //setup the shstrtab
            memcpy(&shdrout.shdr[2], sec_strndx, sizeof(shdrout.shdr[2]));
            set_te32(&shdrout.shdr[2].sh_name, 20);
            set_te32(&shdrout.shdr[2].sh_size, 29); //size of our static shstrtab
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

void PackLinuxElf64::asl_pack2_Shdrs(OutputFile *fo)
{
    // In order to pacify the runtime linker on Android "O" ("Oreo"),
    // we will splice-in a 4KiB page that contains an "extra" copy
    // of the Shdr, any PT_NOTE above xct_off, and shstrtab.
    // File order: Ehdr, Phdr[], section contents below xct_off,
    //    Shdr_copy[], PT_NOTEs.hi, shstrtab.
    xct_va  += asl_delta;
    //xct_off += asl_delta;  // not until ::pack3()

    // Relocate PT_DYNAMIC (in PT_LOAD with PF_W)
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
    // Updated dynseg (.dynamic, in PT_DYNAMIC (PT_LOAD{PF_W})) has not been written.
    // dynseg is in file_image[] but not in low_mem[].

    // Relocate dynsym (DT_SYMTAB) which is below xct_va
    upx_uint64_t const off_dynsym = get_te64(&sec_dynsym->sh_offset);
    upx_uint64_t const sz_dynsym  = get_te64(&sec_dynsym->sh_size);
    if ((upx_uint64_t)file_size < sz_dynsym
    ||  (upx_uint64_t)file_size < off_dynsym
    || ((upx_uint64_t)file_size - off_dynsym) < sz_dynsym) {
        throwCantPack("bad DT_SYMTAB");
    }
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
        if (Elf64_Sym::SHN_ABS == symsec && xct_off <= symval) {
            adjABS(sym, asl_delta);
        }
    }

    // Relocate Phdr virtual addresses, but not physical offsets and sizes
    unsigned char buf_notes[512]; memset(buf_notes, 0, sizeof(buf_notes));
    unsigned len_notes = 0;
    Elf64_Phdr *phdr = (Elf64_Phdr *)lowmem.subref(
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
        if ((upx_uint64_t)file_size < sh_size
        ||  (upx_uint64_t)file_size < sh_offset
        || (Elf64_Shdr::SHT_NOBITS != sh_type
           && ((upx_uint64_t)file_size - sh_offset) < sh_size) ) {
            throwCantPack("bad SHT_STRNDX");
        }

        if (xct_off <= sh_offset) {
            upx_uint64_t addr = get_te64(&shdr->sh_addr);
            set_te64(&shdr->sh_addr, asl_delta + addr);
            set_te64(&shdr->sh_offset, asl_delta + sh_offset);
        }
        switch (sh_type) {
        default: break;
        case Elf64_Shdr::SHT_RELA: {
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
                if (Elf64_Ehdr::EM_AARCH64 == e_machine) switch (r_type) {
                    default: {
                        char msg[90]; snprintf(msg, sizeof(msg),
                            "unexpected relocation %#x [%#x]",
                            r_type, -1 + (unsigned)(sh_size / sh_entsize) - k);
                        throwCantPack(msg);
                    } break;
                    case R_AARCH64_ABS64: // FALL THROUGH
                    case R_AARCH64_GLOB_DAT: // FALL THROUGH
                    case R_AARCH64_RELATIVE: {
                        if (xct_off <= r_addend) {
                            set_te64(&rela->r_addend, asl_delta + r_addend);
                        }
                    } break;
                    case R_AARCH64_JUMP_SLOT: {
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
                    } break;
                }
            }
        }; break;
        case Elf64_Shdr::SHT_REL: {
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
        }; break;
        case Elf64_Shdr::SHT_NOTE: {
            if (!(Elf64_Shdr::SHF_ALLOC & get_te64(&shdr->sh_flags))) {
                // example: version numer of 'gold' linker (static binder)
                if (sizeof(buf_notes) < (sh_size + len_notes)) {
                    throwCantPack("SHT_NOTEs too big");
                }
                set_te64(&shdro[j].sh_offset,
                    len_notes + (e_shnum * sizeof(Elf64_Shdr)) + xct_off);
                memcpy(&buf_notes[len_notes], &file_image[sh_offset], sh_size);
                len_notes += sh_size;
            }
            else { // SHF_ALLOC: in PT_LOAD; but move sh_addr and sh_offset
                // Not sure why we need this conditional.
                // Anyway, some Android have multiple SHT_NOTE sections.
                if (xct_off <= sh_offset) {
                    upx_uint64_t pos = xct_off + e_shnum * sizeof(Elf64_Shdr);
                    set_te64(&shdr->sh_addr,   pos);
                    set_te64(&shdr->sh_offset, pos);
                }
            }
        }; break;
        } // end switch (sh_type)
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
    total_out = sz_elf_hdrs;
    total_in = xct_off;
    //xct_off += asl_delta;  // wait until ::pack3

    memset(&linfo, 0, sizeof(linfo));
    fo->write(&linfo, sizeof(linfo));
}

void PackLinuxElf64::pack1(OutputFile * /*fo*/, Filter &ft)
{
    fi->seek(0, SEEK_SET);
    fi->readx(&ehdri, sizeof(ehdri));
    assert(e_phoff == sizeof(Elf64_Ehdr));  // checked by canPack()
    sz_phdrs = e_phnum * get_te16(&ehdri.e_phentsize);

// We compress separate pieces (usually each PT_LOAD, plus the gaps in the file
// that are not covered by any PT_LOAD), but currently at run time there can be
// only one decompressor method.
// Therefore we must plan ahead because Packer::compressWithFilters tries
// to find the smallest result among the available methods, for one piece only.
// In the future we may allow more than one decompression method at run time.
// For now we must choose only one, and force PackUnix::packExtent
// (==> compressWithFilters) to use it.
    int nfilters = 0;
    {
        int const *fp = getFilters();
        while (FT_END != *fp++) {
            ++nfilters;
        }
    }
    {
        int npieces = 1;  // tail after highest PT_LOAD
        Elf64_Phdr *phdr = phdri;
        for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
            if (PT_LOAD64 == get_te32(&phdr->p_type)) {
                unsigned const  flags = get_te32(&phdr->p_flags);
                unsigned       offset = get_te64(&phdr->p_offset);
                if (!xct_off  // not shlib
                  // new-style shlib: PT_LOAD[0] has symbol table
                  // which must not be compressed, but also lacks PF_X
                ||    (Elf64_Phdr::PF_X & flags)
                  // Read-only, non-first PT_LOAD is _assumed_ to be compressible
                ||  (!(Elf64_Phdr::PF_W & flags) && 0!=offset))
                {
                    ++npieces;  // will attempt compression of this PT_LOAD
                }
            }
        }
        uip->ui_total_passes += npieces;
    }
    int methods[256];
    unsigned nmethods = prepareMethods(methods, ph.method, getCompressionMethods(M_ALL, ph.level));
    if (1 < nmethods) { // Many are available, but we must choose only one
        uip->ui_total_passes += 1;  // the batch for output
        uip->ui_total_passes *= nmethods * (1+ nfilters);  // finding smallest total
        PackHeader orig_ph = ph;
        Filter orig_ft = ft;
        unsigned max_offset = 0;
        unsigned sz_best= ~0u;
        int method_best = 0;
        for (unsigned k = 0; k < nmethods; ++k) { // FIXME: parallelize; cost: working space
            unsigned sz_this = 0;
            Elf64_Phdr *phdr = phdri;
            for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
                if (PT_LOAD64 == get_te32(&phdr->p_type)) {
                    unsigned const  flags = get_te32(&phdr->p_flags);
                    unsigned       offset = get_te64(&phdr->p_offset);
                    unsigned       filesz = get_te64(&phdr->p_filesz);
                    max_offset = UPX_MAX(max_offset, filesz + offset);
                    if (!xct_off  // not shlib
                      // new-style shlib: PT_LOAD[0] has symbol table
                      // which must not be compressed, but also lacks PF_X
                    ||    (Elf64_Phdr::PF_X & flags)
                      // Read-only, non-first PT_LOAD is _assumed_ to be compressible
                    ||  (!(Elf64_Phdr::PF_W & flags) && 0!=offset))
                    {
                        if (xct_off && 0==offset) { // old-style shlib
                            offset  = xct_off;
                            filesz -= xct_off;
                        }
                        fi->seek(offset, SEEK_SET);
                        fi->readx(ibuf, filesz);
                        ft = orig_ft;
                        ph = orig_ph;
                        ph.method = force_method(methods[k]);
                        ph.u_len = filesz;
                        compressWithFilters(&ft, OVERHEAD, NULL_cconf, 10, true);
                        sz_this += ph.c_len;
                    }
                }
            }
            unsigned const sz_tail = file_size - max_offset;  // debuginfo, etc.
            if (sz_tail) {
                fi->seek(max_offset, SEEK_SET);
                fi->readx(ibuf, sz_tail);
                ft = orig_ft;
                ph = orig_ph;
                ph.method = force_method(methods[k]);
                ph.u_len = sz_tail;
                compressWithFilters(&ft, OVERHEAD, NULL_cconf, 10, true);
                sz_this += ph.c_len;
            }
            // FIXME: loader size also depends on method
            if (sz_best > sz_this) {
                sz_best = sz_this;
                method_best = methods[k];
            }
        }
        ft = orig_ft;
        ph = orig_ph;
        ph.method = force_method(method_best);
    }

    note_size = 0;
    Elf64_Phdr *phdr = phdri;
    for (unsigned j=0; j < e_phnum; ++phdr, ++j) {
        if (PT_NOTE64 == get_te32(&phdr->p_type)) {
            note_size += up4(get_te64(&phdr->p_filesz));
        }
    }
    if (note_size) {
        note_body.alloc(note_size);
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

    // only execute if option present
    if (opt->o_unix.preserve_build_id) {
        // set this so we can use elf_find_section_name
        e_shnum = get_te16(&ehdri.e_shnum);
        MemBuffer mb_shdri;
        if (!shdri) {
            mb_shdri.alloc(e_shnum * sizeof(Elf64_Shdr));
            shdri = (Elf64_Shdr *)mb_shdri.getVoidPtr();
            e_shoff = get_te64(&ehdri.e_shoff);
            fi->seek(e_shoff, SEEK_SET);
            fi->readx(shdri, e_shnum * sizeof(Elf64_Shdr));
        }
        //set the shstrtab
        sec_strndx = &shdri[get_te16(&ehdri.e_shstrndx)];

        upx_uint64_t sh_size = get_te64(&sec_strndx->sh_size);
        mb_shstrtab.alloc(sh_size); shstrtab = (char *)mb_shstrtab.getVoidPtr();
        fi->seek(0,SEEK_SET);
        fi->seek(sec_strndx->sh_offset,SEEK_SET);
        fi->readx(mb_shstrtab, sh_size);

        Elf64_Shdr const *buildid = elf_find_section_name(".note.gnu.build-id");
        if (buildid) {
            unsigned bid_sh_size = get_te32(&buildid->sh_size);
            buildid_data.alloc(bid_sh_size);
            buildid_data.clear();
            fi->seek(0,SEEK_SET);
            fi->seek(buildid->sh_offset,SEEK_SET);
            fi->readx((void *)buildid_data, bid_sh_size);

            o_elf_shnum = 3;
            memset(&shdrout,0,sizeof(shdrout));

            //setup the build-id
            memcpy(&shdrout.shdr[1], buildid, sizeof(shdrout.shdr[1]));
            set_te32(&shdrout.shdr[1].sh_name, 1);

            //setup the shstrtab
            memcpy(&shdrout.shdr[2], sec_strndx, sizeof(shdrout.shdr[2]));
            set_te32(&shdrout.shdr[2].sh_name, 20);
            set_te32(&shdrout.shdr[2].sh_size, 29); //size of our static shstrtab
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
    if (!is_LOAD32(&phdr[k])) {
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
        if (is_LOAD32(&phdr[j])) {
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
        if (is_LOAD32(&phdri[k])) {
            uip->ui_total_passes++;
            if (find_LOAD_gap(phdri, k, e_phnum)) {
                uip->ui_total_passes++;
            }
        }
    }
    uip->ui_total_passes -= !!is_shlib;  // not .data of shlib

    // compress extents
    unsigned hdr_u_len = (is_shlib ? xct_off : (sizeof(Elf32_Ehdr) + sz_phdrs));

    total_in =  (is_shlib ?           0 : xct_off);
    total_out = (is_shlib ? sz_elf_hdrs : xct_off);

    uip->ui_pass = 0;
    ft.addvalue = 0;

    unsigned nk_f = 0; unsigned xsz_f = 0;
    for (k = 0; k < e_phnum; ++k)
    if (is_LOAD32(&phdri[k])
    &&  Elf32_Phdr::PF_X & get_te32(&phdri[k].p_flags)) {
        unsigned xsz =     get_te32(&phdri[k].p_filesz);
        if (xsz_f < xsz) {
            xsz_f = xsz;
            nk_f = k;
        }
    }
    int nx = 0;
    for (k = 0; k < e_phnum; ++k)
    if (is_LOAD32(&phdri[k])) {
        if (ft.id < 0x40) {
            // FIXME: ??    ft.addvalue = phdri[k].p_vaddr;
        }
        x.offset = get_te32(&phdri[k].p_offset);
        x.size   = get_te32(&phdri[k].p_filesz);
        if (!is_shlib || hdr_u_len < (u32_t)x.size) {
            if (0 == nx) { // 1st PT_LOAD32 must cover Ehdr at 0==p_offset
                unsigned const delta = hdr_u_len;
                if (ft.id < 0x40) {
                    // FIXME: ??     ft.addvalue += asl_delta;
                }
                if ((off_t)delta == x.size) { // PT_LOAD[0] with ElfXX.Ehdr only
                    // QBE backend - http://c9x.me/compile/
                    hdr_u_len = 0;  // no fiddling necessary!
                    // &ft arg to packExtent will be zero becaue (k != nk_f)
                }
                else {
                    x.offset += delta;
                    x.size   -= delta;
                }
            }
            // compressWithFilters() always assumes a "loader", so would
            // throw NotCompressible for small .data Extents, which PowerPC
            // sometimes marks as PF_X anyway.  So filter only first segment.
            if (k == nk_f || !is_shlib) {
                packExtent(x,
                    (k==nk_f ? &ft : nullptr ), fo, hdr_u_len);
            }
            else {
                total_in += x.size;
            }
        }
        else {
                total_in += x.size;
        }
        hdr_u_len = 0;
        ++nx;
    }
    sz_pack2a = fpad4(fo);  // MATCH01

    // Accounting only; ::pack3 will do the compression and output
    for (k = 0; k < e_phnum; ++k) {
        total_in += find_LOAD_gap(phdri, k, e_phnum);
    }

    if (total_in != (u32_t)file_size)
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
    unsigned const is_asl = (!!opt->o_unix.android_shlib) << 1;  // bit 1
    unsigned const is_shlib = (0!=xct_off) | is_asl;

    // count passes, set ptload vars
    uip->ui_total_passes = 0;
    for (k = 0; k < e_phnum; ++k) {
        if (PT_LOAD64==get_te32(&phdri[k].p_type)) {
            if (!is_shlib) {
                uip->ui_total_passes++;
            }
            else {
                unsigned p_flags = get_te32(&phdri[k].p_flags);
                if (Elf64_Phdr::PF_W & p_flags) {
                    // rtld might write, so cannot compress
                }
                else {
                    upx_uint64_t p_filesz = get_te64(&phdri[k].p_filesz);
                    // First PT_LOAD (partial) only if has instructions
                    if (k || xct_off < p_filesz) {
                        uip->ui_total_passes++;
                    }
                }
            }
            if (find_LOAD_gap(phdri, k, e_phnum)) {
                uip->ui_total_passes++;
            }
        }
    }

    // compress extents
    unsigned hdr_u_len = sizeof(Elf64_Ehdr) + sz_phdrs;

    total_in =  0;
    total_out = 0;
    uip->ui_pass = 0;
    ft.addvalue = 0;

    if (is_shlib) { // prepare to alter Phdrs and Shdrs
        lowmem.alloc(xct_off + (!is_asl
            ? 0
            : e_shnum * sizeof(Elf64_Shdr)));
        memcpy(lowmem, file_image, xct_off);  // android omits Shdr here

        if (is_asl) { // Android shared library
            sz_elf_hdrs = xct_off;
            fo->write(lowmem, xct_off);  // < SHF_EXECINSTR (typ: in .plt or .init)
            total_in  = xct_off;
            total_out = xct_off;

            asl_pack2_Shdrs(fo);
        }
    }
    unsigned nk_f = 0; upx_uint64_t xsz_f = 0;
    for (k = 0; k < e_phnum; ++k)
    if (PT_LOAD64==get_te32(&phdri[k].p_type)
    &&  Elf64_Phdr::PF_X & get_te64(&phdri[k].p_flags)) {
        upx_uint64_t xsz = get_te64(&phdri[k].p_filesz);
        if (xsz_f < xsz) {
            xsz_f = xsz;
            nk_f = k;
        }
    }
    int nx = 0;
    for (k = 0; k < e_phnum; ++k)
    if (PT_LOAD64==get_te32(&phdri[k].p_type)) {
        if (ft.id < 0x40) {
            // FIXME: ??    ft.addvalue = phdri[k].p_vaddr;
        }
        x.offset = get_te64(&phdri[k].p_offset);
        x.size   = get_te64(&phdri[k].p_filesz);
        if (is_shlib) {
            if (x.offset <= xct_off) { // first PT_LOAD
                unsigned const len = umin(x.size, xct_off - x.offset);
                if (len && !is_asl) { // asl_pack2_Shdrs aleady handled
                    fi->seek(x.offset, SEEK_SET);
                    fi->readx(ibuf, x.size);
                    total_in += len;

                    fo->seek(x.offset, SEEK_SET);
                    fo->write(ibuf, len);
                    total_out += len;
                }
                if (len != x.size) {
                    linfo.l_checksum = 0;
                    linfo.l_magic = UPX_MAGIC_LE32;
                    set_le16(&linfo.l_lsize, lsize);  // preliminary (0)
                    linfo.l_version = (unsigned char)ph.version;
                    linfo.l_format =  (unsigned char)ph.format;
                    linfo_off = fo->tell();
                    fo->write(&linfo, sizeof(linfo));
                    total_out += sizeof(linfo);
                    overlay_offset = total_out;

                    p_info hbuf;
                    set_te32(&hbuf.p_progid, 0);
                    set_te32(&hbuf.p_filesize, file_size);
                    set_te32(&hbuf.p_blocksize, blocksize);
                    fo->write(&hbuf, sizeof(hbuf));
                    total_out += sizeof(p_info);

                    x.offset = 0;
                    x.size = sz_elf_hdrs;
                    if (is_asl) {
                        x.size = hdr_u_len;
                    }
                    unsigned in_size = x.size;
                    packExtent(x, nullptr, fo, 0, 0, true);
                    total_in -= in_size;

                    // The rest of first PT_LOAD (above xct_off)
                    x.offset = xct_off;
                    x.size = get_te64(&phdri[k].p_filesz) - len;
                    packExtent(x, &ft, fo, 0, 0, true);
                }
            }
            else {
                if (!(Elf64_Phdr::PF_W & get_te64(&phdri[k].p_flags))) {
                    // Read-only PT_LOAD, assume not written by relocations.
                    // Also assume not the source for R_*_COPY relocation,
                    // therefore compress it.
                    packExtent(x, &ft, fo, 0, 0, true);
                    // De-compressing will re-create it, but otherwise ignore it.
                    Elf64_Phdr *phdro = (Elf64_Phdr *)(1+ (Elf64_Ehdr *)&lowmem[0]);
                    set_te32(&phdro[k].p_type, Elf64_Phdr::PT_NULL);
                }
                else {
                    // Read-write PT_LOAD.
                    // rtld might relocate, so we cannot compress.
                    // (Could compress if not relocated; complicates run-time.)
                    // Postpone writing until "slide", but account for its size.
                    total_in +=  x.size;
                }
            }
        }
        else  // main program, not shared library
        if (hdr_u_len <= (u64_t)x.size) {
            if (0 == nx) { // 1st PT_LOAD64 must cover Ehdr at 0==p_offset
                unsigned const delta = hdr_u_len;
                if (ft.id < 0x40) {
                    // FIXME: ??     ft.addvalue += asl_delta;
                }
                if ((off_t)delta == x.size) { // PT_LOAD[0] with ElfXX.Ehdr only
                    // QBE backend - http://c9x.me/compile/
                    hdr_u_len = 0;  // no fiddling necessary!
                    // &ft arg to packExtent will be zero becaue (k != nk_f)
                }
                else {
                    total_in += delta - hdr_u_len;
                    x.offset += delta;
                    x.size   -= delta;
                }
            }
            // compressWithFilters() always assumes a "loader", so would
            // throw NotCompressible for small .data Extents, which PowerPC
            // sometimes marks as PF_X anyway.  So filter only first segment.
            if (k == nk_f || !is_shlib) {
                packExtent(x,
                    (k==nk_f ? &ft : nullptr ), fo, hdr_u_len, 0, true);
            }
            else {
                total_in += x.size;
            }
        }
        else {
                total_in += x.size;
        }
        hdr_u_len = 0;
        ++nx;
    }
    sz_pack2a = fpad4(fo);  // MATCH01

    // Accounting only; ::pack3 will do the compression and output
    for (k = 0; k < e_phnum; ++k) {
        total_in += find_LOAD_gap(phdri, k, e_phnum);
    }

    if (total_in != (u32_t)file_size)
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
    overlay_offset = xct_off ? xct_off : (sz_elf_hdrs + sizeof(linfo));

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
    set_te32(&elfout.phdr[C_TEXT].p_filesz, sz_pack2 + lsize);
              elfout.phdr[C_TEXT].p_memsz = elfout.phdr[C_TEXT].p_filesz;
    super::pack4(fo, ft);  // write PackHeader and overlay_offset

    fo->seek(0, SEEK_SET);
    if (0!=xct_off) {  // shared library
        { // Shouldn't this special case be handled earlier?
            if (overlay_offset < xct_off) {
                Elf32_Phdr *phdro = (Elf32_Phdr *)(&lowmem[sizeof(Elf32_Ehdr)]);
                set_te32(&phdro->p_flags, Elf32_Phdr::PF_X | get_te32(&phdro->p_flags));
            }
        }
        fo->rewrite(&lowmem[0], sizeof(ehdri) + e_phnum * sizeof(*phdri));
        fo->seek(sz_elf_hdrs, SEEK_SET);
        fo->rewrite(&linfo, sizeof(linfo));

        if (jni_onload_va) {
            unsigned tmp = sz_pack2 + get_te32(&elfout.phdr[C_TEXT].p_vaddr);
            tmp |= (Elf32_Ehdr::EM_ARM==e_machine);  // THUMB mode
            set_te32(&tmp, tmp);
            fo->seek(ptr_udiff_bytes(&jni_onload_sym->st_value, file_image), SEEK_SET);
            fo->rewrite(&tmp, sizeof(tmp));
        }
    }
    else {
        unsigned const reloc = get_te32(&elfout.phdr[C_TEXT].p_vaddr);
        Elf32_Phdr *phdr = &elfout.phdr[C_NOTE];
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
    if (!xct_off) {
        overlay_offset = sz_elf_hdrs + sizeof(linfo);
    }

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
    set_te64(&elfout.phdr[C_TEXT].p_filesz, sz_pack2 + lsize);
              elfout.phdr[C_TEXT].p_memsz = elfout.phdr[C_TEXT].p_filesz;
    super::pack4(fo, ft);  // write PackHeader and overlay_offset

    fo->seek(0, SEEK_SET);
    if (0!=xct_off) {  // shared library
        { // Shouldn't this special case be handled earlier?
            if (overlay_offset < xct_off) {
                Elf64_Phdr *phdro = (Elf64_Phdr *)(&lowmem[sizeof(Elf64_Ehdr)]);
                set_te64(&phdro->p_flags, Elf64_Phdr::PF_X | get_te64(&phdro->p_flags));
            }
        }
        fo->rewrite(&lowmem[0], sizeof(ehdri) + e_phnum * sizeof(Elf64_Phdr));
        //fo->seek(xct_off, SEEK_SET);  // FIXME
        //fo->rewrite(&linfo, sizeof(linfo));
        fo->seek(linfo_off, SEEK_SET);
        fo->rewrite(&linfo, sizeof(linfo));  // duplicate?
    }
    else {
        if (PT_NOTE64 == get_te64(&elfout.phdr[C_NOTE].p_type)) {
            upx_uint64_t const reloc = get_te64(&elfout.phdr[C_TEXT].p_vaddr);
            set_te64(            &elfout.phdr[C_NOTE].p_vaddr,
                reloc + get_te64(&elfout.phdr[C_NOTE].p_vaddr));
            set_te64(            &elfout.phdr[C_NOTE].p_paddr,
                reloc + get_te64(&elfout.phdr[C_NOTE].p_paddr));
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
    upx_uint64_t old_dtinit,
    OutputFile *fo
)
{
    Elf64_Rela *rela = rela0;
    for (int k = relasz / sizeof(Elf64_Rela); --k >= 0; ++rela) {
        upx_uint64_t r_addend = get_te64(&rela->r_addend);
        if (xct_off <= r_addend) {
            r_addend -= asl_delta;
            set_te64(&rela->r_addend, r_addend);
        }

        upx_uint64_t r_offset = get_te64(&rela->r_offset);
        if (xct_off <= r_offset) {
            r_offset -= asl_delta;
            set_te64(&rela->r_offset, r_offset);
        }

        upx_uint64_t r_info   = get_te64(&rela->r_info);
        unsigned r_type = ELF64_R_TYPE(r_info);
        if (Elf64_Ehdr::EM_AARCH64 == e_machine) {
            if (R_AARCH64_RELATIVE == r_type) {
                if (old_dtinit == r_addend) {
                    set_te64(&ptload1[r_offset - load_off], r_addend);
                }
            }
            if (R_AARCH64_JUMP_SLOT == r_type) {
                ++n_jmp_slot;
                // .rela.plt contains offset of the "first time" target
                upx_uint64_t d = r_offset - load_off;
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

// File layout of compressed .so (new-style: 3 or 4 PT_LOAD) shared library:
// 1. new Elf headers: Ehdr, PT_LOAD (r-x), PT_LOAD (rw-, if any), non-PT_LOAD Phdrs
// 2. Space for (original - 2) PT_LOAD Phdr
// 3. Remaining original contents of file below xct_off
// xct_off: (&lowest eXecutable Shdr section; in original PT_LOAD[0] or [1])
// 4. l_info (12 bytes)
// overlay_offset:
// 5. p_info (12 bytes)
// 6. compressed original Elf headers (prefixed by b_info as usual)
// 7. compressed remainder of PT_LOAD above xct_off
// 8. compressed read-only PT_LOAD above xct_off (if any)
// 9. uncompressed Read-Write PT_LOAD (slide down N pages)
// 10. int[6] tables for UPX runtime de-compressor
// (new) DT_INIT:
// 11. UPX runtime de-compressing loader
// 12. compressed gaps between PT_LOADs (and EOF) above xct_off
// 13. 32-byte pack header
// 14. 4-byte overlay offset

void PackLinuxElf64::un_shlib_1(
    OutputFile *const fo,
    MemBuffer &o_elfhdrs,
    unsigned &c_adler,
    unsigned &u_adler,
    Elf64_Phdr const *const dynhdr,
    unsigned const orig_file_size,
    unsigned const szb_info
)
{
    // Below xct_off is not compressed (for benefit of rtld.)
    fi->seek(0, SEEK_SET);
    unsigned const limit_dynhdr = get_te64(&dynhdr->p_offset) + get_te64(&dynhdr->p_filesz);
    fi->readx(ibuf, limit_dynhdr);
    overlay_offset -= sizeof(linfo);
    loader_offset = 0;
    xct_off = overlay_offset;
    e_shoff = get_te64(&ehdri.e_shoff);
    if (e_shoff && e_shnum
    &&  (e_shoff + sizeof(Elf64_Shdr) * e_shnum) <= limit_dynhdr) { // --android-shlib
        ibuf.subref("bad .e_shoff %#lx for %#lx", e_shoff, sizeof(Elf64_Shdr) * e_shnum);
        shdri = (Elf64_Shdr /*const*/ *)ibuf.subref(
            "bad Shdr table", e_shoff, sizeof(Elf64_Shdr)*e_shnum);
        upx_uint64_t xct_off2 = get_te64(&shdri->sh_offset);
        if (e_shoff == xct_off2) {
            xct_off = e_shoff;
        }
        // un-Relocate dynsym (DT_SYMTAB) which is below xct_off
        dynstr = (char const *)elf_find_dynamic(Elf64_Dyn::DT_STRTAB);
        sec_dynsym = elf_find_section_type(Elf64_Shdr::SHT_DYNSYM);
        if (sec_dynsym) {
            upx_uint64_t const off_dynsym = get_te64(&sec_dynsym->sh_offset);
            upx_uint64_t const sz_dynsym  = get_te64(&sec_dynsym->sh_size);
            if (orig_file_size < sz_dynsym
            ||  orig_file_size < off_dynsym
            || (orig_file_size - off_dynsym) < sz_dynsym) {
                throwCantUnpack("bad SHT_DYNSYM");
            }
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
                if (Elf64_Sym::SHN_ABS == symsec && xct_off <= symval) {
                    adjABS(sym, 0u - asl_delta);
                }
            }
        }
    }

    // Decompress first Extent.  Old style covers [0, xct_off)
    // which includes rtld constant data and eXecutable app code below DT_INIT.
    // In old style, the first compressed Extent is redundant
    // except for the compressed original Elf headers.
    // New style covers just Elf headers: the rest below xct_off is
    // rtld constant data: DT_*HASH, DT_SYMTAB, DT_STRTAB, etc.
    // New style puts eXecutable app code in second PT_LOAD
    // in order to mark Elf headers and rtld data as non-eXecutable.
    fi->seek(xct_off, SEEK_SET);
    struct {
        struct l_info l;
        struct p_info p;
        struct b_info b;
    } hdr;
    fi->readx(&hdr, sizeof(hdr));
    fi->seek(-(off_t)sizeof(struct b_info), SEEK_CUR);
    if (hdr.l.l_magic != UPX_MAGIC_LE32
    ||  hdr.l.l_lsize != (unsigned)lsize
    ||  hdr.p.p_filesize != ph.u_file_size) {
        throwCantUnpack("corrupt l_info/p_info");
    }
    ph.c_len = get_te32(&hdr.b.sz_cpr);
    ph.u_len = get_te32(&hdr.b.sz_unc);

    unpackExtent(ph.u_len, fo,
        c_adler, u_adler, false, szb_info);

    // FIXME: what if no output file?  test mode ("-t") or list mode ("-l")
    if (fo) {
        InputFile u_fi;
        // Recover original Elf headers from current output file
        u_fi.open(fo->getName(), 0);
        u_fi.readx((void *)o_elfhdrs,o_elfhdrs.getSize());
        u_fi.close();

        // Re-generate unmodified rtld data below xct_off
        fo->write(&ibuf[ph.u_len], xct_off - ph.u_len);
    }

    Elf64_Phdr const *o_phdr = (Elf64_Phdr const *)(1+ (Elf64_Ehdr const *)(void const *)o_elfhdrs);
    // Handle compressed PT_LOADs (must not have PF_W)
    for (unsigned j = 0; j < e_phnum; ++j, ++o_phdr) {
        unsigned type = get_te32(&o_phdr->p_type);
        unsigned flags = get_te32(&o_phdr->p_flags);
        if (PT_LOAD64 != type || Elf64_Phdr::PF_W & flags) {
            continue;
        }
        unsigned vaddr = get_te64(&o_phdr->p_vaddr);
        if (xct_off <= vaddr) { // not first PT_LOAD must position its output
            if (fo) {
                unsigned o_offset = get_te64(&o_phdr->p_offset);
                fo->seek(o_offset, SEEK_SET);
            }
        }
        // Peek at b_info to find sizes
        fi->readx(&hdr.b, sizeof(hdr.b));
        fi->seek(-(off_t)sizeof(struct b_info), SEEK_CUR);
        ph.c_len = get_te32(&hdr.b.sz_cpr);
        ph.u_len = get_te32(&hdr.b.sz_unc);
        unpackExtent(ph.u_len, fo, c_adler, u_adler, false, szb_info);
    }
    funpad4(fi);
    loader_offset = fi->tell();

    // Handle PT_LOAD with PF_W: writeable, so not compressed.  "Slide"
    o_phdr = (Elf64_Phdr const *)(1+ (Elf64_Ehdr const *)(void const *)o_elfhdrs);
    Elf64_Phdr const *i_phdr = phdri;
    for (unsigned j = 0; j < e_phnum; ++j, ++o_phdr, ++i_phdr) {
        unsigned type = get_te32(&o_phdr->p_type);
        unsigned flags = get_te32(&o_phdr->p_flags);
        if (PT_LOAD64 != type || !(Elf64_Phdr::PF_W & flags)) {
            continue;
        }
        unsigned filesz = get_te64(&o_phdr->p_filesz);
        unsigned o_offset = get_te64(&o_phdr->p_offset);
        unsigned i_offset = get_te64(&i_phdr->p_offset);
        fi->seek(i_offset, SEEK_SET);
        fi->readx(ibuf, filesz);
        total_in += filesz;
        if (fo) {
            fo->seek(o_offset, SEEK_SET);
            fo->write(ibuf, filesz);
        }
        total_out = filesz + o_offset;  // high-water mark
    }

    // Gaps between PT_LOAD will be handled by ::unpack()

    // position fi at loader offset
    fi->seek(loader_offset, SEEK_SET);
}

void PackLinuxElf64::un_DT_INIT(
    unsigned old_dtinit,
    Elf64_Phdr const *const phdro,
    Elf64_Phdr const *const dynhdr,  // in phdri
    OutputFile *fo,
    unsigned is_asl
)
{
    // DT_INIT must be restored.
    // If android_shlib, then the asl_delta relocations must be un-done.
    upx_uint64_t dt_pltrelsz(0), dt_jmprel(0);
    upx_uint64_t dt_relasz(0), dt_rela(0);
    upx_uint64_t const dyn_len = get_te64(&dynhdr->p_filesz);
    upx_uint64_t const dyn_off = get_te64(&dynhdr->p_offset);
    if ((unsigned long)file_size < (dyn_len + dyn_off)) {
        char msg[50]; snprintf(msg, sizeof(msg),
                "bad PT_DYNAMIC .p_filesz %#lx", (long unsigned)dyn_len);
        throwCantUnpack(msg);
    }
    fi->seek(dyn_off, SEEK_SET);
    fi->readx(ibuf, dyn_len);
    Elf64_Dyn *dyn = (Elf64_Dyn *)(void *)ibuf;
    dynseg = dyn; invert_pt_dynamic(dynseg,
        umin(dyn_len, file_size - dyn_off));
    for (unsigned j2= 0; j2 < dyn_len; ++dyn, j2 += sizeof(*dyn)) {
        upx_uint64_t const tag = get_te64(&dyn->d_tag);
        upx_uint64_t       val = get_te64(&dyn->d_val);
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
        } // end switch() on tag when is_asl
        if (upx_dt_init == tag) {
            if (Elf64_Dyn::DT_INIT == tag) {
                set_te64(&dyn->d_val, old_dtinit);
                if (!old_dtinit) { // compressor took the slot
                    dyn->d_tag = Elf64_Dyn::DT_NULL;
                    dyn->d_val = 0;
                }
            }
            else if (Elf64_Dyn::DT_INIT_ARRAY    == tag
            ||       Elf64_Dyn::DT_PREINIT_ARRAY == tag) {
                // The slot must have a R_*_RELATIVE relocation (is_shlib,
                // after all), but ElfXX_Rela ignores the initial contents!
                // So changing the value will get ignored.  Do it anyway.
                // FIXME: we must fix the Rela ?
                Elf64_Phdr const *phdr = phdro;
                for (unsigned j = 0; j < e_phnum; ++j, ++phdr) {
                    upx_uint64_t vaddr = get_te64(&phdr->p_vaddr);
                    upx_uint64_t filesz = get_te64(&phdr->p_filesz);
                    if ((val - vaddr) < filesz) {
                        upx_uint64_t offset = get_te64(&phdr->p_offset);
                        upx_uint64_t oldval;
                        // Counter-act unRel64 if asl_delta
                        set_te64(&oldval, old_dtinit + (is_asl ? asl_delta : 0));
                        // FIXME? the in-memory copy?
                        if (fo) {
                            fo->seek((val - vaddr) + offset, SEEK_SET);
                            fo->write(&oldval, sizeof(oldval));
                        }
                        break;
                    }
                }
            }
        }
    }
    if (fo) { // Write updated dt_*.val
        upx_uint64_t dyn_offo = get_te64(&phdro[dynhdr - phdri].p_offset);
        fo->seek(dyn_offo, SEEK_SET);
        fo->rewrite(ibuf, dyn_len);
    }
    if (is_asl) {
        lowmem.alloc(xct_off);
        fi->seek(0, SEEK_SET);
        fi->read(lowmem, xct_off);  // contains relocation tables
        if (dt_relasz && dt_rela) {
            Elf64_Rela *const rela0 = (Elf64_Rela *)lowmem.subref(
                "bad Rela offset", dt_rela, dt_relasz);
            unRela64(dt_rela, rela0, dt_relasz, ibuf, load_va, old_dtinit, fo);
        }
        if (dt_pltrelsz && dt_jmprel) { // FIXME:  overlap w/ DT_REL ?
            Elf64_Rela *const jmp0 = (Elf64_Rela *)lowmem.subref(
                "bad Jmprel offset", dt_jmprel, dt_pltrelsz);
            unRela64(dt_jmprel, jmp0, dt_pltrelsz, ibuf, load_va, old_dtinit, fo);
        }
        // Modified relocation tables are re-written by unRela64
    }
}

void PackLinuxElf64::unpack(OutputFile *fo)
{
    if (e_phoff != sizeof(Elf64_Ehdr)) {// Phdrs not contiguous with Ehdr
        throwCantUnpack("bad e_phoff");
    }
    unsigned const c_phnum = get_te16(&ehdri.e_phnum);
    unsigned u_phnum = 0;
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
    if ((u32_t)file_size > orig_file_size || blocksize > orig_file_size
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
    Elf64_Phdr const *phdr = nullptr;
    total_in = 0;
    total_out = 0;
    unsigned c_adler = upx_adler32(nullptr, 0);
    unsigned u_adler = upx_adler32(nullptr, 0);

    unsigned is_shlib = 0;
    loader_offset = 0;
    MemBuffer o_elfhdrs;
    Elf64_Phdr const *const dynhdr = elf_find_ptype(Elf64_Phdr::PT_DYNAMIC, phdri, c_phnum);
    if (dynhdr) {
        upx_uint64_t dyn_offset = get_te64(&dynhdr->p_offset);
        upx_uint64_t dyn_filesz = get_te64(&dynhdr->p_filesz);
        dynseg = (Elf64_Dyn const *)ibuf.subref("bad DYNAMIC", dyn_offset, dyn_filesz);
        // Packed shlib? (ET_DYN without -fPIE)
        if (!(Elf64_Dyn::DF_1_PIE & elf_unsigned_dynamic(Elf64_Dyn::DT_FLAGS_1))) {
            is_shlib = 1;
            u_phnum = get_te16(&ehdri.e_phnum);
            o_elfhdrs.alloc(sz_elf_hdrs);
            un_shlib_1(fo, o_elfhdrs, c_adler, u_adler, dynhdr, orig_file_size, szb_info);
            *ehdr = ehdri;
        }
    }
    else { // main executable
        // Uncompress Ehdr and Phdrs: info for control of unpacking
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
        // Rewind: prepare for data phase
        fi->seek(- (off_t) (szb_info + ph.c_len), SEEK_CUR);

        u_phnum = get_te16(&ehdr->e_phnum);
#define MAX_ELF_HDR 1024
        if ((umin64(MAX_ELF_HDR, ph.u_len) - sizeof(Elf64_Ehdr))/sizeof(Elf64_Phdr) < u_phnum) {
            throwCantUnpack("bad compressed e_phnum");
        }
        o_elfhdrs.alloc(sizeof(Elf64_Ehdr) + u_phnum * sizeof(Elf64_Phdr));
        memcpy(o_elfhdrs, ehdr, o_elfhdrs.getSize());
#undef MAX_ELF_HDR

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
                    unpackExtent(filesz, fo,
                        c_adler, u_adler, first_PF_X, szb_info);
                    first_PF_X = false;
                }
                else {
                    unpackExtent(filesz, fo,
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
    unsigned d_info[6];
    unsigned sz_d_info = sizeof(d_info);
    if (!is_shlib) {
        if (get_te32(&phdri[0].p_flags) & Elf64_Phdr::PF_X) {
            // Old style, such as upx-3.91 thru upx-3.95
            switch (this->e_machine) { // FIXME: missing 32-bit EM_386 EM_ARM EM_PPC
                default: {
                    char msg[40]; snprintf(msg, sizeof(msg),
                        "Unknown architecture %d", this->e_machine);
                    throwCantUnpack(msg);
                }; break;
                case Elf64_Ehdr::EM_AARCH64: sz_d_info = 4 * sizeof(unsigned); break;
                case Elf64_Ehdr::EM_PPC64:   sz_d_info = 3 * sizeof(unsigned); break;
                case Elf64_Ehdr::EM_X86_64:  sz_d_info = 2 * sizeof(unsigned); break;
            }
        }
        loader_offset = get_te64(&ehdri.e_entry) - load_va - sz_d_info;
    }

    if (0x1000==get_te64(&phdri[0].p_filesz)  // detect C_BASE style
    &&  0==get_te64(&phdri[1].p_offset)
    &&  0==get_te64(&phdri[0].p_offset)
    &&     get_te64(&phdri[1].p_filesz) == get_te64(&phdri[1].p_memsz)) {
        fi->seek(up4(get_te64(&phdr[1].p_memsz)), SEEK_SET);  // past the loader
    }
    else if (is_shlib
    ||  ((unsigned)(get_te64(&ehdri.e_entry) - load_va) + up4(lsize) +
                ph.getPackHeaderSize() + sizeof(overlay_offset))
            < up4(file_size)) {
        // Loader is not at end; skip past it.
        if (loader_offset) {
            fi->seek(loader_offset, SEEK_SET);
        }
        else {
            funpad4(fi);  // MATCH01
        }
        fi->readx(d_info, sz_d_info);
        if (is_shlib && 0==old_dtinit) {
            old_dtinit = get_te32(&d_info[2 + (0==d_info[0])]);
            is_asl = 1u& get_te32(&d_info[0 + (0==d_info[0])]);
        }
        fi->seek(lsize - sz_d_info, SEEK_CUR);
    }

    // The gaps between PT_LOAD and after last PT_LOAD
    phdr = (Elf64_Phdr const *)(1+ (Elf64_Ehdr const *)(void const *)o_elfhdrs);
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
            unpackExtent(size, fo,
                c_adler, u_adler, false, szb_info,
                is_shlib && ((phdr[j].p_offset != hi_offset)));
                // FIXME: should not depend on is_shlib ?
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
        un_DT_INIT(old_dtinit, (Elf64_Phdr *)(1+ (Elf64_Ehdr *)(void *)o_elfhdrs), dynhdr, fo, is_asl);
    }

    // update header with totals
    ph.c_len = total_in;
    ph.u_len = total_out;

    // all bytes must be written
    if (fo && total_out != orig_file_size)
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

int PackLinuxElf32x86::canUnpack()  // really 'bool'
{
    if (super::canUnpack()) {
        return true;
    }
    return false;
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
    for (; --j>=0; ++phdr) if (is_LOAD32(phdr)) {
        unsigned const t = addr - get_te32(&phdr->p_vaddr);
        if (t < get_te32(&phdr->p_filesz)) {
            unsigned const p_offset = get_te32(&phdr->p_offset);
            if ((u32_t)file_size <= p_offset) { // FIXME: weak
                char msg[40]; snprintf(msg, sizeof(msg),
                    "bad Elf32_Phdr[%d].p_offset %x",
                    -1+ e_phnum - j, p_offset);
                throwCantPack(msg);
            }
            return t + p_offset;
        }
    }
    return 0;
}

u32_t  // returns .p_offset
PackLinuxElf32::check_pt_load(Elf32_Phdr const *const phdr)
{
    u32_t filesz = get_te32(&phdr->p_filesz);
    u32_t offset = get_te32(&phdr->p_offset), offend = filesz + offset;
    u32_t vaddr  = get_te32(&phdr->p_vaddr);
    u32_t paddr  = get_te32(&phdr->p_paddr);
    u32_t align  = get_te32(&phdr->p_align);

    if ((-1+ align) & (paddr ^ vaddr)
    ||  (u32_t)file_size <= (u32_t)offset
    ||  (u32_t)file_size <  (u32_t)offend
    ||  (u32_t)file_size <= (u32_t)filesz) {
        char msg[50]; snprintf(msg, sizeof(msg), "bad PT_LOAD phdr[%u]",
            (unsigned)(phdr - phdri));
        throwCantPack(msg);
    }
    return offset;
}

Elf32_Dyn const *
PackLinuxElf32::elf_has_dynamic(unsigned int key) const
{
    Elf32_Dyn const *dynp= dynseg;
    if (dynp)
    for (; Elf32_Dyn::DT_NULL!=dynp->d_tag; ++dynp) if (get_te32(&dynp->d_tag)==key) {
        return dynp;
    }
    return nullptr;
}

unsigned  // checked .p_offset; sz_dynseg set
PackLinuxElf32::check_pt_dynamic(Elf32_Phdr const *const phdr)
{
    unsigned t = get_te32(&phdr->p_offset), s = sizeof(Elf32_Dyn) + t;
    unsigned vaddr = get_te32(&phdr->p_vaddr);
    unsigned filesz = get_te32(&phdr->p_filesz), memsz = get_te32(&phdr->p_memsz);
    unsigned align = get_te32(&phdr->p_align);
    if (file_size_u < t || s < t
    ||  file_size_u < filesz
    ||  file_size_u < (filesz + t)
    ||  t < (e_phnum*sizeof(Elf32_Phdr) + sizeof(Elf32_Ehdr))
    ||  (3 & t) || (7 & (filesz | memsz))  // .balign 4; 8==sizeof(Elf32_Dyn)
    ||  (-1+ align) & (t ^ vaddr)
    ||  file_size_u <= memsz
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
        if (t && t < (unsigned)file_size) {
            return t + file_image;
        }
        break;
    }
    return nullptr;
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
            upx_uint64_t const p_offset = get_te64(&phdr->p_offset);
            if ((u64_t)file_size <= p_offset) { // FIXME: weak
                char msg[40]; snprintf(msg, sizeof(msg),
                    "bad Elf64_Phdr[%d].p_offset %#lx",
                    -1+ e_phnum - j, (long unsigned)p_offset);
                throwCantPack(msg);
            }
            return t + p_offset;
        }
    }
    return 0;
}

u64_t  // returns .p_offset
PackLinuxElf64::check_pt_load(Elf64_Phdr const *const phdr)
{
    u64_t filesz = get_te64(&phdr->p_filesz);
    u64_t offset = get_te64(&phdr->p_offset), offend = filesz + offset;
    u64_t vaddr  = get_te64(&phdr->p_vaddr);
    u64_t paddr  = get_te64(&phdr->p_paddr);
    u64_t align  = get_te64(&phdr->p_align);

    if ((-1+ align) & (paddr ^ vaddr)
    ||  (u64_t)file_size <= (u64_t)offset
    ||  (u64_t)file_size <  (u64_t)offend
    ||  (u64_t)file_size <= (u64_t)filesz) {
        char msg[50]; snprintf(msg, sizeof(msg), "bad PT_LOAD phdr[%u]",
            (unsigned)(phdr - phdri));
        throwCantPack(msg);
    }
    return offset;
}

Elf64_Dyn const *
PackLinuxElf64::elf_has_dynamic(unsigned int key) const
{
    Elf64_Dyn const *dynp= dynseg;
    if (dynp)
    for (; Elf64_Dyn::DT_NULL!=dynp->d_tag; ++dynp) if (get_te64(&dynp->d_tag)==key) {
        return dynp;
    }
    return nullptr;
}

upx_uint64_t  // checked .p_offset; sz_dynseg set
PackLinuxElf64::check_pt_dynamic(Elf64_Phdr const *const phdr)
{
    upx_uint64_t t = get_te64(&phdr->p_offset), s = sizeof(Elf64_Dyn) + t;
    upx_uint64_t vaddr = get_te64(&phdr->p_vaddr);
    upx_uint64_t filesz = get_te64(&phdr->p_filesz), memsz = get_te64(&phdr->p_memsz);
    upx_uint64_t align = get_te64(&phdr->p_align);
    if (file_size_u < t || s < t
    ||  file_size_u < filesz
    ||  file_size_u < (filesz + t)
    ||  t < (e_phnum*sizeof(Elf64_Phdr) + sizeof(Elf64_Ehdr))
    ||  (7 & t) || (0xf & (filesz | memsz))  // .balign 8; 16==sizeof(Elf64_Dyn)
    ||  (-1+ align) & (t ^ vaddr)
    ||  file_size_u <= memsz
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

static int __acc_cdecl_qsort
qcmp_unsigned(void const *const aa, void const *const bb)
{
    unsigned a = *(unsigned const *)aa;
    unsigned b = *(unsigned const *)bb;
    if (a < b) return -1;
    if (a > b) return  1;
    return  0;
}

void
PackLinuxElf64::invert_pt_dynamic(Elf64_Dyn const *dynp, upx_uint64_t headway)
{
    if (dt_table[Elf64_Dyn::DT_NULL]) {
        return;  // not 1st time; do not change upx_dt_init
    }
    Elf64_Dyn const *const dynp0 = dynp;
    unsigned ndx = 0;
    unsigned const limit = headway / sizeof(*dynp);
    if (dynp)
    for (; ; ++ndx, ++dynp) {
        if (limit <= ndx) {
            throwCantPack("DT_NULL not found");
        }
        upx_uint64_t const d_tag = get_te64(&dynp->d_tag);
        if (d_tag>>32) { // outrageous
            char msg[50]; snprintf(msg, sizeof(msg),
                "bad Elf64_Dyn[%d].d_tag %#lx", ndx, (long unsigned)d_tag);
            throwCantPack(msg);
        }
        if (d_tag < DT_NUM) {
            if (Elf64_Dyn::DT_NEEDED != d_tag
            &&  dt_table[d_tag]
            &&    get_te64(&dynp->d_val)
               != get_te64(&dynp0[-1+ dt_table[d_tag]].d_val)) {
                char msg[50]; snprintf(msg, sizeof(msg),
                    "duplicate DT_%#x: [%#x] [%#x]",
                    (unsigned)d_tag, -1+ dt_table[d_tag], ndx);
                throwCantPack(msg);
            }
            dt_table[d_tag] = 1+ ndx;
        }
        if (Elf64_Dyn::DT_NULL == d_tag) {
            break;  // check here so that dt_table[DT_NULL] is set
        }
    }
    upx_dt_init = 0;
         if (dt_table[Elf64_Dyn::DT_INIT])          upx_dt_init = Elf64_Dyn::DT_INIT;
    else if (dt_table[Elf64_Dyn::DT_PREINIT_ARRAY]) upx_dt_init = Elf64_Dyn::DT_PREINIT_ARRAY;
    else if (dt_table[Elf64_Dyn::DT_INIT_ARRAY])    upx_dt_init = Elf64_Dyn::DT_INIT_ARRAY;

    unsigned const z_str = dt_table[Elf64_Dyn::DT_STRSZ];
    strtab_end = !z_str ? 0 : get_te64(&dynp0[-1+ z_str].d_val);
    if (!z_str || (u64_t)file_size <= strtab_end) { // FIXME: weak
        char msg[50]; snprintf(msg, sizeof(msg),
            "bad DT_STRSZ %#x", strtab_end);
        throwCantPack(msg);
    }

    // DT_SYMTAB has no designated length.
    // End it when next area else starts; often DT_STRTAB.  (FIXME)
    unsigned const x_sym = dt_table[Elf64_Dyn::DT_SYMTAB];
    unsigned const x_str = dt_table[Elf64_Dyn::DT_STRTAB];
    if (x_sym && x_str) {
        upx_uint64_t const v_sym = get_te64(&dynp0[-1+ x_sym].d_val);
        upx_uint64_t const v_str = get_te64(&dynp0[-1+ x_str].d_val);
        unsigned const  z_sym = dt_table[Elf64_Dyn::DT_SYMENT];
        unsigned const sz_sym = !z_sym ? sizeof(Elf64_Sym)
            : get_te64(&dynp0[-1+ z_sym].d_val);
        if (sz_sym < sizeof(Elf64_Sym)) {
            char msg[50]; snprintf(msg, sizeof(msg),
                "bad DT_SYMENT %x", sz_sym);
            throwCantPack(msg);
        }
        if (v_sym < v_str) {
            symnum_end = (v_str - v_sym) / sz_sym;
        }
        if (symnum_end < 1) {
            throwCantPack("bad DT_SYMTAB");
        }
    }
    // DT_HASH, DT_GNU_HASH have no explicit length (except in ElfXX_Shdr),
    // so it is hard to detect when the index of a hash chain is out-of-bounds.
    // Workaround: Assume no overlap of DT_* tables.  Then any given table
    // ends when another table begins.  So find the tables, and sort the offsets.
    unsigned const dt_names[] = { // *.d_val are often in this order
        Elf64_Dyn::DT_SYMTAB,
        Elf64_Dyn::DT_VERSYM,
        Elf64_Dyn::DT_VERNEED,
        Elf64_Dyn::DT_HASH,
        Elf64_Dyn::DT_GNU_HASH,
        Elf64_Dyn::DT_STRTAB,
        Elf64_Dyn::DT_VERDEF,
        Elf64_Dyn::DT_REL,
        Elf64_Dyn::DT_RELA,
        Elf64_Dyn::DT_INIT,
        0,
    };
    unsigned dt_offsets[sizeof(dt_names)/sizeof(dt_names[0])];
    unsigned n_off = 0, k;
    for (unsigned j=0; ((k = dt_names[j]),  k); ++j) {
        dt_offsets[n_off] = 0;  // default to "not found"
        if (k < DT_NUM) { // in range of easy table
            if (dt_table[k]) { // present now
                dt_offsets[n_off] = get_te64(&dynp0[-1+ dt_table[k]].d_val);
            }
        }
        else {
            if (file_image) { // why is this guard necessary?
                dt_offsets[n_off] = elf_unsigned_dynamic(k);  // zero if not found
            }
        }
        if (file_size <= dt_offsets[n_off]) {
            char msg[60]; snprintf(msg, sizeof(msg), "bad DT_{%#x} = %#x (beyond EOF)",
                dt_names[k], dt_offsets[n_off]);
                throwCantPack(msg);
        }
        n_off += !!dt_offsets[n_off];
    }
    dt_offsets[n_off++] = file_size;  // sentinel
    qsort(dt_offsets, n_off, sizeof(dt_offsets[0]), qcmp_unsigned);

    unsigned const v_hsh = elf_unsigned_dynamic(Elf64_Dyn::DT_HASH);
    if (v_hsh && file_image) {
        hashtab = (unsigned const *)elf_find_dynamic(Elf64_Dyn::DT_HASH);
        if (!hashtab) {
            char msg[40]; snprintf(msg, sizeof(msg),
               "bad DT_HASH %#x", v_hsh);
            throwCantPack(msg);
        }
        for (unsigned j = 0; j < n_off; ++j) {
            if (v_hsh == dt_offsets[j]) {
                if (dt_offsets[1+ j]) {
                    hashend = (unsigned const *)(void const *)
                        ((dt_offsets[1+ j] - dt_offsets[j]) + (char const *)hashtab);
                }
                break;
            }
        }
        unsigned const nbucket = get_te32(&hashtab[0]);
        unsigned const *const buckets = &hashtab[2];
        unsigned const *const chains = &buckets[nbucket]; (void)chains;

        unsigned const v_sym = !x_sym ? 0 : get_te32(&dynp0[-1+ x_sym].d_val);
        if ((unsigned)file_size <= nbucket/sizeof(*buckets)  // FIXME: weak
        || !v_sym || (unsigned)file_size <= v_sym
        || ((v_hsh < v_sym) && (v_sym - v_hsh) < sizeof(*buckets)*(2+ nbucket))
        ) {
            char msg[80]; snprintf(msg, sizeof(msg),
                "bad DT_HASH nbucket=%#x  len=%#x",
                nbucket, (v_sym - v_hsh));
            throwCantPack(msg);
        }
        unsigned chmax = 0;
        for (unsigned j= 0; j < nbucket; ++j) {
            unsigned x = get_te32(&buckets[j]);
            if (chmax < x) {
                chmax = x;
            }
        }
        if ((v_hsh < v_sym) && (v_sym - v_hsh) <
                (sizeof(*buckets)*(2+ nbucket) + sizeof(*chains)*(1+ chmax))) {
            char msg[80]; snprintf(msg, sizeof(msg),
                "bad DT_HASH nbucket=%#x  len=%#x",
                nbucket, (v_sym - v_hsh));
            throwCantPack(msg);
        }
    }
    // DT_GNU_HASH often ends at DT_SYMTAB;  FIXME: not for Android?
    unsigned const v_gsh = elf_unsigned_dynamic(Elf64_Dyn::DT_GNU_HASH);
    if (v_gsh && file_image) {
        gashtab = (unsigned const *)elf_find_dynamic(Elf64_Dyn::DT_GNU_HASH);
        if (!gashtab) {
            char msg[40]; snprintf(msg, sizeof(msg),
               "bad DT_GNU_HASH %#x", v_gsh);
            throwCantPack(msg);
        }
        for (unsigned j = 0; j < n_off; ++j) { // linear search of short table
            if (v_gsh == dt_offsets[j]) {
                if (dt_offsets[1+ j]) {
                    gashend = (unsigned const *)(void const *)
                        ((dt_offsets[1+ j] - dt_offsets[j]) + (char const *)gashtab);
                }
                break;
            }
        }
        unsigned const n_bucket = get_te32(&gashtab[0]);
        unsigned const symbias  = get_te32(&gashtab[1]);
        unsigned const n_bitmask = get_te32(&gashtab[2]);
        unsigned const gnu_shift = get_te32(&gashtab[3]);
        upx_uint64_t const *const bitmask = (upx_uint64_t const *)(void const *)&gashtab[4];
        unsigned     const *const buckets = (unsigned const *)&bitmask[n_bitmask];
        unsigned     const *const hasharr = &buckets[n_bucket]; (void)hasharr;
        if (!n_bucket || (1u<<31) <= n_bucket  /* fie on fuzzers */
        || (void const *)&file_image[file_size] <= (void const *)hasharr) {
            char msg[80]; snprintf(msg, sizeof(msg),
                "bad n_bucket %#x\n", n_bucket);
            throwCantPack(msg);
        }
        // unsigned const *const gashend = &hasharr[n_bucket];
        // minimum, except:
        // Rust and Android trim unused zeroes from high end of hasharr[]
        unsigned bmax = 0;
        for (unsigned j= 0; j < n_bucket; ++j) {
            unsigned bj = get_te32(&buckets[j]);
            if (bj) {
                if (bj < symbias) {
                    char msg[90]; snprintf(msg, sizeof(msg),
                            "bad DT_GNU_HASH bucket[%d] < symbias{%#x}\n",
                            bj, symbias);
                    throwCantPack(msg);
                }
                if (bmax < bj) {
                    bmax = bj;
                }
            }
        }
        if (1==n_bucket  && 0==buckets[0]
        &&  1==n_bitmask && 0==bitmask[0]) {
            // 2021-09-11 Rust on RaspberryPi apparently uses this to minimize space.
            // But then the DT_GNU_HASH symbol lookup algorithm always fails?
            // https://github.com/upx/upx/issues/525
        } else
        if ((1+ bmax) < symbias) {
            char msg[90]; snprintf(msg, sizeof(msg),
                    "bad DT_GNU_HASH (1+ max_bucket)=%#x < symbias=%#x", 1+ bmax, symbias);
            throwCantPack(msg);
        }
        bmax -= symbias;

        // preliminary bound on gashend
        Elf64_Shdr const *sec_gash = elf_find_section_type(Elf64_Shdr::SHT_GNU_HASH);
        unsigned const off_symtab = elf_unsigned_dynamic(Elf64_Dyn::DT_SYMTAB);
        unsigned const off_strtab = elf_unsigned_dynamic(Elf64_Dyn::DT_STRTAB);
        unsigned const off_gshtab = elf_unsigned_dynamic(Elf64_Dyn::DT_GNU_HASH);
        if (off_gshtab < file_size  // paranoia
        &&  off_strtab < file_size
        &&  off_symtab < file_size ) {
            unsigned sz_gshtab = 0;
            if (sec_gash && off_gshtab == get_te32(&sec_gash->sh_offset)) {
               sz_gshtab = get_te32(&sec_gash->sh_size);
            }
            else { // heuristics
                if (off_gshtab < off_strtab) {
                    sz_gshtab = off_strtab - off_gshtab;
                }
                else if (off_gshtab < off_symtab) {
                    sz_gshtab = off_symtab - off_gshtab;
                }
            }
            if (sz_gshtab <= (file_size - off_gshtab)) {
                gashend = (unsigned const *)(void const *)
                    (sz_gshtab + (char const *)gashtab);
            }
        }

        upx_uint64_t const v_sym = !x_sym ? 0 : get_te64(&dynp0[-1+ x_sym].d_val);
        unsigned r = 0;
        if (!n_bucket || !n_bitmask || !v_sym
        || (r=1, ((-1+ n_bitmask) & n_bitmask))  // not a power of 2
        || (r=2, (8*sizeof(upx_uint64_t) <= gnu_shift))  // shifted result always == 0
        || (r=3, (n_bucket>>30))  // fie on fuzzers
        || (r=4, (n_bitmask>>30))
        || (r=5, ((file_size/sizeof(unsigned))
                <= ((sizeof(*bitmask)/sizeof(unsigned))*n_bitmask + 2*n_bucket)))  // FIXME: weak
        || (r=6, ((v_gsh < v_sym) && (v_sym - v_gsh) < (sizeof(unsigned)*4  // headers
                + sizeof(*bitmask)*n_bitmask  // bitmask
                + sizeof(*buckets)*n_bucket  // buckets
                + sizeof(*hasharr)*(1+ bmax)  // hasharr
            )) )
        ) {
            char msg[90]; snprintf(msg, sizeof(msg),
                "bad DT_GNU_HASH n_bucket=%#x  n_bitmask=%#x  len=%#lx  r=%d",
                n_bucket, n_bitmask, (long unsigned)(v_sym - v_gsh), r);
            throwCantPack(msg);
        }
    }
    unsigned const e_shstrndx = get_te16(&ehdri.e_shstrndx);
    if (e_shnum <= e_shstrndx
    &&  !(0==e_shnum && 0==e_shstrndx) ) {
        char msg[40]; snprintf(msg, sizeof(msg),
            "bad .e_shstrndx %d >= .e_shnum %d", e_shstrndx, e_shnum);
        throwCantPack(msg);
    }
}

void const *
PackLinuxElf64::elf_find_dynamic(unsigned int key) const
{
    Elf64_Dyn const *dynp= dynseg;
    if (dynp)
    for (; (unsigned)((char const *)dynp - (char const *)dynseg) < sz_dynseg
            && Elf64_Dyn::DT_NULL!=dynp->d_tag; ++dynp) if (get_te64(&dynp->d_tag)==key) {
        upx_uint64_t const t= elf_get_offset_from_address(get_te64(&dynp->d_val));
        if (t && t < (upx_uint64_t)file_size) {
            return t + file_image;
        }
        break;
    }
    return nullptr;
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
        if ((unsigned)(file_size - ((char const *)buckets - (char const *)(void const *)file_image))
                <= sizeof(unsigned)*nbucket ) {
            char msg[80]; snprintf(msg, sizeof(msg),
                "bad nbucket %#x\n", nbucket);
            throwCantPack(msg);
        }
        if (nbucket) {
            unsigned const m = elf_hash(name) % nbucket;
            unsigned si;
            for (si= get_te32(&buckets[m]); 0!=si; si= get_te32(&chains[si])) {
                char const *const p= get_dynsym_name(si, (unsigned)-1);
                if (0==strcmp(name, p)) {
                    return &dynsym[si];
                }
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
        unsigned const *const hasharr = &buckets[n_bucket];
        if ((void const *)&file_image[file_size] <= (void const *)hasharr) {
            char msg[80]; snprintf(msg, sizeof(msg),
                "bad n_bucket %#x\n", n_bucket);
            throwCantPack(msg);
        }
        if (!n_bitmask
        || (unsigned)(file_size - ((char const *)bitmask - (char const *)(void const *)file_image))
                <= sizeof(unsigned)*n_bitmask ) {
            char msg[80]; snprintf(msg, sizeof(msg),
                "bad n_bitmask %#x\n", n_bitmask);
            throwCantPack(msg);
        }
        if (n_bucket) {
            unsigned const h = gnu_hash(name);
            unsigned const hbit1 = 037& h;
            unsigned const hbit2 = 037& (h>>gnu_shift);
            unsigned const w = get_te32(&bitmask[(n_bitmask -1) & (h>>5)]);

            if (1& (w>>hbit1) & (w>>hbit2)) {
                unsigned bucket = get_te32(&buckets[h % n_bucket]);
                if (n_bucket <= bucket) {
                    char msg[90]; snprintf(msg, sizeof(msg),
                            "bad DT_GNU_HASH n_bucket{%#x} <= buckets[%d]{%#x}\n",
                            n_bucket, h % n_bucket, bucket);
                    throwCantPack(msg);
                }
                if (0!=bucket) {
                    Elf32_Sym const *dsp = &dynsym[bucket];
                    unsigned const *hp = &hasharr[bucket - symbias];
                    do if (0==((h ^ get_te32(hp))>>1)) {
                        unsigned st_name = get_te32(&dsp->st_name);
                        char const *const p = get_str_name(st_name, (unsigned)-1);
                        if (0==strcmp(name, p)) {
                            return dsp;
                        }
                    } while (++dsp,
                            (char const *)hp < (char const *)&file_image[file_size]
                        &&  0==(1u& get_te32(hp++)));
                }
            }
        }
    }
    // 2021-12-25  FIXME: Some Rust programs use
    //    (1==n_bucket && 0==buckets[0] && 1==n_bitmask && 0==bitmask[0])
    // to minimize space in DT_GNU_HASH. This causes the fancy lookup to fail.
    // Is a fallback to linear seach assumed?
    // 2022-03-12  Some Rust programs have 0==n_bucket.
    return nullptr;

}

Elf64_Sym const *PackLinuxElf64::elf_lookup(char const *name) const
{
    if (hashtab && dynsym && dynstr) {
        unsigned const nbucket = get_te32(&hashtab[0]);
        unsigned const *const buckets = &hashtab[2];
        unsigned const *const chains = &buckets[nbucket];
        if ((unsigned)(file_size - ((char const *)buckets - (char const *)(void const *)file_image))
                <= sizeof(unsigned)*nbucket ) {
            char msg[80]; snprintf(msg, sizeof(msg),
                "bad nbucket %#x\n", nbucket);
            throwCantPack(msg);
        }
        if (nbucket) { // -rust-musl can have "empty" hashtab
            unsigned const m = elf_hash(name) % nbucket;
            unsigned si;
            for (si= get_te32(&buckets[m]); 0!=si; si= get_te32(&chains[si])) {
                char const *const p= get_dynsym_name(si, (unsigned)-1);
                if (0==strcmp(name, p)) {
                    return &dynsym[si];
                }
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
        unsigned     const *const hasharr = &buckets[n_bucket];

        if ((void const *)&file_image[file_size] <= (void const *)hasharr) {
            char msg[80]; snprintf(msg, sizeof(msg),
                "bad n_bucket %#x\n", n_bucket);
            throwCantPack(msg);
        }
        if (!n_bitmask
        || (unsigned)(file_size - ((char const *)bitmask - (char const *)(void const *)file_image))
                <= sizeof(unsigned)*n_bitmask ) {
            char msg[80]; snprintf(msg, sizeof(msg),
                "bad n_bitmask %#x\n", n_bitmask);
            throwCantPack(msg);
        }
        if (n_bucket) { // -rust-musl can have "empty" gashtab
            unsigned const h = gnu_hash(name);
            unsigned const hbit1 = 077& h;
            unsigned const hbit2 = 077& (h>>gnu_shift);
            upx_uint64_t const w = get_te64(&bitmask[(n_bitmask -1) & (h>>6)]);
            if (1& (w>>hbit1) & (w>>hbit2)) {
                unsigned hhead = get_te32(&buckets[h % n_bucket]);
                if (hhead) {
                    Elf64_Sym const *dsp = &dynsym[hhead];
                    unsigned const *hp = &hasharr[hhead - symbias];
                    unsigned k;
                    do {
                        if (gashend <= hp) {
                            char msg[120]; snprintf(msg, sizeof(msg),
                                "bad gnu_hash[%#tx]  head=%u",
                                hp - hasharr, hhead);
                            throwCantPack(msg);
                        }
                        k = get_te32(hp);
                        if (0==((h ^ k)>>1)) {
                            unsigned const st_name = get_te32(&dsp->st_name);
                            char const *const p = get_str_name(st_name, (unsigned)-1);
                            if (0==strcmp(name, p)) {
                                return dsp;
                            }
                        }
                    } while (++dsp, ++hp, 0==(1u& k));
                }
            }
        }
    }
    // 2021-12-25  FIXME: Some Rust programs use
    //    (1==n_bucket && 0==buckets[0] && 1==n_bitmask && 0==bitmask[0])
    // to minimize space in DT_GNU_HASH. This causes the fancy lookup to fail.
    // Is a fallback to linear seach assumed?
    // 2022-03-12  Some Rust programs have 0==n_bucket.
    return nullptr;

}

void PackLinuxElf32::unpack(OutputFile *fo)
{
    if (e_phoff != sizeof(Elf32_Ehdr)) {// Phdrs not contiguous with Ehdr
        throwCantUnpack("bad e_phoff");
    }
    unsigned const c_phnum = get_te16(&ehdri.e_phnum);
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
    if ((u32_t)file_size > orig_file_size || blocksize > orig_file_size
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
    Elf32_Phdr const *phdr = nullptr;

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
    total_in = 0;
    total_out = 0;
    unsigned c_adler = upx_adler32(nullptr, 0);
    unsigned u_adler = upx_adler32(nullptr, 0);
#define MAX_ELF_HDR 512
    if ((umin(MAX_ELF_HDR, ph.u_len) - sizeof(Elf32_Ehdr))/sizeof(Elf32_Phdr) < u_phnum) {
        throwCantUnpack("bad compressed e_phnum");
    }
#undef MAX_ELF_HDR

    // Packed ET_EXE has no PT_DYNAMIC.
    // Packed ET_DYN has original PT_DYNAMIC for info needed by rtld.
    Elf32_Phdr const *const dynhdr = elf_find_ptype(Elf32_Phdr::PT_DYNAMIC, phdri, c_phnum);
    bool const is_shlib = !!dynhdr;
    if (is_shlib) {
        // Unpack and output the Ehdr and Phdrs for real.
        // This depends on position within input file fi.
        unpackExtent(ph.u_len, fo,
            c_adler, u_adler, false, szb_info);

        // The first PT_LOAD.  Part is not compressed (for benefit of rtld.)
        fi->seek(0, SEEK_SET);
        fi->readx(ibuf, get_te32(&dynhdr->p_offset) + get_te32(&dynhdr->p_filesz));
        overlay_offset -= sizeof(linfo);
        xct_off = overlay_offset;
        e_shoff = get_te32(&ehdri.e_shoff);
        ibuf.subref("bad .e_shoff %#x for %#x", e_shoff, sizeof(Elf32_Shdr) * e_shnum);
        if (e_shoff && e_shnum) { // --android-shlib
            shdri = (Elf32_Shdr /*const*/ *)ibuf.subref(
                "bad Shdr table", e_shoff, sizeof(Elf32_Shdr)*e_shnum);
            unsigned xct_off2 = get_te32(&shdri->sh_offset);
            if (e_shoff == xct_off2) {
                xct_off = e_shoff;
            }
            // un-Relocate dynsym (DT_SYMTAB) which is below xct_off
            unsigned dyn_offset = get_te32(&dynhdr->p_offset);
            unsigned dyn_filesz = get_te32(&dynhdr->p_filesz);
            if (orig_file_size < dyn_offset
            || (orig_file_size - dyn_offset) < dyn_filesz) {
                throwCantUnpack("bad PT_DYNAMIC");
            }
            dynseg = (Elf32_Dyn const *)ibuf.subref("bad DYNAMIC", dyn_offset, dyn_filesz);
            dynstr = (char const *)elf_find_dynamic(Elf32_Dyn::DT_STRTAB);
            sec_dynsym = elf_find_section_type(Elf32_Shdr::SHT_DYNSYM);
            if (sec_dynsym) {
                unsigned const off_dynsym = get_te32(&sec_dynsym->sh_offset);
                unsigned const sz_dynsym  = get_te32(&sec_dynsym->sh_size);
                if (orig_file_size < sz_dynsym
                ||  orig_file_size < off_dynsym
                || (orig_file_size - off_dynsym) < sz_dynsym) {
                    throwCantUnpack("bad SHT_DYNSYM");
                }
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
                    if (Elf32_Sym::SHN_ABS == symsec && xct_off <= symval) {
                        adjABS(sym, 0u - asl_delta);
                    }
                }
            }
        }
        if (fo) {
            fo->write(ibuf + ph.u_len, xct_off - ph.u_len);
        }

        total_in  = xct_off;
        total_out = xct_off;
        ph.u_len = 0;
        // Position the input for next unpackExtent.
        fi->seek(sizeof(linfo) + overlay_offset + sizeof(hbuf) + szb_info + ph.c_len, SEEK_SET);

        // Decompress and unfilter the tail of first PT_LOAD.
        phdr = (Elf32_Phdr *) (void *) (1+ ehdr);
        for (unsigned j=0; j < u_phnum; ++phdr, ++j) {
            if (is_LOAD32(phdr)) {
                ph.u_len = get_te32(&phdr->p_filesz) - xct_off;
                break;
            }
        }
        unpackExtent(ph.u_len, fo,
            c_adler, u_adler, false, szb_info);
    }
    else {  // main executable
        // Decompress each PT_LOAD.
        bool first_PF_X = true;
        phdr = (Elf32_Phdr *) (void *) (1+ ehdr);  // uncompressed
        for (unsigned j=0; j < u_phnum; ++phdr, ++j) {
            if (is_LOAD32(phdr)) {
                unsigned const filesz = get_te32(&phdr->p_filesz);
                unsigned const offset = get_te32(&phdr->p_offset);
                if (fo)
                    fo->seek(offset, SEEK_SET);
                if (Elf32_Phdr::PF_X & get_te32(&phdr->p_flags)) {
                    unpackExtent(filesz, fo,
                        c_adler, u_adler, first_PF_X, szb_info);
                    first_PF_X = false;
                }
                else {
                    unpackExtent(filesz, fo,
                        c_adler, u_adler, false, szb_info);
                }
            }
        }
    }
    phdr = phdri;
    load_va = 0;
    for (unsigned j=0; j < c_phnum; ++j) {
        if (is_LOAD32(phdr)) {
            load_va = get_te32(&phdr->p_vaddr);
            break;
        }
    }
    if (0x1000==get_te32(&phdri[0].p_filesz)  // detect C_BASE style
    &&  0==get_te32(&phdri[1].p_offset)
    &&  0==get_te32(&phdri[0].p_offset)
    &&     get_te32(&phdri[1].p_filesz) == get_te32(&phdri[1].p_memsz)) {
        fi->seek(up4(get_te32(&phdr[1].p_memsz)), SEEK_SET);  // past the loader
    }
    else if (is_shlib
    ||  ((unsigned)(get_te32(&ehdri.e_entry) - load_va) + up4(lsize) +
                ph.getPackHeaderSize() + sizeof(overlay_offset))
            < up4(file_size)) {
        // Loader is not at end; skip past it.
        funpad4(fi);  // MATCH01
        unsigned d_info[4]; fi->readx(d_info, sizeof(d_info));
        if (0==old_dtinit) {
            old_dtinit = get_te32(&d_info[2 + (0==d_info[0])]);
            is_asl = 1u& get_te32(&d_info[0 + (0==d_info[0])]);
        }
        fi->seek(lsize - sizeof(d_info), SEEK_CUR);
    }

    // The gaps between PT_LOAD and after last PT_LOAD
    phdr = (Elf32_Phdr *)&u[sizeof(*ehdr)];
    unsigned hi_offset(0);
    for (unsigned j = 0; j < u_phnum; ++j) {
        unsigned offset = get_te32(&phdr[j].p_offset);
        if (is_LOAD32(&phdr[j])
        &&  hi_offset < offset)
            hi_offset = offset;
    }
    for (unsigned j = 0; j < u_phnum; ++j) {
        unsigned const size = find_LOAD_gap(phdr, j, u_phnum);
        if (size) {
            unsigned const offset = get_te32(&phdr[j].p_offset);
            unsigned const where =  get_te32(&phdr[j].p_filesz) + offset;
            if (fo)
                fo->seek(where, SEEK_SET);
            unpackExtent(size, fo,
                c_adler, u_adler, false, szb_info,
                is_shlib && (offset != hi_offset));
                // FIXME: should not depend on is_shlib ?
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
            if (is_LOAD32(phdr) && 0!=n_ptload++) {
                load_off = get_te32(&phdr->p_offset);
                load_va  = get_te32(&phdr->p_vaddr);
                fi->seek(old_data_off, SEEK_SET);
                fi->readx(ibuf, old_data_len);
                total_in  += old_data_len;
                total_out += old_data_len;

                Elf32_Phdr const *udynhdr = (Elf32_Phdr *)&u[sizeof(*ehdr)];
                for (unsigned j3= 0; j3 < u_phnum; ++j3, ++udynhdr)
                if (Elf32_Phdr::PT_DYNAMIC==get_te32(&udynhdr->p_type)) {
                    unsigned dt_pltrelsz(0), dt_jmprel(0);
                    unsigned dt_relsz(0), dt_rel(0);
                    unsigned const dyn_len = get_te32(&udynhdr->p_filesz);
                    unsigned const dyn_off = get_te32(&udynhdr->p_offset);
                    if ((unsigned long)file_size < (dyn_len + dyn_off)) {
                        char msg[50]; snprintf(msg, sizeof(msg),
                                "bad PT_DYNAMIC .p_filesz %#x", dyn_len);
                        throwCantUnpack(msg);
                    }
                    if (dyn_off < load_off) {
                        continue;  // Oops.  Not really is_shlib ?  [built by 'rust' ?]
                    }
                    Elf32_Dyn *dyn = (Elf32_Dyn *)((unsigned char *)ibuf +
                        (dyn_off - load_off));
                    dynseg = dyn; invert_pt_dynamic(dynseg,
                        umin(dyn_len, file_size - dyn_off));
                    for (unsigned j2= 0; j2 < dyn_len; ++dyn, j2 += sizeof(*dyn)) {
                        unsigned const tag = get_te32(&dyn->d_tag);
                        unsigned       val = get_te32(&dyn->d_val);
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
                            set_te32(&dyn->d_val, val -= asl_delta);
                        }; break;
                        } // end switch()
                        if (upx_dt_init == tag) {
                            if (Elf32_Dyn::DT_INIT == tag) {
                                set_te32(&dyn->d_val, old_dtinit);
                                if (!old_dtinit) { // compressor took the slot
                                    dyn->d_tag = Elf32_Dyn::DT_NULL;
                                    dyn->d_val = 0;
                                }
                            }
                            else if (Elf32_Dyn::DT_INIT_ARRAY    == tag
                            ||       Elf32_Dyn::DT_PREINIT_ARRAY == tag) {
                                if (val < load_va || (unsigned)file_size < (unsigned)val) {
                                    char msg[50]; snprintf(msg, sizeof(msg),
                                            "Bad Dynamic tag %#x %#x",
                                            (unsigned)tag, (unsigned)val);
                                    throwCantUnpack(msg);
                                }
                                set_te32(&ibuf[val - load_va], old_dtinit
                                    + (is_asl ? asl_delta : 0));  // counter-act unRel32
                            }
                        }
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
    if (fo && total_out != orig_file_size)
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
