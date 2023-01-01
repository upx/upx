/* compress_ucl.cpp --

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

/*************************************************************************
//
**************************************************************************/

static int convert_errno_from_ucl(int r) {
    switch (r) {
    case UCL_E_OK:
        return UPX_E_OK;
    case UCL_E_ERROR:
        return UPX_E_ERROR;
    case UCL_E_OUT_OF_MEMORY:
        return UPX_E_OUT_OF_MEMORY;
    case UCL_E_NOT_COMPRESSIBLE:
        return UPX_E_NOT_COMPRESSIBLE;
    case UCL_E_INPUT_OVERRUN:
        return UPX_E_INPUT_OVERRUN;
    case UCL_E_OUTPUT_OVERRUN:
        return UPX_E_OUTPUT_OVERRUN;
    case UCL_E_LOOKBEHIND_OVERRUN:
        return UPX_E_LOOKBEHIND_OVERRUN;
    case UCL_E_EOF_NOT_FOUND:
        return UPX_E_EOF_NOT_FOUND;
    case UCL_E_INPUT_NOT_CONSUMED:
        return UPX_E_INPUT_NOT_CONSUMED;
    case UCL_E_INVALID_ARGUMENT:
        return UPX_E_INVALID_ARGUMENT;
    // UCL extra:
    case UCL_E_OVERLAP_OVERRUN:
        return UPX_E_ERROR;
    }
    return UPX_E_ERROR;
}

extern "C" {
static void __UCL_CDECL wrap_nprogress_ucl(ucl_uint a, ucl_uint b, int state, ucl_voidp user) {
    if (state != -1 && state != 3)
        return;
    upx_callback_p cb = (upx_callback_p) user;
    if (cb && cb->nprogress)
        cb->nprogress(cb, a, b);
}
}

/*************************************************************************
//
**************************************************************************/

int upx_ucl_compress(const upx_bytep src, unsigned src_len, upx_bytep dst, unsigned *dst_len,
                     upx_callback_p cb_parm, int method, int level,
                     const upx_compress_config_t *cconf_parm, upx_compress_result_t *cresult) {
    int r;
    assert(level > 0);
    assert(cresult != nullptr);

    COMPILE_TIME_ASSERT(sizeof(ucl_compress_config_t) == sizeof(REAL_ucl_compress_config_t))

    ucl_progress_callback_t cb;
    cb.callback = nullptr;
    cb.user = nullptr;
    if (cb_parm && cb_parm->nprogress) {
        cb.callback = wrap_nprogress_ucl;
        cb.user = cb_parm;
    }

    ucl_compress_config_t cconf;
    cconf.reset();
    if (cconf_parm) {
        // cconf = cconf_parm->conf_ucl; // struct copy
        memcpy(&cconf, &cconf_parm->conf_ucl, sizeof(cconf));
    }

    ucl_uint *const res = cresult->result_ucl.result;
    // assume no info available - fill in worst case results
    // res[0] = 1;        // min_offset_found - NOT USED
    res[1] = src_len - 1; // max_offset_found
    // res[2] = 2;        // min_match_found - NOT USED
    res[3] = src_len - 1; // max_match_found
    // res[4] = 1;        // min_run_found - NOT USED
    res[5] = src_len; // max_run_found
    res[6] = 1;       // first_offset_found
    // res[7] = 999999;   // same_match_offsets_found - NOT USED

    // prepare bit-buffer settings
    cconf.bb_endian = 0;
    cconf.bb_size = 0;
    if (method >= M_NRV2B_LE32 && method <= M_NRV2E_LE16) {
        static const unsigned char sizes[3] = {32, 8, 16};
        cconf.bb_size = sizes[(method - M_NRV2B_LE32) % 3];
    } else {
        throwInternalError("unknown compression method");
        return UPX_E_ERROR;
    }

    // optimize compression parms
    if (level <= 3 && cconf.max_offset == UCL_UINT_MAX)
        cconf.max_offset = 8 * 1024 - 1;
    else if (level == 4 && cconf.max_offset == UCL_UINT_MAX)
        cconf.max_offset = 32 * 1024 - 1;

    if M_IS_NRV2B (method)
        r = ucl_nrv2b_99_compress(src, src_len, dst, dst_len, &cb, level, &cconf, res);
    else if M_IS_NRV2D (method)
        r = ucl_nrv2d_99_compress(src, src_len, dst, dst_len, &cb, level, &cconf, res);
    else if M_IS_NRV2E (method)
        r = ucl_nrv2e_99_compress(src, src_len, dst, dst_len, &cb, level, &cconf, res);
    else {
        throwInternalError("unknown compression method");
        return UPX_E_ERROR;
    }

    // make sure first_offset_found is set
    if (res[6] == 0)
        res[6] = 1;

    return convert_errno_from_ucl(r);
}

/*************************************************************************
//
**************************************************************************/

int upx_ucl_decompress(const upx_bytep src, unsigned src_len, upx_bytep dst, unsigned *dst_len,
                       int method, const upx_compress_result_t *cresult) {
    int r;

    switch (method) {
    case M_NRV2B_8:
        r = ucl_nrv2b_decompress_safe_8(src, src_len, dst, dst_len, nullptr);
        break;
    case M_NRV2B_LE16:
        r = ucl_nrv2b_decompress_safe_le16(src, src_len, dst, dst_len, nullptr);
        break;
    case M_NRV2B_LE32:
        r = ucl_nrv2b_decompress_safe_le32(src, src_len, dst, dst_len, nullptr);
        break;
    case M_NRV2D_8:
        r = ucl_nrv2d_decompress_safe_8(src, src_len, dst, dst_len, nullptr);
        break;
    case M_NRV2D_LE16:
        r = ucl_nrv2d_decompress_safe_le16(src, src_len, dst, dst_len, nullptr);
        break;
    case M_NRV2D_LE32:
        r = ucl_nrv2d_decompress_safe_le32(src, src_len, dst, dst_len, nullptr);
        break;
    case M_NRV2E_8:
        r = ucl_nrv2e_decompress_safe_8(src, src_len, dst, dst_len, nullptr);
        break;
    case M_NRV2E_LE16:
        r = ucl_nrv2e_decompress_safe_le16(src, src_len, dst, dst_len, nullptr);
        break;
    case M_NRV2E_LE32:
        r = ucl_nrv2e_decompress_safe_le32(src, src_len, dst, dst_len, nullptr);
        break;
    default:
        throwInternalError("unknown decompression method");
        return UPX_E_ERROR;
    }

    UNUSED(cresult);
    return convert_errno_from_ucl(r);
}

/*************************************************************************
//
**************************************************************************/

int upx_ucl_test_overlap(const upx_bytep buf, const upx_bytep tbuf, unsigned src_off,
                         unsigned src_len, unsigned *dst_len, int method,
                         const upx_compress_result_t *cresult) {
    int r;
    UNUSED(tbuf); // not needed for UCL

    switch (method) {
    case M_NRV2B_8:
        r = ucl_nrv2b_test_overlap_8(buf, src_off, src_len, dst_len, nullptr);
        break;
    case M_NRV2B_LE16:
        r = ucl_nrv2b_test_overlap_le16(buf, src_off, src_len, dst_len, nullptr);
        break;
    case M_NRV2B_LE32:
        r = ucl_nrv2b_test_overlap_le32(buf, src_off, src_len, dst_len, nullptr);
        break;
    case M_NRV2D_8:
        r = ucl_nrv2d_test_overlap_8(buf, src_off, src_len, dst_len, nullptr);
        break;
    case M_NRV2D_LE16:
        r = ucl_nrv2d_test_overlap_le16(buf, src_off, src_len, dst_len, nullptr);
        break;
    case M_NRV2D_LE32:
        r = ucl_nrv2d_test_overlap_le32(buf, src_off, src_len, dst_len, nullptr);
        break;
    case M_NRV2E_8:
        r = ucl_nrv2e_test_overlap_8(buf, src_off, src_len, dst_len, nullptr);
        break;
    case M_NRV2E_LE16:
        r = ucl_nrv2e_test_overlap_le16(buf, src_off, src_len, dst_len, nullptr);
        break;
    case M_NRV2E_LE32:
        r = ucl_nrv2e_test_overlap_le32(buf, src_off, src_len, dst_len, nullptr);
        break;
    default:
        throwInternalError("unknown decompression method");
        return UPX_E_ERROR;
    }

    UNUSED(cresult);
    return convert_errno_from_ucl(r);
}

/*************************************************************************
// misc
**************************************************************************/

extern "C" {
static ucl_voidp __UCL_CDECL my_malloc(ucl_uint n) { return calloc(1, n); }
static void __UCL_CDECL my_free(ucl_voidp p) { free(p); }
}

int upx_ucl_init(void) {
    if (ucl_init() != UCL_E_OK)
        return -1;
    if (UCL_VERSION != ucl_version() || strcmp(UCL_VERSION_STRING, ucl_version_string()) != 0)
        return -2;
    ucl_set_malloc_hooks(my_malloc, my_free);
    return 0;
}

const char *upx_ucl_version_string(void) { return ucl_version_string(); }

unsigned upx_ucl_adler32(const void *buf, unsigned len, unsigned adler) {
    return ucl_adler32(adler, (const ucl_bytep) buf, len);
}

#if 0 /* UNUSED */
unsigned upx_ucl_crc32(const void *buf, unsigned len, unsigned crc) {
    return ucl_crc32(crc, (const ucl_bytep) buf, len);
}
#endif

/*************************************************************************
// doctest checks
**************************************************************************/

#if DEBUG && !defined(DOCTEST_CONFIG_DISABLE) && 1

#include "util/membuffer.h"

static bool check_ucl(const int method, const unsigned expected_c_len) {
    const unsigned u_len = 16384;
    const unsigned c_extra = 4096;
    MemBuffer u_buf, c_buf, d_buf;
    unsigned c_len, d_len;
    upx_compress_result_t cresult;
    int r;
    const int level = 3; // don't waste time

    u_buf.alloc(u_len);
    memset(u_buf, 0, u_len);
    c_buf.allocForCompression(u_len, c_extra);
    d_buf.allocForDecompression(u_len);

    c_len = c_buf.getSize() - c_extra;
    r = upx_ucl_compress(raw_bytes(u_buf, u_len), u_len, raw_index_bytes(c_buf, c_extra, c_len),
                         &c_len, nullptr, method, level, NULL_cconf, &cresult);
    if (r != 0 || c_len != expected_c_len)
        return false;

    d_len = d_buf.getSize();
    r = upx_ucl_decompress(raw_index_bytes(c_buf, c_extra, c_len), c_len, raw_bytes(d_buf, d_len),
                           &d_len, method, nullptr);
    if (r != 0 || d_len != u_len || memcmp(u_buf, d_buf, u_len) != 0)
        return false;

    d_len = u_len - 1;
    r = upx_ucl_decompress(raw_index_bytes(c_buf, c_extra, c_len), c_len, raw_bytes(d_buf, d_len),
                           &d_len, method, nullptr);
    if (r == 0)
        return false;

    // TODO: rewrite Packer::findOverlapOverhead() so that we can test it here
    // unsigned x_len = d_len;
    // r = upx_ucl_test_overlap(c_buf, u_buf, c_extra, c_len, &x_len, method, nullptr);
    return true;
}

TEST_CASE("compress_ucl") {
    CHECK(check_ucl(M_NRV2B_8, 34));
    CHECK(check_ucl(M_NRV2B_LE16, 34));
    CHECK(check_ucl(M_NRV2B_LE32, 34));
    CHECK(check_ucl(M_NRV2D_8, 32));
    CHECK(check_ucl(M_NRV2D_LE16, 32));
    CHECK(check_ucl(M_NRV2D_LE32, 34));
    CHECK(check_ucl(M_NRV2E_8, 32));
    CHECK(check_ucl(M_NRV2E_LE16, 32));
    CHECK(check_ucl(M_NRV2E_LE32, 34));
}

#endif // DEBUG

TEST_CASE("upx_ucl_decompress") {
    typedef const upx_byte C;
    C *c_data;
    upx_byte d_buf[16];
    unsigned d_len;
    int r;

    c_data = (C *) "\x92\xff\x10\x00\x00\x00\x00\x00\x48\xff";
    d_len = 16;
    r = upx_ucl_decompress(c_data, 10, d_buf, &d_len, M_NRV2B_8, nullptr);
    CHECK((r == 0 && d_len == 16));
    r = upx_ucl_decompress(c_data, 9, d_buf, &d_len, M_NRV2B_8, nullptr);
    CHECK(r == UPX_E_INPUT_OVERRUN);
    d_len = 15;
    r = upx_ucl_decompress(c_data, 10, d_buf, &d_len, M_NRV2B_8, nullptr);
    CHECK(r == UPX_E_OUTPUT_OVERRUN);

    c_data = (C *) "\x92\xff\x10\x92\x49\x24\x92\xa0\xff";
    d_len = 16;
    r = upx_ucl_decompress(c_data, 9, d_buf, &d_len, M_NRV2D_8, nullptr);
    CHECK((r == 0 && d_len == 16));
    r = upx_ucl_decompress(c_data, 8, d_buf, &d_len, M_NRV2D_8, nullptr);
    CHECK(r == UPX_E_INPUT_OVERRUN);
    d_len = 15;
    r = upx_ucl_decompress(c_data, 9, d_buf, &d_len, M_NRV2D_8, nullptr);
    CHECK(r == UPX_E_OUTPUT_OVERRUN);

    c_data = (C *) "\x90\xff\xb0\x92\x49\x24\x92\xa0\xff";
    d_len = 16;
    r = upx_ucl_decompress(c_data, 9, d_buf, &d_len, M_NRV2E_8, nullptr);
    CHECK((r == 0 && d_len == 16));
    r = upx_ucl_decompress(c_data, 8, d_buf, &d_len, M_NRV2E_8, nullptr);
    CHECK(r == UPX_E_INPUT_OVERRUN);
    d_len = 15;
    r = upx_ucl_decompress(c_data, 9, d_buf, &d_len, M_NRV2E_8, nullptr);
    CHECK(r == UPX_E_OUTPUT_OVERRUN);
}

/* vim:set ts=4 sw=4 et: */
