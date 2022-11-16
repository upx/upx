/* options.h --

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
#ifndef UPX_OPTIONS_H__
#define UPX_OPTIONS_H__ 1

/*************************************************************************
// globals
**************************************************************************/

// options - command
enum {
    CMD_NONE,
    CMD_COMPRESS,
    CMD_DECOMPRESS,
    CMD_TEST,
    CMD_LIST,
    CMD_FILEINFO,
    CMD_HELP,
    CMD_LICENSE,
    CMD_VERSION
};

struct options_t {
    int cmd;

    // compression options
    int method;
    bool method_lzma_seen;
    bool method_nrv2b_seen;
    bool method_nrv2d_seen;
    bool method_nrv2e_seen;
    int level;  // compression level 1..10
    int filter; // preferred filter from Packer::getFilters()
    bool ultra_brute;
    bool all_methods; // try all available compression methods ?
    int all_methods_use_lzma;
    bool all_filters; // try all available filters ?
    bool no_filter;   // force no filter
    bool prefer_ucl;  // prefer UCL
    bool exact;       // user requires byte-identical decompression

    // other options
    int backup;
    int console;
    int force;
    bool force_overwrite;
    int info_mode;
    bool ignorewarn;
    bool no_env;
    bool no_progress;
    const char *output_name;
    bool preserve_mode;
    bool preserve_ownership;
    bool preserve_timestamp;
    int small;
    int verbose;
    bool to_stdout;

    // debug options
    struct {
        int debug_level;
        bool disable_random_id; // for Packer::getRandomId()
        const char *dump_stub_loader;
        char fake_stub_version[4 + 1];     // for internal debugging
        char fake_stub_year[4 + 1];        // for internal debugging
        bool getopt_throw_instead_of_exit; // for doctest
    } debug;

    // overlay handling
    enum { SKIP_OVERLAY = 0, COPY_OVERLAY = 1, STRIP_OVERLAY = 2 };
    int overlay;

    // compression runtime parameters - see struct XXX_compress_config_t
    struct crp_t {
        lzma_compress_config_t crp_lzma;
        ucl_compress_config_t crp_ucl;
        zlib_compress_config_t crp_zlib;
        void reset() {
            crp_lzma.reset();
            crp_ucl.reset();
            crp_zlib.reset();
        }
    };
    crp_t crp;

    // CPU
    enum {
        CPU_DEFAULT = 0,
        CPU_8086 = 1,
        CPU_286 = 2,
        CPU_386 = 3,
        CPU_486 = 4,
        CPU_586 = 5,
        CPU_686 = 6
    };
    int cpu;

    // options for various executable formats
    struct {
        bool split_segments;
    } atari_tos;
    struct {
        bool coff;
    } djgpp2_coff;
    struct {
        bool force_stub;
        bool no_reloc;
    } dos_exe;
    struct {
        bool boot_only;
        bool no_align;
        bool do_8bit;
        bool do_8mib;
    } ps1_exe;
    struct {
        unsigned blocksize;
        bool force_execve;      // force the linux/386 execve format
        bool is_ptinterp;       // is PT_INTERP, so don't adjust auxv_t
        bool use_ptinterp;      // use PT_INTERP /opt/upx/run
        bool make_ptinterp;     // make PT_INTERP [ignore current file!]
        bool unmap_all_pages;   // thus /proc/self/exe vanishes
        unsigned char osabi0;   // replacement if 0==.e_ident[EI_OSABI]
        bool preserve_build_id; // copy the build-id to the compressed binary
        bool android_shlib;     // keep some ElfXX_Shdr for dlopen()
        bool force_pie;         // choose DF_1_PIE instead of is_shlib
    } o_unix;
    struct {
        bool le;
    } watcom_le;
    struct {
        int compress_exports;
        int compress_icons;
        int compress_resources;
        signed char compress_rt[25]; // 25 == RT_LAST
        int strip_relocs;
        const char *keep_resource;
    } win32_pe;

    void reset();
};

extern struct options_t *opt;

#endif /* already included */

/* vim:set ts=4 sw=4 et: */
