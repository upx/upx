/* c_none.cpp --

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


#include "conf.h"

#if (USE_CONSOLE)

/*************************************************************************
//
**************************************************************************/

static int init(FILE *f, int o, int now)
{
    UNUSED(f);
    UNUSED(o);
    UNUSED(now);
    return CON_NONE;
}


static int set_fg(FILE *f, int fg)
{
    UNUSED(f);
    UNUSED(fg);
    return -1;
}


static void print0(FILE *f, const char *s)
{
    UNUSED(f);
    UNUSED(s);
}


static bool intro(FILE *f)
{
    UNUSED(f);
    return 0;
}


console_t console_none =
{
    init,
    set_fg,
    print0,
    intro
};

#endif /* USE_CONSOLE */

/* vim:set ts=4 sw=4 et: */
