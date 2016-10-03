/* screen.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2016 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2016 Laszlo Molnar
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

#ifndef __UPX_SCREEN_H
#define __UPX_SCREEN_H 1

#if (USE_SCREEN)

/*************************************************************************
//
**************************************************************************/

struct screen_data_t;
struct screen_t;
typedef struct screen_t screen_t;

struct screen_t {
    /* public: */
    void (*destroy)(screen_t *s);
    void (*finalize)(screen_t *s);
    void (*atExit)(void); /* atexit/signal handler */

    int (*init)(screen_t *s, int fd);

    void (*refresh)(screen_t *s);

    int (*getMode)(const screen_t *s);
    int (*getPage)(const screen_t *s);
    int (*getRows)(const screen_t *s);
    int (*getCols)(const screen_t *s);
    int (*isMono)(const screen_t *s);

    int (*getFg)(const screen_t *s);
    int (*getBg)(const screen_t *s);
    void (*getCursor)(const screen_t *s, int *x, int *y);
    int (*getCursorShape)(const screen_t *s);

    void (*setFg)(screen_t *s, int);
    void (*setBg)(screen_t *s, int);
    void (*setCursor)(screen_t *s, int x, int y);
    void (*setCursorShape)(screen_t *s, int shape);
    int (*hideCursor)(screen_t *s);

    void (*putChar)(screen_t *s, int c, int x, int y);
    void (*putCharAttr)(screen_t *s, int c, int attr, int x, int y);
    void (*putString)(screen_t *s, const char *, int x, int y);
    void (*putStringAttr)(screen_t *s, const char *, int attr, int x, int y);

    void (*clear)(screen_t *s);
    void (*clearLine)(screen_t *s, int);
    void (*updateLineN)(screen_t *s, const void *, int y, int len);

    int (*scrollUp)(screen_t *s, int);
    int (*scrollDown)(screen_t *s, int);
    int (*getScrollCounter)(const screen_t *s);

    int (*kbhit)(screen_t *s);

    int (*intro)(screen_t *s, void (*)(screen_t *));

    /* private: */
    struct screen_data_t *data;
};

screen_t *sobject_construct(const screen_t *c, size_t data_size);
void sobject_destroy(screen_t *);
screen_t *sobject_get_screen(void);

screen_t *screen_curses_construct(void);
screen_t *screen_djgpp2_construct(void);
screen_t *screen_vcsa_construct(void);
screen_t *screen_win32_construct(void);

void screen_show_frames(screen_t *);

#endif /* USE_SCREEN */

#endif /* already included */

/* vim:set ts=4 sw=4 et: */
