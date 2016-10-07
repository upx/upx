/* util.h --

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

#ifndef __UPX_UTIL_H
#define __UPX_UTIL_H 1

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

/*************************************************************************
// protect against integer overflows and malicious header fields
**************************************************************************/

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

#define New(type, n) new type[mem_size_get_n(sizeof(type), n)]

int ptr_diff(const char *p1, const char *p2);

inline int ptr_diff(const unsigned char *p1, const unsigned char *p2) {
    return ptr_diff((const char *) p1, (const char *) p2);
}

inline int ptr_diff(const void *p1, const void *p2) {
    return ptr_diff((const char *) p1, (const char *) p2);
}

template <class T1, class T2>
inline int ptr_udiff(const T1 &p1, const T2 &p2) {
    int d = ptr_diff(p1, p2);
    assert(d >= 0);
    return d;
}

/*************************************************************************
// some unsigned char string support functions
**************************************************************************/

inline char *strcpy(unsigned char *s1, const unsigned char *s2) {
    return strcpy((char *) s1, (const char *) s2);
}

inline int strcasecmp(const unsigned char *s1, const unsigned char *s2) {
    return strcasecmp((const char *) s1, (const char *) s2);
}

inline size_t strlen(const unsigned char *s) { return strlen((const char *) s); }

#endif /* already included */

/* vim:set ts=4 sw=4 et: */
