/* ui.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2004 Laszlo Molnar
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
   <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>
 */


#ifndef __UPX_UI_H
#define __UPX_UI_H

class InputFile;
class OutputFile;
class Packer;
class UiPacker;

#if defined(WITH_GUI)
class CMainDlg;
#endif


/*************************************************************************
//
**************************************************************************/

struct UiPacker__State;

class UiPacker
{
public:
    UiPacker(const Packer *p_);
public:
    virtual ~UiPacker();

    static void uiConfirmUpdate();
    static void uiPackTotal();
    static void uiUnpackTotal();
    static void uiListTotal(bool uncompress=false);
    static void uiTestTotal();
    static void uiFileInfoTotal();

public:
    virtual void uiPackStart(const OutputFile *fo);
    virtual void uiPackEnd(const OutputFile *fo);
    virtual void uiUnpackStart(const OutputFile *fo);
    virtual void uiUnpackEnd(const OutputFile *fo);
    virtual void uiListStart();
    virtual void uiList(long fu=-1);
    virtual void uiListEnd();
    virtual void uiTestStart();
    virtual void uiTestEnd();
    virtual bool uiFileInfoStart();
    virtual void uiFileInfoEnd();

    // callback
    typedef upx_progress_callback_t cb_t;
    virtual void startCallback(unsigned u_len, unsigned step,
                               int pass, int total_passes);
    virtual void firstCallback();
    virtual void finalCallback(unsigned u_len, unsigned c_len);
    virtual void endCallback();
    virtual cb_t *getCallback() { return &cb; }
protected:
    static void __UPX_CDECL callback(upx_uint isize, upx_uint osize,
                                     int, void *);
    virtual void doCallback(unsigned isize, unsigned osize);

protected:
    virtual void uiUpdate(long fc=-1, long fu=-1);

public:
    static void uiHeader();
    static void uiFooter(const char *n);

protected:
    virtual void printInfo(int nl=0);
    const Packer *p;

    // callback
    cb_t cb;

    // internal state
    UiPacker__State *s;

    // totals
    static long total_files;
    static long total_files_done;
    static long total_c_len;
    static long total_u_len;
    static long total_fc_len;
    static long total_fu_len;
    static long update_c_len;
    static long update_u_len;
    static long update_fc_len;
    static long update_fu_len;

#if defined(WITH_GUI)
    CMainDlg* pMain;
#endif
};


#endif /* already included */


/*
vi:ts=4:et
*/

