/* p_elks.cpp --

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

#if 0

#include "file.h"
#include "filter.h"
#include "packer.h"
#include "p_vmlinz.h"

// FIXME
static const
#include "stub/l_vmlinz.h"

// from elks-0.0.83/Documentation/boot.txt
static const unsigned kernel_entry = 0x100000;
static const unsigned stack_during_uncompression = 0x90000;
static const unsigned zimage_offset = 0x1000;


/*************************************************************************
//
**************************************************************************/

const int *PackElks8086::getCompressionMethods(int method, int level) const
{
    return Packer::getDefaultCompressionMethods_8(method, level);
}


const int *PackElks8086::getFilters() const
{
    return NULL;
}


/*************************************************************************
// common util routines
**************************************************************************/

int PackElks8086::decompressKernel()
{
    // not used
    return 0;
}


void PackElks8086::readKernel()
{
    int klen = file_size - setup_size;
    fi->seek(0, SEEK_SET);

    setup_buf.alloc(setup_size);
    fi->readx(setup_buf, setup_size);

    ibuf.alloc(klen);
    fi->readx(ibuf, klen);

    obuf.allocForCompression(klen);

    ph.u_len = klen;
    ph.filter = 0;
}


/*************************************************************************
//
**************************************************************************/

void PackElks8086::buildLoader(const Filter *ft)
{
    // prepare loader
    initLoader(nrv_loader, sizeof(nrv_loader));
    // FIXME
    UNUSED(ft);
}


void PackElks8086::pack(OutputFile *fo)
{
    readKernel();

    // prepare filter
    Filter ft(ph.level);
    ft.buf_len = ph.u_len;
    ft.addvalue = kernel_entry;
    // compress
    compressWithFilters(&ft, overlap_range, NULL_cconf);

    const unsigned lsize = getLoaderSize();
    MemBuffer loader(lsize);
    memcpy(loader, getLoader(), lsize);

    patchPackHeader(loader, lsize);
#if 0
    // FIXME
    defineFilterSymbols(&ft);
    defineDecompressorSymbols();
    //patch_le32(loader, lsize, "ESI1", zimage_offset + lsize);
    //patch_le32(loader, lsize, "KEIP", kernel_entry);
    //patch_le32(loader, lsize, "STAK", stack_during_uncompression);
#endif

    boot_sect_t * const bs = (boot_sect_t *) ((unsigned char *) setup_buf);
    bs->sys_size = ALIGN_UP(lsize + ph.c_len, 16) / 16;

    fo->write(setup_buf, setup_buf.getSize());
    fo->write(loader, lsize);
    fo->write(obuf, ph.c_len);
#if 0
    printf("%-13s: setup        : %8ld bytes\n", getName(), (long) setup_buf.getSize());
    printf("%-13s: loader       : %8ld bytes\n", getName(), (long) lsize);
    printf("%-13s: compressed   : %8ld bytes\n", getName(), (long) ph.c_len);
#endif

    // finally check the compression ratio
    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();
}


/*************************************************************************
// unpack
**************************************************************************/

int PackElks8086::canUnpack()
{
    if (readFileHeader() != getFormat())
        return false;
    fi->seek(setup_size, SEEK_SET);
    return readPackHeader(1024) ? 1 : -1;
}


void PackElks8086::unpack(OutputFile *)
{
    // no uncompression support for this format yet

    // FIXME: but we could write the uncompressed "Image" image

    throwCantUnpack("build a new kernel instead :-)");
}


#endif /* if 0 */


/*
vi:ts=4:et
*/


