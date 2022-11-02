/* main.cpp --

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
#include "compress.h"
#include "file.h"
#include "packer.h"
#include "p_elf.h"

/*************************************************************************
// options
**************************************************************************/

static const char *argv0 = "";
const char *progname = "";

static acc_getopt_t mfx_getopt;
#define mfx_optarg mfx_getopt.optarg
#define mfx_optind mfx_getopt.optind
#define mfx_option acc_getopt_longopt_t
static void handle_opterr(acc_getopt_p g, const char *f, void *v) {
    struct A {
        va_list ap;
    };
    struct A *a = (struct A *) v;
    fprintf(stderr, "%s: ", g->progname);
    vfprintf(stderr, f, a->ap);
    fprintf(stderr, "\n");
}

/*************************************************************************
// exit handlers
**************************************************************************/

static int exit_code = EXIT_OK;

#if (WITH_GUI)
__acc_static_noinline void do_exit(void) { throw exit_code; }
#else
#if defined(__GNUC__)
static void do_exit(void) __attribute__((__noreturn__));
#endif
static void do_exit(void) {
    static bool in_exit = false;

    if (in_exit)
        exit(exit_code);
    in_exit = true;

    fflush(con_term);
    fflush(stderr);
    exit(exit_code);
}
#endif

#define EXIT_FATAL 3

static bool set_eec(int ec, int *eec) {
    if (ec == EXIT_FATAL) {
        *eec = EXIT_ERROR;
        return 1;
    } else if (ec < 0 || ec == EXIT_ERROR) {
        *eec = EXIT_ERROR;
    } else if (ec == EXIT_WARN) {
        if (!opt->ignorewarn)
            if (*eec == EXIT_OK)
                *eec = ec;
    } else if (ec == EXIT_OK) {
        /* do nothing */
    } else {
        assert(0);
    }
    return 0;
}

bool main_set_exit_code(int ec) { return set_eec(ec, &exit_code); }

__acc_static_noinline void e_exit(int ec) {
    if (opt->debug.getopt_throw_instead_of_exit)
        throw ec;
    (void) main_set_exit_code(ec);
    do_exit();
}

__acc_static_noinline void e_usage(void) {
    if (opt->debug.getopt_throw_instead_of_exit)
        throw EXIT_USAGE;
    show_usage();
    e_exit(EXIT_USAGE);
}

#if 0  // UNUSED
static void e_memory(void)
{
    show_head();
    fflush(con_term);
    fprintf(stderr,"%s: out of memory\n", argv0);
    e_exit(EXIT_MEMORY);
}
#endif // UNUSED

static void e_method(int m, int l) {
    fflush(con_term);
    fprintf(stderr, "%s: illegal method option -- %d/%d\n", argv0, m, l);
    e_usage();
}

static void e_optarg(const char *n) {
    fflush(con_term);
    fprintf(stderr, "%s: invalid argument in option '%s'\n", argv0, n);
    e_exit(EXIT_USAGE);
}

static void e_optval(const char *n) {
    fflush(con_term);
    fprintf(stderr, "%s: invalid value for option '%s'\n", argv0, n);
    e_exit(EXIT_USAGE);
}

#if defined(OPTIONS_VAR)
static void e_envopt(const char *n) {
    fflush(con_term);
    if (n)
        fprintf(stderr, "%s: invalid string '%s' in environment variable '%s'\n", argv0, n,
                OPTIONS_VAR);
    else
        fprintf(stderr, "%s: illegal option in environment variable '%s'\n", argv0, OPTIONS_VAR);
    e_exit(EXIT_USAGE);
}
#endif /* defined(OPTIONS_VAR) */

#if 0  // UNUSED
static void __acc_cdecl_sighandler e_sighandler(int signum)
{
    UNUSED(signum);
    e_exit(EXIT_FATAL);
}
#endif // UNUSED

/*************************************************************************
// check options
**************************************************************************/

static void check_not_both(bool e1, bool e2, const char *c1, const char *c2) {
    if (e1 && e2) {
        fprintf(stderr, "%s: ", argv0);
        fprintf(stderr, "cannot use both '%s' and '%s'\n", c1, c2);
        e_usage();
    }
}

static void check_options(int i, int argc) {
    assert(i <= argc);

    if (opt->cmd != CMD_COMPRESS) {
        // invalidate compression options
        opt->method = 0;
        opt->level = 0;
        opt->exact = 0;
        opt->small = 0;
        opt->crp.reset();
    }

    // set default overlay action
    if (!(opt->cmd == CMD_COMPRESS || opt->cmd == CMD_DECOMPRESS))
        opt->overlay = opt->COPY_OVERLAY;
    else if (opt->overlay < 0)
        opt->overlay = opt->COPY_OVERLAY;

    check_not_both(opt->exact, opt->overlay == opt->STRIP_OVERLAY, "--exact", "--overlay=strip");

    // set default backup option
    if (opt->backup < 0)
        opt->backup = 0;
    if (!(opt->cmd == CMD_COMPRESS || opt->cmd == CMD_DECOMPRESS))
        opt->backup = 1;

    check_not_both(opt->to_stdout, opt->output_name != nullptr, "--stdout", "-o");
    if (opt->to_stdout && opt->cmd == CMD_COMPRESS) {
        fprintf(stderr, "%s: cannot use '--stdout' when compressing\n", argv0);
        e_usage();
    }
    if (opt->to_stdout || opt->output_name) {
        if (i + 1 != argc) {
            fprintf(stderr, "%s: need exactly one argument when using '%s'\n", argv0,
                    opt->to_stdout ? "--stdout" : "-o");
            e_usage();
        }
    }
}

/*************************************************************************
// misc
**************************************************************************/

static void e_help(void) {
    show_help();
    e_exit(EXIT_USAGE);
}

static void set_term(FILE *f) {
    if (f)
        con_term = f;
    else
        con_term = acc_isatty(STDIN_FILENO) ? stderr : stdout;
}

static void set_cmd(int cmd) {
    if (cmd > opt->cmd)
        opt->cmd = cmd;
}

static bool set_method(int m, int l) {
    if (m > 0) {
        if (!Packer::isValidCompressionMethod(m))
            return false;
        // something like "--brute --lzma" should not disable "--brute"
        if (!opt->all_methods)
            opt->method = m;
    }
    if (l > 0)
        opt->level = l;
    set_cmd(CMD_COMPRESS);
    return true;
}

static void set_output_name(const char *n, bool allow_m) {
#if 1
    if (opt->output_name) {
        fprintf(stderr, "%s: option '-o' more than once given\n", argv0);
        e_usage();
    }
#endif
    if (!n || !n[0] || (!allow_m && n[0] == '-')) {
        fprintf(stderr, "%s: missing output name\n", argv0);
        e_usage();
    }
    if (strlen(n) >= ACC_FN_PATH_MAX - 4) {
        fprintf(stderr, "%s: output name too long\n", argv0);
        e_usage();
    }
    opt->output_name = n;
}

/*************************************************************************
// get options
**************************************************************************/

static char *prepare_shortopts(char *buf, const char *n, const struct mfx_option *longopts) {
    char *o = buf;

    for (; n && *n; n++)
        if (*n != ' ')
            *o++ = *n;
    *o = 0;
    for (; longopts && longopts->name; longopts++) {
        int v = longopts->val;
#if !defined(NDEBUG)
        assert(longopts->name[0] != '\0');
        assert(longopts->name[0] != '-');
        if (longopts->has_arg & 0x20)
            assert((longopts->has_arg & 0xf) == 1);
#endif
#if 0
        static char vopts[1024];
        if (v > 0 && v < 1024)
        {
            if (vopts[v] && strchr(buf,v) == nullptr)
                printf("warning: duplicate option %d ('%c')!\n", v, v & 127);
            vopts[v] = 1;
        }
#endif
        if (v > 0 && v < 256 && strchr(buf, v) == nullptr) {
            *o++ = (char) v;
            if ((longopts->has_arg & 0xf) >= 1)
                *o++ = ':';
            if ((longopts->has_arg & 0xf) >= 2)
                *o++ = ':';
            *o = 0;
        }
        if (longopts->has_arg & 0x20)
            assert((longopts->has_arg & 0xf) == 1);
    }
    return buf;
}

template <class T>
static int getoptvar(T *var, const T min_value, const T max_value, const char *arg_fatal) {
    const char *p = mfx_optarg;
    char *endptr;
    int r = 0;
    long n;
    T v;

    if (!p || !p[0]) {
        r = -1;
        goto error;
    }
    // avoid interpretation as octal value
    while (p[0] == '0' && isdigit(p[1]))
        p++;
    n = strtol(p, &endptr, 0);
    if (*endptr != '\0') {
        r = -2;
        goto error;
    }
    v = (T) n;
    if (v < min_value) {
        r = -3;
        goto error;
    }
    if (v > max_value) {
        r = -4;
        goto error;
    }
    *var = v;
    goto done;
error:
    if (arg_fatal != nullptr)
        e_optval(arg_fatal);
done:
    return r;
}

template <class T, T default_value, T min_value, T max_value>
static int getoptvar(OptVar<T, default_value, min_value, max_value> *var, const char *arg_fatal) {
    T v = default_value;
    int r = getoptvar(&v, min_value, max_value, arg_fatal);
    if (r == 0)
        *var = v;
    return r;
}

static int do_option(int optc, const char *arg) {
    int i = 0;

    switch (optc) {
#if 0
    // FIXME: to_stdout doesn't work because of console code mess
    //case 'c':
    case 517:
        opt->to_stdout = true;
        break;
#endif
    case 'd':
        set_cmd(CMD_DECOMPRESS);
        break;
    case 'D':
        opt->debug.debug_level++;
        break;
    case 'f':
        opt->force++;
        break;
    case 909:
        set_cmd(CMD_FILEINFO);
        break;
    case 'h':
    case 'H':
    case '?':
        set_cmd(CMD_HELP);
        break;
    case 'h' + 256:
#if 1
        if (!acc_isatty(STDOUT_FILENO)) {
            /* according to GNU standards */
            set_term(stdout);
            opt->console = CON_FILE;
        }
#endif
        show_help(1);
        e_exit(EXIT_OK);
        break;
    case 'i':
        opt->info_mode++;
        break;
    case 'l':
        set_cmd(CMD_LIST);
        break;
    case 'L':
        set_cmd(CMD_LICENSE);
        break;
    case 'o':
        set_output_name(mfx_optarg, 1);
        break;
    case 'q':
        opt->verbose = (opt->verbose > 1 ? 1 : opt->verbose - 1);
        break;
    case 't':
        set_cmd(CMD_TEST);
        break;
    case 'v':
        opt->verbose = (opt->verbose < 3 ? 3 : opt->verbose + 1);
        break;
    case 'V':
        set_cmd(CMD_VERSION);
        break;
    case 'V' + 256:
    case 998:
        /* according to GNU standards */
        set_term(stdout);
        opt->console = CON_FILE;
        show_version(optc == 998 ? true : false);
        e_exit(EXIT_OK);
        break;

    // method
    case 702:
        opt->method_nrv2b_seen = true;
        if (!set_method(M_NRV2B_LE32, -1))
            e_method(M_NRV2B_LE32, opt->level);
        break;
    case 704:
        opt->method_nrv2d_seen = true;
        if (!set_method(M_NRV2D_LE32, -1))
            e_method(M_NRV2D_LE32, opt->level);
        break;
    case 705:
        opt->method_nrv2e_seen = true;
        if (!set_method(M_NRV2E_LE32, -1))
            e_method(M_NRV2E_LE32, opt->level);
        break;
    case 721:
        opt->method_lzma_seen = true;
        opt->all_methods_use_lzma = 1;
        if (!set_method(M_LZMA, -1))
            e_method(M_LZMA, opt->level);
        break;
    case 722:
        opt->method_lzma_seen = false;
        opt->all_methods_use_lzma = -1; // explicitly disabled
        if (M_IS_LZMA(opt->method))
            opt->method = -1;
        break;
    case 723:
        opt->prefer_ucl = false;
        break;
    case 724:
        opt->prefer_ucl = true;
        break;

    // compression level
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        if (!set_method(-1, optc - '0'))
            e_method(opt->method, optc);
        break;

    case 902: // --ultra-brute
        opt->ultra_brute = true;
        /* fallthrough */
    case 901: // --brute, much like --all-methods --all-filters --best
        opt->all_methods = true;
        if (opt->all_methods_use_lzma != -1)
            opt->all_methods_use_lzma = 1;
        opt->method = -1;
        opt->all_filters = true;
        opt->filter = -1;
        opt->crp.crp_ucl.m_size = 999999;
        /* fallthrough */
    case 900: // --best
        if (!set_method(-1, 10))
            e_method(opt->method, 10);
        break;

    // debug
    case 542:
        if (!mfx_optarg || strlen(mfx_optarg) != 4)
            e_optarg(arg);
        memcpy(opt->debug.fake_stub_version, mfx_optarg, 4);
        break;
    case 543:
        if (!mfx_optarg || strlen(mfx_optarg) != 4)
            e_optarg(arg);
        memcpy(opt->debug.fake_stub_year, mfx_optarg, 4);
        break;
    case 544:
        if (!mfx_optarg || !mfx_optarg[0])
            e_optarg(arg);
        opt->debug.dump_stub_loader = mfx_optarg;
        break;
    case 545:
        opt->debug.disable_random_id = true;
        break;

    // misc
    case 512:
        opt->console = CON_FILE;
        break;
    case 513:
        opt->console = CON_ANSI_MONO;
        break;
    case 514:
        opt->console = CON_ANSI_COLOR;
        break;
    case 516:
        opt->no_progress = true;
        break;
    case 519:
        opt->no_env = true;
        break;
    case 526:
        opt->preserve_mode = false;
        break;
    case 527:
        opt->preserve_ownership = false;
        break;
    case 528:
        opt->preserve_timestamp = false;
        break;
    // compression settings
    case 520: // --small
        if (opt->small < 0)
            opt->small = 0;
        opt->small++;
        break;
    case 521: // --filter=
        getoptvar(&opt->filter, 0, 255, arg);
        opt->all_filters = false;
        break;
    case 522: // --no-filter
        opt->filter = 0;
        opt->all_filters = false;
        opt->no_filter = true;
        break;
    case 523: // --all-filters, also see --brute above
        opt->all_filters = true;
        opt->filter = -1;
        break;
    case 524: // --all-methods, also see --brute above
        opt->all_methods = true;
        if (opt->all_methods_use_lzma != -1)
            opt->all_methods_use_lzma = 1;
        opt->method = -1;
        break;
    case 525: // --exact
        opt->exact = true;
        break;
    // compression runtime parameters
    case 801:
        getoptvar(&opt->crp.crp_ucl.c_flags, 0, 3, arg);
        break;
    case 802:
        getoptvar(&opt->crp.crp_ucl.s_level, 0, 2, arg);
        break;
    case 803:
        getoptvar(&opt->crp.crp_ucl.h_level, 0, 1, arg);
        break;
    case 804:
        getoptvar(&opt->crp.crp_ucl.p_level, 0, 7, arg);
        break;
    case 805:
        getoptvar(&opt->crp.crp_ucl.max_offset, 256u, ~0u, arg);
        break;
    case 806:
        getoptvar(&opt->crp.crp_ucl.max_match, 16u, ~0u, arg);
        break;
    case 807:
        getoptvar(&opt->crp.crp_ucl.m_size, 10000u, 999999u, arg);
        break;
    case 811:
        getoptvar(&opt->crp.crp_lzma.pos_bits, arg);
        break;
    case 812:
        getoptvar(&opt->crp.crp_lzma.lit_pos_bits, arg);
        break;
    case 813:
        getoptvar(&opt->crp.crp_lzma.lit_context_bits, arg);
        break;
    case 814:
        getoptvar(&opt->crp.crp_lzma.dict_size, arg);
        break;
    case 816:
        getoptvar(&opt->crp.crp_lzma.num_fast_bytes, arg);
        break;
    case 821:
        getoptvar(&opt->crp.crp_zlib.mem_level, arg);
        break;
    case 822:
        getoptvar(&opt->crp.crp_zlib.window_bits, arg);
        break;
    case 823:
        getoptvar(&opt->crp.crp_zlib.strategy, arg);
        break;
    // backup
    case 'k':
        opt->backup = 1;
        break;
    case 541:
        if (opt->backup != 1) // do not overide '--backup'
            opt->backup = 0;
        break;
    // overlay
    case 551:
        if (mfx_optarg && strcmp(mfx_optarg, "skip") == 0)
            opt->overlay = opt->SKIP_OVERLAY;
        else if (mfx_optarg && strcmp(mfx_optarg, "copy") == 0)
            opt->overlay = opt->COPY_OVERLAY;
        else if (mfx_optarg && strcmp(mfx_optarg, "strip") == 0)
            opt->overlay = opt->STRIP_OVERLAY;
        else
            e_optarg(arg);
        break;
    case 552:
        opt->overlay = opt->SKIP_OVERLAY;
        break;
    case 553:
        opt->overlay = opt->COPY_OVERLAY;
        break;
    case 554:
        opt->overlay = opt->STRIP_OVERLAY;
        break;
    // CPU
    case 560:
        if (mfx_optarg && strcmp(mfx_optarg, "8086") == 0)
            opt->cpu = opt->CPU_8086;
        else if (mfx_optarg && strcmp(mfx_optarg, "386") == 0)
            opt->cpu = opt->CPU_386;
        else if (mfx_optarg && strcmp(mfx_optarg, "486") == 0)
            opt->cpu = opt->CPU_486;
        else
            e_optarg(arg);
        break;
    case 561:
        opt->cpu = opt->CPU_8086;
        break;
    case 563:
        opt->cpu = opt->CPU_386;
        break;
    case 564:
        opt->cpu = opt->CPU_486;
        break;
    //
    case 600:
        opt->dos_exe.force_stub = true;
        break;
    case 601:
        opt->dos_exe.no_reloc = true;
        break;
    case 610:
        opt->djgpp2_coff.coff = true;
        break;
    case 620:
        opt->watcom_le.le = true;
        break;
    case 630:
        opt->win32_pe.compress_exports = 1;
        if (mfx_optarg && mfx_optarg[0])
            getoptvar(&opt->win32_pe.compress_exports, 0, 1, arg);
        // printf("compress_exports: %d\n", opt->win32_pe.compress_exports);
        break;
    case 631:
        opt->win32_pe.compress_icons = 1;
        if (mfx_optarg && mfx_optarg[0])
            getoptvar(&opt->win32_pe.compress_icons, 0, 3, arg);
        // printf("compress_icons: %d\n", opt->win32_pe.compress_icons);
        break;
    case 632:
        opt->win32_pe.compress_resources = 1;
        if (mfx_optarg && mfx_optarg[0])
            getoptvar(&opt->win32_pe.compress_resources, 0, 1, arg);
        // printf("compress_resources: %d\n", opt->win32_pe.compress_resources);
        break;
    case 633:
        // opt->win32_pe.strip_loadconf - OBSOLETE - IGNORED
        break;
    case 634:
        opt->win32_pe.strip_relocs = 1;
        if (mfx_optarg && mfx_optarg[0])
            getoptvar(&opt->win32_pe.strip_relocs, 0, 1, arg);
        // printf("strip_relocs: %d\n", opt->win32_pe.strip_relocs);
        break;
    case 635:
        if (!mfx_optarg || !mfx_optarg[0])
            e_optarg(arg);
        opt->win32_pe.keep_resource = mfx_optarg;
        break;
    case 650:
        opt->atari_tos.split_segments = true;
        break;
    case 660:
        getoptvar(&opt->o_unix.blocksize, 8192u, ~0u, arg);
        break;
    case 661:
        opt->o_unix.force_execve = true;
        break;
    case 663:
        opt->o_unix.is_ptinterp = true;
        break;
    case 664:
        opt->o_unix.use_ptinterp = true;
        break;
    case 665:
        opt->o_unix.make_ptinterp = true;
        break;
    case 666: // Linux
        opt->o_unix.osabi0 = Elf32_Ehdr::ELFOSABI_LINUX;
        break;
    case 667: // FreeBSD
        opt->o_unix.osabi0 = Elf32_Ehdr::ELFOSABI_FREEBSD;
        break;
    case 668: // NetBSD
        opt->o_unix.osabi0 = Elf32_Ehdr::ELFOSABI_NETBSD;
        break;
    case 669: // OpenBSD
        opt->o_unix.osabi0 = Elf32_Ehdr::ELFOSABI_OPENBSD;
        break;
    case 670:
        opt->ps1_exe.boot_only = true;
        break;
    case 671:
        opt->ps1_exe.no_align = true;
        opt->ps1_exe.boot_only = false;
        break;
    case 672:
        opt->ps1_exe.do_8bit = true;
        break;
    case 673:
        opt->ps1_exe.do_8mib = false;
        break;
    case 674:
        opt->o_unix.unmap_all_pages = true; // val ?
        break;
    case 675:
        opt->o_unix.preserve_build_id = true;
        break;
    case 676:
        opt->o_unix.android_shlib = true;
        break;
    case 677:
        opt->o_unix.force_pie = true;
        break;

#if !defined(DOCTEST_CONFIG_DISABLE)
    case 999: // doctest --dt-XXX option
        break;
#endif

    case '\0':
        return -1;
    case ':':
        return -2;
    default:
        fprintf(stderr, "%s: internal error in getopt (%d)\n", argv0, optc);
        return -3;
    }

    UNUSED(i);
    return 0;
}

int main_get_options(int argc, char **argv) {
    constexpr int *N = nullptr;

    static const struct mfx_option longopts[] = {
        // commands
        {"best", 0x10, N, 900},        // compress best
        {"brute", 0x10, N, 901},       // compress best, brute force
        {"ultra-brute", 0x10, N, 902}, // compress best, brute force
        {"decompress", 0, N, 'd'},     // decompress
        {"fast", 0x10, N, '1'},        // compress faster
        {"fileinfo", 0x10, N, 909},    // display info about file
        {"file-info", 0x10, N, 909},   // display info about file
        {"help", 0, N, 'h' + 256},     // give help
        {"license", 0, N, 'L'},        // display software license
        {"list", 0, N, 'l'},           // list compressed exe
        {"test", 0, N, 't'},           // test compressed file integrity
        {"uncompress", 0, N, 'd'},     // decompress
        {"version", 0, N, 'V' + 256},  // display version number

        // options
        {"force", 0, N, 'f'},          // force overwrite of output files
        {"force-compress", 0, N, 'f'}, //   and compression of suspicious files
        {"info", 0, N, 'i'},           // info mode
        {"no-env", 0x10, N, 519},      // no environment var
        {"no-mode", 0x10, N, 526},     // do not preserve mode (permissions)
        {"no-owner", 0x10, N, 527},    // do not preserve ownership
        {"no-progress", 0, N, 516},    // no progress bar
        {"no-time", 0x10, N, 528},     // do not preserve timestamp
        {"output", 0x21, N, 'o'},
        {"quiet", 0, N, 'q'},  // quiet mode
        {"silent", 0, N, 'q'}, // quiet mode
#if 0
        // FIXME: to_stdout doesn't work because of console code mess
        {"stdout",           0x10, N, 517},     // write output on standard output
        {"to-stdout",        0x10, N, 517},     // write output on standard output
#endif
        {"verbose", 0, N, 'v'}, // verbose mode

        // debug options
        {"debug", 0x10, N, 'D'},
        {"dump-stub-loader", 0x31, N, 544},  // for internal debugging
        {"fake-stub-version", 0x31, N, 542}, // for internal debugging
        {"fake-stub-year", 0x31, N, 543},    // for internal debugging
        {"disable-random-id", 0x10, N, 545}, // for internal debugging

        // backup options
        {"backup", 0x10, N, 'k'},
        {"keep", 0x10, N, 'k'},
        {"no-backup", 0x10, N, 541},

        // overlay options
        {"overlay", 0x31, N, 551}, // --overlay=
        {"skip-overlay", 0x10, N, 552},
        {"no-overlay", 0x10, N, 552}, // old name
        {"copy-overlay", 0x10, N, 553},
        {"strip-overlay", 0x10, N, 554},

        // CPU options
        {"cpu", 0x31, N, 560}, // --cpu=
        {"8086", 0x10, N, 561},
        {"386", 0x10, N, 563},
        {"486", 0x10, N, 564},

        // color options
        {"no-color", 0x10, N, 512},
        {"mono", 0x10, N, 513},
        {"color", 0x10, N, 514},

        // compression method
        {"nrv2b", 0x10, N, 702},   // --nrv2b
        {"nrv2d", 0x10, N, 704},   // --nrv2d
        {"nrv2e", 0x10, N, 705},   // --nrv2e
        {"lzma", 0x10, N, 721},    // --lzma
        {"no-lzma", 0x10, N, 722}, // disable all_methods_use_lzma
        {"prefer-nrv", 0x10, N, 723},
        {"prefer-ucl", 0x10, N, 724},
        // compression settings
        {"all-filters", 0x10, N, 523},
        {"all-methods", 0x10, N, 524},
        {"exact", 0x10, N, 525},  // user requires byte-identical decompression
        {"filter", 0x31, N, 521}, // --filter=
        {"no-filter", 0x10, N, 522},
        {"small", 0x10, N, 520},
        // compression runtime parameters
        {"crp-nrv-cf", 0x31, N, 801},
        {"crp-nrv-sl", 0x31, N, 802},
        {"crp-nrv-hl", 0x31, N, 803},
        {"crp-nrv-pl", 0x31, N, 804},
        {"crp-nrv-mo", 0x31, N, 805},
        {"crp-nrv-mm", 0x31, N, 806},
        {"crp-nrv-ms", 0x31, N, 807},
        {"crp-ucl-cf", 0x31, N, 801},
        {"crp-ucl-sl", 0x31, N, 802},
        {"crp-ucl-hl", 0x31, N, 803},
        {"crp-ucl-pl", 0x31, N, 804},
        {"crp-ucl-mo", 0x31, N, 805},
        {"crp-ucl-mm", 0x31, N, 806},
        {"crp-ucl-ms", 0x31, N, 807},
        {"crp-lzma-pb", 0x31, N, 811},
        {"crp-lzma-lp", 0x31, N, 812},
        {"crp-lzma-lc", 0x31, N, 813},
        {"crp-lzma-ds", 0x31, N, 814},
        {"crp-lzma-fb", 0x31, N, 816},
        {"crp-zlib-ml", 0x31, N, 821},
        {"crp-zlib-wb", 0x31, N, 822},
        {"crp-zlib-st", 0x31, N, 823},

        // atari/tos
        {"split-segments", 0x10, N, 650},
        // djgpp2/coff
        {"coff", 0x10, N, 610},          // produce COFF output
                                         // dos/com
                                         // dos/exe
                                         //{"force-stub",             0x10, 0, 600},
        {"no-reloc", 0x10, N, 601},      // no reloc. record into packer dos/exe
                                         // dos/sys
                                         // unix
        {"blocksize", 0x31, N, 660},     // --blocksize=
        {"force-execve", 0x10, N, 661},  // force linux/386 execve format
        {"is_ptinterp", 0x10, N, 663},   // linux/elf386 PT_INTERP program
        {"use_ptinterp", 0x10, N, 664},  // linux/elf386 PT_INTERP program
        {"make_ptinterp", 0x10, N, 665}, // linux/elf386 PT_INTERP program
        {"Linux", 0x10, N, 666},
        {"linux", 0x10, N, 666},
        {"FreeBSD", 0x10, N, 667},
        {"freebsd", 0x10, N, 667},
        {"NetBSD", 0x10, N, 668},
        {"netbsd", 0x10, N, 668},
        {"OpenBSD", 0x10, N, 669},
        {"openbsd", 0x10, N, 669},
        {"unmap-all-pages", 0x10, N, 674}, // linux /proc/self/exe vanishes
        {"preserve-build-id", 0, N, 675},
        {"android-shlib", 0, N, 676},
        {"force-pie", 0, N, 677},
        // watcom/le
        {"le", 0x10, N, 620}, // produce LE output
                              // win32/pe
        {"compress-exports", 2, N, 630},
        {"compress-icons", 2, N, 631},
        {"compress-resources", 2, N, 632},
        {"strip-loadconf", 0x12, N, 633}, // OBSOLETE - IGNORED
        {"strip-relocs", 0x12, N, 634},
        {"keep-resource", 0x31, N, 635},
        // ps1/exe
        {"boot-only", 0x10, N, 670},
        {"no-align", 0x10, N, 671},
        {"8-bit", 0x10, N, 672},
        {"8mib-ram", 0x10, N, 673},
        {"8mb-ram", 0x10, N, 673},

#if !defined(DOCTEST_CONFIG_DISABLE)
        // [doctest] Query flags - the program quits after them. Available:
        {"dt-c", 0x10, N, 999},
        {"dt-count", 0x10, N, 999},
        {"dt-h", 0x10, N, 999},
        {"dt-help", 0x10, N, 999},
        {"dt-lr", 0x10, N, 999},
        {"dt-list-reporters", 0x10, N, 999},
        {"dt-ltc", 0x10, N, 999},
        {"dt-list-test-cases", 0x10, N, 999},
        {"dt-lts", 0x10, N, 999},
        {"dt-list-test-suites", 0x10, N, 999},
        {"dt-v", 0x10, N, 999},
        {"dt-version", 0x10, N, 999},
        // [doctest] Bool options - can be used like flags and true is assumed. Available:
        {"dt-d", 0x12, N, 999},
        {"dt-duration", 0x12, N, 999},
        {"dt-e", 0x12, N, 999},
        {"dt-exit", 0x12, N, 999},
        {"dt-m", 0x12, N, 999},
        {"dt-minimal", 0x12, N, 999},
        {"dt-nt", 0x12, N, 999},
        {"dt-no-throw", 0x12, N, 999},
        {"dt-nr", 0x12, N, 999},
        {"dt-no-run", 0x12, N, 999},
        {"dt-s", 0x12, N, 999},
        {"dt-success", 0x12, N, 999},
#endif

        {nullptr, 0, nullptr, 0}
    };

    int optc, longind;
    char shortopts[256];

    prepare_shortopts(shortopts, "123456789hH?V", longopts),
        acc_getopt_init(&mfx_getopt, 1, argc, argv);
    mfx_getopt.progname = progname;
    mfx_getopt.opterr = handle_opterr;
    opt->o_unix.osabi0 = Elf32_Ehdr::ELFOSABI_LINUX;
    while ((optc = acc_getopt(&mfx_getopt, shortopts, longopts, &longind)) >= 0) {
        if (do_option(optc, argv[mfx_optind - 1]) != 0)
            e_usage();
    }

    return mfx_optind;
}

void main_get_envoptions() {
#if defined(OPTIONS_VAR)
    constexpr int *N = nullptr;

    /* only some options are allowed in the environment variable */

    static const struct mfx_option longopts[] = {
        // commands
        {"best", 0x10, N, 900},        // compress best
        {"brute", 0x10, N, 901},       // compress best, brute force
        {"ultra-brute", 0x10, N, 902}, // compress best, brute force
        {"fast", 0x10, N, '1'},        // compress faster

        // options
        {"info", 0, N, 'i'},        // info mode
        {"no-progress", 0, N, 516}, // no progress bar
        {"quiet", 0, N, 'q'},       // quiet mode
        {"silent", 0, N, 'q'},      // quiet mode
        {"verbose", 0, N, 'v'},     // verbose mode

        // debug options
        {"disable-random-id", 0x10, N, 545}, // for internal debugging

        // backup options
        {"backup", 0x10, N, 'k'},
        {"keep", 0x10, N, 'k'},
        {"no-backup", 0x10, N, 541},

        // overlay options
        {"overlay", 0x31, N, 551}, // --overlay=
        {"skip-overlay", 0x10, N, 552},
        {"no-overlay", 0x10, N, 552}, // old name
        {"copy-overlay", 0x10, N, 553},
        {"strip-overlay", 0x10, N, 554},

        // CPU options
        {"cpu", 0x31, N, 560}, // --cpu=
        {"8086", 0x10, N, 561},
        {"386", 0x10, N, 563},
        {"486", 0x10, N, 564},

        // color options
        {"no-color", 0x10, N, 512},
        {"mono", 0x10, N, 513},
        {"color", 0x10, N, 514},

        // compression settings
        {"exact", 0x10, N, 525}, // user requires byte-identical decompression

        // compression method
        {"nrv2b", 0x10, N, 702},   // --nrv2b
        {"nrv2d", 0x10, N, 704},   // --nrv2d
        {"nrv2e", 0x10, N, 705},   // --nrv2e
        {"lzma", 0x10, N, 721},    // --lzma
        {"no-lzma", 0x10, N, 722}, // disable all_methods_use_lzma
        {"prefer-nrv", 0x10, N, 723},
        {"prefer-ucl", 0x10, N, 724},
        // compression settings
        // compression runtime parameters

        // win32/pe
        {"compress-exports", 2, N, 630},
        {"compress-icons", 2, N, 631},
        {"compress-resources", 2, N, 632},
        {"strip-loadconf", 0x12, N, 633}, // OBSOLETE - IGNORED
        {"strip-relocs", 0x12, N, 634},
        {"keep-resource", 0x31, N, 635},

        {nullptr, 0, nullptr, 0}};

    char *env, *p;
    const char *var;
    int i, optc, longind;
    int targc;
    const char **targv = nullptr;
    static const char sep[] = " \t";
    char shortopts[256];

    var = getenv(OPTIONS_VAR);
    if (var == nullptr || !var[0])
        return;
    env = strdup(var);
    if (env == nullptr)
        return;

    /* count arguments */
    for (p = env, targc = 1;;) {
        while (*p && strchr(sep, *p))
            p++;
        if (*p == '\0')
            break;
        targc++;
        while (*p && !strchr(sep, *p))
            p++;
        if (*p == '\0')
            break;
        p++;
    }

    /* alloc temp argv */
    if (targc > 1)
        targv = (const char **) calloc(targc + 1, sizeof(char *));
    if (targv == nullptr) {
        free(env);
        return;
    }

    /* fill temp argv */
    targv[0] = argv0;
    for (p = env, targc = 1;;) {
        while (*p && strchr(sep, *p))
            p++;
        if (*p == '\0')
            break;
        targv[targc++] = p;
        while (*p && !strchr(sep, *p))
            p++;
        if (*p == '\0')
            break;
        *p++ = '\0';
    }
    targv[targc] = nullptr;

    /* check that only options are in temp argv */
    for (i = 1; i < targc; i++)
        if (targv[i][0] != '-' || !targv[i][1] || strcmp(targv[i], "--") == 0)
            e_envopt(targv[i]);

    /* handle options */
    prepare_shortopts(shortopts, "123456789", longopts);
    acc_getopt_init(&mfx_getopt, 1, targc, const_cast<char **>(targv));
    mfx_getopt.progname = progname;
    mfx_getopt.opterr = handle_opterr;
    while ((optc = acc_getopt(&mfx_getopt, shortopts, longopts, &longind)) >= 0) {
        if (do_option(optc, targv[mfx_optind - 1]) != 0)
            e_envopt(nullptr);
    }

    if (mfx_optind < targc)
        e_envopt(targv[mfx_optind]);

    /* clean up */
    free(targv);
    free(env);
#endif /* defined(OPTIONS_VAR) */
}

static void first_options(int argc, char **argv) {
    int i;
    int n = argc;

    for (i = 1; i < n; i++) {
        if (strcmp(argv[i], "--") == 0) {
            n = i;
            break;
        }
        if (strcmp(argv[i], "--version") == 0)
            do_option('V' + 256, argv[i]);
        if (strcmp(argv[i], "--version-short") == 0)
            do_option(998, argv[i]);
    }
    for (i = 1; i < n; i++)
        if (strcmp(argv[i], "--help") == 0)
            do_option('h' + 256, argv[i]);
    for (i = 1; i < n; i++)
        if (strcmp(argv[i], "--no-env") == 0)
            do_option(519, argv[i]);
}

/*************************************************************************
// main entry point
**************************************************************************/

int upx_main(int argc, char *argv[]) {
    int i;
    static char default_argv0[] = "upx";
    assert(argc >= 1); // sanity check
    if (!argv[0] || !argv[0][0])
        argv[0] = default_argv0;
    argv0 = argv[0];

    upx_compiler_sanity_check();
    int dt_res = upx_doctest_check(argc, argv);
    if (dt_res != 0) {
        if (dt_res == 2)
            fprintf(stderr, "%s: doctest requested program exit; Stop.\n", argv0);
        else
            fprintf(stderr, "%s: internal error: doctest failed\n", argv0);
        e_exit(EXIT_INIT);
    }

    // Allow serial re-use of upx_main() as a subroutine
    opt->reset();

#if (ACC_OS_CYGWIN || ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_EMX || ACC_OS_TOS || ACC_OS_WIN16 ||  \
     ACC_OS_WIN32 || ACC_OS_WIN64)
    {
        char *prog = fn_basename(argv0);
        char *p;
        bool allupper = true;
        for (p = prog; *p; p++)
            if (islower((unsigned char) *p))
                allupper = false;
        if (allupper)
            fn_strlwr(prog);
        if (p - prog > 4) {
            p -= 4;
            if (fn_strcmp(p, ".exe") == 0 || fn_strcmp(p, ".ttp") == 0)
                *p = 0;
        }
        progname = prog;
    }
#else
    progname = fn_basename(argv0);
#endif
    while (progname[0] == '.' && progname[1] == '/' && progname[2])
        progname += 2;

    set_term(stderr);

    assert(upx_lzma_init() == 0);
    assert(upx_ucl_init() == 0);
    assert(upx_zlib_init() == 0);
#if (WITH_NRV)
    assert(upx_nrv_init() == 0);
#endif

    /* get options */
    first_options(argc, argv);
    if (!opt->no_env)
        main_get_envoptions();
    i = main_get_options(argc, argv);
    assert(i <= argc);

    set_term(nullptr);
    switch (opt->cmd) {
    case CMD_NONE:
        /* default - compress */
        set_cmd(CMD_COMPRESS);
        break;
    case CMD_COMPRESS:
        break;
    case CMD_DECOMPRESS:
        break;
    case CMD_TEST:
        break;
    case CMD_LIST:
        break;
    case CMD_FILEINFO:
        break;
    case CMD_LICENSE:
        show_license();
        e_exit(EXIT_OK);
        break;
    case CMD_HELP:
        show_help(1);
        e_exit(EXIT_OK);
        break;
    case CMD_VERSION:
        show_version();
        e_exit(EXIT_OK);
        break;
    default:
        /* ??? */
        break;
    }

    /* check options */
    if (argc == 1)
        e_help();
    set_term(stderr);
    check_options(i, argc);
    int num_files = argc - i;
    if (num_files < 1) {
        if (opt->verbose >= 2)
            e_help();
        else
            e_usage();
    }

    /* start work */
    set_term(stdout);
    if (do_files(i, argc, argv) != 0)
        return exit_code;

    if (gitrev[0]) {
        // also see UPX_CONFIG_DISABLE_GITREV in CMakeLists.txt
        bool warn_gitrev = true;
        const char *ee = getenv("UPX_DEBUG_DISABLE_GITREV_WARNING");
        if (ee && ee[0] && strcmp(ee, "1") == 0)
            warn_gitrev = false;
        if (warn_gitrev) {
            FILE *f = stdout;
            int fg = con_fg(f, FG_RED);
            con_fprintf(
                f, "\nWARNING: this is an unstable beta version - use for testing only! Really.\n");
            fg = con_fg(f, fg);
            UNUSED(fg);
        }
    }

    return exit_code;
}

/*************************************************************************
// real entry point
**************************************************************************/

#if !(WITH_GUI)

#if 1 && (ACC_OS_DOS32) && defined(__DJGPP__)
#include <crt0.h>
int _crt0_startup_flags = _CRT0_FLAG_UNIX_SBRK;
#endif
#if (ACC_ARCH_M68K && ACC_OS_TOS && ACC_CC_GNUC) && defined(__MINT__)
extern "C" {
extern long _stksize;
long _stksize = 256 * 1024L;
}
#endif
#if (ACC_OS_WIN32 || ACC_OS_WIN64) && (defined(__MINGW32__) || defined(__MINGW64__))
extern "C" {
extern int _dowildcard;
int _dowildcard = -1;
}
#endif

int __acc_cdecl_main main(int argc, char *argv[]) {
#if 0 && (ACC_OS_DOS32) && defined(__DJGPP__)
    // LFN=n may cause problems with 2.03's _rename and mkdir under WinME
    putenv("LFN=y");
#endif
#if (ACC_OS_WIN32 || ACC_OS_WIN64) && (ACC_CC_MSC) && defined(_WRITE_ABORT_MSG) &&                 \
    defined(_CALL_REPORTFAULT)
    _set_abort_behavior(_WRITE_ABORT_MSG, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
    acc_wildargv(&argc, &argv);
    // srand((int) time(nullptr));
    srand((int) clock());

    int r = upx_main(argc, argv);

#if 0 && defined(__GLIBC__)
    //malloc_stats();
#endif
    return r;
}

#endif /* !(WITH_GUI) */

/* vim:set ts=4 sw=4 et: */
