/* p_lx_sep.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2001 Laszlo Molnar
   Copyright (C) 2000-2001 John F. Reiser
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

   John F. Reiser
   jreiser@BitWagon.com
 */


#ifndef __UPX_P_LX_SEP_H  //{
#define __UPX_P_LX_SEP_H


/*************************************************************************
// linux/sep386
**************************************************************************/

class PackLinuxI386sep : public PackLinuxI386elf
{
    typedef PackLinuxI386elf super;
public:
    PackLinuxI386sep(InputFile *f) : super(f) { }
    virtual int getFormat() const { return UPX_F_LINUX_SEP_i386; }
    virtual const char *getName() const { return "linux/sep386"; }

protected:
    virtual const upx_byte *getLoader() const;
    virtual int getLoaderSize() const;
    virtual int getLoaderPrefixSize() const;

    virtual void patchLoader();
    virtual void updateLoader(OutputFile *) {}
};


#endif /*} already included */

/*
vi:ts=4:et
*/

