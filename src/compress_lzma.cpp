/* compress_lzma.cpp --

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
#if !defined(WITH_LZMA)
extern int compress_lzma_dummy;
int compress_lzma_dummy = 0;
#else


/*************************************************************************
// cruft because of pseudo-COM layer
**************************************************************************/

#undef MSDOS
#undef OS2
#undef _WIN32
#undef _WIN32_WCE
#include "lzma/MyInitGuid.h"
//#include "lzma/LZMADecoder.h"
#include "lzma/LZMAEncoder.h"
#undef RC_NORMALIZE

namespace MyLzma {

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

} // namespace


int upx_lzma_compress      ( const upx_bytep src, upx_uint  src_len,
                                   upx_bytep dst, upx_uintp dst_len,
                                   upx_callback_p cb,
                                   int method, int level,
                             const struct upx_compress_config_t *conf,
                                   upx_uintp result )
{
    assert(method == M_LZMA);
    UNUSED(cb); UNUSED(method); UNUSED(level);
    UNUSED(conf_parm); UNUSED(result);

    int r = UPX_E_ERROR;
    HRESULT rh;

    MyLzma::CInStreamRam* isp = new MyLzma::CInStreamRam;
    MyLzma::CInStreamRam& is = *isp;
    MyLzma::COutStreamRam* osp = new MyLzma::COutStreamRam;
    MyLzma::COutStreamRam& os = *osp;
    is.Init(src, src_len);
//    os.Init(dst, *dst_len);
    os.Init(dst, src_len);

#ifndef _NO_EXCEPTIONS
    try {
#endif

    NCompress::NLZMA::CEncoder enc;
    PROPID propIDs[3] = {
        NCoderPropID::kAlgorithm,
        NCoderPropID::kDictionarySize,
        NCoderPropID::kNumFastBytes
    };
    PROPVARIANT properties[3];
    properties[0].vt = VT_UI4;
    properties[1].vt = VT_UI4;
    properties[2].vt = VT_UI4;
    properties[0].ulVal = (UInt32) 2;
//  properties[1].ulVal = (UInt32) dictionarySize;
    properties[1].ulVal = (UInt32) 1024 * 1024;  // FIXME
    properties[2].ulVal = (UInt32) 64;

    if (enc.SetCoderProperties(propIDs, properties, 3) != S_OK)
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

    //result[8] = LzmaGetNumProbs(&s.Properties));
    result[8] = 0; // FIXME

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


/*************************************************************************
// LZMA decompress
**************************************************************************/

#undef _LZMA_IN_CB
#undef _LZMA_OUT_READ
#undef _LZMA_PROB32
#undef _LZMA_LOC_OPT
#include "lzma/LzmaDecode.cpp"

int upx_lzma_decompress    ( const upx_bytep src, upx_uint  src_len,
                                   upx_bytep dst, upx_uintp dst_len,
                                   int method )
{
    assert(method == M_LZMA);

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


#endif /* WITH_LZMA */
/*
vi:ts=4:et:nowrap
*/

