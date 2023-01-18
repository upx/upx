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

#if DEBUG && 0
#ifndef WITH_RANGELESS_FN
#define WITH_RANGELESS_FN 1
#endif
#endif

#if WITH_RANGELESS_FN
#define RANGELESS_FN_ENABLE_RUN_TESTS 1
#if defined(__i386__) && defined(__MSDOS__) && defined(__DJGPP__) && defined(__GNUC__)
#define RANGELESS_FN_ENABLE_PARALLEL 0
#elif defined(__m68k__) && defined(__atarist__) && defined(__GNUC__)
#define RANGELESS_FN_ENABLE_PARALLEL 0
#else
// disable multithreading for now; needs CMake find_package(Threads)
#define RANGELESS_FN_ENABLE_PARALLEL 0
#endif
#endif // WITH_RANGELESS_FN

#include "../conf.h"

/*************************************************************************
//
**************************************************************************/

#if WITH_RANGELESS_FN && RANGELESS_FN_ENABLE_RUN_TESTS

TEST_CASE("rangeless::fn") { CHECK_NOTHROW(rangeless::fn::impl::run_tests()); }

#if RANGELESS_FN_ENABLE_PARALLEL
TEST_CASE("rangeless::fn parallel") {
    // CHECK_NOTHROW(rangeless::mt::impl::run_tests());
    ACC_UNUSED_FUNC(rangeless::mt::impl::run_tests);
}
#endif

#endif // WITH_RANGELESS_FN

/* vim:set ts=4 sw=4 et: */
