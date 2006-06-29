/* compress_lzma.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2006 Laszlo Molnar
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


// INFO: the LZMA SDK is covered by a permissive license which allows
//   using unmodified LZMA source code in UPX and the UPX stubs.
//   See SPECIAL EXCEPTION below.
//
// Quoting from lzma-4.43/lzma.txt:
//
//   LICENSE
//   -------
//
//   LZMA SDK is available under any of the following licenses:
//
//   1) GNU Lesser General Public License (GNU LGPL)
//   2) Common Public License (CPL)
//   3) Simplified license for unmodified code (read SPECIAL EXCEPTION)
//   4) Proprietary license
//
//   It means that you can select one of these four options and follow rules
//   of that license.
//
//   1,2) GNU LGPL and CPL licenses are pretty similar and both these
//   licenses are classified as
//    - "Free software licenses" at http://www.gnu.org/
//    - "OSI-approved" at http://www.opensource.org/
//
//   3) SPECIAL EXCEPTION
//
//   Igor Pavlov, as the author of this code, expressly permits you
//   to statically or dynamically link your code (or bind by name)
//   to the files from LZMA SDK without subjecting your linked
//   code to the terms of the CPL or GNU LGPL.
//   Any modifications or additions to files from LZMA SDK, however,
//   are subject to the GNU LGPL or CPL terms.
//
//   SPECIAL EXCEPTION allows you to use LZMA SDK in applications with closed code,
//   while you keep LZMA SDK code unmodified.


/*************************************************************************
// cruft because of pseudo-COM layer
**************************************************************************/

#undef USE_LZMA_PROPERTIES

#undef MSDOS
#undef OS2
#undef _WIN32
#undef _WIN32_WCE
#undef COMPRESS_MF_MT
#undef _NO_EXCEPTIONS
#include "C/Common/MyInitGuid.h"
//#include "C/7zip/Compress/LZMA/LZMADecoder.h"
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
        cb->nprogress(cb, (unsigned) *inSize, (unsigned) *outSize);
    return S_OK;
}

} // namespace


int upx_lzma_compress      ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   upx_callback_p cb,
                                   int method, int level,
                             const struct upx_compress_config_t *conf_parm,
                                   struct upx_compress_result_t *result )
{
    assert(method == M_LZMA);
    assert(level > 0); assert(result != NULL);

    int r = UPX_E_ERROR;
    HRESULT rh;
    lzma_compress_result_t *res = &result->result_lzma;

    MyLzma::InStreamRam is; is.AddRef();
    MyLzma::OutStreamRam os; os.AddRef();
    is.Init(src, src_len);
    os.Init(dst, *dst_len);

    MyLzma::ProgressInfo progress; progress.AddRef();
    progress.cb = cb;

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
    pr[0].vt = pr[1].vt = pr[2].vt = pr[3].vt = VT_UI4;
    pr[4].vt = pr[5].vt = pr[6].vt = VT_UI4;

    // setup defaults
    pr[0].uintVal = 2;              // 0..4
    pr[1].uintVal = 0;              // 0..4
    pr[2].uintVal = 3;              // 0..8
    pr[3].uintVal = 1024 * 1024;
    pr[4].uintVal = 2;
    pr[5].uintVal = 64;             // 5..
    pr[6].uintVal = 0;
#if 1
    // DEBUG - set sizes so that we use a maxmimum amount of stack
    //  these settings cause res->num_probs == 3147574, i.e. we will
    //  need about 6 MB of stack
    pr[1].uintVal = 4;
    pr[2].uintVal = 8;
#endif

    // FIXME: tune these settings according to level
    switch (level)
    {
    case 1:
        pr[3].uintVal = 256 * 1024;
        pr[4].uintVal = 0;
        break;
    case 2:
        break;
    case 3:
        break;
    case 4:
        break;
    case 5:
        break;
    case 6:
        break;
    case 7:
        break;
    case 8:
        break;
    case 9:
        pr[3].uintVal = 8 * 1024 * 1024;
        break;
    case 10:
        pr[3].uintVal = src_len;
        break;
    default:
        goto error;
    }

    // limit dictionary size
    if (pr[3].uintVal > src_len)
        pr[3].uintVal = src_len;

    // limit num_probs
    if (conf_parm && conf_parm->conf_lzma.max_num_probs)
    {
        for (;;)
        {
            unsigned n = 1846 + (768 << (pr[2].uintVal + pr[1].uintVal));
            if (n <= conf_parm->conf_lzma.max_num_probs)
                break;
            if (pr[1].uintVal > pr[2].uintVal)
            {
                if (pr[1].uintVal == 0)
                    goto error;
                pr[1].uintVal -= 1;
            }
            else
            {
                if (pr[2].uintVal == 0)
                    goto error;
                pr[2].uintVal -= 1;
            }
        }
    }

    res->pos_bits = pr[0].uintVal;
    res->lit_pos_bits = pr[1].uintVal;
    res->lit_context_bits = pr[2].uintVal;
    res->dict_size = pr[3].uintVal;
    //res->num_probs = LzmaGetNumProbs(&s.Properties));
    //res->num_probs = (LZMA_BASE_SIZE + (LZMA_LIT_SIZE << ((Properties)->lc + (Properties)->lp)))
    res->num_probs = 1846 + (768 << (res->lit_context_bits + res->lit_pos_bits));

#ifndef _NO_EXCEPTIONS
    try {
#else
#  error
#endif

    if (enc.SetCoderProperties(propIDs, pr, 7) != S_OK)
        goto error;
    if (enc.WriteCoderProperties(&os) != S_OK)
        goto error;
    if (os.Overflow) {
        //r = UPX_E_OUTPUT_OVERRUN;
        r = UPX_E_NOT_COMPRESSIBLE;
        goto error;
    }
    assert(os.Pos == 5);
#if defined(USE_LZMA_PROPERTIES)
    os.Pos = 1;
#else
    os.Pos = 0;
    // extra stuff in first byte: 5 high bits convenience for stub decompressor
    unsigned t = res->lit_context_bits + res->lit_pos_bits;
    os.WriteByte((t << 3) | res->pos_bits);
    os.WriteByte((res->lit_pos_bits << 4) | (res->lit_context_bits));
#endif

    rh = enc.Code(&is, &os, NULL, NULL, &progress);

#ifndef _NO_EXCEPTIONS
    } catch(...) { return UPX_E_OUT_OF_MEMORY; }
#endif

    assert(is.Pos <=  src_len);
    assert(os.Pos <= *dst_len);
    if (rh == E_OUTOFMEMORY)
        r = UPX_E_OUT_OF_MEMORY;
    else if (os.Overflow)
    {
        assert(os.Pos == *dst_len);
        //r = UPX_E_OUTPUT_OVERRUN;
        r = UPX_E_NOT_COMPRESSIBLE;
    }
    else if (rh == S_OK)
    {
        assert(is.Pos == src_len);
        r = UPX_E_OK;
    }

error:
    *dst_len = os.Pos;
    return r;
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
// decompress
**************************************************************************/

#undef _LZMA_IN_CB
#undef _LZMA_OUT_READ
#undef _LZMA_PROB32
#undef _LZMA_LOC_OPT
#include "C/7zip/Compress/LZMA_C/LzmaDecode.h"
#include "C/7zip/Compress/LZMA_C/LzmaDecode.c"

int upx_lzma_decompress    ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   int method,
                             const struct upx_compress_result_t *result )
{
    assert(method == M_LZMA);
    // see res->num_probs above
    COMPILE_TIME_ASSERT(sizeof(CProb) == 2)
    COMPILE_TIME_ASSERT(LZMA_BASE_SIZE == 1846)
    COMPILE_TIME_ASSERT(LZMA_LIT_SIZE == 768)

    CLzmaDecoderState s; memset(&s, 0, sizeof(s));
    SizeT src_out = 0, dst_out = 0;
    int r = UPX_E_ERROR;
    int rh;

#if defined(USE_LZMA_PROPERTIES)
    if (src_len < 2)
        goto error;
    rh = LzmaDecodeProperties(&s.Properties, src, src_len);
    if (rh != 0)
        goto error;
    src += 1; src_len -= 1;
#else
    if (src_len < 3)
        goto error;
    s.Properties.pb = src[0] & 7;
    s.Properties.lp = (src[1] >> 4);
    s.Properties.lc = src[1] & 15;
    if (s.Properties.pb >= 5) goto error;
    if (s.Properties.lp >= 5) goto error;
    if (s.Properties.lc >= 9) goto error;
    // extra
    if ((src[0] >> 3) != s.Properties.lc + s.Properties.lp) goto error;
    src += 2; src_len -= 2;
#endif

    if (result)
    {
        assert(result->method == method);
        assert(result->result_lzma.pos_bits == (unsigned) s.Properties.pb);
        assert(result->result_lzma.lit_pos_bits == (unsigned) s.Properties.lp);
        assert(result->result_lzma.lit_context_bits == (unsigned) s.Properties.lc);
        assert(result->result_lzma.num_probs == (unsigned) LzmaGetNumProbs(&s.Properties));
    }
    s.Probs = (CProb *) malloc(sizeof(CProb) * LzmaGetNumProbs(&s.Properties));
    if (!s.Probs)
    {
        r = UPX_E_OUT_OF_MEMORY;
        goto error;
    }
    rh = LzmaDecode(&s, src, src_len, &src_out, dst, *dst_len, &dst_out);
    assert(src_out <=  src_len);
    assert(dst_out <= *dst_len);
    if (rh == 0)
    {
        r = UPX_E_OK;
        if (src_out != src_len)
            r = UPX_E_INPUT_NOT_CONSUMED;
    }

error:
    *dst_len = dst_out;
    free(s.Probs);

    UNUSED(result);
    return r;
}


/*************************************************************************
// test_overlap
**************************************************************************/

int upx_lzma_test_overlap  ( const upx_bytep buf, unsigned src_off,
                                   unsigned  src_len, unsigned* dst_len,
                                   int method,
                             const struct upx_compress_result_t *result )
{
    assert(method == M_LZMA);

    // FIXME - implement this
    // Note that Packer::verifyOverlappingDecompression() will
    // verify the final result in any case.
    UNUSED(buf);

    unsigned overlap_overhead = src_off + src_len - *dst_len;
    //printf("upx_lzma_test_overlap: %d\n", overlap_overhead);
    if ((int)overlap_overhead >= 256)
        return UPX_E_OK;

    UNUSED(result);
    return UPX_E_ERROR;
}


#endif /* WITH_LZMA */
/*
vi:ts=4:et:nowrap
*/

