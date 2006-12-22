/* p_vmlinz.cpp --

   This file is part of the UPX executable compressor.

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
    super(f), physical_start(0x100000)
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
        0x49,
        0x26, 0x24, 0x16, 0x13, 0x14, 0x11, FT_ULTRA_BRUTE, 0x25, 0x15, 0x12,
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
        // account for 16-bit h.sys_size, wrap around at 20 bits
        sys_size &= (1 << 20) - 1;
    }

    if (16u * h.sys_size != sys_size)
        return 0;

    // FIXME: add more checks for a valid kernel

    return format;
}


// read full kernel into obuf[], gzip-decompress into ibuf[],
// return decompressed size
int PackVmlinuzI386::decompressKernel()
{
    // read whole kernel image
    obuf.alloc(file_size);
    fi->seek(0, SEEK_SET);
    fi->readx(obuf, file_size);

    // Find "ljmp $__BOOT_CS,$__PHYSICAL_START" if any.
    // See startup_32: in linux/arch/i386/boot/compressed/head.S
    char const *p = (char const *)&obuf[setup_size];
    for (int j= 0; j < 0x200; ++j, ++p)
    if (0==memcmp("\xEA\x00\x00", p, 3) && 0==(0xf & p[3]) && 0==p[4]) {
        /* whole megabyte < 16MB */
        physical_start = get_le32(1+ p);
        break;
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
            throwCantPack("trailing bytes after kernel image; use option '-f' to force packing");
        }


        // see /usr/src/linux/arch/i386/kernel/head.S
        // 2.4.x: cld; mov $...,%eax
        if (memcmp(ibuf, "\xFC\xB8", 2) == 0) goto head_ok;
        // 2.6.x: cld; lgdt ...
        if (memcmp(ibuf, "\xFC\x0F\x01", 3) == 0) goto head_ok;
        // 2.6.x+grsecurity+strongswan+openwall+trustix: ljmp $0x10,...
        if (ibuf[0] == 0xEA && memcmp(ibuf+5, "\x10\x00", 2) == 0) goto head_ok;
        // x86_64 2.6.x
        if (0xB8==ibuf[0]  // mov $...,%eax
        &&  0x8E==ibuf[5] && 0xD8==ibuf[6]  // mov %eax,%ds
        &&  0x0F==ibuf[7] && 0x01==ibuf[8] && 020==(070 & ibuf[9]) // lgdtl
        &&  0xB8==ibuf[14]  // mov $...,%eax
        &&  0x0F==ibuf[19] && 0xA2==ibuf[20]  // cpuid
        ) goto head_ok;

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
              ph.first_offset_found == 1 ? "LINUZ001" : "",
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


static bool defineFilterSymbols(Linker *linker, const Filter *ft)
{
    if (ft->id == 0)
        return false;
    assert(ft->calls > 0);

    linker->defineSymbol("filter_cto", ft->cto);
    linker->defineSymbol("filter_length",
                         (ft->id & 0xf) % 3 == 0 ? ft->calls :
                         ft->lastcall - ft->calls * 4);
    return true;
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
    cconf.conf_lzma.max_num_probs = 1846 + (768 << 4); // ushort: ~28KB stack
    compressWithFilters(&ft, 512, &cconf);

    const unsigned lsize = getLoaderSize();

    defineFilterSymbols(linker, &ft);
    defineDecompressorSymbols();
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
    addLoader("LINUZ000",
              ph.first_offset_found == 1 ? "LINUZ001" : "",
              (0x40==(0xf0 & ft->id)) ? "LZCKLLT1" : (ft->id ? "LZCALLT1" : ""),
              "LBZIMAGE,IDENTSTR",
              "+40", // align the stuff to 4 byte boundary
              "UPX1HEAD", // 32 byte
              "LZCUTPOI",
              NULL);

    // fake alignment for the start of the decompressor
    linker->defineSymbol("LZCUTPOI", 0x1000);

    addLoader(getDecompressorSections(),
              NULL
             );
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
    addLoader("LINUZ990", NULL);
}


void PackBvmlinuzI386::pack(OutputFile *fo)
{
    readKernel();

    // prepare filter
    Filter ft(ph.level);
    ft.buf_len = ph.u_len;
    ft.addvalue = physical_start;  // saves 4 bytes in unfilter code

    upx_compress_config_t cconf; cconf.reset();
    // limit stack size needed for runtime decompression
    cconf.conf_lzma.max_num_probs = 1846 + (768 << 4); // ushort: ~28KB stack
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

    defineFilterSymbols(linker, &ft);
    if (0x40==(0xf0 & ft.id)) {
        linker->defineSymbol("filter_length", ft.buf_len); // redefine
        assert(ft.buf_len == ph.u_len);
    }
    defineDecompressorSymbols();
    linker->defineSymbol("original_entry", physical_start);
    linker->defineSymbol("stack_offset", stack_offset_during_uncompression);
    relocateLoader();

    MemBuffer loader(lsize);
    memcpy(loader, getLoader(), lsize);
    patchPackHeader(loader, lsize);

    boot_sect_t * const bs = (boot_sect_t *) ((unsigned char *) setup_buf);
    bs->sys_size = (ALIGN_UP(lsize + c_len, 16u) / 16) & 0xffff;

    fo->write(setup_buf, setup_buf.getSize());
    fo->write(loader, e_len);
    fo->write(obuf, c_len);
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
    // it is possible to remove the original deflate code (>10KB)

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


