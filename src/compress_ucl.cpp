/* compress_ucl.cpp --

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
#if !defined(WITH_UCL)
extern int compress_ucl_dummy;
int compress_ucl_dummy = 0;
#else

#if 1 && !defined(UCL_USE_ASM) && (ACC_ARCH_I386)
#  if (ACC_CC_GNUC || ACC_CC_INTELC || ACC_CC_MSC || ACC_CC_WATCOMC)
#    define UCL_USE_ASM
#  endif
#endif
#if defined(UCL_NO_ASM)
#  undef UCL_USE_ASM
#endif
#if defined(ACC_CFG_NO_UNALIGNED)
#  undef UCL_USE_ASM
#endif
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

static void wrap_ucl_nprogress(ucl_uint a, ucl_uint b, int state, ucl_voidp user)
{
    if (state != -1 && state != 3) return;
    upx_callback_p cb = (upx_callback_p) user;
    if (cb && cb->nprogress)
        cb->nprogress(cb, a, b, state);
}


int upx_ucl_compress       ( const upx_bytep src, upx_uint  src_len,
                                   upx_bytep dst, upx_uintp dst_len,
                                   upx_callback_p cb_parm,
                                   int method, int level,
                             const struct upx_compress_config_t *conf_parm,
                                   struct upx_compress_result_t *result )
{
    int r = UPX_E_ERROR;
    assert(level > 0); assert(result != NULL);

    ucl_progress_callback_t cb;
    cb.callback = 0;
    cb.user = NULL;
    if (cb_parm && cb_parm->nprogress) {
        cb.callback = wrap_ucl_nprogress;
        cb.user = cb_parm;
    }

    ucl_compress_config_t conf;
    memset(&conf, 0xff, sizeof(conf));
    if (conf_parm)
        conf = conf_parm->conf_ucl; // struct copy

    ucl_uint *res = result->result_ucl.result;

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
                                  &cb, level, &conf, res);
    else if M_IS_NRV2D(method)
        r = ucl_nrv2d_99_compress(src, src_len, dst, dst_len,
                                  &cb, level, &conf, res);
    else if M_IS_NRV2E(method)
        r = ucl_nrv2e_99_compress(src, src_len, dst, dst_len,
                                  &cb, level, &conf, res);
    else
        throwInternalError("unknown compression method");

    return r;
}


/*************************************************************************
//
**************************************************************************/

int upx_ucl_decompress     ( const upx_bytep src, upx_uint  src_len,
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
    case M_NRV2E_8:
        r = ucl_nrv2e_decompress_safe_8(src,src_len,dst,dst_len,NULL);
        break;
    case M_NRV2E_LE16:
        r = ucl_nrv2e_decompress_safe_le16(src,src_len,dst,dst_len,NULL);
        break;
    case M_NRV2E_LE32:
        r = ucl_nrv2e_decompress_safe_le32(src,src_len,dst,dst_len,NULL);
        break;
    default:
        throwInternalError("unknown decompression method");
        break;
    }

    return r;
}


/*************************************************************************
//
**************************************************************************/

int upx_ucl_test_overlap   ( const upx_bytep buf, upx_uint src_off,
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
    case M_NRV2E_8:
        r = ucl_nrv2e_test_overlap_8(buf,src_off,src_len,dst_len,NULL);
        break;
    case M_NRV2E_LE16:
        r = ucl_nrv2e_test_overlap_le16(buf,src_off,src_len,dst_len,NULL);
        break;
    case M_NRV2E_LE32:
        r = ucl_nrv2e_test_overlap_le32(buf,src_off,src_len,dst_len,NULL);
        break;
    default:
        throwInternalError("unknown decompression method");
        break;
    }

    return r;
}


#endif /* WITH_UCL */
/*
vi:ts=4:et:nowrap
*/

