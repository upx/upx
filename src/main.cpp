/* main.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
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
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
 */


#include "conf.h"
#include "mygetopt.h"
#include "file.h"
#include "packer.h"
#include "p_elf.h"


#if 1 && defined(__DJGPP__)
#include <crt0.h>
int _crt0_startup_flags = _CRT0_FLAG_UNIX_SBRK;
#endif


/*************************************************************************
// options
**************************************************************************/

void options_t::reset()
{
    options_t *o = this;
    memset(o, 0, sizeof(*o));
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
#if defined(__DJGPP__)
    o->console = CON_INIT;
#elif defined(USE_SCREEN_WIN32)
    o->console = CON_INIT;
#elif 1 && defined(__linux__)
    o->console = CON_INIT;
#endif
    o->verbose = 2;

    o->win32_pe.compress_exports = 1;
    o->win32_pe.compress_icons = 2;
    o->win32_pe.compress_resources = -1;
    for (unsigned i = 0; i < TABLESIZE(o->win32_pe.compress_rt); i++)
        o->win32_pe.compress_rt[i] = -1;
    o->win32_pe.compress_rt[24] = false;    // 24 == RT_MANIFEST
    o->win32_pe.strip_relocs = -1;
    o->win32_pe.keep_resource = "";
}

static options_t global_options;
options_t *opt = &global_options;

static int done_output_name = 0;
static int done_script_name = 0;


const char *argv0 = "";
const char *progname = "";


static int num_files = -1;
static int exit_code = EXIT_OK;


/*************************************************************************
// exit handlers
**************************************************************************/

#if defined(__GNUC__)
static void do_exit(void) __attribute__((__noreturn__));
#endif
static void do_exit(void)
{
    static bool in_exit = false;

    if (in_exit)
        exit(exit_code);
    in_exit = true;

    fflush(con_term);
    fflush(stderr);
    exit(exit_code);
}


#define EXIT_FATAL  3

static bool set_eec(int ec, int *eec)
{
    if (ec == EXIT_FATAL)
    {
        *eec = EXIT_ERROR;
        return 1;
    }
    else if (ec < 0 || ec == EXIT_ERROR)
    {
        *eec = EXIT_ERROR;
    }
    else if (ec == EXIT_WARN)
    {
        if (!opt->ignorewarn)
            if (*eec == EXIT_OK)
                *eec = ec;
    }
    else if (ec == EXIT_OK)
    {
        /* do nothing */
    }
    else
    {
        assert(0);
    }
    return 0;
}

bool set_ec(int ec)
{
    return set_eec(ec,&exit_code);
}


void e_exit(int ec)
{
    (void) set_ec(ec);
    do_exit();
}


void e_usage(void)
{
    show_usage();
    e_exit(EXIT_USAGE);
}


void e_memory(void)
{
    show_head();
    fflush(con_term);
    fprintf(stderr,"%s: out of memory\n", argv0);
    e_exit(EXIT_MEMORY);
}


static void e_method(int m, int l)
{
    fflush(con_term);
    fprintf(stderr,"%s: illegal method option -- %d/%d\n", argv0, m, l);
    e_usage();
}


static void e_optarg(const char *n)
{
    fflush(con_term);
    fprintf(stderr,"%s: invalid argument in option '%s'\n", argv0, n);
    e_exit(EXIT_USAGE);
}


static void e_optval(const char *n)
{
    fflush(con_term);
    fprintf(stderr,"%s: invalid value for option '%s'\n", argv0, n);
    e_exit(EXIT_USAGE);
}


#if defined(OPTIONS_VAR)
void e_envopt(const char *n)
{
    fflush(con_term);
    if (n)
        fprintf(stderr,"%s: invalid string '%s' in environment variable '%s'\n",
                        argv0, n, OPTIONS_VAR);
    else
        fprintf(stderr,"%s: illegal option in environment variable '%s'\n",
                        argv0, OPTIONS_VAR);
    e_exit(EXIT_USAGE);
}
#endif /* defined(OPTIONS_VAR) */


void __acc_cdecl_sighandler e_sighandler(int signum)
{
    UNUSED(signum);
    e_exit(EXIT_FATAL);
}


/*************************************************************************
// check options
**************************************************************************/

void check_not_both(bool e1, bool e2, const char *c1, const char *c2)
{
    if (e1 && e2)
    {
        fprintf(stderr,"%s: ",argv0);
        fprintf(stderr,"cannot use both '%s' and '%s'\n", c1, c2);
        e_usage();
    }
}


void check_options(int i, int argc)
{
    assert(i <= argc);

    if (opt->cmd != CMD_COMPRESS)
    {
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

    check_not_both(opt->to_stdout, opt->output_name != NULL, "--stdout", "-o");
    if (opt->to_stdout && opt->cmd == CMD_COMPRESS)
    {
        fprintf(stderr,"%s: cannot use '--stdout' when compressing\n", argv0);
        e_usage();
    }
    if (opt->to_stdout || opt->output_name)
    {
        if (i + 1 != argc)
        {
            fprintf(stderr,"%s: need exactly one argument when using '%s'\n",
                    argv0, opt->to_stdout ? "--stdout" : "-o");
            e_usage();
        }
    }
}


/*************************************************************************
// misc
**************************************************************************/

void e_help(void)
{
    show_help();
    e_exit(EXIT_USAGE);
}


static void set_term(FILE *f)
{
    if (f)
        con_term = f;
    else
        con_term = acc_isatty(STDIN_FILENO) ? stderr : stdout;
}


static void set_cmd(int cmd)
{
    if (cmd > opt->cmd)
        opt->cmd = cmd;
}


static bool set_method(int m, int l)
{
    if (m > 0)
    {
        if (!Packer::isValidCompressionMethod(m))
            return false;
#if 1
        // something like "--brute --lzma" should not disable "--brute"
        if (!opt->all_methods)
#endif
        {
            opt->method = m;
            opt->all_methods = false;
        }
    }
    if (l > 0)
        opt->level = l;
    set_cmd(CMD_COMPRESS);
    return true;
}


static void set_output_name(const char *n, bool allow_m)
{
#if 1
    if (done_output_name > 0)
    {
        fprintf(stderr,"%s: option '-o' more than once given\n",argv0);
        e_usage();
    }
#endif
    if (!n || !n[0] || (!allow_m && n[0] == '-'))
    {
        fprintf(stderr,"%s: missing output name\n",argv0);
        e_usage();
    }
    if (strlen(n) >= ACC_FN_PATH_MAX - 4)
    {
        fprintf(stderr,"%s: output name too long\n",argv0);
        e_usage();
    }
    opt->output_name = n;
    done_output_name++;
}

static void set_script_name(const char *n, bool allow_m)
{
#if 1
    if (done_script_name > 0)
    {
        fprintf(stderr,"%s: option '--script' more than once given\n",argv0);
        e_usage();
    }
#endif
    if (!n || !n[0] || (!allow_m && n[0] == '-'))
    {
        fprintf(stderr,"%s: missing script name\n",argv0);
        e_usage();
    }
    if (strlen(n) >= (size_t)opt->o_unix.SCRIPT_MAX - 3)
    {
        fprintf(stderr,"%s: script name too long\n",argv0);
        e_usage();
    }
    opt->o_unix.script_name = n;
    done_script_name++;
}


/*************************************************************************
// get options
**************************************************************************/

static
char* prepare_shortopts(char *buf, const char *n,
                        const struct mfx_option *longopts)
{
    char *o = buf;

    for ( ; n && *n; n++)
        if (*n != ' ')
            *o++ = *n;
    *o = 0;
    for ( ; longopts && longopts->name; longopts++)
    {
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
            if (vopts[v] && strchr(buf,v) == NULL)
                printf("warning: duplicate option %d ('%c')!\n", v, v & 127);
            vopts[v] = 1;
        }
#endif
        if (v > 0 && v < 256 && strchr(buf,v) == NULL)
        {
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
int getoptvar(T *var, const T min_value, const T max_value, const char *arg_fatal)
{
    const char *p = mfx_optarg;
    char *endptr;
    int r = 0;
    long n;
    T v;

    if (!p || !p[0])
        { r = -1; goto error; }
    // avoid interpretation as octal value
    while (p[0] == '0' && isdigit(p[1]))
        p++;
    n = strtol(p, &endptr, 0);
    if (*endptr != '\0')
        { r = -2; goto error; }
    v = (T) n;
    if (v < min_value)
        { r = -3; goto error; }
    if (v > max_value)
        { r = -4; goto error; }
    *var = v;
    goto done;
error:
    if (arg_fatal != NULL)
        e_optval(arg_fatal);
done:
    return r;
}

#if 1 && (ACC_CC_GNUC >= 0x030300)
template <class T, T default_value, T min_value, T max_value>
int getoptvar(OptVar<T,default_value,min_value,max_value> *var, const char *arg_fatal)
{
    T v = default_value;
    int r = getoptvar(&v, min_value, max_value, arg_fatal);
    if (r == 0)
        *var = v;
    return r;
}
#else
template <class T>
int getoptvar(T *var, const char *arg_fatal)
{
    typename T::Type v = T::default_value_c;
    int r = getoptvar(&v, T::min_value_c, T::max_value_c, arg_fatal);
    if (r == 0)
        *var = v;
    return r;
}
#endif


static int do_option(int optc, const char *arg)
{
    int i = 0;

    switch (optc)
    {
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
    case 'h'+256:
#if 1
        if (!acc_isatty(STDOUT_FILENO))
        {
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
        set_output_name(mfx_optarg,1);
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
    case 'V'+256:
        /* according to GNU standards */
        set_term(stdout);
        opt->console = CON_FILE;
        show_version(0);
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
        opt->all_methods_use_lzma = true;
        if (!set_method(M_LZMA, -1))
            e_method(M_LZMA, opt->level);
        break;
    case 722:
        opt->method_lzma_seen = false;
        opt->all_methods_use_lzma = false;
        if (M_IS_LZMA(opt->method))
            opt->method = -1;
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

    case 902:                               // --ultra-brute
        opt->ultra_brute = true;
        /* fallthrough */
    case 901:                               // --brute
        opt->all_methods = true;
        opt->all_methods_use_lzma = true;
        opt->method = -1;
        opt->all_filters = true;
        opt->filter = -1;
        opt->crp.crp_ucl.m_size = 999999;
        /* fallthrough */
    case 900:                               // --best
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

    // mp (meta)
    case 501:
        getoptvar(&opt->mp_compress_task, 1, 999999, arg);
        break;
    case 502:
        opt->mp_query_format = true;
        break;
    case 503:
        opt->mp_query_num_tasks = true;
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
    case 520:                               // --small
        if (opt->small < 0)
            opt->small = 0;
        opt->small++;
        break;
    case 521:                               // --filter=
        getoptvar(&opt->filter, 0, 255, arg);
        opt->all_filters = false;
        break;
    case 522:                               // --no-filter
        opt->filter = 0;
        opt->all_filters = false;
        opt->no_filter = true;
        break;
    case 523:                               // --all-filters
        opt->all_filters = true;
        opt->filter = -1;
        break;
    case 524:                               // --all-methods
        opt->all_methods = true;
        opt->all_methods_use_lzma = true;
        opt->method = -1;
        break;
    case 525:                               // --exact
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
        if (opt->backup != 1)           // do not overide '--backup'
            opt->backup = 0;
        break;
    // overlay
    case 551:
        if (mfx_optarg && strcmp(mfx_optarg,"skip") == 0)
            opt->overlay = opt->SKIP_OVERLAY;
        else if (mfx_optarg && strcmp(mfx_optarg,"copy") == 0)
            opt->overlay = opt->COPY_OVERLAY;
        else if (mfx_optarg && strcmp(mfx_optarg,"strip") == 0)
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
        if (mfx_optarg && strcmp(mfx_optarg,"8086") == 0)
            opt->cpu = opt->CPU_8086;
        else if (mfx_optarg && strcmp(mfx_optarg,"386") == 0)
            opt->cpu = opt->CPU_386;
        else if (mfx_optarg && strcmp(mfx_optarg,"486") == 0)
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
        //printf("compress_exports: %d\n", opt->win32_pe.compress_exports);
        break;
    case 631:
        opt->win32_pe.compress_icons = 1;
        if (mfx_optarg && mfx_optarg[0])
            getoptvar(&opt->win32_pe.compress_icons, 0, 3, arg);
        //printf("compress_icons: %d\n", opt->win32_pe.compress_icons);
        break;
    case 632:
        opt->win32_pe.compress_resources = 1;
        if (mfx_optarg && mfx_optarg[0])
            getoptvar(&opt->win32_pe.compress_resources, 0, 1, arg);
        //printf("compress_resources: %d\n", opt->win32_pe.compress_resources);
        break;
    case 633:
        // opt->win32_pe.strip_loadconf - OBSOLETE - IGNORED
        break;
    case 634:
        opt->win32_pe.strip_relocs = 1;
        if (mfx_optarg && mfx_optarg[0])
            getoptvar(&opt->win32_pe.strip_relocs, 0, 1, arg);
        //printf("strip_relocs: %d\n", opt->win32_pe.strip_relocs);
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
    case 662:
        opt->o_unix.script_name = "/usr/local/lib/upx/upxX";
        if (mfx_optarg && mfx_optarg[0])
            set_script_name(mfx_optarg, 1);
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
    case 666:  // Linux
        opt->o_unix.osabi0 = Elf32_Ehdr::ELFOSABI_LINUX;
        break;
    case 667:  // FreeBSD
        opt->o_unix.osabi0 = Elf32_Ehdr::ELFOSABI_FREEBSD;
        break;
    case 668:  // NetBSD
        opt->o_unix.osabi0 = Elf32_Ehdr::ELFOSABI_NETBSD;
        break;
    case 669:  // OpenBSD
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

    case '\0':
        return -1;
    case ':':
        return -2;
    default:
        fprintf(stderr,"%s: internal error in getopt (%d)\n", argv0, optc);
        return -3;
    }

    UNUSED(i);
    return 0;
}


static int get_options(int argc, char **argv)
{

static const struct mfx_option longopts[] =
{
    // commands
    {"best",             0x10, 0, 900},     // compress best
    {"brute",            0x10, 0, 901},     // compress best, brute force
    {"ultra-brute",      0x10, 0, 902},     // compress best, brute force
    {"decompress",          0, 0, 'd'},     // decompress
    {"fast",             0x10, 0, '1'},     // compress faster
    {"fileinfo",         0x10, 0, 909},     // display info about file
    {"file-info",        0x10, 0, 909},     // display info about file
    {"help",                0, 0, 'h'+256}, // give help
    {"license",             0, 0, 'L'},     // display software license
    {"list",                0, 0, 'l'},     // list compressed exe
    {"test",                0, 0, 't'},     // test compressed file integrity
    {"uncompress",          0, 0, 'd'},     // decompress
    {"version",             0, 0, 'V'+256}, // display version number

    // options
    {"force",               0, 0, 'f'},     // force overwrite of output files
    {"force-compress",      0, 0, 'f'},     //   and compression of suspicious files
    {"info",                0, 0, 'i'},     // info mode
    {"no-env",           0x10, 0, 519},     // no environment var
    {"no-mode",          0x10, 0, 526},     // do not preserve mode (permissions)
    {"no-owner",         0x10, 0, 527},     // do not preserve ownership
    {"no-progress",         0, 0, 516},     // no progress bar
    {"no-time",          0x10, 0, 528},     // do not preserve timestamp
    {"output",           0x21, 0, 'o'},
    {"quiet",               0, 0, 'q'},     // quiet mode
    {"silent",              0, 0, 'q'},     // quiet mode
#if 0
    // FIXME: to_stdout doesn't work because of console code mess
    {"stdout",           0x10, 0, 517},     // write output on standard output
    {"to-stdout",        0x10, 0, 517},     // write output on standard output
#endif
    {"verbose",             0, 0, 'v'},     // verbose mode

    // debug options
    {"debug",            0x10, 0, 'D'},
    {"dump-stub-loader" ,0x31, 0, 544},     // for internal debugging
    {"fake-stub-version",0x31, 0, 542},     // for internal debugging
    {"fake-stub-year"   ,0x31, 0, 543},     // for internal debugging
    {"disable-random-id",0x10, 0, 545},     // for internal debugging

    // backup options
    {"backup",           0x10, 0, 'k'},
    {"keep",             0x10, 0, 'k'},
    {"no-backup",        0x10, 0, 541},

    // overlay options
    {"overlay",          0x31, 0, 551},     // --overlay=
    {"skip-overlay",     0x10, 0, 552},
    {"no-overlay",       0x10, 0, 552},     // old name
    {"copy-overlay",     0x10, 0, 553},
    {"strip-overlay",    0x10, 0, 554},

    // CPU options
    {"cpu",              0x31, 0, 560},     // --cpu=
    {"8086",             0x10, 0, 561},
    {"386",              0x10, 0, 563},
    {"486",              0x10, 0, 564},

    // color options
    {"no-color",         0x10, 0, 512},
    {"mono",             0x10, 0, 513},
    {"color",            0x10, 0, 514},

    // compression method
    {"nrv2b",            0x10, 0, 702},     // --nrv2b
    {"nrv2d",            0x10, 0, 704},     // --nrv2d
    {"nrv2e",            0x10, 0, 705},     // --nrv2e
    {"lzma",             0x10, 0, 721},     // --lzma
    {"no-lzma",          0x10, 0, 722},     // (disable all_methods_use_lzma)
    // compression settings
    {"all-filters",      0x10, 0, 523},
    {"all-methods",      0x10, 0, 524},
    {"exact",            0x10, 0, 525},     // user requires byte-identical decompression
    {"filter",           0x31, 0, 521},     // --filter=
    {"no-filter",        0x10, 0, 522},
    {"small",            0x10, 0, 520},
    // compression runtime parameters
    {"crp-nrv-cf",       0x31, 0, 801},
    {"crp-nrv-sl",       0x31, 0, 802},
    {"crp-nrv-hl",       0x31, 0, 803},
    {"crp-nrv-pl",       0x31, 0, 804},
    {"crp-nrv-mo",       0x31, 0, 805},
    {"crp-nrv-mm",       0x31, 0, 806},
    {"crp-nrv-ms",       0x31, 0, 807},
    {"crp-ucl-cf",       0x31, 0, 801},
    {"crp-ucl-sl",       0x31, 0, 802},
    {"crp-ucl-hl",       0x31, 0, 803},
    {"crp-ucl-pl",       0x31, 0, 804},
    {"crp-ucl-mo",       0x31, 0, 805},
    {"crp-ucl-mm",       0x31, 0, 806},
    {"crp-ucl-ms",       0x31, 0, 807},
    {"crp-lzma-pb",      0x31, 0, 811},
    {"crp-lzma-lp",      0x31, 0, 812},
    {"crp-lzma-lc",      0x31, 0, 813},
    {"crp-lzma-ds",      0x31, 0, 814},
    {"crp-lzma-fb",      0x31, 0, 816},
    {"crp-zlib-ml",      0x31, 0, 821},
    {"crp-zlib-wb",      0x31, 0, 822},
    {"crp-zlib-st",      0x31, 0, 823},
    // [deprecated - only for compatibility with UPX 2.0x]
    {"crp-ms",           0x31, 0, 807},

    // atari/tos
    {"split-segments",   0x10, 0, 650},
    // djgpp2/coff
    {"coff",             0x10, 0, 610},     // produce COFF output
    // dos/com
    // dos/exe
    //{"force-stub",             0x10, 0, 600},
    {"no-reloc",         0x10, 0, 601},     // no reloc. record into packer dos/exe
    // dos/sys
    // unix
    {"blocksize",        0x31, 0, 660},     // --blocksize=
    {"force-execve",     0x10, 0, 661},     // force linux/386 execve format
#if 0
    {"script",           0x31, 0, 662},     // --script=
#endif
    {"is_ptinterp",      0x10, 0, 663},     // linux/elf386 PT_INTERP program
    {"use_ptinterp",     0x10, 0, 664},     // linux/elf386 PT_INTERP program
    {"make_ptinterp",    0x10, 0, 665},     // linux/elf386 PT_INTERP program
    {"Linux",            0x10, 0, 666},
    {"linux",            0x10, 0, 666},
    {"FreeBSD",          0x10, 0, 667},
    {"freebsd",          0x10, 0, 667},
    {"NetBSD",           0x10, 0, 668},
    {"netbsd",           0x10, 0, 668},
    {"OpenBSD",          0x10, 0, 669},
    {"openbsd",          0x10, 0, 669},
    // watcom/le
    {"le",               0x10, 0, 620},     // produce LE output
    // win32/pe
    {"compress-exports",    2, 0, 630},
    {"compress-icons",      2, 0, 631},
    {"compress-resources",  2, 0, 632},
    {"strip-loadconf",   0x12, 0, 633},     // OBSOLETE - IGNORED
    {"strip-relocs",     0x12, 0, 634},
    {"keep-resource",    0x31, 0, 635},
    // ps1/exe
    {"boot-only",        0x10, 0, 670},
    {"no-align",         0x10, 0, 671},
    {"8-bit",            0x10, 0, 672},
    {"8mib-ram",         0x10, 0, 673},
    {"8mb-ram",          0x10, 0, 673},

    // mp (meta) options
    {"mp-compress-task",        0x31, 0, 501},
    {"mp-query-format",         0x10, 0, 502},
    {"mp-query-num-tasks",      0x10, 0, 503},

    { NULL, 0, NULL, 0 }
};

    int optc, longind;
    char buf[256];

    prepare_shortopts(buf,"123456789hH?V",longopts),
    mfx_optind = 0;
    mfx_opterr = 1;
    opt->o_unix.osabi0 = Elf32_Ehdr::ELFOSABI_LINUX;
    while ((optc = mfx_getopt_long(argc, argv, buf, longopts, &longind)) >= 0)
    {
        if (do_option(optc, argv[mfx_optind-1]) != 0)
            e_usage();
    }

    return mfx_optind;
}


#if defined(OPTIONS_VAR)
static void get_envoptions(int argc, char **argv)
{

/* only some options are allowed in the environment variable */

static const struct mfx_option longopts[] =
{
    // commands
    {"best",             0x10, 0, 900},     // compress best
    {"brute",            0x10, 0, 901},     // compress best, brute force
    {"ultra-brute",      0x10, 0, 902},     // compress best, brute force
    {"fast",             0x10, 0, '1'},     // compress faster

    // options
    {"info",                0, 0, 'i'},     // info mode
    {"no-progress",         0, 0, 516},     // no progress bar
    {"quiet",               0, 0, 'q'},     // quiet mode
    {"silent",              0, 0, 'q'},     // quiet mode
    {"verbose",             0, 0, 'v'},     // verbose mode

    // debug options
    {"disable-random-id",0x10, 0, 545},     // for internal debugging

    // backup options
    {"backup",           0x10, 0, 'k'},
    {"keep",             0x10, 0, 'k'},
    {"no-backup",        0x10, 0, 541},

    // overlay options
    {"overlay",          0x31, 0, 551},     // --overlay=
    {"skip-overlay",     0x10, 0, 552},
    {"no-overlay",       0x10, 0, 552},     // old name
    {"copy-overlay",     0x10, 0, 553},
    {"strip-overlay",    0x10, 0, 554},

    // CPU options
    {"cpu",              0x31, 0, 560},     // --cpu=
    {"8086",             0x10, 0, 561},
    {"386",              0x10, 0, 563},
    {"486",              0x10, 0, 564},

    // color options
    {"no-color",         0x10, 0, 512},
    {"mono",             0x10, 0, 513},
    {"color",            0x10, 0, 514},

    // compression settings
    {"exact",            0x10, 0, 525},     // user requires byte-identical decompression

    // compression method
    {"nrv2b",            0x10, 0, 702},     // --nrv2b
    {"nrv2d",            0x10, 0, 704},     // --nrv2d
    {"nrv2e",            0x10, 0, 705},     // --nrv2e
    {"lzma",             0x10, 0, 721},     // --lzma
    {"no-lzma",          0x10, 0, 722},     // (disable all_methods_use_lzma)
    // compression settings
    // compression runtime parameters

    // win32/pe
    {"compress-exports",    2, 0, 630},
    {"compress-icons",      2, 0, 631},
    {"compress-resources",  2, 0, 632},
    {"strip-loadconf",   0x12, 0, 633},     // OBSOLETE - IGNORED
    {"strip-relocs",     0x12, 0, 634},
    {"keep-resource",    0x31, 0, 635},

    { NULL, 0, NULL, 0 }
};

    char *env, *p;
    const char *var;
    int i, optc, longind;
    int targc;
    char **targv = NULL;
    static const char sep[] = " \t";
    char buf[256];

    var = getenv(OPTIONS_VAR);
    if (var == NULL || !var[0])
        return;
    env = strdup(var);
    if (env == NULL)
        return;

    /* count arguments */
    for (p = env, targc = 1; ; )
    {
        while (*p && strchr(sep,*p))
            p++;
        if (*p == '\0')
            break;
        targc++;
        while (*p && !strchr(sep,*p))
            p++;
        if (*p == '\0')
            break;
        p++;
    }

    /* alloc temp argv */
    if (targc > 1)
        targv = (char **) calloc(targc+1,sizeof(char *));
    if (targv == NULL)
    {
        free(env);
        return;
    }

    /* fill temp argv */
    targv[0] = argv[0];
    for (p = env, targc = 1; ; )
    {
        while (*p && strchr(sep,*p))
            p++;
        if (*p == '\0')
            break;
        targv[targc++] = p;
        while (*p && !strchr(sep,*p))
            p++;
        if (*p == '\0')
            break;
        *p++ = '\0';
    }
    targv[targc] = NULL;

    /* check that only options are in temp argv */
    for (i = 1; i < targc; i++)
        if (targv[i][0] != '-' || !targv[i][1] || strcmp(targv[i],"--") == 0)
            e_envopt(targv[i]);

    /* handle options */
    prepare_shortopts(buf,"123456789",longopts);
    mfx_optind = 0;
    mfx_opterr = 1;
    while ((optc = mfx_getopt_long(targc, targv, buf, longopts, &longind)) >= 0)
    {
        if (do_option(optc, targv[mfx_optind-1]) != 0)
            e_envopt(NULL);
    }

    if (mfx_optind < targc)
        e_envopt(targv[mfx_optind]);

    /* clean up */
    free(targv);
    free(env);
    UNUSED(argc);
}
#endif /* defined(OPTIONS_VAR) */


static void first_options(int argc, char **argv)
{
    int i;
    int n = argc;

    for (i = 1; i < n; i++)
    {
        if (strcmp(argv[i],"--") == 0)
        {
            n = i;
            break;
        }
        if (strcmp(argv[i],"--version") == 0)
            do_option('V'+256, argv[i]);
    }
    for (i = 1; i < n; i++)
        if (strcmp(argv[i],"--help") == 0)
            do_option('h'+256, argv[i]);
    for (i = 1; i < n; i++)
        if (strcmp(argv[i],"--no-env") == 0)
            do_option(519, argv[i]);
}


/*************************************************************************
// assert a sane architecture and compiler
**************************************************************************/

template <class T> struct TestBELE {
static bool test(void)
{
    COMPILE_TIME_ASSERT_ALIGNED1(T)
    struct test1_t { char a; T b; } __attribute_packed;
    struct test2_t { char a; T b[3]; } __attribute_packed;
    test1_t t1[7]; UNUSED(t1); test2_t t2[7]; UNUSED(t2);
    COMPILE_TIME_ASSERT(sizeof(test1_t) == 1 + sizeof(T))
    COMPILE_TIME_ASSERT_ALIGNED1(test1_t)
    COMPILE_TIME_ASSERT(sizeof(t1) == 7 + 7*sizeof(T))
    COMPILE_TIME_ASSERT(sizeof(test2_t) == 1 + 3*sizeof(T))
    COMPILE_TIME_ASSERT_ALIGNED1(test2_t)
    COMPILE_TIME_ASSERT(sizeof(t2) == 7 + 21*sizeof(T))
#if defined(__acc_alignof)
    COMPILE_TIME_ASSERT(__acc_alignof(t1) == 1)
    COMPILE_TIME_ASSERT(__acc_alignof(t2) == 1)
#endif
#if 1 && !defined(xUPX_OFFICIAL_BUILD)
    T allbits; allbits = 0; allbits -= 1;
    //++allbits; allbits++; --allbits; allbits--;
    T v1; v1 = 1; v1 *= 2; v1 -= 1;
    T v2; v2 = 1;
    assert( (v1 == v2)); assert(!(v1 != v2));
    assert( (v1 <= v2)); assert( (v1 >= v2));
    assert(!(v1 <  v2)); assert(!(v1 >  v2));
    v2 ^= allbits;
    assert(!(v1 == v2)); assert( (v1 != v2));
    assert( (v1 <= v2)); assert(!(v1 >= v2));
    assert( (v1 <  v2)); assert(!(v1 >  v2));
    v2 += 2;
    assert(v1 == 1); assert(v2 == 0);
    v1 <<= 1; v1 |= v2; v1 >>= 1; v2 &= v1; v2 /= v1; v2 *= v1;
    assert(v1 == 1); assert(v2 == 0);
    if ((v1 ^ v2) != 1) return false;
#endif
    return true;
}};


#define ACC_WANT_ACC_CHK_CH 1
#undef ACCCHK_ASSERT
#include "miniacc.h"

void upx_sanity_check(void)
{
#define ACC_WANT_ACC_CHK_CH 1
#undef ACCCHK_ASSERT
#define ACCCHK_ASSERT(expr)   ACC_COMPILE_TIME_ASSERT(expr)
#include "miniacc.h"
#undef ACCCHK_ASSERT

    COMPILE_TIME_ASSERT(sizeof(char) == 1)
    COMPILE_TIME_ASSERT(sizeof(short) == 2)
    COMPILE_TIME_ASSERT(sizeof(int) == 4)
    COMPILE_TIME_ASSERT(sizeof(long) >= 4)
    COMPILE_TIME_ASSERT(sizeof(void *) >= 4)

    COMPILE_TIME_ASSERT(sizeof(off_t) >= sizeof(long))
    COMPILE_TIME_ASSERT(((off_t) -1) < 0)

    COMPILE_TIME_ASSERT(sizeof(BE16) == 2)
    COMPILE_TIME_ASSERT(sizeof(BE32) == 4)
    COMPILE_TIME_ASSERT(sizeof(BE64) == 8)
    COMPILE_TIME_ASSERT(sizeof(LE16) == 2)
    COMPILE_TIME_ASSERT(sizeof(LE32) == 4)
    COMPILE_TIME_ASSERT(sizeof(LE64) == 8)

    COMPILE_TIME_ASSERT_ALIGNED1(BE16)
    COMPILE_TIME_ASSERT_ALIGNED1(BE32)
    COMPILE_TIME_ASSERT_ALIGNED1(BE64)
    COMPILE_TIME_ASSERT_ALIGNED1(LE16)
    COMPILE_TIME_ASSERT_ALIGNED1(LE32)
    COMPILE_TIME_ASSERT_ALIGNED1(LE64)

    COMPILE_TIME_ASSERT(sizeof(UPX_VERSION_STRING4) == 4 + 1)
    assert(strlen(UPX_VERSION_STRING4) == 4);
    assert(memcmp(UPX_VERSION_STRING4, UPX_VERSION_STRING, 4) == 0);
    COMPILE_TIME_ASSERT(sizeof(UPX_VERSION_YEAR) == 4 + 1)
    assert(strlen(UPX_VERSION_YEAR) == 4);
    assert(memcmp(UPX_VERSION_DATE_ISO, UPX_VERSION_YEAR, 4) == 0);
    assert(memcmp(UPX_VERSION_DATE + strlen(UPX_VERSION_DATE) - 4, UPX_VERSION_YEAR, 4) == 0);

#if 1
    assert(TestBELE<LE16>::test());
    assert(TestBELE<LE32>::test());
    assert(TestBELE<LE64>::test());
    assert(TestBELE<BE16>::test());
    assert(TestBELE<BE32>::test());
    assert(TestBELE<BE64>::test());
    {
    static const unsigned char dd[32] = { 0, 0, 0, 0, 0, 0, 0,
         0xff, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa, 0xf9, 0xf8,
         0, 0, 0, 0,
         0x7f, 0x7e, 0x7d, 0x7c, 0x7b, 0x7a, 0x79, 0x78,
    0, 0, 0, 0, 0 };
    const unsigned char *d;
    d = dd + 7;
    assert(upx_adler32(d, 4) == 0x09f003f7);
    assert(upx_adler32(d, 4, 0) == 0x09ec03f6);
    assert(upx_adler32(d, 4, 1) == 0x09f003f7);
    assert(get_be16(d) == 0xfffe);
    assert(get_be16_signed(d) == -2);
    assert(get_be24(d) == 0xfffefd);
    assert(get_be24_signed(d) == -259);
    assert(get_be32(d) == 0xfffefdfc);
    assert(get_be32_signed(d) == -66052);
    assert(get_le16(d) == 0xfeff);
    assert(get_le16_signed(d) == -257);
    assert(get_le24(d) == 0xfdfeff);
    assert(get_le24_signed(d) == -131329);
    assert(get_le32(d) == 0xfcfdfeff);
    assert(get_le32_signed(d) == -50462977);
    assert(get_le64_signed(d) == ACC_INT64_C(-506097522914230529));
    assert(find_be16(d, 2, 0xfffe) == 0);
    assert(find_le16(d, 2, 0xfeff) == 0);
    assert(find_be32(d, 4, 0xfffefdfc) == 0);
    assert(find_le32(d, 4, 0xfcfdfeff) == 0);
    d += 12;
    assert(get_be16_signed(d) == 32638);
    assert(get_be24_signed(d) == 8355453);
    assert(get_be32_signed(d) == 2138996092);
    assert(get_be64_signed(d) == ACC_INT64_C(9186918263483431288));
    }
#endif
}


/*************************************************************************
// main entry point
**************************************************************************/

#if !defined(WITH_GUI)

#if (ACC_ARCH_M68K && ACC_OS_TOS && ACC_CC_GNUC) && defined(__MINT__)
extern "C" { extern long _stksize; long _stksize = 256 * 1024L; }
#endif

int __acc_cdecl_main main(int argc, char *argv[])
{
    int i;
    static char default_argv0[] = "upx";
//    int cmdline_cmd = CMD_NONE;

#if 0 && defined(__DJGPP__)
    // LFN=n may cause problems with 2.03's _rename and mkdir under WinME
    putenv("LFN=y");
#endif
    acc_wildargv(&argc, &argv);

    upx_sanity_check();
    opt->reset();

    if (!argv[0] || !argv[0][0])
        argv[0] = default_argv0;
    argv0 = argv[0];
#if (ACC_OS_CYGWIN || ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_EMX || ACC_OS_TOS || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)
    {
        char *prog = fn_basename(argv0);
        char *p;
        bool allupper = true;
        for (p = prog; *p; p++)
            if (islower((unsigned char)*p))
                allupper = false;
        if (allupper)
            fn_strlwr(prog);
        if (p - prog > 4)
        {
            p -= 4;
            if (fn_strcmp(p, ".exe") == 0 || fn_strcmp(p, ".ttp") == 0)
                *p = 0;
        }
        progname = prog;
    }
#else
    progname = fn_basename(argv0);
#endif
    while (progname[0] == '.' && progname[1] == '/'  && progname[2])
        progname += 2;

    set_term(stderr);

#if defined(WITH_UCL)
    if (ucl_init() != UCL_E_OK)
    {
        show_head();
        fprintf(stderr,"ucl_init() failed - check your UCL installation !\n");
        if (UCL_VERSION != ucl_version())
            fprintf(stderr,"library version conflict (%lx, %lx) - check your UCL installation !\n",
                    (long) UCL_VERSION, (long) ucl_version());
        e_exit(EXIT_INIT);
    }
#endif
#if defined(WITH_NRV)
    if (nrv_init() != NRV_E_OK || NRV_VERSION != nrv_version())
    {
        show_head();
        fprintf(stderr,"nrv_init() failed - check your NRV installation !\n");
        if (NRV_VERSION != nrv_version())
            fprintf(stderr,"library version conflict (%lx, %lx) - check your NRV installation !\n",
                    (long) NRV_VERSION, (long) nrv_version());
        e_exit(EXIT_INIT);
    }
#endif

    //srand((int) time(NULL));
    srand((int) clock());

    /* get options */
    first_options(argc,argv);
#if defined(OPTIONS_VAR)
    if (!opt->no_env)
        get_envoptions(argc,argv);
#endif
    i = get_options(argc,argv);
    assert(i <= argc);

    set_term(NULL);
//    cmdline_cmd = opt->cmd;
    switch (opt->cmd)
    {
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
        show_version(1);
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
    check_options(i,argc);
    num_files = argc - i;
    if (num_files < 1)
    {
        if (opt->verbose >= 2)
            e_help();
        else
            e_usage();
    }

    /* start work */
    set_term(stdout);
    do_files(i,argc,argv);

#if 0 && (UPX_VERSION_HEX < 0x030000)
    {
        FILE *f = stdout;
        int fg = con_fg(f,FG_RED);
        con_fprintf(f,"\nWARNING: this is an unstable beta version - use for testing only! Really.\n");
        fg = con_fg(f,fg);
        UNUSED(fg);
    }
#endif

#if 0 && defined(__GLIBC__)
    //malloc_stats();
#endif
    return exit_code;
}

#endif /* !defined(WITH_GUI) */


/*
vi:ts=4:et:nowrap
*/

