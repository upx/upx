/* p_psx.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2001 Laszlo Molnar

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

   Markus F.X.J. Oberhumer                   Laszlo Molnar
   markus.oberhumer@jk.uni-linz.ac.at        ml1050@cdata.tvnet.hu
 */


#ifndef __UPX_P_PSX_H
#define __UPX_P_PSX_H

#include "version.h"

#define MIPS_HI(a) (((a)>>16)/*+((a&0x8000)>>15)*/)
#define MIPS_LO(a) (a&0xffff)
#define MIPS_JP(a) ((0x08<<24)|((a&0xfffffff)>>2))
#define CHK_ALIGNED(a,b) (b-(a%b))

#define PS_HDR_SIZE  2048  //one cd sector in bytes
#define PS_HDR       16

#define HDR_SIZE     (sizeof(p_hdr_t))
#define VERSION_ID   (sizeof(UPX_VERSION_STRING))

#define PS_MAX_SIZE  0x1e8000


/*************************************************************************
// psx/exe
**************************************************************************/

class PackPsx : public Packer
{
    typedef Packer super;
public:
    PackPsx(InputFile *f) : super(f) { }
    virtual int getVersion() const { return 11; }
    virtual int getFormat() const { return UPX_F_PSX_EXE; }
    virtual const char *getName() const { return "psx/exe"; }
    virtual const int *getCompressionMethods(int method, int level) const;
    virtual const int *getFilters() const;

    virtual void pack(OutputFile *fo);
    virtual void unpack(OutputFile *fo);

    virtual bool canPack();
    virtual int canUnpack();

protected:
    virtual int buildLoader(const Filter *ft);
    virtual int patch_mips_le16(void *b, int blen, const void *old, unsigned new_);
    virtual int patch_mips_le32(void *b, int blen, const void *old, unsigned new_);
    virtual int patch_hi_lo(void *b, int blen, const void *old_hi, const void *old_lo, unsigned new_);

#if 0
    typedef struct
    {
        upx_byte id[VERSION_ID];
        upx_byte ph_hdr[32];
        LE32 hdr_adler;
        exec_hdr_t exec_bkup;
    }
    upx_hdr_t;
#endif

    struct psx_exe_t
    {
        char id[8];
        LE32 text;
        LE32 data;
        LE32 epc;
        LE32 gp;
        LE32 tx_ptr;
        LE32 tx_len;
        LE32 da_ptr;
        LE32 da_len;
        LE32 bs_ptr;
        LE32 bs_len;
        LE32 sd_ptr;
        LE32 sd_len;
        LE32 sp,fp,gp0,ra,k0;
        char origin[60];
        char pad[14];
    };

    struct psx_exe_t ih, oh;

    upx_uint overlap;
    upx_uint scan_count;

    upx_uint file_data_size;
    upx_uint right_file_size;

    MemBuffer pad_data;
};


#endif /* already included */


/*
vi:ts=4:et
*/

