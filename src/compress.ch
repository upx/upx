/* compress.ch --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2002 Laszlo Molnar
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
 */


/*************************************************************************
//
**************************************************************************/

#if defined(upx_adler32)
unsigned upx_adler32(const void *buf, unsigned len, unsigned adler)
{
    if (len == 0)
        return adler;
    assert(buf != NULL);
    return ucl_adler32(adler, (const ucl_bytep)buf, len);
}
#endif


#if defined(upx_crc32)
unsigned upx_crc32(const void *buf, unsigned len, unsigned crc)
{
    if (len == 0)
        return crc;
    assert(buf != NULL);
    return ucl_crc32(crc, (const ucl_bytep)buf, len);
}
#endif


/*************************************************************************
//
**************************************************************************/

#if defined(upx_compress)

int upx_compress           ( const upx_bytep src, upx_uint  src_len,
                                   upx_bytep dst, upx_uintp dst_len,
                                   upx_progress_callback_t *cb,
                                   int method, int level,
                             const struct upx_compress_config_t *conf_parm,
                                   upx_uintp result )
{
    struct upx_compress_config_t conf;
    upx_uint result_buffer[16];
    int r = UPX_E_ERROR;

    assert(level > 0);
    memset(&conf, 0xff, sizeof(conf));
    if (conf_parm)
        conf = *conf_parm;      // struct copy
    if (!result)
        result = result_buffer;

    // assume no info available - fill in worst case results
    //result[0] = 1;              // min_offset_found - NOT USED
    result[1] = src_len - 1;    // max_offset_found
    //result[2] = 2;              // min_match_found - NOT USED
    result[3] = src_len - 1;    // max_match_found
    //result[4] = 1;              // min_run_found - NOT USED
    result[5] = src_len;        // max_run_found
    result[6] = 1;              // first_offset_found
    //result[7] = 999999;         // same_match_offsets_found - NOT USED

    // prepare bit-buffer settings
    conf.bb_endian = 0;
    conf.bb_size = 0;
    if (method >= M_NRV2B_LE32 && method <= M_NRV2E_LE16)
    {
        int n = (method - M_NRV2B_LE32) % 3;
        if (n == 0)
            conf.bb_size = 32;
        else if (n == 1)
            conf.bb_size = 8;
        else
            conf.bb_size = 16;
    }
    else
        throwInternalError("unknown compression method");

    // optimize compression parms
    if (level <= 3 && conf.max_offset == UPX_UINT_MAX)
        conf.max_offset = 8*1024-1;
    else if (level == 4 && conf.max_offset == UPX_UINT_MAX)
        conf.max_offset = 32*1024-1;

    if M_IS_NRV2B(method)
        r = ucl_nrv2b_99_compress(src, src_len, dst, dst_len,
                                  cb, level, &conf, result);
    else if M_IS_NRV2D(method)
        r = ucl_nrv2d_99_compress(src, src_len, dst, dst_len,
                                  cb, level, &conf, result);
#if defined(ALG_NRV2E)
    else if M_IS_NRV2E(method)
        r = ucl_nrv2e_99_compress(src, src_len, dst, dst_len,
                                  cb, level, &conf, result);
#endif
    else
        throwInternalError("unknown compression method");

    return r;
}

#endif /* upx_compress */


/*************************************************************************
//
**************************************************************************/

#if defined(upx_decompress)

int upx_decompress         ( const upx_bytep src, upx_uint  src_len,
                                   upx_bytep dst, upx_uintp dst_len,
                                   int method )
{
    int r = UPX_E_ERROR;

    switch (method)
    {
    case M_NRV2B_8:
        r = ucl_nrv2b_decompress_safe_8(src,src_len,dst,dst_len,NULL);
        break;
    case M_NRV2B_LE16:
        r = ucl_nrv2b_decompress_safe_le16(src,src_len,dst,dst_len,NULL);
        break;
    case M_NRV2B_LE32:
        r = ucl_nrv2b_decompress_safe_le32(src,src_len,dst,dst_len,NULL);
        break;
    case M_NRV2D_8:
        r = ucl_nrv2d_decompress_safe_8(src,src_len,dst,dst_len,NULL);
        break;
    case M_NRV2D_LE16:
        r = ucl_nrv2d_decompress_safe_le16(src,src_len,dst,dst_len,NULL);
        break;
    case M_NRV2D_LE32:
        r = ucl_nrv2d_decompress_safe_le32(src,src_len,dst,dst_len,NULL);
        break;
#if defined(ALG_NRV2E)
    case M_NRV2E_8:
        r = ucl_nrv2e_decompress_safe_8(src,src_len,dst,dst_len,NULL);
        break;
    case M_NRV2E_LE16:
        r = ucl_nrv2e_decompress_safe_le16(src,src_len,dst,dst_len,NULL);
        break;
    case M_NRV2E_LE32:
        r = ucl_nrv2e_decompress_safe_le32(src,src_len,dst,dst_len,NULL);
        break;
#endif
    default:
        throwInternalError("unknown decompression method");
        break;
    }

    return r;
}

#endif /* upx_decompress */


/*************************************************************************
//
**************************************************************************/

#if defined(upx_test_overlap)

int upx_test_overlap       ( const upx_bytep buf, upx_uint src_off,
                                   upx_uint  src_len, upx_uintp dst_len,
                                   int method )
{
    int r = UPX_E_ERROR;

    switch (method)
    {
    case M_NRV2B_8:
        r = ucl_nrv2b_test_overlap_8(buf,src_off,src_len,dst_len,NULL);
        break;
    case M_NRV2B_LE16:
        r = ucl_nrv2b_test_overlap_le16(buf,src_off,src_len,dst_len,NULL);
        break;
    case M_NRV2B_LE32:
        r = ucl_nrv2b_test_overlap_le32(buf,src_off,src_len,dst_len,NULL);
        break;
    case M_NRV2D_8:
        r = ucl_nrv2d_test_overlap_8(buf,src_off,src_len,dst_len,NULL);
        break;
    case M_NRV2D_LE16:
        r = ucl_nrv2d_test_overlap_le16(buf,src_off,src_len,dst_len,NULL);
        break;
    case M_NRV2D_LE32:
        r = ucl_nrv2d_test_overlap_le32(buf,src_off,src_len,dst_len,NULL);
        break;
#if defined(ALG_NRV2E)
    case M_NRV2E_8:
        r = ucl_nrv2e_test_overlap_8(buf,src_off,src_len,dst_len,NULL);
        break;
    case M_NRV2E_LE16:
        r = ucl_nrv2e_test_overlap_le16(buf,src_off,src_len,dst_len,NULL);
        break;
    case M_NRV2E_LE32:
        r = ucl_nrv2e_test_overlap_le32(buf,src_off,src_len,dst_len,NULL);
        break;
#endif
    default:
        throwInternalError("unknown decompression method");
        break;
    }

    return r;
}

#endif /* upx_test_overlap */


/*
vi:ts=4:et:nowrap
*/

