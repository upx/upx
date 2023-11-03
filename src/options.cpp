/* options.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2023 Laszlo Molnar
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

#include "conf.h"

static Options global_options;
Options *opt = &global_options; // also see class PackMaster

#if WITH_THREADS
std::mutex opt_lock_mutex;
#endif

/*************************************************************************
// reset
**************************************************************************/

void Options::reset() noexcept {
#define opt ERROR_DO_NOT_USE_opt // protect against using the wrong variable
    Options *const o = this;
    mem_clear(o);
    o->crp.reset();

    o->cmd = CMD_NONE;
    o->method = M_NONE;
    o->level = -1;
    o->filter = FT_NONE;

    o->backup = -1;
    o->overlay = -1;
    o->preserve_mode = true;
    o->preserve_ownership = true;
    o->preserve_timestamp = true;
    o->verbose = 2;

    o->console = CON_FILE;
#if (ACC_OS_DOS32) && defined(__DJGPP__)
    o->console = CON_INIT;
#elif (USE_SCREEN_WIN32)
    o->console = CON_INIT;
#elif 1 && defined(__linux__)
    o->console = CON_INIT;
#endif
    // support NO_COLOR, see https://no-color.org/
    // "... when present and not an empty string (regardless of its value)"
    const char *e = getenv("NO_COLOR");
    if (e && e[0])
        o->console = CON_FILE;

    // options for various executable formats

    o->o_unix.osabi0 = 3; // 3 == ELFOSABI_LINUX

    o->win32_pe.compress_exports = 1;
    o->win32_pe.compress_icons = 2;
    o->win32_pe.compress_resources = -1;
    for (size_t i = 0; i < TABLESIZE(o->win32_pe.compress_rt); i++)
        o->win32_pe.compress_rt[i] = -1;
    o->win32_pe.compress_rt[24] = false; // 24 == RT_MANIFEST
    o->win32_pe.strip_relocs = -1;
    o->win32_pe.keep_resource = "";
#undef opt
}

/*************************************************************************
// doctest checks
**************************************************************************/

TEST_CASE("Options::reset") {
#define opt ERROR_DO_NOT_USE_opt // protect against using the wrong variable
    COMPILE_TIME_ASSERT(std::is_standard_layout<Options>::value)
    COMPILE_TIME_ASSERT(std::is_nothrow_default_constructible<Options>::value)
    COMPILE_TIME_ASSERT(std::is_trivially_copyable<Options>::value)

    Options local_options;
    Options *const o = &local_options;
    o->reset();
    CHECK(o->o_unix.osabi0 == 3);
    static_assert(TABLESIZE(o->win32_pe.compress_rt) == 25); // 25 == RT_LAST
    CHECK(o->win32_pe.compress_exports);
    CHECK(o->win32_pe.compress_icons);
    CHECK(o->win32_pe.strip_relocs);
    // issue 728
    CHECK(o->win32_pe.compress_resources);
    for (size_t i = 0; i < 24; i++)
        CHECK(o->win32_pe.compress_rt[i]);
    CHECK(!o->win32_pe.compress_rt[24]); // 24 == RT_MANIFEST
#undef opt
}

template <size_t N>
static inline void test_options(const char *(&a)[N]) {
    (void) main_get_options((int) (N - 1), ACC_UNCONST_CAST(char **, a));
}

TEST_CASE("getopt") {
#if WITH_THREADS
    std::lock_guard<std::mutex> lock(opt_lock_mutex);
#endif
    Options *const saved_opt = opt;
    Options local_options;
    opt = &local_options;
    opt->reset();
    opt->debug.getopt_throw_instead_of_exit = true;
    static const char a0[] = "<argv0>";

    SUBCASE("issue 587") {
        const char *a[] = {a0, "--brute", "--lzma", nullptr};
        test_options(a);
        CHECK(opt->all_methods);
        CHECK(opt->all_methods_use_lzma == 1);
    }
    SUBCASE("issue 587") {
        const char *a[] = {a0, "--lzma", "--brute", nullptr};
        test_options(a);
        CHECK(opt->all_methods);
        CHECK(opt->all_methods_use_lzma == 1);
    }
    SUBCASE("issue 587") {
        const char *a[] = {a0, "--brute", "--no-lzma", nullptr};
        test_options(a);
        CHECK(opt->all_methods);
        CHECK(opt->all_methods_use_lzma == -1);
    }
    SUBCASE("issue 587") {
        const char *a[] = {a0, "--no-lzma", "--brute", nullptr};
        test_options(a);
        CHECK(opt->all_methods);
        CHECK(opt->all_methods_use_lzma == -1);
    }
    SUBCASE("issue 587") {
        const char *a[] = {a0, "--no-lzma", "--lzma", nullptr};
        test_options(a);
        CHECK(!opt->all_methods);
        CHECK(opt->all_methods_use_lzma == 1);
        CHECK(opt->method == M_LZMA);
    }
    SUBCASE("issue 587") {
        const char *a[] = {a0, "--no-lzma", "--lzma", "--brute", nullptr};
        test_options(a);
        CHECK(opt->all_methods);
        CHECK(opt->all_methods_use_lzma == 1);
        CHECK(opt->method == -1);
    }
    SUBCASE("issue 587") {
        const char *a[] = {a0, "--lzma", "--no-lzma", nullptr};
        test_options(a);
        CHECK(!opt->all_methods);
        CHECK(opt->all_methods_use_lzma == -1);
        CHECK(opt->method == -1);
    }
    SUBCASE("issue 587") {
        const char *a[] = {a0, "--lzma", "--no-lzma", "--brute", nullptr};
        test_options(a);
        CHECK(opt->all_methods);
        CHECK(opt->all_methods_use_lzma == -1);
        CHECK(opt->method == -1);
    }

    opt = saved_opt;
}

/* vim:set ts=4 sw=4 et: */
