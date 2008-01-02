/* c_init.cpp --

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

FILE *con_term = NULL;

#if defined(USE_CONSOLE)

/*************************************************************************
//
**************************************************************************/

static console_t * const me = &console_init;
console_t * con = &console_init;

int con_mode = CON_INIT;


static void try_init(console_t *c, FILE *f)
{
    int k;

    assert(c);
    assert(c->init);
    k = c->init(f,opt->console,con_mode);
    if (k == CON_INIT)
        return;
#if 0
    if (con_mode != CON_INIT && opt->console != CON_INIT)
        if (k != opt->console)
            return;
#endif
    if (k > con_mode)
    {
        con_mode = k;
        con = c;
        con->init = 0;
        if (!con->set_fg)
            con->set_fg = console_none.set_fg;
        if (!con->print0)
            con->print0 = console_none.print0;
        if (!con->intro)
            con->intro = console_none.intro;
    }
}


static int do_init(FILE *f)
{
    assert(con_mode == CON_INIT);

    try_init(&console_none,f);
    assert(con != me);
    assert(con == &console_none);
    if (opt->console == CON_NONE || opt->to_stdout)
        return con_mode;
    try_init(&console_file,f);
    if (!acc_isatty(STDIN_FILENO) || !acc_isatty(STDOUT_FILENO) || !acc_isatty(STDERR_FILENO))
        return con_mode;

#if defined(USE_ANSI)
    try_init(&console_ansi_mono,f);
    try_init(&console_ansi_color,f);
#endif
#if defined(USE_SCREEN)
    try_init(&console_screen,f);
#endif
#if defined(USE_AALIB)
    try_init(&console_aalib,f);
#endif

    return con_mode;
}


/*************************************************************************
//
**************************************************************************/

static int init(FILE *f, int o, int now)
{
    if (con != me)
        return con_mode;
    assert(o == -1);
    assert(now == -1);
    UNUSED(o);
    UNUSED(now);
    return do_init(f);
}


static int set_fg(FILE *f, int fg)
{
    if (con == me)
        init(f,-1,-1);
    assert(con != me);
    return con->set_fg(f,fg);
}


static bool intro(FILE *f)
{
    if (con == me)
        init(f,-1,-1);
    assert(con != me);
    return con->intro(f);
}


console_t console_init =
{
    init,
    set_fg,
    0,
    intro
};


void __acc_cdecl_va con_fprintf(FILE *f, const char *format, ...)
{
    va_list args;
    char buf[80*25];

    va_start(args, format);
    upx_vsnprintf(buf, sizeof(buf), format,args);
    va_end(args);

    if (con == me)
        init(f,-1,-1);
    assert(con != me);
    con->print0(f,buf);
}

#endif /* USE_CONSOLE */


/*
vi:ts=4:et
*/

