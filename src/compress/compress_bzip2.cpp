/* compress_bzip2.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
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

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
 */

#include "../conf.h"

void bzip2_compress_config_t::reset() noexcept { mem_clear(this); }

#if WITH_BZIP2
#include "compress.h"
#include "../util/membuffer.h"
#include <bzip2/bzlib.h>

#if defined(BZ_NO_STDIO) || 1
// we need to supply bz_internal_error() when building with BZ_NO_STDIO
extern "C" {
extern void bz_internal_error(int);
void bz_internal_error(int errcode) { throwInternalError("bz_internal_error %d", errcode); }
}
#endif // BZ_NO_STDIO

static int convert_errno_from_bzip2(int r) {
    switch (r) {
    case BZ_OK:
        return UPX_E_OK;
    case BZ_MEM_ERROR:
        return UPX_E_OUT_OF_MEMORY;
    // TODO later: convert to UPX_E_INPUT_OVERRUN, UPX_E_OUTPUT_OVERRUN
    default:
        break;
    }
    return UPX_E_ERROR;
}

/*************************************************************************
//
**************************************************************************/

int upx_bzip2_compress(const upx_bytep src, unsigned src_len, upx_bytep dst, unsigned *dst_len,
                       upx_callback_t *cb_parm, int method, int level,
                       const upx_compress_config_t *cconf_parm, upx_compress_result_t *cresult) {
    assert(method == M_BZIP2);
    assert(level > 0);
    assert(cresult != nullptr);
    UNUSED(cb_parm);
    int r = UPX_E_ERROR;
    const bzip2_compress_config_t *const lcconf = cconf_parm ? &cconf_parm->conf_bzip2 : nullptr;
    bzip2_compress_result_t *const res = &cresult->result_bzip2;
    res->reset();

    int blockSize100k = (src_len + 100000 - 1) / 100000;
    if (blockSize100k < 1)
        blockSize100k = 1;
    if (blockSize100k > 9)
        blockSize100k = 9;
    // idea for later: could enhance lcconf to allow setting blockSize100k
    UNUSED(lcconf);
    if (level <= 3 && blockSize100k > level)
        blockSize100k = level;

    char *dest = (char *) dst;
    char *source = (char *) const_cast<byte *>(src);
    r = BZ2_bzBuffToBuffCompress(dest, dst_len, source, src_len, blockSize100k, 0, 0);
    return convert_errno_from_bzip2(r);
}

/*************************************************************************
//
**************************************************************************/

int upx_bzip2_decompress(const upx_bytep src, unsigned src_len, upx_bytep dst, unsigned *dst_len,
                         int method, const upx_compress_result_t *cresult) {
    assert(method == M_BZIP2);
    UNUSED(method);
    UNUSED(cresult);
    char *dest = (char *) dst;
    char *source = (char *) const_cast<byte *>(src);
    int small = 0;
    int r = BZ2_bzBuffToBuffDecompress(dest, dst_len, source, src_len, small, 0);
    return convert_errno_from_bzip2(r);
}

/*************************************************************************
// test_overlap - see <ucl/ucl.h> for semantics
**************************************************************************/

int upx_bzip2_test_overlap(const upx_bytep buf, const upx_bytep tbuf, unsigned src_off,
                           unsigned src_len, unsigned *dst_len, int method,
                           const upx_compress_result_t *cresult) {
    assert(method == M_BZIP2);

    MemBuffer b(src_off + src_len);
    memcpy(b + src_off, buf + src_off, src_len);
    unsigned saved_dst_len = *dst_len;
    int r = upx_bzip2_decompress(raw_index_bytes(b, src_off, src_len), src_len,
                                 raw_bytes(b, *dst_len), dst_len, method, cresult);
    if (r != UPX_E_OK)
        return r;
    if (*dst_len != saved_dst_len)
        return UPX_E_ERROR;
    // NOTE: there is a very tiny possibility that decompression has
    //   succeeded but the data is not restored correctly because of
    //   in-place buffer overlapping, so we use an extra memcmp().
    if (tbuf != nullptr && memcmp(tbuf, b, *dst_len) != 0)
        return UPX_E_ERROR;
    return UPX_E_OK;
}

/*************************************************************************
// misc
**************************************************************************/

int upx_bzip2_init(void) { return 0; }

const char *upx_bzip2_version_string(void) { return BZ2_bzlibVersion(); }

/*************************************************************************
// doctest checks
**************************************************************************/

#if DEBUG && !defined(DOCTEST_CONFIG_DISABLE) && 1

static bool check_bzip2(const int method, const int level, const unsigned expected_c_len) {
    const unsigned u_len = 16384;
    const unsigned c_extra = 4096;
    MemBuffer u_buf, c_buf, d_buf;
    unsigned c_len, d_len;
    upx_compress_result_t cresult;
    int r;

    u_buf.alloc(u_len);
    memset(u_buf, 0, u_len);
    c_buf.allocForCompression(u_len, c_extra);
    d_buf.allocForDecompression(u_len);

    c_len = c_buf.getSize() - c_extra;
    r = upx_bzip2_compress(raw_bytes(u_buf, u_len), u_len, raw_index_bytes(c_buf, c_extra, c_len),
                           &c_len, nullptr, method, level, NULL_cconf, &cresult);
    if (r != 0 || c_len != expected_c_len)
        return false;

    d_len = d_buf.getSize();
    r = upx_bzip2_decompress(raw_index_bytes(c_buf, c_extra, c_len), c_len, raw_bytes(d_buf, d_len),
                             &d_len, method, nullptr);
    if (r != 0 || d_len != u_len || memcmp(u_buf, d_buf, u_len) != 0)
        return false;

    d_len = u_len - 1;
    r = upx_bzip2_decompress(raw_index_bytes(c_buf, c_extra, c_len), c_len, raw_bytes(d_buf, d_len),
                             &d_len, method, nullptr);
    if (r == 0)
        return false;

    // TODO: rewrite Packer::findOverlapOverhead() so that we can test it here
    // unsigned x_len = d_len;
    // r = upx_bzip2_test_overlap(c_buf, u_buf, c_extra, c_len, &x_len, method, nullptr);
    return true;
}

TEST_CASE("compress_bzip2") { CHECK(check_bzip2(M_BZIP2, 9, 46)); }

#endif // DEBUG

TEST_CASE("upx_bzip2_decompress") {
#if 0  // TODO later, see above
    const byte *c_data;
    byte d_buf[32];
    unsigned d_len;
    int r;

    c_data = (const byte *) "\x28\xb5\x2f\xfd\x20\x20\x3d\x00\x00\x08\xff\x01\x00\x34\x4e\x08";
    d_len = 32;
    r = upx_bzip2_decompress(c_data, 16, d_buf, &d_len, M_BZIP2, nullptr);
    CHECK((r == 0 && d_len == 32));
    r = upx_bzip2_decompress(c_data, 15, d_buf, &d_len, M_BZIP2, nullptr);
    CHECK(r == UPX_E_INPUT_OVERRUN);
    d_len = 31;
    r = upx_bzip2_decompress(c_data, 16, d_buf, &d_len, M_BZIP2, nullptr);
    CHECK(r == UPX_E_OUTPUT_OVERRUN);
    UNUSED(r);
#endif // TODO
}

#endif // WITH_BZIP2

/* vim:set ts=4 sw=4 et: */
