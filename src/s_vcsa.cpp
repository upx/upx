/* s_vcsa.cpp -- Linux /dev/vcsa screen driver

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

#if defined(USE_SCREEN) && defined(USE_SCREEN_VCSA)

#include "screen.h"

#define this local_this

#define mask_fg 0x0f
#define mask_bg 0xf0

/* #define USE_SCROLLBACK */


/*************************************************************************
// direct screen access ( /dev/vcsaNN )
**************************************************************************/

#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#if defined(__linux__)
#  include <linux/kd.h>
#  include <linux/kdev_t.h>
#  include <linux/major.h>
#endif


struct screen_data_t
{
    int fd;
    int mode;
    int page;
    int cols;
    int rows;
    int cursor_x;
    int cursor_y;
    int scroll_counter;
    unsigned char attr;
    unsigned char init_attr;
    unsigned char map[256];
    unsigned short empty_line[256];
#ifdef USE_SCROLLBACK
    /* scrollback buffer */
    unsigned short sb_buf[32][256];
    int sb_size;
    int sb_base;
    int sb_sp;
#endif /* USE_SCROLLBACK */
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
    return ((attr & 0xff) << 8) | (this->data->map[ch & 0xff] & 0xff);
}


static int getMode(const screen_t *this)
{
    return this->data->mode;
}


static int getPage(const screen_t *this)
{
    return this->data->page;
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
    /* FIXME */
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
    this->data->attr = (this->data->attr & mask_bg) | (fg & mask_fg);
}


static void setBg(screen_t *this, int bg)
{
    this->data->attr = (this->data->attr & mask_fg) | (bg & mask_bg);
}


/* private */
static int gotoxy(screen_t *this, int x, int y)
{
    if (x >= 0 && y >= 0 && x < this->data->cols && y < this->data->rows)
    {
        if (lseek(this->data->fd, 4 + (x + y * this->data->cols) * 2, SEEK_SET) != -1)
        {
            return 0;
        }
    }
    return -1;
}


static void setCursor(screen_t *this, int x, int y)
{
    if (gotoxy(this,x,y) == 0)
    {
        unsigned char b[2] = { x, y };
        if (lseek(this->data->fd, 2, SEEK_SET) != -1)
            write(this->data->fd, b, 2);
        this->data->cursor_x = x;
        this->data->cursor_y = y;
    }
}


static void getCursor(const screen_t *this, int *x, int *y)
{
    int cx = this->data->cursor_x;
    int cy = this->data->cursor_y;
#if 1
    if (lseek(this->data->fd, 2, SEEK_SET) != -1)
    {
        unsigned char b[2];
        if (read(this->data->fd, b, 2) == 2)
        {
            if (b[0] < this->data->cols && b[1] < this->data->rows)
            {
                cx = b[0];
                cy = b[1];
            }
        }
    }
#endif
    if (x) *x = cx;
    if (y) *y = cy;
}


static void putCharAttr(screen_t *this, int ch, int attr, int x, int y)
{
    unsigned short a = make_cell(this,ch,attr);

    if (gotoxy(this,x,y) == 0)
        write(this->data->fd, &a, 2);
}


static void putChar(screen_t *this, int ch, int x, int y)
{
    putCharAttr(this,ch,this->data->attr,x,y);
}


static void putStringAttr(screen_t *this, const char *s, int attr, int x, int y)
{
    assert((int)strlen(s) <= 256);
    assert(x + (int)strlen(s) <= this->data->cols);
    while (*s)
        putCharAttr(this,*s++,attr,x++,y);
}


static void putString(screen_t *this, const char *s, int x, int y)
{
    putStringAttr(this,s,this->data->attr,x,y);
}


/* private */
static void getChar(screen_t *this, int *ch, int *attr, int x, int y)
{
    unsigned short a;

    if (gotoxy(this,x,y) == 0 && read(this->data->fd, &a, 2) == 2)
    {
        if (ch)
            *ch = a & 0xff;
        if (attr)
            *attr = (a >> 8) & 0xff;
    }
}


/* private */
static int init_scrnmap(screen_t *this, int fd)
{
    int scrnmap_done = 0;
    int i;

#if 1 && defined(GIO_UNISCRNMAP) && defined(E_TABSZ)
    if (!scrnmap_done)
    {
        unsigned short scrnmap[E_TABSZ];
        if (ioctl(fd, GIO_UNISCRNMAP, scrnmap) == 0)
        {
            for (i = 0; i < E_TABSZ; i++)
                this->data->map[scrnmap[i] & 0xff] = i;
            scrnmap_done = 1;
        }
    }
#endif
#if 1 && defined(GIO_SCRNMAP) && defined(E_TABSZ)
    if (!scrnmap_done)
    {
        unsigned char scrnmap[E_TABSZ];
        if (ioctl(fd, GIO_SCRNMAP, scrnmap) == 0)
        {
            for (i = 0; i < E_TABSZ; i++)
                this->data->map[scrnmap[i] & 0xff] = i;
            scrnmap_done = 1;
        }
    }
#endif

    return scrnmap_done;
}


static int init(screen_t *this, int fd)
{
    struct stat st;

    if (!this || !this->data)
        return -1;

    this->data->fd = -1;
    this->data->mode = -1;
    this->data->page = 0;
#ifdef USE_SCROLLBACK
    this->data->sb_size = 32;
    this->data->sb_base = 0;
    this->data->sb_sp = 0;
#endif
    if (fd < 0 || !acc_isatty(fd))
        return -1;
    if (fstat(fd,&st) != 0)
        return -1;

    /* check if we are running in a virtual console */
#if defined(MINOR) && defined(MAJOR) && defined(TTY_MAJOR)
    if (MAJOR(st.st_rdev) == TTY_MAJOR)
    {
        char vc_name[64];
        unsigned char vc_data[4];
        int i;
        int attr;
        unsigned short a;

        upx_snprintf(vc_name, sizeof(vc_name), "/dev/vcsa%d", (int) MINOR(st.st_rdev));
        this->data->fd = open(vc_name, O_RDWR);
        if (this->data->fd == -1)
        {
            upx_snprintf(vc_name, sizeof(vc_name), "/dev/vcc/a%d", (int) MINOR(st.st_rdev));
            this->data->fd = open(vc_name, O_RDWR);
        }
        if (this->data->fd != -1)
        {
            if (read(this->data->fd, vc_data, 4) == 4)
            {
                this->data->mode = 3;
                this->data->rows = vc_data[0];
                this->data->cols = vc_data[1];
                this->data->cursor_x = vc_data[2];
                this->data->cursor_y = vc_data[3];

                for (i = 0; i < 256; i++)
                    this->data->map[i] = i;
                i = init_scrnmap(this,this->data->fd) ||
                    init_scrnmap(this,STDIN_FILENO);

                getChar(this,NULL,&attr,this->data->cursor_x,this->data->cursor_y);
                this->data->init_attr = attr;
                this->data->attr = attr;
                a = make_cell(this,' ',attr);
                for (i = 0; i < 256; i++)
                    this->data->empty_line[i] = a;
            }
            else
            {
                close(this->data->fd);
                this->data->fd = -1;
            }
        }
    }
#endif

    if (this->data->mode < 0)
        return -1;

    return 0;
}


static void finalize(screen_t *this)
{
    if (this->data->fd != -1)
        (void) close(this->data->fd);
}


static void updateLineN(screen_t *this, const void *line, int y, int len)
{
    if (len > 0 && len <= 2*this->data->cols && gotoxy(this,0,y) == 0)
    {
        int i;
        unsigned char new_line[len];
        unsigned char *l1 = new_line;
        const unsigned char *l2 = (const unsigned char *) line;

        for (i = 0; i < len; i += 2)
        {
            *l1++ = *l2++;
            *l1++ = this->data->map[*l2++];
        }
        write(this->data->fd, new_line, len);
    }
}


static void clearLine(screen_t *this, int y)
{
    if (gotoxy(this,0,y) == 0)
        write(this->data->fd, this->data->empty_line, 2*this->data->cols);
}


static void clear(screen_t *this)
{
    int y;

    for (y = 0; y < this->data->rows; y++)
        clearLine(this,y);
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
        gotoxy(this,0,y);
        read(this->data->fd, buf, sizeof(buf));
        sb_push(this,buf,sizeof(buf));
    }
#endif

    /* move screen up */
    if (lines < sr)
    {
        unsigned short buf[ (sr-lines)*sc ];
        gotoxy(this,0,lines);
        read(this->data->fd, buf, sizeof(buf));
        gotoxy(this,0,0);
        write(this->data->fd, buf, sizeof(buf));
    }

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
        unsigned short buf[ (sr-lines)*sc ];
        gotoxy(this,0,0);
        read(this->data->fd, buf, sizeof(buf));
        gotoxy(this,0,lines);
        write(this->data->fd, buf, sizeof(buf));
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


static int getCursorShape(const screen_t *this)
{
    UNUSED(this);
    return 0;
}


static void setCursorShape(screen_t *this, int shape)
{
    UNUSED(this);
    UNUSED(shape);
}


static int kbhit(screen_t *this)
{
    const int fd = STDIN_FILENO;
    const unsigned long usec = 0;
    struct timeval tv;
    fd_set fds;

    UNUSED(this);
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    tv.tv_sec  = usec / 1000000;
    tv.tv_usec = usec % 1000000;
    return (select(fd + 1, &fds, NULL, NULL, &tv) > 0);
}


static int intro(screen_t *this, void (*show_frames)(screen_t *) )
{
    int shape;
    struct termios term_old, term_new;
    int term_r;

    if ((this->data->init_attr & mask_bg) != BG_BLACK)
        return 0;

    term_r = tcgetattr(STDIN_FILENO, &term_old);
    if (term_r == 0)
    {
        term_new = term_old;
        term_new.c_lflag &= ~(ISIG | ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &term_new);
    }

    shape = getCursorShape(this);
    setCursorShape(this,0x2000);
    show_frames(this);
    if (this->data->rows > 24)
        setCursor(this,this->data->cursor_x,this->data->cursor_y+1);
    setCursorShape(this,shape);

    while (kbhit(this))
        (void) fgetc(stdin);
    if (term_r == 0)
        tcsetattr(STDIN_FILENO, TCSANOW, &term_old);

    return 1;
}


static const screen_t driver =
{
    sobject_destroy,
    finalize,
    0,                  /* atExit */
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
    0,                  /* hideCursor */
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
    kbhit,
    intro,
    (struct screen_data_t *) 0
};


/* public constructor */
screen_t *screen_vcsa_construct(void)
{
    return sobject_construct(&driver,sizeof(*driver.data));
}


#endif /* defined(USE_SCREEN) && defined(USE_SCREEN_VCSA) */


/*
vi:ts=4:et
*/

