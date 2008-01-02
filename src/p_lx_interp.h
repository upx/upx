/* p_lx_interp.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
   Copyright (C) 2000-2008 John F. Reiser
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

   John F. Reiser
   <jreiser@users.sourceforge.net>
 */


#ifndef __UPX_P_LX_INTERP_H  //{
#define __UPX_P_LX_INTERP_H


/*************************************************************************
// linux/interp386
**************************************************************************/

class PackLinuxElf32x86interp : public PackLinuxElf32x86
{
    typedef PackLinuxElf32x86 super;
public:
    PackLinuxElf32x86interp(InputFile *f);
    virtual ~PackLinuxElf32x86interp();
    virtual int getVersion() const { return 13; }
    virtual int getFormat() const { return UPX_F_LINUX_ELFI_i386; }
    virtual const char *getName() const { return "linux/elfi386"; }
    virtual const char *getFullName(const options_t *) const { return "i386-linux.elf.interp"; }

    virtual bool canPack();
    virtual void unpack(OutputFile *fo);

protected:
    virtual void pack1(OutputFile *, Filter &);  // generate executable header
    virtual void pack2(OutputFile *, Filter &);  // append compressed data
    virtual void pack3(OutputFile *, Filter &);  // build loader
};


#endif /*} already included */


/*
vi:ts=4:et
*/

