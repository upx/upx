/* s_object.cpp -- base of all screen drivers

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

#define this local_this

#include "screen.h"


/*************************************************************************
//
**************************************************************************/

// ugly hacks
static screen_t *last_screen = NULL;

screen_t *sobject_get_screen(void)
{
    return last_screen;
}


void sobject_destroy(screen_t *this)
{
    last_screen = NULL;
    if (!this)
        return;
    if (this->data)
    {
        if (this->finalize)
            this->finalize(this);
        free(this->data);
        this->data = NULL;
    }
    free(this);
}


screen_t *sobject_construct(const screen_t *c, size_t data_size)
{
    screen_t *this;

    last_screen = NULL;

    /* allocate object */
    this = (screen_t *) malloc(sizeof(*this));
    if (!this)
        return NULL;

    /* copy function table */
    *this = *c;

    /* initialize instance variables */
    this->data = (struct screen_data_t *) malloc(data_size);
    if (!this->data)
    {
        free(this);
        return NULL;
    }
    memset(this->data,0,data_size);

    last_screen = this;
    return this;
}


#endif /* defined(USE_SCREEN) */


/*
vi:ts=4:et
*/

