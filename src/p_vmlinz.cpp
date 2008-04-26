/* p_vmlinz.cpp --

   This file is part of the UPX executable compressor.

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
 */


#include "conf.h"

#include "file.h"
#include "filter.h"
#include "packer.h"
#include "p_vmlinz.h"
#include "linker.h"
#include <zlib.h>

static const
#include "stub/i386-linux.kernel.vmlinuz.h"

static const unsigned stack_offset_during_uncompression = 0x9000;
// add to "real mode pointer" in %esi; total 0x99000 is typical

// from /usr/src/linux/arch/i386/boot/compressed/Makefile
static const unsigned zimage_offset = 0x1000;
static const unsigned bzimage_offset = 0x100000;


/*************************************************************************
//
**************************************************************************/

PackVmlinuzI386::PackVmlinuzI386(InputFile *f) :
    super(f), physical_start(0x100000), page_offset(0), config_physical_align(0)
{
    bele = &N_BELE_RTP::le_policy;
    COMPILE_TIME_ASSERT(sizeof(boot_sect_t) == 0x218);
}


const int *PackVmlinuzI386::getCompressionMethods(int method, int level) const
{
    return Packer::getDefaultCompressionMethods_le32(method, level);
}


const int *PackVmlinuzI386::getFilters() const
{
    static const int filters[] = {
        0x26, 0x24, 0x49, 0x46, 0x16, 0x13, 0x14, 0x11,
        FT_ULTRA_BRUTE, 0x25, 0x15, 0x12,
    FT_END };
    return filters;
}

const int *PackBvmlinuzI386::getFilters() const
{
    // The destination buffer might be relocated at runtime.
    static const int filters[] = {
        0x49, 0x46,
    FT_END };
    return filters;
}


bool PackVmlinuzI386::canPack()
{
    return readFileHeader() == getFormat();
}


/*************************************************************************
// common util routines
**************************************************************************/

int PackVmlinuzI386::readFileHeader()
{
    boot_sect_t h;

    setup_size = 0;

    fi->readx(&h, sizeof(h));
    if (h.boot_flag != 0xAA55)
        return 0;
    const bool hdrs = (memcmp(h.hdrs, "HdrS", 4) == 0);

    setup_size = (1 + (h.setup_sects ? h.setup_sects : 4)) * 0x200;
    if (setup_size <= 0 || setup_size >= file_size)
        return 0;

    int format = UPX_F_VMLINUZ_i386;
    unsigned sys_size = ALIGN_UP((unsigned) file_size, 16u) - setup_size;

    const unsigned char *p = (const unsigned char *) &h + 0x1e3;

    if (hdrs && memcmp(p, "\x0d\x0a\x07""ELKS", 7) == 0)
    {
        format = UPX_F_ELKS_8086;
    }
    else if (hdrs && (h.load_flags & 1) != 0)
    {
        format = UPX_F_BVMLINUZ_i386;
    }

    if (0x204<=h.version) {
        if ((16u * h.sys_size) != sys_size)
            return 0;
    }
    else { // h.sys_size is only 2 bytes
        if ((16u * (0xffff & h.sys_size)) != (~(~0u<<20) & sys_size))
            return 0;
    }

    // FIXME: add more checks for a valid kernel

    return format;
}


static int is_pow2(unsigned const x)
{
    return !(x & (x - 1));
}

// read full kernel into obuf[], gzip-decompress into ibuf[],
// return decompressed size
int PackVmlinuzI386::decompressKernel()
{
    // read whole kernel image
    obuf.alloc(file_size);
    fi->seek(0, SEEK_SET);
    fi->readx(obuf, file_size);

    {
    const upx_byte *base = NULL;
    unsigned relocated = 0;

    // See startup_32: in linux/arch/i386/boot/compressed/head.S
    const upx_byte *p = &obuf[setup_size];
    unsigned cpa_0 = 0;
    unsigned cpa_1 = 0;
    int j;
    for ((p = &obuf[setup_size]), (j= 0); j < 0x200; ++j, ++p) {
        if (0==memcmp("\x89\xeb\x81\xc3", p, 4)
        &&  0==memcmp("\x81\xe3",      8+ p, 2)) {
            // movl %ebp,%ebx
            // addl $imm.w,%ebx
            // andl $imm.w,%ebx
            cpa_0 = 1+ get_te32( 4+ p);
            cpa_1 =    get_te32(10+ p);
            break;
        }
    }
    for ((p = &obuf[setup_size]), (j= 0); j < 0x200; ++j, ++p) {
        if (0==memcmp("\x8d\x83",    p, 2)  // leal d32(%ebx),%eax
        &&  0==memcmp("\xff\xe0", 6+ p, 2)  // jmp *%eax
        ) {
            relocated = get_te32(2+ p);
        }
        if (0==memcmp("\xE8\x00\x00\x00\x00\x5D", p, 6)) {
            // "call 1f; 1f: pop %ebp"  determines actual execution address.
            // linux-2.6.21 (spring 2007) and later; upx stub needs work
            // unless LOAD_PHYSICAL_ADDR is known.
            // Allowed code is:  linux-2.6.23/arch/x86/head_32.S  2008-01-01
            //      call 1f
            //  1:  popl %ebp
            //      subl $1b, %ebp  # 32-bit immediate
            //      movl $LOAD_PHYSICAL_ADDR, %ebx
            //
            if (0==memcmp("\x81\xed", 6+  p, 2)      // subl $imm.w,%ebp
            &&  0==memcmp("\xbb",     12+ p, 1) ) {  // movl $imm.w,%ebx
                physical_start = get_te32(13+ p);
            } else
            if (0==memcmp("\x81\xed", 6+  p, 2)  // subl $imm.w,%ebp
            &&  is_pow2(cpa_0) && (0u-cpa_0)==cpa_1) {
                base = (5+ p) - get_te32(8+ p);
                config_physical_align = cpa_0;
            }
            else {
                throwCantPack("Unrecognized relocatable kernel");
            }
        }
        // Find "ljmp $__BOOT_CS,$__PHYSICAL_START" if any.
        if (0==memcmp("\xEA\x00\x00", p, 3) && 0==(0xf & p[3]) && 0==p[4]) {
            /* whole megabyte < 16 MiB */
            physical_start = get_te32(1+ p);
            break;
        }
    }
    if (base && relocated) {
        p = base + relocated;
        for (j = 0; j < 0x200; ++j, ++p) {
            if (0==memcmp("\x01\x9c\x0b", p, 3)  // addl %ebx,d32(%ebx,%ecx)
            ) {
                page_offset = 0u - get_te32(3+ p);
            }
            if (0==memcmp("\x89\xeb",    p, 2)  // movl %ebp,%ebx
            &&  0==memcmp("\x81\xeb", 2+ p, 2)  // subl $imm32,%ebx
            ) {
                physical_start = get_te32(4+ p);
            }
        }
    }
    }

    checkAlreadyPacked(obuf + setup_size, UPX_MIN(file_size - setup_size, (off_t)1024));

    for (int gzoff = setup_size; gzoff < file_size; gzoff++)
    {
        // find gzip header (2 bytes magic + 1 byte method "deflated")
        int off = find(obuf + gzoff, file_size - gzoff, "\x1F\x8B\x08", 3);
        if (off < 0)
            break;
        gzoff += off;
        const int gzlen = file_size - gzoff;
        if (gzlen < 256)
            break;
        // check gzip flag byte
        unsigned char flags = obuf[gzoff + 3];
        if ((flags & 0xe0) != 0)        // reserved bits set
            continue;
        //printf("found gzip header at offset %d\n", gzoff);

        // try to decompress
        int klen;
        int fd;
        off_t fd_pos;
        for (;;)
        {
            klen = -1;
            fd = -1;
            fd_pos = -1;
            // open
            fi->seek(gzoff, SEEK_SET);
            fd = dup(fi->getFd());
            if (fd < 0)
                break;
            gzFile zf = gzdopen(fd, "rb");
            if (zf == NULL)
                break;
            // estimate gzip-decompressed kernel size & alloc buffer
            if (ibuf.getSize() == 0)
                ibuf.alloc(gzlen * 3);
            // decompress
            klen = gzread(zf, ibuf, ibuf.getSize());
            fd_pos = lseek(fd, 0, SEEK_CUR);
            gzclose(zf);
            fd = -1;
            if (klen != (int)ibuf.getSize())
                break;
            // realloc and try again
            unsigned s = ibuf.getSize();
            ibuf.dealloc();
            ibuf.alloc(3 * s / 2);
        }
        if (fd >= 0)
            (void) close(fd);
        if (klen <= 0)
            continue;

        if (klen <= gzlen)
            continue;

        if (opt->force > 0)
            return klen;

        // some checks
        if (fd_pos != file_size)
        {
            //printf("fd_pos: %ld, file_size: %ld\n", (long)fd_pos, (long)file_size);

            // linux-2.6.21.5/arch/i386/boot/compressed/vmlinux.lds
            // puts .data.compressed ahead of .text, .rodata, etc;
            // so piggy.o need not be last in bzImage.  Alas.
            //throwCantPack("trailing bytes after kernel image; use option '-f' to force packing");
        }


        // see /usr/src/linux/arch/i386/kernel/head.S
        // 2.4.x: [cli;] cld; mov $...,%eax
        if (memcmp(ibuf,     "\xFC\xB8", 2) == 0) goto head_ok;
        if (memcmp(ibuf, "\xFA\xFC\xB8", 3) == 0) goto head_ok;
        // 2.6.21.5 CONFIG_PARAVIRT  mov %cs,%eax; test $3,%eax; jne ...;
        if (memcmp(ibuf, "\x8c\xc8\xa9\x03\x00\x00\x00\x0f\x85", 9) == 0) goto head_ok;
        if (memcmp(ibuf, "\x8c\xc8\xa8\x03\x0f\x85", 6) == 0) goto head_ok;
        // 2.6.x: [cli;] cld; lgdt ...
        if (memcmp(ibuf,     "\xFC\x0F\x01", 3) == 0) goto head_ok;
        if (memcmp(ibuf, "\xFA\xFC\x0F\x01", 4) == 0) goto head_ok;
        // 2.6.x+grsecurity+strongswan+openwall+trustix: ljmp $0x10,...
        if (ibuf[0] == 0xEA && memcmp(ibuf+5, "\x10\x00", 2) == 0) goto head_ok;
        // x86_64 2.6.x
        if (0xB8==ibuf[0]  // mov $...,%eax
        &&  0x8E==ibuf[5] && 0xD8==ibuf[6]  // mov %eax,%ds
        &&  0x0F==ibuf[7] && 0x01==ibuf[8] && 020==(070 & ibuf[9]) // lgdtl
        &&  0xB8==ibuf[14]  // mov $...,%eax
        &&  0x0F==ibuf[19] && 0xA2==ibuf[20]  // cpuid
        ) goto head_ok;

        // cmpw   $0x207,0x206(%esi)  Debian vmlinuz-2.6.24-12-generic
        if (0==memcmp("\x66\x81\xbe\x06\x02\x00\x00\x07\x02", ibuf, 9)) goto head_ok;

        // testb  $0x40,0x211(%esi)  Fedora vmlinuz-2.6.25-0.218.rc8.git7.fc9.i686
        if (0==memcmp("\xf6\x86\x11\x02\x00\x00\x40", ibuf, 7)) goto head_ok;

        // rex.W prefix for x86_64
        if (0x48==ibuf[0]) throwCantPack("x86_64 bzImage is not yet supported");

        throwCantPack("unrecognized kernel architecture; use option '-f' to force packing");
    head_ok:

        // FIXME: more checks for special magic bytes in ibuf ???
        // FIXME: more checks for kernel architecture ???

        return klen;
    }

    return 0;
}


void PackVmlinuzI386::readKernel()
{
    int klen = decompressKernel();
    if (klen <= 0)
        throwCantPack("kernel decompression failed");
    //OutputFile::dump("kernel.img", ibuf, klen);

    // copy the setup boot code
    setup_buf.alloc(setup_size);
    memcpy(setup_buf, obuf, setup_size);
    //OutputFile::dump("setup.img", setup_buf, setup_size);

    obuf.dealloc();
    obuf.allocForCompression(klen);

    ph.u_len = klen;
    ph.filter = 0;
}


Linker* PackVmlinuzI386::newLinker() const
{
    return new ElfLinkerX86;
}


/*************************************************************************
// vmlinuz specific
**************************************************************************/

void PackVmlinuzI386::buildLoader(const Filter *ft)
{
    // prepare loader
    initLoader(stub_i386_linux_kernel_vmlinuz, sizeof(stub_i386_linux_kernel_vmlinuz));
    addLoader("LINUZ000",
              ph.first_offset_found == 1 ? "LINUZ010" : "",
              ft->id ? "LZCALLT1" : "",
              "LZIMAGE0",
              getDecompressorSections(),
              NULL
             );
    if (ft->id)
    {
        assert(ft->calls > 0);
        addLoader("LZCALLT9", NULL);
        addFilter32(ft->id);
    }
    addLoader("LINUZ990,IDENTSTR,UPX1HEAD", NULL);
}


void PackVmlinuzI386::pack(OutputFile *fo)
{
    readKernel();

    // prepare filter
    Filter ft(ph.level);
    ft.buf_len = ph.u_len;
    ft.addvalue = physical_start;  // saves 4 bytes in unfilter code

    // compress
    upx_compress_config_t cconf; cconf.reset();
    // limit stack size needed for runtime decompression
    cconf.conf_lzma.max_num_probs = 1846 + (768 << 4); // ushort: ~28 KiB stack
    compressWithFilters(&ft, 512, &cconf);

    const unsigned lsize = getLoaderSize();

    defineDecompressorSymbols();
    defineFilterSymbols(&ft);
    linker->defineSymbol("src_for_decompressor", zimage_offset + lsize);
    linker->defineSymbol("original_entry", physical_start);
    linker->defineSymbol("stack_offset", stack_offset_during_uncompression);
    relocateLoader();

    MemBuffer loader(lsize);
    memcpy(loader, getLoader(), lsize);
    patchPackHeader(loader, lsize);

    boot_sect_t * const bs = (boot_sect_t *) ((unsigned char *) setup_buf);
    bs->sys_size = ALIGN_UP(lsize + ph.c_len, 16u) / 16;

    fo->write(setup_buf, setup_buf.getSize());
    fo->write(loader, lsize);
    fo->write(obuf, ph.c_len);
#if 0
    printf("%-13s: setup        : %8ld bytes\n", getName(), (long) setup_buf.getSize());
    printf("%-13s: loader       : %8ld bytes\n", getName(), (long) lsize);
    printf("%-13s: compressed   : %8ld bytes\n", getName(), (long) ph.c_len);
#endif

    // verify
    verifyOverlappingDecompression();

    // finally check the compression ratio
    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();
}


/*************************************************************************
// bvmlinuz specific
**************************************************************************/

void PackBvmlinuzI386::buildLoader(const Filter *ft)
{
    // prepare loader
    initLoader(stub_i386_linux_kernel_vmlinuz, sizeof(stub_i386_linux_kernel_vmlinuz));
    if (0!=page_offset) { // relocatable kernel
        assert(0==ft->id || 0x40==(0xf0 & ft->id));  // others assume fixed buffer address
        addLoader("LINUZ000,LINUZVGA,LINUZ101,LINUZ110",
            ((0!=config_physical_align) ? "LINUZ120" : "LINUZ130"),
            "LINUZ140,LZCUTPOI,LINUZ141",
            (ft->id ? "LINUZ145" : ""),
            (ph.first_offset_found == 1 ? "LINUZ010" : ""),
            NULL);
    }
    else {
        addLoader("LINUZ000,LINUZVGA,LINUZ001",
              ph.first_offset_found == 1 ? "LINUZ010" : "",
              (0x40==(0xf0 & ft->id)) ? "LZCKLLT1" : (ft->id ? "LZCALLT1" : ""),
              "LBZIMAGE,IDENTSTR",
              "+40", // align the stuff to 4 byte boundary
              "UPX1HEAD", // 32 byte
              "LZCUTPOI",
              NULL);
        // fake alignment for the start of the decompressor
        //linker->defineSymbol("LZCUTPOI", 0x1000);
    }

    addLoader(getDecompressorSections(), NULL);

    if (ft->id)
    {
            assert(ft->calls > 0);
        if (0x40==(0xf0 & ft->id)) {
            addLoader("LZCKLLT9", NULL);
        }
        else {
            addLoader("LZCALLT9", NULL);
        }
        addFilter32(ft->id);
    }
    if (0!=page_offset) {
        addLoader("LINUZ150,IDENTSTR,+40,UPX1HEAD", NULL);
        unsigned const l_len = getLoaderSize();
        unsigned const c_len = ALIGN_UP(ph.c_len, 4u);
        unsigned const e_len = getLoaderSectionStart("LINUZ141") -
                               getLoaderSectionStart("LINUZ110");
        linker->defineSymbol("compressed_length", c_len);
        linker->defineSymbol("load_physical_address", physical_start);  // FIXME
        if (0!=config_physical_align) {
            linker->defineSymbol("neg_config_physical_align", 0u - config_physical_align);
        }
        linker->defineSymbol("neg_length_mov", 0u - ALIGN_UP(c_len + l_len, 4u));
        linker->defineSymbol("neg_page_offset", 0u - page_offset);
        //linker->defineSymbol("physical_start", physical_start);
        linker->defineSymbol("unc_length", ph.u_len);
        linker->defineSymbol("dec_offset", ph.overlap_overhead + e_len);
        linker->defineSymbol("unc_offset", ph.overlap_overhead + ph.u_len - c_len);
    }
    else {
        addLoader("LINUZ990", NULL);
    }
}


void PackBvmlinuzI386::pack(OutputFile *fo)
{
    readKernel();

    // prepare filter
    Filter ft(ph.level);
    ft.buf_len = ph.u_len;
    ft.addvalue = 0;  // The destination buffer might be relocated at runtime.

    upx_compress_config_t cconf; cconf.reset();
    // limit stack size needed for runtime decompression
    cconf.conf_lzma.max_num_probs = 1846 + (768 << 4); // ushort: ~28 KiB stack

    // FIXME: new stub allows most of low memory as stack for Bvmlinuz ?
    //cconf.conf_lzma.max_num_probs = (0x99000 - 0x10250)>>1; // ushort: 560560 stack

    compressWithFilters(&ft, 512, &cconf);

    // align everything to dword boundary - it is easier to handle
    unsigned c_len = ph.c_len;
    memset(obuf + c_len, 0, 4);
    c_len = ALIGN_UP(c_len, 4u);

    const unsigned lsize = getLoaderSize();

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

    const int e_len = getLoaderSectionStart("LZCUTPOI");
    assert(e_len > 0);

    if (0==page_offset) {  // not relocatable kernel
        const unsigned d_len4 = ALIGN_UP(lsize - e_len, 4u);
        const unsigned decompr_pos = ALIGN_UP(ph.u_len + ph.overlap_overhead, 16u);
        const unsigned copy_size = c_len + d_len4;
        const unsigned edi = decompr_pos + d_len4 - 4;          // copy to
        const unsigned esi = ALIGN_UP(c_len + lsize, 4u) - 4;   // copy from

        linker->defineSymbol("decompressor", decompr_pos - bzimage_offset + physical_start);
        linker->defineSymbol("src_for_decompressor", physical_start + decompr_pos - c_len);
        linker->defineSymbol("words_to_copy", copy_size / 4);
        linker->defineSymbol("copy_dest", physical_start + edi);
        linker->defineSymbol("copy_source", bzimage_offset + esi);
    }

    defineFilterSymbols(&ft);
    defineDecompressorSymbols();
    if (0==page_offset) {
        linker->defineSymbol("original_entry", physical_start);
    }
    linker->defineSymbol("stack_offset", stack_offset_during_uncompression);
    relocateLoader();

    MemBuffer loader(lsize);
    memcpy(loader, getLoader(), lsize);
    patchPackHeader(loader, lsize);

    boot_sect_t * const bs = (boot_sect_t *) ((unsigned char *) setup_buf);
    bs->sys_size = (ALIGN_UP(lsize + c_len, 16u) / 16);

    fo->write(setup_buf, setup_buf.getSize());

    unsigned const e_pfx = (0==page_offset) ? 0 : getLoaderSectionStart("LINUZ110");
    if (0!=page_offset) {
        fo->write(loader, e_pfx);
    }
    else {
        fo->write(loader, e_len);
    }
    fo->write(obuf, c_len);
    if (0!=page_offset) {
        fo->write(loader + e_pfx, e_len - e_pfx);
    }
    fo->write(loader + e_len, lsize - e_len);
#if 0
    printf("%-13s: setup        : %8ld bytes\n", getName(), (long) setup_buf.getSize());
    printf("%-13s: entry        : %8ld bytes\n", getName(), (long) e_len);
    printf("%-13s: compressed   : %8ld bytes\n", getName(), (long) c_len);
    printf("%-13s: decompressor : %8ld bytes\n", getName(), (long) (lsize - e_len));
#endif

    // verify
    verifyOverlappingDecompression();

    // finally check the compression ratio
    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();
}


/*************************************************************************
// unpack
**************************************************************************/

int PackVmlinuzI386::canUnpack()
{
    if (readFileHeader() != getFormat())
        return false;
    fi->seek(setup_size, SEEK_SET);
    return readPackHeader(1024) ? 1 : -1;
}


void PackVmlinuzI386::unpack(OutputFile *fo)
{
    // no uncompression support for this format, so that
    // it is possible to remove the original deflate code (>10 KiB)

    // FIXME: but we could write the uncompressed "vmlinux" image

    ibuf.alloc(ph.c_len);
    obuf.allocForUncompression(ph.u_len);

    fi->seek(setup_size + ph.buf_offset + ph.getPackHeaderSize(), SEEK_SET);
    fi->readx(ibuf, ph.c_len);

    // decompress
    decompress(ibuf, obuf);

    // unfilter
    Filter ft(ph.level);
    ft.init(ph.filter, physical_start);
    ft.cto = (unsigned char) ph.filter_cto;
    ft.unfilter(obuf, ph.u_len);

    // write decompressed file
    if (fo)
    {
        throwCantUnpack("build a new kernel instead :-)");
        //fo->write(obuf, ph.u_len);
    }
}


/*
vi:ts=4:et
*/


