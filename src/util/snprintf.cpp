/* snprintf.cpp -- string wrapper

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

#include "../conf.h"

/*************************************************************************
// UPX version of string functions, with assertions and sane limits
**************************************************************************/

int upx_safe_vsnprintf(char *str, upx_rsize_t max_size, const char *format, va_list ap) {
#undef vsnprintf
    size_t size;

    // preconditions
    assert(max_size <= UPX_RSIZE_MAX_STR);
    if (str != nullptr)
        assert(max_size > 0);
    else
        assert(max_size == 0);

    long long len = vsnprintf(str, max_size, format, ap);
    assert(len >= 0);
    assert(len < UPX_RSIZE_MAX_STR);
    size = (size_t) len + 1;

    // postconditions
    assert(size > 0);
    assert(size <= UPX_RSIZE_MAX_STR);
    if (str != nullptr) {
        assert(size <= max_size);
        assert(str[size - 1] == '\0');
    }

    return ACC_ICONV(int, size - 1); // snprintf() returns length, not size
#define vsnprintf upx_safe_vsnprintf
}

int upx_safe_snprintf(char *str, upx_rsize_t max_size, const char *format, ...) {
    va_list ap;
    int len;

    va_start(ap, format);
    len = upx_safe_vsnprintf(str, max_size, format, ap);
    va_end(ap);
    return len;
}

int upx_safe_vasprintf(char **ptr, const char *format, va_list ap) {
    int len;

    assert(ptr != nullptr);
    *ptr = nullptr;

    va_list ap_copy;
    va_copy(ap_copy, ap);
    len = upx_safe_vsnprintf(nullptr, 0, format, ap_copy);
    va_end(ap_copy);

    if (len >= 0) {
        *ptr = (char *) malloc(len + 1);
        assert(*ptr != nullptr);
        if (*ptr == nullptr)
            return -1;
        int len2 = upx_safe_vsnprintf(*ptr, len + 1, format, ap);
        assert(len2 == len);
    }
    return len;
}

int upx_safe_asprintf(char **ptr, const char *format, ...) {
    va_list ap;
    int len;

    va_start(ap, format);
    len = upx_safe_vasprintf(ptr, format, ap);
    va_end(ap);
    return len;
}

char *upx_safe_xprintf(const char *format, ...) {
    char *ptr = nullptr;
    va_list ap;
    int len;

    va_start(ap, format);
    len = upx_safe_vasprintf(&ptr, format, ap);
    va_end(ap);
    UNUSED(len);
    assert(ptr != nullptr);
    return ptr;
}

upx_rsize_t upx_safe_strlen(const char *s) {
#undef strlen
    assert(s != nullptr);
    size_t len = strlen(s);
    assert(len < UPX_RSIZE_MAX_STR);
    return len;
#define strlen upx_safe_strlen
}

/* vim:set ts=4 sw=4 et: */
