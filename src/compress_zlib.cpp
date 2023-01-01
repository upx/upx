/* compress_zlib.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2023 Laszlo Molnar
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
#include "util/membuffer.h"
#include <zlib/zlib.h>
#include <zlib/deflate.h>

void zlib_compress_config_t::reset() {
    mem_clear(this, sizeof(*this));

    mem_level.reset();
    window_bits.reset();
    strategy.reset();
}

static int convert_errno_from_zlib(int zr) {
    switch (zr) {
    case Z_OK:
        return UPX_E_OK;
    // positive values
    case Z_STREAM_END:
        return UPX_E_ERROR;
    case Z_NEED_DICT:
        return UPX_E_ERROR;
    // negative values
    case Z_ERRNO:
        return UPX_E_ERROR;
    case Z_STREAM_ERROR:
        return UPX_E_ERROR;
    case Z_DATA_ERROR:
        return UPX_E_ERROR;
    case Z_MEM_ERROR:
        return UPX_E_OUT_OF_MEMORY;
    case Z_BUF_ERROR:
        return UPX_E_OUTPUT_OVERRUN;
    case Z_VERSION_ERROR:
        return UPX_E_ERROR;
    case -7: // UPX extra
        return UPX_E_INPUT_OVERRUN;
    }
    return UPX_E_ERROR;
}

/*************************************************************************
//
**************************************************************************/

int upx_zlib_compress(const upx_bytep src, unsigned src_len, upx_bytep dst, unsigned *dst_len,
                      upx_callback_p cb_parm, int method, int level,
                      const upx_compress_config_t *cconf_parm, upx_compress_result_t *cresult) {
    assert(method == M_DEFLATE);
    assert(level > 0);
    assert(cresult != nullptr);
    UNUSED(cb_parm);
    int r = UPX_E_ERROR;
    int zr;
    const zlib_compress_config_t *const lcconf = cconf_parm ? &cconf_parm->conf_zlib : nullptr;
    zlib_compress_result_t *const res = &cresult->result_zlib;

    if (level == 10)
        level = 9;

    zlib_compress_config_t::mem_level_t mem_level;
    zlib_compress_config_t::window_bits_t window_bits;
    zlib_compress_config_t::strategy_t strategy;
    // cconf overrides
    if (lcconf) {
        oassign(mem_level, lcconf->mem_level);
        oassign(window_bits, lcconf->window_bits);
        oassign(strategy, lcconf->strategy);
    }

    res->dummy = 0;

    z_stream s;
    s.zalloc = (alloc_func) nullptr;
    s.zfree = (free_func) nullptr;
    s.next_in = ACC_UNCONST_CAST(upx_bytep, src);
    s.avail_in = src_len;
    s.next_out = dst;
    s.avail_out = *dst_len;
    s.total_in = s.total_out = 0;

    zr = (int) deflateInit2(&s, level, Z_DEFLATED, 0 - (int) window_bits, mem_level, strategy);
    if (zr != Z_OK)
        goto error;
    assert(s.state->level == level);
    zr = deflate(&s, Z_FINISH);
    if (zr != Z_STREAM_END)
        goto error;
    zr = deflateEnd(&s);
    if (zr != Z_OK)
        goto error;
    r = UPX_E_OK;
    goto done;
error:
    (void) deflateEnd(&s);
    r = convert_errno_from_zlib(zr);
    if (r == UPX_E_OK)
        r = UPX_E_ERROR;
done:
    if (r == UPX_E_OK) {
        if (s.avail_in != 0 || s.total_in != src_len)
            r = UPX_E_ERROR;
    }
    assert(s.total_in <= src_len);
    assert(s.total_out <= *dst_len);
    *dst_len = s.total_out;
    return r;
}

/*************************************************************************
//
**************************************************************************/

int upx_zlib_decompress(const upx_bytep src, unsigned src_len, upx_bytep dst, unsigned *dst_len,
                        int method, const upx_compress_result_t *cresult) {
    assert(method == M_DEFLATE);
    UNUSED(method);
    UNUSED(cresult);
    int r = UPX_E_ERROR;
    int zr;

    z_stream s;
    s.zalloc = (alloc_func) nullptr;
    s.zfree = (free_func) nullptr;
    s.next_in = ACC_UNCONST_CAST(upx_bytep, src);
    s.avail_in = src_len;
    s.next_out = dst;
    s.avail_out = *dst_len;
    s.total_in = s.total_out = 0;

    zr = inflateInit2(&s, -15);
    if (zr != Z_OK)
        goto error;
    zr = inflate(&s, Z_FINISH);
    if (zr != Z_STREAM_END) {
        if (zr == Z_BUF_ERROR && s.avail_in == 0)
            zr = -7; // UPX extra
        goto error;
    }
    zr = inflateEnd(&s);
    if (zr != Z_OK)
        goto error;
    r = UPX_E_OK;
    goto done;
error:
    (void) inflateEnd(&s);
    r = convert_errno_from_zlib(zr);
    if (r == UPX_E_OK)
        r = UPX_E_ERROR;
done:
    if (r == UPX_E_OK) {
        if (s.avail_in != 0 || s.total_in != src_len)
            r = UPX_E_INPUT_NOT_CONSUMED;
    }
    assert(s.total_in <= src_len);
    assert(s.total_out <= *dst_len);
    *dst_len = s.total_out;
    return r;
}

/*************************************************************************
// test_overlap - see <ucl/ucl.h> for semantics
**************************************************************************/

int upx_zlib_test_overlap(const upx_bytep buf, const upx_bytep tbuf, unsigned src_off,
                          unsigned src_len, unsigned *dst_len, int method,
                          const upx_compress_result_t *cresult) {
    assert(method == M_DEFLATE);

    MemBuffer b(src_off + src_len);
    memcpy(b + src_off, buf + src_off, src_len);
    unsigned saved_dst_len = *dst_len;
    int r = upx_zlib_decompress(raw_index_bytes(b, src_off, src_len), src_len,
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

int upx_zlib_init(void) {
    if (strcmp(ZLIB_VERSION, zlibVersion()) != 0)
        return -2;
    return 0;
}

const char *upx_zlib_version_string(void) { return zlibVersion(); }

#if 0 /* UNUSED */
unsigned upx_zlib_adler32(const void *buf, unsigned len, unsigned adler) {
    return adler32(adler, (const Bytef *) buf, len);
}
#endif

#if 0 /* UNUSED */
unsigned upx_zlib_crc32(const void *buf, unsigned len, unsigned crc) {
    return crc32(crc, (const Bytef *) buf, len);
}
#endif

/*************************************************************************
// doctest checks
**************************************************************************/

#if DEBUG && !defined(DOCTEST_CONFIG_DISABLE) && 1

#include "util/membuffer.h"

static bool check_zlib(const int method, const int level, const unsigned expected_c_len) {
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
    r = upx_zlib_compress(raw_bytes(u_buf, u_len), u_len, raw_index_bytes(c_buf, c_extra, c_len),
                          &c_len, nullptr, method, level, NULL_cconf, &cresult);
    if (r != 0 || c_len != expected_c_len)
        return false;

    d_len = d_buf.getSize();
    r = upx_zlib_decompress(raw_index_bytes(c_buf, c_extra, c_len), c_len, raw_bytes(d_buf, d_len),
                            &d_len, method, nullptr);
    if (r != 0 || d_len != u_len || memcmp(u_buf, d_buf, u_len) != 0)
        return false;

    d_len = u_len - 1;
    r = upx_zlib_decompress(raw_index_bytes(c_buf, c_extra, c_len), c_len, raw_bytes(d_buf, d_len),
                            &d_len, method, nullptr);
    if (r == 0)
        return false;

    // TODO: rewrite Packer::findOverlapOverhead() so that we can test it here
    // unsigned x_len = d_len;
    // r = upx_zlib_test_overlap(c_buf, u_buf, c_extra, c_len, &x_len, method, nullptr);
    return true;
}

TEST_CASE("compress_zlib") {
    CHECK(check_zlib(M_DEFLATE, 1, 89));
    CHECK(check_zlib(M_DEFLATE, 3, 89));
    CHECK(check_zlib(M_DEFLATE, 5, 33));
}

#endif // DEBUG

TEST_CASE("upx_zlib_decompress") {
    typedef const upx_byte C;
    C *c_data;
    upx_byte d_buf[16];
    unsigned d_len;
    int r;

    c_data = (C *) "\xfb\xff\x1f\x15\x00\x00";
    d_len = 16;
    r = upx_zlib_decompress(c_data, 6, d_buf, &d_len, M_DEFLATE, nullptr);
    CHECK((r == 0 && d_len == 16));
    r = upx_zlib_decompress(c_data, 5, d_buf, &d_len, M_DEFLATE, nullptr);
    CHECK(r == UPX_E_INPUT_OVERRUN);
    d_len = 15;
    r = upx_zlib_decompress(c_data, 6, d_buf, &d_len, M_DEFLATE, nullptr);
    CHECK(r == UPX_E_OUTPUT_OVERRUN);
}

/* vim:set ts=4 sw=4 et: */
