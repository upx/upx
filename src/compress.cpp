/* compress.cpp --

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


#include "conf.h"
#include "version.h"

#if 0 && defined(WITH_NRV) && defined(__i386__)
#  define NRV_USE_ASM
#endif
#if 0 && defined(WITH_UCL) && defined(__i386__)
#  define UCL_USE_ASM
#endif
#if defined(WITH_NRV)
#  include <nrv/nrv2b.h>
#  include <nrv/nrv2d.h>
//#  include <nrv/nrv2e.h>
#  if !defined(NRV_VERSION) || (NRV_VERSION < 0x008000L)
#    error
#  endif
   NRV_EXTERN_CDECL(int) nrv2b_99_compress_internal(const upx_byte *, ...);
   NRV_EXTERN_CDECL(int) nrv2d_99_compress_internal(const upx_byte *, ...);
   NRV_EXTERN_CDECL(int) nrv2e_99_compress_internal(const upx_byte *, ...);
#elif defined(WITH_UCL)
#  define nrv2b_99_compress_internal    ucl_nrv2b_99_compress
#  define nrv2d_99_compress_internal    ucl_nrv2d_99_compress
#  define nrv2e_99_compress_internal    ucl_nrv2e_99_compress
#endif
#if 1 && defined(WITH_NRV) && defined(NRV_USE_ASM)
#  if 1 &&  defined(NRV_USE_ASM)
#    define nrv2b_decompress_safe_8       nrv2b_decompress_asm_safe_8
#    define nrv2b_decompress_safe_le16    nrv2b_decompress_asm_safe_le16
#    define nrv2b_decompress_safe_le32    nrv2b_decompress_asm_safe_le32
#    define nrv2d_decompress_safe_8       nrv2d_decompress_asm_safe_8
#    define nrv2d_decompress_safe_le16    nrv2d_decompress_asm_safe_le16
#    define nrv2d_decompress_safe_le32    nrv2d_decompress_asm_safe_le32
#    define nrv2e_decompress_safe_8       nrv2e_decompress_asm_safe_8
#    define nrv2e_decompress_safe_le16    nrv2e_decompress_asm_safe_le16
#    define nrv2e_decompress_safe_le32    nrv2e_decompress_asm_safe_le32
#  endif
   NRV_EXTERN_CDECL(int) nrv2b_decompress_safe_8(const upx_byte *, ...);
   NRV_EXTERN_CDECL(int) nrv2b_decompress_safe_le16(const upx_byte *, ...);
   NRV_EXTERN_CDECL(int) nrv2b_decompress_safe_le32(const upx_byte *, ...);
   NRV_EXTERN_CDECL(int) nrv2d_decompress_safe_8(const upx_byte *, ...);
   NRV_EXTERN_CDECL(int) nrv2d_decompress_safe_le16(const upx_byte *, ...);
   NRV_EXTERN_CDECL(int) nrv2d_decompress_safe_le32(const upx_byte *, ...);
   NRV_EXTERN_CDECL(int) nrv2e_decompress_safe_8(const upx_byte *, ...);
   NRV_EXTERN_CDECL(int) nrv2e_decompress_safe_le16(const upx_byte *, ...);
   NRV_EXTERN_CDECL(int) nrv2e_decompress_safe_le32(const upx_byte *, ...);
#elif defined(WITH_UCL)
#  if defined(UCL_USE_ASM)
#    include <ucl/ucl_asm.h>
#    define nrv2b_decompress_safe_8     ucl_nrv2b_decompress_asm_safe_8
#    define nrv2b_decompress_safe_le16  ucl_nrv2b_decompress_asm_safe_le16
#    define nrv2b_decompress_safe_le32  ucl_nrv2b_decompress_asm_safe_le32
#    define nrv2d_decompress_safe_8     ucl_nrv2d_decompress_asm_safe_8
#    define nrv2d_decompress_safe_le16  ucl_nrv2d_decompress_asm_safe_le16
#    define nrv2d_decompress_safe_le32  ucl_nrv2d_decompress_asm_safe_le32
#    define nrv2e_decompress_safe_8     ucl_nrv2e_decompress_asm_safe_8
#    define nrv2e_decompress_safe_le16  ucl_nrv2e_decompress_asm_safe_le16
#    define nrv2e_decompress_safe_le32  ucl_nrv2e_decompress_asm_safe_le32
#  else
#    define nrv2b_decompress_safe_8     ucl_nrv2b_decompress_safe_8
#    define nrv2b_decompress_safe_le16  ucl_nrv2b_decompress_safe_le16
#    define nrv2b_decompress_safe_le32  ucl_nrv2b_decompress_safe_le32
#    define nrv2d_decompress_safe_8     ucl_nrv2d_decompress_safe_8
#    define nrv2d_decompress_safe_le16  ucl_nrv2d_decompress_safe_le16
#    define nrv2d_decompress_safe_le32  ucl_nrv2d_decompress_safe_le32
#    define nrv2e_decompress_safe_8     ucl_nrv2e_decompress_safe_8
#    define nrv2e_decompress_safe_le16  ucl_nrv2e_decompress_safe_le16
#    define nrv2e_decompress_safe_le32  ucl_nrv2e_decompress_safe_le32
#  endif
#else
#  error
#endif

#if 0 && defined(WITH_ZLIB) && defined(M_ZLIB)
#  include <zlib.h>
#endif


/*************************************************************************
//
**************************************************************************/

unsigned upx_adler32(const void *buf, unsigned len, unsigned adler)
{
    if (len == 0)
        return adler;
    assert(buf != NULL);
#if defined(WITH_NRV)
    return nrv_adler32(adler, (const nrv_byte *)buf, len);
#elif defined(WITH_UCL)
    return ucl_adler32(adler, (const ucl_byte *)buf, len);
#endif
}


#if (UPX_VERSION_HEX >= 0x019000)
unsigned upx_adler32(unsigned adler, const void *buf, unsigned len)
{
    return upx_adler32(buf, len, adler);
}
#endif /* UPX_VERSION_HEX */



#if 0 // NOT USED
unsigned upx_crc32(const void *buf, unsigned len, unsigned crc)
{
    if (len == 0)
        return crc;
    assert(buf != NULL);
#if defined(WITH_NRV)
    return nrv_crc32(crc, (const nrv_byte *)buf, len);
#elif defined(WITH_UCL)
    return ucl_crc32(crc, (const ucl_byte *)buf, len);
#endif
}
#endif


/*************************************************************************
//
**************************************************************************/

int upx_compress           ( const upx_byte *src, upx_uint  src_len,
                                   upx_byte *dst, upx_uint *dst_len,
                                   upx_progress_callback_t *cb,
                                   int method, int level,
                             const struct upx_compress_config_t *conf_parm,
                                   upx_uintp result)
{
    struct upx_compress_config_t conf;
    upx_uint result_buffer[16];
    int r = UPX_E_ERROR;

    assert(level > 0);
    memset(&conf, 0xff, sizeof(conf));
    if (conf_parm)
        conf = *conf_parm;     // struct copy
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

#if 0 && defined(WITH_ZLIB) && defined(M_ZLIB)
    if (method == M_ZLIB)
    {
        uLong destLen = src_len + src_len / 8 + 256;
        r = compress2(dst, &destLen, src, src_len, UPX_MIN(level, 9));
        *dst_len = destLen;
        if (r == Z_MEM_ERROR)
            return UPX_E_OUT_OF_MEMORY;
        if (r != Z_OK)
            return UPX_E_ERROR;
        return UPX_E_OK;
    }
#endif

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

#if 1 && defined(WITH_NRV)
    if (level == 1 && conf.bb_size == 32 &&
        conf.max_offset >= src_len && conf.max_match >= src_len)
    {
        if (method == M_NRV2B_LE32)
        {
            upx_byte wrkmem[NRV2B_1_16_MEM_COMPRESS];
#if defined(__UPX_CHECKER)
            memset(wrkmem,0,NRV2B_1_16_MEM_COMPRESS);
#endif
            r = nrv2b_1_16_compress(src, src_len, dst, dst_len, wrkmem);
        }
        else if (method == M_NRV2D_LE32)
        {
            upx_byte wrkmem[NRV2D_1_16_MEM_COMPRESS];
#if defined(__UPX_CHECKER)
            memset(wrkmem,0,NRV2D_1_16_MEM_COMPRESS);
#endif
            r = nrv2d_1_16_compress(src, src_len, dst, dst_len, wrkmem);
        }
#if 0
        else if (method == M_NRV2E_LE32)
        {
            upx_byte wrkmem[NRV2E_1_16_MEM_COMPRESS];
#if defined(__UPX_CHECKER)
            memset(wrkmem,0,NRV2E_1_16_MEM_COMPRESS);
#endif
            r = nrv2e_1_16_compress(src, src_len, dst, dst_len, wrkmem);
        }
#endif
        else
            throwInternalError("unknown compression method");
        return r;
    }
#endif

    // optimize compression parms
    if (level <= 3 && conf.max_offset == UPX_UINT_MAX)
        conf.max_offset = 8*1024-1;
    else if (level == 4 && conf.max_offset == UPX_UINT_MAX)
        conf.max_offset = 32*1024-1;
#if defined(WITH_NRV)
    else if (level <= 6 && conf.max_offset == UPX_UINT_MAX)
        conf.max_offset = 1*1024*1024-1;
    else if (level <= 8 && conf.max_offset == UPX_UINT_MAX)
        conf.max_offset = 2*1024*1024-1;
    else if (level <= 10 && conf.max_offset == UPX_UINT_MAX)
        conf.max_offset = 4*1024*1024-1;
#endif

    if M_IS_NRV2B(method)
        r = nrv2b_99_compress_internal(src, src_len, dst, dst_len,
                                       cb, level, &conf, result);
    else if M_IS_NRV2D(method)
        r = nrv2d_99_compress_internal(src, src_len, dst, dst_len,
                                       cb, level, &conf, result);
#if 0
    else if M_IS_NRV2E(method)
        r = nrv2e_99_compress_internal(src, src_len, dst, dst_len,
                                       cb, level, &conf, result);
#endif
    else
        throwInternalError("unknown compression method");

    return r;
}


/*************************************************************************
//
**************************************************************************/

int upx_decompress         ( const upx_byte *src, upx_uint  src_len,
                                   upx_byte *dst, upx_uint *dst_len,
                                   int method )
{
    int r = UPX_E_ERROR;

#if 0 && defined(WITH_ZLIB) && defined(M_ZLIB)
    if (method == M_ZLIB)
    {
        uLong destLen = *dst_len;
        r = uncompress(dst, &destLen, src, src_len);
        *dst_len = destLen;
        if (r == Z_MEM_ERROR)
            return UPX_E_OUT_OF_MEMORY;
        if (r != Z_OK)
            return UPX_E_ERROR;
        return UPX_E_OK;
    }
#endif

    switch (method)
    {
    case M_NRV2B_8:
        r = nrv2b_decompress_safe_8(src,src_len,dst,dst_len,NULL);
        break;
    case M_NRV2B_LE16:
        r = nrv2b_decompress_safe_le16(src,src_len,dst,dst_len,NULL);
        break;
    case M_NRV2B_LE32:
        r = nrv2b_decompress_safe_le32(src,src_len,dst,dst_len,NULL);
        break;
    case M_NRV2D_8:
        r = nrv2d_decompress_safe_8(src,src_len,dst,dst_len,NULL);
        break;
    case M_NRV2D_LE16:
        r = nrv2d_decompress_safe_le16(src,src_len,dst,dst_len,NULL);
        break;
    case M_NRV2D_LE32:
        r = nrv2d_decompress_safe_le32(src,src_len,dst,dst_len,NULL);
        break;
#if 0
    case M_NRV2E_8:
        r = nrv2e_decompress_safe_8(src,src_len,dst,dst_len,NULL);
        break;
    case M_NRV2E_LE16:
        r = nrv2e_decompress_safe_le16(src,src_len,dst,dst_len,NULL);
        break;
    case M_NRV2E_LE32:
        r = nrv2e_decompress_safe_le32(src,src_len,dst,dst_len,NULL);
        break;
#endif
    default:
        throwInternalError("unknown decompression method");
        break;
    }

    return r;
}


/*************************************************************************
//
**************************************************************************/

#if (UPX_VERSION_HEX >= 0x019000)

int upx_test_overlap       ( const upx_byte *buf, upx_uint src_off,
                                   upx_uint  src_len, upx_uint *dst_len,
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
#if 0
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

#endif /* UPX_VERSION_HEX */


/*
vi:ts=4:et:nowrap
*/

