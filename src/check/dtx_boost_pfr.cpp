/* dtx_.cpp -- DocTest eXtra checks

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

#if DEBUG
#ifndef WITH_BOOST_PFR
#define WITH_BOOST_PFR 1
#endif
#endif

#include "../conf.h"

/*************************************************************************
//
**************************************************************************/

#if WITH_BOOST_PFR

TEST_CASE("boost::pfr") {
    struct Foo {
        BE16 b16;
        BE32 b32;
        BE64 b64;
        LE16 l16;
        LE32 l32;
        LE64 l64;
    };

    {
        int i = -1;
        CHECK_EQ(strcmp(pfr_str(i), "-1"), 0);
        BE32 b32;
        b32 = 1;
        LE32 l32;
        l32 = 2;
        CHECK_EQ(strcmp(pfr_str(b32), "1"), 0);
        CHECK_EQ(strcmp(pfr_str(l32), "2"), 0);
    }
    {
        Foo foo;
        foo.b16 = 1;
        foo.b32 = 2;
        foo.b64 = 3;
        foo.l16 = 4;
        foo.l32 = 5;
        foo.l64 = 6;
        CHECK_EQ(strcmp(pfr_str("foo", "=", foo), "foo = {1, 2, 3, 4, 5, 6}"), 0);
    }
    {
#if (ACC_ABI_BIG_ENDIAN)
#else
        constexpr Foo foo{{1}, {1}, {1}, {1}, {1}, {1}};
        CHECK_EQ(strcmp(pfr_str(foo), "{256, 16777216, 72057594037927936, 1, 1, 1}"), 0);
#endif
    }
}

#endif // WITH_BOOST_PFR

/* vim:set ts=4 sw=4 et: */
