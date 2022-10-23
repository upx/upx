/* options.cpp --

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

#include "conf.h"

/*************************************************************************
// options
**************************************************************************/

void options_t::reset() {
    options_t *o = this;
    mem_clear(o, sizeof(*o));
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

    o->console = CON_FILE;
#if (ACC_OS_DOS32) && defined(__DJGPP__)
    o->console = CON_INIT;
#elif (USE_SCREEN_WIN32)
    o->console = CON_INIT;
#elif 1 && defined(__linux__)
    o->console = CON_INIT;
#endif
    o->verbose = 2;

    opt->o_unix.osabi0 = 3; // 3 == ELFOSABI_LINUX

    o->win32_pe.compress_exports = 1;
    o->win32_pe.compress_icons = 2;
    o->win32_pe.compress_resources = -1;
    for (unsigned i = 0; i < TABLESIZE(o->win32_pe.compress_rt); i++)
        o->win32_pe.compress_rt[i] = -1;
    o->win32_pe.compress_rt[24] = false; // 24 == RT_MANIFEST
    o->win32_pe.strip_relocs = -1;
    o->win32_pe.keep_resource = "";
}

static options_t global_options;
options_t *opt = &global_options;

/*************************************************************************
//
**************************************************************************/

template <size_t N>
static inline void test_options(const char *(&a)[N]) {
    (void) main_get_options((int) (N - 1), ACC_UNCONST_CAST(char **, a));
}

TEST_CASE("getopt") {
    options_t *saved_opt = opt;
    options_t local_options;
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
