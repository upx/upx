/* ui.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2001 Laszlo Molnar
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

   Markus F.X.J. Oberhumer   Laszlo Molnar
   markus@oberhumer.com      ml1050@cdata.tvnet.hu
 */


#include "conf.h"
#include "file.h"
#include "screen.h"
#include "ui.h"
#include "packer.h"


#if 1 && defined(USE_SCREEN)
#define UI_USE_SCREEN
#endif


enum {
    M_QUIET,        // nothing at all '-qqq'
    M_INFO,         // print a one line info after compression '-qq'
    M_MSG,          // print "compressing", then "\r" and M_INFO
    M_CB_TERM,      // 1 line callback using stdout
    M_CB_SCREEN     // 2 line callback using screen
};


struct UiPacker::State
{
    int mode;

    unsigned u_len;
    unsigned step;
    unsigned next_update;

    int pass;
    int total_passes;

    // message stuff
    char msg_buf[1+79+1];
    int pos;            // last progress bar position
    int counter;        // for spinner

    int bar_pos;
    int bar_len;
    int pass_digits;    // number of digits needed to print total_passes

#if defined(UI_USE_SCREEN)
    screen_t *screen;
    int b_cx, b_cy;
    int s_cx, s_cy;
    int s_fg, s_bg;
    int c_fg;
    int scroll_up;
    int cursor_shape;
#else
    void *screen;
#endif

    // debug
    unsigned progress_updates;
};


long UiPacker::total_files = 0;
long UiPacker::total_files_done = 0;
long UiPacker::total_c_len = 0;
long UiPacker::total_u_len = 0;
long UiPacker::total_fc_len = 0;
long UiPacker::total_fu_len = 0;
long UiPacker::update_c_len = 0;
long UiPacker::update_u_len = 0;
long UiPacker::update_fc_len = 0;
long UiPacker::update_fu_len = 0;

#define clear_cb()    memset(&cb, 0, sizeof(cb))


static const char header_line1[] =
    "        File size         Ratio      Format      Name\n";
static const char header_line2[] =
#ifdef __MSDOS__
    "   컴컴컴컴컴컴컴컴컴컴   컴컴컴   컴컴컴컴컴�   컴컴컴컴컴�\n";
#else
    "   --------------------   ------   -----------   -----------\n";
#endif


static const char *mkline(unsigned long fu_len, unsigned long fc_len,
                          unsigned long u_len, unsigned long c_len,
                          const char *format_name, const char *filename,
                          bool decompress=false)
{
    static char buf[2000];
    char r[7+1];
    const char *f;

    unsigned ratio = get_ratio(fu_len, fc_len);
    upx_snprintf(r,sizeof(r),"%3d.%02d%%", ratio / 10000, (ratio % 10000) / 100);
    if (decompress)
        f = "%10ld <-%10ld  %7s  %13s  %s";
    else
        f = "%10ld ->%10ld  %7s  %13s  %s";
    upx_snprintf(buf,sizeof(buf),f,
                 fu_len, fc_len, r, center_string(format_name,13), filename);
    UNUSED(u_len); UNUSED(c_len);
    return buf;
}


/*************************************************************************
//
**************************************************************************/

UiPacker::UiPacker(const Packer *p_) :
    p(p_), s(NULL)
{
    clear_cb();

    s = new State;
    memset(s,0,sizeof(*s));
    s->msg_buf[0] = '\r';

#if defined(UI_USE_SCREEN)
    // ugly hack
    s->screen = sobject_get_screen();
#endif

    if (opt->verbose < 0)
        s->mode = M_QUIET;
    else if (opt->verbose == 0 || !isatty(STDOUT_FILENO))
        s->mode = M_INFO;
    else if (opt->verbose == 1 || opt->no_progress)
        s->mode = M_MSG;
    else if (!s->screen)
        s->mode = M_CB_TERM;
    else
        s->mode = M_CB_SCREEN;
}


UiPacker::~UiPacker()
{
    clear_cb();
    delete s; s = NULL;
}


/*************************************************************************
// start callback
**************************************************************************/

void UiPacker::startCallback(unsigned u_len, unsigned step,
                             int pass, int total_passes)
{
    s->u_len = u_len;
    s->step = step;
    s->next_update = step;

    s->pass = pass;
    s->total_passes = total_passes;

    s->bar_len = 64;
    s->pos = -2;
    s->counter = 0;
    s->bar_pos = 1;             // because of the leading `\r'
    s->progress_updates = 0;
    s->pass_digits = 0;

    clear_cb();

    if (s->pass < 0)            // no callback wanted
        return;

    if (s->mode <= M_INFO)
        return;
    if (s->mode == M_MSG)
    {
        if (pass <= 1)
        {
            con_fprintf(stdout,"Compressing %s [%s]",p->fi->getName(),p->getName());
            fflush(stdout);
            printSetNl(2);
        }
        return;
    }

    cb.callback = callback;
    cb.user = this;

    if (s->mode == M_CB_TERM)
    {
        const char *fname = fn_basename(p->fi->getName());
        int l = (int) strlen(fname);
        if (l > 0 && l <= 30)
        {
            strcpy(&s->msg_buf[s->bar_pos], fname);
            s->bar_pos += l;
            s->msg_buf[s->bar_pos++] = ' ';
            s->msg_buf[s->bar_pos++] = ' ';
            s->bar_len -= l + 2;
        }
    }

    // set pass
    if (total_passes > 1)
    {
        do {
            s->pass_digits++;
            total_passes /= 10;
        } while (total_passes > 0);
        int l = sprintf(&s->msg_buf[s->bar_pos], "%*d/%*d  ",
                        s->pass_digits, s->pass,
                        s->pass_digits, s->total_passes);
        if (l > 0 && s->bar_len - l > 10)
        {
            s->bar_len -= l;
            s->bar_pos += l;
        }
    }

#if defined(UI_USE_SCREEN)
    if (s->mode == M_CB_SCREEN)
    {
        s->screen->getCursor(s->screen,&s->s_cx,&s->s_cy);
        s->s_fg = s->screen->getFg(s->screen);
        s->s_bg = s->screen->getBg(s->screen);

        // FIXME: this message can be longer than one line.
        //        must adapt endCallback() for this case.
        con_fprintf(stdout,"Compressing %s [%s]\n",p->fi->getName(),p->getName());
        s->screen->getCursor(s->screen,&s->b_cx,&s->b_cy);
        if (s->b_cy == s->s_cy)
            s->scroll_up++;
        if (s->screen->hideCursor)
            s->cursor_shape = s->screen->hideCursor(s->screen);
    }
#endif /* UI_USE_SCREEN */
}


// may only get called directly after startCallback()
void UiPacker::firstCallback()
{
    if (s->pos == -2)
        doCallback(0, 0);
}


// make sure we reach 100% in the progress bar
void UiPacker::finalCallback(unsigned u_len, unsigned c_len)
{
    s->next_update = u_len;
    doCallback(u_len, c_len);
}


/*************************************************************************
// end callback
**************************************************************************/

void UiPacker::endCallback()
{
    if (s->pass < 0)            // no callback wanted
        return;

    const bool done = (s->total_passes <= 0 || s->pass >= s->total_passes);

    if (s->mode == M_CB_TERM)
    {
        if (done)
            printClearLine(stdout);
        else
            printSetNl(2);
    }

    // restore screen
#if defined(UI_USE_SCREEN)
    if (s->mode == M_CB_SCREEN)
    {
#if 0
        if (s->scroll_up)
            s->screen->scrollDown(screen,s->scroll_up);
        else
            s->screen->clearLine(s->screen,s->s_cy+1);
        s->screen->clearLine(s->screen,s->s_cy);
        s->screen->setCursor(s->screen,s->s_cx,s->s_cy);
#else
        assert(s->s_cx == 0 && s->b_cx == 0);
        s->screen->clearLine(s->screen,s->b_cy-1);
        s->screen->clearLine(s->screen,s->b_cy);
        s->screen->setCursor(s->screen,s->b_cx,s->b_cy-1);
#endif
        s->screen->setFg(s->screen,s->s_fg);
        s->screen->setBg(s->screen,s->s_bg);
        if (s->cursor_shape > 0)
            s->screen->setCursorShape(s->screen,s->cursor_shape);
    }
#endif /* UI_USE_SCREEN */

    clear_cb();
#if 0
    printf("callback: pass %d, step %6d, updates %6d\n",
           s->pass, s->step, s->progress_updates);
#endif
}


/*************************************************************************
// the callback
**************************************************************************/

void __UPX_ENTRY UiPacker::callback(upx_uint isize, upx_uint osize, int state, void * user)
{
    //printf("%6d %6d %d\n", is, os, state);
    if (state != -1 && state != 3) return;
    if (user)
    {
        UiPacker *uip = reinterpret_cast<UiPacker *>(user);
        uip->doCallback(isize,osize);
    }
}


void UiPacker::doCallback(unsigned isize, unsigned osize)
{
    static const char spinner[] = "|/-\\";

    if (s->pass < 0)            // no callback wanted
        return;

    if (s->u_len == 0 || isize > s->u_len)
        return;
    // check if we should update the display
    if (s->step > 0 && isize > 0 && isize < s->u_len)
    {
        if (isize < s->next_update)
            return;
        s->next_update += s->step;
    }

#if 0
    printf("%6d %6d %6d %6d\n", isize, osize, s->step, s->next_update);
    return;
#endif

    // compute progress position
    int pos = -1;
    if (isize >= s->u_len)
        pos = s->bar_len;
    else if (isize > 0)
    {
        pos = get_ratio(s->u_len, isize) * s->bar_len / 1000000;
        assert(pos >= 0); assert(pos <= s->bar_len);
    }
    if (pos < s->pos)
        return;
    if (pos < 0 && pos == s->pos)
        return;

    // fill the progress bar
    char *m = &s->msg_buf[s->bar_pos];
    *m++ = '[';
    for (int i = 0; i < s->bar_len; i++)
    {
#ifdef __MSDOS__
        //*m++ = i <= pos ? '\xdb' : '.';
        //*m++ = i <= pos ? '\xb0' : '\x07';
        //*m++ = i <= pos ? '\xb1' : '\x07';
        *m++ = i <= pos ? '\xfe' : '\xf9';
#else
        *m++ = i <= pos ? '*' : '.';
#endif
    }
    *m++ = ']';

    // compute current compression ratio
    unsigned ratio = 1000000;
    if (osize > 0)
        ratio = get_ratio(isize, osize);

    sprintf(m,"  %3d.%1d%%  %c ",
            ratio / 10000, (ratio % 10000) / 1000,
            spinner[s->counter]);
    assert((int)strlen(s->msg_buf) < 1 + 80);

    s->pos = pos;
    s->counter = (s->counter + 1) & 3;
    s->progress_updates++;

    if (s->mode == M_CB_TERM)
    {
        const char *msg = &s->msg_buf[0];
        int fg = con_fg(stdout,FG_CYAN);
        con_fprintf(stdout,"%s",msg);      // avoid backslash interpretation
        (void) con_fg(stdout,fg);
        fflush(stdout);
        printSetNl(1);
        UNUSED(fg);
        return;
    }

#if defined(UI_USE_SCREEN)
    if (s->mode == M_CB_SCREEN)
    {
        const char *msg = &s->msg_buf[1];
#if 0
        s->screen->putString(s->screen,msg,s->b_cx,s->b_cy);
#else
        // FIXME: this doesn't honor `--mono' etc.
        int attr = FG_CYAN | s->s_bg;
        s->screen->putStringAttr(s->screen,msg,attr,s->b_cx,s->b_cy);
#endif
        s->screen->refresh(s->screen);
    }
#endif /* UI_USE_SCREEN */
}


/*************************************************************************
// pack
**************************************************************************/

void UiPacker::uiPackStart(const OutputFile *fo)
{
    total_files++;
    UNUSED(fo);
}


void UiPacker::uiPackEnd(const OutputFile *fo)
{
    uiUpdate(fo->getBytesWritten());

    if (s->mode == M_QUIET)
        return;
    if (s->mode == M_MSG)
    {
        // We must put this here and not in endCallback() as we may
        // have multiple passes.
        printClearLine(stdout);
    }

    const char *name = p->fi->getName();
    if (opt->output_name)
        name = opt->output_name;
    else if (opt->to_stdout)
        name = "<stdout>";
    con_fprintf(stdout,"%s\n",
                mkline(p->ph.u_file_size, fo->getBytesWritten(),
                       p->ph.u_len, p->ph.c_len,
                       p->getName(), fn_basename(name)));
    printSetNl(0);
}


void UiPacker::uiPackTotal()
{
    uiListTotal();
    uiFooter("Packed");
}


/*************************************************************************
// unpack
**************************************************************************/

void UiPacker::uiUnpackStart(const OutputFile *fo)
{
    total_files++;
    UNUSED(fo);
}


void UiPacker::uiUnpackEnd(const OutputFile *fo)
{
    uiUpdate(-1, fo->getBytesWritten());

    const char *name = p->fi->getName();
    if (opt->output_name)
        name = opt->output_name;
    else if (opt->to_stdout)
        name = "<stdout>";
    con_fprintf(stdout,"%s\n",
                mkline(fo->getBytesWritten(), p->file_size,
                       p->ph.u_len, p->ph.c_len,
                       p->getName(), fn_basename(name), true));
    printSetNl(0);
}


void UiPacker::uiUnpackTotal()
{
    uiListTotal(true);
    uiFooter("Unpacked");
}


/*************************************************************************
// list
**************************************************************************/

void UiPacker::uiListStart()
{
    total_files++;
}


void UiPacker::uiList(long fu_len)
{
    if (fu_len < 0)
        fu_len = p->ph.u_file_size;
    const char *name = p->fi->getName();
    con_fprintf(stdout,"%s\n",
                mkline(fu_len, p->file_size,
                       p->ph.u_len, p->ph.c_len,
                       p->getName(), name));
    printSetNl(0);
}


void UiPacker::uiListEnd()
{
    uiUpdate();
}


void UiPacker::uiListTotal(bool decompress)
{
    if (opt->verbose >= 1 && total_files >= 2)
    {
        char name[32];
        upx_snprintf(name,sizeof(name),"[ %ld file%s ]", total_files_done, total_files_done == 1 ? "" : "s");
        con_fprintf(stdout,"%s%s\n",
                    header_line2,
                    mkline(total_fu_len, total_fc_len,
                           total_u_len, total_c_len,
                           "", name, decompress));
        printSetNl(0);
    }
}


/*************************************************************************
// test
**************************************************************************/

void UiPacker::uiTestStart()
{
    total_files++;

    if (opt->verbose >= 1)
    {
        con_fprintf(stdout,"testing %s ", p->fi->getName());
        fflush(stdout);
        printSetNl(1);
    }
}


void UiPacker::uiTestEnd()
{
    if (opt->verbose >= 1)
    {
        con_fprintf(stdout,"[OK]\n");
        fflush(stdout);
        printSetNl(0);
    }
    uiUpdate();
}


void UiPacker::uiTestTotal()
{
    uiFooter("Tested");
}


/*************************************************************************
// info
**************************************************************************/

bool UiPacker::uiFileInfoStart()
{
    total_files++;

    int fg = con_fg(stdout,FG_CYAN);
    con_fprintf(stdout,"%s [%s]\n", p->fi->getName(), p->getName());
    fg = con_fg(stdout,fg);
    UNUSED(fg);
    if (p->ph.c_len > 0)
    {
        con_fprintf(stdout,"  %8ld bytes", (long)p->file_size);
        con_fprintf(stdout,", compressed by UPX %d, method %d, level %d, filter 0x%02x/0x%02x\n",
                    p->ph.version, p->ph.method, p->ph.level, p->ph.filter, p->ph.filter_cto);
        return false;
    }
    else
    {
        con_fprintf(stdout,"  %8ld bytes", (long)p->file_size);
        con_fprintf(stdout,", not compressed by UPX\n");
        return true;
    }
}

void UiPacker::uiFileInfoEnd()
{
    uiUpdate();
}


void UiPacker::uiFileInfoTotal()
{
}


/*************************************************************************
// util
**************************************************************************/

void UiPacker::uiHeader()
{
    static bool done = false;
    if (done)
        return;
    done = true;
    if (opt->cmd == CMD_TEST || opt->cmd == CMD_FILEINFO)
        return;
    if (opt->verbose >= 1)
    {
        con_fprintf(stdout,"%s%s", header_line1, header_line2);
    }
}


void UiPacker::uiFooter(const char *t)
{
    static bool done = false;
    if (done)
        return;
    done = true;
    if (opt->verbose >= 1)
    {
        long n1 = total_files;
        long n2 = total_files_done;
        long n3 = total_files - total_files_done;
        if (n3 == 0)
            con_fprintf(stdout,"\n%s %ld file%s.\n",
                        t, n1, n1 == 1 ? "" : "s");
        else
            con_fprintf(stdout,"\n%s %ld file%s: %ld ok, %ld error%s.\n",
                        t, n1, n1 == 1 ? "" : "s", n2, n3, n3 == 1 ? "" : "s");
    }
}


void UiPacker::uiUpdate(long fc_len, long fu_len)
{
    update_fc_len = (fc_len >= 0) ? fc_len : p->file_size;
    update_fu_len = (fu_len >= 0) ? fu_len : p->ph.u_file_size;
    update_c_len = p->ph.c_len;
    update_u_len = p->ph.u_len;
}


void UiPacker::uiConfirmUpdate()
{
    total_files_done++;
    total_fc_len += update_fc_len;
    total_fu_len += update_fu_len;
    total_c_len += update_c_len;
    total_u_len += update_u_len;
}


/*
vi:ts=4:et
*/

