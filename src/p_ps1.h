/* p_ps1.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2004 Laszlo Molnar
   Copyright (C) 2002-2004 Jens Medoch
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

   Jens Medoch
   <jssg@users.sourceforge.net>
 */


#ifndef __UPX_P_PS1_H
#define __UPX_P_PS1_H


/*************************************************************************
// ps1/exe
**************************************************************************/

class PackPs1 : public Packer
{
    typedef Packer super;
public:
    PackPs1(InputFile *f);
    virtual int getVersion() const { return 13; }
    virtual int getFormat() const { return UPX_F_PS1_EXE; }
    virtual const char *getName() const { return "ps1/exe"; }
    virtual const int *getCompressionMethods(int method, int level) const;
    virtual const int *getFilters() const;

    virtual void pack(OutputFile *fo);
    virtual void unpack(OutputFile *fo);

    virtual bool canPack();
    virtual int canUnpack();

protected:
    virtual void patch_mips_le(void *b, int blen, const void *old, unsigned new_);
    virtual int buildLoader(const Filter *ft);

    virtual int readFileHeader();
    virtual bool checkFileHeader();

    struct ps1_exe_t
    {
        // ident string
        char id[8];
        // is NULL
        LE32 text;
        // is NULL
        LE32 data;
        // initial program counter
        LE32 epc;
        // initial gp register value
        LE32 gp;
        // load offset of binary data
        LE32 tx_ptr, tx_len;
        LE32 da_ptr, da_len;
        LE32 bs_ptr, bs_len;
        // initial stack params
        LE32 is_ptr, is_len;
        // saved regs on execution
        LE32 sp, fp, gp0, ra, k0;
        // origin Jap/USA/Europe
        char origin[60];

        // some safety space after that
        char pad[8];
        // backup of the original header (epc - is_len)
        LE32 ih_bkup[10];
        // plus checksum for the backup
        LE32 ih_csum;
        // id & the upx header.
    }
    __attribute_packed;

    ps1_exe_t ih, oh;

    bool isCon;
    bool is32Bit;
    unsigned overlap;
    unsigned sa_cnt;

    // filesize-PS_HDR_SIZE
    unsigned fdata_size;
};


#endif /* already included */


/*
vi:ts=4:et
*/


