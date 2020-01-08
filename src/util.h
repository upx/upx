/* util.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2020 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2020 Laszlo Molnar
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

#ifndef __UPX_UTIL_H
#define __UPX_UTIL_H 1

/*************************************************************************
// protect against integer overflows and malicious header fields
**************************************************************************/

#define New(type, n) new type[mem_size_get_n(sizeof(type), n)]

upx_rsize_t mem_size(upx_uint64_t element_size, upx_uint64_t n, upx_uint64_t extra1 = 0,
                     upx_uint64_t extra2 = 0);
upx_rsize_t mem_size_get_n(upx_uint64_t element_size, upx_uint64_t n);

inline void mem_size_assert(upx_uint64_t element_size, upx_uint64_t n, upx_uint64_t extra1 = 0,
                            upx_uint64_t extra2 = 0) {
    (void) mem_size(element_size, n, extra1, extra2); // sanity check
}

bool mem_size_valid(upx_uint64_t element_size, upx_uint64_t n, upx_uint64_t extra1 = 0,
                    upx_uint64_t extra2 = 0);
bool mem_size_valid_bytes(upx_uint64_t bytes);

int ptr_diff(const void *p1, const void *p2);
unsigned ptr_udiff(const void *p1, const void *p2); // asserts p1 >= p2

void mem_clear(void *p, size_t n);

/*************************************************************************
// misc. support functions
**************************************************************************/

char *fn_basename(const char *name);
int fn_strcmp(const char *n1, const char *n2);
char *fn_strlwr(char *n);
bool fn_has_ext(const char *name, const char *ext, bool ignore_case = true);

bool file_exists(const char *name);
bool maketempname(char *ofilename, size_t size, const char *ifilename, const char *ext,
                  bool force = true);
bool makebakname(char *ofilename, size_t size, const char *ifilename, bool force = true);

unsigned get_ratio(upx_uint64_t u_len, upx_uint64_t c_len);
bool set_method_name(char *buf, size_t size, int method, int level);
void center_string(char *buf, size_t size, const char *s);

int find(const void *b, int blen, const void *what, int wlen);
int find_be16(const void *b, int blen, unsigned what);
int find_be32(const void *b, int blen, unsigned what);
int find_be64(const void *b, int blen, upx_uint64_t what);
int find_le16(const void *b, int blen, unsigned what);
int find_le32(const void *b, int blen, unsigned what);
int find_le64(const void *b, int blen, upx_uint64_t what);

int mem_replace(void *b, int blen, const void *what, int wlen, const void *r);

#endif /* already included */

/* vim:set ts=4 sw=4 et: */
