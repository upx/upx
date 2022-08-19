/* p_com.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2022 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2022 Laszlo Molnar
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


#ifndef __UPX_P_COM_H
#define __UPX_P_COM_H 1


/*************************************************************************
// dos/com
**************************************************************************/

class PackCom : public Packer
{
    typedef Packer super;
public:
    PackCom(InputFile *f) : super(f) { bele = &N_BELE_RTP::le_policy; }
    virtual int getVersion() const override { return 13; }
    virtual int getFormat() const override { return UPX_F_DOS_COM; }
    virtual const char *getName() const override { return "dos/com"; }
    //virtual const char *getFullName(const options_t *o) const override { return o && o->cpu == o->CPU_8086 ? "i086-dos16.com" : "i286-dos16.com"; }
    virtual const char *getFullName(const options_t *) const override { return "i086-dos16.com"; }
    virtual const int *getCompressionMethods(int method, int level) const override;
    virtual const int *getFilters() const override;

    virtual void pack(OutputFile *fo) override;
    virtual void unpack(OutputFile *fo) override;

    virtual bool canPack() override;
    virtual int canUnpack() override;

protected:
    virtual unsigned getCallTrickOffset() const { return 0x100; }
    virtual Linker* newLinker() const override;

protected:
    virtual void buildLoader(const Filter *ft) override;
    virtual void patchLoader(OutputFile *fo, upx_byte *, int, unsigned);
    virtual void addFilter16(int filter_id);
};


#endif /* already included */

/* vim:set ts=4 sw=4 et: */
