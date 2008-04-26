/* p_djgpp2.cpp --

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
#include "p_djgpp2.h"
#include "linker.h"

static const
#include "stub/i386-dos32.djgpp2.h"
static const
#include "stub/i386-dos32.djgpp2-stubify.h"


/*************************************************************************
//
**************************************************************************/

PackDjgpp2::PackDjgpp2(InputFile *f) :
    super(f), coff_offset(0)
{
    bele = &N_BELE_RTP::le_policy;
    COMPILE_TIME_ASSERT(sizeof(external_scnhdr_t) == 40)
    COMPILE_TIME_ASSERT(sizeof(coff_header_t) == 0xa8)
    COMPILE_TIME_ASSERT_ALIGNED1(external_scnhdr_t)
    COMPILE_TIME_ASSERT_ALIGNED1(coff_header_t)
    COMPILE_TIME_ASSERT(sizeof(stub_i386_dos32_djgpp2_stubify) == 2048)
    COMPILE_TIME_ASSERT(STUB_I386_DOS32_DJGPP2_STUBIFY_ADLER32 == 0xbf689ba8)
    COMPILE_TIME_ASSERT(STUB_I386_DOS32_DJGPP2_STUBIFY_CRC32   == 0x2ae982b2)
    //printf("0x%08x\n", upx_adler32(stubify_stub, sizeof(stubify_stub)));
    //assert(upx_adler32(stubify_stub, sizeof(stubify_stub)) == STUBIFY_STUB_ADLER32);
}


const int *PackDjgpp2::getCompressionMethods(int method, int level) const
{
    return Packer::getDefaultCompressionMethods_le32(method, level);
}


const int *PackDjgpp2::getFilters() const
{
    static const int filters[] = {
        0x26, 0x24, 0x49, 0x46, 0x16, 0x13, 0x14, 0x11,
        FT_ULTRA_BRUTE, 0x25, 0x15, 0x12,
    FT_END };
    return filters;
}


unsigned PackDjgpp2::findOverlapOverhead(const upx_bytep buf,
                                         const upx_bytep tbuf,
                                         unsigned range,
                                         unsigned upper_limit) const
{
    unsigned o = super::findOverlapOverhead(buf, tbuf, range, upper_limit);
    o = (o + 0x3ff) &~ 0x1ff;
    return o;
}


Linker* PackDjgpp2::newLinker() const
{
    return new ElfLinkerX86;
}


void PackDjgpp2::buildLoader(const Filter *ft)
{
    // prepare loader
    initLoader(stub_i386_dos32_djgpp2, sizeof(stub_i386_dos32_djgpp2));
    addLoader("IDENTSTR,DJ2MAIN1",
              ft->id ? "DJCALLT1" : "",
              ph.first_offset_found == 1 ? "DJ2MAIN2" : "",
              M_IS_LZMA(ph.method) ? "LZMA_INIT_STACK" : "",
              getDecompressorSections(),
              M_IS_LZMA(ph.method) ? "LZMA_DONE_STACK" : "",
              "DJ2BSS00",
              NULL
             );
    if (ft->id)
    {
        assert(ft->calls > 0);
        addLoader("DJCALLT2", NULL);
        addFilter32(ft->id);
    }
    addLoader("DJRETURN,+40C,UPX1HEAD", NULL);
}


/*************************************************************************
// util
**************************************************************************/

void PackDjgpp2::handleStub(OutputFile *fo)
{
    if (fo && !opt->djgpp2_coff.coff)
    {
        if (coff_offset > 0)
        {
            // copy stub from exe
            Packer::handleStub(fi, fo, coff_offset);
        }
        else
        {
            // "stubify" stub
            info("Adding stub: %ld bytes", (long)sizeof(stub_i386_dos32_djgpp2_stubify));
            fo->write(stub_i386_dos32_djgpp2_stubify, sizeof(stub_i386_dos32_djgpp2_stubify));
        }
    }
}


static bool is_dlm(InputFile *fi, long coff_offset)
{
    unsigned char buf[4];
    long off;

    try {
        fi->seek(coff_offset, SEEK_SET);
        fi->readx(buf, 4);
        off = get_le32(buf);
        if (off < 0 || off > coff_offset + 4)
            return false;
        fi->seek(off, SEEK_SET);
        fi->readx(buf, 4);
        if (memcmp(buf, "DLMF", 4) == 0)
            return true;
    } catch (const IOException&) {
    }
    return false;
}


static void handle_allegropak(InputFile *fi, OutputFile *fo)
{
    unsigned char buf[0x4000];
    int pfsize = 0;

    try {
        fi->seek(-8, SEEK_END);
        fi->readx(buf, 8);
        if (memcmp(buf, "slh+", 4) != 0)
            return;
        pfsize = get_be32_signed(buf+4);
        if (pfsize <= 8 || (off_t) pfsize >= (off_t) fi->st.st_size)
            return;
        fi->seek(-pfsize, SEEK_END);
    } catch (const IOException&) {
        return;
    }
    while (pfsize > 0)
    {
        const int len = UPX_MIN(pfsize, (int)sizeof(buf));
        fi->readx(buf, len);
        fo->write(buf, len);
        pfsize -= len;
    }
}


int PackDjgpp2::readFileHeader()
{
    unsigned char hdr[0x1c];
    unsigned char magic[8];

    fi->seek(0, SEEK_SET);
    fi->readx(hdr, sizeof(hdr));
    if (get_le16(hdr) == 0x5a4d)        // MZ exe signature, stubbed?
    {
        coff_offset = 512 * get_le16(hdr+4);
        if (get_le16(hdr+2) != 0)
            coff_offset += get_le16(hdr+2) - 512;
        fi->seek(512, SEEK_SET);
        fi->readx(magic, 8);
        if (memcmp("go32stub", magic, 8) != 0)
            return 0;                   // not V2 image
        fi->seek(coff_offset, SEEK_SET);
        if (fi->read(&coff_hdr, sizeof(coff_hdr)) != sizeof(coff_hdr))
            throwCantPack("skipping djgpp symlink");
    }
    else
    {
        fi->seek(coff_offset, SEEK_SET);
        fi->readx(&coff_hdr, 0xa8);
    }
    if (coff_hdr.f_magic != 0x014c)    // I386MAGIC
        return 0;
    if ((coff_hdr.f_flags & 2) == 0)   // F_EXEC - COFF executable
        return 0;
    if (coff_hdr.a_magic != 0413)      // ZMAGIC - demand load format
        return 0;
    // FIXME: check for Linux etc.

    text = coff_hdr.sh;
    data = text + 1;
    bss  = data + 1;
    return UPX_F_DJGPP2_COFF;
}


// "strip" debug info
void PackDjgpp2::stripDebug()
{
    coff_hdr.f_symptr = 0;
    coff_hdr.f_nsyms = 0;
    coff_hdr.f_flags = 0x10f;   // 0x100: "32 bit machine: LSB first"
    memset(text->misc, 0, 12);
}


/*************************************************************************
//
**************************************************************************/

bool PackDjgpp2::canPack()
{
    if (!readFileHeader())
        return false;
    if (is_dlm(fi, coff_offset))
        throwCantPack("can't handle DLM");

    if (opt->force == 0)
        if (text->size != coff_hdr.a_tsize || data->size != coff_hdr.a_dsize)
            throwAlreadyPacked();
    if (text->vaddr + text->size != data->vaddr
        || data->vaddr + data->size != bss->vaddr)
    {
        if (text->vaddr + text->size < data->vaddr &&
            data->vaddr - text->vaddr == data->scnptr - text->scnptr)
        {
            // This hack is needed to compress Quake 1!
            text->size = coff_hdr.a_tsize = data->vaddr - text->vaddr;
        }
        else
            throwAlreadyPacked();
    }
    // FIXME: check for Linux etc.
    return true;
}


/*************************************************************************
//
**************************************************************************/

void PackDjgpp2::pack(OutputFile *fo)
{
    handleStub(fo);

    // patch coff header #1: "strip" debug info
    stripDebug();

    // read file
    const unsigned size = text->size + data->size;
    const unsigned tpos = text->scnptr;
    const unsigned hdrsize = 20 + 28 + sizeof(external_scnhdr_t) * coff_hdr.f_nscns;
    const unsigned usize = size + hdrsize;
    if (hdrsize < sizeof(coff_hdr) || hdrsize > tpos)
        throwCantPack("coff header error");

    ibuf.alloc(usize);
    obuf.allocForCompression(usize);

    fi->seek(coff_offset, SEEK_SET);
    fi->readx(ibuf, hdrsize);             // orig. coff header
    fi->seek(coff_offset + tpos, SEEK_SET);
    fi->readx(ibuf + hdrsize, size);

    // prepare packheader
    ph.u_len = usize;
    // prepare filter
    Filter ft(ph.level);
    ft.buf_len = usize - data->size;
    ft.addvalue = text->vaddr - hdrsize;
    // compress
    upx_compress_config_t cconf; cconf.reset();
    // limit stack size needed for runtime decompression
    cconf.conf_lzma.max_num_probs = 1846 + (768 << 4); // ushort: ~28 KiB stack
    compressWithFilters(&ft, 512, &cconf);

    // patch coff header #2
    const unsigned lsize = getLoaderSize();
    assert(lsize % 4 == 0);
    text->size = lsize;                   // new size of .text
    data->size = ph.c_len;                // new size of .data

    unsigned stack = 1024 + ph.overlap_overhead + getDecompressorWrkmemSize();
    stack = ALIGN_UP(stack, 16u);
    if (bss->size < stack)  // give it a .bss
        bss->size = stack;

    text->scnptr = sizeof(coff_hdr);
    data->scnptr = text->scnptr + text->size;
    data->vaddr = bss->vaddr + ((data->scnptr + data->size) & 0x1ff) - data->size + ph.overlap_overhead - 0x200;
    coff_hdr.f_nscns = 3;

    linker->defineSymbol("original_entry", coff_hdr.a_entry);
    linker->defineSymbol("length_of_bss", ph.overlap_overhead / 4);
    defineDecompressorSymbols();
    assert(bss->vaddr == ((size + 0x1ff) &~ 0x1ff) + (text->vaddr &~ 0x1ff));
    linker->defineSymbol("stack_for_lzma", bss->vaddr + bss->size);
    linker->defineSymbol("start_of_uncompressed", text->vaddr - hdrsize);
    linker->defineSymbol("start_of_compressed", data->vaddr);
    defineFilterSymbols(&ft);

    // we should not overwrite our decompressor during unpacking
    // the original coff header (which is put just before the
    // beginning of the original .text section)
    assert(text->vaddr > hdrsize + lsize + sizeof(coff_hdr));

    // patch coff header #3
    text->vaddr = sizeof(coff_hdr);
    coff_hdr.a_entry = sizeof(coff_hdr) + getLoaderSection("DJ2MAIN1");
    bss->vaddr += ph.overlap_overhead;
    bss->size -= ph.overlap_overhead;

    // because of a feature (bug?) in stub.asm we need some padding
    memcpy(obuf+data->size, "UPX", 3);
    data->size = ALIGN_UP(data->size, 4);

    linker->defineSymbol("DJ2MAIN1", coff_hdr.a_entry);
    relocateLoader();

    // prepare loader
    MemBuffer loader(lsize);
    memcpy(loader, getLoader(), lsize);
    patchPackHeader(loader, lsize);

    // write coff header, loader and compressed file
    fo->write(&coff_hdr, sizeof(coff_hdr));
    fo->write(loader, lsize);
    if (opt->debug.dump_stub_loader)
        OutputFile::dump(opt->debug.dump_stub_loader, loader, lsize);
    fo->write(obuf, data->size);
#if 0
    printf("%-13s: coff hdr   : %8ld bytes\n", getName(), (long) sizeof(coff_hdr));
    printf("%-13s: loader     : %8ld bytes\n", getName(), (long) lsize);
    printf("%-13s: compressed : %8ld bytes\n", getName(), (long) data->size);
#endif

    // verify
    verifyOverlappingDecompression();

    // handle overlay
    // FIXME: only Allegro pakfiles are supported
    handle_allegropak(fi, fo);

    // finally check the compression ratio
    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();
}


/*************************************************************************
//
**************************************************************************/

int PackDjgpp2::canUnpack()
{
    if (!readFileHeader())
        return false;
    if (is_dlm(fi, coff_offset))
        throwCantUnpack("can't handle DLM");
    fi->seek(coff_offset, SEEK_SET);
    return readPackHeader(4096) ? 1 : -1;
}


/*************************************************************************
//
**************************************************************************/

void PackDjgpp2::unpack(OutputFile *fo)
{
    handleStub(fo);

    ibuf.alloc(ph.c_len);
    obuf.allocForUncompression(ph.u_len);

    fi->seek(coff_offset + ph.buf_offset + ph.getPackHeaderSize(), SEEK_SET);
    fi->readx(ibuf, ph.c_len);

    // decompress
    decompress(ibuf, obuf);

    coff_header_t *chdr = (coff_header_t*) obuf.getVoidPtr();
    text = chdr->sh;
    data = text + 1;

    const unsigned hdrsize = 20 + 28
        + sizeof(external_scnhdr_t) * chdr->f_nscns;

    unsigned addvalue;
    if (ph.version >= 14)
        addvalue = text->vaddr - hdrsize;
    else
        addvalue = text->vaddr &~ 0x1ff; // for old versions

    // unfilter
    if (ph.filter)
    {
        Filter ft(ph.level);
        ft.init(ph.filter, addvalue);
        ft.cto = (unsigned char) ph.filter_cto;
        if (ph.version < 11)
        {
            unsigned char ctobuf[4];
            fi->readx(ctobuf, 4);
            ft.cto = (unsigned char) (get_le32(ctobuf) >> 24);
        }
        ft.unfilter(obuf, ph.u_len - data->size);
    }

    if (ph.version < 14)
    {
        // fixup for the aligning bug in strip 2.8+
        text->scnptr &= 0x1ff;
        data->scnptr = text->scnptr + text->size;
        // write decompressed file
        if (fo)
            fo->write(obuf, ph.u_len);
    }
    else
    {
        // write the header
        // some padding might be required between the end
        // of the header and the start of the .text section

        const unsigned padding = text->scnptr - hdrsize;
        ibuf.clear(0, padding);

        if (fo)
        {
            fo->write(obuf, hdrsize);
            fo->write(ibuf, padding);
            fo->write(obuf + hdrsize, ph.u_len - hdrsize);
        }
    }

    if (fo)
        handle_allegropak(fi, fo);
}


/*
vi:ts=4:et
*/

