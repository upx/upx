/* help.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) Markus Franz Xaver Johannes Oberhumer
   Copyright (C) Laszlo Molnar
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

#if !defined(__has_attribute)
static constexpr int has_attribute = 0;
#else
static constexpr int has_attribute = 1;
#endif
#if !defined(__has_builtin)
static constexpr int has_builtin = 0;
#else
static constexpr int has_builtin = 1;
#endif
#if !defined(__has_cpp_attribute)
static constexpr int has_cpp_attribute = 0;
#else
static constexpr int has_cpp_attribute = 1;
#endif
#if !defined(__has_declspec_attribute)
static constexpr int has_declspec_attribute = 0;
#else
static constexpr int has_declspec_attribute = 1;
#endif
#if !defined(__has_feature)
static constexpr int has_feature = 0;
#else
static constexpr int has_feature = 1;
#endif
#if !defined(__has_include)
static constexpr int has_include = 0;
#else
static constexpr int has_include = 1;
#endif
#if !defined(__has_warning)
static constexpr int has_warning = 0;
#else
static constexpr int has_warning = 1;
#endif

#if defined(_WIN32_WINNT)
static constexpr long long initial_win32_winnt = _WIN32_WINNT + 0LL;
#else
static constexpr long long initial_win32_winnt = 0;
#endif

#define WANT_WINDOWS_LEAN_H 1 // _WIN32_WINNT
#include "conf.h"
#include "compress/compress.h" // upx_ucl_version_string()
// for list_all_packers():
#include "filter.h" // Filter::isValidFilter
#include "packer.h"
#include "packmast.h" // PackMaster::visitAllPackers

/*************************************************************************
// header
**************************************************************************/

// also see UPX_CONFIG_DISABLE_GITREV in CMakeLists.txt
#if defined(UPX_VERSION_GITREV)
const char gitrev[] = UPX_VERSION_GITREV;
#else
const char gitrev[1] = {0};
#endif

void show_header() {
    FILE *f = con_term;
    int fg;

    static bool header_done;
    if (header_done)
        return;
    header_done = true;

    fg = con_fg(f, FG_GREEN);
    // clang-format off
    con_fprintf(f,
                "                       Ultimate Packer for eXecutables\n"
                "                          Copyright (C) 1996 - " UPX_VERSION_YEAR "\n"
#if defined(UPX_VERSION_GITREV)
                "UPX git-%6.6s%c"
#else
                "UPX %-11s"
#endif
                " Markus Oberhumer, Laszlo Molnar & John Reiser  %14s\n\n",
#if defined(UPX_VERSION_GITREV)
                gitrev,
                (sizeof(gitrev)-1 > 6 && gitrev[sizeof(gitrev)-2] == '+') ? '+' : ' ',
#else
                UPX_VERSION_STRING,
#endif
                UPX_VERSION_DATE);
    // clang-format on
    fg = con_fg(f, fg);
    UNUSED(fg);
}

/*************************************************************************
// usage
**************************************************************************/

void show_usage() {
    FILE *f = con_term;

    con_fprintf(f, "Usage: %s [-123456789dlthVL] [-qvfk] [-o file] %sfile..\n", progname,
#if (ACC_OS_DOS32) && defined(__DJGPP__)
                "[@]");
#else
                "");
#endif
}

/*************************************************************************
// list_all_packers()
**************************************************************************/

namespace {
struct PackerNames final {
    explicit PackerNames() noexcept = default;
    ~PackerNames() noexcept = default;

    static constexpr unsigned MAX_NAMES = 64; // arbitrary limit, increase as needed
    struct Entry final {
        int format;
        const char *fname;
        const char *sname;
        unsigned methods_count;
        unsigned filters_count;
        unsigned methods[PackerBase::MAX_METHODS];
        unsigned filters[PackerBase::MAX_FILTERS];
    };
    Entry names_array[MAX_NAMES];
    Entry *names[MAX_NAMES];
    unsigned names_count = 0;
    const Options *o = nullptr;

    noinline void add(const PackerBase *pb) {
        assert_noexcept(pb != nullptr);
        const int format = pb->getFormat();
        if (format != 25)
            for (unsigned i = 0; i < names_count; i++)
                assert_noexcept(names_array[i].format != format);
        assert_noexcept(names_count < MAX_NAMES);
        Entry &e = names_array[names_count];
        mem_clear(&e);
        names[names_count++] = &e;
        e.format = format;
        assert_noexcept(Packer::isValidFormat(e.format));
        e.fname = pb->getFullName(o);
        e.sname = pb->getName();
        assert_noexcept(e.fname != nullptr && e.fname[0]);
        assert_noexcept(e.sname != nullptr && e.sname[0]);
        for (const int *m = pb->getCompressionMethods(M_ALL, 10); *m != M_END; m++) {
            if (*m >= 0) {
                assert_noexcept(Packer::isValidCompressionMethod(*m));
                assert_noexcept(*m != 0);
                for (unsigned mm = 0; mm < e.methods_count; mm++)
                    assert_noexcept(e.methods[mm] != (unsigned) *m);
                assert_noexcept(e.methods_count < PackerBase::MAX_METHODS);
                e.methods[e.methods_count++] = *m;
            }
        }
        for (const int *f = pb->getFilters(); f != nullptr && *f != FT_END; f++) {
            if (*f >= 0) {
                assert_noexcept(Filter::isValidFilter(*f));
                assert_noexcept(*f != 0);
                for (unsigned ff = 0; ff < e.filters_count; ff++)
                    assert_noexcept(e.filters[ff] != (unsigned) *f);
                assert_noexcept(e.filters_count < PackerBase::MAX_FILTERS);
                e.filters[e.filters_count++] = *f;
            }
        }
        assert_noexcept(e.methods_count >= 1);
        upx_gnomesort(e.methods, e.methods_count, sizeof(e.methods[0]), ne32_compare);
        upx_gnomesort(e.filters, e.filters_count, sizeof(e.filters[0]), ne32_compare);
        NO_printf("%2u %3d %-36s %2u %2u\n", names_count, e.format, e.fname, e.methods_count,
                  e.filters_count);
    }
    static tribool visit(PackerBase *pb, void *user) {
        assert_noexcept(pb != nullptr);
        assert_noexcept(user != nullptr);
        NO_fprintf(stderr, "visit %s\n", pb->getFullName(nullptr));
        PackerNames *self = (PackerNames *) user;
        self->add(pb);
        return false;
    }
    static int __acc_cdecl_qsort compare_fname(const void *aa, const void *bb) {
        const Entry *a = *(const Entry *const *) aa;
        const Entry *b = *(const Entry *const *) bb;
        return strcmp(a->fname, b->fname);
    }
};
} // namespace

static noinline void list_all_packers(FILE *f, int verbose) {
    Options o;
    o.reset();
    o.o_unix.use_ptinterp = true;
    PackerNames pn;
    pn.o = &o;
    (void) PackMaster::visitAllPackers(PackerNames::visit, nullptr, &o, &pn);
    // NOLINTNEXTLINE(bugprone-multi-level-implicit-pointer-conversion)
    upx_gnomesort(pn.names, pn.names_count, sizeof(pn.names[0]), PackerNames::compare_fname);
    size_t pos = 0;
    for (size_t i = 0; i < pn.names_count; i++) {
        const PackerNames::Entry &e = *pn.names[i];
        const char *const fn = e.fname;
        const char *const sn = e.sname;
        if (verbose >= 3) {
            con_fprintf(f, "    %-36s %s\n", fn, sn);
            con_fprintf(f, "        methods:");
            for (size_t j = 0; j < e.methods_count; j++)
                con_fprintf(f, " %#x", e.methods[j]);
            con_fprintf(f, "\n");
            con_fprintf(f, "        filters:");
            for (size_t j = 0; j < e.filters_count; j++)
                con_fprintf(f, " %#x", e.filters[j]);
            con_fprintf(f, "\n");
        } else if (verbose >= 2) {
            con_fprintf(f, "    %-36s %s\n", fn, sn);
        } else {
            size_t fl = strlen(fn);
            if (pos == 0) {
                con_fprintf(f, "  %s", fn);
                pos = 2 + fl;
            } else if (pos + 1 + fl > 80) {
                con_fprintf(f, "\n  %s", fn);
                pos = 2 + fl;
            } else {
                con_fprintf(f, " %s", fn);
                pos += 1 + fl;
            }
        }
    }
    if (verbose < 2 && pn.names_count)
        con_fprintf(f, "\n");
}

/*************************************************************************
// help
**************************************************************************/

void show_help(int verbose) {
    FILE *f = con_term;
    int fg;

    show_header();
    show_usage();

    // clang-format off
    fg = con_fg(f, FG_YELLOW);
    con_fprintf(f, "\nCommands:\n");
    fg = con_fg(f, fg);
    con_fprintf(f,
                "  -1     compress faster                   -9    compress better\n"
                "%s"
                "  -d     decompress                        -l    list compressed file\n"
                "  -t     test compressed file              -V    display version number\n"
                "%s"
                "  -h     give %s help                    -L    display software license\n%s",
                verbose == 0 ? "" : "  --best compress best (can be slow for big files)\n",
                verbose == 0 ? "" : "  --fileinfo show parameters of already-compressed file\n",
                verbose == 0 ? "more" : "this",
                verbose == 0 ? "" : "\n");

    fg = con_fg(f, FG_YELLOW);
    con_fprintf(f, "Options:\n");
    fg = con_fg(f, fg);

    con_fprintf(f,
                "  -q     be quiet                          -v    be verbose\n"
                "  -oFILE write output to 'FILE'\n"
                "  -f     force compression of suspicious files\n"
                "%s%s%s"
                , (verbose == 0) ? "  -k     keep backup files\n" : ""
                , (verbose > 0) ? "  -i     info mode\n" : ""
                , (verbose > 0) ? "  --no-color, --mono, --color, --no-progress   change look\n" : ""
                );

    if (verbose > 0)
    {
        fg = con_fg(f, FG_YELLOW);
        con_fprintf(f, "\nCompression tuning options:\n");
        fg = con_fg(f, fg);
        con_fprintf(f,
                    "  --lzma              try LZMA [slower but tighter than NRV]\n"
                    "  --brute             try all available compression methods & filters [slow]\n"
                    "  --ultra-brute       try even more compression variants [very slow]\n"
                    "\n");
        fg = con_fg(f, FG_YELLOW);
        con_fprintf(f, "Backup options:\n");
        fg = con_fg(f, fg);
        con_fprintf(f,
                    "  -k, --backup        keep backup files\n"
                    "  --no-backup         no backup files [default]\n"
                    "\n");
        fg = con_fg(f, FG_YELLOW);
        con_fprintf(f, "Overlay options:\n");
        fg = con_fg(f, fg);
        con_fprintf(f,
                    "  --overlay=copy      copy any extra data attached to the file [default]\n"
                    "  --overlay=strip     strip any extra data attached to the file [DANGEROUS]\n"
                    "  --overlay=skip      don't compress a file with an overlay\n"
                    "\n");
        fg = con_fg(f, FG_YELLOW);
        con_fprintf(f, "File system options:\n");
        fg = con_fg(f, fg);
        con_fprintf(f,
                    "  --force-overwrite   force overwrite of output files\n"
#if defined(__unix__)
                    "  --link              preserve hard links (Unix only) [USE WITH CARE]\n"
                    "  --no-link           do not preserve hard links but rename files [default]\n"
#endif
                    "  --no-mode           do not preserve file mode (aka permissions)\n"
                    "  --no-owner          do not preserve file ownership\n"
                    "  --no-time           do not preserve file timestamp\n"
                    "\n");
        fg = con_fg(f, FG_YELLOW);
        con_fprintf(f, "Options for djgpp2/coff:\n");
        fg = con_fg(f, fg);
        con_fprintf(f,
                    "  --coff              produce COFF output [default: EXE]\n"
                    "\n");
        fg = con_fg(f, FG_YELLOW);
        con_fprintf(f, "Options for dos/com:\n");
        fg = con_fg(f, fg);
        con_fprintf(f,
                    "  --8086              make compressed com work on any 8086\n"
                    "\n");
        fg = con_fg(f, FG_YELLOW);
        con_fprintf(f, "Options for dos/exe:\n");
        fg = con_fg(f, fg);
        con_fprintf(f,
                    "  --8086              make compressed exe work on any 8086\n"
                    "  --no-reloc          put no relocations in to the exe header\n"
                    "\n");
        fg = con_fg(f, FG_YELLOW);
        con_fprintf(f, "Options for dos/sys:\n");
        fg = con_fg(f, fg);
        con_fprintf(f,
                    "  --8086              make compressed sys work on any 8086\n"
                    "\n");
        fg = con_fg(f, FG_YELLOW);
        con_fprintf(f, "Options for linux/elf:\n");
        fg = con_fg(f, fg);
        con_fprintf(f,
                    "  --preserve-build-id     copy .gnu.note.build-id to compressed output\n"
                    "  --catch-sigsegv         debug errors in hardware or de-compressor\n"
                    "\n");
        fg = con_fg(f, FG_YELLOW);
        con_fprintf(f, "Options for ps1/exe:\n");
        fg = con_fg(f, fg);
        con_fprintf(f,
                    "  --8-bit             uses 8 bit size compression [default: 32 bit]\n"
                    "  --8mib-ram          8 megabyte memory limit [default: 2 MiB]\n"
                    "  --boot-only         disables client/host transfer compatibility\n"
                    "  --no-align          don't align to 2048 bytes [enables: --console-run]\n"
                    "\n");
        fg = con_fg(f, FG_YELLOW);
        con_fprintf(f, "Options for watcom/le:\n");
        fg = con_fg(f, fg);
        con_fprintf(f,
                    "  --le                produce LE output [default: EXE]\n"
                    "\n");
        fg = con_fg(f, FG_YELLOW);
        con_fprintf(f, "Options for win32/pe, win64/pe & rtm32/pe:\n");
        fg = con_fg(f, fg);
        con_fprintf(f,
                    "  --compress-exports=0    do not compress the export section\n"
                    "  --compress-exports=1    compress the export section [default]\n"
                    "  --compress-icons=0      do not compress any icons\n"
                    "  --compress-icons=1      compress all but the first icon\n"
                    "  --compress-icons=2      compress all but the first icon directory [default]\n"
                    "  --compress-icons=3      compress all icons\n"
                    "  --compress-resources=0  do not compress any resources at all\n"
                    "  --keep-resource=list    do not compress resources specified by list\n"
                    "  --strip-relocs=0        do not strip relocations\n"
                    "  --strip-relocs=1        strip relocations [default]\n"
                    "\n");
    }
    // clang-format on

    con_fprintf(f, "file..   executables to (de)compress\n");

    if (verbose > 0) {
        fg = con_fg(f, FG_YELLOW);
        con_fprintf(f, "\nThis version supports:\n");
        fg = con_fg(f, fg);
        list_all_packers(f, verbose);
    } else {
        con_fprintf(f, "\nType '%s --help' for more detailed help.\n", progname);
    }

    con_fprintf(f, "\nUPX comes with ABSOLUTELY NO WARRANTY; "
                   "for details visit https://upx.github.io\n");

#if DEBUG || TESTING
    fg = con_fg(f, FG_RED);
    con_fprintf(f, "\nWARNING: this version is compiled with"
#if DEBUG
                   " -DDEBUG"
#endif
#if TESTING
                   " -DTESTING"
#endif
                   "\n");
    fg = con_fg(f, fg);
#endif

    UNUSED(fg);
}

/*************************************************************************
// license
**************************************************************************/

void show_license() {
    FILE *f = con_term;

    show_header();

    // clang-format off
    con_fprintf(f,
        "   This program may be used freely, and you are welcome to\n"
        "   redistribute it under certain conditions.\n"
        "\n"
        "   This program is distributed in the hope that it will be useful,\n"
        "   but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        "   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
        "   UPX License Agreements for more details.\n"
        "\n"
        "   You should have received a copy of the UPX License Agreements\n"
        "   along with this program; see the files COPYING and LICENSE.\n"
        "   If not, visit one of the following pages:\n"
        "\n"
    );
    int fg = con_fg(f, FG_CYAN);
    con_fprintf(f,
        "        https://upx.github.io\n"
        "        https://www.oberhumer.com/opensource/upx/\n"
    );
    (void) con_fg(f, FG_ORANGE);
    con_fprintf(f,
        "\n"
        "   Markus F.X.J. Oberhumer              Laszlo Molnar\n"
        "   <markus@oberhumer.com>               <ezerotven+github@gmail.com>\n"
    );
    // clang-format on
    fg = con_fg(f, fg);
    UNUSED(fg);
}

/*************************************************************************
// version
**************************************************************************/

void show_version(bool one_line) {
    FILE *f = stdout;
    const char *v;

#if defined(UPX_VERSION_GIT_DESCRIBE)
    fprintf(f, "upx %s\n", UPX_VERSION_GIT_DESCRIBE);
#elif defined(UPX_VERSION_GITREV)
    fprintf(f, "upx %s\n", UPX_VERSION_STRING "-git-" UPX_VERSION_GITREV);
#else
    fprintf(f, "upx %s\n", UPX_VERSION_STRING);
#endif
    if (one_line)
        return;

#if (WITH_NRV)
    v = upx_nrv_version_string();
    if (v != nullptr && v[0])
        fprintf(f, "NRV data compression library %s\n", v);
#endif
#if (WITH_UCL)
    v = upx_ucl_version_string();
    if (v != nullptr && v[0])
        fprintf(f, "UCL data compression library %s\n", v);
#endif
#if (WITH_ZLIB)
    v = upx_zlib_version_string();
    if (v != nullptr && v[0])
        fprintf(f, "zlib data compression library %s\n", v);
#endif
#if (WITH_LZMA)
    v = upx_lzma_version_string();
    if (v != nullptr && v[0])
        fprintf(f, "LZMA SDK version %s\n", v);
#endif
#if (WITH_ZSTD)
    v = upx_zstd_version_string();
    if (v != nullptr && v[0])
        fprintf(f, "zstd data compression library %s\n", v);
#endif
#if (WITH_BZIP2)
    v = upx_bzip2_version_string();
    if (v != nullptr && v[0])
        fprintf(f, "bzip2 data compression library %s\n", v);
#endif
#if !defined(DOCTEST_CONFIG_DISABLE)
    fprintf(f, "doctest C++ testing framework version %s\n", DOCTEST_VERSION_STR);
#endif
    // clang-format off
    fprintf(f, "Copyright (C) 1996-2026 Markus Franz Xaver Johannes Oberhumer\n");
    fprintf(f, "Copyright (C) 1996-2026 Laszlo Molnar\n");
    fprintf(f, "Copyright (C) 2000-2026 John F. Reiser\n");
#if (WITH_ZLIB)
    // see vendor/zlib/LICENSE
    fprintf(f, "Copyright (C) 1995" "-2024 Jean-loup Gailly and Mark Adler\n");
#endif
#if (WITH_LZMA)
    // see vendor/lzma-sdk
    fprintf(f, "Copyright (C) 1999" "-2006 Igor Pavlov\n");
#endif
#if (WITH_ZSTD)
    // see vendor/zstd/LICENSE; main author is Yann Collet
    fprintf(f, "Copyright (C) 2015" "-2024 Meta Platforms, Inc. and affiliates\n");
#endif
#if (WITH_BZIP2)
    // see vendor/bzip2/bzlib.h
    fprintf(f, "Copyright (C) 1996" "-2010 Julian Seward\n");
#endif
#if !defined(DOCTEST_CONFIG_DISABLE)
    // see vendor/doctest/LICENSE.txt
    fprintf(f, "Copyright (C) 2016" "-2023 Viktor Kirilov\n");
#endif
    // clang-format on
    fprintf(f, "UPX comes with ABSOLUTELY NO WARRANTY; for details type '%s -L'.\n", progname);
}

/*************************************************************************
// sysinfo
// undocumented and subject to change
**************************************************************************/

void show_sysinfo(const char *options_var) {
    FILE *f = con_term;

    show_header();

    if (opt->verbose >= 1) {
        con_fprintf(f, "UPX version: ");
        fflush(f);
        show_version(true);
        con_fprintf(f, "UPX version internal: 0x%06x %s\n", UPX_VERSION_HEX, UPX_VERSION_STRING);
    }
    fflush(stdout);

    // Compilation Flags
    {
        size_t cf_count = 0;
        auto cf_print = [f, &cf_count](const char *name, const char *fmt, long long v,
                                       int need_verbose = 2) noexcept {
            if (opt->verbose < need_verbose)
                return;
            if (cf_count++ == 0)
                con_fprintf(f, "\nCompilation flags:\n");
            con_fprintf(f, "  %s = ", name);
            con_fprintf(f, fmt, v);
            con_fprintf(f, "\n");
        };

        // language
        cf_print("__cplusplus", "%lld", __cplusplus + 0, 3);
#if defined(_MSVC_LANG)
        cf_print("_MSVC_LANG", "%lld", _MSVC_LANG + 0, 3);
#endif
#if defined(upx_is_constant_evaluated)
        cf_print("upx_is_constant_evaluated", "%lld", 1, 3);
#endif

        // compiler
#if defined(ACC_CC_CLANG)
        cf_print("ACC_CC_CLANG", "0x%06llx", ACC_CC_CLANG + 0, 3);
#endif
#if defined(ACC_CC_GNUC)
        cf_print("ACC_CC_GNUC", "0x%06llx", ACC_CC_GNUC + 0, 3);
#endif
#if defined(ACC_CC_MSC)
        cf_print("ACC_CC_MSC", "%lld", ACC_CC_MSC + 0, 3);
#endif
#if defined(__clang__)
        cf_print("__clang__", "%lld", __clang__ + 0);
#endif
#if defined(__clang_major__)
        cf_print("__clang_major__", "%lld", __clang_major__ + 0);
#endif
#if defined(__clang_minor__)
        cf_print("__clang_minor__", "%lld", __clang_minor__ + 0, 3);
#endif
#if defined(__clang_patchlevel__)
        cf_print("__clang_patchlevel__", "%lld", __clang_patchlevel__ + 0, 3);
#endif
#if defined(__GNUC__)
        cf_print("__GNUC__", "%lld", __GNUC__ + 0);
#endif
#if defined(__GNUC_MINOR__)
        cf_print("__GNUC_MINOR__", "%lld", __GNUC_MINOR__ + 0, 3);
#endif
#if defined(__GNUC_PATCHLEVEL__)
        cf_print("__GNUC_PATCHLEVEL__", "%lld", __GNUC_PATCHLEVEL__ + 0, 3);
#endif
#if defined(_MSC_VER)
        cf_print("_MSC_VER", "%lld", _MSC_VER + 0);
#endif
#if defined(_MSC_FULL_VER)
        cf_print("_MSC_FULL_VER", "%lld", _MSC_FULL_VER + 0);
#endif
        cf_print("__has_attribute", "%lld", has_attribute, 4);
        cf_print("__has_builtin", "%lld", has_builtin, 4);
        cf_print("__has_cpp_attribute", "%lld", has_cpp_attribute, 4);
        cf_print("__has_declspec_attribute", "%lld", has_declspec_attribute, 4);
        cf_print("__has_feature", "%lld", has_feature, 4);
        cf_print("__has_include", "%lld", has_include, 4);
        cf_print("__has_warning", "%lld", has_warning, 4);

        // architecture
#if defined(__CHERI__)
        cf_print("__CHERI__", "%lld", __CHERI__ + 0, 3);
#endif
#if defined(__CHERI_PURE_CAPABILITY__)
        cf_print("__CHERI_PURE_CAPABILITY__", "%lld", __CHERI_PURE_CAPABILITY__ + 0, 3);
#endif
#if defined(__mips_hard_float)
        cf_print("__mips_hard_float", "%lld", __mips_hard_float + 0);
#endif
#if defined(__mips_soft_float)
        cf_print("__mips_soft_float", "%lld", __mips_soft_float + 0);
#endif
#if defined(__wasm__)
        cf_print("__wasm__", "%lld", __wasm__ + 0);
#endif
#if defined(__wasm32__)
        cf_print("__wasm32__", "%lld", __wasm32__ + 0);
#endif
#if defined(__wasm64__)
        cf_print("__wasm64__", "%lld", __wasm64__ + 0);
#endif

        // OS and libc
#if defined(WINVER)
        cf_print("WINVER", "0x%04llx", WINVER + 0);
#endif
#if defined(_WIN32_WINNT)
        cf_print("_WIN32_WINNT", "0x%04llx", _WIN32_WINNT + 0);
        if (initial_win32_winnt != 0 && initial_win32_winnt != _WIN32_WINNT + 0)
            cf_print("INITIAL_WIN32_WINNT", "0x%04llx", initial_win32_winnt);
#endif
#if defined(__MSVCRT_VERSION__)
        cf_print("__MSVCRT_VERSION__", "0x%04llx", __MSVCRT_VERSION__ + 0);
#endif
#if defined(__MINGW64_VERSION_MAJOR)
        cf_print("__MINGW64_VERSION_MAJOR", "%lld", __MINGW64_VERSION_MAJOR + 0);
#endif
#if defined(__USE_MINGW_ANSI_STDIO)
        cf_print("__USE_MINGW_ANSI_STDIO", "%lld", __USE_MINGW_ANSI_STDIO + 0, 3);
#endif
#if defined(__ELF__)
        cf_print("__ELF__", "%lld", __ELF__ + 0, 3);
#endif
#if defined(__GLIBC__)
        cf_print("__GLIBC__", "%lld", __GLIBC__ + 0);
#endif
#if defined(__GLIBC_MINOR__)
        cf_print("__GLIBC_MINOR__", "%lld", __GLIBC_MINOR__ + 0);
#endif
#if defined(__wasi__)
        cf_print("__wasi__", "%lld", __wasi__ + 0);
#endif

        // misc compilation options
#if defined(__PIC__)
        cf_print("__PIC__", "%lld", __PIC__ + 0, 3);
#elif defined(__pic__)
        cf_print("__pic__", "%lld", __pic__ + 0, 3);
#endif
#if defined(__PIE__)
        cf_print("__PIE__", "%lld", __PIE__ + 0, 3);
#elif defined(__pie__)
        cf_print("__pie__", "%lld", __pie__ + 0, 3);
#endif
#if defined(__SIZEOF_INT128__)
        cf_print("__SIZEOF_INT128__", "%lld", __SIZEOF_INT128__ + 0, 3);
#endif
#if defined(__SIZEOF_LONG_LONG__) && (__SIZEOF_LONG_LONG__ + 0 > 8)
        cf_print("__SIZEOF_LONG_LONG__", "%lld", __SIZEOF_LONG_LONG__ + 0, 3);
#endif
        cf_print("__SIZEOF_POINTER__", "%lld", (long long) sizeof(void *), 3);
#if defined(__SIZEOF_POINTER__) && (__SIZEOF_POINTER__ + 0 > 8)
        cf_print("__SIZEOF_POINTER__", "%lld", __SIZEOF_POINTER__ + 0, 3);
#endif
        cf_print("__SIZEOF_SIZE_T__", "%lld", (long long) sizeof(size_t), 3);
#if (ACC_ABI_BIG_ENDIAN)
        cf_print("ACC_ABI_BIG_ENDIAN", "%lld", ACC_ABI_BIG_ENDIAN + 0, 4);
#elif (ACC_ABI_LITTLE_ENDIAN)
        cf_print("ACC_ABI_LITTLE_ENDIAN", "%lld", ACC_ABI_LITTLE_ENDIAN + 0, 4);
#else
#error "ACC_ABI_ENDIAN"
#endif
#if defined(UPX_CONFIG_DISABLE_WSTRICT)
        cf_print("UPX_CONFIG_DISABLE_WSTRICT", "%lld", UPX_CONFIG_DISABLE_WSTRICT + 0, 4);
#endif
#if defined(UPX_CONFIG_DISABLE_WERROR)
        cf_print("UPX_CONFIG_DISABLE_WERROR", "%lld", UPX_CONFIG_DISABLE_WERROR + 0, 4);
#endif
#if defined(DOCTEST_CONFIG_DISABLE)
        cf_print("DOCTEST_CONFIG_DISABLE", "%lld", DOCTEST_CONFIG_DISABLE + 0, 3);
#endif
#if defined(UPX_CONFIG_USE_STABLE_SORT)
        cf_print("UPX_CONFIG_USE_STABLE_SORT", "%lld", UPX_CONFIG_USE_STABLE_SORT + 0, 3);
#endif
#if defined(WITH_XSPAN)
        cf_print("WITH_XSPAN", "%lld", WITH_XSPAN + 0, 3);
#endif
#if defined(XSPAN_CONFIG_ENABLE_DEBUG)
        cf_print("XSPAN_CONFIG_ENABLE_DEBUG", "%lld", XSPAN_CONFIG_ENABLE_DEBUG + 0, 3);
#endif
#if defined(WITH_THREADS)
        cf_print("WITH_THREADS", "%lld", WITH_THREADS + 0);
#endif

        UNUSED(cf_count);
        UNUSED(cf_print);
        UNUSED(initial_win32_winnt);
    }

    // run-time settings
#if defined(HAVE_LOCALTIME) && defined(HAVE_GMTIME)
    {
        auto tm2str = [](char *s, size_t size, const struct tm *tmp) noexcept {
            snprintf(s, size, "%04d-%02d-%02d %02d:%02d:%02d", (int) tmp->tm_year + 1900,
                     (int) tmp->tm_mon + 1, (int) tmp->tm_mday, (int) tmp->tm_hour,
                     (int) tmp->tm_min, (int) tmp->tm_sec);
        };

        char s[64];
        const time_t t = time(nullptr);
        tm2str(s, sizeof(s), localtime(&t));
        con_fprintf(f, "\n");
        con_fprintf(f, "Local time is:  %s\n", s);
        tm2str(s, sizeof(s), gmtime(&t));
        con_fprintf(f, "UTC time is:    %s\n", s);
    }
#endif

    // environment
    if (options_var && options_var[0]) {
        const char *e = upx_getenv(options_var);
        con_fprintf(f, "\n");
        if (e && e[0])
            con_fprintf(f, "Contents of environment variable %s: '%s'\n\n", options_var, e);
        else if (e)
            con_fprintf(f, "Environment variable '%s' is set but empty.\n\n", options_var);
        else
            con_fprintf(f, "Environment variable '%s' is not set.\n\n", options_var);
    }
}

/* vim:set ts=4 sw=4 et: */
