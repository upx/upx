/* compress.cpp --

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
#include "compress.h"
#include "mem.h"
#include <zlib.h>


/*************************************************************************
//
**************************************************************************/

unsigned upx_adler32(const void *buf, unsigned len, unsigned adler)
{
    if (len == 0)
        return adler;
    assert(buf != NULL);
#if 0
    return adler32(adler, (const Bytef *) buf, len); // zlib
#elif defined(WITH_UCL)
    return ucl_adler32(adler, (const ucl_bytep) buf, len);
#else
#  error
#endif
}


#if 0 /* UNUSED */
unsigned upx_crc32(const void *buf, unsigned len, unsigned crc)
{
    if (len == 0)
        return crc;
    assert(buf != NULL);
#if 0
    return crc32(crc, (const Bytef *) buf, len); // zlib
#elif defined(WITH_UCL)
    return ucl_crc32(crc, (const ucl_bytep) buf, len);
#else
#  error
#endif
}
#endif /* UNUSED */


/*************************************************************************
//
**************************************************************************/

int upx_compress           ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   upx_callback_p cb,
                                   int method, int level,
                             const upx_compress_config_t *cconf,
                                   upx_compress_result_t *cresult )
{
    int r = UPX_E_ERROR;
    upx_compress_result_t cresult_buffer;

    assert(method > 0); assert(level > 0);

#if 1
    // set available bytes in dst
    if (*dst_len == 0)
        *dst_len = MemBuffer::getSizeForCompression(src_len);
#else
    // force users to provide *dst_len
    assert(*dst_len != 0);
#endif
    // for UPX, we always require a reasonably sized outbut buffer
    assert(*dst_len >= MemBuffer::getSizeForCompression(src_len));

    if (!cresult)
        cresult = &cresult_buffer;
    memset(cresult, 0, sizeof(*cresult));
#if 1
    // debug
    cresult->method = method;
    cresult->level = level;
    cresult->u_len = src_len;
    cresult->c_len = 0;
#endif

    if (0) {
    }
#if defined(WITH_LZMA)
    else if (M_IS_LZMA(method))
        r = upx_lzma_compress(src, src_len, dst, dst_len,
                              cb, method, level, cconf, cresult);
#endif
#if defined(WITH_NRV)
    else if (M_IS_NRV2B(method) || M_IS_NRV2D(method) || M_IS_NRV2E(method))
        r = upx_nrv_compress(src, src_len, dst, dst_len,
                             cb, method, level, cconf, cresult);
#endif
#if defined(WITH_UCL)
    else if (M_IS_NRV2B(method) || M_IS_NRV2D(method) || M_IS_NRV2E(method))
        r = upx_ucl_compress(src, src_len, dst, dst_len,
                             cb, method, level, cconf, cresult);
#endif
    else {
        throwInternalError("unknown compression method");
    }

#if 1
    // debug
    cresult->c_len = *dst_len;
#endif
    return r;
}


/*************************************************************************
//
**************************************************************************/

int upx_decompress         ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   int method,
                             const upx_compress_result_t *cresult )
{
    int r = UPX_E_ERROR;

    assert(*dst_len > 0);
    assert(src_len < *dst_len); // must be compressed

    if (cresult && cresult->method == 0)
        cresult = NULL;

    if (0) {
    }
#if defined(WITH_LZMA)
    else if (M_IS_LZMA(method))
        r = upx_lzma_decompress(src, src_len, dst, dst_len, method, cresult);
#endif
#if defined(WITH_NRV)
    else if (M_IS_NRV2B(method) || M_IS_NRV2D(method) || M_IS_NRV2E(method))
        r = upx_nrv_decompress(src, src_len, dst, dst_len, method, cresult);
#endif
#if defined(WITH_UCL)
    else if (M_IS_NRV2B(method) || M_IS_NRV2D(method) || M_IS_NRV2E(method))
        r = upx_ucl_decompress(src, src_len, dst, dst_len, method, cresult);
#endif
#if defined(WITH_ZLIB)
    else if (M_IS_DEFLATE(method))
        r = upx_zlib_decompress(src, src_len, dst, dst_len, method, cresult);
#endif
    else {
        throwInternalError("unknown decompression method");
    }

    return r;
}


/*************************************************************************
//
**************************************************************************/

int upx_test_overlap       ( const upx_bytep buf,
                             const upx_bytep tbuf,
                                   unsigned  src_off, unsigned src_len,
                                   unsigned* dst_len,
                                   int method,
                             const upx_compress_result_t *cresult )
{
    int r = UPX_E_ERROR;

    if (cresult && cresult->method == 0)
        cresult = NULL;

    assert(*dst_len > 0);
    assert(src_len < *dst_len); // must be compressed
    unsigned overlap_overhead = src_off + src_len - *dst_len;
    assert((int)overlap_overhead > 0);

    if (0) {
    }
#if defined(WITH_LZMA)
    else if (M_IS_LZMA(method))
        r = upx_lzma_test_overlap(buf, tbuf, src_off, src_len, dst_len, method, cresult);
#endif
#if defined(WITH_NRV)
    else if (M_IS_NRV2B(method) || M_IS_NRV2D(method) || M_IS_NRV2E(method))
        r = upx_nrv_test_overlap(buf, tbuf, src_off, src_len, dst_len, method, cresult);
#endif
#if defined(WITH_UCL)
    else if (M_IS_NRV2B(method) || M_IS_NRV2D(method) || M_IS_NRV2E(method))
        r = upx_ucl_test_overlap(buf, tbuf, src_off, src_len, dst_len, method, cresult);
#endif
    else {
        throwInternalError("unknown decompression method");
    }

    return r;
}


/*
vi:ts=4:et:nowrap
*/

