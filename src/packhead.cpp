/* packhead.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2000 Laszlo Molnar
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

   Markus F.X.J. Oberhumer                   Laszlo Molnar
   markus.oberhumer@jk.uni-linz.ac.at        ml1050@cdata.tvnet.hu
 */


#include "conf.h"
#include "packer.h"


/*************************************************************************
// packheader
//
// We try to be able to unpack UPX 0.7x (versions 8 & 9) and at
// least to detect older versions, so this is a little bit messy.
**************************************************************************/

// simple checksum for the header itself (since version 10)
static unsigned char get_packheader_checksum(const upx_bytep buf, int len)
{
    assert(get_le32(buf) == UPX_MAGIC_LE32);
    //printf("1 %d\n", len);
    buf += 4;
    len -= 4;
    unsigned c = 0;
    while (len-- > 0)
        c += *buf++;
    c %= 251;
    //printf("2 %d\n", c);
    return (unsigned char) c;
}


/*************************************************************************
//
**************************************************************************/

static int get_packheader_size(int version, int format)
{
    int n = 0;
    if (version <= 3)
        n = 24;
    else if (version <= 9)
    {
        if (format == UPX_F_DOS_COM || format == UPX_F_DOS_SYS)
            n = 20;
        else if (format == UPX_F_DOS_EXE || format == UPX_F_DOS_EXEH)
            n = 25;
        else
            n = 28;
    }
    else
    {
        if (format == UPX_F_DOS_COM || format == UPX_F_DOS_SYS)
            n = 22;
        else if (format == UPX_F_DOS_EXE || format == UPX_F_DOS_EXEH)
            n = 27;
        else
            n = 32;
    }
    if (n == 0)
        throwCantUnpack("unknown header version");
    return n;
}


int PackHeader::getPackHeaderSize() const
{
    return get_packheader_size(version, format);
}


/*************************************************************************
//
**************************************************************************/

void PackHeader::putPackHeader(upx_bytep buf, unsigned len)
{
#if defined(UNUPX)
    throwBadLoader();
#else
    int offset = find_le32(buf,len,magic);
    if (offset < 0)
        throwBadLoader();
    upx_bytep l = buf + offset;

    l[4] = (unsigned char) version;
    l[5] = (unsigned char) format;
    l[6] = (unsigned char) method;
    l[7] = (unsigned char) level;

    // the new variable length header
    if (format < 128)
    {
        set_le32(l+8,u_adler);
        set_le32(l+12,c_adler);
        if (format == UPX_F_DOS_COM || format == UPX_F_DOS_SYS)
        {
            set_le16(l+16,u_len);
            set_le16(l+18,c_len);
            l[20] = (unsigned char) filter;
        }
        else if (format == UPX_F_DOS_EXE || format == UPX_F_DOS_EXEH)
        {
            set_le24(l+16,u_len);
            set_le24(l+19,c_len);
            set_le24(l+22,u_file_size);
            l[25] = (unsigned char) filter;
        }
        else
        {
            set_le32(l+16,u_len);
            set_le32(l+20,c_len);
            set_le32(l+24,u_file_size);
            l[28] = (unsigned char) filter;
            l[29] = (unsigned char) filter_cto;
            l[30] = 0;
        }
    }
    else
    {
        set_be32(l+8,u_len);
        set_be32(l+12,c_len);
        set_be32(l+16,u_adler);
        set_be32(l+20,c_adler);
        set_be32(l+24,u_file_size);
        l[28] = (unsigned char) filter;
        l[29] = (unsigned char) filter_cto;
        l[30] = 0;
    }

    // store header_checksum
    const int hs = getPackHeaderSize();
    l[hs - 1] = get_packheader_checksum(l, hs - 1);
#endif /* UNUPX */
}


/*************************************************************************
//
**************************************************************************/

bool PackHeader::fillPackHeader(upx_bytep buf, unsigned len)
{
    int offset = find_le32(buf,len,magic);
    if (offset < 0)
        return false;
    const int hlen = len - offset;
    if (hlen < 8)
        return false;

    upx_bytep l = buf + offset;
    buf_offset = offset;

    version = l[4];
    format = l[5];
    method = l[6];
    level = l[7];
    filter_cto = 0;

    const int hs = getPackHeaderSize();
    if (hs > hlen)
        throwCantUnpack("header corrupted");

    // the new variable length header
    int off_filter = 0;
    if (format < 128)
    {
        u_adler = get_le32(l+8);
        c_adler = get_le32(l+12);
        if (format == UPX_F_DOS_COM || format == UPX_F_DOS_SYS)
        {
            u_len = get_le16(l+16);
            c_len = get_le16(l+18);
            u_file_size = u_len;
            off_filter = 20;
        }
        else if (format == UPX_F_DOS_EXE || format == UPX_F_DOS_EXEH)
        {
            u_len = get_le24(l+16);
            c_len = get_le24(l+19);
            u_file_size = get_le24(l+22);
            off_filter = 25;
        }
        else
        {
            u_len = get_le32(l+16);
            c_len = get_le32(l+20);
            u_file_size = get_le32(l+24);
            off_filter = 28;
            filter_cto = l[29];
        }
    }
    else
    {
        u_len = get_be32(l+8);
        c_len = get_be32(l+12);
        u_adler = get_be32(l+16);
        c_adler = get_be32(l+20);
        u_file_size = get_be32(l+24);
        off_filter = 28;
        filter_cto = l[29];
    }

    if (version >= 10)
        filter = l[off_filter];
    else if ((level & 128) == 0)
        filter = 0;
    else
    {
        // convert old flags to new filter id
        level &= 127;
        if (format == UPX_F_DOS_COM || format == UPX_F_DOS_SYS)
            filter = 0x06;
        else
            filter = 0x26;
    }
    level &= 15;

    return true;
}


bool PackHeader::checkPackHeader(const upx_bytep hbuf, int hlen) const
{
    if (version == 0xff)
        throwCantUnpack("cannot unpack UPX ;-)");

    const int hs = getPackHeaderSize();
    if (hlen <= 0 || hs > hlen)
        throwCantUnpack("header corrupted");

    // check header_checksum
    if (version > 9)
        if (hbuf[hs - 1] != get_packheader_checksum(hbuf, hs - 1))
            throwCantUnpack("header corrupted");

    return true;
}


/*
vi:ts=4:et:nowrap
*/

