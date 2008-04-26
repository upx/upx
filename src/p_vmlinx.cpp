/* p_vmlinx.cpp -- pack vmlinux ET_EXEC file (before bootsect or setup)

   This file is part of the UPX executable compressor.

   Copyright (C) 2004-2008 John Reiser
   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
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

   John Reiser
   <jreiser@users.sourceforge.net>
 */


#include "conf.h"

#include "file.h"
#include "filter.h"
#include "packer.h"
#include "p_vmlinx.h"
#include "linker.h"

static const
#include "stub/i386-linux.kernel.vmlinux.h"
static const
#include "stub/amd64-linux.kernel.vmlinux.h"
static const
#include "stub/arm-linux.kernel.vmlinux.h"
static const
#include "stub/armeb-linux.kernel.vmlinux.h"
static const
#include "stub/powerpc-linux.kernel.vmlinux.h"


/*************************************************************************
//
**************************************************************************/

template <class T>
PackVmlinuxBase<T>::PackVmlinuxBase(InputFile *f,
                                    unsigned e_machine, unsigned elfclass, unsigned elfdata,
                                    char const *const boot_label) :
    super(f),
    my_e_machine(e_machine), my_elfclass(elfclass), my_elfdata(elfdata),
    my_boot_label(boot_label),
    n_ptload(0), phdri(NULL), shdri(NULL), shstrtab(NULL)
{
    ElfClass::compileTimeAssertions();
    bele = N_BELE_CTP::getRTP<BeLePolicy>();
}

template <class T>
PackVmlinuxBase<T>::~PackVmlinuxBase()
{
    delete [] phdri;
    delete [] shdri;
    delete [] shstrtab;
}

template <class T>
int PackVmlinuxBase<T>::getStrategy(Filter &/*ft*/)
{
    // Called just before reading and compressing each block.
    // Might want to adjust blocksize, etc.

    // If user specified the filter, then use it (-2==strategy).
    // Else try the first two filters, and pick the better (2==strategy).
    return (opt->no_filter ? -3 : ((opt->filter > 0) ? -2 : 2));
}

template <class T>
int __acc_cdecl_qsort
PackVmlinuxBase<T>::compare_Phdr(void const *aa, void const *bb)
{
    Phdr const *const a = (Phdr const *)aa;
    Phdr const *const b = (Phdr const *)bb;
    unsigned const xa = a->p_type - Phdr::PT_LOAD;
    unsigned const xb = b->p_type - Phdr::PT_LOAD;
            if (xa < xb)         return -1;  // PT_LOAD first
            if (xa > xb)         return  1;
    if (a->p_paddr < b->p_paddr) return -1;  // ascending by .p_paddr
    if (a->p_paddr > b->p_paddr) return  1;
                                 return  0;
}

template <class T>
typename T::Shdr const *PackVmlinuxBase<T>::getElfSections()
{
    Shdr const *p, *shstrsec=0;
    shdri = new Shdr[(unsigned) ehdri.e_shnum];
    fi->seek(ehdri.e_shoff, SEEK_SET);
    fi->readx(shdri, ehdri.e_shnum * sizeof(*shdri));
    int j;
    for (p = shdri, j= ehdri.e_shnum; --j>=0; ++p) {
        if (Shdr::SHT_STRTAB==p->sh_type
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

template <class T>
bool PackVmlinuxBase<T>::canPack()
{
    fi->seek(0, SEEK_SET);
    fi->readx(&ehdri, sizeof(ehdri));

    // now check the ELF header
    if (memcmp(&ehdri, "\x7f\x45\x4c\x46", 4)
    ||  ehdri.e_ident[Ehdr::EI_CLASS] != my_elfclass
    ||  ehdri.e_ident[Ehdr::EI_DATA] != my_elfdata
    ||  ehdri.e_ident[Ehdr::EI_VERSION] != Ehdr::EV_CURRENT
    ||  !memcmp(&ehdri.e_ident[8], "FreeBSD", 7)  // branded
    ||  ehdri.e_machine != my_e_machine
    ||  ehdri.e_version != 1  // version
    ||  ehdri.e_ehsize != sizeof(ehdri)  // different <elf.h> ?
    ) {
        return false;
    }

    // additional requirements for vmlinux
    if (ehdri.e_type != Ehdr::ET_EXEC
    ||  ehdri.e_phoff != sizeof(ehdri)  // Phdr not contiguous with Ehdr
    ||  ehdri.e_phentsize!=sizeof(Phdr)
    ||  !is_valid_e_entry(ehdri.e_entry)
    ) {
        return false;
    }

    // A Linux kernel must have a __ksymtab section. [??]
    Shdr const *p, *const shstrsec = getElfSections();
    if (0==shstrsec) {
        return false;
    }
    {
        int j;
        for (p = shdri, j= ehdri.e_shnum; --j>=0; ++p) {
            if (Shdr::SHT_PROGBITS==p->sh_type
            && 0==strcmp("__ksymtab", p->sh_name + shstrtab)) {
                break;
            }
        }
        if (j < 0) {
            return false;
        }
    }

    phdri = new Phdr[(unsigned) ehdri.e_phnum];
    fi->seek(ehdri.e_phoff, SEEK_SET);
    fi->readx(phdri, ehdri.e_phnum * sizeof(*phdri));

    // Put PT_LOAD together at the beginning, ascending by .p_paddr.
    qsort(phdri, ehdri.e_phnum, sizeof(*phdri), compare_Phdr);

    // Find convex hull of physical addresses, and count the PT_LOAD.
    // Ignore ".bss": .p_filesz < .p_memsz
    unsigned phys_lo= ~0u, phys_hi= 0u;
    for (unsigned j = 0; j < ehdri.e_phnum; ++j) {
        if (Phdr::PT_LOAD==phdri[j].p_type) {
            // Check for general sanity (not necessarily required.)
            if (0xfff & (phdri[j].p_offset | phdri[j].p_paddr
                       | phdri[j].p_align  | phdri[j].p_vaddr) ) {
                return false;
            }
            if (phys_lo > phdri[j].p_paddr) {
                phys_lo = phdri[j].p_paddr;
            }
            if (phys_hi < (phdri[j].p_filesz + phdri[j].p_paddr)) {
                phys_hi = (phdri[j].p_filesz + phdri[j].p_paddr);
            }
            ++n_ptload;
        }
    }
    paddr_min = phys_lo;
    sz_ptload = phys_hi - phys_lo;
    return 0 < n_ptload;
}

#include "p_elf.h"

template <class T>
void PackVmlinuxBase<T>::pack(OutputFile *fo)
{
    unsigned fo_off = 0;
    Ehdr ehdro;
    TE32 tmp_u32;

    // NULL
    // .text(PT_LOADs) .note(1st page) .note(rest)
    // .shstrtab .symtab .strtab
    Shdr shdro[1+3+3];
    memset(shdro, 0, sizeof(shdro));

    ibuf.alloc(file_size);
    obuf.allocForCompression(file_size);

    // .e_ident, .e_machine, .e_version, .e_flags
    memcpy(&ehdro, &ehdri, sizeof(ehdro));
    ehdro.e_type = Ehdr::ET_REL;
    ehdro.e_entry = 0;
    ehdro.e_phoff = 0;
    ehdro.e_shoff = sizeof(ehdro);
    ehdro.e_phentsize = 0;
    ehdro.e_phnum = 0;
    ehdro.e_shnum = 1+3+3;
    ehdro.e_shstrndx = 4;
    fo->write(&ehdro, sizeof(ehdro)); fo_off+= sizeof(ehdro);
    fo->write(shdro, sizeof(shdro)); fo_off+= sizeof(shdro);

// Notice overlap [containment] of physical PT_LOAD[2] into PTLOAD[1]
// in this vmlinux for x86_64 from Fedora Core 6 on 2007-01-07:
//Program Headers:
//  Type           Offset             VirtAddr           PhysAddr
//                 FileSiz            MemSiz              Flags  Align
//  LOAD           0x0000000000200000 0xffffffff80200000 0x0000000000200000
//                 0x000000000034bce8 0x000000000034bce8  R E    200000
//  LOAD           0x000000000054c000 0xffffffff8054c000 0x000000000054c000
//                 0x00000000000ed004 0x00000000001702a4  RWE    200000
//  LOAD           0x0000000000800000 0xffffffffff600000 0x00000000005f5000
//                 0x0000000000000c08 0x0000000000000c08  RWE    200000
//  NOTE           0x0000000000000000 0x0000000000000000 0x0000000000000000
//                 0x0000000000000000 0x0000000000000000  R      8
// Therefore we must "compose" the convex hull to be loaded.

    ph.u_len = sz_ptload;
    memset(ibuf, 0, sz_ptload);
    for (unsigned j = 0; j < ehdri.e_phnum; ++j) {
        if (Phdr::PT_LOAD==phdri[j].p_type) {
            fi->seek(phdri[j].p_offset, SEEK_SET);
            fi->readx(ibuf + ((unsigned) phdri[j].p_paddr - paddr_min), phdri[j].p_filesz);
        }
    }
    checkAlreadyPacked(ibuf + ph.u_len - 1024, 1024);

    // prepare filter
    ph.filter = 0;
    Filter ft(ph.level);
    ft.buf_len = ph.u_len;
    ft.addvalue = 0;  // we are independent of actual runtime address; see ckt32

    upx_compress_config_t cconf; cconf.reset();
    // limit stack size needed for runtime decompression
    cconf.conf_lzma.max_num_probs = 1846 + (768 << 4); // ushort: ~28 KiB stack

    unsigned ppc32_extra = 0;
    if (Ehdr::EM_PPC==my_e_machine) {
        // output layout:
        //      .long UPX_MAGIC_LE32
        //      .long L20 - L10
        // L10:
        //      b_info for Ehdr; compressed Ehdr; .balign 4  // one block only
        //      b_info for LOAD; compressed LOAD; .balign 4  // possibly many blocks
        //          // This allows per-block filters!
        // L20:
        //      b f_decompress
        // +4:  f_unfilter(char *buf, unsigned len, unsigned cto8, unsigned ftid)
        //          // Code for multiple filters can "daisy chain" on ftid.
        //      f_decompress(char const *src, unsigned  src_len,
        //                   char       *dst, unsigned *dst_len, int method)
        unsigned tmp;
        tmp = UPX_MAGIC_LE32; fo->write(&tmp, sizeof(tmp)); fo_off += sizeof(tmp);
        tmp = 0;              fo->write(&tmp, sizeof(tmp)); fo_off += sizeof(tmp);
        ppc32_extra += 2*sizeof(tmp);
        unsigned const len_unc = sizeof(ehdri) + sizeof(Phdr) * ehdri.e_phnum;
        MemBuffer unc_hdr(len_unc);
        MemBuffer cpr_hdr; cpr_hdr.allocForCompression(len_unc);
        memcpy(&unc_hdr[0],             &ehdri, sizeof(ehdri));
        memcpy(&unc_hdr[sizeof(ehdri)],  phdri, sizeof(Phdr) * ehdri.e_phnum);
        unsigned len_cpr = 0;
        int const r = upx_compress(unc_hdr, len_unc, cpr_hdr, &len_cpr,
            NULL, ph.method, 10, NULL, NULL );
        if (UPX_E_OK!=r || len_unc<=len_cpr)  // FIXME: allow no compression
            throwInternalError("Ehdr compression failed");

        struct b_info { // 12-byte header before each compressed block
            unsigned sz_unc;  // uncompressed_size
            unsigned sz_cpr;  //   compressed_size
            unsigned char b_method;  // compression algorithm
            unsigned char b_ftid;  // filter id
            unsigned char b_cto8;  // filter parameter
            unsigned char b_unused;  // FIXME: !=0  for partial-block unfilter
            // unsigned f_offset, f_len;  // only if    partial-block unfilter
        }
        __attribute_packed;

        struct b_info hdr_info;
        set_be32(&hdr_info.sz_unc, len_unc);
        set_be32(&hdr_info.sz_cpr, len_cpr);
        hdr_info.b_method = ph.method;
        hdr_info.b_ftid = 0;
        hdr_info.b_cto8 = 0;
        hdr_info.b_unused = 0;
        fo->write(&hdr_info, sizeof(hdr_info)); fo_off += sizeof(hdr_info);
        unsigned const frag = (3& (0u-len_cpr));
        ppc32_extra += sizeof(hdr_info) + len_cpr + frag;
        fo_off += len_cpr + frag;
        fo->write(cpr_hdr, len_cpr + frag);

        // Partial filter: .text and following contiguous SHF_EXECINSTR
        upx_bytep f_ptr = ibuf;
        unsigned  f_len = 0;
        Shdr const *shdr = 1+ shdri;  // skip empty shdr[0]
        if (ft.buf_len==0  // not specified yet  FIXME: was set near construction
        && (Shdr::SHF_ALLOC     & shdr->sh_flags)
        && (Shdr::SHF_EXECINSTR & shdr->sh_flags)) {
            // shdr[1] is instructions (probably .text)
            f_ptr = ibuf + (unsigned) (shdr->sh_offset - phdri[0].p_offset);
            f_len = shdr->sh_size;
            ++shdr;
            for (int j= ehdri.e_shnum - 2; --j>=0; ++shdr) {
                unsigned prev_end = shdr[-1].sh_size + shdr[-1].sh_offset;
                prev_end += ~(0u-shdr[0].sh_addralign) & (0u-prev_end);  // align_up
                if ((Shdr::SHF_ALLOC     & shdr->sh_flags)
                &&  (Shdr::SHF_EXECINSTR & shdr->sh_flags)
                &&  shdr[0].sh_offset==prev_end) {
                    f_len += shdr->sh_size;
                }
                else {
                    break;
                }
            }
        }
        else { // ft.buf_len already specified, or Shdr[1] not instructions
            f_ptr = ibuf;
            f_len = ph.u_len;
        }
        compressWithFilters(ibuf, ph.u_len, obuf,
            f_ptr, f_len,  // filter range
            NULL, 0,  // hdr_ptr, hdr_len
            &ft, 512, &cconf, getStrategy(ft));

        set_be32(&hdr_info.sz_unc, ph.u_len);
        set_be32(&hdr_info.sz_cpr, ph.c_len);
        hdr_info.b_ftid = ft.id;
        hdr_info.b_cto8 = ft.cto;
        if (ph.u_len!=f_len) {
            hdr_info.b_unused = 1;  // flag for partial filter
        }
        fo->write(&hdr_info, sizeof(hdr_info)); fo_off += sizeof(hdr_info);
        ppc32_extra += sizeof(hdr_info);
        if (ph.u_len!=f_len) {
            set_be32(&hdr_info.sz_unc, f_ptr - (upx_bytep) ibuf);
            set_be32(&hdr_info.sz_cpr, f_len);
            fo->write(&hdr_info, 2*sizeof(unsigned)); fo_off += 2*sizeof(unsigned);
            ppc32_extra += 2*sizeof(unsigned);
        }

#if 0  /*{ Documentation: changes to arch/powerpc/boot/main.c */
struct b_info {
    unsigned sz_unc;
    unsigned sz_cpr;
    unsigned char b_method;
    unsigned char b_ftid;
    unsigned char b_cto8;
    unsigned char b_unused;
};
typedef int (*upx_f_unc)(  /* uncompress */
    unsigned char const *src, unsigned  src_len,
    unsigned char       *dst, unsigned *dst_len,
    int method
);
typedef int (*upx_f_unf)(  /* unfilter */
    unsigned char *buf,
    unsigned len,
    unsigned cto8,
    unsigned ftid
);

unsigned char const *upx_expand(  // return updated src [aligned, too]
    unsigned char const *src,
    unsigned char       *dst,
    unsigned *const dst_len,
    upx_f_unc const f_unc
)
{
    if (0==*dst_len) {
        *dst_len = ((struct b_info const *)src)->sz_unc;
    }
    upx_f_unf const f_unf = (upx_f_unf)(sizeof(int) + (char *)f_unc);
    unsigned total_len = 0;
    unsigned need = *dst_len;
    while (0 < need) {
        struct b_info const *const b_hdr = (struct b_info const *)src;
        src = (unsigned char const *)(1+ b_hdr);
        unsigned blk_len = b_hdr->sz_unc;
        int const rv = (*f_unc)(src, b_hdr->sz_cpr, dst, &blk_len, b_hdr->b_method);
        if (0!=rv) {
            printf("decompress error %d\n\r", rv);
            exit();
        }
        if (b_hdr->b_ftid) {
            (*f_unf)(dst, b_hdr->sz_unc, b_hdr->b_cto8, b_hdr->b_ftid);
        }
        src += b_hdr->sz_cpr;
        src += (3& -(int)src);  // 4-byte align
        dst += b_hdr->sz_unc;
        total_len += b_hdr->sz_unc;
        need -= b_hdr->sz_unc;
    }
    *dst_len = total_len;
    return src;
}

void prep_kernel(unsigned long a1, unsigned long a2)
{
    upx_f_unc f_unc = (upx_f_unc)0;

    vmlinuz.addr = (unsigned long)_vmlinux_start;
    vmlinuz.size = (unsigned long)(_vmlinux_end - _vmlinux_start);

    if (0x55505821 == *(unsigned *)vmlinuz.addr) {
        /* was compressed by upx */
        /* Uncompress elfheader */
        unsigned const *const fwa = (unsigned *)vmlinuz.addr;
        f_unc = (upx_f_unc)((char *)&fwa[2] + fwa[1]);
        unsigned dst_len = 0;
        unsigned char const *src = (unsigned char const *)&fwa[2];
        src = upx_expand(src, (unsigned char *)elfheader, &dst_len, f_unc);
        unsigned const src_len = src - (unsigned char const *)&fwa[0];
        vmlinuz.addr += src_len;
        vmlinuz.size -= src_len;
    } else {
        memcpy(elfheader, (const void *)vmlinuz.addr,
               sizeof(elfheader));
    }
=====
    /* Eventually gunzip the kernel */
    if (f_unc) {
        /* was compressed by upx */
        printf("upx_expand (0x%lx <- 0x%lx:0x%0lx)...",
               vmlinux.addr, vmlinuz.addr, vmlinuz.addr+vmlinuz.size);
        unsigned dst_len = vmlinux.size - elfoffset;
        upx_expand((unsigned char const *)vmlinuz.addr,
                   (unsigned char       *)vmlinux.addr, &dst_len, f_unc);
        printf("done 0x%lx bytes\n\r", len);
    } else {
        memmove((void *)vmlinux.addr,(void *)vmlinuz.addr,
            vmlinuz.size);
        /* Skip over the ELF header */
#ifdef DEBUG
        printf("... skipping 0x%lx bytes of ELF header\n\r",
                elfoffset);
#endif
        vmlinux.addr += elfoffset;
    }
=====
}
#endif  /*}*/

    }
    else {
        compressWithFilters(&ft, 512, &cconf, getStrategy(ft));
    }
    unsigned const txt_c_len = ph.c_len;

    const unsigned lsize = getLoaderSize();

    defineDecompressorSymbols();
    defineFilterSymbols(&ft);
    relocateLoader();

    MemBuffer loader(lsize);
    memcpy(loader, getLoader(), lsize);
    patchPackHeader(loader, lsize);

#define shstrtab local_shstrtab // avoid -Wshadow warning
    char const shstrtab[]= "\0.text\0.note\0.shstrtab\0.symtab\0.strtab";
    char const *p = shstrtab;
    while (0!=*p++) ;
    shdro[1].sh_name = ptr_diff(p, shstrtab);
    shdro[1].sh_type = Shdr::SHT_PROGBITS;
    shdro[1].sh_flags = Shdr::SHF_ALLOC | Shdr::SHF_EXECINSTR;
    shdro[1].sh_offset = fo_off - ppc32_extra;
    shdro[1].sh_size = ppc32_extra + txt_c_len + lsize;  // plus more ...
    shdro[1].sh_addralign = 1;  // default

    fo_off += write_vmlinux_head(fo, &shdro[1]);
    fo->write(obuf, txt_c_len); fo_off += txt_c_len;
    unsigned const a = (shdro[1].sh_addralign -1) & (0u-(ppc32_extra + txt_c_len));
    if (0!=a) { // align
        fo_off += a;
        shdro[1].sh_size += a;
        fo->seek(a, SEEK_CUR);
    }
    fo->write(loader, lsize); fo_off += lsize;

#if 0
    printf("%-13s: compressed   : %8u bytes\n", getName(), txt_c_len);
    printf("%-13s: decompressor : %8u bytes\n", getName(), lsize);
#endif
    verifyOverlappingDecompression();

    // .note with 1st page --------------------------------
    ph.u_len = phdri[0].p_offset;
    fi->seek(0, SEEK_SET);
    fi->readx(ibuf, ph.u_len);
    compress(ibuf, ph.u_len, obuf, &cconf);

    while (0!=*p++) ;
    shdro[2].sh_name = ptr_diff(p, shstrtab);
    shdro[2].sh_type = Shdr::SHT_NOTE;
    shdro[2].sh_offset = fo_off;
    shdro[2].sh_size = sizeof(ph.u_len) + ph.c_len;
    shdro[2].sh_addralign = 1;
    tmp_u32 = ph.u_len; fo->write(&tmp_u32, 4);
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
    compress(ibuf, ph.u_len, obuf, &cconf);
    ph.level = old_level;

    // while (0!=*p++) ;  // name is the same
    shdro[3].sh_name = ptr_diff(p, shstrtab);
    shdro[3].sh_type = Shdr::SHT_NOTE;
    shdro[3].sh_offset = fo_off;
    shdro[3].sh_size = sizeof(ph.u_len) + ph.c_len;
    shdro[3].sh_addralign = 1;
    tmp_u32 = ph.u_len; fo->write(&tmp_u32, 4);
    fo->write(obuf, ph.c_len); fo_off += shdro[3].sh_size;

    while (0!=*p++) ;
    shdro[4].sh_name = ptr_diff(p, shstrtab);
    shdro[4].sh_type = Shdr::SHT_STRTAB;
    shdro[4].sh_offset = fo_off;
    shdro[4].sh_size = sizeof(shstrtab);  // already includes terminating '\0'
    shdro[4].sh_addralign = 1;
    fo->write(shstrtab, shdro[4].sh_size); fo_off += shdro[4].sh_size;

    fo_off = ~3 & (3+ fo_off); fo->seek(fo_off, SEEK_SET);
    while (0!=*p++) ;
    shdro[5].sh_name = ptr_diff(p, shstrtab);
    shdro[5].sh_type = Shdr::SHT_SYMTAB;
    shdro[5].sh_offset = fo_off;
    shdro[5].sh_size = ((Ehdr::EM_PPC==my_e_machine) + 5)*sizeof(Sym);
    //shdro[5].sh_flags = Shdr::SHF_INFO_LINK;
    shdro[5].sh_link = 6;  // to .strtab for symbols
    shdro[5].sh_info = 1+3;  // number of non-global symbols [binutils/bfd/elf.c]
    shdro[5].sh_addralign = 4;
    shdro[5].sh_entsize = sizeof(Sym);

    Sym sec_sym;

    // Symbol 0; no references, but bfd demands it.
    memset(&sec_sym, 0, sizeof(sec_sym));
    fo->write(&sec_sym, sizeof(sec_sym)); fo_off += sizeof(sec_sym);

    // Each section before .shstrtab needs a symbol.
    sec_sym.st_info = sec_sym.make_st_info(Sym::STB_LOCAL, Sym::STT_SECTION);
    sec_sym.st_other = Sym::STV_DEFAULT;
    sec_sym.st_shndx = 1;  // .text
    fo->write(&sec_sym, sizeof(sec_sym)); fo_off += sizeof(sec_sym);
    sec_sym.st_shndx = 2;  // .note
    fo->write(&sec_sym, sizeof(sec_sym)); fo_off += sizeof(sec_sym);
    sec_sym.st_shndx = 3;  // .note
    fo->write(&sec_sym, sizeof(sec_sym)); fo_off += sizeof(sec_sym);

    // the symbol we care about
    Sym unc_ker;
    unc_ker.st_name = 1;  // 1 byte into strtab
    unc_ker.st_value = 0;
    unc_ker.st_size = ppc32_extra + txt_c_len;
    unc_ker.st_info = unc_ker.make_st_info(Sym::STB_GLOBAL, Sym::STT_FUNC);
    unc_ker.st_other = Sym::STV_DEFAULT;
    unc_ker.st_shndx = 1;  // .text
    fo->write(&unc_ker, sizeof(unc_ker)); fo_off += sizeof(unc_ker);

    unsigned const lablen = strlen(my_boot_label);
    if (Ehdr::EM_PPC==my_e_machine) {
        unc_ker.st_name += 1+ lablen;
        unc_ker.st_value = unc_ker.st_size;
        unc_ker.st_size = 0;
        fo->write(&unc_ker, sizeof(unc_ker)); fo_off += sizeof(unc_ker);
    }
    while (0!=*p++) ;
    shdro[6].sh_name = ptr_diff(p, shstrtab);
    shdro[6].sh_type = Shdr::SHT_STRTAB;
    shdro[6].sh_offset = fo_off;
    shdro[6].sh_size = 2+ lablen + (Ehdr::EM_PPC==my_e_machine)*(1+ 12);  // '\0' before and after
    shdro[6].sh_addralign = 1;
    fo->seek(1, SEEK_CUR);  // the '\0' before
    fo->write(my_boot_label, 1+ lablen);  // include the '\0' terminator
    if (Ehdr::EM_PPC==my_e_machine) {
        fo->write("_vmlinux_end", 1+ 12); fo_off += 1+ 12;
    }
    fo_off += 2+ lablen;

    fo->seek(0, SEEK_SET);
    fo->write(&ehdro, sizeof(ehdro));
    fo->write(&shdro, sizeof(shdro));
    if (Ehdr::EM_PPC==my_e_machine) {
        fo->seek(sizeof(unsigned), SEEK_CUR);
        set_be32(&ppc32_extra, ppc32_extra - 2*sizeof(unsigned) + txt_c_len);
        fo->write(&ppc32_extra, sizeof(ppc32_extra));
    }

    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();
#undef shstrtab
}

template <class T>
int PackVmlinuxBase<T>::canUnpack()
{
    fi->seek(0, SEEK_SET);
    fi->readx(&ehdri, sizeof(ehdri));

    // now check the ELF header
    if (memcmp(&ehdri, "\x7f\x45\x4c\x46", 4)
    ||  ehdri.e_ident[Ehdr::EI_CLASS] != my_elfclass
    ||  ehdri.e_ident[Ehdr::EI_DATA] != my_elfdata
    ||  ehdri.e_ident[Ehdr::EI_VERSION] != Ehdr::EV_CURRENT
    ||  !memcmp(&ehdri.e_ident[8], "FreeBSD", 7)  // branded
    ||  ehdri.e_machine != my_e_machine
    ||  ehdri.e_version != 1  // version
    ||  ehdri.e_ehsize != sizeof(ehdri)  // different <elf.h> ?
    )
        return false;

    if (ehdri.e_type != Ehdr::ET_REL
    //i386 fails  ||  ehdri.e_shoff != sizeof(ehdri)  // Shdr not contiguous with Ehdr
    ||  ehdri.e_shentsize!=sizeof(Shdr)
    ||  ehdri.e_shnum < 4
    ||  (unsigned)file_size < (ehdri.e_shnum * sizeof(Shdr) + ehdri.e_shoff)
    )
        return false;

    // find the .shstrtab section
    Shdr const *const shstrsec = getElfSections();
    if (0==shstrsec) {
        return false;
    }

    // check for .text .note .note  and sane (.sh_size + .sh_offset)
    p_note0 = p_note1 = p_text = 0;
    int j;
    Shdr *p;
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
        return -1; // format is known, but definitely is not packed

    return true;
}

template <class T>
void PackVmlinuxBase<T>::unpack(OutputFile *fo)
{
    TE32 word;
    PackHeader const ph_tmp(ph);

    fi->seek(p_note0->sh_offset, SEEK_SET);
    fi->readx(&word, sizeof(word));
    ph.u_len = word;
    ph.c_len = p_note0->sh_size - sizeof(word);
    ibuf.alloc(ph.c_len);
    fi->readx(ibuf, ph.c_len);
    obuf.allocForUncompression(ph.u_len);
    decompress(ibuf, obuf, false);
    fo->write(obuf, ph.u_len);
    obuf.dealloc();
    ibuf.dealloc();

    ph = ph_tmp;
    if (!has_valid_vmlinux_head()) {
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
    fi->readx(&word, sizeof(word));
    ph.u_len = word;
    ph.c_len = p_note1->sh_size - sizeof(word);
    ibuf.alloc(ph.c_len);
    fi->readx(ibuf, p_note1->sh_size - sizeof(ph.u_len));
    obuf.allocForUncompression(ph.u_len);
    decompress(ibuf, obuf, false);
    fo->write(obuf, ph.u_len);
    obuf.dealloc();
    ibuf.dealloc();

    ph = ph_tmp;
}


/*************************************************************************
//
**************************************************************************/

const int *PackVmlinuxI386::getCompressionMethods(int method, int level) const
{
    return Packer::getDefaultCompressionMethods_le32(method, level);
}


const int *PackVmlinuxI386::getFilters() const
{
    static const int filters[] = {
        0x49, 0x46,
    FT_END };
    return filters;
}

const int *PackVmlinuxARMEL::getCompressionMethods(int method, int level) const
{
    return Packer::getDefaultCompressionMethods_8(method, level);
}

const int *PackVmlinuxARMEB::getCompressionMethods(int method, int level) const
{
    return Packer::getDefaultCompressionMethods_8(method, level);
}

const int *PackVmlinuxPPC32::getCompressionMethods(int method, int level) const
{
    // No real dependency on LE32.
    return Packer::getDefaultCompressionMethods_le32(method, level);
}


const int *PackVmlinuxARMEL::getFilters() const
{
    static const int f50[] = { 0x50, FT_END };
    return f50;
}

const int *PackVmlinuxARMEB::getFilters() const
{
    static const int f51[] = { 0x51, FT_END };
    return f51;
}

const int *PackVmlinuxPPC32::getFilters() const
{
    static const int fd0[] = { 0xd0, FT_END };
    return fd0;
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

bool PackVmlinuxI386::is_valid_e_entry(Addr e_entry)
{
    return 0==(0x000fffff & e_entry); // entry on whole 1 MiB
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
    if (ft->id) {
        assert(ft->calls > 0);
        if (0x40==(0xf0 & ft->id)) {
            addLoader("LXCKLLT9", NULL);
        }
        else {
            addLoader("LXCALLT9", NULL);
        }
        addFilter32(ft->id);
    }
    addLoader("LINUX990",
              ((ph.first_offset_found == 1) ? "LINUX991" : ""),
              "LINUX992,IDENTSTR,UPX1HEAD", NULL);
}

void PackVmlinuxAMD64::buildLoader(const Filter *ft)
{
    // prepare loader
    initLoader(stub_amd64_linux_kernel_vmlinux, sizeof(stub_amd64_linux_kernel_vmlinux));
    addLoader("LINUX000",
              (0x40==(0xf0 & ft->id)) ? "LXCKLLT1" : (ft->id ? "LXCALLT1" : ""),
              "LXMOVEUP",
              getDecompressorSections(),
              NULL
             );
    if (ft->id) {
        assert(ft->calls > 0);
        if (0x40==(0xf0 & ft->id)) {
            addLoader("LXCKLLT9", NULL);
        }
        else {
            addLoader("LXCALLT9", NULL);
        }
        addFilter32(ft->id);
    }
    addLoader("LINUX990",
              ((ph.first_offset_found == 1) ? "LINUX991" : ""),
              "LINUX992,IDENTSTR,UPX1HEAD", NULL);
}

bool PackVmlinuxARMEL::is_valid_e_entry(Addr e_entry)
{
    return 0xc0008000==e_entry;
}

bool PackVmlinuxARMEB::is_valid_e_entry(Addr e_entry)
{
    return 0xc0008000==e_entry;
}

bool PackVmlinuxPPC32::is_valid_e_entry(Addr e_entry)
{
    return 0xc0000000==e_entry;
}

Linker* PackVmlinuxARMEL::newLinker() const
{
    return new ElfLinkerArmLE;
}

Linker* PackVmlinuxARMEB::newLinker() const
{
    return new ElfLinkerArmBE;
}

Linker* PackVmlinuxPPC32::newLinker() const
{
    return new ElfLinkerPpc32;
}



void PackVmlinuxARMEL::buildLoader(const Filter *ft)
{
    // prepare loader
    initLoader(stub_arm_linux_kernel_vmlinux, sizeof(stub_arm_linux_kernel_vmlinux));
    addLoader("LINUX000", NULL);
    if (ft->id) {
        assert(ft->calls > 0);
        addLoader("LINUX010", NULL);
    }
    addLoader("LINUX020", NULL);
    if (ft->id) {
        addFilter32(ft->id);
    }
    addLoader("LINUX030", NULL);
         if (ph.method == M_NRV2E_8) addLoader("NRV2E", NULL);
    else if (ph.method == M_NRV2B_8) addLoader("NRV2B", NULL);
    else if (ph.method == M_NRV2D_8) addLoader("NRV2D", NULL);
    else if (M_IS_LZMA(ph.method))   addLoader("LZMA_ELF00,LZMA_DEC10,LZMA_DEC30", NULL);
    else throwBadLoader();
    addLoader("IDENTSTR,UPX1HEAD", NULL);
}

void PackVmlinuxARMEB::buildLoader(const Filter *ft)
{
    // prepare loader
    initLoader(stub_armeb_linux_kernel_vmlinux, sizeof(stub_armeb_linux_kernel_vmlinux));
    addLoader("LINUX000", NULL);
    if (ft->id) {
        assert(ft->calls > 0);
        addLoader("LINUX010", NULL);
    }
    addLoader("LINUX020", NULL);
    if (ft->id) {
        addFilter32(ft->id);
    }
    addLoader("LINUX030", NULL);
         if (ph.method == M_NRV2E_8) addLoader("NRV2E", NULL);
    else if (ph.method == M_NRV2B_8) addLoader("NRV2B", NULL);
    else if (ph.method == M_NRV2D_8) addLoader("NRV2D", NULL);
    else if (M_IS_LZMA(ph.method))   addLoader("LZMA_ELF00,LZMA_DEC10,LZMA_DEC30", NULL);
    else throwBadLoader();
    addLoader("IDENTSTR,UPX1HEAD", NULL);
}

void PackVmlinuxPPC32::buildLoader(const Filter *ft)
{
    // prepare loader
    initLoader(stub_powerpc_linux_kernel_vmlinux, sizeof(stub_powerpc_linux_kernel_vmlinux));
    addLoader("LINUX000", NULL);
    if (ft->id) {
        assert(ft->calls > 0);
        addLoader("LINUX010", NULL);
    }
    addLoader("LINUX020", NULL);
    if (ft->id) {
        addFilter32(ft->id);
    }
    addLoader("LINUX030", NULL);
         if (ph.method == M_NRV2E_LE32) addLoader("NRV2E", NULL);
    else if (ph.method == M_NRV2B_LE32) addLoader("NRV2B", NULL);
    else if (ph.method == M_NRV2D_LE32) addLoader("NRV2D", NULL);
    else if (M_IS_LZMA(ph.method))   addLoader("LZMA_ELF00,LZMA_DEC10,LZMA_DEC30", NULL);
    else throwBadLoader();
    addLoader("IDENTSTR,UPX1HEAD", NULL);
}


static const
#include "stub/i386-linux.kernel.vmlinux-head.h"
static const
#include "stub/amd64-linux.kernel.vmlinux-head.h"
static const
#include "stub/arm-linux.kernel.vmlinux-head.h"
static const
#include "stub/armeb-linux.kernel.vmlinux-head.h"
static const
#include "stub/powerpc-linux.kernel.vmlinux-head.h"

unsigned PackVmlinuxI386::write_vmlinux_head(
    OutputFile *const fo,
    Shdr *const stxt
)
{
    // COMPRESSED_LENGTH
    fo->write(&stub_i386_linux_kernel_vmlinux_head[0],
        sizeof(stub_i386_linux_kernel_vmlinux_head)-(1+ 4) +1);
    TE32 tmp_u32; tmp_u32 = ph.c_len; fo->write(&tmp_u32, 4);

    stxt->sh_size += sizeof(stub_i386_linux_kernel_vmlinux_head);

    return sizeof(stub_i386_linux_kernel_vmlinux_head);
}

unsigned PackVmlinuxAMD64::write_vmlinux_head(
    OutputFile *const fo,
    Shdr *const stxt
)
{
    // COMPRESSED_LENGTH
    fo->write(&stub_amd64_linux_kernel_vmlinux_head[0],
        sizeof(stub_amd64_linux_kernel_vmlinux_head)-(1+ 4) +1);
    TE32 tmp_u32; tmp_u32 = ph.c_len; fo->write(&tmp_u32, 4);
printf("  Compressed length=0x%x\n", ph.c_len);
printf("UnCompressed length=0x%x\n", ph.u_len);

    stxt->sh_size += sizeof(stub_amd64_linux_kernel_vmlinux_head);

    return sizeof(stub_amd64_linux_kernel_vmlinux_head);
}

void PackVmlinuxARMEL::defineDecompressorSymbols()
{
    super::defineDecompressorSymbols();
    linker->defineSymbol(  "COMPRESSED_LENGTH", ph.c_len);
    linker->defineSymbol("UNCOMPRESSED_LENGTH", ph.u_len);
    linker->defineSymbol("METHOD", ph.method);
}

void PackVmlinuxARMEB::defineDecompressorSymbols()
{
    super::defineDecompressorSymbols();
    linker->defineSymbol(  "COMPRESSED_LENGTH", ph.c_len);
    linker->defineSymbol("UNCOMPRESSED_LENGTH", ph.u_len);
    linker->defineSymbol("METHOD", ph.method);
}

void PackVmlinuxPPC32::defineDecompressorSymbols()
{
    super::defineDecompressorSymbols();
    // linker->defineSymbol(  "COMPRESSED_LENGTH", ph.c_len);
    // linker->defineSymbol("UNCOMPRESSED_LENGTH", ph.u_len);
    // linker->defineSymbol("METHOD", ph.method);
}

void PackVmlinuxI386::defineDecompressorSymbols()
{
    super::defineDecompressorSymbols();
    linker->defineSymbol("ENTRY_POINT", phdri[0].p_paddr);
    linker->defineSymbol("PHYSICAL_START", phdri[0].p_paddr);
}

void PackVmlinuxAMD64::defineDecompressorSymbols()
{
    super::defineDecompressorSymbols();
    // We assume a 32-bit boot loader, so we use the 32-bit convention
    // of "enter at the beginning" (startup_32).  The 64-bit convention
    // would be to use ehdri.e_entry (startup_64).
    linker->defineSymbol("ENTRY_POINT", phdri[0].p_paddr);
    linker->defineSymbol("PHYSICAL_START", phdri[0].p_paddr);
}

unsigned PackVmlinuxARMEL::write_vmlinux_head(
    OutputFile *const fo,
    Shdr *const stxt
)
{
    // First word from vmlinux-head.S
    fo->write(&stub_arm_linux_kernel_vmlinux_head[0], 4);

    // Second word
    TE32 tmp_u32;
    unsigned const t = (0xff000000 &
            BeLePolicy::get32(&stub_arm_linux_kernel_vmlinux_head[4]))
        | (0x00ffffff & (0u - 1 + ((3+ ph.c_len)>>2)));
    tmp_u32 = t;
    fo->write(&tmp_u32, 4);

    stxt->sh_addralign = 4;
    stxt->sh_size += sizeof(stub_arm_linux_kernel_vmlinux_head);

    return sizeof(stub_arm_linux_kernel_vmlinux_head);
}

unsigned PackVmlinuxARMEB::write_vmlinux_head(
    OutputFile *const fo,
    Shdr *const stxt
)
{
    // First word from vmlinux-head.S
    fo->write(&stub_armeb_linux_kernel_vmlinux_head[0], 4);

    // Second word
    TE32 tmp_u32;
    unsigned const t = (0xff000000 &
            BeLePolicy::get32(&stub_armeb_linux_kernel_vmlinux_head[4]))
        | (0x00ffffff & (0u - 1 + ((3+ ph.c_len)>>2)));
    tmp_u32 = t;
    fo->write(&tmp_u32, 4);

    stxt->sh_addralign = 4;
    stxt->sh_size += sizeof(stub_armeb_linux_kernel_vmlinux_head);

    return sizeof(stub_armeb_linux_kernel_vmlinux_head);
}

unsigned PackVmlinuxPPC32::write_vmlinux_head(
    OutputFile * /*const fo*/,
    Shdr * /*const stxt*/
)
{
    return 0;
}


bool PackVmlinuxARMEL::has_valid_vmlinux_head()
{
    TE32 buf[2];
    fi->seek(p_text->sh_offset + sizeof(stub_arm_linux_kernel_vmlinux_head) -8, SEEK_SET);
    fi->readx(buf, sizeof(buf));
    //unsigned const word0 = buf[0];
    unsigned const word1 = buf[1];
    if (0xeb==(word1>>24)
    &&  (0x00ffffff& word1)==(0u - 1 + ((3+ ph.c_len)>>2))) {
        return true;
    }
    return false;
}

bool PackVmlinuxARMEB::has_valid_vmlinux_head()
{
    TE32 buf[2];
    fi->seek(p_text->sh_offset + sizeof(stub_armeb_linux_kernel_vmlinux_head) -8, SEEK_SET);
    fi->readx(buf, sizeof(buf));
    //unsigned const word0 = buf[0];
    unsigned const word1 = buf[1];
    if (0xeb==(word1>>24)
    &&  (0x00ffffff& word1)==(0u - 1 + ((3+ ph.c_len)>>2))) {
        return true;
    }
    return false;
}

bool PackVmlinuxPPC32::has_valid_vmlinux_head()
{
    TE32 buf[2];
    fi->seek(p_text->sh_offset + sizeof(stub_powerpc_linux_kernel_vmlinux_head) -8, SEEK_SET);
    fi->readx(buf, sizeof(buf));
    //unsigned const word0 = buf[0];
    unsigned const word1 = buf[1];
    if (0xeb==(word1>>24)
    &&  (0x00ffffff& word1)==(0u - 1 + ((3+ ph.c_len)>>2))) {
        return true;
    }
    return false;
}

bool PackVmlinuxI386::has_valid_vmlinux_head()
{
    unsigned char buf[5];
    fi->seek(p_text->sh_offset + sizeof(stub_i386_linux_kernel_vmlinux_head) -5, SEEK_SET);
    fi->readx(&buf[0], 5);
    if (0xE8!=buf[0] ||  BeLePolicy::get32(&buf[1]) != ph.c_len)
    {
        return false;
    }
    return true;
}

bool PackVmlinuxAMD64::has_valid_vmlinux_head()
{
    unsigned char buf[5];
    fi->seek(p_text->sh_offset + sizeof(stub_amd64_linux_kernel_vmlinux_head) -5, SEEK_SET);
    fi->readx(&buf[0], 5);
    if (0xE8!=buf[0] ||  BeLePolicy::get32(&buf[1]) != ph.c_len)
    {
        return false;
    }
    return true;
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
//targets := vmlinux upx-piggy.o
//
//LDFLAGS_vmlinux := -Ttext $(IMAGE_OFFSET) -e startup_32
//
//$(obj)/vmlinux: $(obj)/upx-piggy.o FORCE
//	$(call if_changed,ld)
//	@:
//
//$(obj)/upx-piggy.o: vmlinux FORCE
//	upx --lzma -f -o $@ $<; touch $@
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
//#   boot/compressed/vmlinux         by ld upx-piggy.o
//#              boot/vmlinux.bin     by objcopy
//#              boot/bzImage         by arch/i386/boot/tools/build with
//#                                     bootsect and setup
//#
//-----

#if 0  /*{*/
// For Debian nslu2-linux (2.6.19), only this Makefile changes:
--- ./debian/build/build-arm-none-ixp4xx/arch/arm/boot/compressed/Makefile.orig	2006-11-29 13:57:37.000000000 -0800
+++ ./debian/build/build-arm-none-ixp4xx/arch/arm/boot/compressed/Makefile	2006-12-16 02:39:38.000000000 -0800
@@ -5,7 +5,7 @@
 #

 HEAD	= head.o
-OBJS	= misc.o
+OBJS	=
 FONTC	= drivers/video/console/font_acorn_8x8.c

 FONT = $(addprefix ../../../../drivers/video/console/, font_acorn_8x8.o)
@@ -73,8 +73,8 @@

 SEDFLAGS	= s/TEXT_START/$(ZTEXTADDR)/;s/BSS_START/$(ZBSSADDR)/

-targets       := vmlinux vmlinux.lds piggy.gz piggy.o $(FONT) \
-		 head.o misc.o $(OBJS)
+targets       := vmlinux vmlinux.lds upx-piggy.o $(FONT) \
+		 head.o $(OBJS)
 EXTRA_CFLAGS  := -fpic
 EXTRA_AFLAGS  :=

@@ -95,20 +95,16 @@
 # would otherwise mess up our GOT table
 CFLAGS_misc.o := -Dstatic=

-$(obj)/vmlinux: $(obj)/vmlinux.lds $(obj)/$(HEAD) $(obj)/piggy.o \
-	 	$(addprefix $(obj)/, $(OBJS)) FORCE
+$(obj)/vmlinux: $(obj)/vmlinux.lds $(obj)/$(HEAD) \
+	 	$(addprefix $(obj)/, $(OBJS)) $(obj)/upx-piggy.o FORCE
 	$(call if_changed,ld)
 	@:

-$(obj)/piggy.gz: $(obj)/../Image FORCE
-	$(call if_changed,gzip)
-
-$(obj)/piggy.o:  $(obj)/piggy.gz FORCE
+$(obj)/upx-piggy.o:  vmlinux FORCE
+	upx --lzma -f -o $@ $<; touch $@

 CFLAGS_font_acorn_8x8.o := -Dstatic=

 $(obj)/vmlinux.lds: $(obj)/vmlinux.lds.in arch/arm/boot/Makefile .config
 	@sed "$(SEDFLAGS)" < $< > $@

-$(obj)/misc.o: $(obj)/misc.c include/asm/arch/uncompress.h lib/inflate.c
-
#endif  /*}*/

// Approximate translation for Linux 2.4.x:
// - - -
// arch/i386/Makefile: LD_FLAGS=-e startup_32
//----- arch/i386/boot/compressed/Makefile
//# linux/arch/i386/boot/compressed/Makefile
//#
//# create a compressed vmlinux image from the original vmlinux
//#
//
//HEAD =
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
//  $ gcc -m32 -o test-piggy -nostartfiles -nostdlib test-piggy.o piggy.o
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
//      pushl $0x600000  # 6 MiB length
//      pushl $0x100000  # 1 MiB address
//      call mmap
//      leal -0x9000(%esp),%esi  # expect "lea 0x9000(%esi),%esp" later
///* Fall into .text of upx-compressed vmlinux. */
//-----

// Example test jig for ARM:
//-----main.c
//unsigned work[(1<<16)/sizeof(unsigned)];
//unsigned kernel[(3<<20)/sizeof(unsigned)];
//
///* In order to avoid complaints such as
//     /usr/bin/ld: ERROR: libgcc_s.so uses hardware FP, whereas main uses software FP
//   when building this test program, then you must change the .e_flags
//   in the header of the compressed, relocatble output from upx, from
//      Flags: 0x202, has entry point, GNU EABI, software FP
//   to
//      Flags: 0x0
//*/
//
//main()
//{
//        char *const end = decompress_kernel(kernel, work, (1<<(16-2))+work, 0x1234);
//        write(1, kernel, (char *)end - (char *)kernel);
//        return 0;
//}
//-----


/*************************************************************************
//
**************************************************************************/

const int *PackVmlinuxAMD64::getCompressionMethods(int method, int level) const
{
    return Packer::getDefaultCompressionMethods_le32(method, level);
}


const int *PackVmlinuxAMD64::getFilters() const
{
    static const int filters[] = {
        0x49, 0x46,
    -1 };
    return filters;
}

bool PackVmlinuxAMD64::is_valid_e_entry(Addr e_entry)
{
    return 0x200000<=e_entry; // 2 MiB
}

Linker* PackVmlinuxAMD64::newLinker() const
{
    return new ElfLinkerX86;
}



// instantiate instances
template class PackVmlinuxBase<ElfClass_LE32>;
template class PackVmlinuxBase<ElfClass_BE32>;
template class PackVmlinuxBase<ElfClass_LE64>;


/*
vi:ts=4:et
*/

