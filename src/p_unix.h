/* p_unix.h --

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


#ifndef __UPX_P_UNIX_H
#define __UPX_P_UNIX_H


/*************************************************************************
// Abstract class for all Unix-type packers.
// Already provides most of the functionality.
**************************************************************************/

class PackUnix : public Packer
{
    typedef Packer super;
protected:
    PackUnix(InputFile *f);
public:
    virtual int getVersion() const { return 13; }
    virtual const int *getFilters() const { return NULL; }
    virtual int getStrategy(Filter &);

    virtual void pack(OutputFile *fo);
    virtual void unpack(OutputFile *fo);

    virtual bool canPack();
    virtual int canUnpack();

protected:
    // called by the generic pack()
    virtual void pack1(OutputFile *, Filter &);  // generate executable header
    virtual void pack2(OutputFile *, Filter &);  // append compressed data
    virtual void pack3(OutputFile *, Filter &);  // append loader
    virtual void pack4(OutputFile *, Filter &);  // append PackHeader

    virtual void patchLoader() = 0;
    virtual void patchLoaderChecksum();
    virtual void updateLoader(OutputFile *) = 0;

    virtual void writePackHeader(OutputFile *fo);

    virtual bool checkCompressionRatio(unsigned, unsigned) const;

protected:
    struct Extent {
        off_t offset;
        off_t size;
    };
    virtual void packExtent(const Extent &x,
        unsigned &total_in, unsigned &total_out, Filter *, OutputFile *,
        unsigned hdr_len = 0);
    virtual void unpackExtent(unsigned wanted, OutputFile *fo,
        unsigned &total_in, unsigned &total_out,
        unsigned &c_adler, unsigned &u_adler,
        bool first_PF_X, unsigned szb_info );

    int exetype;
    unsigned blocksize;
    unsigned progid;              // program id
    unsigned overlay_offset;      // used when decompressing

    MemBuffer loader;
    int lsize;
    MemBuffer pt_dynamic;
    int sz_dynamic;

    unsigned b_len;  // total length of b_info blocks

    // must agree with stub/linux.hh
    struct b_info { // 12-byte header before each compressed block
        unsigned sz_unc;  // uncompressed_size
        unsigned sz_cpr;  //   compressed_size
        unsigned char b_method;  // compression algorithm
        unsigned char b_ftid;  // filter id
        unsigned char b_cto8;  // filter parameter
        unsigned char b_unused;
    }
    __attribute_packed;
    struct l_info { // 12-byte trailer in header for loader
        LE32 l_checksum;
        LE32 l_magic;
        LE16 l_lsize;
        unsigned char l_version;
        unsigned char l_format;
    }
    __attribute_packed;

    struct p_info { // 12-byte packed program header
        unsigned p_progid;
        unsigned p_filesize;
        unsigned p_blocksize;
    }
    __attribute_packed;

    struct l_info linfo;

    // do not change !!!
    enum { OVERHEAD = 2048 };
};


/*************************************************************************
// abstract classes encapsulating endian issues
// note: UPX_MAGIC is always stored in le32 format
**************************************************************************/
class PackUnixBe32 : public PackUnix
{
    typedef PackUnix super;
protected:
    PackUnixBe32(InputFile *f) : super(f) { bele = &N_BELE_RTP::be_policy; }

    // must agree with stub/linux.hh
    struct b_info { // 12-byte header before each compressed block
        BE32 sz_unc;  // uncompressed_size
        BE32 sz_cpr;  //   compressed_size
        unsigned char b_method;  // compression algorithm
        unsigned char b_ftid;  // filter id
        unsigned char b_cto8;  // filter parameter
        unsigned char b_unused;
    }
    __attribute_packed;
    struct l_info { // 12-byte trailer in header for loader
        BE32 l_checksum;
        BE32 l_magic;
        BE16 l_lsize;
        unsigned char l_version;
        unsigned char l_format;
    }
    __attribute_packed;

    struct p_info { // 12-byte packed program header
        BE32 p_progid;
        BE32 p_filesize;
        BE32 p_blocksize;
    }
    __attribute_packed;

};


class PackUnixLe32 : public PackUnix
{
    typedef PackUnix super;
protected:
    PackUnixLe32(InputFile *f) : super(f) { bele = &N_BELE_RTP::le_policy; }

    // must agree with stub/linux.hh
    struct b_info { // 12-byte header before each compressed block
        LE32 sz_unc;  // uncompressed_size
        LE32 sz_cpr;  //   compressed_size
        unsigned char b_method;  // compression algorithm
        unsigned char b_ftid;  // filter id
        unsigned char b_cto8;  // filter parameter
        unsigned char b_unused;
    }
    __attribute_packed;
    struct l_info { // 12-byte trailer in header for loader
        LE32 l_checksum;
        LE32 l_magic;
        LE16 l_lsize;
        unsigned char l_version;
        unsigned char l_format;
    }
    __attribute_packed;

    struct p_info { // 12-byte packed program header
        LE32 p_progid;
        LE32 p_filesize;
        LE32 p_blocksize;
    }
    __attribute_packed;

};


/*************************************************************************
// solaris/sparc
**************************************************************************/

#if 0
class PackSolarisSparc : public PackUnixBe32
{
    typedef PackUnixBe32 super;
public:
    PackSolarisSparc(InputFile *f) : super(f) { }
    virtual int getFormat() const { return UPX_F_SOLARIS_SPARC; }
    virtual const char *getName() const { return "solaris/sparc"; }

    virtual bool canPack();

protected:
    virtual upx_byte *getLoader() const;
    virtual int getLoaderSize() const;

    virtual void patchLoader();
};
#endif /* #if 0 */


#endif /* already included */


/*
vi:ts=4:et
*/

