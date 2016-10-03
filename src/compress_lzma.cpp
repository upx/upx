/* compress_lzma.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2016 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2016 Laszlo Molnar
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
   <markus@oberhumer.com>               <ezerotven+github@gmail.com>
 */


#include "conf.h"
#include "compress.h"
#include "mem.h"

#if (ACC_CC_MSC)
#  pragma warning(disable: 4456) // -Wno-shadow
#endif
#if (ACC_CC_MSC && (_MSC_VER < 1900))
#  pragma warning(disable: 4127) // warning C4127: conditional expression is constant
#endif


void lzma_compress_config_t::reset()
{
    memset(this, 0, sizeof(*this));

    pos_bits.reset();
    lit_pos_bits.reset();
    lit_context_bits.reset();
    dict_size.reset();
    fast_mode = 2;
    num_fast_bytes.reset();
    match_finder_cycles = 0;

    max_num_probs = 0;
}


#if !(WITH_LZMA)
extern int compress_lzma_dummy;
int compress_lzma_dummy = 0;
#else
#define opt compress_lzma_opt

#undef USE_LZMA_PROPERTIES


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


#if (WITH_LZMA >= 0x461)
#include "C/7zVersion.h"
#if (WITH_LZMA != ((0x1000 * (MY_VER_MAJOR / 10)) + (0x0100 * (MY_VER_MAJOR % 10)) +\
                   (0x0010 * (MY_VER_MINOR / 10)) + (0x0001 * (MY_VER_MINOR % 10))))
#  error "WITH_LZMA version mismatch"
#endif

#if (WITH_LZMA >= 0x938)
#include "C/7zTypes.h"
#else
#include "C/Types.h"
#endif

static void *cb_alloc(void *, size_t size) {
    return malloc(size);
}
static void cb_free(void *, void *ptr) {
    free(ptr);
}
#endif


/*************************************************************************
// compress defaults
**************************************************************************/

static int prepare(lzma_compress_result_t *res,
                   unsigned src_len, int method, int level,
                   const lzma_compress_config_t *lcconf)
{
    // setup defaults
    res->pos_bits            = 2;                   // 0 .. 4
    res->lit_pos_bits        = 0;                   // 0 .. 4
    res->lit_context_bits    = 3;                   // 0 .. 8
    res->dict_size           = 4 * 1024 * 1024;     // 1 .. 2**30
    res->fast_mode           = 2;
    res->num_fast_bytes      = 64;                  // 5 .. 273
    res->match_finder_cycles = 0;
#if 1
    res->pos_bits            = lzma_compress_config_t::pos_bits_t::default_value_c;
    res->lit_pos_bits        = lzma_compress_config_t::lit_pos_bits_t::default_value_c;
    res->lit_context_bits    = lzma_compress_config_t::lit_context_bits_t::default_value_c;
    res->dict_size           = lzma_compress_config_t::dict_size_t::default_value_c;
    res->num_fast_bytes      = lzma_compress_config_t::num_fast_bytes_t::default_value_c;
#endif
    // method overrides
    if (method >= 0x100) {
        res->pos_bits         = (method >> 16) & 15;
        res->lit_pos_bits     = (method >> 12) & 15;
        res->lit_context_bits = (method >>  8) & 15;
    }
#if 0
    // DEBUG - set sizes so that we use a maxmimum amount of stack.
    //  These settings cause res->num_probs == 3147574, i.e. we will
    //  need about 6 MiB of stack during runtime decompression.
    res->lit_pos_bits     = 4;
    res->lit_context_bits = 8;
#endif

    // FIXME: tune these settings according to level
    switch (level)
    {
    case 1:
        res->dict_size      = 256 * 1024;
        res->fast_mode      = 0;
        res->num_fast_bytes = 8;
        break;
    case 2:
        res->dict_size      = 256 * 1024;
        res->fast_mode      = 0;
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
        res->dict_size      = 8 * 1024 * 1024;
        break;
    case 10:
        res->dict_size      = src_len;
        break;
    default:
        goto error;
    }

    // cconf overrides
    if (lcconf)
    {
        oassign(res->pos_bits, lcconf->pos_bits);
        oassign(res->lit_pos_bits, lcconf->lit_pos_bits);
        oassign(res->lit_context_bits, lcconf->lit_context_bits);
        oassign(res->dict_size, lcconf->dict_size);
        oassign(res->num_fast_bytes, lcconf->num_fast_bytes);
    }

    // limit dictionary size
    if (res->dict_size > src_len)
        res->dict_size = src_len;

    // limit num_probs
    if (lcconf && lcconf->max_num_probs)
    {
        for (;;)
        {
            unsigned n = 1846 + (768 << (res->lit_context_bits + res->lit_pos_bits));
            if (n <= lcconf->max_num_probs)
                break;
            if (res->lit_pos_bits > res->lit_context_bits)
            {
                if (res->lit_pos_bits == 0)
                    goto error;
                res->lit_pos_bits -= 1;
            }
            else
            {
                if (res->lit_context_bits == 0)
                    goto error;
                res->lit_context_bits -= 1;
            }
        }
    }

    res->num_probs = 1846 + (768 << (res->lit_context_bits + res->lit_pos_bits));
    //printf("\nlzma_compress config: %u %u %u %u %u\n", res->pos_bits, res->lit_pos_bits, res->lit_context_bits, res->dict_size, res->num_probs);
    return 0;

error:
    return -1;
}


/*************************************************************************
// compress
**************************************************************************/

#if (WITH_LZMA >= 0x461)
#define _7ZIP_ST 1
#define kLiteralNextStates kLiteralNextStates_enc
#include "C/LzmaEnc.h"
#include "C/LzmaEnc.c"
#include "C/LzFind.c"
#undef kLiteralNextStates
#undef kNumFullDistances


struct upx_ICompressProgress {
    SRes (*Progress)(void *p, UInt64 inSize, UInt64 outSize);
    upx_callback_p cb;
};

static SRes cb_progress(void *p, UInt64 inSize, UInt64 outSize) {
    upx_callback_p cb = ((upx_ICompressProgress *) p)->cb;
    if (cb && cb->nprogress)
        cb->nprogress(cb, (unsigned) inSize, (unsigned) outSize);
    return SZ_OK;
}


int upx_lzma_compress      ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   upx_callback_p cb,
                                   int method, int level,
                             const upx_compress_config_t *cconf_parm,
                                   upx_compress_result_t *cresult )
{
    assert(M_IS_LZMA(method));
    assert(level > 0); assert(cresult != NULL);
    COMPILE_TIME_ASSERT(LZMA_PROPS_SIZE == 5)

    int r = UPX_E_ERROR;
    int rh;

    unsigned dst_avail = *dst_len;
    *dst_len = 0;

    ISzAlloc cba; cba.Alloc = cb_alloc; cba.Free = cb_free;
    upx_ICompressProgress progress;
    progress.Progress = cb_progress; progress.cb = cb;

    const lzma_compress_config_t *lcconf = cconf_parm ? &cconf_parm->conf_lzma : NULL;
    lzma_compress_result_t *res = &cresult->result_lzma;
    if (prepare(res, src_len, method, level, lcconf) != 0)
        goto error;

    CLzmaEncProps props; memset(&props, 0, sizeof(props));
    props.level    = level;
    LzmaEncProps_Init(&props);
    props.pb       = res->pos_bits;
    props.lp       = res->lit_pos_bits;
    props.lc       = res->lit_context_bits;
    props.dictSize = res->dict_size;
    props.algo     = res->fast_mode ? 1 : 0;
    props.fb       = res->num_fast_bytes;
    props.mc       = res->match_finder_cycles;

    unsigned char props_buf[LZMA_PROPS_SIZE];
    SizeT props_buf_size; props_buf_size = LZMA_PROPS_SIZE;

#if (USE_LZMA_PROPERTIES)
    if (dst_avail < 1)
        goto error;
    dst[0] = 0; // filled below
    dst += 1; *dst_len += 1; dst_avail -= 1;
#else
    if (dst_avail < 2)
        goto error;
    // extra stuff in first byte: 5 high bits convenience for stub decompressor
    unsigned tt; tt = res->lit_context_bits + res->lit_pos_bits;
    dst[0] = ((tt << 3) | res->pos_bits);
    dst[1] = ((res->lit_pos_bits << 4) | (res->lit_context_bits));
    dst += 2; *dst_len += 2; dst_avail -= 2;
#endif
    SizeT x_len; x_len = dst_avail;
    rh = LzmaEncode(dst, &x_len, src, src_len,
                    &props, props_buf, &props_buf_size, 0,
                    (ICompressProgress *) (void *) &progress, &cba, &cba);
    assert(x_len <= dst_avail);
    *dst_len += x_len;
    if (rh == SZ_OK) {
#if (USE_LZMA_PROPERTIES)
        dst[-1] = props_buf[0];
#endif
        r = UPX_E_OK;
    }

error:
    //printf("\nlzma_compress: %d: %u %u %u %u %u, %u - > %u\n", r, res->pos_bits, res->lit_pos_bits, res->lit_context_bits, res->dict_size, res->num_probs, src_len, *dst_len);
    return r;
}

#endif /* (WITH_LZMA >= 0x461) */


/*************************************************************************
// compress - cruft because of pseudo-COM layer
**************************************************************************/

#if (WITH_LZMA < 0x461)

#undef MSDOS
#undef OS2
#undef _WIN32
#undef _WIN32_WCE
#undef COMPRESS_MF_MT
#undef _NO_EXCEPTIONS
#if (WITH_LZMA >= 0x449)
#  define INITGUID 1
//#  include "CPP/7zip/Compress/LZMA/LZMADecoder.h"
#  include "CPP/7zip/Compress/LZMA/LZMAEncoder.h"
#else
#  include "C/Common/MyInitGuid.h"
//#  include "C/7zip/Compress/LZMA/LZMADecoder.h"
#  include "C/7zip/Compress/LZMA/LZMAEncoder.h"
#endif

namespace MyLzma {

struct InStream: public ISequentialInStream, public CMyUnknownImp
{
    virtual ~InStream() { }
    MY_UNKNOWN_IMP
    const Byte *b_buf; size_t b_size; size_t b_pos;
    void Init(const Byte *data, size_t size) {
        b_buf = data; b_size = size; b_pos = 0;
    }
    STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize);
};

STDMETHODIMP InStream::Read(void *data, UInt32 size, UInt32 *processedSize)
{
    size_t remain = b_size - b_pos;
    if (size > remain) size = (UInt32) remain;
    memcpy(data, b_buf + b_pos, size);
    b_pos += size;
    if (processedSize != NULL) *processedSize = size;
    return S_OK;
}

struct OutStream : public ISequentialOutStream, public CMyUnknownImp
{
    virtual ~OutStream() { }
    MY_UNKNOWN_IMP
    Byte *b_buf; size_t b_size; size_t b_pos; bool overflow;
    void Init(Byte *data, size_t size) {
        b_buf = data; b_size = size; b_pos = 0; overflow = false;
    }
    HRESULT WriteByte(Byte c) {
        if (b_pos >= b_size) { overflow = true; return E_FAIL; }
        b_buf[b_pos++] = c;
        return S_OK;
    }
    STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize);
};

STDMETHODIMP OutStream::Write(const void *data, UInt32 size, UInt32 *processedSize)
{
    size_t remain = b_size - b_pos;
    if (size > remain) size = (UInt32) remain, overflow = true;
    memcpy(b_buf + b_pos, data, size);
    b_pos += size;
    if (processedSize != NULL) *processedSize = size;
    return overflow ? E_FAIL : S_OK;
}

struct ProgressInfo : public ICompressProgressInfo, public CMyUnknownImp
{
    virtual ~ProgressInfo() { }
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


#if (ACC_CC_INTELC_GNUC)
//#  pragma warning(disable: 424)         // #424: extra ";" ignored
#  pragma warning(error: 424)         // #424: extra ";" ignored
#endif

#if (WITH_LZMA >= 0x449)
#  include "C/Alloc.c"
#  include "C/7zCrc.c"
#  include "C/Compress/Lz/MatchFinder.c"
//#  include "CPP/7zip/Common/InBuffer.cpp"
#  include "CPP/7zip/Common/OutBuffer.cpp"
#  include "CPP/7zip/Common/StreamUtils.cpp"
//#  include "CPP/7zip/Compress/LZ/LZOutWindow.cpp"
//#  include "CPP/7zip/Compress/LZMA/LZMADecoder.cpp"
#  include "CPP/7zip/Compress/LZMA/LZMAEncoder.cpp"
#  include "CPP/7zip/Compress/RangeCoder/RangeCoderBit.cpp"
#else
#  include "C/Common/Alloc.cpp"
#  include "C/Common/CRC.cpp"
//#  include "C/7zip/Common/InBuffer.cpp"
#  include "C/7zip/Common/OutBuffer.cpp"
#  include "C/7zip/Common/StreamUtils.cpp"
#  include "C/7zip/Compress/LZ/LZInWindow.cpp"
//#  include "C/7zip/Compress/LZ/LZOutWindow.cpp"
//#  include "C/7zip/Compress/LZMA/LZMADecoder.cpp"
#  include "C/7zip/Compress/LZMA/LZMAEncoder.cpp"
#  include "C/7zip/Compress/RangeCoder/RangeCoderBit.cpp"
#endif
#undef RC_NORMALIZE


int upx_lzma_compress      ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   upx_callback_p cb,
                                   int method, int level,
                             const upx_compress_config_t *cconf_parm,
                                   upx_compress_result_t *cresult )
{
    assert(M_IS_LZMA(method));
    assert(level > 0); assert(cresult != NULL);

    int r = UPX_E_ERROR;
    HRESULT rh;
    const lzma_compress_config_t *lcconf = cconf_parm ? &cconf_parm->conf_lzma : NULL;
    lzma_compress_result_t *res = &cresult->result_lzma;

    MyLzma::InStream is; is.AddRef();
    MyLzma::OutStream os; os.AddRef();
    is.Init(src, src_len);
    os.Init(dst, *dst_len);

    MyLzma::ProgressInfo progress; progress.AddRef();
    progress.cb = cb;

    NCompress::NLZMA::CEncoder enc;
    const PROPID propIDs[8] = {
        NCoderPropID::kPosStateBits,        // 0  pb    _posStateBits(2)
        NCoderPropID::kLitPosBits,          // 1  lp    _numLiteralPosStateBits(0)
        NCoderPropID::kLitContextBits,      // 2  lc    _numLiteralContextBits(3)
        NCoderPropID::kDictionarySize,      // 3  ds
        NCoderPropID::kAlgorithm,           // 4  fm    _fastmode
        NCoderPropID::kNumFastBytes,        // 5  fb
        NCoderPropID::kMatchFinderCycles,   // 6  mfc   _matchFinderCycles, _cutValue
        NCoderPropID::kMatchFinder          // 7  mf
    };
    PROPVARIANT pr[8];
#ifdef COMPRESS_MF_BT4
    const unsigned nprops = 8;
    static wchar_t matchfinder[] = L"BT4";
    assert(NCompress::NLZMA::FindMatchFinder(matchfinder) >= 0);
    pr[7].vt = VT_BSTR; pr[7].bstrVal = matchfinder;
#else
    const unsigned nprops = 7;
#endif
    pr[0].vt = pr[1].vt = pr[2].vt = pr[3].vt = VT_UI4;
    pr[4].vt = pr[5].vt = pr[6].vt = VT_UI4;
    if (prepare(res, src_len, method, level, lcconf) != 0)
        goto error;
    pr[0].uintVal = res->pos_bits;
    pr[1].uintVal = res->lit_pos_bits;
    pr[2].uintVal = res->lit_context_bits;
    pr[3].uintVal = res->dict_size;
    pr[4].uintVal = res->fast_mode;
    pr[5].uintVal = res->num_fast_bytes;
    pr[6].uintVal = res->match_finder_cycles;

#ifndef _NO_EXCEPTIONS
    try {
#else
#  error
#endif

    if (enc.SetCoderProperties(propIDs, pr, nprops) != S_OK)
        goto error;
    if (enc.WriteCoderProperties(&os) != S_OK)
        goto error;
    if (os.overflow) {
        //r = UPX_E_OUTPUT_OVERRUN;
        r = UPX_E_NOT_COMPRESSIBLE;
        goto error;
    }
    assert(os.b_pos == 5);
#if (USE_LZMA_PROPERTIES)
    os.b_pos = 1;
#else
    os.b_pos = 0;
    // extra stuff in first byte: 5 high bits convenience for stub decompressor
    unsigned t = res->lit_context_bits + res->lit_pos_bits;
    os.WriteByte(Byte((t << 3) | res->pos_bits));
    os.WriteByte(Byte((res->lit_pos_bits << 4) | (res->lit_context_bits)));
#endif

    rh = enc.Code(&is, &os, NULL, NULL, &progress);

#ifndef _NO_EXCEPTIONS
    } catch (...) { rh = E_OUTOFMEMORY; }
#endif

    assert(is.b_pos <=  src_len);
    assert(os.b_pos <= *dst_len);
    if (rh == E_OUTOFMEMORY)
        r = UPX_E_OUT_OF_MEMORY;
    else if (os.overflow)
    {
        assert(os.b_pos == *dst_len);
        //r = UPX_E_OUTPUT_OVERRUN;
        r = UPX_E_NOT_COMPRESSIBLE;
    }
    else if (rh == S_OK)
    {
        assert(is.b_pos == src_len);
        r = UPX_E_OK;
    }

error:
    *dst_len = (unsigned) os.b_pos;
    //printf("\nlzma_compress: %d: %u %u %u %u %u, %u - > %u\n", r, res->pos_bits, res->lit_pos_bits, res->lit_context_bits, res->dict_size, res->num_probs, src_len, *dst_len);
    //printf("%u %u %u\n", is.__m_RefCount, os.__m_RefCount, progress.__m_RefCount);
    return r;
}

#endif /* (WITH_LZMA < 0x461) */


/*************************************************************************
// decompress
**************************************************************************/

#if (WITH_LZMA >= 0x461)

#undef _LZMA_PROB32
#include "C/LzmaDec.h"
#include "C/LzmaDec.c"


int upx_lzma_decompress    ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   int method,
                             const upx_compress_result_t *cresult )
{
#define Properties prop
    assert(M_IS_LZMA(method));
    // see res->num_probs above
    COMPILE_TIME_ASSERT(sizeof(CLzmaProb) == 2)
    COMPILE_TIME_ASSERT(LZMA_PROPS_SIZE == 5)
    COMPILE_TIME_ASSERT(LZMA_BASE_SIZE == 1846)
    COMPILE_TIME_ASSERT(LZMA_LIT_SIZE == 768)

    ISzAlloc cba; cba.Alloc = cb_alloc; cba.Free = cb_free;
    CLzmaDec s; memset(&s, 0, sizeof(s));
    SizeT srcLen = 0;
    int r = UPX_E_ERROR;
    SRes rh;
    ELzmaStatus status;

#if (USE_LZMA_PROPERTIES)
    rh = LzmaProps_Decode(&s.Properties, src, src_len);
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
    if ((src[0] >> 3) != (int)(s.Properties.lc + s.Properties.lp)) goto error;
    src += 2; src_len -= 2;
#endif
    s.Properties.dicSize = 0;

    if (cresult)
    {
        assert(cresult->method == method);
        assert(cresult->result_lzma.pos_bits == (unsigned) s.Properties.pb);
        assert(cresult->result_lzma.lit_pos_bits == (unsigned) s.Properties.lp);
        assert(cresult->result_lzma.lit_context_bits == (unsigned) s.Properties.lc);
        assert(cresult->result_lzma.num_probs == (unsigned) LzmaProps_GetNumProbs(&s.Properties));
        const lzma_compress_result_t *res = &cresult->result_lzma;
        UNUSED(res);
        //printf("\nlzma_decompress config: %u %u %u %u %u\n", res->pos_bits, res->lit_pos_bits, res->lit_context_bits, res->dict_size, res->num_probs);
    }

    rh = LzmaDec_AllocateProbs2(&s, &s.Properties, &cba);
    if (rh != SZ_OK)
    {
        r = UPX_E_OUT_OF_MEMORY;
        goto error;
    }

    s.dic = dst; s.dicPos = 0; s.dicBufSize = *dst_len;
    LzmaDec_Init(&s);
    srcLen = src_len;
    rh = LzmaDec_DecodeToDic(&s, *dst_len, src, &srcLen, LZMA_FINISH_ANY, &status);
    assert(srcLen <= src_len);
    assert(s.dicPos <= *dst_len);
    if (rh == SZ_OK && status == LZMA_STATUS_NEEDS_MORE_INPUT)
        rh = SZ_ERROR_INPUT_EOF;
    if (rh == SZ_OK && status == LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK)
    {
        r = UPX_E_OK;
        if (srcLen != src_len)
            r = UPX_E_INPUT_NOT_CONSUMED;
    }
error:
    *dst_len = s.dicPos;
    LzmaDec_FreeProbs(&s, &cba);
    return r;
#undef Properties
}


#endif /* (WITH_LZMA >= 0x461) */


/*************************************************************************
// decompress
**************************************************************************/

#if (WITH_LZMA < 0x461)

#undef _LZMA_IN_CB
#undef _LZMA_OUT_READ
#undef _LZMA_PROB32
#undef _LZMA_LOC_OPT
#if (WITH_LZMA >= 0x449)
#  include "C/Compress/Lzma/LzmaDecode.h"
#  include "C/Compress/Lzma/LzmaDecode.c"
#else
#  include "C/7zip/Compress/LZMA_C/LzmaDecode.h"
#  include "C/7zip/Compress/LZMA_C/LzmaDecode.c"
#endif

int upx_lzma_decompress    ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   int method,
                             const upx_compress_result_t *cresult )
{
    assert(M_IS_LZMA(method));
    // see res->num_probs above
    COMPILE_TIME_ASSERT(sizeof(CProb) == 2)
    COMPILE_TIME_ASSERT(LZMA_BASE_SIZE == 1846)
    COMPILE_TIME_ASSERT(LZMA_LIT_SIZE == 768)

    CLzmaDecoderState s; memset(&s, 0, sizeof(s));
    SizeT src_out = 0, dst_out = 0;
    int r = UPX_E_ERROR;
    int rh;

#if (USE_LZMA_PROPERTIES)
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

    if (cresult)
    {
        assert(cresult->method == method);
        assert(cresult->result_lzma.pos_bits == (unsigned) s.Properties.pb);
        assert(cresult->result_lzma.lit_pos_bits == (unsigned) s.Properties.lp);
        assert(cresult->result_lzma.lit_context_bits == (unsigned) s.Properties.lc);
        assert(cresult->result_lzma.num_probs == (unsigned) LzmaGetNumProbs(&s.Properties));
        const lzma_compress_result_t *res = &cresult->result_lzma;
        UNUSED(res);
        //printf("\nlzma_decompress config: %u %u %u %u %u\n", res->pos_bits, res->lit_pos_bits, res->lit_context_bits, res->dict_size, res->num_probs);
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
    return r;
}

#endif /* (WITH_LZMA < 0x461) */


/*************************************************************************
// test_overlap - see <ucl/ucl.h> for semantics
**************************************************************************/

int upx_lzma_test_overlap  ( const upx_bytep buf,
                             const upx_bytep tbuf,
                                   unsigned  src_off, unsigned src_len,
                                   unsigned* dst_len,
                                   int method,
                             const upx_compress_result_t *cresult )
{
    assert(M_IS_LZMA(method));

    MemBuffer b(src_off + src_len);
    memcpy(b + src_off, buf + src_off, src_len);
    unsigned saved_dst_len = *dst_len;
    int r = upx_lzma_decompress(b + src_off, src_len, b, dst_len, method, cresult);
    if (r != UPX_E_OK)
        return r;
    if (*dst_len != saved_dst_len)
        return UPX_E_ERROR;
    // NOTE: there is a very tiny possibility that decompression has
    //   succeeded but the data is not restored correctly because of
    //   in-place buffer overlapping.
    if (tbuf != NULL && memcmp(tbuf, b, *dst_len) != 0)
        return UPX_E_ERROR;
    return UPX_E_OK;
}


/*************************************************************************
// misc
**************************************************************************/

int upx_lzma_init(void)
{
    return 0;
}

const char *upx_lzma_version_string(void)
{
#if (WITH_LZMA >= 0x461)
#   error "invalid WITH_LZMA version"
    return MY_VERSION;
#elif (WITH_LZMA + 0 == 0x457)
#   error "invalid WITH_LZMA version"
    return "4.57";
#elif (WITH_LZMA + 0 == 0x449)
#   error "invalid WITH_LZMA version"
    return "4.49";
#elif (WITH_LZMA + 0 == 0x443)
    return "4.43";
#else
#   error "unknown WITH_LZMA version"
    return NULL;
#endif
}


#endif /* WITH_LZMA */

/* vim:set ts=4 sw=4 et: */
