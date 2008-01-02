/* compress_zlib.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
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
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
 */


#include "conf.h"
#include "compress.h"
#include "mem.h"


void zlib_compress_config_t::reset()
{
    memset(this, 0, sizeof(*this));

    mem_level.reset();
    window_bits.reset();
    strategy.reset();
}


#if !defined(WITH_ZLIB)
extern int compress_zlib_dummy;
int compress_zlib_dummy = 0;
#else

#include <zlib.h>


static int convert_errno_from_zlib(int zr)
{
    switch (zr)
    {
    case Z_OK:                  return UPX_E_OK;
    case Z_DATA_ERROR:          return UPX_E_ERROR;
    case Z_NEED_DICT:           return UPX_E_ERROR;
    }
    return UPX_E_ERROR;
}


/*************************************************************************
//
**************************************************************************/

int upx_zlib_compress      ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   upx_callback_p cb_parm,
                                   int method, int level,
                             const upx_compress_config_t *cconf_parm,
                                   upx_compress_result_t *cresult )
{
    assert(method == M_DEFLATE);
    assert(level > 0); assert(cresult != NULL);
    UNUSED(cb_parm);
    int r = UPX_E_ERROR;
    int zr;
    const zlib_compress_config_t *lcconf = cconf_parm ? &cconf_parm->conf_zlib : NULL;
    zlib_compress_result_t *res = &cresult->result_zlib;

    if (level == 10)
        level = 9;

    zlib_compress_config_t::mem_level_t mem_level;
    zlib_compress_config_t::window_bits_t window_bits;
    zlib_compress_config_t::strategy_t strategy;
    // cconf overrides
    if (lcconf)
    {
        oassign(mem_level, lcconf->mem_level);
        oassign(window_bits, lcconf->window_bits);
        oassign(strategy, lcconf->strategy);
    }

    res->dummy = 0;

    z_stream s;
    s.zalloc = (alloc_func) 0;
    s.zfree = (free_func) 0;
    s.next_in = const_cast<upx_bytep>(src); // UNCONST
    s.avail_in = src_len;
    s.next_out = dst;
    s.avail_out = *dst_len;
    s.total_in = s.total_out = 0;

    zr = deflateInit2(&s, level, Z_DEFLATED, 0 - (int)window_bits,
                      mem_level, strategy);
    if (zr != Z_OK)
        goto error;
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
    if (r == UPX_E_OK)
    {
        if (s.avail_in != 0 || s.total_in != src_len)
            r = UPX_E_ERROR;
    }
    assert(s.total_in  <=  src_len);
    assert(s.total_out <= *dst_len);
    *dst_len = s.total_out;
    return r;
}


/*************************************************************************
//
**************************************************************************/

int upx_zlib_decompress    ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   int method,
                             const upx_compress_result_t *cresult )
{
    assert(method == M_DEFLATE);
    UNUSED(method);
    UNUSED(cresult);
    int r = UPX_E_ERROR;
    int zr;

    z_stream s;
    s.zalloc = (alloc_func) 0;
    s.zfree = (free_func) 0;
    s.next_in = const_cast<upx_bytep>(src); // UNCONST
    s.avail_in = src_len;
    s.next_out = dst;
    s.avail_out = *dst_len;
    s.total_in = s.total_out = 0;

    zr = inflateInit2(&s, -15);
    if (zr != Z_OK)
        goto error;
    zr = inflate(&s, Z_FINISH);
    if (zr != Z_STREAM_END)
        goto error;
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
    if (r == UPX_E_OK)
    {
        if (s.avail_in != 0 || s.total_in != src_len)
            r = UPX_E_INPUT_NOT_CONSUMED;
    }
    assert(s.total_in  <=  src_len);
    assert(s.total_out <= *dst_len);
    *dst_len = s.total_out;
    return r;
}


/*************************************************************************
// test_overlap - see <ucl/ucl.h> for semantics
**************************************************************************/

int upx_zlib_test_overlap  ( const upx_bytep buf,
                             const upx_bytep tbuf,
                                   unsigned  src_off, unsigned src_len,
                                   unsigned* dst_len,
                                   int method,
                             const upx_compress_result_t *cresult )
{
    assert(method == M_DEFLATE);

    MemBuffer b(src_off + src_len);
    memcpy(b + src_off, buf + src_off, src_len);
    unsigned saved_dst_len = *dst_len;
    int r = upx_zlib_decompress(b + src_off, src_len, b, dst_len, method, cresult);
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

const char *upx_zlib_version_string(void)
{
    return zlibVersion();
}


#endif /* WITH_ZLIB */
/*
vi:ts=4:et:nowrap
*/

