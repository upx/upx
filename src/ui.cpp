/* ui.cpp --

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
#include "file.h"
#include "ui.h"
#include "screen.h"
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
    int pos;                // last progress bar position
    unsigned spin_counter;  // for spinner

    int bar_pos;
    int bar_len;
    int pass_digits;        // number of digits needed to print total_passes

#if defined(UI_USE_SCREEN)
    screen_t *screen;
    int screen_init_done;
    int b_cx, b_cy;
    int s_cx, s_cy;
    int s_fg, s_bg;
    int c_fg;
    int scroll_up;
    int cursor_shape;
#else
    void *screen;
#endif
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


/*************************************************************************
// constants
**************************************************************************/

static const char header_line1[] =
    "        File size         Ratio      Format      Name\n";
static char header_line2[] =
    "   --------------------   ------   -----------   -----------\n";

static char progress_filler[] = ".*[]";


static void init_global_constants(void)
{
#if 0 && (ACC_OS_DOS16 || ACC_OS_DOS32)
    // FIXME: should test codepage here

    static bool done = false;
    if (done)
        return;
    done = true;

#if 1 && defined(__DJGPP__)
    /* check for Windows NT/2000/XP */
    if (_get_dos_version(1) == 0x0532)
        return;
#endif

    char *p;
    for (p = header_line2; *p; p++)
        if (*p == '-')
            *p = '\xc4';

    //strcpy(progress_filler, "\x07\xb0[]");
    //strcpy(progress_filler, "\x07\xb1[]");
    strcpy(progress_filler, "\xf9\xfe[]");
#endif
}


/*************************************************************************
//
**************************************************************************/

static const char *mkline(unsigned long fu_len, unsigned long fc_len,
                          unsigned long u_len, unsigned long c_len,
                          const char *format_name, const char *filename,
                          bool decompress=false)
{
    static char buf[2048];
    char r[7+1];
    char fn[13+1];
    const char *f;

    // Large ratios can happen because of overlays that are
    // appended after a program is packed.
    unsigned ratio = get_ratio(fu_len, fc_len) + 50;
#if 1
    if (ratio >= 1000*1000)
        strcpy(r, "overlay");
#else
    if (ratio >= 10*1000*1000)      // >= "1000%"
        strcpy(r, "999.99%");
#endif
    else
        upx_snprintf(r, sizeof(r), "%3u.%02u%%", ratio / 10000, (ratio % 10000) / 100);
    if (decompress)
        f = "%10ld <-%10ld  %7s  %13s  %s";
    else
        f = "%10ld ->%10ld  %7s  %13s  %s";
    center_string(fn, sizeof(fn), format_name);
    assert(strlen(fn) == 13);
    upx_snprintf(buf, sizeof(buf), f, fu_len, fc_len, r, fn, filename);
    UNUSED(u_len); UNUSED(c_len);
    return buf;
}


/*************************************************************************
//
**************************************************************************/

UiPacker::UiPacker(const Packer *p_) :
    ui_pass(0), ui_total_passes(0), p(p_), s(NULL)
{
    init_global_constants();

    cb.reset();

    s = new State;
    memset(s,0,sizeof(*s));
    s->msg_buf[0] = '\r';

#if defined(UI_USE_SCREEN)
    // FIXME - ugly hack
    s->screen = sobject_get_screen();
#endif

    if (opt->verbose < 0)
        s->mode = M_QUIET;
    else if (opt->verbose == 0 || !acc_isatty(STDOUT_FILENO))
        s->mode = M_INFO;
    else if (opt->verbose == 1 || opt->no_progress)
        s->mode = M_MSG;
    else if (s->screen == NULL)
        s->mode = M_CB_TERM;
    else
        s->mode = M_CB_SCREEN;
}


UiPacker::~UiPacker()
{
    cb.reset();
    delete s; s = NULL;
}


/*************************************************************************
// start callback
**************************************************************************/

void UiPacker::printInfo(int nl)
{
    if (opt->all_methods && s->total_passes > 1)
        con_fprintf(stdout, "Compressing %s [%s]%s", p->fi->getName(), p->getName(), nl ? "\n" : "");
    else
    {
        char method_name[32+1];
        set_method_name(method_name, sizeof(method_name), p->ph.method, p->ph.level);
        con_fprintf(stdout, "Compressing %s [%s, %s]%s", p->fi->getName(), p->getName(), method_name, nl ? "\n" : "");
    }
}


void UiPacker::startCallback(unsigned u_len, unsigned step,
                             int pass, int total_passes)
{
    s->u_len = u_len;
    s->step = step;
    s->next_update = step;

    s->pass = pass;
    s->total_passes = total_passes;
    //printf("startCallback %d %d\n", s->pass, s->total_passes);

    s->bar_len = 64;
    s->pos = -2;
    s->spin_counter = 0;
    s->bar_pos = 1;             // because of the leading '\r'
    s->pass_digits = 0;

    cb.reset();

    if (s->pass < 0)            // no callback wanted
        return;

    if (s->mode <= M_INFO)
        return;
    if (s->mode == M_MSG)
    {
        if (pass <= 1)
        {
            printInfo(0);
            fflush(stdout);
            printSetNl(2);
        }
        return;
    }

#if (ACC_CC_MSC && (_MSC_VER == 1300))
    cb.nprogress = &UiPacker::progress_callback;
#else
    cb.nprogress = progress_callback;
#endif
    cb.user = this; // parameter for static function UiPacker::progress_callback()

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
        int buflen, l;
        do {
            s->pass_digits++;
            total_passes /= 10;
        } while (total_passes > 0);
        buflen = sizeof(s->msg_buf) - s->bar_pos;
        l = upx_snprintf(&s->msg_buf[s->bar_pos], buflen, "%*d/%*d  ",
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
        if (!s->screen_init_done)
        {
            s->screen_init_done = 1;
            if (s->screen->hideCursor)
                s->cursor_shape = s->screen->hideCursor(s->screen);
            s->s_fg = s->screen->getFg(s->screen);
            s->s_bg = s->screen->getBg(s->screen);
            s->screen->getCursor(s->screen,&s->s_cx,&s->s_cy);
            s->scroll_up = s->screen->getScrollCounter(s->screen);
            printInfo(1);
            s->screen->getCursor(s->screen,&s->b_cx,&s->b_cy);
            s->scroll_up = s->screen->getScrollCounter(s->screen) - s->scroll_up;
        }
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
    bool done = (s->total_passes <= 0 || s->pass >= s->total_passes);
    endCallback(done);
}

void UiPacker::endCallback(bool done)
{
    if (s->pass < 0)            // no callback wanted
        return;

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
        if (done)
        {
            int cx, cy, sy;
            assert(s->screen_init_done);
            s->screen_init_done = 0;
            assert(s->s_cx == 0 && s->b_cx == 0);
            s->screen->getCursor(s->screen, &cx, &cy);
            sy = UPX_MAX(0, s->s_cy - s->scroll_up);
            while (cy >= sy)
                s->screen->clearLine(s->screen, cy--);
            s->screen->setCursor(s->screen, s->s_cx, sy);
            s->screen->setFg(s->screen,s->s_fg);
            s->screen->setBg(s->screen,s->s_bg);
            if (s->cursor_shape > 0)
                s->screen->setCursorShape(s->screen,s->cursor_shape);
        }
        else
        {
            // not needed:
//            s->screen->clearLine(s->screen, s->b_cy);
//            s->screen->setCursor(s->screen, s->b_cx, s->b_cy);
        }
    }
#endif /* UI_USE_SCREEN */

    cb.reset();
#if 0
    printf("callback: pass %d, step %6d, updates %6d\n",
           s->pass, s->step, s->spin_counter);
#endif
}


/*************************************************************************
// the callback
**************************************************************************/

void __acc_cdecl UiPacker::progress_callback(upx_callback_p cb, unsigned isize, unsigned osize)
{
    //printf("%6d %6d %d\n", isize, osize, state);
    UiPacker *self = (UiPacker *) cb->user;
    self->doCallback(isize, osize);
}


void UiPacker::doCallback(unsigned isize, unsigned osize)
{
    int i;
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

    // compute progress position
    int pos = -1;
    if (isize >= s->u_len)
        pos = s->bar_len;
    else if (isize > 0)
    {
        pos = get_ratio(s->u_len, isize) * s->bar_len / 1000000;
        assert(pos >= 0); assert(pos <= s->bar_len);
    }

#if 0
    printf("%6d %6d %6d %6d %3d %3d\n", isize, osize, s->step, s->next_update, pos, s->pos);
    return;
#endif

    if (pos < s->pos)
        return;
    if (pos < 0 && pos == s->pos)
        return;

    // fill the progress bar
    char *m = &s->msg_buf[s->bar_pos];
    *m++ = progress_filler[2];
    for (i = 0; i < s->bar_len; i++)
        *m++ = progress_filler[i <= pos];
    *m++ = progress_filler[3];

    // compute current compression ratio
    unsigned ratio = 1000000;
    if (osize > 0)
        ratio = get_ratio(isize, osize);

    int buflen = (int) (&s->msg_buf[sizeof(s->msg_buf)] - m);
    upx_snprintf(m, buflen, "  %3d.%1d%%  %c ",
                 ratio / 10000, (ratio % 10000) / 1000,
                 spinner[s->spin_counter & 3]);
    assert(strlen(s->msg_buf) < 1 + 80);

    s->pos = pos;
    s->spin_counter++;

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
        // FIXME: this doesn't honor '--mono' etc.
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
    uiUpdate(fo->st_size());

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
                mkline(p->ph.u_file_size, fo->st_size(),
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

    if (s->mode == M_QUIET)
        return;

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
        upx_snprintf(name, sizeof(name), "[ %ld file%s ]",
                     total_files_done, total_files_done == 1 ? "" : "s");
        con_fprintf(stdout, "%s%s\n",
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
    con_fprintf(stdout,"%s [%s, %s]\n", p->fi->getName(), p->getFullName(opt), p->getName());
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

