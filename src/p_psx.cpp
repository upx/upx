/* p_psx.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2001 Laszlo Molnar

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
#include "p_psx.h"
#include "version.h"

static const
#include "stub/l_psx.h"

//#define TESTING

/*************************************************************************
//
**************************************************************************/

const int *PackPsx::getCompressionMethods(int method, int level) const
{
    static const int m_nrv2b[] = { M_NRV2B_LE32, M_NRV2D_LE32, -1 };
    static const int m_nrv2d[] = { M_NRV2D_LE32, M_NRV2B_LE32, -1 };

    if (M_IS_NRV2B(method))
        return m_nrv2b;
    if (M_IS_NRV2D(method))
        return m_nrv2d;
    if (level == 1 || file_size-PS_HDR_SIZE < 512*1024)
        return m_nrv2b;
    return m_nrv2d;

}

const int *PackPsx::getFilters() const
{
    return NULL;
}


/*************************************************************************
//
**************************************************************************/

bool PackPsx::canPack()
{
    right_file_size = 0;
    file_data_size = file_size-PS_HDR_SIZE;

    fi->seek(0, SEEK_SET);
    fi->readx(&ih, sizeof(ih));

    if ((memcmp(&ih.id,"PS-X EXE",8) != 0) && (memcmp(&ih.id,"EXE X-SP",8) != 0))
        return false;
    checkAlreadyPacked(&ih, sizeof(ih));
    if (file_data_size != ih.tx_len || (ih.tx_len & 3))
        if (!opt->force)
            throwCantPack("check header for file size (try --force)");
        else
            right_file_size = file_data_size;
    if (file_size <= (PS_HDR_SIZE*3) && !opt->force)
        throwCantPack("file is too small");
    if (file_size > PS_MAX_SIZE && !opt->force)
        throwCantPack("file is too big (try --force)");

//    i_hdrp = 0;

    return true;
}


/*************************************************************************
//
**************************************************************************/

int PackPsx::buildLoader(const Filter *)
{
    initLoader(nrv_loader,sizeof(nrv_loader));
    addLoader("PSXMAIN0", "PSXDECO0", NULL);
    if (M_IS_NRV2B(ph.method))
        addLoader("PSXN2BD0", NULL);
    else if (M_IS_NRV2D(ph.method))
        addLoader("PSXN2DD0", NULL);
    else
        throwBadLoader();
    if (scan_count)
        addLoader("PSXMSET0", NULL);
    addLoader("PSXEXIT0""PSXPHDR0", NULL);
    return getLoaderSize();
}


/*************************************************************************
//
**************************************************************************/

void PackPsx::pack(OutputFile *fo)
{

    ibuf.alloc(file_size);
    obuf.allocForCompression(file_data_size);
    upx_byte *p_scan = ibuf+(file_size-1);

    // read file
    fi->seek(0,SEEK_SET);
    fi->readx(ibuf, file_size);

    // this scans the end of file for the 2048 bytes sector alignment
    // this should be padded with zeros
    scan_count = 0;
    while (!(*p_scan--)) { if ((scan_count += 1) > (0xfffc<<3)) break; }
    scan_count = ALIGN_DOWN(scan_count,8);

    // prepare packheader
    ph.u_len = (file_data_size - scan_count);
    ph.filter = 0;

    Filter ft(ph.level);
    // compress (max_match = 65535)
    compressWithFilters(&ft, 512, 0, NULL, 0, 65535, 0, PS_HDR_SIZE);

    if (ph.overlap_overhead <= scan_count)
        overlap = 0;
    else
    {
        overlap = ALIGN_UP((ph.overlap_overhead-scan_count),4);
        if (!opt->force)
            throwCantPack("packed data overlap (try --force)");
        else
        {
            opt->info_mode += !opt->info_mode ? 1 : 0;
            infoWarning("%s will load to a %d bytes higher offset",fi->getName(),overlap);
        }
    }

    memcpy(&oh, &ih, sizeof(&ih));

//    oki_hdrp->upx_hdr.hdr_adler = upx_adler32(&i_hdrp->exec,sizeof(exec_hdr_t));

    const int lsize = getLoaderSize();
    const int h_len = lsize-getLoaderSectionStart("PSXPHDR0");
    const int e_len = lsize-h_len;
    const int d_len = e_len-getLoaderSectionStart("PSXDECO0");
    upx_uint pad_code = 0;
    upx_uint pad = 0;

    MemBuffer loader(lsize);
    memcpy(loader,getLoader(),lsize);

    pad = !right_file_size ? ih.tx_len : right_file_size;
    pad = ALIGN_DOWN(pad,4);
    pad_code = CHK_ALIGNED((ph.c_len),4);

    upx_uint decomp_data_start = ih.tx_ptr;

    // set the offset for compressed stuff at the very end of file
    upx_uint comp_data_start = (decomp_data_start+pad)-ph.c_len+(overlap ? overlap : 0);

    pad = 0;  //clear pad, because of temp use

    // align the packed file to mode 2 data sector size (2048)
    if (!opt->psx.no_align)
        pad = CHK_ALIGNED((ph.c_len+pad_code+e_len),2048);

    int entry = (comp_data_start-e_len-pad_code);

    patch_mips_le32(loader,e_len,"JPEP",MIPS_JP(ih.epc));
    if (scan_count)
        patch_mips_le16(loader,e_len,"SC",MIPS_LO(scan_count>>3));
    patch_hi_lo(loader,e_len,"OH","OL",decomp_data_start);
//    patch_hi_lo(loader,e_len,"LH","LL",ph.u_len+pad_code+pad);
    patch_hi_lo(loader,e_len,"CH","CL",comp_data_start);
    patch_hi_lo(loader,e_len,"DH","DL",entry+(e_len-d_len));
    patch_mips_le16(loader,e_len,"LS",d_len);

    // set the file load address
    oh.tx_ptr = entry-pad;
    // set the correct file len in header
    oh.tx_len = (ph.c_len+e_len+pad+pad_code);
    // set the code entry
    oh.epc = entry;

    // build upx header
//    memcpy(&i_hdrp->upx_hdr.id,UPX_VERSION_STRING,sizeof(UPX_VERSION_STRING));
//    memcpy(&i_hdrp->upx_hdr.ph_hdr,loader+e_len,h_len);

//    patchPackHeader(&i_hdrp->upx_hdr,PS_HDR_SIZE);
    // write loader + compressed file
    fo->write(ibuf,PS_HDR_SIZE);
    // clear ibuf to get padding data (0)
    upx_bytep paddata = ibuf+PS_HDR_SIZE;
    memset(paddata,0,file_data_size);
    // align the file (mode 2 sector data size)
    if (pad)
        fo->write(paddata,pad);
    // entry
    fo->write(loader,e_len);
    // align the compressed data for the mips runtime code
    if (pad_code)
        fo->write(paddata,pad_code);

    fo->write(obuf,ph.c_len);
#if 0
    printf("%-13s: compressed   : %8ld bytes\n", getName(), (long) ph.c_len);
    printf("%-13s: decompressor : %8ld bytes\n", getName(), (long) e_len);
#endif

}


/*************************************************************************
//
**************************************************************************/

int PackPsx::canUnpack()
{
    // FIXME: set ih
    if (!readPackHeader(sizeof(ih)))
        return false;
    if (file_size <= (off_t) ph.c_len)
        return false;
    return true;
}


/*************************************************************************
//
**************************************************************************/

void PackPsx::unpack(OutputFile *fo)
{
    ibuf.alloc(file_size);

//    i_hdrp = (ps_x_exe_t*)ibuf.getBuf();

    // read the file header
    fi->seek(0x0,SEEK_SET);
    // and the remaining stuff
    fi->readx(ibuf,file_size);

#if 0
    // test & restore orig exec hdr
    if (i_hdrp->upx_hdr.hdr_adler != upx_adler32(&i_hdrp->upx_hdr.exec_bkup,sizeof(exec_hdr_t)))
        throwCantUnpack("file is possibly modified/hacked/protected; take care!");
    memcpy(&i_hdrp->exec,&i_hdrp->upx_hdr.exec_bkup,sizeof(exec_hdr_t));

    upx_uint pad = (i_hdrp->exec.tx_len)-ph.u_len;

    memset(&i_hdrp->upx_hdr,0,sizeof(upx_hdr_t));

    obuf.allocForUncompression(ph.u_len+pad);
    memset(obuf,0,obuf.getSize());
#endif

    // decompress
    decompress(ibuf+(file_size-ph.c_len),obuf);

    // write decompressed file
    if (fo)
    {
        fo->write(ibuf,PS_HDR_SIZE);
//        fo->write(obuf,ph.u_len+pad);
    }
}

// functions for marker handling
int PackPsx::patch_mips_le16(void *b, int blen, const void *old, unsigned new_)
{
    unsigned char w[2];

    set_le16(&w,get_be16(old));
    return patch_le16(b, blen, &w, new_);
}

int PackPsx::patch_mips_le32(void *b, int blen, const void *old, unsigned new_)
{
    unsigned char w[4];

    set_le32(&w,get_be32(old));
    return patch_le32(b, blen, &w, new_);
}

int PackPsx::patch_hi_lo(void *b, int blen, const void *old_hi, const void *old_lo, unsigned new_)
{
    patch_mips_le16(b, blen, old_lo, MIPS_LO(new_));
    patch_mips_le16(b, blen, old_hi, MIPS_HI(new_));
    return 1;
}


/*
vi:ts=4:et:nowrap
*/

