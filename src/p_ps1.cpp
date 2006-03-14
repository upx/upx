/* p_ps1.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2006 Laszlo Molnar
   Copyright (C) 2002-2006 Jens Medoch
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
   <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>

   Jens Medoch
   <jssg@users.sourceforge.net>
 */


#include "conf.h"
#include "file.h"
#include "filter.h"
#include "packer.h"
#include "p_ps1.h"

static const
#include "stub/l_ps1b.h"
static const
#include "stub/l_ps1c.h"


#define MIPS_HI(a)          ((a) >> 16)
#define MIPS_LO(a)          ((a) & 0xffff)
#define MIPS_JP(a)          ((0x08 << 24) | (((a) & 0x0fffffff) >> 2))
#define TIL_ALIGNED(a,b)    (((b) - ((a) & ((b) - 1))) & (b) - 1)

#define CD_SEC              2048
#define PS_HDR_SIZE         CD_SEC
#define PS_RAM_SIZE         0x200000
#define PS_MIN_SIZE         (PS_HDR_SIZE*3)
#define PS_MAX_SIZE         0x1e8000

#define SZ_IH_BKUP          (10 * sizeof(LE32))
#define HD_CODE_OFS         (sizeof(ps1_exe_t))

#define K0_BS               (0x80000000)
#define K1_BS               (0xa0000000)
#define FIX_PSVR            (K1_BS - (ih.epc & K0_BS)) + (PS_HDR_SIZE - HD_CODE_OFS)


/*************************************************************************
// ps1 exe looks like this:
// 1. <header>  2048 bytes
// 2. <body>    plain binary
//
// header:  contains the ps1_exe_t structure 188 bytes at offset zero
//          rest is filled with zeros to reach the required
//          cd mode 2 data sector size of 2048 bytes
// body:    contains the binary data / code of the executable
//          reqiures: executable code must be aligned to 4
//                    must be aligned to 2048 to run from a CD
//          optional: not aligned to 2048 (for console run only)
**************************************************************************/

PackPs1::PackPs1(InputFile *f) :
    super(f),
    isCon(!opt->ps1_exe.boot_only), is32Bit(!opt->ps1_exe.do_8bit),
    overlap(0), sa_cnt(0)
{
    COMPILE_TIME_ASSERT(sizeof(ps1_exe_t) == 188);
    COMPILE_TIME_ASSERT(PS_HDR_SIZE > sizeof(ps1_exe_t));
    COMPILE_TIME_ASSERT(SZ_IH_BKUP == 40);
#if 1 || defined(WITH_NRV)
    COMPILE_TIME_ASSERT(sizeof(nrv_boot_loader) == 3935);
    COMPILE_TIME_ASSERT(NRV_BOOT_LOADER_CRC32 == 0x0ac25782);
    COMPILE_TIME_ASSERT(sizeof(nrv_con_loader) == 2829);
    COMPILE_TIME_ASSERT(NRV_CON_LOADER_CRC32 == 0x923b55c4);
#endif
    fdata_size = file_size - PS_HDR_SIZE;
}


const int *PackPs1::getCompressionMethods(int method, int level) const
{
    if (is32Bit)
        return Packer::getDefaultCompressionMethods_le32(method, level);
    else
        return Packer::getDefaultCompressionMethods_8(method, level);
}


const int *PackPs1::getFilters() const
{
    return NULL;
}


/*************************************************************************
// util
//   readFileHeader() reads ih and checks for illegal values
//   checkFileHeader() checks ih for legal but unsupported values
**************************************************************************/

int PackPs1::readFileHeader()
{
    fi->seek(0, SEEK_SET);
    fi->readx(&ih, sizeof(ih));
    if (memcmp(&ih.id,"PS-X EXE",8) != 0 &&
        memcmp(&ih.id,"EXE X-SP",8) != 0)
        return 0;
    if (ih.text != 0 || ih.data != 0)
        return 0;
    return UPX_F_PS1_EXE;
}


bool PackPs1::checkFileHeader()
{
    if (fdata_size != ih.tx_len || (ih.tx_len & 3))
    {
        if (!opt->force)
            throwCantPack("file size entry damaged (try --force)");
        else
        {
            opt->info_mode += !opt->info_mode ? 1 : 0;
            infoWarning("fixing damaged header, keeping backup file");
            opt->backup = 1;
            ih.tx_len = fdata_size;
        }
    }
    if (!opt->force &&
       (ih.da_ptr != 0 || ih.da_len != 0 ||
        ih.bs_ptr != 0 || ih.bs_len != 0))
    {
        infoWarning("unsupported header field entry");
        return false;
    }
    if (!opt->force && ih.is_ptr == 0)
    {
        infoWarning("stack pointer field empty");
        return false;
    }
    return true;
}


/*************************************************************************
// patch util
**************************************************************************/

void PackPs1::patch_mips_le(void *b, int blen, const void *old, unsigned new_)
{
    size_t patch_len = strlen((const char*)old);

    if (patch_len == 2)
    {
        unsigned char w[2];

        set_le16(w, get_be16(old));
        patch_le16(b, blen, w, MIPS_LO(new_));
    }
    else if (patch_len == 4)
    {
        unsigned char w[4];

        set_le32(w, get_be32(old));
        int boff = find(b, blen, w, 4);

        if (boff == -1)
        {
            patch_le16(b, blen, &w[0], MIPS_LO(new_));
            patch_le16(b, blen, &w[2], MIPS_HI(new_));
        }
        else
            patch_le32((unsigned char *)b + boff, (blen-boff), &w, new_);
    }
    else
        throwInternalError("bad marker length");
}


/*************************************************************************
//
**************************************************************************/

bool PackPs1::canPack()
{
    unsigned char buf[PS_HDR_SIZE-HD_CODE_OFS];

    if (!readFileHeader())
        return false;

    fi->readx(buf, sizeof(buf));
    checkAlreadyPacked(buf, sizeof(buf));

    for (unsigned i = 0; i < sizeof(buf); i++)
        if (buf[i] != 0)
            if (!opt->force)
                throwCantPack("unknown data in header (try --force)");
            else
            {
                opt->info_mode += !opt->info_mode ? 1 : 0;
                infoWarning("clearing header, keeping backup file");
                opt->backup = 1;
                break;
            }
    if (!checkFileHeader())
        throwCantPack("unsupported header flags (try --force)");
    if (!opt->force && file_size < PS_MIN_SIZE)
        throwCantPack("file is too small (try --force)");
    if (!opt->force && file_size > PS_MAX_SIZE)
        throwCantPack("file is too big (try --force)");
    return true;
}


/*************************************************************************
//
**************************************************************************/

int PackPs1::buildLoader(const Filter *)
{
    if (isCon)
        initLoader(nrv_con_loader,sizeof(nrv_con_loader));
    else
        initLoader(nrv_boot_loader,sizeof(nrv_boot_loader));

    addLoader("PS1START",
              isCon ? ph.c_len & 3 ? "PS1PADCD" : "" : "PS1ENTRY",
              ih.tx_ptr & 0xffff ?  "PS1CONHL" : "PS1CONHI",
              isCon ? "PS1ENTRY" : "",
              NULL);

    if (ph.method == M_NRV2B_8)
        addLoader("PS1N2B08", NULL);
    else if (ph.method == M_NRV2D_8)
        addLoader("PS1N2D08", NULL);
    else if (ph.method == M_NRV2E_8)
        addLoader("PS1N2E08", NULL);
    else if (ph.method == M_NRV2B_LE32)
        addLoader("PS1N2B32", NULL);
    else if (ph.method == M_NRV2D_LE32)
        addLoader("PS1N2D32", NULL);
    else if (ph.method == M_NRV2E_LE32)
        addLoader("PS1N2E32", NULL);
    else
        throwInternalError("unknown compression method");

    if (sa_cnt)
        addLoader(sa_cnt > (0x10000 << 2) ? "PS1MSETB" : "PS1MSETS",
                  ih.tx_len & 3 ? "PS1MSETU" : "PS1MSETA",
                  NULL);

    addLoader("PS1EXITC", "IDENTSTR", "PS1PAHDR",
              isCon ? "PS1SREGS" : "",
              NULL);

    return getLoaderSize();
}


/*************************************************************************
//
**************************************************************************/

void PackPs1::pack(OutputFile *fo)
{
    ibuf.alloc(fdata_size);
    obuf.allocForCompression(fdata_size);
    const upx_byte *p_scan = ibuf+fdata_size;

    // read file
    fi->seek(PS_HDR_SIZE,SEEK_SET);
    fi->readx(ibuf,fdata_size);

    // scan EOF for 2048 bytes sector alignment
    // the removed space will secure in-place decompression
    while (!(*--p_scan)) { if (sa_cnt++ > (0x10000<<5) || sa_cnt >= fdata_size-1024) break; }

    if (sa_cnt > (0x10000<<2))
        sa_cnt = ALIGN_DOWN(sa_cnt,32);
    else
        sa_cnt = ALIGN_DOWN(sa_cnt,4);

    // prepare packheader
    ph.u_len = (fdata_size - sa_cnt);
    ph.filter = 0;
    Filter ft(ph.level);

    // compress (max_match = 65535)
    compressWithFilters(&ft, 512, 0, NULL, 0, 65535);

    if (ph.overlap_overhead > sa_cnt)
    {
        if (!opt->force)
        {
            infoWarning("not in-place decompressible");
            throwCantPack("packed data overlap (try --force)");
        }
        else
        {
            overlap = ALIGN_UP((ph.overlap_overhead-sa_cnt),4);
            opt->info_mode += !opt->info_mode ? 1 : 0;
            infoWarning("%s: memory overlap %d bytes",fi->getName(),overlap);
        }
    }

    memcpy(&oh, &ih, sizeof(ih));
    memcpy(&oh.ih_bkup, &ih.epc, SZ_IH_BKUP);
    oh.ih_csum = upx_adler32(&ih.epc, SZ_IH_BKUP);

    if (ih.is_ptr == 0)
        oh.is_ptr = PS_RAM_SIZE-0x10;

    if (ih.da_ptr != 0 || ih.da_len != 0 ||
        ih.bs_ptr != 0 || ih.bs_len != 0)
        oh.da_ptr = oh.da_len =
        oh.bs_ptr = oh.bs_len = 0;

    const int lsize = getLoaderSize();
    MemBuffer loader(lsize);
    memcpy(loader, getLoader(), lsize);

    patchPackHeader(loader,lsize);

    unsigned pad = 0;
    unsigned filelen = ALIGN_UP(ih.tx_len, 4);
    unsigned pad_code = TIL_ALIGNED(ph.c_len, 4);

    const unsigned decomp_data_start = ih.tx_ptr;
    const unsigned comp_data_start = ((decomp_data_start + filelen + overlap) - ph.c_len);

    const int h_len = lsize - getLoaderSectionStart("IDENTSTR");
    const int c_len = lsize - h_len;
    int d_len = 0;
    int e_len = 0;

    if (isCon)
    {
        e_len = lsize - h_len;
        d_len = e_len - getLoaderSectionStart("PS1ENTRY");
    }
    else
    {
        d_len = (lsize - h_len) - getLoaderSectionStart("PS1ENTRY");
        e_len = (lsize - d_len) - h_len;
    }

    patch_mips_le(loader,c_len,"JPEP",MIPS_JP(ih.epc));
    if (sa_cnt)
        patch_mips_le(loader,c_len,"SC",
                      MIPS_LO(sa_cnt > (0x10000 << 2) ? sa_cnt >> 5 : sa_cnt >> 2));
    if (ih.tx_ptr & 0xffff)
        patch_mips_le(loader,c_len,"DECO",decomp_data_start);
    else
        patch_mips_le(loader,c_len,"DE",MIPS_HI(decomp_data_start));

    const unsigned entry = comp_data_start - e_len - pad_code;
    oh.tx_ptr = entry;
    oh.tx_len = ph.c_len + e_len + pad_code;
    oh.epc = comp_data_start - e_len - pad_code;

    if (!opt->ps1_exe.no_align)
    {
        pad = oh.tx_len;
        oh.tx_len = ALIGN_UP(oh.tx_len, CD_SEC);
        pad = oh.tx_len - pad;
        oh.tx_ptr -= pad;
    }

    ibuf.clear(0,fdata_size);
    upx_bytep paddata = ibuf;

    if (isCon)
    {
        if (pad_code)
            patch_mips_le(loader, c_len, "PC", pad_code);
        patch_mips_le(loader, c_len, "DCRT", entry + (e_len - d_len));
        patch_mips_le(loader, c_len, "LS",
                      d_len + get_le32(&loader[getLoaderSectionStart("PS1SREGS")]));

        // ps1_exe_t structure 188 bytes
        fo->write(&oh,sizeof(oh));
        // id & upx header
        fo->write(loader + e_len,h_len);
    }
    else
    {
        patch_mips_le(loader, c_len, "CPDO", comp_data_start);
        patch_mips_le(loader, e_len, "PSVR", FIX_PSVR);

        // ps1_exe_t structure 188 bytes
        fo->write(&oh,sizeof(oh));
        // decompressor
        fo->write(loader + e_len, d_len);
        // id & upx header
        fo->write(loader + e_len + d_len, h_len);
    }

    // header size is 2048 bytes
    fo->write(paddata, PS_HDR_SIZE - fo->getBytesWritten());
    // sector alignment
    if (pad)
        fo->write(paddata, pad);
    // entry
    fo->write(loader, e_len);
    // code must be aligned to 4!
    if (pad_code)
        fo->write(paddata, pad_code);
    // compressed binary / data
    fo->write(obuf, ph.c_len);

    verifyOverlappingDecompression();
    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();

#if 0
    printf("%-13s: uncompressed : %8ld bytes\n", getName(), (long) ph.u_len);
    printf("%-13s: compressed   : %8ld bytes\n", getName(), (long) ph.c_len);
    printf("%-13s: decompressor : %8ld bytes\n", getName(), (long) lsize - h_len);
    printf("%-13s: code entry   : %08X bytes\n", getName(), (unsigned int) oh.epc);
    printf("%-13s: load address : %08X bytes\n", getName(), (unsigned int) oh.tx_ptr);
    printf("%-13s: eof in mem IF: %08X bytes\n", getName(), (unsigned int) ih.tx_ptr+ih.tx_len);
    printf("%-13s: eof in mem OF: %08X bytes\n", getName(), (unsigned int) oh.tx_ptr+oh.tx_len);
    char method_name[32+1]; set_method_name(method_name, sizeof(method_name), ph.method, ph.level);
    printf("%-13s: compressor   : %s\n", getName(), method_name);
#endif
}


/*************************************************************************
//
**************************************************************************/

int PackPs1::canUnpack()
{
    if (!readFileHeader())
        return false;
    if (!readPackHeader(1024))
        return false;
    // check header as set by packer
    if (ih.ih_csum != upx_adler32(&ih.ih_bkup, SZ_IH_BKUP) &&
       (ph.c_len >= fdata_size))
        throwCantUnpack("header damaged");
    // generic check
    if (!checkFileHeader())
        throwCantUnpack("unsupported header flags");
    return true;
}


/*************************************************************************
//
**************************************************************************/

void PackPs1::unpack(OutputFile *fo)
{
    // restore orig exec hdr
    memcpy(&oh, &ih, sizeof(ih));
    memcpy(&oh.epc, &ih.ih_bkup, SZ_IH_BKUP);

    // check for removed sector alignment
    assert(oh.tx_len >= ph.u_len);
    const unsigned pad = oh.tx_len - ph.u_len;

    ibuf.alloc(fdata_size > PS_HDR_SIZE ? fdata_size : PS_HDR_SIZE);
    obuf.allocForUncompression(ph.u_len, pad);

    fi->seek(PS_HDR_SIZE, SEEK_SET);
    fi->readx(ibuf, fdata_size);

    // clear backup and checksum of header
    memset(&oh.ih_bkup, 0, SZ_IH_BKUP+4);

    // decompress
    decompress(ibuf + (fdata_size - ph.c_len), obuf);

    // write decompressed file
    if (fo)
    {
        // write header
        fo->write(&oh, sizeof(oh));
        // align the ps exe header (mode 2 sector data size)
        ibuf.clear();
        // write uncompressed data + pad
        fo->write(ibuf, PS_HDR_SIZE - sizeof(oh));
        obuf.clear(ph.u_len, pad);
        fo->write(obuf, ph.u_len + pad);
    }
}

/*
vi:ts=4:et:nowrap
*/

