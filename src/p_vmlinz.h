/* p_vmlinz.h --

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


#ifndef __UPX_P_VMLINZ_H
#define __UPX_P_VMLINZ_H


/*************************************************************************
// vmlinuz/i386 (zlib compressed Linux kernel image)
**************************************************************************/

class PackVmlinuzI386 : public Packer
{
    typedef Packer super;
public:
    PackVmlinuzI386(InputFile *f);
    virtual int getVersion() const { return 1; }
    virtual int getFormat() const { return UPX_F_VMLINUZ_i386; }
    virtual const char *getName() const { return "vmlinuz/386"; }
    virtual int getCompressionMethod() const;
    virtual const int *getFilters() const;

    virtual void pack(OutputFile *fo);
    virtual void unpack(OutputFile *fo);

    virtual bool canPack();
    virtual int canUnpack();

protected:
    virtual int readFileHeader();
    virtual int uncompressKernel();
    virtual void readKernel();

//    virtual const upx_byte *getLoader() const;
//    virtual int getLoaderSize() const;

    struct boot_sect_t
    {
        char    _[0x1f1];
        char    setup_sects;
        char    _1[2];
        LE16    sys_size;
        char    _2[8];
        LE16    boot_flag;     // 0xAA55
        // 0x200
        char    _3[2];
        char    hdrs[4];        // "HdrS"
        LE16    version;        // boot protocol
        char    _4[9];
        char    load_flags;
        char    _5[2];
        LE32    code32_start;

        // some more uninteresting fields here ...
        // see /usr/src/linux/Documentation/i386/
    };

    MemBuffer setup_buf;
    int setup_size;
};


/*************************************************************************
// bvmlinuz/i386 (zlib compressed Linux kernel image)
**************************************************************************/

class PackBvmlinuzI386 : public PackVmlinuzI386
{
    typedef PackVmlinuzI386 super;
public:
    PackBvmlinuzI386(InputFile *f) : super(f) { }
    virtual int getFormat() const { return UPX_F_BVMLINUZ_i386; }
    virtual const char *getName() const { return "bvmlinuz/386"; }

    virtual void pack(OutputFile *fo);
};


#endif /* already included */


/*
vi:ts=4:et
*/

