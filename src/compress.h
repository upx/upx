/* compress.h --

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


#ifndef __UPX_COMPRESS_H
#define __UPX_COMPRESS_H 1


/*************************************************************************
//
**************************************************************************/

#if (WITH_LZMA)
int upx_lzma_init(void);
const char *upx_lzma_version_string(void);
int upx_lzma_compress      ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   upx_callback_p cb,
                                   int method, int level,
                             const upx_compress_config_t *cconf,
                                   upx_compress_result_t *cresult );
int upx_lzma_decompress    ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   int method,
                             const upx_compress_result_t *cresult );
int upx_lzma_test_overlap  ( const upx_bytep buf,
                             const upx_bytep tbuf,
                                   unsigned  src_off, unsigned src_len,
                                   unsigned* dst_len,
                                   int method,
                             const upx_compress_result_t *cresult );
#endif


#if (WITH_NRV)
int upx_nrv_init(void);
const char *upx_nrv_version_string(void);
int upx_nrv_compress       ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   upx_callback_p cb,
                                   int method, int level,
                             const upx_compress_config_t *cconf,
                                   upx_compress_result_t *cresult );
int upx_nrv_decompress     ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   int method,
                             const upx_compress_result_t *cresult );
int upx_nrv_test_overlap   ( const upx_bytep buf,
                             const upx_bytep tbuf,
                                   unsigned  src_off, unsigned src_len,
                                   unsigned* dst_len,
                                   int method,
                             const upx_compress_result_t *cresult );
#endif


#if (WITH_UCL)
int upx_ucl_init(void);
const char *upx_ucl_version_string(void);
int upx_ucl_compress       ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   upx_callback_p cb,
                                   int method, int level,
                             const upx_compress_config_t *cconf,
                                   upx_compress_result_t *cresult );
int upx_ucl_decompress     ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   int method,
                             const upx_compress_result_t *cresult );
int upx_ucl_test_overlap   ( const upx_bytep buf,
                             const upx_bytep tbuf,
                                   unsigned  src_off, unsigned src_len,
                                   unsigned* dst_len,
                                   int method,
                             const upx_compress_result_t *cresult );
unsigned upx_ucl_adler32(const void *buf, unsigned len, unsigned adler);
unsigned upx_ucl_crc32  (const void *buf, unsigned len, unsigned crc);
#endif


#if (WITH_ZLIB)
int upx_zlib_init(void);
const char *upx_zlib_version_string(void);
int upx_zlib_compress      ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   upx_callback_p cb,
                                   int method, int level,
                             const upx_compress_config_t *cconf,
                                   upx_compress_result_t *cresult );
int upx_zlib_decompress    ( const upx_bytep src, unsigned  src_len,
                                   upx_bytep dst, unsigned* dst_len,
                                   int method,
                             const upx_compress_result_t *cresult );
int upx_zlib_test_overlap  ( const upx_bytep buf,
                             const upx_bytep tbuf,
                                   unsigned  src_off, unsigned src_len,
                                   unsigned* dst_len,
                                   int method,
                             const upx_compress_result_t *cresult );
unsigned upx_zlib_adler32(const void *buf, unsigned len, unsigned adler);
unsigned upx_zlib_crc32  (const void *buf, unsigned len, unsigned crc);
#endif


#endif /* already included */

/* vim:set ts=4 sw=4 et: */
