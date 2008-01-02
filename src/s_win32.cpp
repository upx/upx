/* s_win32.cpp -- Win32 console screen driver

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

#if defined(USE_SCREEN_WIN32)

#include "screen.h"

#define this local_this

#define mask_fg 0x0f
#define mask_bg 0xf0


/*************************************************************************
// direct screen access
**************************************************************************/

#if (ACC_CC_MSC && (_MSC_VER >= 1000 && _MSC_VER < 1200))
   /* avoid -W4 warnings in <conio.h> */
#  pragma warning(disable: 4032)
   /* avoid -W4 warnings in <windows.h> */
#  pragma warning(disable: 4201 4214 4514)
#endif
#if defined(__RSXNT__)
#  define timeval win32_timeval  /* struct timeval already in <sys/time.h> */
#endif
#include <windows.h>
#if defined(HAVE_CONIO_H)
#  include <conio.h>
#endif


struct screen_data_t
{
    HANDLE hi;
    HANDLE ho;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
#if 0
    char title[512];
#endif

    int mode;
    int cols;
    int rows;
    int cursor_x;
    int cursor_y;
    int scroll_counter;

    WORD attr;
    WORD init_attr;

    CHAR_INFO empty_cell;
    CHAR_INFO empty_line[256];
};


#define P(x)    ((SHORT) (x))

static const COORD pos00 = { 0, 0 };
static const COORD size11 = { 1, 1 };


/* atExit information */
static struct
{
    int is_valid;
    HANDLE ho;
    CONSOLE_CURSOR_INFO cci;
} ae;


static void refresh(screen_t *this)
{
    UNUSED(this);
}


static int getMode(const screen_t *this)
{
    return this->data->mode;
}


static int getPage(const screen_t *this)
{
    UNUSED(this);
    return 0;
}


static int getRows(const screen_t *this)
{
    return this->data->rows;
}


static int getCols(const screen_t *this)
{
    return this->data->cols;
}


static int isMono(const screen_t *this)
{
    UNUSED(this);
    return 0;
}


static int getFg(const screen_t *this)
{
    return this->data->attr & mask_fg;
}


static int getBg(const screen_t *this)
{
    return this->data->attr & mask_bg;
}


static void setFg(screen_t *this, int fg)
{
    this->data->attr = (WORD) ((this->data->attr & mask_bg) | (fg & mask_fg));
    SetConsoleTextAttribute(this->data->ho, this->data->attr);
}


static void setBg(screen_t *this, int bg)
{
    this->data->attr = (WORD) ((this->data->attr & mask_fg) | (bg & mask_bg));
    SetConsoleTextAttribute(this->data->ho, this->data->attr);
}


static void setCursor(screen_t *this, int x, int y)
{
    if (x >= 0 && y >= 0 && x < this->data->cols && y < this->data->rows)
    {
        COORD coord = { P(x), P(y) };
        SetConsoleCursorPosition(this->data->ho, coord);
        this->data->cursor_x = x;
        this->data->cursor_y = y;
    }
}


static void getCursor(const screen_t *this, int *x, int *y)
{
    int cx = this->data->cursor_x;
    int cy = this->data->cursor_y;
#if 1
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(this->data->ho, &csbi))
    {
        cx = csbi.dwCursorPosition.X;
        cy = csbi.dwCursorPosition.Y;
#if 0
        assert(cx == this->data->cursor_x);
        assert(cy == this->data->cursor_y);
#endif
    }
#endif
    if (x) *x = cx;
    if (y) *y = cy;
}


static void putCharAttr(screen_t *this, int ch, int attr, int x, int y)
{
    CHAR_INFO ci;
    SMALL_RECT region = { P(x), P(y), P(x), P(y) };
    ci.Char.UnicodeChar = 0;
    ci.Char.AsciiChar = (CHAR) ch;
    ci.Attributes = (WORD) attr;
    WriteConsoleOutputA(this->data->ho, &ci, size11, pos00, &region);
}


static void putChar(screen_t *this, int ch, int x, int y)
{
    this->putCharAttr(this, ch, this->data->attr, x, y);
}


static void putStringAttr(screen_t *this, const char *s, int attr, int x, int y)
{
    int i;
    int l = (int) strlen(s);
    if (l <= 0)
        return;
    assert(l <= 256);
    assert(x + l <= this->data->cols);
    CHAR_INFO ci[256];
    COORD size = { P(l), 1 };
    SMALL_RECT region = { P(x), P(y), P(x + l - 1), P(y) };
    for (i = 0; i < l; i++)
    {
        ci[i].Char.UnicodeChar = 0;
        ci[i].Char.AsciiChar = *s++;
        ci[i].Attributes = (WORD) attr;
    }
    WriteConsoleOutputA(this->data->ho, &ci[0], size, pos00, &region);
}


static void putString(screen_t *this, const char *s, int x, int y)
{
    this->putStringAttr(this, s, this->data->attr, x, y);
}


/* private */
static int cci2shape(const CONSOLE_CURSOR_INFO *cci)
{
    int shape = cci->dwSize & 255;
    if (!cci->bVisible)
        shape |= 0x2000;
    return shape;
}


static int getCursorShape(const screen_t *this)
{
    CONSOLE_CURSOR_INFO cci;
    GetConsoleCursorInfo(this->data->ho, &cci);
    return cci2shape(&cci);
}


static void setCursorShape(screen_t *this, int shape)
{
    CONSOLE_CURSOR_INFO cci;
    cci.dwSize = shape & 255;
    cci.bVisible = (shape & 0x2000) ? 0 : 1;
    SetConsoleCursorInfo(this->data->ho, &cci);
}


static int hideCursor(screen_t *this)
{
    CONSOLE_CURSOR_INFO cci;
    int shape;

    GetConsoleCursorInfo(this->data->ho, &cci);
    shape = cci2shape(&cci);
    if (cci.bVisible)
    {
        cci.bVisible = 0;
        SetConsoleCursorInfo(this->data->ho, &cci);
    }
    return shape;
}


static int init(screen_t *this, int fd)
{
    HANDLE hi, ho;
    CONSOLE_SCREEN_BUFFER_INFO *csbi;
    DWORD mode;
    WORD attr;
    int i;

    if (!this || !this->data)
        return -1;

    this->data->hi = INVALID_HANDLE_VALUE;
    this->data->ho = INVALID_HANDLE_VALUE;
    this->data->mode = -1;
    if (fd < 0 || !acc_isatty(fd))
        return -1;

    hi = GetStdHandle(STD_INPUT_HANDLE);
    ho = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hi == INVALID_HANDLE_VALUE || ho == INVALID_HANDLE_VALUE)
        return -1;
    if (!GetConsoleMode(ho, &mode))
        return -1;
    csbi = &this->data->csbi;
    if (!GetConsoleScreenBufferInfo(ho, csbi))
        return -1;
    if (!GetConsoleCursorInfo(ho, &ae.cci))
        return -1;
#if 0
    if (!GetConsoleTitle(this->data->title, sizeof(this->data->title)))
        return -1;
#endif

#if 0
    this->data->cols = csbi->srWindow.Right - csbi->srWindow.Left + 1;
    this->data->rows = csbi->srWindow.Bottom - csbi->srWindow.Top + 1;
    if (csbi->srWindow.Left != 0 || csbi->srWindow.Top != 0)
        return -1;
    if (this->data->cols != csbi->dwSize.X)
        return -1;
#else
    this->data->cols = csbi->dwSize.X;
    this->data->rows = csbi->dwSize.Y;
#if 0
    if (csbi->srWindow.Left != 0)
        return -1;
#endif
#endif

    this->data->cursor_x = csbi->dwCursorPosition.X;
    this->data->cursor_y = csbi->dwCursorPosition.Y;

    ae.ho = ho;
    ae.is_valid = 1;

    attr = csbi->wAttributes;
    this->data->hi = hi;
    this->data->ho = ho;
    this->data->mode = 3;       // ???
    this->data->attr = attr;
    this->data->init_attr = attr;
    this->data->empty_cell.Char.UnicodeChar = 0;
    this->data->empty_cell.Char.AsciiChar = ' ';
    this->data->empty_cell.Attributes = attr;
    for (i = 0; i < 256; i++)
        this->data->empty_line[i] = this->data->empty_cell;

    return 0;
}


static void updateLineN(screen_t *this, const void *line, int y, int len)
{
    if (y >= 0 && y < this->data->rows && len > 0 && len <= 2*this->data->cols)
    {
#if 0
        const char *s = (const char *) line;
        int l = len / 2;
        int i;

        assert(l <= 256);
        CHAR_INFO ci[256];
        COORD size = { P(l), 1 };
        SMALL_RECT region = { 0, P(y), P(0 + l - 1), P(y) };
        for (i = 0; i < l; i++)
        {
            ci[i].Char.UnicodeChar = 0;
            ci[i].Char.AsciiChar = *s++;
            ci[i].Attributes = *s++;
        }
        WriteConsoleOutputA(this->data->ho, &ci[0], size, pos00, &region);
#endif
        UNUSED(line);
    }
}


static void clearLine(screen_t *this, int y)
{
    if (y >= 0 && y < this->data->rows)
    {
        COORD size = { P(this->data->cols), 1 };
        SMALL_RECT region = { 0, P(y), P(this->data->cols-1), P(y) };
        WriteConsoleOutputA(this->data->ho, this->data->empty_line, size, pos00, &region);
    }
}


static void clear(screen_t *this)
{
    int y;
    for (y = 0; y < this->data->rows; y++)
        this->clearLine(this, y);
}


/* private */
static int do_scroll(screen_t *this, int lines, int way)
{
    if (lines <= 0 || lines > this->data->rows)
        return 0;
    if (lines == this->data->rows)
    {
        this->clear(this);
        return lines;
    }

    SMALL_RECT rect = { 0, 0, P(this->data->cols-1), P(this->data->rows-1) };
    //SMALL_RECT clip = rect;
    COORD dest = { 0, 0 };
    switch (way)
    {
        case 0:
            rect.Top = P(rect.Top + lines);
            break;
        case 1:
            rect.Bottom = P(rect.Bottom - lines);
            dest.Y = P(dest.Y + lines);
            break;
    }
    //ScrollConsoleScreenBuffer(this->data->ho, &rect, &clip, dest, &this->data->empty_cell);
    ScrollConsoleScreenBuffer(this->data->ho, &rect, NULL, dest, &this->data->empty_cell);

    return lines;
}

static int scrollUp(screen_t *this, int lines)
{
    lines = do_scroll(this, lines, 0);
    this->data->scroll_counter += lines;
    return lines;
}

static int scrollDown(screen_t *this, int lines)
{
    lines = do_scroll(this, lines, 1);
    this->data->scroll_counter -= lines;
    return lines;
}

static int getScrollCounter(const screen_t *this)
{
    return this->data->scroll_counter;
}


static int s_kbhit(screen_t *this)
{
    UNUSED(this);
#if defined(HAVE_CONIO_H)
# if defined(__RSXNT__)
    return 0;
# elif defined(__BORLANDC__) || defined(__WATCOMC__)
    return kbhit();
# else
    return _kbhit();
# endif
#else
    return 0;
#endif
}


static int intro(screen_t *this, void (*show_frames)(screen_t *) )
{
    UNUSED(this);
    UNUSED(show_frames);
    return 0;
}


static void atExit(void)
{
    static int done = 0;
    if (done) return;
    done = 1;
    if (ae.is_valid)
    {
    }
}


static const screen_t driver =
{
    sobject_destroy,
    0,                  /* finalize, */
    atExit,
    init,
    refresh,
    getMode,
    getPage,
    getRows,
    getCols,
    isMono,
    getFg,
    getBg,
    getCursor,
    getCursorShape,
    setFg,
    setBg,
    setCursor,
    setCursorShape,
    hideCursor,
    putChar,
    putCharAttr,
    putString,
    putStringAttr,
    clear,
    clearLine,
    updateLineN,
    scrollUp,
    scrollDown,
    getScrollCounter,
    s_kbhit,
    intro,
    (struct screen_data_t *) 0
};


/* public constructor */
screen_t *screen_win32_construct(void)
{
    return sobject_construct(&driver,sizeof(*driver.data));
}


#endif /* defined(USE_SCREEN) && (ACC_OS_WIN32 || ACC_OS_WIN64) */


/*
vi:ts=4:et
*/

