/* p_com.cpp --

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
#include "p_com.h"

static const
#include "stub/l_com.h"

#define STACKSIZE 0x60

//#define TESTING


/*************************************************************************
//
**************************************************************************/

int PackCom::getCompressionMethod() const
{
    if (M_IS_NRV2B(opt->method))
        return M_NRV2B_LE16;
#if 0
    // NOT IMPLEMENTED
    if (M_IS_NRV2D(opt->method))
        return M_NRV2D_LE16;
#endif
    return M_NRV2B_LE16;
}


const int *PackCom::getFilters() const
{
    static const int filters[] = { 0x06, 0x03, 0x04, 0x01, 0x05, 0x02, -1 };
    return filters;
}


/*************************************************************************
//
**************************************************************************/

bool PackCom::canPack()
{
    unsigned char buf[128];

    fi->readx(buf,128);
    if (memcmp(buf,"MZ",2) == 0 || memcmp(buf,"ZM",2) == 0 // .exe
        || memcmp (buf,"\xff\xff\xff\xff",4) == 0) // .sys
        return false;
    if (!fn_has_ext(fi->getName(),"com"))
        return false;
    if (find_le32(buf,128,UPX_MAGIC_LE32))
        throwAlreadyPacked();
    if (file_size < 1024)
        throwCantPack("file is too small");
    if (file_size > 0xFF00)
        throwCantPack("file is too big for dos/com");
    return true;
}


/*************************************************************************
//
**************************************************************************/

void PackCom::patchLoader(OutputFile *fo,
                          upx_byte *loader, int lsize,
                          unsigned calls, unsigned overlapoh)
{
    const int filter_id = ph.filter;
    const int e_len = getLoaderSection("COMCUTPO");
    const int d_len = lsize - e_len;
    assert(e_len > 0 && e_len < 256);
    assert(d_len > 0 && d_len < 256);

    const unsigned upper_end = ph.u_len + overlapoh + d_len + 0x100;
    if (upper_end + STACKSIZE > 0xfffe)
        throwNotCompressible();

    if (filter_id)
    {
        assert(calls > 0);
        patch_le16(loader,lsize,"CT",calls);
    }
    // NOTE: Depends on: decompr_start == cutpoint+1 !!!
    patch_le16(loader,e_len,"JM",upper_end - 0xff - d_len - getLoaderSection("UPX1HEAD"));
    loader[getLoaderSection("COMSUBSI") - 1] = (upx_byte) -e_len;
    patch_le16(loader,e_len,"DI",upper_end);
    patch_le16(loader,e_len,"SI",ph.c_len + lsize + 0x100);
    patch_le16(loader,e_len,"CX",ph.c_len + lsize);
    patch_le16(loader,e_len,"SP",upper_end + STACKSIZE);

    // write loader + compressed file
    fo->write(loader,e_len);            // entry
    fo->write(obuf,ph.c_len);
    fo->write(loader+e_len,d_len);      // decompressor
#if 0
    printf("%-13s: entry        : %8ld bytes\n", getName(), (long) e_len);
    printf("%-13s: compressed   : %8ld bytes\n", getName(), (long) ph.c_len);
    printf("%-13s: decompressor : %8ld bytes\n", getName(), (long) d_len);
#endif
}


int PackCom::buildLoader(const Filter *ft)
{
    const int filter_id = ft->id;
    initLoader(nrv2b_loader,sizeof(nrv2b_loader));
    addLoader("COMMAIN1""COMSUBSI",
              filter_id ? "COMCALLT" : "",
              "COMMAIN2""UPX1HEAD""COMCUTPO""NRV2B160",
              filter_id ? "NRVDDONE" : "NRVDRETU",
              "NRVDECO1",
              ph.max_offset_found <= 0xd00 ? "NRVLED00" : "NRVGTD00",
              "NRVDECO2""NRV2B169",
              NULL
             );
    if (filter_id)
        addFilter16(filter_id);
    return getLoaderSize();
}


void PackCom::addFilter16(int filter_id)
{
    assert(filter_id > 0);
    assert(isValidFilter(filter_id));

    if (filter_id % 3 == 0)
        addLoader("CALLTR16",
                  filter_id < 4 ? "CT16SUB0" : "",
                  filter_id < 4 ? "" : (opt->cpu == opt->CPU_8086 ? "CT16I086" : "CT16I286""CT16SUB0"),
                  "CALLTRI2",
                  getFormat() == UPX_F_DOS_COM ? "CORETURN" : "",
                  NULL
                 );
    else
        addLoader(filter_id%3 == 1 ? "CT16E800" : "CT16E900",
                  "CALLTRI5",
                  getFormat() == UPX_F_DOS_COM ? "CT16JEND" : "CT16JUL2",
                  filter_id < 4 ? "CT16SUB1" : "",
                  filter_id < 4 ? "" : (opt->cpu == opt->CPU_8086 ? "CT16I087" : "CT16I287""CT16SUB1"),
                  "CALLTRI6",
                  NULL
                 );
}


/*************************************************************************
//
**************************************************************************/

void PackCom::pack(OutputFile *fo)
{
    // read file
    ibuf.alloc(file_size);
    obuf.allocForCompression(file_size);
    fi->seek(0,SEEK_SET);
    fi->readx(ibuf,file_size);

    // prepare packheader
    ph.u_len = file_size;
    ph.filter = 0;
    // prepare filter
    Filter ft(opt->level);
    ft.addvalue = getCallTrickOffset();
    // prepare other settings
    const unsigned overlap_range = ph.u_len < 0xFE00 - ft.addvalue ? 32 : 0;
    unsigned overlapoh;

    int strategy = -1;      // try the first working filter
    if (opt->filter >= 0 && isValidFilter(opt->filter))
        // try opt->filter or 0 if that fails
        strategy = -2;
    else if (opt->all_filters || opt->level > 9)
        // choose best from all available filters
        strategy = 0;
    else if (opt->level == 9)
        // choose best from the first 4 filters
        strategy = 4;
    compressWithFilters(&ft, &overlapoh, overlap_range, strategy);

    const int lsize = getLoaderSize();
    MemBuffer loader(lsize);
    memcpy(loader,getLoader(),lsize);
    putPackHeader(loader,lsize);

    const unsigned calls = ft.id % 3 ? ft.lastcall - 2 * ft.calls : ft.calls;
    patchLoader(fo, loader, lsize, calls, overlapoh);

    // verify
    verifyOverlappingDecompression(&obuf, overlapoh);
}


/*************************************************************************
//
**************************************************************************/

int PackCom::canUnpack()
{
    if (!readPackHeader(128, 0))
        return false;
    if (file_size <= (off_t) ph.c_len)
        return false;
    return true;
}


/*************************************************************************
//
**************************************************************************/

void PackCom::unpack(OutputFile *fo)
{
    ibuf.alloc(file_size);
    obuf.allocForUncompression(ph.u_len);

    // read whole file
    fi->seek(0,SEEK_SET);
    fi->readx(ibuf,file_size);

    // get compressed data offset
    int e_len = ph.buf_offset + ph.getPackHeaderSize();
    if (file_size <= e_len + (off_t)ph.c_len)
        throwCantUnpack("file damaged");

    // decompress
    decompress(ibuf+e_len,obuf);

    // unfilter
    Filter ft(ph.level);
    ft.init(ph.filter, getCallTrickOffset());
    ft.unfilter(obuf,ph.u_len);

    // write decompressed file
    if (fo)
        fo->write(obuf,ph.u_len);
}


/*
vi:ts=4:et
*/

