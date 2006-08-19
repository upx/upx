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
#include "linker.h"

static const
#include "stub/mipsel.r3000-ps1.h"

#define CD_SEC              2048
#define PS_HDR_SIZE         CD_SEC
#define PS_RAM_SIZE         ram_size
#define PS_MIN_SIZE         (PS_HDR_SIZE*3)
#define PS_MAX_SIZE         ((PS_RAM_SIZE*95) / 100)

#define SZ_IH_BKUP          (10 * sizeof(LE32))
#define HD_CODE_OFS         (sizeof(ps1_exe_t) + sz_cbh)

#define K0_BS               (0x80000000)
#define K1_BS               (0xa0000000)
#define FIX_PSVR            (K1_BS - (ih.epc & K0_BS)) + (PS_HDR_SIZE - HD_CODE_OFS)

// lui / addiu
#define MIPS_HI(a)          (((a) >> 16) + (((a) & 0x8000) >> 15))
#define MIPS_LO(a)          ((a) & 0xffff)
#define MIPS_PC16(a)        ((a) >> 2)
#define MIPS_PC26(a)        (((a) & 0x0fffffff) >> 2)

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
    build_Loader(0), sa_cnt(0), overlap(0), sz_lunc(0), sz_lcpr(0), pad_code(0)
{
    COMPILE_TIME_ASSERT(sizeof(ps1_exe_t) == 136);
    COMPILE_TIME_ASSERT(sizeof(ps1_exe_hb_t) == 44);
    COMPILE_TIME_ASSERT(sizeof(ps1_exe_chb_t) == 5);

    COMPILE_TIME_ASSERT(PS_HDR_SIZE > sizeof(ps1_exe_t));
    COMPILE_TIME_ASSERT(SZ_IH_BKUP == 40);
#if 0 // 1 || defined(WITH_NRV)
    COMPILE_TIME_ASSERT(sizeof(nrv_loader) == 14812);
    COMPILE_TIME_ASSERT(NRV_BOOT_LOADER_CRC32 == 0x0);
#endif
    fdata_size = file_size - PS_HDR_SIZE;
    ram_size = !opt->ps1_exe.do_8mb ? 0x200000 : 0x800000;
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

Linker* PackPs1::newLinker() const
{
    class ElfLinkerMipsLE : public ElfLinker
    {
        typedef ElfLinker super;

        virtual void relocate1(Relocation *rel, upx_byte *location,
                               unsigned value, const char *type)
        {
            if (strcmp(type, "R_MIPS_HI16") == 0)
                set_le16(location, get_le16(location) + MIPS_HI(value));
            else if (strcmp(type, "R_MIPS_LO16") == 0)
                set_le16(location, get_le16(location) + MIPS_LO(value));
            else if (strcmp(type, "R_MIPS_PC16") == 0)
            {
                value -= rel->section->offset + rel->offset;
                set_le16(location, get_le16(location) + MIPS_PC16(value));
            }
            else if (strcmp(type, "R_MIPS_26") == 0)
                set_le32(location, get_le32(location) + MIPS_PC26(value));
            else if (strcmp(type, "R_MIPS_32") == 0)
                set_le32(location, get_le32(location) + value);
            else
                super::relocate1(rel, location, value, type);
        }
    };
    return new ElfLinkerMipsLE;
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
    if (memcmp(&ih.id, "PS-X EXE", 8) != 0 &&
        memcmp(&ih.id, "EXE X-SP", 8) != 0)
        return 0;
    if (ih.text != 0 || ih.data != 0)
        return 0;
    return UPX_F_PS1_EXE;
}

bool PackPs1::readBkupHeader()
{
    fi->seek(sizeof(ps1_exe_t)+8, SEEK_SET);
    fi->readx(&bh, sizeof(bh));

    if (bh.ih_csum != upx_adler32(&bh, SZ_IH_BKUP))
    {
        unsigned char buf[sizeof(bh)];
        fi->seek(sizeof(ps1_exe_t), SEEK_SET);
        fi->readx(buf, sizeof(bh));
        if (!getBkupHeader(buf, (unsigned char *)&bh))
            return false;
    }
    return true;
}

#define INIT_BH_BKUP(p, l)  {(p)->id = '1'; (p)->len = l;}
#define ADLER16(a)          ((a) >> 16 ^ ((a) & 0xffff))

void PackPs1::putBkupHeader(const unsigned char *src, unsigned char *dst, unsigned *len)
{
    unsigned sz_cbh;

    if (src && dst)
    {
        unsigned char *cpr_bh = new unsigned char[MemBuffer::getSizeForCompression(SZ_IH_BKUP)];

        ps1_exe_chb_t * p = (ps1_exe_chb_t * )cpr_bh;

        int r = upx_compress(src, SZ_IH_BKUP,
                             &p->ih_bkup, &sz_cbh, NULL, M_NRV2E_8, 10, NULL, NULL );
        if (r != UPX_E_OK || sz_cbh >= SZ_IH_BKUP)
            throwInternalError("header compression failed");
        INIT_BH_BKUP(p, sz_cbh);
        *len = ALIGN_UP(sz_cbh + sizeof(ps1_exe_chb_t) - 1, 4);
        p->ih_csum = ADLER16(upx_adler32(&ih.epc, SZ_IH_BKUP));
        memcpy(dst, cpr_bh, SZ_IH_BKUP);
        delete [] cpr_bh;
    }
    else
        throwInternalError("failed to create backup header");
}

#define ADLER16_HI(a,b)     ((((a) & 0xffff) ^ (b)) << 16)
#define ADLER16_LO(a,b)     (((a) >> 16) ^ (b))
#define RE_ADLER16(a,b)     (ADLER16_HI(a,b) | ADLER16_LO(a,b))

bool PackPs1::getBkupHeader(unsigned char *p, unsigned char *dst)
{
    ps1_exe_chb_t *src = (ps1_exe_chb_t*)p;

    if (src && src->id == '1' && dst)
    {
        unsigned char *unc_bh = new unsigned char[MemBuffer::getSizeForUncompression(SZ_IH_BKUP)];

        unsigned sz_bh = SZ_IH_BKUP;
        int r = upx_decompress((const unsigned char *)&src->ih_bkup, src->len,
                               unc_bh, &sz_bh, M_NRV2E_8, NULL );
        if (r != UPX_E_OK || sz_bh != SZ_IH_BKUP)
            throwInternalError("header decompression failed");
        unsigned ad = upx_adler32(unc_bh, SZ_IH_BKUP);
        unsigned ch = src->ih_csum;
        if (ad != RE_ADLER16(ad,ch))
            throwInternalError("backup header damaged");
        memcpy(dst, unc_bh, SZ_IH_BKUP);
        delete [] unc_bh;
    }
    else
        return false;
    return true;
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
//
**************************************************************************/

bool PackPs1::canPack()
{
    unsigned char buf[PS_HDR_SIZE - sizeof(ps1_exe_t)];

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
    if (!opt->force && file_size > (off_t) PS_MAX_SIZE)
        throwCantPack("file is too big (try --force)");
    return true;
}


/*************************************************************************
//
**************************************************************************/

void PackPs1::buildPS1Loader(const Filter *)
{
    const char *p = NULL;
    unsigned gap = 0;

    if (ph.method == M_NRV2B_8)
        p = isCon ? "nrv2b.small,gb.8bit.sub,nrv.small.done" :
                    "nrv2b.8bit";
    else if (ph.method == M_NRV2D_8)
        p = isCon ? "nrv2d.small,gb.8bit.sub,nrv.small.done" :
                    "nrv2d.8bit";
    else if (ph.method == M_NRV2E_8)
        p = isCon ? "nrv2e.small,gb.8bit.sub,nrv.small.done" :
                    "nrv2e.8bit";
    else if (ph.method == M_NRV2B_LE32)
        p = isCon ? "nrv2b.small,gb.32bit.sub,nrv.small.done" :
                    "nrv2b.32bit";
    else if (ph.method == M_NRV2D_LE32)
        p = isCon ? "nrv2d.small,gb.32bit.sub,nrv.small.done" :
                    "nrv2d.32bit";
    else if (ph.method == M_NRV2E_LE32)
        p = isCon ? "nrv2e.small,gb.32bit.sub,nrv.small.done" :
                    "nrv2e.32bit";
    else if (ph.method == M_LZMA)
        p = "nrv2b.small,gb.8bit.sub,nrv.small.done,lzma.prep";
    else
        throwInternalError("unknown compression method");

    if ((gap = ALIGN_GAP((ph.c_len + (isCon ? sz_lcpr : 0)),4)))
        pad_code = gap;
    else
        pad_code = 0;
    linker->addSection("pad.code", &pad_code, gap, 0);


    if (isCon)
        if (ph.method == M_LZMA)
            addLoader("con.start", p,
                      ih.tx_ptr & 0xffff ?  "dec.ptr" : "dec.ptr.hi",
                      "con.entry", "pad.code", "lzma.exec", NULL);
        else
            addLoader("con.start", "con.mcpy",
                      ph.c_len & 3 ? "con.padcd" : "",
                      ih.tx_ptr & 0xffff ?  "dec.ptr" : "dec.ptr.hi",
                      "con.entry", p,
                      sa_cnt ? sa_cnt > (0x10000 << 2) ? "memset.long" : "memset.short" : "",
                      "con.exit", "pad.code", NULL);
    else
        if (ph.method == M_LZMA)
            addLoader("cdb.start.lzma", "pad.code", "cdb.entry.lzma", p, "cdb.lzma.cpr",
                      ih.tx_ptr & 0xffff ?  "dec.ptr" : "dec.ptr.hi",
                      "lzma.exec", NULL);
        else
            addLoader("cdb.start", "pad.code", "cdb.entry",
                      ih.tx_ptr & 0xffff ?  "cdb.dec.ptr" : "cdb.dec.ptr.hi",
                      p,
                      sa_cnt ? sa_cnt > (0x10000 << 2) ? "memset.long" : "memset.short" : "",
                      "cdb.exit", "pad.code", NULL);

    addLoader("UPX1HEAD", "IDENTSTR", NULL);
}

int PackPs1::buildLoader(const Filter *)
{
    unsigned sa_tmp = sa_cnt;
    if (M_IS_NRV2B(ph.method) || M_IS_NRV2D(ph.method) || M_IS_NRV2E(ph.method) || ph.method == M_LZMA)
    {
        if (ph.overlap_overhead > sa_cnt)
        {
            if (!opt->force)
            {
                infoWarning("not in-place decompressible");
                throwCantPack("packed data overlap (try --force)");
            }
            else
                sa_tmp += overlap = ALIGN_UP((ph.overlap_overhead-sa_tmp),4);
        }
        if (ph.method == M_LZMA && !build_Loader)
        {
            initLoader(nrv_loader,sizeof(nrv_loader));
            addLoader(isCon ? "LZMA_DEC20" : "LZMA_DEC10", "lzma.init", NULL);
            addLoader(sa_tmp > (0x10000 << 2) ? "memset.long" : "memset.short",
                      "con.exit", NULL);
        }
        else if (ph.method == M_LZMA && build_Loader)
        {
            unsigned char *cprLoader = new unsigned char[MemBuffer::getSizeForCompression(sz_lunc)];
            int r = upx_compress(getLoader(), sz_lunc, cprLoader, &sz_lcpr,
                                 NULL, M_NRV2B_8, 10, NULL, NULL );
            if (r != UPX_E_OK || sz_lcpr >= sz_lunc)
                throwInternalError("loader compression failed");
            initLoader(nrv_loader,sizeof(nrv_loader), 0,
                      (ph.method != M_LZMA || isCon) ? 0 : 1);
            linker->addSection("lzma.exec", cprLoader, sz_lcpr, 0);
            delete [] cprLoader;
            buildPS1Loader();
        }
        else
        {
            initLoader(nrv_loader,sizeof(nrv_loader), 0,
                      (ph.method != M_LZMA || isCon) ? 0 : 1);
            buildPS1Loader();
        }
    }
    else
        throwInternalError("unknown compression method");

    freezeLoader();
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
    upx_compress_config_t cconf; cconf.reset();
    cconf.conf_ucl.max_match = 65535;
    cconf.conf_lzma.max_num_probs = 1846 + (768 << 4); // ushort: ~28KB stack
    compressWithFilters(&ft, sa_cnt, 0, NULL, &cconf);

    if (overlap)
    {
        opt->info_mode += !opt->info_mode ? 1 : 0;
        infoWarning("%s: memory overlap %d bytes", fi->getName(), overlap);
        sa_cnt += overlap;
    }

    unsigned lzma_init = 0;

    if (ph.method == M_LZMA)
    {
        sz_lunc = getLoaderSize();

        lzma_init = 0u - (sz_lunc - linker->getSymbolOffset("lzma.init"));
        defineDecompressorSymbols();
        linker->defineSymbol("lzma_decoder", linker->getSymbolOffset(isCon ? "LZMA_DEC20" : "LZMA_DEC10"));
        linker->defineSymbol("entry", ih.epc);
        linker->defineSymbol("SC",
                             sa_cnt > (0x10000 << 2) ? sa_cnt >> 5 : sa_cnt >> 2);
        linker->relocate();

        build_Loader = 1;
        buildLoader(&ft);
    }

    memcpy(&oh, &ih, sizeof(ih));

    unsigned sz_cbh;
    putBkupHeader((const unsigned char *)&ih.epc, (unsigned char *)&bh, &sz_cbh);

    if (ih.is_ptr == 0)
        oh.is_ptr = PS_RAM_SIZE-0x10;

    if (ih.da_ptr != 0 || ih.da_len != 0 ||
        ih.bs_ptr != 0 || ih.bs_len != 0)
        oh.da_ptr = oh.da_len =
        oh.bs_ptr = oh.bs_len = 0;

    const int lsize = getLoaderSize();
    MemBuffer loader(lsize);
    memcpy(loader, getLoader(), lsize);

    unsigned filelen = ALIGN_UP(ih.tx_len, 4);

    const unsigned decomp_data_start = ih.tx_ptr;
    const unsigned comp_data_start = (decomp_data_start + filelen + overlap) - ph.c_len;

    const int h_len = lsize - getLoaderSectionStart("UPX1HEAD");
    int d_len = 0;
    int e_len = 0;

    if (isCon)
    {
        e_len = lsize - h_len;
        d_len = e_len - getLoaderSectionStart("con.entry");
    }
    else
    {
        d_len = (lsize - h_len) - getLoaderSectionStart((ph.method == M_LZMA) ? "cdb.entry.lzma" : "cdb.entry");
        e_len = (lsize - d_len) - h_len;
    }

    linker->defineSymbol("entry", ih.epc);
    linker->defineSymbol("SC", MIPS_LO(sa_cnt > (0x10000 << 2) ?
                                       sa_cnt >> 5 : sa_cnt >> 2));
    linker->defineSymbol("DECO",decomp_data_start);

    linker->defineSymbol("LS", (ph.method == M_LZMA ? sz_lunc + 16 : (d_len-pad_code)));

    const unsigned entry = comp_data_start - e_len;
    oh.epc = oh.tx_ptr = entry;
    oh.tx_len = ph.c_len + e_len;

    unsigned pad = 0;

    if (!opt->ps1_exe.no_align)
    {
        pad = oh.tx_len;
        oh.tx_len = ALIGN_UP(oh.tx_len, CD_SEC);
        pad = oh.tx_len - pad;
        oh.tx_ptr -= pad;
    }

    ibuf.clear(0,fdata_size);
    upx_bytep paddata = ibuf;

    if (ph.method == M_LZMA)
    {
        linker->defineSymbol("lzma_init_off", lzma_init);
        defineDecompressorSymbols();
    }
    if (isCon)
    {
        linker->defineSymbol("PAD", pad_code);
        if (ph.method == M_LZMA)
            linker->defineSymbol("DCRT", (entry + getLoaderSectionStart("lzma.exec")));
        else
            linker->defineSymbol("DCRT", (entry + (e_len - d_len)));
        linker->relocate();
        memcpy(loader, getLoader(), lsize);
        patchPackHeader(loader, lsize);
    }
    else
    {
        linker->defineSymbol("PSVR", FIX_PSVR);
        linker->defineSymbol("CPDO", comp_data_start);
        if (ph.method == M_LZMA)
            linker->defineSymbol("lzma_cpr", getLoaderSectionStart("lzma.exec")
                                             - getLoaderSectionStart("cdb.entry.lzma"));

        linker->relocate();
        memcpy(loader, getLoader(), lsize);
        patchPackHeader(loader,lsize);
    }

    // ps1_exe_t structure
    fo->write(&oh, sizeof(oh));
    fo->write(&bh, sz_cbh);
    // decompressor
    fo->write(loader + e_len, isCon ? h_len : (d_len + h_len));

    // header size is 2048 bytes + sector alignment
    fo->write(paddata, (pad + PS_HDR_SIZE) - fo->getBytesWritten());
    // entry
    fo->write(loader, e_len);
    // compressed body
    fo->write(obuf, ph.c_len);

    verifyOverlappingDecompression();
    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();

#if 0
    printf("%-13s: uncompressed : %8ld bytes\n", getName(), (long) ph.u_len);
    printf("%-13s: compressed   : %8ld bytes\n", getName(), (long) ph.c_len);
    printf("%-13s: decompressor : %8ld bytes\n", getName(), (long) lsize - h_len);
    printf("%-13s: header comp  : %8ld bytes\n", getName(), (long) sz_cbh);
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
    if (!readPackHeader(CD_SEC))
        return false;
    // check header as set by packer
    if (!readBkupHeader() || ph.c_len >= fdata_size)
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
    memcpy(&oh.epc, &bh, SZ_IH_BKUP);

    // check for removed sector alignment
    assert(oh.tx_len >= ph.u_len);
    const unsigned pad = oh.tx_len - ph.u_len;

    ibuf.alloc(fdata_size > PS_HDR_SIZE ? fdata_size : PS_HDR_SIZE);
    obuf.allocForUncompression(ph.u_len, pad);

    fi->seek(PS_HDR_SIZE, SEEK_SET);
    fi->readx(ibuf, fdata_size);

    // decompress
    decompress(ibuf + (fdata_size - ph.c_len), obuf);

    // write decompressed file
    if (fo)
    {
        // write header
        fo->write(&oh, sizeof(oh));
        // align the ps exe header (mode 2 sector data size)
        ibuf.clear();
        fo->write(ibuf, PS_HDR_SIZE - fo->getBytesWritten());
        // write uncompressed data + pad
        obuf.clear(ph.u_len, pad);
        fo->write(obuf, ph.u_len + pad);
    }
}

/*
vi:ts=4:et:nowrap
*/
