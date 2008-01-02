/* c_screen.cpp -- console screen driver

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

#if defined(USE_SCREEN)

#include "screen.h"

#define mask_fg 0x0f
#define mask_bg 0xf0


/*************************************************************************
//
**************************************************************************/

static int do_init(screen_t *s, int fd)
{
    int fg, bg;

    if (s->init(s,fd) != 0)
        return -1;

    if (s->getCols(s) < 80 || s->getCols(s) > 256)
        return -1;
    if (s->getRows(s) < 24)
        return -1;

    fg = s->getFg(s);
    bg = s->getBg(s);
    if (s->isMono(s))
        fg = -1;
    if (fg == (bg >> 4))
        return -1;
    if (bg != BG_BLACK)
        if (!s->isMono(s))
        {
            /* return 0; */     /* we could emulate ANSI mono */
            return -1;
        }

    return 0;
}


static screen_t *do_construct(screen_t *s, int fd)
{
    if (!s)
        return NULL;
    if (do_init(s,fd) != 0)
    {
        s->destroy(s);
        return NULL;
    }
    return s;
}


/*************************************************************************
//
**************************************************************************/

static screen_t *screen = NULL;

static void __acc_cdecl_atexit do_destroy(void)
{
    if (screen)
    {
        if (screen->atExit)
            screen->atExit();
        screen->destroy(screen);
        screen = NULL;
    }
}


static int mode = -1;
static int init_fg = -1;
static int init_bg = -1;
static int cur_fg = -1;
static int cur_bg = -1;


static int init(FILE *f, int o, int now)
{
    int fd = fileno(f);
    int n;

    UNUSED(now);
    assert(screen == NULL);

    if (o == CON_SCREEN)
        n = CON_SCREEN;
    else if (o == CON_INIT)                 /* use by default */
        n = CON_SCREEN;
    else if (o == CON_ANSI_COLOR)           /* can emulate ANSI color */
        n = CON_ANSI_COLOR;
    else if (o == CON_ANSI_MONO)            /* can emulate ANSI mono */
        n = CON_ANSI_MONO;
    else
        return CON_INIT;

#if defined(__DJGPP__)
    if (!screen)
        screen = do_construct(screen_djgpp2_construct(),fd);
#endif
#if defined(USE_SCREEN_WIN32)
    if (!screen)
        screen = do_construct(screen_win32_construct(),fd);
#endif
#if defined(USE_SCREEN_VCSA)
    if (!screen)
        screen = do_construct(screen_vcsa_construct(),fd);
#endif
#if defined(USE_SCREEN_CURSES)
    if (!screen && o == CON_SCREEN)
        screen = do_construct(screen_curses_construct(),fd);
#endif
    if (!screen)
        return CON_INIT;

    mode = screen->getMode(screen);
    init_fg = cur_fg = screen->getFg(screen);
    init_bg = cur_bg = screen->getBg(screen);
    if (screen->isMono(screen))
        cur_fg = -1;

    atexit(do_destroy);
    return n;
}


static int set_fg(FILE *f, int fg)
{
    const int last_fg = cur_fg;
    int f1 = fg & mask_fg;
    int f2 = init_fg & mask_fg;

    UNUSED(f);
    cur_fg = fg;
    if (screen->isMono(screen))
    {
        const int b = (init_bg & mask_bg) >> 4;
        if (fg == -1)           /* restore startup fg */
            f1 = f2;
        else if (b == 0)
            f1 = (f2 <= 8) ? 15 : 8;
        else if (b <= 8)
            f1 = (f2 == 0) ? 15 : 0;
        else
            f1 = (f2 == 0) ? 8 : 0;
    }
    else if (con_mode == CON_ANSI_MONO && f1 != f2)
    {
        f1 = f2 ^ 0x08;
    }

    screen->setFg(screen,f1 & mask_fg);
    return last_fg;
}


static void print0(FILE *f, const char *ss)
{
    int cx, cy;
    int old_cx = 0, old_cy = 0;
    const int sx = screen->getCols(screen);
    const int sy = screen->getRows(screen);
    int pass;

    // Note:
    //   We use 2 passes to avoid unnecessary system calls because
    //   scrollUp() under Win32 is *extremely* slow.
    UNUSED(f);

    screen->getCursor(screen,&old_cx,&old_cy);
    cx = old_cx; cy = old_cy;

    for (pass = 0; pass < 2; pass++)
    {
        const char *s = ss;
        // char buffer for pass 2
        char p[256+1];
        int pi = 0, px = 0, py = 0;

        for (;;)
        {
            // walk over whitespace
            for (;;)
            {
                if (*s == '\n')
                {
                    cx = 0;
                    cy++;
                }
                else if (*s == '\r')
                {
                    cx = 0;
                    if (pass > 0 && cy < sy)
                        screen->clearLine(screen,cy);
                }
                else
                    break;
                s++;
            }
            // adjust cursor
            if (cx >= sx)
            {
                cx = 0;
                cy++;
            }
            if (pass > 0)
            {
                // check if we should print something
                if (pi > 0 && (*s == 0 || py != cy))
                {
                    p[pi] = 0;
                    screen->putString(screen,p,px,py);
                    pi = 0;
                }
                // check if we should scroll even more (this can happen
                // if the string is longer than sy lines)
                if (cy >= sy)
                {
                    int scroll_y = cy - sy + 1;
                    screen->scrollUp(screen,scroll_y);
                    cy -= scroll_y;
                    if (cy < 0)
                        cy = 0;
                }
            }
            // done ?
            if (*s == 0)
                break;
            if (pass > 0)
            {
                // store current char
                if (pi == 0)
                {
                    px = cx;
                    py = cy;
                }
                p[pi++] = *s;
            }
            // advance
            cx++;
            s++;
        }

        if (pass == 0)
        {
            // end of pass 1 - scroll up, restore cursor
            if (cy >= sy)
            {
                int scroll_y = cy - sy + 1;
                screen->scrollUp(screen,scroll_y);
                cy = old_cy - scroll_y;
                if (cy < 0)
                    cy = 0;
            }
            else
                cy = old_cy;
            cx = old_cx;
        }
    }

    screen->setCursor(screen,cx,cy);
    screen->refresh(screen);
}


static bool intro(FILE *f)
{
    UNUSED(f);
#if defined(USE_FRAMES)
    if (screen->intro)
        return screen->intro(screen,screen_show_frames);
#endif
    return 0;
}


console_t console_screen =
{
    init,
    set_fg,
    print0,
    intro
};


#endif /* USE_SCREEN */


/*
vi:ts=4:et
*/

