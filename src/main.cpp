/* main.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2000 Laszlo Molnar
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

   Markus F.X.J. Oberhumer                   Laszlo Molnar
   markus.oberhumer@jk.uni-linz.ac.at        ml1050@cdata.tvnet.hu
 */


#include "conf.h"
#include "mygetopt.h"
#include "file.h"


/*************************************************************************
// options
**************************************************************************/

void init_options(struct options_t *o)
{
    memset(o, 0, sizeof(*o));
    memset(&o->crp, 0xff, sizeof(o->crp));

    o->cmd = CMD_NONE;
    o->method = -1;
    o->level = 7;
    o->mem_level = -1;
    o->filter = -1;

    o->backup = -1;
    o->overlay = -1;

#if defined(__MFX_DOS) || defined(__MFX_WIN)
    o->console = CON_INIT;
#else
    o->console = CON_FILE;
#endif
    o->verbose = 2;

    o->w32pe.compress_exports = 1;
    o->w32pe.compress_icons = 2;
    o->w32pe.compress_resources = true;
    o->w32pe.strip_relocs = -1;
}

static struct options_t global_options;
struct options_t * volatile opt = &global_options;

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
static void do_exit(void) __attribute__((noreturn));
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
    fprintf(stderr,"%s: invalid argument in option `%s'\n", argv0, n);
    e_exit(EXIT_USAGE);
}


#if defined(OPTIONS_VAR)
void e_envopt(const char *n)
{
    fflush(con_term);
    if (n)
        fprintf(stderr,"%s: invalid string `%s' in environment variable `%s'\n",
                        argv0, n, OPTIONS_VAR);
    else
        fprintf(stderr,"%s: illegal option in environment variable `%s'\n",
                        argv0, OPTIONS_VAR);
    e_exit(EXIT_USAGE);
}
#endif /* defined(OPTIONS_VAR) */


RETSIGTYPE SIGTYPEENTRY e_sighandler(int signum)
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
        fprintf(stderr,"cannot use both `%s' and `%s'\n", c1, c2);
        e_usage();
    }
}


void check_options(int i, int argc)
{
    assert(i <= argc);

    // set default overlay action
    if (!(opt->cmd == CMD_COMPRESS || opt->cmd == CMD_DECOMPRESS))
        opt->overlay = opt->COPY_OVERLAY;
    else if (opt->overlay < 0)
        opt->overlay = opt->COPY_OVERLAY;

    // set default backup option
    if (opt->backup < 0)
        opt->backup = 0;
    if (!(opt->cmd == CMD_COMPRESS || opt->cmd == CMD_DECOMPRESS))
        opt->backup = 1;

    check_not_both(opt->to_stdout, opt->output_name != NULL, "--stdout", "-o");
    if (opt->to_stdout || opt->output_name)
    {
        if (i + 1 != argc)
        {
            fprintf(stderr,"%s: need exactly one argument when using `%s'\n",
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
        con_term = isafile(STDIN_FILENO) ? stderr : stdout;
}


static void set_cmd(int cmd)
{
    if (cmd > opt->cmd)
        opt->cmd = cmd;
}


static bool set_method(int m, int l)
{
    if (m > 0)
        opt->method = m;
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
        fprintf(stderr,"%s: option `-o' more than once given\n",argv0);
        e_usage();
    }
#endif
    if (!n || !n[0] || (!allow_m && n[0] == '-'))
    {
        fprintf(stderr,"%s: missing output name\n",argv0);
        e_usage();
    }
    if (strlen(n) >= PATH_MAX - 4)
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
        fprintf(stderr,"%s: option `--script' more than once given\n",argv0);
        e_usage();
    }
#endif
    if (!n || !n[0] || (!allow_m && n[0] == '-'))
    {
        fprintf(stderr,"%s: missing script name\n",argv0);
        e_usage();
    }
    if (strlen(n) >= opt->unix.SCRIPT_MAX - 3)
    {
        fprintf(stderr,"%s: script name too long\n",argv0);
        e_usage();
    }
    opt->unix.script_name = n;
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
int getoptvar(T *var, T minval, T maxval)
{
    const char *p = mfx_optarg;
    char *endptr;

    if (!p || !p[0])
        return -1;
    // avoid interpretation as octal value
    while (p[0] == '0' && isdigit(p[1]))
        p++;
    long n = strtol(p, &endptr, 0);
    if (*endptr != '\0')
        return -2;
    T v = (T) n;
    if (v < minval)
        return -3;
    if (v > maxval)
        return -4;
    *var = v;
    return 0;
}


static int do_option(int optc, const char *arg)
{
    int i = 0;

    switch (optc)
    {
    //case 'c':
    case 517:
        opt->to_stdout = true;
        break;
    case 'd':
        set_cmd(CMD_DECOMPRESS);
        break;
    case 'D':
        opt->debug++;
        break;
    case 'f':
        opt->force++;
        break;
    case 901:
        set_cmd(CMD_FILEINFO);
        break;
    case 'h':
    case 'H':
    case '?':
        set_cmd(CMD_HELP);
        break;
    case 'h'+256:
        /* according to GNU standards */
        set_term(stdout);
        opt->console = CON_FILE;
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
        if (!set_method(M_NRV2B_LE32, -1))
            e_method(M_NRV2B_LE32, opt->level);
        break;
    case 704:
        if (!set_method(M_NRV2D_LE32, -1))
            e_method(M_NRV2D_LE32, opt->level);
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
    case 900:
        if (!set_method(-1, 10))
            e_method(opt->method, 10);
        break;

    // compression memory level
    case 902:
        getoptvar(&opt->mem_level, 1, 9);
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
    // compression settings
    case 525:
        opt->small = 1;
        break;
    case 526:
        getoptvar(&opt->filter, 0, 255);
        opt->all_filters = false;
        break;
    case 527:
        opt->all_filters = true;
        opt->filter = -1;
        break;
    // compression parms
    case 531:
        getoptvar(&opt->crp.c_flags, 0, 3);
        break;
    case 532:
        getoptvar(&opt->crp.s_level, 0, 2);
        break;
    case 533:
        getoptvar(&opt->crp.h_level, 0, 1);
        break;
    case 534:
        getoptvar(&opt->crp.p_level, 0, 7);
        break;
    case 535:
        getoptvar(&opt->crp.max_offset, 256u, ~0u);
        break;
    case 536:
        getoptvar(&opt->crp.max_match, 16u, ~0u);
        break;
    case 537:
        getoptvar(&opt->crp.m_size, 1024u, 512*1024u);
        break;
    // backup
    case 'k':
        opt->backup = 1;
        break;
    case 541:
        if (opt->backup != 1)           // do not overide `--backup'
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
        opt->dos.force_stub = true;
        break;
    case 601:
        opt->dos.no_reloc = true;
        break;
    case 610:
        opt->djgpp2.coff = true;
        break;
    case 620:
        opt->wcle.le = true;
        break;
    case 630:
        opt->w32pe.compress_exports = 1;
        getoptvar(&opt->w32pe.compress_exports, 0, 1);
        //printf("compress_exports: %d\n", opt->w32pe.compress_exports);
        break;
    case 631:
        opt->w32pe.compress_icons = 1;
        getoptvar(&opt->w32pe.compress_icons, 0, 2);
        //printf("compress_icons: %d\n", opt->w32pe.compress_icons);
        break;
    case 632:
        opt->w32pe.compress_resources = true;
        if (mfx_optarg && strcmp(mfx_optarg,"0") == 0)
            opt->w32pe.compress_resources = false;
        //printf("compress_resources: %d\n", opt->w32pe.compress_resources);
        break;
    case 633:
        opt->w32pe.strip_relocs = 1;
        if (mfx_optarg && strcmp(mfx_optarg,"0") == 0)
            opt->w32pe.strip_relocs = 0;
        //printf("strip_relocs: %d\n", opt->w32pe.strip_relocs);
        break;
    case 650:
        opt->tos.split_segments = true;
        break;
    case 660:
        getoptvar(&opt->unix.blocksize, 8192u, ~0u);
        break;
    case 661:
        opt->unix.script_name = "/usr/local/lib/upx/upxX";
        if (mfx_optarg && mfx_optarg[0])
            set_script_name(mfx_optarg,1);
        break;

    case '\0':
        return -1;
    case ':':
        return -2;
    default:
        fprintf(stderr,"%s: internal error in getopt (%d)\n",argv0,optc);
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
    {"best",                0, 0, 900},     // compress best
    {"decompress",          0, 0, 'd'},     // decompress
    {"fast",                0, 0, '1'},     // compress faster
    {"fileinfo",         0x10, 0, 901},     // display info about file
    {"help",                0, 0, 'h'+256}, // give help
    {"license",             0, 0, 'L'},     // display software license
    {"list",                0, 0, 'l'},     // list compressed exe
    {"test",                0, 0, 't'},     // test compressed file integrity
    {"uncompress",          0, 0, 'd'},     // decompress
    {"version",             0, 0, 'V'+256}, // display version number

    // options
    {"debug",            0x10, 0, 'D'},
    {"force",               0, 0, 'f'},     // force overwrite of output files
    {"force-compress",      0, 0, 'f'},     //   and compression of suspicious files
    {"info",                0, 0, 'i'},     // info mode
    {"no-env",           0x10, 0, 519},     // no environment var
    {"no-progress",         0, 0, 516},     // no progress bar
    {"output",           0x21, 0, 'o'},
    {"quiet",               0, 0, 'q'},     // quiet mode
    {"silent",              0, 0, 'q'},     // quiet mode
    {"stdout",           0x10, 0, 517},     // write output on standard output
    {"to-stdout",        0x10, 0, 517},     // write output on standard output
    {"verbose",             0, 0, 'v'},     // verbose mode

    // backup options
    {"backup",              0, 0, 'k'},
    {"keep",                0, 0, 'k'},
    {"no-backup",           0, 0, 541},

    // overlay options
    {"overlay",          0x31, 0, 551},     // --overlay=
    {"skip-overlay",        0, 0, 552},
    {"no-overlay",          0, 0, 552},     // old name
    {"copy-overlay",        0, 0, 553},
    {"strip-overlay",       0, 0, 554},

    // CPU options
    {"cpu",              0x31, 0, 560},     // --cpu=
    {"8086",             0x10, 0, 561},
    {"386",              0x10, 0, 563},
    {"486",              0x10, 0, 564},

    // color options
    {"no-color",            0, 0, 512},
    {"mono",                0, 0, 513},
    {"color",               0, 0, 514},

    // compression method
    {"2b",               0x10, 0, 702},     // --2b
    {"n2b",              0x10, 0, 702},     // --n2b
    {"nrv2b",            0x10, 0, 702},     // --nrv2b
    {"2d",               0x10, 0, 704},     // --2d
    {"n2d",              0x10, 0, 704},     // --n2d
    {"nrv2d",            0x10, 0, 704},     // --nrv2d
    // compression settings
    {"all-filters",      0x10, 0, 527},
    {"filter",           0x31, 0, 526},     // --filter=
    {"mem",              0x31, 0, 902},     // --mem=
    {"small",            0x10, 0, 525},
    // compression runtime parameters
    {"crp-cf",           0x31, 0, 531},
    {"crp-sl",           0x31, 0, 532},
    {"crp-hl",           0x31, 0, 533},
    {"crp-pl",           0x31, 0, 534},
    {"crp-mo",           0x31, 0, 535},
    {"crp-mm",           0x31, 0, 536},
    {"crp-ms",           0x31, 0, 537},

    // atari/tos
    {"split-segments",   0x10, 0, 650},
    // djgpp2/coff
    {"coff",                0, 0, 610},     // produce COFF output
    // dos/com
    // dos/exe
    //{"force-stub",             0x10, 0, 600},
    {"no-reloc",         0x10, 0, 601},     // no reloc. record into packer dos/exe
    // dos/sys
    // unix
    {"blocksize",        0x31, 0, 660},     // --blocksize=
    {"script",           0x31, 0, 661},     // --script=
    // watcom/le
    {"le",                  0, 0, 620},     // produce LE output
    // win32/pe
    {"compress-exports",    2, 0, 630},
    {"compress-icons",      2, 0, 631},
    {"compress-resources",  2, 0, 632},
    {"strip-relocs",        2, 0, 633},

    { 0, 0, 0, 0 }
};

    int optc, longind;
    char buf[256];

    prepare_shortopts(buf,"123456789hH?V",longopts),
    mfx_optind = 0;
    mfx_opterr = 1;
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
    {"best",                0, 0, 900},     // compress best
    {"fast",                0, 0, '1'},     // compress faster

    // options
    {"info",                0, 0, 'i'},     // info mode
    {"no-progress",         0, 0, 516},     // no progress bar
    {"quiet",               0, 0, 'q'},     // quiet mode
    {"silent",              0, 0, 'q'},     // quiet mode
    {"verbose",             0, 0, 'v'},     // verbose mode

    // backup options
    {"backup",              0, 0, 'k'},
    {"keep",                0, 0, 'k'},
    {"no-backup",        0x10, 0, 541},

    // overlay options
    {"overlay",          0x31, 0, 551},     // --overlay=
    {"skip-overlay",        0, 0, 552},
    {"no-overlay",          0, 0, 552},     // old name
    {"copy-overlay",        0, 0, 553},
    {"strip-overlay",       0, 0, 554},

    // CPU options
    {"cpu",              0x31, 0, 560},     // --cpu=
    {"8086",             0x10, 0, 561},
    {"386",              0x10, 0, 563},
    {"486",              0x10, 0, 564},

    // color options
    {"no-color",            0, 0, 512},
    {"mono",                0, 0, 513},
    {"color",               0, 0, 514},

    // win32/pe
    {"compress-exports",    2, 0, 630},
    {"compress-icons",      2, 0, 631},
    {"compress-resources",  2, 0, 632},
    {"strip-relocs",        2, 0, 633},

    { 0, 0, 0, 0 }
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

    for (i = 1; i < argc; i++)
        if (strcmp(argv[i],"--version") == 0)
            do_option('V'+256, argv[i]);
    for (i = 1; i < argc; i++)
        if (strcmp(argv[i],"--help") == 0)
            do_option('h'+256, argv[i]);
    for (i = 1; i < argc; i++)
        if (strcmp(argv[i],"--no-env") == 0)
            do_option(519, argv[i]);
}


/*************************************************************************
// assert a sane architecture
**************************************************************************/

void upx_sanity_check(void)
{
    assert(sizeof(char) == 1);
    assert(sizeof(short) == 2);
    assert(sizeof(int) == 4);
    assert(sizeof(long) >= 4);
    assert(sizeof(void *) >= 4);
    assert(sizeof(long) >= sizeof(void *));
    assert(((off_t) -1) < 0);

    assert(sizeof(BE16) == 2);
    assert(sizeof(BE32) == 4);
    assert(sizeof(LE16) == 2);
    assert(sizeof(LE32) == 4);

    struct align_assertion_1a_t
    {
        struct foo_t {
            char c1;
            LE16 v[4];
        } d[3];
    };
    struct align_assertion_1b_t
    {
        struct foo_t {
            char c1;
            char v[4*2];
        } d[3];
    };
    struct align_assertion_2a_t
    {
        struct foo_t {
            char c1;
            LE32 v[4];
        } d[3];
    };
    struct align_assertion_2b_t
    {
        struct foo_t {
            char c1;
            char v[4*4];
        } d[3];
    };
    //printf("%d\n", (int) sizeof(align_assertion_1a_t));
    //printf("%d\n", (int) sizeof(align_assertion_1b_t));
    //printf("%d\n", (int) sizeof(align_assertion_2a_t));
    //printf("%d\n", (int) sizeof(align_assertion_2b_t));
    assert(sizeof(align_assertion_1a_t) == sizeof(align_assertion_1b_t));
    assert(sizeof(align_assertion_2a_t) == sizeof(align_assertion_2b_t));
    assert(sizeof(align_assertion_1a_t) == 3*9);
    assert(sizeof(align_assertion_2a_t) == 3*17);
}


/*************************************************************************
// main entry point
**************************************************************************/

#if !defined(WITH_GUI)

int main(int argc, char *argv[])
{
    int i;
    static char default_argv0[] = "upx";
    int cmdline_cmd = CMD_NONE;

    upx_sanity_check();
    init_options(opt);

#if defined(WITH_MSS)
    MSS_DISABLE_LOG_OUTPUT;
#endif

#if defined(__EMX__)
    _response(&argc,&argv);
    _wildcard(&argc,&argv);
#endif

    if (!argv[0] || !argv[0][0])
        argv[0] = default_argv0;
    argv0 = argv[0];
#if defined(DOSISH)
    {
        char *prog = fn_basename(argv0);
        char *p;
        bool allupper = 1;
        for (p = prog; *p; p++)
            if (islower((unsigned char)*p))
                allupper = 0;
        if (allupper)
            fn_strlwr(prog);
        if (strlen(prog) > 4)
        {
            p = prog + strlen(prog) - 4;
            if (fn_strcmp(p, ".exe") == 0 || fn_strcmp(p, ".ttp") == 0)
                *p = 0;
        }
        progname = prog;
    }
#else
    progname = fn_basename(argv0);
#endif

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
    if (nrv_init() != NRV_E_OK)
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

    set_term(0);
    cmdline_cmd = opt->cmd;
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

    if (opt->cmd != CMD_COMPRESS)
    {
        // invalidate compression options
        opt->method = 0;
        opt->level = 0;
        opt->mem_level = 0;
        memset(&opt->crp, 0xff, sizeof(opt->crp));
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

#if 0 && defined(__GLIBC__)
    //malloc_stats();
#endif
    do_exit();
    return exit_code;
}

#endif /* !defined(WITH_GUI) */


/*
vi:ts=4:et:nowrap
*/

