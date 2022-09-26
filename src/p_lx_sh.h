/* p_lx_sh.h --

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


#ifndef __UPX_P_LX_SH_H  //{
#define __UPX_P_LX_SH_H 1


/*************************************************************************
// linux/sh386
**************************************************************************/

class PackLinuxI386sh : public PackLinuxI386
{
    typedef PackLinuxI386 super;
public:
    PackLinuxI386sh(InputFile *f);
    virtual ~PackLinuxI386sh();
    virtual int getVersion() const override { return 13; }
    virtual int getFormat() const override { return UPX_F_LINUX_SH_i386; }
    virtual const char *getName() const override { return "linux.sh/i386"; }
    virtual const char *getFullName(const options_t *) const override { return "i386-linux.elf.shell"; }
    virtual const int *getFilters() const override { return nullptr; }
    virtual void buildLoader(const Filter *) override;

    virtual void pack1(OutputFile *fo, Filter &ft) override;
    virtual off_t pack3(OutputFile *fo, Filter &ft) override;

    virtual bool canPack() override;
    // virtual void unpack(OutputFile *fo) { super::unpack(fo); }
    virtual bool canUnpackVersion(int version) const override
        { return (version >= 11); }

protected:
    virtual bool getShellName(char *buf);

    virtual void patchLoader() override;

    int o_shname;  // offset to name_of_shell
    int l_shname;  // length of name_of_shell
};


#endif /*} already included */

/* vim:set ts=4 sw=4 et: */
