/* p_djgpp2.cpp --

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
#include "filter.h"
#include "packer.h"
#include "p_djgpp2.h"

static const
#include "stub/stubify.h"

static const
#include "stub/l_djgpp2.h"


/*************************************************************************
//
**************************************************************************/

PackDjgpp2::PackDjgpp2(InputFile *f) :
    super(f), coff_offset(0)
{
    assert(sizeof(coff_hdr) == 0xa8);
    assert(sizeof(stubify_stub) == 2048);
}


int PackDjgpp2::getCompressionMethod() const
{
    if (M_IS_NRV2B(opt->method))
        return M_NRV2B_LE32;
    if (M_IS_NRV2D(opt->method))
        return M_NRV2D_LE32;
    return opt->level > 1 && file_size >= 512*1024 ? M_NRV2D_LE32 : M_NRV2B_LE32;
}

const int *PackDjgpp2::getFilters() const
{
    static const int filters[] = { 0x26, 0x24, 0x11, 0x14, 0x13, 0x16,
                                   0x25, 0x15, 0x12, -1 };
    return filters;
}


int PackDjgpp2::buildLoader(const Filter *ft)
{
    // prepare loader
    initLoader(nrv_loader,sizeof(nrv_loader));
    addLoader("IDENTSTR""DJ2MAIN1",
              ft->id ? "DJCALLT1" : "",
              "DJ2MAIN2",
              getDecompressor(),
              "DJ2BSS00",
              NULL
             );
    if (ft->id)
    {
        assert(ft->calls > 0);
        addLoader("DJCALLT2",NULL);
        addFilter32(ft->id);
    }
    addLoader("DJRETURN+40DXXXXUPX1HEAD",NULL);
    return getLoaderSize();
}


/*************************************************************************
// util
**************************************************************************/

void PackDjgpp2::handleStub(OutputFile *fo)
{
    if (fo && !opt->djgpp2.coff)
    {
        if (coff_offset > 0)
        {
            // copy stub from exe
            Packer::handleStub(fi,fo,coff_offset);
        }
        else
        {
            // "stubify" stub
            info("Adding stub: %ld bytes", (long)sizeof(stubify_stub));
            fo->write(stubify_stub,sizeof(stubify_stub));
        }
    }
}


static bool is_dlm(InputFile *fi, long coff_offset)
{
    unsigned char buf[4];
    long off;

    try {
        fi->seek(coff_offset,SEEK_SET);
        fi->readx(buf,4);
        off = get_le32(buf);
        if (off < 0 || off > coff_offset + 4)
            return false;
        fi->seek(off,SEEK_SET);
        fi->readx(buf,4);
        if (memcmp(buf,"DLMF",4) == 0)
            return true;
    } catch (IOException&) {
    }
    return false;
}


static void handle_allegropak(InputFile *fi, OutputFile *fo)
{
    unsigned char buf[0x4000];
    unsigned pfsize=0, ic;

    try {
        fi->seek(-8,SEEK_END);
        fi->readx(buf,8);
        if (memcmp(buf,"slh+",4) != 0)
            return;
        pfsize = get_be32(buf+4);
        fi->seek(-(off_t)pfsize,SEEK_END);
    } catch (IOException&) {
        return;
    }
    while (pfsize)
    {
        ic = pfsize < sizeof(buf) ? pfsize : sizeof(buf);
        fi->readx(buf,ic);
        fo->write(buf,ic);
        pfsize -= ic;
    }
}


bool PackDjgpp2::readFileHeader()
{
    unsigned char hdr[0x1c];
    unsigned char magic[8];

    fi->seek(0,SEEK_SET);
    fi->readx(hdr,sizeof(hdr));
    if (get_le16(hdr) == 0x5a4d)        // MZ exe signature, stubbed?
    {
        coff_offset = 512 * get_le16(hdr+4);
        if (get_le16(hdr+2) != 0)
            coff_offset += get_le16(hdr+2) - 512;
        fi->seek(512,SEEK_SET);
        fi->readx(magic,8);
        if (memcmp("go32stub",magic,8) != 0)
            return false;               // not V2 image
        fi->seek(coff_offset,SEEK_SET);
        if (fi->read(&coff_hdr,sizeof(coff_hdr)) != sizeof(coff_hdr))
            throwCantPack("skipping djgpp symlink");
    }
    else
    {
        fi->seek(coff_offset,SEEK_SET);
        fi->readx(&coff_hdr,0xa8);
    }
    if (coff_hdr.f_magic != 0x014c)    // I386MAGIC
        return false;
    if ((coff_hdr.f_flags & 2) == 0)   // F_EXEC - COFF executable
        return false;
    if (coff_hdr.a_magic != 0413)      // ZMAGIC - demand load format
        return false;
    // FIXME: check for Linux etc.

    text = coff_hdr.sh;
    data = text + 1;
    bss  = data + 1;
    return true;
}


// "strip" debug info
void PackDjgpp2::stripDebug()
{
    coff_hdr.f_symptr = 0;
    coff_hdr.f_nsyms = 0;
    coff_hdr.f_flags = 0x10f;   // 0x100: "32 bit machine: LSB first"
    memset(text->misc,0,12);
}


/*************************************************************************
//
**************************************************************************/

bool PackDjgpp2::canPack()
{
    if (!readFileHeader())
        return false;
    if (is_dlm(fi,coff_offset))
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
    const unsigned usize = size + (tpos & 0x1ff);
    const unsigned hdrsize = 20 + 28 + (40 * coff_hdr.f_nscns);
    if (hdrsize < sizeof(coff_hdr) || hdrsize > tpos)
        throwCantPack("coff header error");

    ibuf.alloc(usize);
    obuf.allocForCompression(usize);

    fi->seek(coff_offset,SEEK_SET);
    fi->readx(ibuf,hdrsize);             // orig. coff header
    memset(ibuf + hdrsize, 0, tpos - hdrsize);
    fi->seek(coff_offset+tpos,SEEK_SET);
    fi->readx(ibuf + (tpos & 0x1ff),size);

#if 0
    // filter
    Filter ft(opt->level);
    tryFilters(&ft, ibuf, usize - data->size, text->vaddr & ~0x1ff);

    // compress
    ph.filter = ft.id;
    ph.filter_cto = ft.cto;
    ph.u_len = usize;
    if (!compress(ibuf,obuf))
        throwNotCompressible();

    unsigned overlapoh = findOverlapOverhead(obuf,ibuf,512);
    overlapoh = (overlapoh + 0x3ff) & ~0x1ff;

    // verify filter
    ft.verifyUnfilter();
#else
    // new version using compressWithFilters()

    // prepare packheader
    ph.u_len = usize;
    ph.filter = 0;
    // prepare filter
    Filter ft(opt->level);
    ft.buf_len = usize - data->size;
    ft.addvalue = text->vaddr & ~0x1ff;
    // prepare other settings
    const unsigned overlap_range = 512;
    unsigned overlapoh;

    int strategy = -1;      // try the first working filter
    if (opt->filter >= 0 && isValidFilter(opt->filter))
        // try opt->filter or 0 if that fails
        strategy = -2;
    else if (opt->all_filters)
        // choose best from all available filters
        strategy = 0;
    compressWithFilters(&ft, &overlapoh, overlap_range, strategy);
    overlapoh = (overlapoh + 0x3ff) & ~0x1ff;
#endif

    // patch coff header #2
    const unsigned lsize = getLoaderSize();
    text->size = lsize;                   // new size of .text
    data->size = ph.c_len;                // new size of .data

    if (bss->size < overlapoh)            // give it a .bss
        bss->size = overlapoh;

    text->scnptr = sizeof(coff_hdr);
    data->scnptr = text->scnptr + text->size;
    data->vaddr = bss->vaddr + ((data->scnptr + data->size) & 0x1ff) - data->size + overlapoh - 0x200;
    coff_hdr.f_nscns = 3;

    // prepare loader
    MemBuffer loader(lsize);
    memcpy(loader,getLoader(),lsize);

    // patch loader
    patchPackHeader(loader,lsize);
    patch_le32(loader,lsize,"ENTR",coff_hdr.a_entry);
    patchFilter32(loader, lsize, &ft);
    patch_le32(loader,lsize,"BSSL",overlapoh/4);
    assert(bss->vaddr == ((size + 0x1ff) &~ 0x1ff) + (text->vaddr &~ 0x1ff));
    patch_le32(loader,lsize,"OUTP",text->vaddr &~ 0x1ff);
    patch_le32(loader,lsize,"INPP",data->vaddr);

    // patch coff header #3
    text->vaddr = sizeof(coff_hdr);
    coff_hdr.a_entry = sizeof(coff_hdr) + getLoaderSection("DJ2MAIN1");
    bss->vaddr += overlapoh;
    bss->size -= overlapoh;

    // because of a feature (bug?) in stub.asm we need some padding
    memcpy(obuf+data->size,"UPX",3);
    data->size = ALIGN_UP(data->size,4);

    // write coff header, loader and compressed file
    fo->write(&coff_hdr,sizeof(coff_hdr));
    fo->write(loader,lsize);
    fo->write(obuf,data->size);
#if 0
    printf("%-13s: coff hdr   : %8ld bytes\n", getName(), (long) sizeof(coff_hdr));
    printf("%-13s: loader     : %8ld bytes\n", getName(), (long) lsize);
    printf("%-13s: compressed : %8ld bytes\n", getName(), (long) data->size);
#endif

    // verify
    verifyOverlappingDecompression(&obuf, overlapoh);

    // handle overlay
    // FIXME: only Allegro pakfiles are supported
    handle_allegropak(fi,fo);

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
    if (is_dlm(fi,coff_offset))
        throwCantUnpack("can't handle DLM");
    return readPackHeader(1024, coff_offset) ? 1 : -1;
}


/*************************************************************************
//
**************************************************************************/

void PackDjgpp2::unpack(OutputFile *fo)
{
    handleStub(fo);

    ibuf.alloc(ph.c_len);
    obuf.allocForUncompression(ph.u_len);

    fi->seek(coff_offset + ph.buf_offset + ph.getPackHeaderSize(),SEEK_SET);
    fi->readx(ibuf,ph.c_len);

    // decompress
    decompress(ibuf,obuf);

    // unfilter
    if (ph.filter)
    {
        memcpy(&coff_hdr,obuf,sizeof(coff_hdr));

        Filter ft(ph.level);
        ft.init(ph.filter, text->vaddr &~ 0x1ff);
        ft.cto = (unsigned char) ph.filter_cto;
        if (ph.version < 11)
        {
            unsigned char ctobuf[4];
            fi->readx(ctobuf, 4);
            ft.cto = (unsigned char) (get_le32(ctobuf) >> 24);
        }
        ft.unfilter(obuf, ph.u_len - data->size);
    }

    // fixup for the aligning bug in strip 2.8+
    text = ((coff_header_t*) (unsigned char *) obuf)->sh;
    data = text + 1;
    text->scnptr &= 0x1ff;
    data->scnptr = text->scnptr + text->size;

    // write decompressed file
    if (fo)
    {
        fo->write(obuf,ph.u_len);
        handle_allegropak(fi,fo);
    }
}


/*
vi:ts=4:et
*/

