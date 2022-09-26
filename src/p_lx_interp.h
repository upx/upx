/* p_lx_interp.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2022 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2022 Laszlo Molnar
   Copyright (C) 2000-2022 John F. Reiser
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

   John F. Reiser
   <jreiser@users.sourceforge.net>
 */


#ifndef __UPX_P_LX_INTERP_H  //{
#define __UPX_P_LX_INTERP_H 1


/*************************************************************************
// linux/interp386
**************************************************************************/

class PackLinuxElf32x86interp : public PackLinuxElf32x86
{
    typedef PackLinuxElf32x86 super;
public:
    PackLinuxElf32x86interp(InputFile *f);
    virtual ~PackLinuxElf32x86interp();
    virtual int getVersion() const override { return 13; }
    virtual int getFormat() const override { return UPX_F_LINUX_ELFI_i386; }
    virtual const char *getName() const override { return "linux/elfi386"; }
    virtual const char *getFullName(const options_t *) const override { return "i386-linux.elf.interp"; }

    virtual bool canPack() override;
    virtual void unpack(OutputFile *fo) override;

protected:
    virtual void pack1(OutputFile *, Filter &) override;  // generate executable header
    virtual int  pack2(OutputFile *, Filter &) override;  // append compressed data
    virtual off_t pack3(OutputFile *, Filter &) override;  // build loader
};


#endif /*} already included */

/* vim:set ts=4 sw=4 et: */
