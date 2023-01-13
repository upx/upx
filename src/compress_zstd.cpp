/* compress_zstd.cpp --

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

#include "conf.h"

void zstd_compress_config_t::reset() { mem_clear(this, sizeof(*this)); }

#if WITH_ZSTD
#include "compress.h"
#include "util/membuffer.h"
#include <zstd/lib/zstd.h>
#include <zstd/lib/zstd_errors.h>
#include <zstd/lib/compress/hist.h>

static int convert_errno_from_zstd(size_t zr) {
    const ZSTD_ErrorCode ze = ZSTD_getErrorCode(zr);
    switch (ze) {
    case ZSTD_error_memory_allocation:
        return UPX_E_OUT_OF_MEMORY;
    case ZSTD_error_srcSize_wrong:
        return UPX_E_INPUT_OVERRUN;
    case ZSTD_error_dstSize_tooSmall:
        return UPX_E_OUTPUT_OVERRUN;
    default:
        break;
    }
    return UPX_E_ERROR;
}

/*************************************************************************
// TODO later: use advanced compression API for compression finetuning
**************************************************************************/

int upx_zstd_compress(const upx_bytep src, unsigned src_len, upx_bytep dst, unsigned *dst_len,
                      upx_callback_p cb_parm, int method, int level,
                      const upx_compress_config_t *cconf_parm, upx_compress_result_t *cresult) {
    assert(method == M_ZSTD);
    assert(level > 0);
    assert(cresult != nullptr);
    UNUSED(cb_parm);
    int r = UPX_E_ERROR;
    size_t zr;
    const zstd_compress_config_t *const lcconf = cconf_parm ? &cconf_parm->conf_zstd : nullptr;
    zstd_compress_result_t *const res = &cresult->result_zstd;

    // TODO later: map level 1..10 to zstd-level 1..22
    if (level == 10)
        level = 22;

    // cconf overrides
    if (lcconf) {
        UNUSED(lcconf);
    }

    res->dummy = 0;

    zr = ZSTD_compress(dst, *dst_len, src, src_len, level);
    if (ZSTD_isError(zr)) {
        *dst_len = 0; // TODO ???
        r = convert_errno_from_zstd(zr);
        assert(r != UPX_E_OK);
    } else {
        assert(zr <= *dst_len);
        *dst_len = (unsigned) zr;
        r = UPX_E_OK;
    }

    return r;
}

/*************************************************************************
//
**************************************************************************/

int upx_zstd_decompress(const upx_bytep src, unsigned src_len, upx_bytep dst, unsigned *dst_len,
                        int method, const upx_compress_result_t *cresult) {
    assert(method == M_ZSTD);
    UNUSED(method);
    UNUSED(cresult);
    int r = UPX_E_ERROR;
    size_t zr;

    zr = ZSTD_decompress(dst, *dst_len, src, src_len);
    if (ZSTD_isError(zr)) {
        *dst_len = 0; // TODO ???
        r = convert_errno_from_zstd(zr);
        assert(r != UPX_E_OK);
    } else {
        assert(zr <= *dst_len);
        *dst_len = (unsigned) zr;
        r = UPX_E_OK;
    }

    return r;
}

/*************************************************************************
// test_overlap - see <ucl/ucl.h> for semantics
**************************************************************************/

int upx_zstd_test_overlap(const upx_bytep buf, const upx_bytep tbuf, unsigned src_off,
                          unsigned src_len, unsigned *dst_len, int method,
                          const upx_compress_result_t *cresult) {
    assert(method == M_ZSTD);

    MemBuffer b(src_off + src_len);
    memcpy(b + src_off, buf + src_off, src_len);
    unsigned saved_dst_len = *dst_len;
    int r = upx_zstd_decompress(raw_index_bytes(b, src_off, src_len), src_len,
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

int upx_zstd_init(void) {
    if (strcmp(ZSTD_VERSION_STRING, ZSTD_versionString()) != 0)
        return -2;
    return 0;
}

const char *upx_zstd_version_string(void) { return ZSTD_VERSION_STRING; }

/*************************************************************************
// doctest checks
**************************************************************************/

#if DEBUG && !defined(DOCTEST_CONFIG_DISABLE) && 1

#include "util/membuffer.h"

static bool check_zstd(const int method, const int level, const unsigned expected_c_len) {
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
    r = upx_zstd_compress(raw_bytes(u_buf, u_len), u_len, raw_index_bytes(c_buf, c_extra, c_len),
                          &c_len, nullptr, method, level, NULL_cconf, &cresult);
    if (r != 0 || c_len != expected_c_len)
        return false;

    d_len = d_buf.getSize();
    r = upx_zstd_decompress(raw_index_bytes(c_buf, c_extra, c_len), c_len, raw_bytes(d_buf, d_len),
                            &d_len, method, nullptr);
    if (r != 0 || d_len != u_len || memcmp(u_buf, d_buf, u_len) != 0)
        return false;

    d_len = u_len - 1;
    r = upx_zstd_decompress(raw_index_bytes(c_buf, c_extra, c_len), c_len, raw_bytes(d_buf, d_len),
                            &d_len, method, nullptr);
    if (r == 0)
        return false;

    // TODO: rewrite Packer::findOverlapOverhead() so that we can test it here
    // unsigned x_len = d_len;
    // r = upx_zstd_test_overlap(c_buf, u_buf, c_extra, c_len, &x_len, method, nullptr);
    return true;
}

TEST_CASE("compress_zstd") {
    CHECK(check_zstd(M_ZSTD, 1, 19));
    CHECK(check_zstd(M_ZSTD, 3, 19));
    CHECK(check_zstd(M_ZSTD, 5, 19));
}

#endif // DEBUG

TEST_CASE("upx_zstd_decompress") {
    typedef const upx_byte C;
    C *c_data;
    upx_byte d_buf[32];
    unsigned d_len;
    int r;

    c_data = (C *) "\x28\xb5\x2f\xfd\x20\x20\x3d\x00\x00\x08\xff\x01\x00\x34\x4e\x08";
    d_len = 32;
    r = upx_zstd_decompress(c_data, 16, d_buf, &d_len, M_ZSTD, nullptr);
    CHECK((r == 0 && d_len == 32));
    r = upx_zstd_decompress(c_data, 15, d_buf, &d_len, M_ZSTD, nullptr);
    CHECK(r == UPX_E_INPUT_OVERRUN);
    d_len = 31;
    r = upx_zstd_decompress(c_data, 16, d_buf, &d_len, M_ZSTD, nullptr);
    CHECK(r == UPX_E_OUTPUT_OVERRUN);
}

#endif // WITH_ZSTD

/* vim:set ts=4 sw=4 et: */
