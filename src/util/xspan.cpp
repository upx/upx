/* xspan -- a minimally invasive checked memory smart pointer

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
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

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
 */

#include "../conf.h"

#if WITH_XSPAN

XSPAN_NAMESPACE_BEGIN

// debugging stats
struct XSpanStats {
    upx_std_atomic(size_t) check_range_counter;
};
static XSpanStats xspan_stats;

// HINT: set env-var "UPX_DEBUG_DOCTEST_DISABLE=1" for improved debugging experience
__acc_noinline void xspan_fail_nullptr() {
    throwCantUnpack("xspan unexpected NULL pointer; take care!");
}
__acc_noinline void xspan_fail_nullbase() {
    throwCantUnpack("xspan unexpected NULL base; take care!");
}
__acc_noinline void xspan_fail_not_same_base() {
    throwInternalError("xspan unexpected base pointer; take care!");
}

__acc_noinline void xspan_fail_range_nullptr() {
    throwCantUnpack("xspan_check_range: unexpected NULL pointer; take care!");
}
__acc_noinline void xspan_fail_range_nullbase() {
    throwCantUnpack("xspan_check_range: unexpected NULL base; take care!");
}
__acc_noinline void xspan_fail_range_range() {
    throwCantUnpack("xspan_check_range: pointer out of range; take care!");
}

void xspan_check_range(const void *p, const void *base, ptrdiff_t size_in_bytes) {
    if __acc_very_unlikely (p == nullptr)
        xspan_fail_range_nullptr();
    if __acc_very_unlikely (base == nullptr)
        xspan_fail_range_nullbase();
    ptrdiff_t off = (const char *) p - (const char *) base;
    if __acc_very_unlikely (off < 0 || off > size_in_bytes)
        xspan_fail_range_range();
    xspan_stats.check_range_counter += 1;
    // fprintf(stderr, "xspan_check_range done\n");
}

XSPAN_NAMESPACE_END

#endif // WITH_XSPAN

/* vim:set ts=4 sw=4 et: */
