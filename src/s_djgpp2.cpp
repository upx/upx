/* s_djgpp2.cpp -- djggp2 DOS screen driver

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

#if defined(USE_SCREEN) && defined(__DJGPP__)

#include "screen.h"

#define this local_this

#define mask_fg 0x0f
#define mask_bg 0xf0

/* #define USE_SCROLLBACK */


/*************************************************************************
// direct screen access
**************************************************************************/

#include <dos.h>
#if 0
#include <conio.h>
#endif
#include <dpmi.h>
#include <go32.h>
#include <sys/exceptn.h>
#include <sys/farptr.h>
#include <sys/movedata.h>
#define dossel  _go32_info_block.selector_for_linear_memory
#define co80    _go32_info_block.linear_address_of_primary_screen
#undef kbhit


struct screen_data_t
{
    int mode;
    int cols;
    int rows;
    int cursor_x;
    int cursor_y;
    int scroll_counter;
    unsigned char attr;
    unsigned char init_attr;
    unsigned char empty_attr;
    unsigned short empty_cell;
#ifdef USE_SCROLLBACK
    /* scrollback buffer */
    unsigned short sb_buf[32][256];
    int sb_size;
    int sb_base;
    int sb_sp;
#endif /* USE_SCROLLBACK */
};


/* atExit information */
static struct
{
    int cursor_shape;
} ae = {
    -1
};


#ifdef USE_SCROLLBACK
static __inline__ void sb_add(screen_t *this, int *val, int inc)
{
    *val = (*val + inc) & (this->data->sb_size - 1);
}

static void sb_push(screen_t *this, const unsigned short *line, int len)
{
    memcpy(this->data->sb_buf[this->data->sb_sp],line,len);
    sb_add(this,&this->data->sb_sp,1);
    if (this->data->sb_sp == this->data->sb_base)
        sb_add(this,&this->data->sb_base,1);
}

static const unsigned short *sb_pop(screen_t *this)
{
    if (this->data->sb_sp == this->data->sb_base)
        return NULL;
    sb_add(this,&this->data->sb_sp,-1);
    return this->data->sb_buf[this->data->sb_sp];
}
#endif /* USE_SCROLLBACK */


static void refresh(screen_t *this)
{
    UNUSED(this);
}


static __inline__
unsigned short make_cell(screen_t *this, int ch, int attr)
{
    UNUSED(this);
    return (unsigned short) (((attr & 0xff) << 8) | (ch & 0xff));
}


static int getMode(const screen_t *this)
{
    UNUSED(this);
    return ScreenMode();
}


static int getPage(const screen_t *this)
{
    UNUSED(this);
    return _farpeekb(dossel, 0x462);
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
    if (this->data->mode == 7)
        return 1;
    if ((_farpeekb(dossel, 0x465) & (4 | 16)) != 0)
        return 1;
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
    this->data->attr = (this->data->attr & mask_bg) | (fg & mask_fg);
}


static void setBg(screen_t *this, int bg)
{
    this->data->attr = (this->data->attr & mask_fg) | (bg & mask_bg);
}


static void setCursor(screen_t *this, int x, int y)
{
    if (x >= 0 && y >= 0 && x < this->data->cols && y < this->data->rows)
    {
        ScreenSetCursor(y,x);
        this->data->cursor_x = x;
        this->data->cursor_y = y;
    }
}


/*
// I added ScreenGetCursor, because when upx prints something longer than
// 1 line (an error message for example), the this->data->cursor_y can
// have a bad value - ml1050

// FIXME:
//   Laszlo: when does this happen ? This probably indicates a
//     bug in c_screen.cpp(print0) I've introduced with
//     the 2 passes implementation.
*/

static void getCursor(const screen_t *this, int *x, int *y)
{
    int cx = this->data->cursor_x;
    int cy = this->data->cursor_y;
#if 1
    ScreenGetCursor(&cy,&cx);
#endif
    if (x) *x = cx;
    if (y) *y = cy;
}


static void putCharAttr(screen_t *this, int ch, int attr, int x, int y)
{
    UNUSED(this);
    ScreenPutChar(ch,attr,x,y);
}


static void putChar(screen_t *this, int ch, int x, int y)
{
    ScreenPutChar(ch,this->data->attr,x,y);
}


static void putStringAttr(screen_t *this, const char *s, int attr, int x, int y)
{
    UNUSED(this);
    assert((int)strlen(s) <= 256);
    assert(x + (int)strlen(s) <= this->data->cols);
    ScreenPutString(s,attr,x,y);
}


static void putString(screen_t *this, const char *s, int x, int y)
{
    assert((int)strlen(s) <= 256);
    assert(x + (int)strlen(s) <= this->data->cols);
    ScreenPutString(s,this->data->attr,x,y);
}


/* private */
static void getChar(screen_t *this, int *ch, int *attr, int x, int y)
{
    UNUSED(this);
    ScreenGetChar(ch,attr,x,y);
}


static int getCursorShape(const screen_t *this)
{
    UNUSED(this);
    return _farpeekw(dossel, 0x460);
}


static void setCursorShape(screen_t *this, int shape)
{
    __dpmi_regs r;

    memset(&r,0,sizeof(r));         /* just in case... */
    r.x.ax = 0x0103;
#if 1
    if (this)
        r.h.al = getMode(this);     /* required for buggy BIOSes */
#endif
    r.x.cx = shape & 0x7f1f;
    __dpmi_int(0x10, &r);
}


static int hideCursor(screen_t *this)
{
    int shape = getCursorShape(this);
    setCursorShape(this,0x2000);
    return shape;
}


static int init(screen_t *this, int fd)
{
    int mode;
    int cols, rows;
    int attr;

#if 0
    /* force linkage of conio.o */
    (void) _conio_kbhit();
#endif

    if (!this || !this->data)
        return -1;

    this->data->mode = -1;
#ifdef USE_SCROLLBACK
    this->data->sb_size = 32;
    this->data->sb_base = 0;
    this->data->sb_sp = 0;
#endif
    if (fd < 0 || !acc_isatty(fd))
        return -1;
    if (getPage(this) != 0)
        return -1;

#if 1 && defined(__DJGPP__)
    /* check for Windows NT/2000/XP */
    if (_get_dos_version(1) == 0x0532)
        return -1;
#endif

    cols = ScreenCols();
    rows = ScreenRows();
    mode = getMode(this);
    if (mode > 0x13)
    {
        /* assume this is some SVGA/VESA text mode */
        __dpmi_regs r;

        memset(&r,0,sizeof(r));     /* just in case... */
        r.x.ax = 0x4f03;            /* VESA - get current video mode */
        __dpmi_int(0x10, &r);
        if (r.h.ah == 0)
            mode = r.x.bx;
    }
    else
    {
        if (mode != 2 && mode != 3 && mode != 7)
            return -1;
    }
    ScreenGetCursor(&this->data->cursor_y,&this->data->cursor_x);
    getChar(this,NULL,&attr,this->data->cursor_x,this->data->cursor_y);
    this->data->init_attr = attr;
    if (mode != 7)
    {
        /* Does it normally blink when bg has its 3rd bit set?  */
        int b_mask = (_farpeekb(dossel, 0x465) & 0x20) ? 0x70 : 0xf0;
        attr = attr & (mask_fg | b_mask);
    }
    this->data->mode = mode;
    this->data->cols = cols;
    this->data->rows = rows;
    this->data->attr = attr;
    this->data->empty_attr = attr;
    this->data->empty_cell = make_cell(this,' ',attr);

    ae.cursor_shape = getCursorShape(this);

    return 0;
}


static void updateLineN(screen_t *this, const void *line, int y, int len)
{
    if (y >= 0 && y < this->data->rows && len > 0 && len <= 2*this->data->cols)
        movedata(_my_ds(),(unsigned)line,dossel,co80+y*this->data->cols*2,len);
}


static void clearLine(screen_t *this, int y)
{
    if (y >= 0 && y < this->data->rows)
    {
        unsigned sp = co80 + y * this->data->cols * 2;
        unsigned short a = this->data->empty_cell;
        int i = this->data->cols;

        _farsetsel(dossel);
        do {
            _farnspokew(sp, a);
            sp += 2;
        } while (--i);
    }
}


static void clear(screen_t *this)
{
    unsigned char attr = ScreenAttrib;
    ScreenAttrib = this->data->empty_attr;
    ScreenClear();
    ScreenAttrib = attr;
}


static int scrollUp(screen_t *this, int lines)
{
    int sr = this->data->rows;
    int sc = this->data->cols;
    int y;


    if (lines <= 0 || lines > sr)
        return 0;

#ifdef USE_SCROLLBACK
    /* copy to scrollback buffer */
    for (y = 0; y < lines; y++)
    {
        unsigned short buf[ sc ];
        movedata(dossel,co80+y*this->data->cols*2,_my_ds(),(unsigned)buf,sizeof(buf));
        sb_push(this,buf,sizeof(buf));
    }
#endif

    /* move screen up */
    if (lines < sr)
        movedata(dossel,co80+lines*sc*2,dossel,co80,(sr-lines)*sc*2);

    /* fill in blank lines at bottom */
    for (y = sr - lines; y < sr; y++)
        clearLine(this,y);

    this->data->scroll_counter += lines;
    return lines;
}


static int scrollDown(screen_t *this, int lines)
{
    int sr = this->data->rows;
    int sc = this->data->cols;
    int y;

    if (lines <= 0 || lines > sr)
        return 0;

    /* move screen down */
    if (lines < sr)
    {
        /* !@#% movedata can't handle overlapping regions... */
        /* movedata(dossel,co80,dossel,co80+lines*sc*2,(sr-lines)*sc*2); */
        unsigned short buf[ (sr-lines)*sc ];
        movedata(dossel,co80,_my_ds(),(unsigned)buf,sizeof(buf));
        movedata(_my_ds(),(unsigned)buf,dossel,co80+lines*sc*2,sizeof(buf));
    }

    /* copy top lines from scrollback buffer */
    for (y = lines; --y >= 0; )
    {
#ifdef USE_SCROLLBACK
        const unsigned short *buf = sb_pop(this);
        if (buf == NULL)
            clearLine(this,y);
        else
            updateLineN(this,buf,y,sc*2);
#else
        clearLine(this,y);
#endif
    }

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
    return kbhit();
}


static int intro(screen_t *this, void (*show_frames)(screen_t *) )
{
    int shape;
    unsigned short old_flags = __djgpp_hwint_flags;

    if ((this->data->init_attr & mask_bg) != BG_BLACK)
        return 0;

    __djgpp_hwint_flags |= 3;
    while (kbhit())
        (void) getkey();

    shape = hideCursor(this);
    show_frames(this);
    setCursorShape(this,shape);

    while (kbhit())
        (void) getkey();
    __djgpp_hwint_flags = old_flags;

    return 1;
}


static void atExit(void)
{
    static int done = 0;
    if (done) return;
    done = 1;
    if (ae.cursor_shape >= 0)
        setCursorShape(NULL,ae.cursor_shape);
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
screen_t *screen_djgpp2_construct(void)
{
    return sobject_construct(&driver,sizeof(*driver.data));
}


#endif /* defined(USE_SCREEN) && defined(__DJGPP__) */


/*
vi:ts=4:et
*/

