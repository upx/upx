/* compress.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2004 Laszlo Molnar
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


#include "conf.h"
#include "compress.h"


/*************************************************************************
//
**************************************************************************/

unsigned upx_adler32(const void *buf, unsigned len, unsigned adler)
{
    if (len == 0)
        return adler;
    assert(buf != NULL);
#if defined(WITH_UCL)
    return ucl_adler32(adler, (const ucl_bytep)buf, len);
#else
#  error
#endif
}


unsigned upx_crc32(const void *buf, unsigned len, unsigned crc)
{
    if (len == 0)
        return crc;
    assert(buf != NULL);
#if defined(WITH_UCL)
    return ucl_crc32(crc, (const ucl_bytep)buf, len);
#else
#  error
#endif
}


/*************************************************************************
//
**************************************************************************/

int upx_compress           ( const upx_bytep src, upx_uint  src_len,
                                   upx_bytep dst, upx_uintp dst_len,
                                   upx_callback_p cb,
                                   int method, int level,
                             const struct upx_compress_config_t *conf,
                                   upx_uintp result )
{
    int r = UPX_E_ERROR;
    upx_uint result_buffer[16];

    assert(level > 0);
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
    result[8] = 0;

#if defined(WITH_LZMA)
    if (M_IS_LZMA(method))
        return upx_lzma_compress(src, src_len, dst, dst_len,
                                 cb, method, level, conf, result);
#endif
#if defined(WITH_NRV)
    if (M_IS_NRV2B(method) || M_IS_NRV2D(method) || M_IS_NRV2E(method))
        return upx_nrv_compress(src, src_len, dst, dst_len,
                                cb, method, level, conf, result);
#endif
#if defined(WITH_UCL)
    if (M_IS_NRV2B(method) || M_IS_NRV2D(method) || M_IS_NRV2E(method))
        return upx_ucl_compress(src, src_len, dst, dst_len,
                                cb, method, level, conf, result);
#endif

    throwInternalError("unknown compression method");
    return r;
}


/*************************************************************************
//
**************************************************************************/

int upx_decompress         ( const upx_bytep src, upx_uint  src_len,
                                   upx_bytep dst, upx_uintp dst_len,
                                   int method )
{
    int r = UPX_E_ERROR;

#if defined(WITH_LZMA)
    if (M_IS_LZMA(method))
        return upx_lzma_decompress(src, src_len, dst, dst_len, method);
#endif
#if defined(WITH_NRV)
    if (M_IS_NRV2B(method) || M_IS_NRV2D(method) || M_IS_NRV2E(method))
        return upx_nrv_decompress(src, src_len, dst, dst_len, method);
#endif
#if defined(WITH_UCL)
    if (M_IS_NRV2B(method) || M_IS_NRV2D(method) || M_IS_NRV2E(method))
        return upx_ucl_decompress(src, src_len, dst, dst_len, method);
#endif

    throwInternalError("unknown decompression method");
    return r;
}


/*************************************************************************
//
**************************************************************************/

int upx_test_overlap       ( const upx_bytep buf, upx_uint src_off,
                                   upx_uint  src_len, upx_uintp dst_len,
                                   int method )
{
    int r = UPX_E_ERROR;

    assert(src_len < *dst_len); // must be compressed
    unsigned overlap_overhead = src_off + src_len - *dst_len;
    assert((int)overlap_overhead > 0);

#if defined(WITH_LZMA)
    if (M_IS_LZMA(method))
        return upx_lzma_test_overlap(buf, src_off, src_len, dst_len, method);
#endif
#if defined(WITH_NRV)
    if (M_IS_NRV2B(method) || M_IS_NRV2D(method) || M_IS_NRV2E(method))
        return upx_nrv_test_overlap(buf, src_off, src_len, dst_len, method);
#endif
#if defined(WITH_UCL)
    if (M_IS_NRV2B(method) || M_IS_NRV2D(method) || M_IS_NRV2E(method))
        return upx_ucl_test_overlap(buf, src_off, src_len, dst_len, method);
#endif

    throwInternalError("unknown decompression method");
    return r;
}


/*
vi:ts=4:et:nowrap
*/

