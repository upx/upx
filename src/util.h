/* util.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2000 Laszlo Molnar
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

   Markus F.X.J. Oberhumer                   Laszlo Molnar
   markus.oberhumer@jk.uni-linz.ac.at        ml1050@cdata.tvnet.hu
 */


#ifndef __UPX_UTIL_H
#define __UPX_UTIL_H


/*************************************************************************
// misc. support functions
**************************************************************************/

int upx_snprintf(char *str, long n, const char *format, ...);
int upx_vsnprintf(char *str, long n, const char *format, va_list ap);

char *fn_basename(const char *name);
int fn_strcmp(const char *n1, const char *n2);
char *fn_strlwr(char *n);
bool fn_has_ext(const char *name, const char *ext);

bool file_exists(const char *name);
bool maketempname(char *ofilename, const char *ifilename,
                  const char *ext, bool force=true);
bool makebakname(char *ofilename, const char *ifilename, bool force=true);
bool isafile(int fd);

unsigned get_ratio(unsigned long packedsize, unsigned long size,
                   unsigned long scale=100);
char *center_string(const char *name, size_t s);


upx_bytep find(const void * b, int blen, const void * what, int wlen);
upx_bytep find_be16(const void * b, int blen, unsigned what);
upx_bytep find_be32(const void * b, int blen, unsigned what);
upx_bytep find_le16(const void * b, int blen, unsigned what);
upx_bytep find_le32(const void * b, int blen, unsigned what);


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

