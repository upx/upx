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
#undef COMPRESS_MF_MT
#include "C/Common/MyInitGuid.h"
#include "C/7zip/Compress/LZMA/LZMADecoder.h"
#include "C/7zip/Compress/LZMA/LZMAEncoder.h"

namespace MyLzma {

struct InStreamRam: public ISequentialInStream, public CMyUnknownImp
{
    MY_UNKNOWN_IMP
    const Byte *Data; size_t Size; size_t Pos;
    void Init(const Byte *data, size_t size) {
        Data = data; Size = size; Pos = 0;
    }
    STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize);
};

STDMETHODIMP InStreamRam::Read(void *data, UInt32 size, UInt32 *processedSize)
{
    UInt32 remain = Size - Pos;
    if (size > remain) size = remain;
    memmove(data, Data + Pos, size);
    Pos += size;
    if (processedSize != NULL) *processedSize = size;
    return S_OK;
}

struct OutStreamRam : public ISequentialOutStream, public CMyUnknownImp
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

STDMETHODIMP OutStreamRam::Write(const void *data, UInt32 size, UInt32 *processedSize)
{
    UInt32 i;
    for (i = 0; i < size && Pos < Size; i++)
        Data[Pos++] = ((const Byte *)data)[i];
    if (processedSize != NULL) *processedSize = i;
    if (i != size) { Overflow = true; return E_FAIL; }
    return S_OK;
}


struct ProgressInfo : public ICompressProgressInfo, public CMyUnknownImp
{
    MY_UNKNOWN_IMP
    STDMETHOD(SetRatioInfo)(const UInt64 *inSize, const UInt64 *outSize);
    upx_callback_p cb;
};

STDMETHODIMP ProgressInfo::SetRatioInfo(const UInt64 *inSize, const UInt64 *outSize)
{
    if (cb && cb->nprogress)
        cb->nprogress(cb, (upx_uint) *inSize, (upx_uint) *outSize, 3);
    return S_OK;
}

} // namespace


int upx_lzma_compress      ( const upx_bytep src, upx_uint  src_len,
                                   upx_bytep dst, upx_uintp dst_len,
                                   upx_callback_p cb,
                                   int method, int level,
                             const struct upx_compress_config_t *conf_parm,
                                   upx_uintp result )
{
    assert(method == M_LZMA);
    UNUSED(cb); UNUSED(method); UNUSED(level);
    UNUSED(conf_parm); UNUSED(result);

    int r = UPX_E_ERROR;
    HRESULT rh;

    MyLzma::InStreamRam is; is.AddRef();
    MyLzma::OutStreamRam os; os.AddRef();
    is.Init(src, src_len);
//    os.Init(dst, *dst_len);
    os.Init(dst, src_len);

    MyLzma::ProgressInfo progress; progress.AddRef();
    progress.cb = cb;

#ifndef _NO_EXCEPTIONS
    try {
#endif

    NCompress::NLZMA::CEncoder enc;
    const PROPID propIDs[7] = {
        NCoderPropID::kPosStateBits,        // 0  pb  _posStateBits(2)
        NCoderPropID::kLitPosBits,          // 1  lp  _numLiteralPosStateBits(0)
        NCoderPropID::kLitContextBits,      // 2  lc  _numLiteralContextBits(3)
        NCoderPropID::kDictionarySize,      // 3
        NCoderPropID::kAlgorithm,           // 4      _fastmode
        NCoderPropID::kNumFastBytes,        // 5
        NCoderPropID::kMatchFinderCycles    // 6
    };
    PROPVARIANT pr[7];
    pr[0].vt = pr[1].vt = pr[2].vt = VT_UI4;
    pr[3].vt = pr[4].vt = pr[5].vt = VT_UI4;
    pr[6].vt = VT_UI4;

    // setup defaults
    pr[0].uintVal = 2;
    pr[1].uintVal = 0;
    pr[2].uintVal = 3;
    pr[3].uintVal = src_len;
    pr[4].uintVal = 2;
    pr[5].uintVal = 64;
    pr[6].uintVal = 0;

    // FIXME: tune these according to level
    if (pr[3].uintVal > src_len)
        pr[3].uintVal = src_len;

    if (enc.SetCoderProperties(propIDs, pr, 3) != S_OK)
        goto error;
    if (enc.WriteCoderProperties(&os) != S_OK)
        goto error;
    if (os.Overflow || os.Pos != 5)
        goto error;
#if 0 // defined(_LZMA_OUT_READ)
#else
    os.Pos -= 4; // do not encode dict_size
#endif

    rh = enc.Code(&is, &os, NULL, NULL, &progress);
    if (rh == E_OUTOFMEMORY)
        r = UPX_E_OUT_OF_MEMORY;
#if 0
    else if (os.Overflow)
        r = UPX_E_OUPUT_OVERRUN; // FIXME - not compressible
#endif
    else if (rh == S_OK)
        r = UPX_E_OK;

    //result[8] = LzmaGetNumProbs(&s.Properties));
    //result[8] = (LZMA_BASE_SIZE + (LZMA_LIT_SIZE << ((Properties)->lc + (Properties)->lp)))
    result[8] = 1846 + (768 << (pr[2].uintVal + pr[1].uintVal));

error:
    *dst_len = os.Pos;
    return r;

#ifndef _NO_EXCEPTIONS
    } catch(...) { return UPX_E_OUT_OF_MEMORY; }
#endif
}


#include "C/Common/Alloc.cpp"
#include "C/Common/CRC.cpp"
//#include "C/7zip/Common/InBuffer.cpp"
#include "C/7zip/Common/OutBuffer.cpp"
#include "C/7zip/Common/StreamUtils.cpp"
#include "C/7zip/Compress/LZ/LZInWindow.cpp"
//#include "C/7zip/Compress/LZ/LZOutWindow.cpp"
//#include "C/7zip/Compress/LZMA/LZMADecoder.cpp"
#include "C/7zip/Compress/LZMA/LZMAEncoder.cpp"
#include "C/7zip/Compress/RangeCoder/RangeCoderBit.cpp"
#undef RC_NORMALIZE


/*************************************************************************
// LZMA decompress
**************************************************************************/

#undef _LZMA_IN_CB
#undef _LZMA_OUT_READ
#undef _LZMA_PROB32
#undef _LZMA_LOC_OPT
#include "C/7zip/Compress/LZMA_C/LzmaDecode.c"

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
#if 0 // defined(_LZMA_OUT_READ)
    src += LZMA_PROPERTIES_SIZE; src_len -= LZMA_PROPERTIES_SIZE;
#else
    src += 1; src_len -= 1;
#endif
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


/*************************************************************************
//
**************************************************************************/

int upx_lzma_test_overlap  ( const upx_bytep buf, upx_uint src_off,
                                   upx_uint  src_len, upx_uintp dst_len,
                                   int method )
{
    assert(method == M_LZMA);

    /* FIXME */
    UNUSED(buf);

    unsigned overlap_overhead = src_off + src_len - *dst_len;
    //printf("upx_lzma_test_overlap: %d\n", overlap_overhead);
    if ((int)overlap_overhead >= 256)
        return UPX_E_OK;

    return UPX_E_ERROR;
}


#endif /* WITH_LZMA */
/*
vi:ts=4:et:nowrap
*/

