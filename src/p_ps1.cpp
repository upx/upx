/* p_ps1.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2001 Laszlo Molnar
   Copyright (C) 2002      Jens Medoch

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
#include "stub/l_ps1.h"


#define MIPS_HI(a)      (((a) >> 16) /*+(((a)&0x8000)>>15)*/)
#define MIPS_LO(a)      ((a) & 0xffff)
#define MIPS_JP(a)      ((0x08 <<24) | (((a) & 0x0fffffff) >> 2))
#define CHK_ALIGNED(a,b) ((b) - ((a) % (b)))

#define PS_HDR_SIZE     2048  // one CD sector in bytes
#define PS_MAX_SIZE     0x1e8000

#define IH_BKUP         (10*sizeof(LE32))
#define EIGHTBIT


/*************************************************************************
//
**************************************************************************/

PackPs1::PackPs1(InputFile *f) :
    super(f)
{
    COMPILE_TIME_ASSERT(sizeof(ps1_exe_t) == 188);
    COMPILE_TIME_ASSERT(IH_BKUP == 40);

    fdata_size = cfile_size = 0;
    sa_cnt = 0;
}

const int *PackPs1::getCompressionMethods(int method, int level) const
{
#ifdef EIGHTBIT
    return Packer::getDefaultCompressionMethods_8(method, level);
#else
    return Packer::getDefaultCompressionMethods_LE32(method, level);
#endif
}

const int *PackPs1::getFilters() const
{
    return NULL;
}


// functions for marker handling
int PackPs1::patch_mips_le16(void *b, int blen, const void *old, unsigned new_)
{
    unsigned char w[2];

    set_le16(&w,get_be16(old));
    return patch_le16(b, blen, &w, new_);
}

int PackPs1::patch_mips_le32(void *b, int blen, const void *old, unsigned new_)
{
    unsigned char w[4];

    set_le32(&w,get_be32(old));
    return patch_le32(b, blen, &w, new_);
}

int PackPs1::patch_hi_lo(void *b, int blen, const void *old_hi, const void *old_lo, unsigned new_)
{
    patch_mips_le16(b, blen, old_lo, MIPS_LO(new_));
    patch_mips_le16(b, blen, old_hi, MIPS_HI(new_));
    return 1;
}


/*************************************************************************
//
**************************************************************************/

bool PackPs1::canPack()
{
    unsigned char buf[256];
    fdata_size = file_size-PS_HDR_SIZE;

    fi->seek(0, SEEK_SET);
    fi->readx(&ih, sizeof(ih));

    if ((memcmp(&ih.id,"PS-X EXE",8) != 0) && (memcmp(&ih.id,"EXE X-SP",8) != 0))
        return false;
    fi->readx(buf, sizeof(buf));
    checkAlreadyPacked(&buf, sizeof(buf));
    if (fdata_size != ih.tx_len || (ih.tx_len & 3))
    {
        if (!opt->force)
            throwCantPack("check header for file size (try --force)");
        cfile_size = fdata_size;
    }
    if (file_size <= (PS_HDR_SIZE*3) && !opt->force)
        throwCantPack("file is too small (try --force)");
    if (file_size > PS_MAX_SIZE && !opt->force)
        throwCantPack("file is too big (try --force)");
    return true;
}


/*************************************************************************
//
**************************************************************************/

int PackPs1::buildLoader(const Filter *)
{
    initLoader(nrv_loader,sizeof(nrv_loader));
    addLoader("PSXPREP0","PSXSTSZ0","PSXMAIN0",
              ih.tx_ptr&0xffff ?  "PSXJSTA0" : "PSXJSTH0",
              "PSXDECO0",
              NULL);
#ifdef EIGHTBIT
    if (ph.method == M_NRV2B_8)
        addLoader("PSXN2BD0", NULL);
    else if (ph.method == M_NRV2D_8)
        addLoader("PSXN2DD0", NULL);
    else if (ph.method == M_NRV2E_8)
        addLoader("PSXN2ED0", NULL);
#else
   if (ph.method == M_NRV2B_LE32)
        addLoader("PSXN2BD0", NULL);
    else if (ph.method == M_NRV2D_LE32)
        addLoader("PSXN2DD0", NULL);
    else if (ph.method == M_NRV2E_LE32)
        addLoader("PSXN2ED0", NULL);
#endif
    else
        throwInternalError("unknown compression method");
    if (sa_cnt)
        addLoader((sa_cnt > 0xfffc) ? "MSETBIG0" : "MSETSML0",    // set small/big memset
                  (ih.tx_len & 3)   ? "MSETUAL0" : "MSETALG0",    // un/aligned memset
                  NULL);
    addLoader("PSXEXIT0", "IDENTSTR", "PSXPHDR0", NULL);
    return getLoaderSize();
}


/*************************************************************************
//
**************************************************************************/

void PackPs1::pack(OutputFile *fo)
{

    ibuf.alloc(fdata_size);
    obuf.allocForCompression(fdata_size);
    upx_byte *p_scan = ibuf+(fdata_size-1);

    // read file
    fi->seek(PS_HDR_SIZE,SEEK_SET);
    fi->readx(ibuf,fdata_size);

    // this scans the end of file for 2048 bytes sector alignment
    // this should be padded with zeros
    while (!(*p_scan--)) { if ((sa_cnt += 1) > (0xfffc<<3)) break; }
    if (sa_cnt > 0xfffc)
        sa_cnt = ALIGN_DOWN(sa_cnt,8);
    else
        sa_cnt = ALIGN_DOWN(sa_cnt,4);

    // prepare packheader
    ph.u_len = (fdata_size - sa_cnt);
    ph.filter = 0;

    Filter ft(ph.level);
    // compress (max_match = 65535)
    compressWithFilters(&ft, 512, 0, NULL, 0, 65535, 0, 0);

    if (ph.overlap_overhead <= sa_cnt)
        overlap = 0;
    else
    {
        if (!opt->force)
            throwCantPack("packed data overlap (try --force)");
        else
        {
            overlap = ALIGN_UP((ph.overlap_overhead-sa_cnt),4);
            opt->info_mode += !opt->info_mode ? 1 : 0;
            infoWarning("%s will load to a %d bytes higher offset",fi->getName(),overlap);
        }
    }

    memcpy(&oh, &ih, sizeof(ih));
    memcpy(&oh.ih_bkup, &ih.epc, IH_BKUP);
    oh.ih_csum = upx_adler32(&ih.epc, IH_BKUP);

    const int lsize = getLoaderSize();
    const int h_len = lsize-getLoaderSectionStart("IDENTSTR");
    const int e_len = lsize-h_len;
    const int d_len = e_len-getLoaderSectionStart("PSXDECO0");
    int s_len;
    getLoaderSection("PSXSTSZ0",&s_len);   // get size of pushed/poped regs

    MemBuffer loader(lsize);
    memcpy(loader,getLoader(),lsize);

    unsigned pad, pad_code;
    pad = cfile_size ? cfile_size : ih.tx_len;
    pad = ALIGN_DOWN(pad, 4);
    pad_code = CHK_ALIGNED(ph.c_len, 4);

    unsigned decomp_data_start = ih.tx_ptr;

    // set the offset for compressed stuff at the very end of file
    unsigned comp_data_start = (decomp_data_start+pad)-ph.c_len+(overlap ? overlap : 0);

    pad = 0;
    if (!opt->ps1.no_align)
        // align the packed file to mode 2 data sector size (2048)
        pad = CHK_ALIGNED(ph.c_len+pad_code+e_len, 2048);

    const int entry = comp_data_start - e_len - pad_code;
    patchPackHeader(loader,lsize);
    patch_mips_le32(loader,e_len,"JPEP",MIPS_JP(ih.epc));
    if (sa_cnt)
        patch_mips_le16(loader,e_len,"SC",
                        MIPS_LO(sa_cnt > 0xfffc ? sa_cnt >> 3 : sa_cnt));
    if (ih.tx_ptr & 0xffff)
        patch_hi_lo(loader,e_len,"OH","OL",decomp_data_start);
    else
        patch_mips_le16(loader,e_len,"OH",decomp_data_start>>16);
    patch_hi_lo(loader,e_len,"CH","CL",comp_data_start);
    patch_hi_lo(loader,e_len,"DH","DL",entry+(e_len-d_len));
    patch_mips_le16(loader,e_len,"LS",d_len+s_len);

    // set the file load address
    oh.tx_ptr = entry-pad;
    // set the correct file len in header
    oh.tx_len = (ph.c_len+e_len+pad+pad_code);
    // set the code entry
    oh.epc = entry;

    // write loader + compressed file
    upx_bytep paddata = ibuf;
    memset(paddata,0,fdata_size);

    fo->write(&oh,sizeof(oh));

    // id & upx header
    fo->write(loader+e_len,h_len);
    // align the ps exe header (mode 2 sector data size)
    fo->write(paddata,PS_HDR_SIZE-fo->getBytesWritten());
    if (pad)
        // align the file
        fo->write(paddata,pad);
    // entry
    fo->write(loader,e_len);
    if (pad_code)
        // align the mips runtime code
        fo->write(paddata,pad_code);
    fo->write(obuf,ph.c_len);

    // verify
    verifyOverlappingDecompression();

    // finally check the compression ratio
    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();

#if 0
    printf("%-13s: compressed   : %8ld bytes\n", getName(), (long) ph.c_len);
    printf("%-13s: decompressor : %8ld bytes\n", getName(), (long) e_len);

    printf("%-13s: code entry   :%0X8 bytes\n", getName(), (unsigned int) oh.epc);
    printf("%-13s: load address :%0X8 bytes\n", getName(), (unsigned int) oh.tx_ptr);
    printf("%-13s: section size : %8ld bytes\n", getName(), (long) oh.tx_len);
    printf("%-13s: eof in mem IF:%0X8 bytes\n", getName(), (unsigned int) ih.tx_ptr+ih.tx_len);
    printf("%-13s: eof in mem OF:%0X8 bytes\n", getName(), (unsigned int) oh.tx_ptr+oh.tx_len);
#endif
}


/*************************************************************************
//
**************************************************************************/

int PackPs1::canUnpack()
{
    if (!readPackHeader(0x400))
        return false;
    if (file_size <= (off_t) ph.c_len)
        return false;
    return true;
}


/*************************************************************************
//
**************************************************************************/

void PackPs1::unpack(OutputFile *fo)
{
    fdata_size = file_size-PS_HDR_SIZE;
    ibuf.alloc(file_size);

    fi->seek(0x0,SEEK_SET);
    fi->readx(&ih,sizeof(ih));
    fi->seek(PS_HDR_SIZE,SEEK_SET);
    fi->readx(ibuf,fdata_size);

    // test & restore orig exec hdr
    if (ih.ih_csum != upx_adler32(&ih.ih_bkup,IH_BKUP))
        throwCantUnpack("file is possibly modified/hacked/protected; take care!");

    memcpy(&oh, &ih, sizeof(ih));
    memcpy(&oh.epc, &ih.ih_bkup, IH_BKUP);

    // clear backup and checksum of header
    memset(&oh.ih_bkup,0,IH_BKUP+4);

    // check for removed sector alignment
    const unsigned pad = oh.tx_len - ph.u_len;

    obuf.allocForUncompression(ph.u_len+pad);
    memset(obuf,0,obuf.getSize());

    // decompress
    decompress(ibuf+(fdata_size-ph.c_len),obuf);

    // write decompressed file
    if (fo)
    {
        fo->write(&oh,sizeof(oh));
        memset(ibuf,0,fdata_size);
        fo->write(ibuf,PS_HDR_SIZE-fo->getBytesWritten());
        fo->write(obuf,ph.u_len+pad);
    }
}

/*
vi:ts=4:et:nowrap
*/

