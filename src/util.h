/* util.h --

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


#ifndef __UPX_UTIL_H
#define __UPX_UTIL_H


/*************************************************************************
// misc. support functions
**************************************************************************/

char *fn_basename(const char *name);
int fn_strcmp(const char *n1, const char *n2);
char *fn_strlwr(char *n);
bool fn_has_ext(const char *name, const char *ext, bool ignore_case=true);

bool file_exists(const char *name);
bool maketempname(char *ofilename, size_t size,
                  const char *ifilename, const char *ext, bool force=true);
bool makebakname(char *ofilename, size_t size,
                 const char *ifilename, bool force=true);

unsigned get_ratio(unsigned u_len, unsigned c_len);
bool set_method_name(char *buf, size_t size, int method, int level);
void center_string(char *buf, size_t size, const char *s);


int find(const void *b, int blen, const void *what, int wlen);
int find_be16(const void *b, int blen, unsigned what);
int find_be32(const void *b, int blen, unsigned what);
int find_be64(const void *b, int blen, acc_uint64l_t what);
int find_le16(const void *b, int blen, unsigned what);
int find_le32(const void *b, int blen, unsigned what);
int find_le64(const void *b, int blen, acc_uint64l_t what);

int mem_replace(void *b, int blen, const void *what, int wlen, const void *r);


#if (ACC_CC_BORLANDC && (__BORLANDC__ < 0x0530))
#elif (ACC_CC_DMC && (__DMC__ < 0x830))
#elif (ACC_CC_MSC && (_MSC_VER < 1310))
#else
template <class T>
inline int ptr_diff(const T *p1, const T *p2)
{
    COMPILE_TIME_ASSERT(sizeof(T) == 1)
    assert(p1 != NULL); assert(p2 != NULL);
    ptrdiff_t d = (const char*) p1 - (const char*) p2;
    assert((int)d == d);
    return (int) d;
}
#endif
inline int ptr_diff(const void *p1, const void *p2)
{
    assert(p1 != NULL); assert(p2 != NULL);
    ptrdiff_t d = (const char*) p1 - (const char*) p2;
    assert((int)d == d);
    return (int) d;
}


/*************************************************************************
// some unsigned char string support functions
**************************************************************************/

inline char *strcpy(unsigned char *s1,const unsigned char *s2)
{
    return strcpy((char*) s1,(const char*) s2);
}

inline int strcasecmp(const unsigned char *s1,const unsigned char *s2)
{
    return strcasecmp((const char*) s1,(const char*) s2);
}

inline size_t strlen(const unsigned char *s)
{
    return strlen((const char*) s);
}


/*************************************************************************
//
**************************************************************************/

#if 0
bool upx_isdigit(int c);
bool upx_islower(int c);
bool upx_isspace(int c);
int upx_tolower(int c);
#undef isdigit
#undef islower
#undef isspace
#undef tolower
#define isdigit     upx_isdigit
#define islower     upx_islower
#define isspace     upx_isspace
#define tolower     upx_tolower
#endif


#endif /* already included */


/*
vi:ts=4:et
*/

