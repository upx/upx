/* c_screen.cpp -- screen driver console output

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2000 Laszlo Molnar

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

#if defined(USE_SCREEN)

#include "screen.h"

#define mask_fg 0x0f
#define mask_bg 0xf0


/*************************************************************************
//
**************************************************************************/

static screen_t *do_construct(screen_t *s, int fd)
{
    if (!s)
        return NULL;
    if (s->init(s,fd) != 0)
    {
        s->destroy(s);
        return NULL;
    }
    return s;
}


static screen_t *screen = NULL;

static void do_destroy(void)
{
    if (screen)
    {
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
    int n = CON_INIT;

    UNUSED(now);
    assert(screen == NULL);
    atexit(do_destroy);
#if defined(__DJGPP__)
    if (!screen)
        screen = do_construct(screen_djgpp2_construct(),fd);
#endif
#if defined(__MFX_WIN32)
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
    if (screen->getCols(screen) < 80 || screen->getCols(screen) > 256)
        return CON_INIT;
    if (screen->getRows(screen) < 24)
        return CON_INIT;
    if (cur_fg == (cur_bg >> 4))
        return CON_INIT;
    if (cur_bg != BG_BLACK)
        if (!screen->isMono(screen))
        {
            /* return CON_ANSI_MONO; */     /* we could emulate ANSI mono */
            return CON_INIT;
        }

    if (o == CON_SCREEN)
        n = CON_SCREEN;
    if (o == CON_INIT)                      /* use by default */
        n = CON_SCREEN;
    if (o == CON_ANSI_COLOR)                /* can emulate ANSI color */
        n = CON_ANSI_COLOR;
    if (o == CON_ANSI_MONO)                 /* can emulate ANSI mono */
        n = CON_ANSI_MONO;

    if (screen->atExit)
        atexit(screen->atExit);
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
    int c_cx, c_cy;
    char p[256+1];
    int pi = 0, px = -1, py = -1;
    const int sx = screen->getCols(screen);
    const int sy = screen->getRows(screen);
    int pass;

    // Note:
    //   We use 2 passes to avoid unnecessary system calls because
    //   scrollScreen() under Win32 is *extremely* slow.
    UNUSED(f);
    screen->getCursor(screen,&cx,&cy);
    c_cx = cx; c_cy = cy;
    for (pass = 0; pass < 2; pass++)
    {
        const char *s = ss;
        int scroll_y = 0;
        while (*s)
        {
            for ( ; *s; s++)
            {
                if (*s == '\n')
                {
                    c_cy++;
                    c_cx = 0;
                }
                else if (*s == '\r')
                {
                    c_cx = 0;
#if 1
                    if (pass > 0 && c_cy < sy)
                        screen->clearLine(screen,c_cy);
#endif
                }
                else
                    break;
            }
            if (c_cx >= sx)
            {
                c_cy++;
                c_cx = 0;
            }
            if (pass > 0 && pi > 0 && py != c_cy)
            {
                screen->putString(screen,p,px,py);
                pi = 0;
            }
            if (c_cy >= sy)
            {
                int l = c_cy - sy + 1;
                if (pass > 0)
                    c_cy -= screen->scrollUp(screen,l);
                else
                {
                    scroll_y += l;
                    c_cy -= l;
                }
                if (c_cy < 0)
                    c_cy = 0;
                c_cx = 0;
            }
            if (*s)
            {
                if (pass > 0)
                {
                    if (pi == 0)
                        px = c_cx, py = c_cy;
                    p[pi++] = *s;
                    p[pi] = 0;
                }
                c_cx++;
                s++;
            }
        }
        if (pass == 0)
        {
            c_cx = cx;
            if (scroll_y > 0)
            {
                c_cy -= screen->scrollUp(screen,scroll_y);
                if (c_cy < 0)
                    c_cy = 0;
            }
            else
                c_cy = cy;
        }
    }
    if (pi > 0)
        screen->putString(screen,p,px,py);
    screen->setCursor(screen,c_cx,c_cy);
    screen->refresh(screen);
}


static upx_bool intro(FILE *f)
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

