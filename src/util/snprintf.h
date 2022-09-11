/* snprintf.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2022 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2022 Laszlo Molnar
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

#pragma once
#ifndef UPX_SNPRINTF_H__
#define UPX_SNPRINTF_H__ 1

/*************************************************************************
// UPX version of string functions, with assertions and sane limits
**************************************************************************/

// info: snprintf() returns length and NOT size, but max_size is indeed size (incl NUL)

int upx_safe_vsnprintf(char *str, upx_rsize_t max_size, const char *format, va_list ap);
int upx_safe_snprintf(char *str, upx_rsize_t max_size, const char *format, ...)
    attribute_format(3, 4);

// malloc's *ptr
int upx_safe_vasprintf(char **ptr, const char *format, va_list ap);
int upx_safe_asprintf(char **ptr, const char *format, ...) attribute_format(2, 3);

// returns a malloc'd pointer
char *upx_safe_xprintf(const char *format, ...) attribute_format(1, 2);

upx_rsize_t upx_safe_strlen(const char *);

// globally redirect some functions
#undef snprintf
#undef sprintf
#undef vsnprintf
#define snprintf upx_safe_snprintf
#define sprintf ERROR_sprintf_IS_DANGEROUS_USE_snprintf
#define vsnprintf upx_safe_vsnprintf

#undef strlen
#define strlen upx_safe_strlen

/*************************************************************************
// some unsigned char string support functions
**************************************************************************/

inline unsigned char *strcpy(unsigned char *s1, const unsigned char *s2) {
    return (unsigned char *) strcpy((char *) s1, (const char *) s2);
}

inline int strcmp(const unsigned char *s1, const unsigned char *s2) {
    return strcmp((const char *) s1, (const char *) s2);
}

inline int strcasecmp(const unsigned char *s1, const unsigned char *s2) {
    return strcasecmp((const char *) s1, (const char *) s2);
}

inline upx_rsize_t upx_safe_strlen(const unsigned char *s) {
    return upx_safe_strlen((const char *) s);
}

#endif /* already included */

/* vim:set ts=4 sw=4 et: */
