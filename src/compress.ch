/* compress.ch --

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


#if 1 && defined(UCL_USE_ASM)
#  include <ucl/ucl_asm.h>
#  define ucl_nrv2b_decompress_safe_8       ucl_nrv2b_decompress_asm_safe_8
#  define ucl_nrv2b_decompress_safe_le16    ucl_nrv2b_decompress_asm_safe_le16
#  define ucl_nrv2b_decompress_safe_le32    ucl_nrv2b_decompress_asm_safe_le32
#  define ucl_nrv2d_decompress_safe_8       ucl_nrv2d_decompress_asm_safe_8
#  define ucl_nrv2d_decompress_safe_le16    ucl_nrv2d_decompress_asm_safe_le16
#  define ucl_nrv2d_decompress_safe_le32    ucl_nrv2d_decompress_asm_safe_le32
#  define ucl_nrv2e_decompress_safe_8       ucl_nrv2e_decompress_asm_safe_8
#  define ucl_nrv2e_decompress_safe_le16    ucl_nrv2e_decompress_asm_safe_le16
#  define ucl_nrv2e_decompress_safe_le32    ucl_nrv2e_decompress_asm_safe_le32
#endif


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

#if defined(ALG_LZMA)

#undef MSDOS
#undef OS2
#undef _WIN32
#undef _WIN32_WCE
#include "lzma/MyInitGuid.h"
//#include "lzma/LZMADecoder.h"
#include "lzma/LZMAEncoder.h"
#undef RC_NORMALIZE


struct CInStreamRam: public ISequentialInStream, public CMyUnknownImp
{
    MY_UNKNOWN_IMP
    const Byte *Data; size_t Size; size_t Pos;
    void Init(const Byte *data, size_t size) {
        Data = data; Size = size; Pos = 0;
    }
    STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize);
};

STDMETHODIMP CInStreamRam::Read(void *data, UInt32 size, UInt32 *processedSize)
{
    UInt32 remain = Size - Pos;
    if (size > remain) size = remain;
    memmove(data, Data + Pos, size);
    Pos += size;
    if (processedSize != NULL) *processedSize = size;
    return S_OK;
}

struct COutStreamRam : public ISequentialOutStream, public CMyUnknownImp
{
    MY_UNKNOWN_IMP
    Byte *Data; size_t Size; size_t Pos; bool Overflow;
    void Init(Byte *data, size_t size) {
        Data = data; Size = size; Pos = 0; Overflow = false;
    }
    HRESULT WriteByte(Byte b) {
        if (Pos >= Size) { Overflow = true; return E_FAIL; }
        Data[Pos++] = b;
        return S_OK;
    }
    STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize);
};

STDMETHODIMP COutStreamRam::Write(const void *data, UInt32 size, UInt32 *processedSize)
{
    UInt32 i;
    for (i = 0; i < size && Pos < Size; i++)
        Data[Pos++] = ((const Byte *)data)[i];
    if (processedSize != NULL) *processedSize = i;
    if (i != size) { Overflow = true; return E_FAIL; }
    return S_OK;
}


static
int upx_lzma_compress      ( const upx_bytep src, upx_uint  src_len,
                                   upx_bytep dst, upx_uintp dst_len,
                                   upx_progress_callback_t *cb,
                                   int level,
                             const struct upx_compress_config_t *conf_parm,
                                   upx_uintp result )
{
    UNUSED(cb); UNUSED(level);
    UNUSED(conf_parm); UNUSED(result);

    int r = UPX_E_ERROR;
    HRESULT rh;

    CInStreamRam* isp = new CInStreamRam;
    CInStreamRam& is = *isp;
    COutStreamRam* osp = new COutStreamRam;
    COutStreamRam& os = *osp;
    is.Init(src, src_len);
//    os.Init(dst, *dst_len);
    os.Init(dst, src_len);

#ifndef _NO_EXCEPTIONS
    try {
#endif

    NCompress::NLZMA::CEncoder enc;
    PROPID propIDs[] = {
        NCoderPropID::kAlgorithm,
        NCoderPropID::kDictionarySize,
        NCoderPropID::kNumFastBytes,
    };
    const int kNumProps = sizeof(propIDs) / sizeof(propIDs[0]);
    PROPVARIANT properties[kNumProps];
    properties[0].vt = VT_UI4;
    properties[1].vt = VT_UI4;
    properties[2].vt = VT_UI4;
    properties[0].ulVal = (UInt32)2;
//  properties[1].ulVal = (UInt32)dictionarySize;
    properties[1].ulVal = (UInt32)1024 * 1024;  // FIXME
    properties[2].ulVal = (UInt32)64;

    if (enc.SetCoderProperties(propIDs, properties, kNumProps) != S_OK)
        goto error;
    if (enc.WriteCoderProperties(&os) != S_OK)
        goto error;
    if (os.Pos != 5)
        goto error;

    rh = enc.Code(&is, &os, 0, 0, 0);
    if (rh == E_OUTOFMEMORY)
        r = UPX_E_OUT_OF_MEMORY;
#if 0
    else if (os.Overflow)
        r = UPX_E_OUPUT_OVERRUN; // FIXME - not compressible
#endif
    else if (rh == S_OK)
        r = UPX_E_OK;
error:
    *dst_len = os.Pos;
    return r;

#ifndef _NO_EXCEPTIONS
    } catch(...) { return UPX_E_OUT_OF_MEMORY; }
#endif
}


#include "lzma/Alloc.cpp"
#include "lzma/CRC.cpp"
//#include "lzma/LZMADecoder.cpp"
#include "lzma/LZMAEncoder.cpp"
#include "lzma/LZInWindow.cpp"
#include "lzma/OutBuffer.cpp"
#include "lzma/RangeCoderBit.cpp"
#include "lzma/StreamUtils.cpp"



#undef _LZMA_IN_CB
#undef _LZMA_OUT_READ
#undef _LZMA_PROB32
#undef _LZMA_LOC_OPT
#include "lzma/LzmaDecode.cpp"

static
int upx_lzma_decompress    ( const upx_bytep src, upx_uint  src_len,
                                   upx_bytep dst, upx_uintp dst_len )
{
    CLzmaDecoderState s;
    SizeT src_out = 0, dst_out = 0;
    int r;

    s.Probs = NULL;
#if defined(LzmaDecoderInit)
    LzmaDecoderInit(&s);
#endif
    r = LzmaDecodeProperties(&s.Properties, src, src_len);
    if (r != 0)
        goto error;
    src += LZMA_PROPERTIES_SIZE; src_len -= LZMA_PROPERTIES_SIZE;
    s.Probs = (CProb *) malloc(sizeof(CProb) * LzmaGetNumProbs(&s.Properties));
    if (!s.Probs)
        r = UPX_E_OUT_OF_MEMORY;
    else
        r = LzmaDecode(&s, src, src_len, &src_out, dst, *dst_len, &dst_out);
error:
    *dst_len = dst_out;
    free(s.Probs);
    return r;
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
    if (method >= M_NRV2B_LE32 && method <= M_CL1B_LE16)
    {
        static const unsigned char sizes[3]={32,8,16};
        conf.bb_size = sizes[(method - M_NRV2B_LE32) % 3];
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
#if 0  /*{*/
    else if M_IS_CL1B(method)
        r = cl1b_compress(src, src_len, dst, dst_len,
                                  cb, level, &conf, result);
#endif  /*}*/
#if defined(ALG_LZMA)
    else if M_IS_LZMA(method)
        r = upx_lzma_compress(src, src_len, dst, dst_len,
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
#if 0  /*{*/
    case M_CL1B_8:
        r = cl1b_decompress_safe_8(src,src_len,dst,dst_len,NULL);
        break;
    case M_CL1B_LE16:
        r = cl1b_decompress_safe_le16(src,src_len,dst,dst_len,NULL);
        break;
    case M_CL1B_LE32:
        r = cl1b_decompress_safe_le32(src,src_len,dst,dst_len,NULL);
        break;
#endif  /*}*/
#if defined(ALG_LZMA)
    case M_LZMA:
        r = upx_lzma_decompress(src,src_len,dst,dst_len);
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
#if 0  /*{*/
    case M_CL1B_8:
        r = cl1b_test_overlap_8(buf,src_off,src_len,dst_len,NULL);
        break;
    case M_CL1B_LE16:
        r = cl1b_test_overlap_le16(buf,src_off,src_len,dst_len,NULL);
        break;
    case M_CL1B_LE32:
        r = cl1b_test_overlap_le32(buf,src_off,src_len,dst_len,NULL);
        break;
#endif  /*}*/
#if defined(ALG_LZMA)
    case M_LZMA:
        /* FIXME */
        if (src_off >= 256)
            r = UPX_E_OK;
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

